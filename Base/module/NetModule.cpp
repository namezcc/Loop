#include "NetModule.h"
#include "MsgModule.h"
#include "BuffPool.h"
#include "Protocol.h"
#include "ToolFunction.h"

#define UV_ALLOC_BUFF_SIZE 4096

NetModule::NetModule(BaseLayer* l):BaseModule(l), m_port(0)
{
	memset(m_conns, 0, sizeof(m_conns));
	for (int32_t i = 0; i < MAX_CLIENT_CONN; i++)
		m_sock_pool.push_back(i);
};

void NetModule::Init()
{
	m_mgsModule = GetLayer()->GetModule<MsgModule>();

	m_mgsModule->AddMsgCall(L_SOCKET_CLOSE, BIND_CALL(OnCloseSocket, NetMsg));
	m_mgsModule->AddMsgCall(L_SOCKET_SEND_DATA, BIND_CALL(OnSocketSendData,NetMsg));
	m_mgsModule->AddMsgCall(L_SOCKET_BROAD_DATA, BIND_CALL(OnBroadData,BroadMsg));
	m_mgsModule->AddMsgCall(L_TO_CONNET_SERVER, BIND_CALL(OnConnectServer, NetServer));
}

void NetModule::Execute()
{

}

void NetModule::AfterInit()
{
	StartListen();
}

void NetModule::SetProtoType(ProtoType ptype)
{
	m_proto = new Protocol(ptype);
}

void NetModule::SetBind(const int & port, uv_loop_t * loop)
{
	m_uvloop = loop;
	m_port = port;
}

void NetModule::StartListen()
{
	struct sockaddr_in addr;
	ASSERT(0 == uv_ip4_addr("0.0.0.0", m_port, &addr));
	int r;
	r = uv_tcp_init(m_uvloop, &m_hand);
	ASSERT(r == 0);
	r = uv_tcp_bind(&m_hand, (const struct sockaddr*) &addr, 0);
	ASSERT(r == 0);
	m_hand.data = this;
	r = uv_listen((uv_stream_t*)&m_hand, SOMAXCONN, Connection_cb);
	ASSERT(r == 0);
	LP_INFO << "start listen :"<< getLocalIp() << " port:" << m_port;
}

void NetModule::Connection_cb(uv_stream_t * serhand, int status)
{
	if (status != 0)
	{
		LP_ERROR << "Connect error " << uv_err_name(status);
		return;
	}

	auto netmod = (NetModule*)serhand->data;
	uv_tcp_t* client = GET_LOOP(uv_tcp_t);
	int r = uv_tcp_init(netmod->m_uvloop, client);
	if (r != 0)
	{
		LOOP_RECYCLE(client);
		return;
	}
	r = uv_accept(serhand, (uv_stream_t*)client);
	if (r != 0)
	{
		LOOP_RECYCLE(client);
		return;
	}
	netmod->Connected(client);
}

void NetModule::Connected(uv_tcp_t* conn, bool client)
{
	if (m_sock_pool.empty())
	{
		LP_ERROR << "add session no sock index";
		return;
	}

	conn->close_cb = &NetModule::on_close_client;
	auto cn = GET_LOOP(Conn);
	cn->conn = conn;
	cn->netmodule = this;
	cn->socket = m_sock_pool.front();
	m_sock_pool.pop_front();

	if (m_conns[cn->socket] != NULL)
	{
		LP_ERROR << "add session sock index used:" << cn->socket;
	}

	m_conns[cn->socket] = cn;
	conn->data = cn;

	if (client)
	{
		auto sock = GET_LAYER_MSG(NetMsg);
		sock->socket = cn->socket;
		m_mgsModule->SendMsg(L_SOCKET_CONNET, sock);
	}

	int r = uv_read_start((uv_stream_t*)conn, read_alloc, after_read);
	ASSERT(r == 0);
}

void NetModule::read_alloc(uv_handle_t * client, size_t suggested_size, uv_buf_t * buf)
{
	int32_t gsize;
	buf->base = GET_POOL_BUFF(UV_ALLOC_BUFF_SIZE, gsize);
	buf->len = gsize;
}

void NetModule::after_read(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf)
{
	auto conn = (Conn*)client->data;
	auto server = conn->netmodule;
	auto sock = conn->socket;
	if (nread < 0) {
		/* Error or EOF */
		//ASSERT(nread == UV_EOF);
		if(buf->base)
			PUSH_POOL_BUFF(buf->base, buf->len);

		//uv_close ��� socket �ÿ� �����ȱ���
		uv_close((uv_handle_t*)client, client->close_cb);
		return;
	}

	if (nread == 0) {
		/* Everything OK, but nothing read. */
		if (buf->base)
			PUSH_POOL_BUFF(buf->base, buf->len);
		return;
	}
	
	if (!server->ReadPack(conn,buf->base, (int)nread))
	{
		uv_close((uv_handle_t*)client, client->close_cb);
	}
	PUSH_POOL_BUFF(buf->base, buf->len);
}

