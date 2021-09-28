#ifndef TEST_PROCESS_MODULE_H
#define TEST_PROCESS_MODULE_H
#include "BaseModule.h"

COM_MOD_CLASS;

class ProcessModule;
class ScheduleModule;
class EventModule;


class TestProcessModule:public BaseModule
{
public:
	TestProcessModule(BaseLayer* l);
	~TestProcessModule();

private:
	// Í¨¹ý BaseModule ¼Ì³Ð
	virtual void Init() override;
	void initServerName();

	void onServerConnect(SHARE<NetServer>& ser);
	void onServerClose(SHARE<NetServer>& ser);

	void onGetAllServerInfo(NetMsg* msg);

	void onServerState(NetMsg* msg);
	void onGetMachineServerInfo(SHARE<BaseMsg>& msg);
	void onGetServerInfo(SHARE<BaseMsg>& msg, c_pull & pull, SHARE<BaseCoro>& coro);
	void onServerOpt(NetMsg* msg);

	std::string getServerPid(int32_t type, int32_t id);
	void closeServer(int32_t type, int32_t id);
	void openServer(int32_t type, int32_t id);

	void sendConnInfo(int32_t type,int32_t id);



	ProcessModule* m_processModule;
	ScheduleModule* m_scheduleModule;
	EventModule* m_event_mod;
	COM_MOD_OBJ;
	

	std::map<int32_t, std::string> m_server_name;
	std::map<int64_t, int32_t> m_server_state;
	std::map<int32_t, std::vector<ServerConfigInfo>> m_all_server;
	std::vector<ServerConfigInfo> m_self_server;
};

#endif
