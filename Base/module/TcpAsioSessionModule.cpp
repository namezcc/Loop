#include "TcpAsioSessionModule.h"
#include "MsgModule.h"
#include "Protocol.h"

#define ASIO_READ_BUFF_SIZE 4096

thread_local int32_t AsioSession::SOCKET = 0;


TcpAsioSessionModule::TcpAsioSessionModule(BaseLayer * l):BaseModule(l)
{
}

TcpAsioSessionModule::~TcpAsioSessionModule()
{
}

void TcpAsioSessionModule::Init()
{
	m_sendBuff.makeRoom(ASIO_READ_BUFF_SIZE);
	m_msgModule = GET_MODULE(MsgModule);

	m_msgModule->AddMsgCall(L_SOCKET_CLOSE, BIND_CALL(OnCloseSocket, NetSocket));
	m_msgModule->AddMsgCall(L_SOCKET_SEND_DATA, BIND_CALL(OnSocketSendData, NetMsg));
	m_msgModule->AddMsgCall(L_SOCKET_BROAD_DATA, BIND_CALL(OnBroadData, BroadMsg));
	m_msgModule->AddMsgCall(L_TO_CONNET_SERVER, BIND_CALL(OnConnectServer, NetServer));
}

void TcpAsioSessionModule::AfterInit()
{
	DoAccept();
	LP_WARN << "start listen port:" << m_accptor->local_endpoint().port();
}

void TcpAsioSessionModule::Execute()
{
	m_context.poll();
}

void TcpAsioSessionModule::DoAccept()
{
	m_accptor->async_accept([this](boost::system::error_code ec, tcp::socket socket) {
		if (!ec)
			AddNewSession(socket);
		else
			LP_ERROR << "do_accept error:" << ec.message();
		DoAccept();
	});
}

void TcpAsioSessionModule::SetProtoType(ProtoType ptype)
{
	m_proto = new Protocol(ptype);
}

void TcpAsioSessionModule::OnCloseSocket(NetSocket * msg)
{
	CloseSession(msg->socket,true);
}

void TcpAsioSessionModule::OnSocketSendData(NetMsg * nMsg)
{
	auto it = m_session.find(nMsg->socket);
	if (it == m_session.end() || it->second->m_close)
		return;
	CombinBuff(nMsg);
	boost::system::error_code ec;
	it->second->m_sock->write_some(boost::asio::buffer(m_sendBuff.m_buff, m_sendBuff.m_size), ec);
	if (ec)
		LP_ERROR << ec.message();
}

void TcpAsioSessionModule::OnBroadData(BroadMsg * nMsg)
{
	if (nMsg->m_socks.size() == 0)
		return;

	CombinBuff(nMsg);

	for (size_t i = 0; i < nMsg->m_socks.size(); i++)
	{
		auto it = m_session.find(nMsg->m_socks[i]);
		if (it == m_session.end())
			continue;
		it->second->m_sock->write_some(boost::asio::buffer(m_sendBuff.m_buff, m_sendBuff.m_size));
	}
}

void TcpAsioSessionModule::OnConnectServer(NetServer * ser)
{
	auto s = new tcp::socket(m_context);
	auto tmpser = GET_LAYER_MSG(NetServer);
	*tmpser = *ser;
	s->async_connect(tcp::endpoint(boost::asio::ip::address_v4::from_string(ser->ip), ser->port),
		[this, s, tmpser](boost::system::error_code ec)
	{
		if (!ec)
		{
			tmpser->socket = AddNewSession(*s, false);
			tmpser->state = CONN_STATE::CONNECT;
		}
		else
		{
			tmpser->socket = -1;
			tmpser->state = CONN_STATE::CLOSE;
		}
		m_msgModule->SendMsg(L_SERVER_CONNECTED, tmpser);
		delete s;
	});
}

void TcpAsioSessionModule::CombinBuff(NetMsg * nMsg)
{
	m_proto->EncodeSendData(m_sendBuff, nMsg);
}

int32_t TcpAsioSessionModule::AddNewSession(tcp::socket & sock, bool clien)
{
	auto s = GET_SHARE(AsioSession);
	s->m_sock = std::make_shared<tcp::socket>(std::move(sock));
	s->m_buff.makeRoom(ASIO_READ_BUFF_SIZE);
	m_session[s->m_sockId] = s;
	if (clien)
	{
		auto sock = GET_LAYER_MSG(NetSocket);
		sock->socket = s->m_sockId;
		m_msgModule->SendMsg(L_SOCKET_CONNET, sock);
	}
	DoReadData(s);
	return s->m_sockId;
}

void TcpAsioSessionModule::DoReadData(SHARE<AsioSession> session)
{
	session->m_sock->async_read_some(as::buffer(session->m_buff.m_buff, ASIO_READ_BUFF_SIZE),
	[this, session](boost::system::error_code ec, std::size_t length) {
		if (!ec)
		{
			session->m_decodeBuff.append(session->m_buff.m_buff, (uint32_t)length);
			if (m_proto->DecodeReadData(session->m_decodeBuff, [this,&session](int32_t mid,char* buff,int32_t rlength) {
				auto msg = GetLayer()->GetLayerMsg<NetMsg>();
				msg->mid = mid;
				msg->socket = session->m_sockId;
				msg->push_front(GetLayer(), buff, rlength);
				m_msgModule->SendMsg(msg->mid, msg);
			}))
			{
				DoReadData(session);
				return;
			}
		}
		if (!session->m_close)
			CloseSession(session->m_sockId);
	});
}

void TcpAsioSessionModule::CloseSession(const int32_t & sock, bool active)
{
	auto it = m_session.find(sock);
	if (it == m_session.end())
		return;

	if (!it->second->m_close)
	{
		it->second->m_close = true;
		if (it->second->m_sock && it->second->m_sock->is_open())
			it->second->m_sock->close();
		it->second->m_sock.reset();
		if (!active)
		{
			auto sock = GET_LAYER_MSG(NetSocket);
			sock->socket = it->second->m_sockId;
			m_msgModule->SendMsg(L_SOCKET_CLOSE, sock);
		}
	}
	m_session.erase(it);
}
