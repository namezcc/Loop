#include "TestTime.h"
#include "MsgModule.h"

TestTime::TestTime(BaseLayer * l):BaseModule(l),m_beg(0), m_num(0)
{
}

TestTime::~TestTime()
{
}

void TestTime::Init()
{
	m_msgModule = GET_MODULE(MsgModule);

	m_msgModule->AddMsgCallBack(2001, this, &TestTime::OnTestBegin);
	m_msgModule->AddMsgCallBack(2002, this, &TestTime::OnTest);
	m_msgModule->AddMsgCallBack(2003, this, &TestTime::OnTestEnd);
}

void TestTime::OnTestBegin(NetMsg * s)
{
	m_beg = GetMilliSecend();
	LP_WARN << "Test begin:" << m_beg;
	m_num = 0;
}

void TestTime::OnTest(NetMsg * s)
{
	//do something
	//auto t = GetMilliSecend();
	++m_num;
}

void TestTime::OnTestEnd(NetMsg * s)
{
	auto mend = GetMilliSecend();
	LP_WARN << "use time ms:" << mend - m_beg << "  num:"<< m_num;
}

