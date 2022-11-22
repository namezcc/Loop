#include "Coro3Module.h"
#include "MsgModule.h"
#include "ScheduleModule.h"

#include "protoPB/base/LPBase.pb.h"

Coro3Module::Coro3Module(BaseLayer * l):BaseModule(l)
{
}

Coro3Module::~Coro3Module()
{
}

void Coro3Module::Init()
{
	m_msgModule = GET_MODULE(MsgModule);
	m_schedule = GET_MODULE(ScheduleModule);

	m_schedule->AddInterValTask(BIND_TIME(OnBeginTest), 3000, -1, 3000);
}

void Coro3Module::OnBeginTest(int64_t & dt)
{
	m_msgModule->DoCoroFunc([this](c_pull& p, SHARE<BaseCoro>& c) {
		CoroTest1(p, c);
	});
}

void Coro3Module::CoroTest1(c_pull & pull, SHARE<BaseCoro>& coro)
{
	int32_t base = rand() % 5000;
	std::cout << "--------------------------------" << std::endl;
	std::cout << "coro 'login' test begin:" << base << std::endl;
	int32_t hash = rand();
	LPMsg::ServerInfo msg;
	msg.set_id(base);
	msg.set_type(0);

	/*auto msg2 = m_sendProxyDb->RequestToProxyDb(msg, hash, N_CORO_TEST_1, pull, coro);
	auto netmsg = (NetServerMsg*)msg2->m_data;
	TRY_PARSEPB(LPMsg::ServerInfo, netmsg);
	std::cout << "coro 'login' test get:" << pbMsg.id() << std::endl;
	std::cout << "--------------------------------" << std::endl;*/
}
