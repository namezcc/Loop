#include "ServerInfoModule.h"
#include "MsgModule.h"
#include "MysqlModule.h"
#include "NetObjectModule.h"
#include "JsonHelp.h"
#include "ServerMsgDefine.h"
#include "TransMsgModule.h"

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
	m_trans_mos = GET_MODULE(TransMsgModule);
	
	//m_msgModule->AddMsgCallBack(L_HL_GET_MACHINE_LIST, this, &ServerInfoModule::OnGetMachineList);

	m_msgModule->AddMsgCall(N_ALL_SERVER_INFO, BIND_NETMSG(onGetAllServerInfo));
	

	m_msgModule->AddMsgCall(N_WEB_TEST, BIND_NETMSG(onWebTest));
	m_msgModule->AddMsgCall(N_WBE_REQUEST_1, BIND_NETMSG(onWebRequest));
	m_msgModule->AddAsynMsgCall(N_WEB_VIEW_MACHINE, BIND_ASYN_NETMSG(onWebGetMachineInfo));
	m_msgModule->AddAsynMsgCall(N_WEB_GET_SERVER_INFO, BIND_ASYN_NETMSG(onWebGetServerInfo));
	m_msgModule->AddMsgCall(N_WEB_SERVER_OPT, BIND_NETMSG(onWebServerOpt));

}

void ServerInfoModule::BeforExecute()
{
	//m_mysqlModule->InitTable<ServerInfo>("console");
	InitServers();
}

void ServerInfoModule::InitServers()
{
	/*m_mysqlModule->Select(m_console, "select * from console;");
	for (auto& ser:m_console)
		ser->status = CONN_STATE::CLOSE;*/

	std::vector<dbMachine> machine;
	m_mysqlModule->Select(machine, "select * from machine;");

	for (auto& m:machine)
	{
		m_machine[m.id] = m;
	}

	std::vector<MachineServer> server;
	m_mysqlModule->Select(server, "select * from server;");

	Int64Struct i64;

	for (auto& s:server)
	{
		i64.height32 = s.type;
		i64.lower32 = s.id;

		s.state = -1;

		m_server[s.machine][i64.i64] = s;
	}
}

void ServerInfoModule::OnGetMachineList(NetMsg * sock)
{
	//StringBuffer jbuff;
	//rapidjson::Writer<StringBuffer> jw(jbuff);
	//Document doc;
	//doc.SetArray();
	//auto& allcter = doc.GetAllocator();
	//for (auto& ser:m_console)
	//{
	//	Value item(rapidjson::kObjectType);
	//	item.AddMember("id", ser->id, allcter);
	//	item.AddMember("ip", ser->ip, allcter);
	//	item.AddMember("port", ser->port, allcter);
	//	item.AddMember("name", ser->name, allcter);
	//	item.AddMember("inline", ser->status == CONN_STATE::CONNECT, allcter);
	//	doc.PushBack(item, allcter);
	//}
	//doc.Accept(jw);
	//std::string s = jbuff.GetString();

	//auto msg = GET_LAYER_MSG(NetMsg);
	//msg->socket = sock->socket;
	///*msg->len = s.size();
	//msg->msg = new char[msg->len+1];
	//strcpy(msg->msg, s.c_str());*/
	//msg->push_front(GetLayer(), s.c_str(), (int32_t)s.size());
	//m_msgModule->SendMsg(LY_HTTP_LOGIC, 0, L_HL_GET_MACHINE_LIST, msg);
}

