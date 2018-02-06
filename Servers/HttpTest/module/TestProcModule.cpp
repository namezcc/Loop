#include "TestProcModule.h"
#include "ProcessModule.h"
#include "ScheduleModule.h"

TestProcModule::TestProcModule(BaseLayer * l):BaseModule(l)
{
}

TestProcModule::~TestProcModule()
{
}

void TestProcModule::Init()
{
	m_scheduleModule = GetLayer()->GetModule<ScheduleModule>();
	m_procModule = GetLayer()->GetModule<ProcessModule>();

	m_scheduleModule->AddInterValTask(this, &TestProcModule::ProcTest, 1000, 1, 3000);
}

void TestProcModule::Execute()
{
}

void TestProcModule::ProcTest(int64_t & nt)
{
	m_procModule->CreateServer("Test", "-p 24001 -t 0 -n 1");


}