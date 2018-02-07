#include "TestModule.h"
#include "ScheduleModule.h"
#include "NetObjectModule.h"
#include "TransMsgModule.h"
#include "MsgModule.h"
#include "MysqlModule.h"
#include "RedisModule.h"
#include <iostream>
#include <iomanip>

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
	//m_trans = GetLayer()->GetModule<TransMsgModule>();
	m_msg = GetLayer()->GetModule<MsgModule>();
	m_mysqlModule = GetLayer()->GetModule<MysqlModule>();
	m_redisModule = GetLayer()->GetModule<RedisModule>();

	//m_msg->AddMsgCallBack<NetMsg>(N_TRANS_TEST, this, &TestModule::OnTransTest);

	m_schedule->AddInterValTask(this, &TestModule::RedisTest, 1000,1,3000);
	//m_schedule->AddInterValTask(this, &TestModule::TransTest, 1000, 1, 10000);
}

void TestModule::AfterInit()
{
	m_mysqlModule->Connect("test", "127.0.0.1", "root", "123456");
	if (m_redisModule->Connect("127.0.0.1", "NoahGameFrame"))
		cout << "connect redis success..." << endl;
}

void TestModule::Execute()
{
}

void TestModule::RunPrint(int64_t nt)
{
	auto ser = GetLayer()->GetServer();

	if(ser->type==SERVER_TYPE::LOOP_GAME)
		m_netObject->AddServerConn(SERVER_TYPE::LOOP_PROXY_GS, 1, "127.0.0.1", 25001);
	else if(ser->type == SERVER_TYPE::LOOP_MYSQL)
		m_netObject->AddServerConn(SERVER_TYPE::LOOP_PROXY_GS, 1, "127.0.0.1", 25001);
}

void TestModule::TransTest(int64_t nt)
{
	auto ser = GetLayer()->GetServer();
	if (ser->type != SERVER_TYPE::LOOP_GAME)
		return;

	auto len = sizeof(ServerNode);
	auto msg = new char[len];
	memcpy(msg, ser, len);

	ServerNode to{ SERVER_TYPE::LOOP_MYSQL ,1};
	m_trans->SendToServer(to, N_TRANS_TEST, len, msg);
}

void TestModule::SqlTest(int64_t nt)
{
	MultRow res;
	SqlRow files;
	m_mysqlModule->Select("select * from tuser;",res, files);
	int w = 10;
	for (auto& f:files)
		cout <<setw(w)<< f;
	cout << endl;
	for (auto& row:res)
	{
		for (auto& v:row)
			cout << setw(w) << v;
		cout << endl;
	}
}

void TestModule::RedisTest(int64_t nt)
{
	string key("S6:Player:6-104000022");

	vector<pair<string, string>> res;
	m_redisModule->HGetAll(key, res);

	for (auto& p:res)
	{
		cout << p.first << "  <------------>  " << p.second << endl;
	}
}

void TestModule::OnTransTest(NetMsg* msg)
{
	auto s = (ServerNode*)msg->msg;

	cout << "Get trans test type:" << s->type << " id:" << s->serid << endl;
}