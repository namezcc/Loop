#include "Coro1Module.h"
#include "MsgModule.h"
#include "ScheduleModule.h"
#include "TransMsgModule.h"

#include "protoPB/base/LPBase.pb.h"

Coro1Module::Coro1Module(BaseLayer* l):BaseModule(l)
{
}

Coro1Module::~Coro1Module()
{
}

void Coro1Module::Init()
{
	m_msgModule = GET_MODULE(MsgModule);
	m_schedule = GET_MODULE(ScheduleModule);
	m_transModule = GET_MODULE(TransMsgModule);

	//m_schedule->AddInterValTask(this, &Coro1Module::OnBeginTest, 3000, 1, 3000);
	m_msgModule->AddAsynMsgCallBack(N_CORO_TEST_1, this, &Coro1Module::CoroTest2);
}

void Coro1Module::OnBeginTest(const int64_t & dt)
{
	m_msgModule->DoCoroFunc([this](c_pull& p,SHARE<BaseCoro>& c) {
		CoroTest1(p, c);
	});
}

void Coro1Module::CoroTest1(c_pull & pull, SHARE<BaseCoro>& coro)
{
	std::cout << "coro test begin" << std::endl;

	auto sock = GET_LAYER_MSG(NetSocket);
	sock->socket = 1;
	DieTest die;
	auto msg = m_msgModule->RequestAsynMsg(L_CORO_2_TEST_1, sock, pull, coro, LAYER_TYPE::LY_MYSQL, 0);
	auto sock2 = (NetSocket*)msg->m_data;
	std::cout << "coro get form mysql step 2 sock: " << sock2->socket<<std::endl;
	sock = GET_LAYER_MSG(NetSocket);
	sock->socket = 11;
	m_msgModule->ResponseMsg(msg, sock, LAYER_TYPE::LY_MYSQL, 0);
}

void Coro1Module::CoroTest2(SHARE<BaseMsg>& msg, c_pull & pull, SHARE<BaseCoro>& coro)
{
	auto netmsg = (NetServerMsg*)msg->m_data;
	TRY_PARSEPB(LPMsg::ServerInfo, netmsg);
	std::cout << "coro forward get sock:" << pbMsg.id() << std::endl;
	auto sock = GET_LAYER_MSG(NetSocket);
	DieTest die;
	sock->socket = pbMsg.id();
	auto msg2 = m_msgModule->RequestAsynMsg(L_CORO_2_TEST_1, sock, pull, coro, LAYER_TYPE::LY_MYSQL, 0);
	sock = (NetSocket*)msg2->m_data;
	std::cout << "coro forward back sock:" << sock->socket << std::endl;
	pbMsg.set_id(sock->socket);
	m_transModule->ResponseBackServerMsg(netmsg->path, msg, pbMsg);
}
