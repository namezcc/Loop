#include "TcpAsioSessionModule.h"
#include "MsgModule.h"
#include "Protocol.h"

#define ASIO_READ_BUFF_SIZE 4096

#include <boost/bind.hpp>

TcpAsioSessionModule::TcpAsioSessionModule(BaseLayer * l):BaseModule(l), m_io_pool(8)
, m_send_msg_head(NULL), m_send_msg_tail(NULL), m_close_list(NULL)
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
	LP_INFO << "start listen port:" << m_accptor->local_endpoint().port();
	//m_io_pool.run();
}

void TcpAsioSessionModule::Execute()
{
	m_context.poll();
	sendMsgToLayer();
	extureCloseSock();
}

std::vector<SHARE<AsioSession>> _vec;

void TcpAsioSessionModule::DoAccept()
{
	//std::shared_ptr<tcp::socket> sock(new tcp::socket(m_io_pool.get_io_service()));
	std::shared_ptr<tcp::socket> sock(new tcp::socket(m_context));

	/*auto ssss = GET_SHARE(AsioSession);
	_vec.push_back(ssss);
	m_accptor->async_accept(*sock, boost::bind(&TcpAsioSessionModule::testHandle, this, boost::asio::placeholders::error, ssss));*/

	m_accptor->async_accept(*sock, 
	[this,sock](boost::system::error_code ec) {
		if (!ec)
			AddNewSession(sock);
		else
			LP_ERROR << "do_accept error:" << ec.message();
		DoAccept();
	});

	/*m_accptor->async_accept([this](boost::system::error_code ec, tcp::socket socket) {
		if (!ec)
			AddNewSession(socket);
		else
			LP_ERROR << "do_accept error:" << ec.message();
		DoAccept();
	});*/
}

void TcpAsioSessionModule::testHandle(boost::system::error_code ec,SHARE<AsioSession>& ss)
{
	DoAccept();
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
	if (session == NULL)
		return;

	auto buff = GET_LAYER_MSG(BuffBlock);
	m_proto->EncodeSendData(*buff, nMsg);

	boost::asio::async_write(*session->m_sock, boost::asio::buffer(buff->m_buff, buff->getSize()),
	[this, buff](boost::system::error_code ec, std::size_t length) {
		if (ec)
		{
			LP_ERROR << ec.message();
		}

		if ((int32_t)length != buff->getOffect())
			LP_ERROR << "write data len not over";
		RECYCLE_LAYER_MSG(buff);
	});
}

void TcpAsioSessionModule::OnBroadData(BroadMsg * nMsg)
{
	if (nMsg->m_socks.size() == 0)
		return;

	auto buff = GET_LAYER_MSG(BuffBlock);
	m_proto->EncodeSendData(*buff, nMsg);

	auto shar = SHARE<BuffBlock>(buff, [this](BuffBlock* p) {
		RECYCLE_LAYER_MSG(p);
	});

	for (size_t i = 0; i < nMsg->m_socks.size(); i++)
	{
		auto sockIndex = nMsg->m_socks[i];
		if (!CHECK_SOCK_INDEX(sockIndex))
			continue;
		auto session = m_session[sockIndex];
		if (session == NULL)
			continue;


		boost::asio::async_write(*session->m_sock, boost::asio::buffer(buff->m_buff, buff->getSize()),
			[this, buff, shar](boost::system::error_code ec, std::size_t length) {
			if (ec)
			{
				LP_ERROR << ec.message();
			}

			if ((int32_t)length != buff->getOffect())
				LP_ERROR << "write data len not over";
		});
	}
}

void TcpAsioSessionModule::OnConnectServer(NetServer * ser)
{
	std::shared_ptr<tcp::socket> s(new tcp::socket(m_context));
	auto tmpser = GET_LAYER_MSG(NetServer);
	*tmpser = *ser;
	s->async_connect(tcp::endpoint(boost::asio::ip::address_v4::from_string(ser->ip), ser->port),
		[this, s, tmpser](boost::system::error_code ec)
	{
		if (!ec)
		{
			tmpser->socket = AddNewSession(s, false);
			tmpser->state = CONN_STATE::CONNECT;
		}
		else
		{
			tmpser->socket = -1;
			tmpser->state = CONN_STATE::CLOSE;
		}
		m_msgModule->SendMsg(L_SERVER_CONNECTED, tmpser);
	});
}

