#include "TestProcessModule.h"
#include "ProcessModule.h"
#include "ScheduleModule.h"

TestProcessModule::TestProcessModule(BaseLayer* l):BaseModule(l)
{

}

TestProcessModule::~TestProcessModule()
{
}

void TestProcessModule::Init()
{
	m_processModule = GET_MODULE(ProcessModule);
	m_scheduleModule = GET_MODULE(ScheduleModule);

	m_scheduleModule->AddInterValTask(BIND_TIME(CreateProcessTest), 5000, 1, 10000);
}

void TestProcessModule::CreateProcessTest(int64_t & dt)
{
	//m_processModule->CreateServer(SERVER_TYPE::LOOP_MASTER,1,3000);

	int a = 100;
	std::cout << dt << std::endl;
	int* b = NULL;
	*b = 100;
	int c = 1;
}
