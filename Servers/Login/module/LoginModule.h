#ifndef LOGIN_MODULE_H
#define LOGIN_MODULE_H

#include "BaseModule.h"

class MsgModule;
class NetObjectModule;
struct AccoutInfo;
class SendProxyDbModule;
class RedisModule;

struct ClientObj
{
	int sock;
	string pass;
};

class LoginModule:public BaseModule
{
public:
	LoginModule(BaseLayer* l);
	~LoginModule();

private:
	// Í¨¹ý BaseModule ¼Ì³Ð
	virtual void Init() override;

	void OnClientLogin(NetMsg* msg);
	void OnGetAccountInfo(NetMsg* msg);

	void CreateAccount(const string& name, const string& pass);
	void OnCreateAccount(NetMsg* msg);

private:
	MsgModule * m_msgModule;
	NetObjectModule* m_netModule;
	SendProxyDbModule* m_sendProxyDb;
	RedisModule* m_redisModule;

	map<string, SHARE<AccoutInfo>> m_cash;
	map<string,SHARE<ClientObj>> m_tmpClient;
};

#endif