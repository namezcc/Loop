#ifndef TEST_MODULE
#define TEST_MODULE

#include "BaseModule.h"

class ScheduleModule;
class MsgModule;
class EventModule;
class NetObjectModule;

class TestCallModule:public BaseModule
{
public:
	TestCallModule(BaseLayer* l);
	~TestCallModule() {};

	void setSend(bool send = true) { m_send = send; };

private:
	// Í¨¹ý BaseModule ¼Ì³Ð
	virtual void Init() override;

	void startSend(int64_t& dt);
	void OnTimeTest(int64_t& dt);

	void OnGetData(NetSocket* sock);
	void OnTestData(NetMsg* msg);
	
	void OnEvent1(const int32_t& arg);
	void OnEvent2(NetSocket* sock);
	void OnEvent3(SHARE<NetSocket>& sock);

	bool m_send;

	ScheduleModule* m_schedule;
	MsgModule* m_msgModule;
	EventModule* m_eventModule;
	NetObjectModule* m_netModule;

	int64_t m_start;
};

#endif