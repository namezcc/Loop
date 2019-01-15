#include "NetModule.h"
#include "TcpServerModule.h"
#include "MsgModule.h"
#include "BuffPool.h"

thread_local int32_t Conn::SOCKET = 0;

void NetModule::Init()
{
	m_mgsModule = GetLayer()->GetModule<MsgModule>();

	m_mgsModule->AddMsgCall(L_SOCKET_CLOSE, BIND_CALL(OnCloseSocket,NetSocket));
	m_mgsModule->AddMsgCall(L_SOCKET_SEND_DATA, BIND_CALL(OnSocketSendData,NetMsg));
	m_mgsModule->AddMsgCall(L_SOCKET_BROAD_DATA, BIND_CALL(OnBroadData,BroadMsg));
	
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

void NetModule::read_alloc(uv_handle_t * client, size_t suggested_size, uv_buf_t * buf)
{
	int32_t gsize;
	buf->base = GET_LOCAL_BUFF(4096, gsize);
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
			RCY_LOCAL_BUFF(buf->base, buf->len);

		//uv_close ��� socket �ÿ� �����ȱ���
		uv_close((uv_handle_t*)client, client->close_cb);
		return;
	}

	if (nread == 0) {
		/* Everything OK, but nothing read. */
		if (buf->base)
			RCY_LOCAL_BUFF(buf->base, buf->len);
		return;
	}
	
	if (!server->ReadPack(conn,buf->base, (int)nread))
	{
		uv_close((uv_handle_t*)client, client->close_cb);
	}
	RCY_LOCAL_BUFF(buf->base, buf->len);
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

		if (head.size <= (int)oldbuf.use - read)
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
		auto head = GET_LOOP(LocalBuffBlock);
		head->write(encode, MsgHead::HEAD_SIZE);

		uv_write_t* whand = GetLayer()->GetLoopObj<uv_write_t>();
		Write_t* buf = GetLayer()->GetLoopObj<Write_t>();
		buf->baseModule = this;
		buf->SetBlock(head);
		whand->data = buf;
		uv_write(whand, (uv_stream_t*)it->second->conn, &buf->buf, 1, After_write);

		BuffBlock* buff = NULL;
		while (buff = nMsg->popBuffBlock())
		{
			if (buff->m_size == 0)
			{
				RECYCLE_LAYER_MSG(buff);
				continue;
			}
			whand = GetLayer()->GetLoopObj<uv_write_t>();
			buf = GetLayer()->GetLoopObj<Write_t>();
			buf->baseModule = this;
			buf->SetBlock(buff);
			whand->data = buf;
			uv_write(whand, (uv_stream_t*)it->second->conn, &buf->buf, 1, After_write);
		}
	}
}

void NetModule::OnBroadData(BroadMsg * nMsg)
{
	if (nMsg->m_socks.size() == 0)
		return;

	char encode[MsgHead::HEAD_SIZE];
	MsgHead::Encode(encode, nMsg->mid, nMsg->getLen());
	auto head = GET_LOOP(LocalBuffBlock);
	head->write(encode, MsgHead::HEAD_SIZE);
	m_broadBuff.push_back(head);
	BuffBlock* buff = NULL;
	while (buff = nMsg->popBuffBlock())
	{
		if (buff->m_size == 0)
		{
			RECYCLE_LAYER_MSG(buff);
			continue;
		}
		m_broadBuff.push_back(buff);
	}

	for (size_t i = 0; i < nMsg->m_socks.size(); i++)
	{
		auto it = m_conns.find(nMsg->m_socks[i]);
		if (it == m_conns.end())
			continue;

		for (size_t j = 0; j < m_broadBuff.size(); j++)
		{
			uv_write_t* whand = GetLayer()->GetLoopObj<uv_write_t>();
			Write_t* buf = GetLayer()->GetLoopObj<Write_t>();
			buf->baseModule = this;
			buf->SetBlock(m_broadBuff[j]);
			whand->data = buf;
			uv_write(whand, (uv_stream_t*)it->second->conn, &buf->buf, 1, After_write);
		}
	}
	if (m_broadBuff.front()->m_ref == 0)
	{//no one send need recycle
		for (size_t i = 0; i < m_broadBuff.size(); i++)
		{
			if (m_broadBuff[i]->m_looplist)
				RECYCLE_LAYER_MSG(m_broadBuff[i]);
			else
				LOOP_RECYCLE((LocalBuffBlock*)m_broadBuff[i]);		//回收要指明子类型,否则不会调回收函数
		}
	}
	m_broadBuff.clear();
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