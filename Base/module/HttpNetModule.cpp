#include "HttpNetModule.h"
#include "MsgModule.h"

void HttpNetModule::Init()
{
	m_mgsModule = GetLayer()->GetModule<MsgModule>();

	m_mgsModule->AddMsgCallBack<NetSocket>(L_SOCKET_CLOSE, this, &HttpNetModule::OnCloseSocket);
	m_mgsModule->AddMsgCallBack<NetMsg>(L_SOCKET_SEND_HTTP_DATA, this, &HttpNetModule::OnSendHttpMsg);

	m_mgsModule->AddMsgCallBack<NetServer>(L_CONNECT_PHP_CGI, this, &HttpNetModule::OnConnectPHPCgi);
}

void HttpNetModule::Execute()
{
}

bool HttpNetModule::ReadPack(int socket, char * buf, int len)
{
	auto it = m_conns.find(socket);
	if (it == m_conns.end())
		return false;

	auto msg = new NetMsg();
	msg->len = len;
	msg->msg = new char[msg->len];
	msg->mid = N_RECV_HTTP_MSG;
	msg->socket = socket;
	memcpy(msg->msg, buf, msg->len);
	m_mgsModule->SendMsg(N_RECV_HTTP_MSG, msg);
	return true;
}

bool HttpNetModule::ReadPackMid(int socket, char* buf, int len, int mid)
{
	auto it = m_conns.find(socket);
	if (it == m_conns.end())
		return false;

	auto msg = new NetMsg();
	msg->len = len;
	msg->msg = new char[msg->len];
	msg->mid = mid;
	msg->socket = socket;
	memcpy(msg->msg, buf, msg->len);
	m_mgsModule->SendMsg(mid, msg);
	return true;
}

void HttpNetModule::OnSendHttpMsg(NetMsg * msg)
{
	auto it = m_conns.find(msg->socket);
	if (it != m_conns.end())
	{
		char* enbuf = msg->msg;
		msg->msg = nullptr;

		uv_write_t* whand = GetLayer()->GetLoopObj<uv_write_t>();
		Write_t* buf = GetLayer()->GetLoopObj<Write_t>();
		buf->baseModule = this;
		buf->buf.base = enbuf;
		buf->buf.len = msg->len;
		whand->data = buf;
		uv_write(whand, (uv_stream_t*)it->second->conn, &buf->buf, 1, After_write);
	}
}

void HttpNetModule::OnConnectPHPCgi(NetServer * ser)
{
	struct sockaddr_in addr;
	uv_tcp_t* client = GetLayer()->GetLoopObj<uv_tcp_t>();
	uv_connect_t* connect_req = GetLayer()->GetLoopObj<uv_connect_t>();
	connect_req->data = this;
	int r;

	ASSERT(0 == uv_ip4_addr(ser->ip.c_str(), ser->port, &addr));
	ASSERT(client != NULL);
	ASSERT(connect_req != NULL);

	r = uv_tcp_init(m_uvloop, client);
	ASSERT(r == 0);

	client->data = new NetServer(*ser);

	r = uv_tcp_connect(connect_req, client, (const struct sockaddr*) &addr, Connect_cb);
	ASSERT(r == 0);
}

void HttpNetModule::Connect_cb(uv_connect_t* req, int status)
{
	int r;
	ASSERT(req != NULL);

	auto md = (HttpNetModule*)req->data;
	auto cli = (uv_tcp_t*)req->handle;
	auto ser = (NetServer*)cli->data;
	if (status == 0)
	{
		ser->socket = cli->socket;
		md->m_mgsModule->SendMsg(L_PHP_CGI_CONNECTED, ser);

		cli->data = md;
		cli->read_cb = HttpNetModule::After_read_CGI;
		md->Connected(cli, false);
	}
	else {
		cout << "PHP CGI not run on host:" << ser->ip << " port:" << ser->port << endl;
		delete ser;
		md->GetLayer()->Recycle(cli);
	}
	md->GetLayer()->Recycle(req);
}

void HttpNetModule::After_read_CGI(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf)
{
	auto server = (HttpNetModule*)client->data;
	auto sc = (uv_tcp_t*)client;
	auto sock = sc->socket;
	if (nread < 0) {
		/* Error or EOF */
		//ASSERT(nread == UV_EOF);
		delete[] buf->base;
		//uv_close 会把 socket 置空 所以先保存
		uv_close((uv_handle_t*)client, client->close_cb);
		//再置回
		sc->socket = sock;
		return;
	}

	if (nread == 0) {
		/* Everything OK, but nothing read. */
		delete[] buf->base;
		return;
	}

	if (!server->ReadPackMid(sc->socket, buf->base, nread, N_RECV_PHP_CGI_MSG))
	{
		uv_close((uv_handle_t*)client, client->close_cb);
		sc->socket = sock;
	}
	delete[] buf->base;
}