#include "TcpClientModule.h"
#include "Define.h"
#include "NetModule.h"
#include "MsgModule.h"

TcpClientModule::TcpClientModule(BaseLayer* l):BaseModule(l)
{
}

TcpClientModule::~TcpClientModule()
{
}

void TcpClientModule::Init()
{
	m_netmodule = GetLayer()->GetModule<NetModule>();
	m_msgmodule = GetLayer()->GetModule<MsgModule>();

	m_msgmodule->AddMsgCallBack<NetServer>(L_TO_CONNET_SERVER,this, &TcpClientModule::OnConnectServer);
}

void TcpClientModule::Execute()
{
}

void TcpClientModule::OnConnectServer(NetServer* ser)
{
	struct sockaddr_in addr;
	uv_tcp_t* client = GetLayer()->GetLoopObj<uv_tcp_t>();
	uv_connect_t* connect_req = GetLayer()->GetLoopObj<uv_connect_t>();
	connect_req->data = this;
	int r;

	ASSERT(0 == uv_ip4_addr(ser->ip.c_str(), ser->port, &addr));
	ASSERT(client != NULL);
	ASSERT(connect_req != NULL);

	r = uv_tcp_init(uv_default_loop(), client);
	ASSERT(r == 0);

	client->data = new NetServer(*ser);

	r = uv_tcp_connect(connect_req,client,(const struct sockaddr*) &addr,Connect_cb);
	ASSERT(r == 0);
}

void TcpClientModule::Connect_cb(uv_connect_t* req, int status) 
{
	int r;

	ASSERT(req != NULL);
	ASSERT(status == 0);

	auto md = (TcpClientModule*)req->data;
	/* Not that the server will send anything, but otherwise we'll never know */
	/* when the server closes the connection. */
	auto cli = (uv_tcp_t*)req->handle;
	auto ser = (NetServer*)cli->data;
	
	ser->socket = cli->socket;
	md->m_msgmodule->SendMsg(L_SERVER_CONNECTED, ser);

	cli->data = md->m_netmodule;
	cli->close_cb = &TcpClientModule::On_server_close;
	md->m_netmodule->Connected(cli, false);
}

void TcpClientModule::On_server_close(uv_handle_t* client)
{
	auto tcpcli = (uv_tcp_t*)client;
	auto server = (NetModule*)tcpcli->data;
	auto sock = new NetSocket(tcpcli->socket);
	server->GetMsgModule()->SendMsg(L_SERVER_CLOSE, sock);
	server->RemoveConn(tcpcli->socket);
}