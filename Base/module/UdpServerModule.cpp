#include "UdpServerModule.h"
#include "ScheduleModule.h"

thread_local int32_t UdpConn::SOCKET = 0;

UdpServerModule::UdpServerModule(BaseLayer * l):BaseModule(l), m_sockIndex(0), m_tmpcash(NULL),
m_rSendHead({}), m_rSendTail(NULL)
{
}

UdpServerModule::~UdpServerModule()
{
}

void UdpServerModule::Init()
{
	m_schedule = GET_MODULE(ScheduleModule);

	m_schedule->AddInterValTask(this, &UdpServerModule::Loop_Once, 10);
}

void UdpServerModule::Execute()
{
	m_context.poll();
}

void UdpServerModule::Listen(const int32_t & port)
{
	m_socket.reset(new udp::socket(m_context, udp::endpoint(udp::v4(), port)));
	Do_receive();
}

void UdpServerModule::Loop_Once(int64_t& dt)
{
	SetTick(dt);
	CheckReSend();
}

void UdpServerModule::SendData(int32_t sock, const char * data, const int32_t & size)
{
	auto it = m_clients.find(sock);
	if (it == m_clients.end())
		return;

	int8_t pn = ceil(size / float(UDP_DATA_SIZE));
	int8_t idx = 0;
	int32_t sendsize = 0;
	while (sendsize < size)
	{
		auto rsize = size - sendsize > UDP_DATA_SIZE ? UDP_DATA_SIZE : size - sendsize;
		auto buf = DecodeSendBuff(it->second, data + sendsize, rsize, pn, idx++);
		SendData(it->second, buf);
		sendsize += rsize;
	}
}

void UdpServerModule::CloseSocket(int32_t sock)
{
	m_clients.erase(sock);
}

void UdpServerModule::Do_receive()
{
	if (!m_tmpcash)
		m_tmpcash = GET_LOOP(UdpBuff);
	m_socket->async_receive_from(as::buffer(m_tmpcash->buf, UDP_MUT_SIZE), m_accept,
	[this](boost::system::error_code ec, std::size_t size) {
		if (!ec && size > 0)
		{
			if (size == 4)
				OnClientConnect();
			else
			{
				m_tmpcash->size = size;
				auto buff = m_tmpcash;
				m_tmpcash = NULL;
				ReceiveDecode(buff);
			}
		}
		Do_receive();
	});
}

void UdpServerModule::ReceiveDecode(UdpBuff * buf)
{
	auto sock = buf->read<int32_t>();
	auto it = m_clients.find(sock);
	if (it == m_clients.end())
	{
		LOOP_RECYCLE(buf);
		return;
	}

	auto ptype = buf->read<int8_t>();
	switch (ptype)
	{
	case NOR_PACK:
		ReceiveNorPack(sock, it->second, buf);
		break;
	case REQ_PACK:
		ReceiveReqPack(sock, it->second, buf);
		break;
	case ACK_GET:
		ReceiveAckPack(sock, it->second, buf);
		break;
	default:
		break;
	}
	LOOP_RECYCLE(buf);
}

void UdpServerModule::ReceiveNorPack(const int32_t& sock, SHARE<UdpConn>& conn, UdpBuff * buf)
{
	auto pn = buf->read<int8_t>();
	for (size_t i = 0; i < pn; i++)
	{
		auto size = buf->read<int16_t>();
		const char* buff = &buf->buf[buf->offset];
		buf->offset += size;
		if (m_onRead)
			m_onRead(sock, buff, size);
	}
}

void UdpServerModule::ReceiveReqPack(const int32_t& sock, SHARE<UdpConn>& conn, UdpBuff * buf)
{
	auto reqsize = buf->read<int8_t>();
	for (size_t i = 0; i < reqsize; i++)
	{
		auto packid = buf->read<uint16_t>();
		Resendpack(conn, packid);
	}
}