void NetModule::on_close_client(uv_handle_t* client) {
	auto tcpcli = (uv_tcp_t*)client;
	auto conn = (Conn*)tcpcli->data;
	auto server = conn->netmodule;

	auto sock = server->GetLayer()->GetLayerMsg<NetMsg>();
	sock->socket = conn->socket;
	server->m_mgsModule->SendMsg(L_SOCKET_CLOSE, sock);

	if (CHECK_SOCK_INDEX(conn->socket))
	{
		if (server->m_conns[conn->socket] == conn)
		{
			server->m_sock_pool.push_front(conn->socket);
			server->m_conns[conn->socket] = NULL;
			LOOP_RECYCLE(conn);
		}
	}
}

void NetModule::OnActiveClose(uv_handle_t * client)
{
	auto tcpcli = (uv_tcp_t*)client;
	auto conn = (Conn*)tcpcli->data;
	auto server = conn->netmodule;

	if (CHECK_SOCK_INDEX(conn->socket))
	{
		if (server->m_conns[conn->socket] == conn)
		{
			server->m_sock_pool.push_front(conn->socket);
			server->m_conns[conn->socket] = NULL;
			LOOP_RECYCLE(conn);
		}
	}
}

bool NetModule::ReadPack(Conn* conn, char* buf, int len)
{
	conn->buffer.combin(buf, len);

	if (m_proto->DecodeReadData(conn->buffer, [this,conn](int32_t mid, char* buff, int32_t rlength) {
		auto msg = GetLayer()->GetLayerMsg<NetMsg>();
		msg->mid = mid;
		msg->socket = conn->socket;
		msg->push_front(GetLayer(), buff, rlength);
		m_mgsModule->SendMsg(mid, msg);
	})) {
		return true;
	}
	else
		return false;
}

void NetModule::OnCloseSocket(NetMsg* msg)
{
	if (CHECK_SOCK_INDEX(msg->socket))
	{
		auto conn = m_conns[msg->socket];
		if (conn != NULL)
		{
			uv_close((uv_handle_t*)conn->conn, NetModule::OnActiveClose);
		}
	}
}

void NetModule::OnSocketSendData(NetMsg* nMsg)
{
	if (!CHECK_SOCK_INDEX(nMsg->socket))
		return;
	assert(nMsg != NULL);
	auto conn = m_conns[nMsg->socket];
	if (conn != NULL)
	{
		auto buff = GET_LOOP(LocalBuffBlock);
		m_proto->EncodeSendData(*buff, nMsg);
		uv_write_t* whand = GET_LOOP(uv_write_t);
		Write_t* buf = GET_LOOP(Write_t);
		buf->SetBlock(buff);
		whand->data = buf;
		uv_write(whand, (uv_stream_t*)conn->conn, &buf->buf, 1, After_write);
	}
}

void NetModule::OnBroadData(BroadMsg * nMsg)
{
	if (nMsg->m_socks.size() == 0)
		return;

	auto buff = GET_LOOP(LocalBuffBlock);
	m_proto->EncodeSendData(*buff, nMsg);

	for (size_t i = 0; i < nMsg->m_socks.size(); i++)
	{
		auto sockIndex = nMsg->m_socks[i];
		if (!CHECK_SOCK_INDEX(sockIndex))
			continue;
		auto conn = m_conns[sockIndex];
		if (conn == NULL)
			continue;

		uv_write_t* whand = GET_LOOP(uv_write_t);
		Write_t* buf = GET_LOOP(Write_t);
		buf->SetBlock(buff);
		whand->data = buf;
		uv_write(whand, (uv_stream_t*)conn->conn, &buf->buf, 1, After_write);
	}
	if (buff->m_ref == 0)
		LOOP_RECYCLE(buff);
}

void NetModule::OnConnectServer(NetServer * ser)
{
	struct sockaddr_in addr;
	uv_tcp_t* client = GET_LOOP(uv_tcp_t);
	uv_connect_t* connect_req = GET_LOOP(uv_connect_t);
	connect_req->data = this;
	int r;

	ASSERT(0 == uv_ip4_addr(ser->ip.c_str(), ser->port, &addr));

	r = uv_tcp_init(m_uvloop, client);
	ASSERT(r == 0);

	auto tmpser = GET_LAYER_MSG(NetServer);
	*tmpser = *ser;
	client->data = tmpser;

	r = uv_tcp_connect(connect_req, client, (const struct sockaddr*) &addr, ConnectServerBack);
	ASSERT(r == 0);
}

void NetModule::ConnectServerBack(uv_connect_t * req, int status)
{
	auto md = (NetModule*)req->data;
	auto cli = (uv_tcp_t*)req->handle;
	auto ser = (NetServer*)cli->data;
	if (status == 0)
	{
		md->Connected(cli, false);
		Conn* conn = (Conn*)cli->data;
		ser->socket = conn->socket;
		ser->state = CONN_STATE::CONNECT;
	}
	else {
		ser->socket = -1;
		ser->state = CONN_STATE::CLOSE;
		LOOP_RECYCLE(cli);
	}
	md->m_mgsModule->SendMsg(L_SERVER_CONNECTED, ser);
	LOOP_RECYCLE(req);
}

void NetModule::After_write(uv_write_t* req, int status) {

	Write_t* buf = (Write_t*)req->data;

	LOOP_RECYCLE(buf);
	LOOP_RECYCLE(req);

	if (status == 0)
		return;
	fprintf(stderr,
		"uv_write error: %s - %s\n",
		uv_err_name(status),
		uv_strerror(status));
}