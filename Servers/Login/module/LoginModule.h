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
class ScheduleModule;

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
	// 通过 BaseModule 继承
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

	void testSqlMsg(int64_t& dt);
	void OnSqlMsg(NetMsg* msg);
private:
	MsgModule * m_msgModule;
	NetObjectModule* m_netModule;
	SendProxyDbModule* m_sendProxyDb;
	RedisModule* m_redisModule;
	RoomStateModule* m_roomModule;
	TransMsgModule* m_transModule;
	EventModule* m_eventModule;
	ScheduleModule* m_schedule;

	map<string, SHARE<AccoutInfo>> m_cash;
	map<string,SHARE<ClientObj>> m_tmpClient;
};

#endif