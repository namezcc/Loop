#include "Coro2Module.h"
#include "MsgModule.h"
#include "Coro1Module.h"

Coro2Module::Coro2Module(BaseLayer * l):BaseModule(l)
{
}

Coro2Module::~Coro2Module()
{
}

void Coro2Module::Init()
{
	m_msgModule = GET_MODULE(MsgModule);

	//m_msgModule->AddMsgCallBack(L_CORO_2_TEST_1, this, &Coro2Module::CoroTest1);
	m_msgModule->AddMsgCallBack(L_CORO_2_TEST_1, this, &Coro2Module::CoroTest3);
	//m_msgModule->AddAsynMsgCallBack(L_CORO_2_TEST_1, this, &Coro2Module::CoroTest2);

}

void Coro2Module::CoroTest1(NetSocket * sock)
{
	std::cout << "get coro msg sock:" << sock->socket << std::endl;
}

void Coro2Module::CoroTest2(SHARE<BaseMsg>& msg, c_pull & pull, SHARE<BaseCoro>& coro)
{
	auto sock = (NetSocket*)msg->m_data;
	std::cout << "get coro2 msg sock:" << sock->socket << std::endl;
	DieTest die;
	auto rep = GET_LAYER_MSG(NetSocket);
	rep->socket = 10;
	auto msg2 = m_msgModule->ResponseAsynMsg(msg, rep,pull,coro);
	sock = (NetSocket*)msg2->m_data;
	std::cout << "get coro2 msg step2 sock:" << sock->socket << std::endl;


}

void Coro2Module::CoroTest3(SHARE<BaseMsg>& msg)
{
	auto sock = (NetSocket*)msg->m_data;
	auto rep = GET_LAYER_MSG(NetSocket);
	rep->socket = sock->socket+10;
	m_msgModule->ResponseMsg(msg, rep);
}
