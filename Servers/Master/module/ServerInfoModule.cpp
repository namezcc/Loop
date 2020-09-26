#include "ServerInfoModule.h"
#include "MsgModule.h"
#include "MysqlModule.h"
#include "ReflectData.h"
#include "JsonHelp.h"

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

	m_msgModule->AddMsgCallBack(L_HL_GET_MACHINE_LIST, this, &ServerInfoModule::OnGetMachineList);

}

void ServerInfoModule::BeforExecute()
{
	m_mysqlModule->InitTable<ServerInfo>("console");
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
	StringBuffer jbuff;
	rapidjson::Writer<StringBuffer> jw(jbuff);
	Document doc;
	doc.SetArray();
	auto& allcter = doc.GetAllocator();
	for (auto& ser:m_console)
	{
		Value item(rapidjson::kObjectType);
		item.AddMember("id", ser->id, allcter);
		item.AddMember("ip", ser->ip, allcter);
		item.AddMember("port", ser->port, allcter);
		item.AddMember("name", ser->name, allcter);
		item.AddMember("inline", ser->status == CONN_STATE::CONNECT, allcter);
		doc.PushBack(item, allcter);
	}
	doc.Accept(jw);
	std::string s = jbuff.GetString();

	auto msg = GET_LAYER_MSG(NetMsg);
	msg->socket = sock->socket;
	/*msg->len = s.size();
	msg->msg = new char[msg->len+1];
	strcpy(msg->msg, s.c_str());*/
	msg->push_front(GetLayer(), s.c_str(), (int32_t)s.size());
	m_msgModule->SendMsg(LY_HTTP_LOGIC, 0, L_HL_GET_MACHINE_LIST, msg);
}