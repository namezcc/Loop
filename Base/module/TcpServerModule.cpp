#include "TcpServerModule.h"
#include "NetModule.h"

void TcpServer::Init()
{
	m_netModule = GetLayer()->GetModule<NetModule>();
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