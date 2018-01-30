#ifndef TEST_MODULE_H
#define TEST_MODULE_H
#include "BaseModule.h"

class ScheduleModule;
class NetObjectModule;
class TransMsgModule;
class MsgModule;
class MysqlModule;

class TestModule:public BaseModule
{
public:
	TestModule(BaseLayer* l);
	~TestModule();

protected:
	virtual void Init() override;
	virtual void AfterInit();
	virtual void Execute() override;

	void RunPrint(int64_t nt);
	void TransTest(int64_t nt);

	void SqlTest(int64_t nt);

	void OnTransTest(NetMsg* msg);
private:
	ScheduleModule* m_schedule;
	NetObjectModule* m_netObject;
	TransMsgModule* m_trans;
	MsgModule* m_msg;
	MysqlModule* m_mysqlModule;
};

#endif