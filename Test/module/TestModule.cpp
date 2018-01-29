#include "TestModule.h"
#include "ScheduleModule.h"
#include "NetObjectModule.h"
#include "TransMsgModule.h"
#include "MsgModule.h"
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
	m_trans = GetLayer()->GetModule<TransMsgModule>();
	m_msg = GetLayer()->GetModule<MsgModule>();

	m_msg->AddMsgCallBack<NetMsg>(N_TRANS_TEST, this, &TestModule::OnTransTest);

	m_schedule->AddInterValTask(this, &TestModule::RunPrint, 1000,1,3000);
	m_schedule->AddInterValTask(this, &TestModule::TransTest, 1000, 1, 10000);
}

void TestModule::Execute()
{
}

void TestModule::RunPrint(int64_t nt)
{
	auto ser = Single::GetInstence<ServerNode>();//GetLayer()->GetServer();

	if(ser->type==SERVER_TYPE::LOOP_GAME)
		m_netObject->AddServerConn(SERVER_TYPE::LOOP_PROXY_GS, 1, "127.0.0.1", 25001);
	else if(ser->type == SERVER_TYPE::LOOP_MYSQL)
		m_netObject->AddServerConn(SERVER_TYPE::LOOP_PROXY_GS, 1, "127.0.0.1", 25001);
}

void TestModule::TransTest(int64_t nt)
{
	auto ser = Single::GetInstence<ServerNode>();//GetLayer()->GetServer();
	if (ser->type != SERVER_TYPE::LOOP_GAME)
		return;

	auto len = sizeof(ServerNode);
	auto msg = new char[len];
	memcpy(msg, ser, len);

	ServerNode to{ SERVER_TYPE::LOOP_MYSQL ,1};
	m_trans->SendToServer(to, N_TRANS_TEST, len, msg);
}

void TestModule::OnTransTest(NetMsg* msg)
{
	auto s = (ServerNode*)msg->msg;

	cout << "Get trans test type:" << s->type << " id:" << s->serid << endl;
}