#include "UdpServerModule.h"
#include "ScheduleModule.h"
thread_local int32_t UdpConn::SOCKET = 0;

UdpServerModule::UdpServerModule(BaseLayer * l):BaseModule(l), m_tmpcash(NULL),
m_rSendHead({}), m_rSendTail(NULL)
{
}

UdpServerModule::~UdpServerModule()
{
}

void UdpServerModule::Init()
{
	m_schedule = GET_MODULE(ScheduleModule);

	m_schedule->AddInterValTask(BIND_TIME(Loop_Once), 10);
}

void UdpServerModule::Execute()
{
	auto dt = GetMilliSecend();
	SetTick(dt);
	m_context.poll();
}

void UdpServerModule::Listen(const int32_t & port)
{
	/*auto sock = new udp::socket(m_context);
	auto ep = udp::endpoint(udp::v4(), port);
	sock->open(ep.protocol());
	sock->set_option(udp::socket::reuse_address(true));
	std::cout << "blocking " << sock->non_blocking() << std::endl;
	sock->bind(udp::endpoint(udp::v4(), port));
	m_socket.reset(sock);*/
	m_socket.reset(new udp::socket(m_context, udp::endpoint(udp::v4(), port)));
	Do_receive();
}

void UdpServerModule::Loop_Once(int64_t& dt)
{
	CheckReSend();
}

void UdpServerModule::SendData(int32_t sock, const char * data, const int32_t & size, bool _ack)
{
	auto it = m_clients.find(sock);
	if (it == m_clients.end())
		return;

	int8_t pn = static_cast<int8_t>(std::ceil(size / float(UDP_DATA_SIZE)));
	int8_t idx = 0;
	int32_t sendsize = 0;
	while (sendsize < size)
	{
		auto rsize = size - sendsize > UDP_DATA_SIZE ? UDP_DATA_SIZE : size - sendsize;
		auto buf = DecodeSendBuff(it->second, data + sendsize, rsize, pn, idx++, _ack);
		SendData(it->second, buf);
		sendsize += rsize;
	}
}

void UdpServerModule::CloseSocket(int32_t sock, bool call)
{
	auto it = m_clients.find(sock);
	if (it == m_clients.end())
		return;
	m_clients.erase(sock);
	if (call && m_onClose)
		m_onClose(sock);
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
				m_tmpcash->size = (int32_t)size;
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
		return;

	auto ptype = buf->read<int8_t>();
	switch (ptype)
	{
	case NOR_PACK:
		ReceiveNorPack(sock, it->second, buf);
		break;
	case ACK_GET:
		ReceiveAckPack(sock, it->second, buf);
		break;
	case PING_PACK:
		ReceivePingPack(it->second);
		break;
	default:
		break;
	}
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
		}
	}
}

void UdpServerModule::ReceivePingPack(SHARE<UdpConn>& conn)
{
	conn->outTime = m_nowTick + UDP_OUT_TIME_LINK;

	auto ub = GET_LOOP(UdpBuff);
	ub->write(static_cast<int8_t>(1));
	SendDataRecycle(conn, ub);
}

void UdpServerModule::OnClientConnect()
{
	auto conn = GET_SHARE(UdpConn);
	auto nsock = conn->socket;
	m_clients[nsock] = conn;
	conn->addr = m_accept;
	conn->outTime = m_nowTick + UDP_OUT_TIME_LINK;

	char bufsock[4];
	PB::WriteInt(bufsock, nsock);
	auto buff = DecodeSendBuff(conn, bufsock, sizeof(int32_t), 1, 0,true);
	SendData(conn, buff);
	if (m_onConnect)
		m_onConnect(nsock);
}

UdpBuff * UdpServerModule::DecodeSendBuff(SHARE<UdpConn>& conn, const char * buff, const int16_t & size, const int8_t& pn, const int8_t& idx,bool _ack)
{
	auto ub = GET_LOOP(UdpBuff);
	auto ptr = conn->PushHestory(ub,_ack);

	ub->write(ub->packId);
	ub->write((int8_t)(_ack));
	ub->write(idx);
	ub->write(static_cast<int8_t>(pn - 1 - idx));
	ub->write(size);
	ub->write(buff, size);
	if(_ack)
		PushBack(conn.get(),ptr);
	return ub;
}

void UdpServerModule::SendData(SHARE<UdpConn>& conn, UdpBuff * buff)
{
	if (buff->m_rsendNode)
		RealSendData(buff->buf, buff->size, conn->addr);
	else
		SendDataRecycle(conn, buff);
}

void UdpServerModule::SendData(UdpConn* conn, UdpBuff * buff)
{
	if (buff->m_rsendNode)
		RealSendData(buff->buf, buff->size, conn->addr);
	/*else
		SendDataRecycle(conn,buff);*/
}

void UdpServerModule::SendDataRecycle(SHARE<UdpConn>& conn, UdpBuff * buff)
{
	m_socket->async_send_to(as::buffer(buff->buf, buff->size), conn->addr, 
	[buff](boost::system::error_code, std::size_t) {
		LOOP_RECYCLE(buff);
	});
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
			if(node->m_buff == NULL)
				LOOP_RECYCLE(node);
			else
			{
				auto buff = *node->m_buff;
				if(buff == NULL)
					LOOP_RECYCLE(node);
				else
				{
					if(buff->packId != node->packId)
						LOOP_RECYCLE(node);
					else
					{
						SendData(node->m_conn, buff);
						node->point = m_nowTick + UDP_OUT_TIME_RESEND;
						PushBack(node);
					}
				}
			}
			node = m_rSendHead.next;
		}
		else
			break;
	}
}

void UdpServerModule::PushBack(UdpConn* conn,UdpBuff** buff)
{
	auto node = GET_LOOP(RsendNode);
	node->next = NULL;
	node->packId = (*buff)->packId;
	node->point = UDP_OUT_TIME_RESEND + m_nowTick;
	node->m_buff = buff;
	node->m_conn = conn;
	(*buff)->m_rsendNode = node;
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
	auto next = m_rSendHead.next->next;
	auto node = m_rSendHead.next;
	node->next = NULL;
	m_rSendHead.next = next;
	if (!next)
		m_rSendTail = NULL;
	return node;
}
