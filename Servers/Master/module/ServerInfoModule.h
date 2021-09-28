#ifndef SERVER_INFO_MODULE_H
#define SERVER_INFO_MODULE_H

#include "BaseModule.h"

#include "ReflectData.h"

class MsgModule;
class MysqlModule;
class NetObjectModule;
class TransMsgModule;

class ServerInfoModule:public BaseModule
{
public:
	ServerInfoModule(BaseLayer* l);
	~ServerInfoModule();

private:
	// 通过 BaseModule 继承
	virtual void Init() override;
	virtual void BeforExecute() override;

	void InitServers();

	void OnGetMachineList(NetMsg* sock);

	void onGetAllServerInfo(NetMsg* msg);

	void onWebTest(NetMsg* msg);
	void onWebRequest(NetMsg* msg);
	void onWebGetMachineInfo(NetMsg* msg, c_pull & pull, SHARE<BaseCoro>& coro);
	void onWebGetServerInfo(NetMsg* msg, c_pull & pull, SHARE<BaseCoro>& coro);
	void onWebServerOpt(NetMsg* msg);
	


	MachineServer* getServerInfo(int32_t mid, int32_t type, int32_t id);

private:
	MsgModule* m_msgModule;
	MysqlModule* m_mysqlModule;
	NetObjectModule* m_net_mod;
	TransMsgModule* m_trans_mos;

	std::map<int32_t, std::map<int64_t, MachineServer>> m_server;
	std::map<int32_t, dbMachine> m_machine;

};

#endif