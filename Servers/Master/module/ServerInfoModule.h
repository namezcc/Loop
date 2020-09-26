#ifndef SERVER_INFO_MODULE_H
#define SERVER_INFO_MODULE_H

#include "BaseModule.h"

class MsgModule;
class MysqlModule;
struct ServerInfo;

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

	void OnGetMachineList(NetSocket* sock);
private:
	MsgModule* m_msgModule;
	MysqlModule* m_mysqlModule;

	vector<SHARE<ServerInfo>> m_console;
};

#endif