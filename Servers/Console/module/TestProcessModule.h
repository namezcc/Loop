#ifndef TEST_PROCESS_MODULE_H
#define TEST_PROCESS_MODULE_H
#include "BaseModule.h"

class ProcessModule;
class ScheduleModule;

class TestProcessModule:public BaseModule
{
public:
	TestProcessModule(BaseLayer* l);
	~TestProcessModule();

private:
	// ͨ�� BaseModule �̳�
	virtual void Init() override;

	void CreateProcessTest(int64_t& dt);


	ProcessModule* m_processModule;
	ScheduleModule* m_scheduleModule;
};

#endif
