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

	m_eventModule->AddEventCallBack(E_SERVER_CONNECT, this, &ProxyNodeModule::OnServerConnect);

	m_shortPath = m_transModule->GetFromSelfPath(3, 0);
	m_shortPath[1]->type = SERVER_TYPE::LOOP_PROXY;
	m_shortPath[1]->serid = 0;

	m_longPath = m_transModule->GetFromSelfPath(5, 0);
	m_longPath[1]->type = SERVER_TYPE::LOOP_PROXY;
	m_longPath[2]->type = SERVER_TYPE::LOOP_PROXY_PP;
	m_longPath[2]->serid = 0;
	m_longPath[3]->type = SERVER_TYPE::LOOP_PROXY;
}

void ProxyNodeModule::OnServerConnect(SHARE<NetServer>& ser)
{
	if (ser->type == SERVER_TYPE::LOOP_PROXY)
	{
		m_proxyId = ser->serid;

		m_shortPath[1]->serid = m_proxyId;
		m_longPath[1]->serid = m_proxyId;
	}
}

void ProxyNodeModule::SendToNode(const int32_t& proxyId, const int32_t & stype, const int32_t & serid, const int32_t& mid, gpb::Message & msg)
{
	if (proxyId == m_proxyId)
	{
		m_shortPath[2]->type = stype;
		m_shortPath[2]->serid = serid;
		m_transModule->SendToServer(m_shortPath, mid, msg);
	}
	else
	{
		m_longPath[3]->serid = proxyId;
		m_longPath[4]->type = stype;
		m_longPath[4]->serid = serid;
		m_transModule->SendToServer(m_longPath, mid, msg);
	}
}