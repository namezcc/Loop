#include "NetModule.h"
#include "TcpServerModule.h"
#include "MsgModule.h"

thread_local int32_t Conn::SOCKET = 0;

void NetModule::Init()
{
	m_mgsModule = GetLayer()->GetModule<MsgModule>();

	m_mgsModule->AddMsgCallBack(L_SOCKET_CLOSE, this, &NetModule::OnCloseSocket);
	m_mgsModule->AddMsgCallBack(L_SOCKET_SEND_DATA, this, &NetModule::OnSocketSendData);
}

void NetModule::Execute()
{

}

void NetModule::Connected(uv_tcp_t* conn, bool client)
{
	conn->close_cb = &NetModule::on_close_client;
	auto ser = (NetModule*)conn->data;

	auto cn = ser->GetLayer()->GetSharedLoop<Conn>();
	cn->conn = conn;
	cn->netmodule = ser;
	auto uvsocket = cn->socket;
	m_conns[uvsocket] = cn;
	conn->data = cn.get();

	if (client)
	{
		auto sock = ser->GetLayer()->GetLayerMsg<NetSocket>();
		sock->socket = uvsocket;
		m_mgsModule->SendMsg(L_SOCKET_CONNET, sock);
	}

	int r = uv_read_start((uv_stream_t*)conn, read_alloc, conn->read_cb);
	ASSERT(r == 0);
}

void NetModule::after_read(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf)
{
	auto conn = (Conn*)client->data;
	auto server = conn->netmodule;
	auto sock = conn->socket;
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
	
	if (!server->ReadPack(conn,buf->base, nread))
	{
		uv_close((uv_handle_t*)client, client->close_cb);
	}
	delete[] buf->base;
}

void NetModule::on_close_client(uv_handle_t* client) {
	auto tcpcli = (uv_tcp_t*)client;
	auto conn = (Conn*)tcpcli->data;
	auto server = conn->netmodule;

	auto sock = server->GetLayer()->GetLayerMsg<NetSocket>();
	sock->socket = conn->socket;
	server->m_mgsModule->SendMsg(L_SOCKET_CLOSE, sock);
	server->RemoveConn(conn->socket);
}

void NetModule::OnActiveClose(uv_handle_t * client)
{
	auto tcpcli = (uv_tcp_t*)client;
	auto conn = (Conn*)tcpcli->data;
	auto server = conn->netmodule;
	server->m_waitClose.erase(conn->socket);
}

void NetModule::RemoveConn(const int& socket)
{
	m_conns.erase(socket);
}

bool NetModule::ReadPack(Conn* conn, char* buf, int len)
{
	auto socket = conn->socket;

	NetBuffer& oldbuf = conn->buffer;
	oldbuf.combin(buf, len);

	bool res = true;
	int read = 0;
	while (oldbuf.use - read >= MsgHead::HEAD_SIZE)
	{
		MsgHead head;
		if (!MsgHead::Decode(head, oldbuf.buf + read))
		{
			res = false;
			break;
		}

		if (head.size <= oldbuf.use - read)
		{//cpm pack
			auto msg = GetLayer()->GetLayerMsg<NetMsg>();
			auto len = head.size - MsgHead::HEAD_SIZE;
			msg->mid = head.mid;
			msg->socket = socket;
			msg->push_front(GetLayer(),oldbuf.buf + read + MsgHead::HEAD_SIZE,len);
			m_mgsModule->SendMsg(head.mid,msg);

			read += head.size;
		}
		else
		{//half
			break;
		}
	}
	oldbuf.moveHalf(read);
	return res;
}

void NetModule::OnCloseSocket(NetSocket* msg)
{
	auto it = m_conns.find(msg->socket);
	if (it != m_conns.end())
	{
		uv_close((uv_handle_t*)it->second->conn, NetModule::OnActiveClose);
		m_waitClose[it->second->socket] = it->second;
		m_conns.erase(it);
	}
}

void NetModule::OnSocketSendData(NetMsg* nMsg)
{
	auto it = m_conns.find(nMsg->socket);
	if (it != m_conns.end())
	{
		char encode[MsgHead::HEAD_SIZE];
		MsgHead::Encode(encode, nMsg->mid, nMsg->getLen());
		nMsg->push_front(GetLayer(),encode,MsgHead::HEAD_SIZE);

		auto buff = nMsg->getCombinBuff(GetLayer());

		uv_write_t* whand = GetLayer()->GetLoopObj<uv_write_t>();
		Write_t* buf = GetLayer()->GetLoopObj<Write_t>();
		buf->baseModule = this;
		buf->buf.base = buff->m_buff;
		buff->m_buff = NULL;

		buf->buf.len = buff->m_size;
		whand->data = buf;

		uv_write(whand, (uv_stream_t*)it->second->conn, &buf->buf, 1, After_write);
	}
}

void NetModule::After_write(uv_write_t* req, int status) {

	Write_t* buf = (Write_t*)req->data;

	buf->baseModule->GetLayer()->Recycle(buf);
	buf->baseModule->GetLayer()->Recycle(req);

	if (status == 0)
		return;
	fprintf(stderr,
		"uv_write error: %s - %s\n",
		uv_err_name(status),
		uv_strerror(status));
}