void UdpServerModule::ReceiveAckPack(const int32_t& sock, SHARE<UdpConn>& conn, UdpBuff * buf)
{
	auto acksize = buf->read<int8_t>();
	for (size_t i = 0; i < acksize; i++)
	{
		auto packid = buf->read<uint16_t>();
		auto idx = packid % UDP_CASH_SIZE;
		if (conn->hestory[idx] && conn->hestory[idx]->packId == packid)
		{
			LOOP_RECYCLE(conn->hestory[idx]);
			conn->hestory[idx] = NULL;
			if (idx == conn->minIndex)
			{
				while (conn->minIndex != conn->maxIndex)
				{
					++conn->minIndex;
					if (conn->hestory[conn->minIndex])
						break;
				}
			}
		}
		//std::cout << "ack .................. pack id:" << packid << std::endl;
	}
	//std::cout << "ack ...........======== minIndex id:" << (int32_t)conn->minIndex << " maxIndex:" << (int32_t)conn->maxIndex << std::endl;
}

void UdpServerModule::Resendpack(SHARE<UdpConn>& conn, const uint16_t & packId)
{
	auto idx = packId % UDP_CASH_SIZE;
	if (conn->hestory[idx] && conn->hestory[idx]->packId == packId)
		SendData(conn, conn->hestory[idx]);
}

void UdpServerModule::OnClientConnect()
{
	auto conn = GET_SHARE(UdpConn);;
	auto nsock = conn->socket;
	m_clients[nsock] = conn;
	conn->addr = m_accept;

	auto buff = DecodeSendBuff(conn, (char*)&nsock, sizeof(int32_t), 1, 0);
	SendData(conn, buff);
	if (m_onConnect)
		m_onConnect(nsock);
}

UdpBuff * UdpServerModule::DecodeSendBuff(SHARE<UdpConn>& conn, const char * buff, const int16_t & size, const int8_t& pn, const int8_t& idx)
{
	auto ub = GET_LOOP(UdpBuff);
	conn->PushHestory(ub);

	ub->write(ub->packId);
	ub->write(idx);
	ub->write(static_cast<int8_t>(pn - 1 - idx));
	ub->write(size);
	ub->write(buff, size);
	ub->reSendDT = m_nowTick + UDP_OUT_TIME_RESEND;
	PushBack(conn->socket, ub->packId);
	return ub;
}

void UdpServerModule::SendData(SHARE<UdpConn>& conn, UdpBuff * buff)
{
	RealSendData(buff->buf, buff->size, conn->addr);
	//²âÊÔ 50% ¶ª°ü
	/*auto randSend = rand() % 2;
	if (randSend)
		RealSendData(buff->buf, buff->size, conn->addr);
	else
		std::cout << "rand Lost Pack id:" << buff->packId << std::endl;*/
}

void UdpServerModule::RealSendData(const char * data, const int32_t & len, as::ip::udp::endpoint & addr)
{
	m_socket->async_send_to(as::buffer(data, len), addr, [](boost::system::error_code, std::size_t) {});
}

void UdpServerModule::CheckReSend()
{
	auto node = m_rSendHead.next;
	while (node)
	{
		if (m_nowTick > node->point)
		{
			node = PopHead();

			auto it = m_clients.find(node->sock);
			if (it == m_clients.end())
			{
				LOOP_RECYCLE(node);
			}
			else
			{
				auto idx = node->packId % UDP_CASH_SIZE;
				if (it->second->hestory[idx] && it->second->hestory[idx]->packId == node->packId)
				{
					SendData(it->second, it->second->hestory[idx]);
					node->point = m_nowTick + UDP_OUT_TIME_RESEND;
					PushBack(node);
				}
				else
				{
					LOOP_RECYCLE(node);
				}
			}
			node = m_rSendHead.next;
		}
		else
			break;
	}
}

void UdpServerModule::PushBack(const int32_t & sock, const uint16_t & packId)
{
	auto node = GET_LOOP(RsendNode);
	node->next = NULL;
	node->packId = packId;
	node->sock = sock;
	node->point = UDP_OUT_TIME_RESEND + m_nowTick;
	PushBack(node);
}

void UdpServerModule::PushBack(RsendNode * node)
{
	if (m_rSendTail == NULL)
	{
		m_rSendHead.next = node;
		m_rSendTail = node;
	}
	else
	{
		m_rSendTail->next = node;
		m_rSendTail = node;
	}
}

RsendNode * UdpServerModule::PopHead()
{
	if (!m_rSendTail)
		return NULL;
	auto next = m_rSendHead.next->next;
	auto node = m_rSendHead.next;
	node->next = NULL;
	m_rSendHead.next = next;
	if (!next)
		m_rSendTail = NULL;
	return node;
}