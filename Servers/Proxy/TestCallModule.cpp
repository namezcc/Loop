#include "TestCallModule.h"
#include "ScheduleModule.h"
#include "MsgModule.h"

TestCallModule::TestCallModule(BaseLayer * l):BaseModule(l), m_send(false)
{
}

void TestCallModule::Init()
{
	m_msgModule = GET_MODULE(MsgModule);
	m_msgModule->AddMsgCallBack(20001, this, &TestCallModule::OnGetData);

	if (m_send)
	{
		m_schedule = GET_MODULE(ScheduleModule);
		m_schedule->AddInterValTask(this, &TestCallModule::startSend, 0, 1,3000);
	}
}

void TestCallModule::startSend(int64_t & dt)
{
	for (int32_t i = 0; i < 100000; i++)
	{
		auto msg = GET_LAYER_MSG(NetSocket);
		msg->socket = i;
		m_msgModule->SendMsg(20001, msg);
	}
}

void TestCallModule::OnGetData(NetSocket * sock)
{
	if (sock->socket == 0)
		m_start = GetMilliSecend();

	if (sock->socket == 10000 - 1)
	{
		auto endtime = GetMilliSecend();
		LP_INFO << " use tme " << endtime - m_start << " ms";
	}
}
