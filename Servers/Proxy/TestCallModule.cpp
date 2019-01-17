#include "TestCallModule.h"
#include "ScheduleModule.h"
#include "MsgModule.h"
#include "EventModule.h"

TestCallModule::TestCallModule(BaseLayer * l):BaseModule(l), m_send(false)
{
}

void TestCallModule::Init()
{
	m_msgModule = GET_MODULE(MsgModule);

	m_msgModule->AddMsgCall(20001, BIND_CALL(OnGetData, NetSocket));

	if (m_send)
	{
		m_schedule = GET_MODULE(ScheduleModule);
		m_eventModule = GET_MODULE(EventModule);

		m_schedule->AddInterValTask(BIND_TIME(startSend), 0, 1,3000);

		m_eventModule->AddEventCall(30001, BIND_EVENT(OnEvent1, int32_t));
		m_eventModule->AddEventCall(30002, BIND_EVENT(OnEvent2, NetSocket*));
		m_eventModule->AddEventCall(30003, BIND_EVENT(OnEvent3, SHARE<NetSocket>));
	}
}

void TestCallModule::startSend(int64_t & dt)
{
	auto t1 = GetMilliSecend();

	for (size_t i = 0; i < 1000000; i++)
	{
		m_eventModule->SendEvent(30001, (int32_t)10);
		//m_eventModule->SendEvent2(30001, 10);
	}
	
	auto t2 = GetMilliSecend();
	LP_INFO << "use time " << t2 - t1 << "ms";
	/*auto sock = GET_SHARE(NetSocket);
	sock->socket = 100;
	m_eventModule->SendEvent(30002, sock.get());
	m_eventModule->SendEvent(30003, sock);*/

	/*for (int32_t i = 0; i < 100000; i++)
	{
		auto msg = GET_LAYER_MSG(NetSocket);
		msg->socket = i;
		m_msgModule->SendMsg(20001, msg);
	}*/
}

void TestCallModule::OnGetData(NetSocket * sock)
{
	if (sock->socket == 0)
		m_start = GetMilliSecend();

	if (sock->socket == 100000 - 1)
	{
		auto endtime = GetMilliSecend();
		LP_INFO << " use tme " << endtime - m_start << " ms";
	}
}

void TestCallModule::OnEvent1(const int32_t & arg)
{
	int i = 0;
	++i;
	//LP_INFO << "event 1 :" << arg;
}

void TestCallModule::OnEvent2(NetSocket * sock)
{
	LP_INFO << "event 2 :" << sock->socket;
}

void TestCallModule::OnEvent3(SHARE<NetSocket>& sock)
{
	LP_INFO << "event 3 :" << sock->socket;
}
