#ifndef TEST_MODULE
#define TEST_MODULE

#include "BaseModule.h"

class ScheduleModule;
class MsgModule;

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
	void OnGetData(NetSocket* sock);

	bool m_send;

	ScheduleModule* m_schedule;
	MsgModule* m_msgModule;
	int64_t m_start;
};

#endif