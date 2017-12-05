#include "TestModule.h"
#include "ScheduleModule.h"
#include "NetObjectModule.h"
#include <iostream>

TestModule::TestModule(BaseLayer* l):BaseModule(l)
{

}

TestModule::~TestModule()
{
}

void TestModule::Init()
{
	m_schedule = GetLayer()->GetModule<ScheduleModule>();
	m_netObject = GetLayer()->GetModule<NetObjectModule>();

	m_schedule->AddInterValTask(this, &TestModule::RunPrint, 1,1,3);
}

void TestModule::Execute()
{
}

void TestModule::RunPrint(int64_t nt)
{
	m_netObject->AddServerConn(1, "127.0.0.1", 15001);
}