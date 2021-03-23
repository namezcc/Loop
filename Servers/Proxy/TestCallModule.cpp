#include "TestCallModule.h"
#include "ScheduleModule.h"
#include "MsgModule.h"
#include "EventModule.h"
#include "NetObjectModule.h"

TestCallModule::TestCallModule(BaseLayer * l):BaseModule(l), m_send(false)
{
}

void TestCallModule::Init()
{
	m_msgModule = GET_MODULE(MsgModule);
	m_netModule = GET_MODULE(NetObjectModule);

	m_msgModule->AddMsgCall(10001, BIND_CALL(OnGetData, NetMsg));
	m_msgModule->AddMsgCall(11001, BIND_CALL(OnTestData, NetMsg));
	
	m_netModule->SetAccept(true, CONN_CLIENT);

	/*if (m_send)
	{
		m_schedule = GET_MODULE(ScheduleModule);
		m_eventModule = GET_MODULE(EventModule);

		m_schedule->AddInterValTask(BIND_TIME(startSend), 1000, -1,3000);

		for (size_t i = 0; i < 60; i++)
		{
			m_schedule->AddTimePointTask(BIND_TIME(OnTimeTest), -1, i);
		}

		m_eventModule->AddEventCall(30001, BIND_EVENT(OnEvent1, int32_t));
		m_eventModule->AddEventCall(30002, BIND_EVENT(OnEvent2, NetSocket*));
		m_eventModule->AddEventCall(30003, BIND_EVENT(OnEvent3, SHARE<NetSocket>));
	}*/
}

void TestCallModule::startSend(int64_t & dt)
{
	//auto t1 = GetMilliSecend();

	//for (size_t i = 0; i < 1000000; i++)
	//{
	//	auto sock = GET_LOOP(NetSocket);
	//	LOOP_RECYCLE(sock);
	//	//m_eventModule->SendEvent(30001, (int32_t)10);
	//	//m_eventModule->SendEvent2(30001, 10);
	//}
	//auto t2 = GetMilliSecend();
	//LP_INFO << "use time " << t2 - t1 << "ms";

	//for (size_t i = 0; i < 1000000; i++)
	//{
	//	auto sock = Single::LocalInstance<Block2<NetSocket>>()->allocateNewOnce();
	//	Single::LocalInstance<Block2<NetSocket>>()->deallcate(sock);
	//	//m_eventModule->SendEvent(30001, (int32_t)10);
	//	//m_eventModule->SendEvent2(30001, 10);
	//}
	//auto t3 = GetMilliSecend();
	//LP_INFO << "use time " << t3 - t2 << "ms";
	/*auto sock = GET_SHARE(NetSocket);
	sock->socket = 100;
	m_eventModule->SendEvent(30002, sock.get());
	m_eventModule->SendEvent(30003, sock);*/

	for (int32_t i = 0; i < 10; i++)
	{
		auto msg = GET_LAYER_MSG(NetMsg);
		msg->socket = i;
		m_msgModule->SendMsg(20001, msg);
	}
}

void TestCallModule::OnTimeTest(int64_t & dt)
{
	int a = 0;
	++a;
}

void TestCallModule::OnGetData(NetMsg * sock)
{
	if (sock->socket == 0)
		m_start = GetMilliSecend();

	if (sock->socket == 10 - 1)
	{
		auto endtime = GetMilliSecend();
		//LP_INFO << " use tme " << endtime - m_start << " ms";
	}
}

void TestCallModule::OnTestData(NetMsg * msg)
{
	//assert(msg != NULL);
	auto buff = GET_LAYER_MSG(BuffBlock);
	buff->makeRoom(msg->getLen());
	buff->write(msg->getNetBuff(), msg->getLen());
	m_netModule->SendNetMsg(msg->socket, msg->mid, buff);
}

void TestCallModule::OnEvent1(const int32_t & arg)
{
	int i = 0;
	++i;
	//LP_INFO << "event 1 :" << arg;
}

void TestCallModule::OnEvent2(NetMsg * sock)
{
	LP_INFO << "event 2 :" << sock->socket;
}

void TestCallModule::OnEvent3(SHARE<NetMsg>& sock)
{
	LP_INFO << "event 3 :" << sock->socket;
}