int32_t TcpAsioSessionModule::AddNewSession(const std::shared_ptr<tcp::socket> & sock, bool clien)
{
	if (m_sock_pool.empty())
	{
		LP_ERROR << "add session no sock index";
		return -1;
	}

	auto s = GET_LOOP(AsioSession);
	s->m_sock = sock;
	s->m_sock->set_option(tcp::no_delay(true));
	s->m_buff.makeRoom(ASIO_READ_BUFF_SIZE);
	s->m_sockId = m_sock_pool.front();
	m_sock_pool.pop_front();

	if (m_session[s->m_sockId] != NULL)
	{
		LP_ERROR << "add session sock index used:" << s->m_sockId;
		assert(0);
	}

	m_session[s->m_sockId] = s;
	if (clien)
	{
		auto sock = GET_LAYER_MSG(NetMsg);
		sock->socket = s->m_sockId;
		m_msgModule->SendMsg(L_SOCKET_CONNET, sock);
	}

	auto shar = SHARE<AsioSession>(s, [this](AsioSession* p) {
		pushCloseSock(p->m_sockId);
	});

	DoReadData(shar);
	return s->m_sockId;
}

void TcpAsioSessionModule::DoReadData(SHARE<AsioSession> session)
{
	session->m_sock->async_read_some(as::buffer(session->m_buff.m_buff, ASIO_READ_BUFF_SIZE),
	[this, session](boost::system::error_code ec, std::size_t length) {
		if (!ec)
		{
			session->m_decodeBuff.append(session->m_buff.m_buff, (uint32_t)length);
			if (m_proto->DecodeReadData(session->m_decodeBuff, [this, &session](int32_t mid, char* buff, int32_t rlength) {
				auto msg = GET_LAYER_MSG(NetMsg);
				msg->mid = mid;
				msg->socket = session->m_sockId;
				msg->push_front(GetLayer(), buff, rlength);
				pushMsg(msg);
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
	});

	/*session->m_sock->async_read_some(as::buffer(session->m_buff.m_buff, ASIO_READ_BUFF_SIZE),
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
	});*/
}

void TcpAsioSessionModule::CloseSession(const int32_t & sock, bool active)
{
	if (!CHECK_SOCK_INDEX(sock))
		return;

	auto session = m_session[sock];
	if (session == NULL)
		return;
	
	if (session->m_sock)
	{
		auto assock = session->m_sock;
		session->m_sock.reset();
		assock->get_io_context().post([assock]() {
			assock->close();
		});
	}
		
	if (!active)
	{
		auto sock = GET_LAYER_MSG(NetMsg);
		sock->socket = session->m_sockId;
		m_msgModule->SendMsg(L_SOCKET_CLOSE, sock);
	}
	else if(session->m_active == false)
	{
		session->m_active = true;
		return;
	}

	m_sock_pool.push_front(sock);
	m_session[sock] = NULL;
	LOOP_RECYCLE(session);
}

void TcpAsioSessionModule::pushMsg(NetMsg * msg)
{
	msg->m_next_data = NULL;

	std::lock_guard<std::mutex> _g(m_msg_mutex);

	if (m_send_msg_tail)
	{
		m_send_msg_tail->m_next_data = msg;
		m_send_msg_tail = msg;
	}
	else
	{
		m_send_msg_head = msg;
		m_send_msg_tail = msg;
	}
}

void TcpAsioSessionModule::sendMsgToLayer()
{
	NetMsg* msg = NULL;
	{
		std::lock_guard<std::mutex> _g(m_msg_mutex);
		if (m_send_msg_head)
		{
			msg = m_send_msg_head;
			m_send_msg_head = NULL;
			m_send_msg_tail = NULL;
		}
	}

	while (msg)
	{
		auto sm = msg;
		msg = (NetMsg*)sm->m_next_data;
		sm->m_next_data = NULL;
		m_msgModule->SendMsg(sm->mid, sm);
	}
}

void TcpAsioSessionModule::pushCloseSock(int32_t sock)
{
	auto msg = GET_LAYER_MSG(NetMsg);
	msg->socket = sock;

	std::lock_guard<std::mutex> _g(m_close_mutex);
	msg->m_next_data = m_close_list;
	m_close_list = msg;
}

void TcpAsioSessionModule::extureCloseSock()
{
	NetMsg* sock = NULL;

	{
		std::lock_guard<std::mutex> _g(m_close_mutex);
		sock = m_close_list;
		m_close_list = NULL;
	}

	while (sock)
	{
		auto _s = sock;
		sock = sock->m_next_data;
		CloseSession(_s->socket);
		RECYCLE_LAYER_MSG(_s);
	}
}
