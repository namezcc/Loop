#ifndef CORO_1_MODULE_H
#define CORO_1_MODULE_H
#include "BaseModule.h"

class MsgModule;
class ScheduleModule;
class TransMsgModule;

struct DieTest
{
	~DieTest()
	{
		std::cout << "DieTest" << std::endl;
	}
};

class Coro1Module:public BaseModule
{
public:
	Coro1Module(BaseLayer* l);
	~Coro1Module();

	// Í¨¹ý BaseModule ¼Ì³Ð
	virtual void Init() override;

protected:

	void OnBeginTest(const int64_t& dt);
	void CoroTest1(c_pull& pull, SHARE<BaseCoro>& coro);
	void CoroTest2(SHARE<BaseMsg>& msg, c_pull& pull, SHARE<BaseCoro>& coro);

private:
	MsgModule* m_msgModule;
	ScheduleModule* m_schedule;
	TransMsgModule* m_transModule;
};

#endif
