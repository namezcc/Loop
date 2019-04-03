#ifndef LOGIN_MODULE_H
#define LOGIN_MODULE_H

#include "BaseModule.h"

class MsgModule;
class NetObjectModule;
class SendProxyDbModule;
class RedisModule;
class RoomStateModule;
class TransMsgModule;
class EventModule;

struct AccoutInfo;
struct RoomServer;


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

	void OnClientConnect(const int32_t& sock);
	void OnTestPing(NetMsg* msg);
	void OnClientLogin(SHARE<BaseMsg>& comsg, c_pull & pull, SHARE<BaseCoro>& coro);
	void OnGetAccountInfo(SHARE<BaseMsg>& comsg, c_pull & pull, SHARE<BaseCoro>& coro);

	void CreateAccount(const string& name, const string& pass, c_pull & pull, SHARE<BaseCoro>& coro);
	void OnCreateAccount(SHARE<BaseMsg>& comsg, c_pull & pull, SHARE<BaseCoro>& coro);

	bool TryPlayerLogin(SHARE<ClientObj>& client, SHARE<AccoutInfo>& account, c_pull & pull, SHARE<BaseCoro>& coro,bool isCreate=false);

	void SendRoomInfo(SHARE<AccoutInfo>& account, SHARE<ClientObj>& client, RoomServer* room, c_pull & pull, SHARE<BaseCoro>& coro);
	void RemoveClient(const std::string& account);
private:
	MsgModule * m_msgModule;
	NetObjectModule* m_netModule;
	SendProxyDbModule* m_sendProxyDb;
	RedisModule* m_redisModule;
	RoomStateModule* m_roomModule;
	TransMsgModule* m_transModule;
	EventModule* m_eventModule;

	map<string, SHARE<AccoutInfo>> m_cash;
	map<string,SHARE<ClientObj>> m_tmpClient;
};

#endif