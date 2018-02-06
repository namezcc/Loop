#ifndef TESTPROC_MODULE_H
#define TESTPROC_MODULE_H

#include "BaseModule.h"

class ScheduleModule;
class ProcessModule;

class TestProcModule:public BaseModule
{
public:
	TestProcModule(BaseLayer* l);
	~TestProcModule();

private:
	// Í¨¹ý BaseModule ¼Ì³Ð
	virtual void Init() override;
	virtual void Execute() override;

	void ProcTest(int64_t& nt);

protected:
	ScheduleModule* m_scheduleModule;
	ProcessModule* m_procModule;
};

#endif