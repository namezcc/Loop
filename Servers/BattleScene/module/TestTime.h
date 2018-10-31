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

	// Í¨¹ý BaseModule ¼Ì³Ð
	virtual void Init() override;

	void OnTestBegin(NetSocket* s);
	void OnTest(NetSocket* s);
	void OnTestEnd(NetSocket* s);


	int64_t m_beg;
	int32_t m_num;
	MsgModule* m_msgModule;
};
#endif
