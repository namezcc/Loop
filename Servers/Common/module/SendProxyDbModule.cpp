#include "SendProxyDbModule.h"
#include "TransMsgModule.h"

SendProxyDbModule::SendProxyDbModule(BaseLayer * l):BaseModule(l)
{
}

SendProxyDbModule::~SendProxyDbModule()
{
}

void SendProxyDbModule::Init()
{
	m_tranModule = GET_MODULE(TransMsgModule);

	m_proxy.serid = 0;
	m_proxy.type = LOOP_PROXY_DB;
}

void SendProxyDbModule::SendToProxyDb(google::protobuf::Message & msg, const int & hash, const int & mid)
{
	int len;
	char* send = PB::PBToChar(msg, len, sizeof(hash));
	*(int*)send = hash;
	m_tranModule->SendToServer(m_proxy, mid, send, len);
}
