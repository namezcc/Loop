#ifndef PROXY_NODE_MODULE_H
#define PROXY_NODE_MODULE_H

#include "BaseModule.h"

class EventModule;
class TransMsgModule;

class ProxyNodeModule:public BaseModule
{
public:
	ProxyNodeModule(BaseLayer* l);
	~ProxyNodeModule();

	inline int32_t GetProxyId() { return m_proxyId; }
	void SendToNode(const int32_t& proxyId,const int32_t& stype,const int32_t& serid, const int32_t& mid,gpb::Message& msg);
private:
	// Í¨¹ý BaseModule ¼Ì³Ð
	virtual void Init() override;
	void OnServerConnect(SHARE<NetServer>& ser);


	EventModule* m_eventModule;
	TransMsgModule* m_transModule;

	int32_t m_proxyId;

	VecPath m_shortPath;
	VecPath m_longPath;
};

#endif
