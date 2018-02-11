#include "ServerInfoModule.h"
#include "MsgModule.h"
#include "MysqlModule.h"
#include "ReflectData.h"
#include "json/json.h"

ServerInfoModule::ServerInfoModule(BaseLayer * l):BaseModule(l)
{
}

ServerInfoModule::~ServerInfoModule()
{
}

void ServerInfoModule::Init()
{
	m_msgModule = GET_MODULE(MsgModule);
	m_mysqlModule = GET_MODULE(MysqlModule);

	m_msgModule->AddMsgCallBack<NetSocket>(L_HL_GET_MACHINE_LIST, this, &ServerInfoModule::OnGetMachineList);

}

void ServerInfoModule::BeforExecute()
{
	InitServers();
}

void ServerInfoModule::InitServers()
{
	m_mysqlModule->Select(m_console, "select * from console;");
	for (auto& ser:m_console)
		ser->status = CONN_STATE::CLOSE;

}

void ServerInfoModule::OnGetMachineList(NetSocket * sock)
{
	Json::Value res;

	for (auto& ser:m_console)
	{
		Json::Value item;
		item["id"] = ser->id;
		item["ip"] = ser->ip;
		item["port"] = ser->post;
		item["name"] = ser->name;
		item["inline"] = ser->status == CONN_STATE::CONNECT;
		res.append(item);
	}

	auto msg = new HttpJsonMsg();
	msg->sock = sock->socket;
	msg->json = move(res.toStyledString());
	m_msgModule->SendMsg(LY_HTTP_LOGIC, 0, L_HL_GET_MACHINE_LIST, msg);
}