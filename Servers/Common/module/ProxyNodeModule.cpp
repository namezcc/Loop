#include "ProxyNodeModule.h"
#include "EventModule.h"
#include "TransMsgModule.h"

ProxyNodeModule::ProxyNodeModule(BaseLayer * l):BaseModule(l), m_proxyId(0)
{
}

ProxyNodeModule::~ProxyNodeModule()
{
}

void ProxyNodeModule::Init()
{
	m_eventModule = GET_MODULE(EventModule);
	m_transModule = GET_MODULE(TransMsgModule);

	m_eventModule->AddEventCall(E_SERVER_CONNECT,BIND_EVENT(OnServerConnect,SHARE<NetServer>));


}

void ProxyNodeModule::OnServerConnect(SHARE<NetServer>& ser)
{
	if (ser->type == SERVER_TYPE::LOOP_PROXY)
	{
		m_proxyId = ser->serid;

	}
}

void ProxyNodeModule::SendToNode(const int32_t& proxyId, const int32_t & stype, const int32_t & serid, const int32_t& mid, gpb::Message & msg)
{
	if (proxyId == m_proxyId)
	{
		
	}
	else
	{
		
	}
}