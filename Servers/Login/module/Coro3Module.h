#ifndef CORO_3_MODULE_H
#define CORO_3_MODULE_H

#include "BaseModule.h"

class MsgModule;
class ScheduleModule;
class SendProxyDbModule;

template<typename T>
struct DiePrint
{
	DiePrint(const T& _d):m_d(_d)
	{}
	~DiePrint()
	{
		std::cout << "DiePrint:" << m_d << std::endl;
	}
	T m_d;
};

class Coro3Module:public BaseModule
{
public:
	Coro3Module(BaseLayer* l);
	~Coro3Module();

	virtual void Init() override;
protected:
	void OnBeginTest(int64_t& dt);
	void CoroTest1(c_pull& pull, SHARE<BaseCoro>& coro);

private:
	MsgModule * m_msgModule;
	ScheduleModule* m_schedule;
	SendProxyDbModule* m_sendProxyDb;
};

#endif
