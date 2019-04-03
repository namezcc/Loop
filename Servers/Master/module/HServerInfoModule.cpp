#include "HServerInfoModule.h"
#include "HttpLogicModule.h"
#include "MsgModule.h"

HServerInfoModule::HServerInfoModule(BaseLayer* l):BaseModule(l)
{
}

HServerInfoModule::~HServerInfoModule()
{
}

void HServerInfoModule::Init()
{
	m_httpModule = GET_MODULE(HttpLogicModule);
	m_msgModule = GET_MODULE(MsgModule);

	m_httpModule->AddUrlCallBack("^/getMachineList", GET, this, &HServerInfoModule::OnReqGetMachineList);
	m_msgModule->AddMsgCallBack(L_HL_GET_MACHINE_LIST, this, &HServerInfoModule::OnGetMachineList);
}

void HServerInfoModule::OnReqGetMachineList(HttpMsg * msg)
{
	msg->opration = HttpMsg::NONE;
	auto sock = GET_LAYER_MSG(NetSocket);
	sock->socket = msg->socket;
	m_msgModule->SendMsg(LY_LOGIC,0, L_HL_GET_MACHINE_LIST, sock);
}

void HServerInfoModule::OnGetMachineList(NetMsg * msg)
{
	auto buff = msg->getCombinBuff();
	m_httpModule->SendHttpMsg(msg->socket, [buff](HttpMsg* hmsg) {
		hmsg->response.buff.append(buff->m_buff, buff->m_size);
		hmsg->opration = HttpMsg::SEND;
	});
}