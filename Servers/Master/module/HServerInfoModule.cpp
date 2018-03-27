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
	m_msgModule->AddMsgCallBack<NetMsg>(L_HL_GET_MACHINE_LIST, this, &HServerInfoModule::OnGetMachineList);
}

void HServerInfoModule::OnReqGetMachineList(HttpMsg * msg)
{
	msg->opration = HttpMsg::NONE;
	m_msgModule->SendMsg(LY_LOGIC,0, L_HL_GET_MACHINE_LIST, new NetSocket(msg->socket));
}

void HServerInfoModule::OnGetMachineList(NetMsg * msg)
{
	m_httpModule->SendHttpMsg(msg->socket, [&msg](HttpMsg* hmsg) {
		hmsg->response.buff.append(msg->msg, msg->len);
		hmsg->opration = HttpMsg::SEND;
	});
}