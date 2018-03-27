#ifndef DB_PROXY_MODULE_H
#define DB_PROXY_MODULE_H

#include "BaseModule.h"

class NetObjectModule;
class MsgModule;
class EventModule;
class TransMsgModule;
class ScheduleModule;

class DBProxyModule:public BaseModule
{
public:
	DBProxyModule(BaseLayer* l);
	~DBProxyModule();

private:
	// Í¨¹ý BaseModule ¼Ì³Ð
	virtual void Init() override;

	void OnServerConnect(SHARE<NetServer>& ser);
	void OnServerClose(SHARE<NetServer>& ser);
	void OnCheckProxy(int64_t& dt);
	
	void OnGetMysqlGroup(NetMsg* msg);

	void OnCreateAccount(NetMsg* msg);
	void OnMysqlMsg(NetMsg* msg);

	void SendToProxy(int group,const int& mid,NetMsg* msg);
private:
	NetObjectModule * m_netObjModule;
	MsgModule* m_msgModule;
	EventModule* m_eventModule;
	TransMsgModule* m_tranModule;
	ScheduleModule* m_scheduleModule;

	uint32_t m_hash;
	map<int, vector<SHARE<ServerNode>>> m_proxyGroup;
	map<int, int> m_groupMap;
	vector<int> m_groups;
	vector<SHARE<ServerNode>> m_path;
	vector<SHARE<ServerNode>> m_getGrouppath;
	map<int, SHARE<NetServer>> m_tmpProxy;
};

#endif