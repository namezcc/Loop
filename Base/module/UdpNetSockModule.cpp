#include "UdpNetSockModule.h"
#include "MsgModule.h"
#include "EventModule.h"

UdpNetSockModule::UdpNetSockModule(BaseLayer * l):BaseModule(l)
{
}

UdpNetSockModule::~UdpNetSockModule()
{
}

void UdpNetSockModule::Init()
{
	m_msgModule = GET_MODULE(MsgModule);
	m_eventModule = GET_MODULE(EventModule);

	m_msgModule->AddMsgCall(L_UDP_SOCKET_CONNECT, BIND_CALL(OnSocketConnet, NetMsg));
	m_msgModule->AddMsgCall(L_UDP_SOCKET_CLOSE, BIND_CALL(OnSocketClose, NetMsg));
}

void UdpNetSockModule::OnSocketConnet(NetMsg * sock)
{
	m_eventModule->SendEvent(E_UDP_SOCKET_CONNECT, sock->socket);
}

void UdpNetSockModule::OnSocketClose(NetMsg * sock)
{
	m_eventModule->SendEvent(E_UDP_SOCKET_CLOSE, sock->socket);
}

void UdpNetSockModule::SendNetMsg(const int & socket, const int & mid, const google::protobuf::Message & pbmsg)
{
	NetMsg* nMsg = GET_LAYER_MSG(NetMsg);
	nMsg->socket = socket;
	nMsg->mid = mid;
	auto buff = GET_LAYER_MSG(BuffBlock);
	buff->makeRoom(pbmsg.ByteSize());
	buff->write(pbmsg);
	nMsg->push_front(buff);
	m_msgModule->SendMsg(LY_UDP_NET,0,L_SOCKET_SEND_DATA, nMsg);
}

void UdpNetSockModule::SendNetMsg(const int & socket, const int32_t & mid, BuffBlock * buff)
{
	NetMsg* nMsg = GET_LAYER_MSG(NetMsg);
	nMsg->socket = socket;
	nMsg->mid = mid;
	nMsg->push_front(buff);
	m_msgModule->SendMsg(LY_UDP_NET, 0,L_SOCKET_SEND_DATA, nMsg);
}


void UdpNetSockModule::BroadNetMsg(std::vector<int32_t>& socks, const int32_t & mid, const gpb::Message & pbmsg)
{
	auto nMsg = GET_LAYER_MSG(BroadMsg);

	nMsg->m_socks = std::move(socks);
	nMsg->mid = mid;
	auto buff = GET_LAYER_MSG(BuffBlock);
	buff->makeRoom(pbmsg.ByteSize());
	buff->write(pbmsg);
	nMsg->push_front(buff);
	m_msgModule->SendMsg(LY_UDP_NET, 0, L_SOCKET_BROAD_DATA, nMsg);
}

void UdpNetSockModule::BroadNetMsg(std::vector<int32_t>& socks, const int32_t & mid, BuffBlock * buff)
{
	auto nMsg = GET_LAYER_MSG(BroadMsg);
	nMsg->m_socks = std::move(socks);
	nMsg->mid = mid;
	nMsg->push_front(buff);
	m_msgModule->SendMsg(LY_UDP_NET, 0, L_SOCKET_BROAD_DATA, nMsg);
}

void UdpNetSockModule::CloseNetObject(const int & socket)
{
	auto sock = GET_LAYER_MSG(NetMsg);
	sock->socket = socket;
	m_msgModule->SendMsg(LY_UDP_NET, 0, L_SOCKET_CLOSE, sock);
}