void ServerInfoModule::onGetAllServerInfo(NetMsg * msg)
{
	auto buf = LAYER_BUFF;

	auto begindex = buf->getOffect();
	buf->writeInt32(0);

	int32_t num = 0;

	for (auto& ms:m_server)
	{
		auto ip = m_machine[ms.first].ip;
		for (auto& s:ms.second)
		{
			buf->writeInt32(s.second.type);
			buf->writeInt32(s.second.id);
			buf->writeString(ip);
			buf->writeInt32(s.second.port);
			buf->writeInt32(ms.first);
			++num;
		}
	}

	auto endindex = buf->getOffect();
	buf->setOffect(begindex);
	buf->writeInt32(num);
	buf->setOffect(endindex);

	m_net_mod->SendNetMsg(msg->socket, N_ALL_SERVER_INFO, buf);
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

void ServerInfoModule::onWebGetMachineInfo(NetMsg * msg, c_pull & pull, SHARE<BaseCoro>& coro)
{
	auto pack = msg->m_buff;
	auto repid = pack->readInt32();
	auto id = pack->readInt32();

	if (m_trans_mos->GetServer(LOOP_CONSOLE, id) == NULL)
	{
		auto buf = GET_LAYER_MSG(BuffBlock);
		buf->writeInt32(repid);
		buf->writeInt32(0);
		buf->writeInt32(0);
		m_net_mod->SendNetMsg(msg->socket, N_WBE_ON_RESPONSE, buf);
		return;
	}
	
	auto gmsg = m_trans_mos->RequestServerAsynMsg(ServerNode{ LOOP_CONSOLE,(int16_t)id }, N_WEB_VIEW_MACHINE, NULL, pull, coro);
	{
		auto gpack = gmsg->m_buff;


		auto buf = LAYER_BUFF;
		buf->writeInt32(repid);
		buf->writeInt32(1);
		
		auto num = gpack->readInt32();
		Int64Struct i64;

		for (int32_t i = 0; i < num; i++)
		{
			i64.height32 = gpack->readInt32();
			i64.lower32 = gpack->readInt32();
			m_server[id][i64.i64].state = gpack->readInt32();
		}

		buf->writeInt32((int32_t)m_server[id].size());

		for (auto& s:m_server[id])
		{
			buf->writeInt32(s.second.type);
			buf->writeInt32(s.second.id);

			if (s.second.state < 0)
				buf->writeInt32(0);
			else
				buf->writeInt32(1);

			buf->writeInt32(s.second.state);
		}

		m_net_mod->SendNetMsg(msg->socket, N_WBE_ON_RESPONSE, buf);
	}
}

void ServerInfoModule::onWebGetServerInfo(NetMsg * msg, c_pull & pull, SHARE<BaseCoro>& coro)
{
	auto pack = msg->m_buff;
	auto repid = pack->readInt32();
	auto mid = pack->readInt32();
	auto type = pack->readInt32();
	auto serid = pack->readInt32();

	if (m_trans_mos->GetServer(LOOP_CONSOLE, mid) == NULL)
	{
		auto buf = GET_LAYER_MSG(BuffBlock);
		buf->writeInt32(repid);
		buf->writeInt32(0);
		m_net_mod->SendNetMsg(msg->socket, N_WBE_ON_RESPONSE, buf);
		return;
	}

	auto reqbuf = LAYER_BUFF;
	reqbuf->writeInt32(type);
	reqbuf->writeInt32(serid);

	auto gmsg = m_trans_mos->RequestServerAsynMsg(ServerNode{ LOOP_CONSOLE,(int16_t)mid }, N_WEB_GET_SERVER_INFO, reqbuf, pull, coro);
	{
		auto gpack = gmsg->m_buff;
		auto buf = LAYER_BUFF;
		buf->writeInt32(repid);

		int32_t buflen = 0;
		auto data = gpack->readBuff(buflen);
		buf->writeBuff(data, buflen);

		m_net_mod->SendNetMsg(msg->socket, N_WBE_ON_RESPONSE, buf);
	}
}

void ServerInfoModule::onWebServerOpt(NetMsg * msg)
{
	auto p = msg->m_buff;
	auto mid = p->readInt32();
	auto sertype = p->readInt32();
	auto serid = p->readInt32();
	auto opt = p->readInt32();

	if (sertype > 0 || serid > 0)
	{
		if (getServerInfo(mid, sertype, serid) == NULL)
			return;
	}

	auto buf = LAYER_BUFF;
	buf->writeInt32(sertype);
	buf->writeInt32(serid);
	buf->writeInt32(opt);

	m_trans_mos->SendToServer(SNODE(LOOP_CONSOLE, mid), N_WEB_SERVER_OPT, buf);
}

MachineServer * ServerInfoModule::getServerInfo(int32_t mid, int32_t type, int32_t id)
{
	auto its = m_server.find(mid);
	if (its == m_server.end())
		return NULL;

	Int64Struct i64(type,id);
	
	auto it = its->second.find(i64.i64);
	if (it == its->second.end())
		return NULL;
	return &it->second;
}
