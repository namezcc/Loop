#include "UdpNetModule.h"
#include "UdpServerModule.h"
#include "MsgModule.h"
#include "NetModule.h"
#include "ScheduleModule.h"

void UdpCashNode::init(FactorManager * fm)
{
	m_buff = NULL;
	m_udpbuff = NULL;
}

void UdpCashNode::recycle(FactorManager * fm)
{
	m_buff = NULL;
	m_udpbuff = NULL;
}

UdpNetModule::UdpNetModule(BaseLayer * l):BaseModule(l)
{
}

UdpNetModule::~UdpNetModule()
{
}

void UdpNetModule::Init()
{
	m_udpServer = GET_MODULE(UdpServerModule);
	m_msgModule = GET_MODULE(MsgModule);
	m_schedule = GET_MODULE(ScheduleModule);
	
	m_msgModule->AddMsgCallBack2(L_SOCKET_CLOSE, this, &UdpNetModule::OnCloseSocket);
	m_msgModule->AddMsgCallBack2(L_SOCKET_SEND_DATA, this, &UdpNetModule::OnSocketSendData);
	m_msgModule->AddMsgCallBack2(L_SOCKET_BROAD_DATA, this, &UdpNetModule::OnBroadData);

	m_schedule->AddInterValTask(this, &UdpNetModule::TickSendBuff, 3);

	InitHandle();
}

void UdpNetModule::InitHandle()
{
	m_udpServer->BindOnConnect([this](int32_t sock) {
		auto msg = GET_LAYER_MSG(NetSocket);
		msg->socket = sock;
		m_msgModule->SendMsg(L_UDP_SOCKET_CONNECT, msg);
	});

	m_udpServer->BindOnClose([this](int32_t sock) {
		auto msg = GET_LAYER_MSG(NetSocket);
		msg->socket = sock;
		m_msgModule->SendMsg(L_UDP_SOCKET_CLOSE, msg);
		m_cashSendBuff.erase(sock);
	});

	m_udpServer->BindOnReadPack([this](const int32_t& sock,const char* buf,const int32_t& size) {
		if (size < MsgHead::HEAD_SIZE)
			return;
		MsgHead head;
		if (!MsgHead::Decode(head, const_cast<char*>(buf)))
			return;

		auto msg = GET_LAYER_MSG(NetMsg);
		msg->mid = head.mid;
		msg->socket = sock;
		msg->push_front(GetLayer(), buf + MsgHead::HEAD_SIZE, size - MsgHead::HEAD_SIZE);
		m_msgModule->SendMsg(head.mid, msg);
	});
}

void UdpNetModule::OnCloseSocket(NetSocket * msg)
{
	m_udpServer->CloseSocket(msg->socket,false);
	m_cashSendBuff.erase(msg->socket);
}

void UdpNetModule::OnSocketSendData(NetMsg * nMsg)
{
	char encode[MsgHead::HEAD_SIZE];
	MsgHead::Encode(encode, nMsg->mid, nMsg->getLen());
	nMsg->push_front(GetLayer(), encode, MsgHead::HEAD_SIZE);
	auto buff = nMsg->getCombinBuff(GetLayer());
	bool canCombin = buff->m_size < UDP_DATA_SIZE - MsgHead::HEAD_SIZE;

	auto& ls = m_cashSendBuff[nMsg->socket];

	if (canCombin && ls.size() > 0)
	{
		auto& back = ls.back();
		if (back->m_udpbuff && back->m_udpbuff->size+buff->m_size <= UDP_DATA_SIZE)
		{
			back->m_udpbuff->write(buff->m_buff, buff->m_size);
			return;
		}
	}
	
	auto cash = GET_SHARE(UdpCashNode);
	if (!canCombin)
		cash->m_buff = buff;
	else
	{
		cash->m_udpbuff = GET_SHARE(UdpBuff);
		cash->m_udpbuff->write(buff->m_buff, buff->m_size);
	}
	ls.push_back(cash);
}

void UdpNetModule::OnBroadData(BroadMsg * nMsg)
{
	if (nMsg->m_socks.size() == 0)
		return;

	char encode[MsgHead::HEAD_SIZE];
	MsgHead::Encode(encode, nMsg->mid, nMsg->getLen());
	nMsg->push_front(GetLayer(), encode, MsgHead::HEAD_SIZE);
	auto buff = nMsg->getCombinBuff(GetLayer());
	bool canCombin = buff->m_size < UDP_DATA_SIZE - MsgHead::HEAD_SIZE;

	for (auto& sock:nMsg->m_socks)
	{
		auto& ls = m_cashSendBuff[sock];

		if (canCombin && ls.size() > 0)
		{
			auto& back = ls.back();
			if (back->m_udpbuff && back->m_udpbuff->size + buff->m_size <= UDP_DATA_SIZE)
			{
				back->m_udpbuff->write(buff->m_buff, buff->m_size);
				continue;
			}
		}

		auto cash = GET_SHARE(UdpCashNode);
		if (!canCombin)
			cash->m_buff = buff;
		else
		{
			cash->m_udpbuff = GET_SHARE(UdpBuff);
			cash->m_udpbuff->write(buff->m_buff, buff->m_size);
		}
		ls.push_back(cash);
	}
}

void UdpNetModule::TickSendBuff(int64_t & dt)
{
	for (auto& ls:m_cashSendBuff)
	{
		for (auto& buf:ls.second)
		{
			if(buf->m_buff)
				m_udpServer->SendData(ls.first, buf->m_buff->m_buff, buf->m_buff->m_size);
			else if(buf->m_udpbuff)
				m_udpServer->SendData(ls.first, buf->m_udpbuff->buf, buf->m_udpbuff->size);
		}
		ls.second.clear();
	}
	//m_cashSendBuff.clear();
}
