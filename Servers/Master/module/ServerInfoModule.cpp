#include "ServerInfoModule.h"
#include "MsgModule.h"
#include "MysqlModule.h"
#include "NetObjectModule.h"
#include "ReflectData.h"
#include "JsonHelp.h"
#include "ServerMsgDefine.h"

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
	m_net_mod = GET_MODULE(NetObjectModule);
	
	//m_msgModule->AddMsgCallBack(L_HL_GET_MACHINE_LIST, this, &ServerInfoModule::OnGetMachineList);
	m_msgModule->AddMsgCall(N_WEB_TEST, BIND_NETMSG(onWebTest));
	m_msgModule->AddMsgCall(N_WBE_REQUEST_1, BIND_NETMSG(onWebRequest));

}

void ServerInfoModule::BeforExecute()
{
	//m_mysqlModule->InitTable<ServerInfo>("console");
	//InitServers();
}

void ServerInfoModule::InitServers()
{
	/*m_mysqlModule->Select(m_console, "select * from console;");
	for (auto& ser:m_console)
		ser->status = CONN_STATE::CLOSE;*/

}

void ServerInfoModule::OnGetMachineList(NetMsg * sock)
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

void ServerInfoModule::onWebTest(NetMsg * msg)
{
	auto pack = msg->m_buff;
	auto val = pack->readInt32();
	LP_INFO << "get web msg " << val;

	m_net_mod->AcceptConn(msg->socket);

	auto buf = GET_LAYER_MSG(BuffBlock);
	buf->writeInt32(222);
	buf->writeInt64(111);
	buf->writeString("test_string");
	m_net_mod->SendNetMsg(msg->socket, N_WEB_TEST2, buf);
}

void ServerInfoModule::onWebRequest(NetMsg * msg)
{
	auto pack = msg->m_buff;
	auto reponeid = pack->readInt32();

	LP_INFO << "get requst id:" << reponeid;

	auto buf = GET_LAYER_MSG(BuffBlock);
	buf->writeInt32(reponeid);
	buf->writeString("test_stringaaaaaa");
	m_net_mod->SendNetMsg(msg->socket, N_WBE_ON_RESPONSE, buf);
}
