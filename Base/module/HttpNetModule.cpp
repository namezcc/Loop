#include "HttpNetModule.h"
#include "MsgModule.h"

void HttpNetModule::Init()
{
	m_mgsModule = GetLayer()->GetModule<MsgModule>();

	m_mgsModule->AddMsgCallBack(L_SOCKET_CLOSE, this, &HttpNetModule::OnCloseSocket);
	m_mgsModule->AddMsgCallBack(L_SOCKET_SEND_HTTP_DATA, this, &HttpNetModule::OnSendHttpMsg);

	m_mgsModule->AddMsgCallBack(L_CONNECT_PHP_CGI, this, &HttpNetModule::OnConnectPHPCgi);
}

void HttpNetModule::Execute()
{
}

bool HttpNetModule::ReadPack(Conn* conn, char * buf, int len)
{
	auto msg = GetLayer()->GetLayerMsg<NetMsg>();
	msg->mid = N_RECV_HTTP_MSG;
	msg->socket = conn->socket;
	msg->push_front(GetLayer(),buf,len);
	m_mgsModule->SendMsg(N_RECV_HTTP_MSG, msg);
	return true;
}

bool HttpNetModule::ReadPackMid(Conn* conn, char* buf, int len, int mid)
{
	auto socket = conn->socket;
	auto msg = GetLayer()->GetLayerMsg<NetMsg>();
	msg->mid = mid;
	msg->socket = socket;
	msg->push_front(GetLayer(),buf,len);
	m_mgsModule->SendMsg(mid, msg);
	return true;
}

void HttpNetModule::OnSendHttpMsg(NetMsg * msg)
{
	auto it = m_conns.find(msg->socket);
	if (it != m_conns.end())
	{
		auto buff = msg->getCombinBuff(GetLayer());

		uv_write_t* whand = GetLayer()->GetLoopObj<uv_write_t>();
		Write_t* buf = GetLayer()->GetLoopObj<Write_t>();
		buf->baseModule = this;
		buf->buf.base = buff->m_buff;
		buff->m_buff = NULL;	//转移buff 置为 NULL
		buf->buf.len = buff->m_size;
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

	auto tmpser = GetLayer()->GetLayerMsg<NetServer>();
	*tmpser = *ser;
	client->data = tmpser;

	r = uv_tcp_connect(connect_req, client, (const struct sockaddr*) &addr, Connect_cb);
	ASSERT(r == 0);
}

void HttpNetModule::Connect_cb(uv_connect_t* req, int status)
{
	ASSERT(req != NULL);

	auto md = (HttpNetModule*)req->data;
	auto cli = (uv_tcp_t*)req->handle;
	auto ser = (NetServer*)cli->data;
	if (status == 0)
	{
		cli->data = md;
		cli->read_cb = HttpNetModule::After_read_CGI;
		md->Connected(cli, false);
		Conn* conn = (Conn*)cli->data;
		ser->socket = conn->socket;
		md->m_mgsModule->SendMsg(L_PHP_CGI_CONNECTED, ser);
	}
	else {
		//cout << "PHP CGI not run on host:" << ser->ip << " port:" << ser->port << endl;
		LP_ERROR(md->m_mgsModule)<<"PHP CGI not run on host:"<<ser->ip<<" port:"<<ser->port;
		//delete ser;
		md->GetLayer()->RecycleLayerMsg(ser);
		md->GetLayer()->Recycle(cli);
	}
	md->GetLayer()->Recycle(req);
}

void HttpNetModule::After_read_CGI(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf)
{
	auto sc = (Conn*)client->data;
	auto server = (HttpNetModule*)sc->netmodule;
	if (nread < 0) {
		/* Error or EOF */
		//ASSERT(nread == UV_EOF);
		delete[] buf->base;
		//uv_close ��� socket �ÿ� �����ȱ���
		uv_close((uv_handle_t*)client, client->close_cb);
		return;
	}

	if (nread == 0) {
		/* Everything OK, but nothing read. */
		delete[] buf->base;
		return;
	}

	if (!server->ReadPackMid(sc, buf->base, nread, N_RECV_PHP_CGI_MSG))
	{
		uv_close((uv_handle_t*)client, client->close_cb);
	}
	delete[] buf->base;
}