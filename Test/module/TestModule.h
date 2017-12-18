#ifndef TEST_MODULE_H
#define TEST_MODULE_H
#include "BaseModule.h"

class ScheduleModule;
class NetObjectModule;
class TestModule:public BaseModule
{
public:
	TestModule(BaseLayer* l);
	~TestModule();

protected:
	virtual void Init() override;
	virtual void Execute() override;

	void RunPrint(int64_t nt);
private:
	ScheduleModule* m_schedule;
	NetObjectModule* m_netObject;
};

#endif