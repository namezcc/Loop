#include "TcpAsioSessionModule.h"
#include "MsgModule.h"
#include "Protocol.h"

#define ASIO_READ_BUFF_SIZE 4096

thread_local int32_t AsioSession::SOCKET = 0;


TcpAsioSessionModule::TcpAsioSessionModule(BaseLayer * l):BaseModule(l)
{
	memset(m_session, 0, sizeof(m_session));
	for (int32_t i = 0; i < MAX_CLIENT_CONN; i++)
		m_sock_pool.push_back(i);

}

TcpAsioSessionModule::~TcpAsioSessionModule()
{
}

void TcpAsioSessionModule::Init()
{
	m_msgModule = GET_MODULE(MsgModule);

	m_msgModule->AddMsgCall(L_SOCKET_CLOSE, BIND_CALL(OnCloseSocket, NetMsg));
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

void TcpAsioSessionModule::OnCloseSocket(NetMsg * msg)
{
	CloseSession(msg->socket,true);
}

void TcpAsioSessionModule::OnSocketSendData(NetMsg * nMsg)
{
	if (!CHECK_SOCK_INDEX(nMsg->socket))
		return;
	auto session = m_session[nMsg->socket];
	if (session == NULL || session->m_close)
		return;

	auto buff = GET_SHARE(LocalBuffBlock);
	m_proto->EncodeSendData(*buff, nMsg);

	boost::system::error_code ec;

	boost::asio::async_write(*session->m_sock, boost::asio::buffer(buff->m_buff, buff->getSize()),
	[this, buff](boost::system::error_code ec, std::size_t length) {
		if (ec)
			LP_ERROR << ec.message();

		if (length != buff->getOffect())
			LP_ERROR << "write data len not over";
	});

	/*session->m_sock->async_write_some(boost::asio::buffer(buff->m_buff, buff->getSize()), 
	[this,buff](boost::system::error_code ec, std::size_t length) {
		if (ec)
			LP_ERROR << ec.message();
	});*/

	//session->m_sock->write_some(boost::asio::buffer(buff->m_buff, buff->getSize()), ec);
}

void TcpAsioSessionModule::OnBroadData(BroadMsg * nMsg)
{
	if (nMsg->m_socks.size() == 0)
		return;

	auto buff = GET_SHARE(LocalBuffBlock);
	m_proto->EncodeSendData(*buff, nMsg);

	for (size_t i = 0; i < nMsg->m_socks.size(); i++)
	{
		auto sockIndex = nMsg->m_socks[i];
		if (!CHECK_SOCK_INDEX(sockIndex))
			continue;
		auto session = m_session[sockIndex];
		if (session == NULL)
			continue;


		boost::asio::async_write(*session->m_sock, boost::asio::buffer(buff->m_buff, buff->getSize()),
			[this, buff](boost::system::error_code ec, std::size_t length) {
			if (ec)
				LP_ERROR << ec.message();

			if (length != buff->getOffect())
				LP_ERROR << "write data len not over";
		});

		/*session->m_sock->async_write_some(boost::asio::buffer(buff->m_buff, buff->getSize()),
			[this, buff](boost::system::error_code ec, std::size_t length) {
			if (ec)
				LP_ERROR << ec.message();
		});*/
		//session->m_sock->write_some(boost::asio::buffer(buff->m_buff, buff->getSize()));
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

int32_t TcpAsioSessionModule::AddNewSession(tcp::socket & sock, bool clien)
{
	if (m_sock_pool.empty())
	{
		LP_ERROR << "add session no sock index";
		return -1;
	}

	auto s = GET_LOOP(AsioSession);
	s->m_sock = std::make_shared<tcp::socket>(std::move(sock));
	s->m_buff.makeRoom(ASIO_READ_BUFF_SIZE);
	s->m_sockId = m_sock_pool.front();
	m_sock_pool.pop_front();

	if (m_session[s->m_sockId] != NULL)
	{
		LP_ERROR << "add session sock index used:" << s->m_sockId;
	}

	m_session[s->m_sockId] = s;
	if (clien)
	{
		auto sock = GET_LAYER_MSG(NetMsg);
		sock->socket = s->m_sockId;
		m_msgModule->SendMsg(L_SOCKET_CONNET, sock);
	}
	DoReadData(s);
	return s->m_sockId;
}

void TcpAsioSessionModule::DoReadData(AsioSession* session)
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
			else
			{
				LP_ERROR << "decode error";
			}
		}
		if (!session->m_close)
			CloseSession(session->m_sockId);
	});
}

void TcpAsioSessionModule::CloseSession(const int32_t & sock, bool active)
{
	if (!CHECK_SOCK_INDEX(sock))
		return;

	auto session = m_session[sock];
	if (session == NULL)
		return;

	if (!session->m_close)
	{
		session->m_close = true;
		if (session->m_sock && session->m_sock->is_open())
			session->m_sock->close();
		session->m_sock.reset();
		if (!active)
		{
			auto sock = GET_LAYER_MSG(NetMsg);
			sock->socket = session->m_sockId;
			m_msgModule->SendMsg(L_SOCKET_CLOSE, sock);
		}
	}
	LOOP_RECYCLE(session);
	m_sock_pool.push_front(sock);
	m_session[sock] = NULL;
}
