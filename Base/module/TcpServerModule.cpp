#include "TcpServerModule.h"
#include "NetModule.h"
#include "MsgModule.h"

void TcpServer::Init()
{
	m_netModule = GetLayer()->GetModule<NetModule>();

}

void TcpServer::AfterInit()
{
	start();
}

void TcpServer::SetBind(const int & port, uv_loop_t * loop)
{
	m_uvloop = loop;
	m_port = port;
}

void TcpServer::start()
{
	struct sockaddr_in addr;
	ASSERT(0 == uv_ip4_addr("0.0.0.0", m_port, &addr));
	int r;
	r = uv_tcp_init(m_uvloop, &m_hand);
	ASSERT(r == 0);
	r = uv_tcp_bind(&m_hand, (const struct sockaddr*) &addr, 0);
	ASSERT(r == 0);
	m_hand.data = this;
	r = uv_listen((uv_stream_t*)&m_hand, 4096, connection_cb);
	ASSERT(r == 0);
	//std::cout << "start listen port:" << port << std::endl;
	LP_WARN(m_netModule->GetMsgModule()) << "start listen port:" << m_port;
}

void TcpServer::connection_cb(uv_stream_t* serhand, int status)
{
	ASSERT(status == 0);
	auto server = (TcpServer*)serhand->data;
	uv_tcp_t* client = server->GetLayer()->GetLoopObj<uv_tcp_t>();
	int r = uv_tcp_init(server->m_uvloop, client);
	ASSERT(r == 0);
	r = uv_accept(serhand, (uv_stream_t*)client);
	ASSERT(r == 0);
	client->data = server->m_netModule;
	client->read_cb = NetModule::after_read;
	server->m_netModule->Connected(client);
}