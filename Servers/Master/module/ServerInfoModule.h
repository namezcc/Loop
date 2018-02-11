#ifndef SERVER_INFO_MODULE_H
#define SERVER_INFO_MODULE_H

#include "BaseModule.h"

class MsgModule;
class MysqlModule;
class ServerInfo;

class ServerInfoModule:public BaseModule
{
public:
	ServerInfoModule(BaseLayer* l);
	~ServerInfoModule();

private:
	// Í¨¹ý BaseModule ¼Ì³Ð
	virtual void Init() override;
	virtual void BeforExecute() override;

	void InitServers();

	void OnGetMachineList(NetSocket* sock);
private:
	MsgModule* m_msgModule;
	MysqlModule* m_mysqlModule;

	vector<SHARE<ServerInfo>> m_console;
};

#endif