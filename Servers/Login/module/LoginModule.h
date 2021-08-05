﻿#ifndef LOGIN_MODULE_H
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
	void onTestAsyncPing(NetMsg* msg , c_pull & pull, SHARE<BaseCoro>& coro);
	void OnClientLogin(SHARE<BaseMsg>& comsg, c_pull & pull, SHARE<BaseCoro>& coro);

	void RemoveClient(const std::string& account);

private:
	MsgModule * m_msgModule;
	NetObjectModule* m_netModule;
	SendProxyDbModule* m_sendProxyDb;
	RedisModule* m_redisModule;
	RoomStateModule* m_roomModule;
	TransMsgModule* m_transModule;
	EventModule* m_eventModule;
	ScheduleModule* m_schedule;

	ServerNode m_lock_server;
	ServerPath m_db_path;

	map<string, SHARE<AccoutInfo>> m_cash;
	std::map<string,ClientObj> m_tmpClient;
};

#endif