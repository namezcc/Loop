#ifndef TEST_TIME_H
#define TEST_TIME_H

#include "BaseModule.h"

class MsgModule;

class TestTime:public BaseModule
{
public:
	TestTime(BaseLayer* l);
	~TestTime();

private:

	// 通过 BaseModule 继承
	virtual void Init() override;

	void OnTestBegin(NetMsg* s);
	void OnTest(NetMsg* s);
	void OnTestEnd(NetMsg* s);


	int64_t m_beg;
	int32_t m_num;
	MsgModule* m_msgModule;
};
#endif
