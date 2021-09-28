#include "TestProcessModule.h"
#include "ProcessModule.h"
#include "ScheduleModule.h"
#include "Common_mod.h"
#include "EventModule.h"
#include "LPFile.h"
#include "JsonHelp.h"

TestProcessModule::TestProcessModule(BaseLayer* l):BaseModule(l)
{

}

TestProcessModule::~TestProcessModule()
{
}

void TestProcessModule::Init()
{
	initServerName();

	m_processModule = GET_MODULE(ProcessModule);
	m_scheduleModule = GET_MODULE(ScheduleModule);
	m_event_mod = GET_MODULE(EventModule);

	COM_MOD_INIT;

	m_event_mod->AddEventCall(E_SERVER_CONNECT, BIND_EVENT(onServerConnect, SHARE<NetServer>&));
	m_event_mod->AddEventCall(E_SERVER_CLOSE, BIND_EVENT(onServerClose, SHARE<NetServer>&));
	

	m_msg_mod->AddMsgCall(N_ALL_SERVER_INFO, BIND_NETMSG(onGetAllServerInfo));
	m_msg_mod->AddMsgCall(N_SEND_SERVER_STATE, BIND_NETMSG(onServerState));
	m_msg_mod->AddMsgCall(N_WEB_VIEW_MACHINE, BIND_SHARE_CALL(onGetMachineServerInfo));
	m_msg_mod->AddAsynMsgCall(N_WEB_GET_SERVER_INFO, BIND_ASYN_CALL(onGetServerInfo));
	m_msg_mod->AddMsgCall(N_WEB_SERVER_OPT, BIND_NETMSG(onServerOpt));

}

void TestProcessModule::initServerName()
{
	JsonHelp jhelp2;
	if (jhelp2.ParseFile(LoopFile::GetRootPath().append("commonconf/Server.json")))
	{
		for (auto& v : jhelp2.GetDocument().GetArray())
			m_server_name[v["type"].GetInt()] = v["name"].GetString();
	}
}

void TestProcessModule::onServerConnect(SHARE<NetServer>& ser)
{
	if (ser->type == LOOP_MASTER)
	{
		m_trans_mod->SendToServer(ServerNode{ LOOP_MASTER,1 }, N_ALL_SERVER_INFO, NULL);
	}
	else
	{
		sendConnInfo(ser->type, ser->serid);
	}
}

void TestProcessModule::onServerClose(SHARE<NetServer>& ser)
{
	Int64Struct i64;

	i64.height32 = ser->type;
	i64.lower32 = ser->serid;

	m_server_state[i64.i64] = -1;
}

void TestProcessModule::onGetAllServerInfo(NetMsg * msg)
{
	m_all_server.clear();
	m_self_server.clear();

	auto p = msg->m_buff;
	auto num = p->readInt32();

	auto selfserverid = GetLayer()->GetServer()->serid;

	for (int32_t i = 0; i < num; i++)
	{
		ServerConfigInfo info = {};
		info.type = p->readInt32();
		info.server_id = p->readInt32();
		info.ip = p->readString();
		info.port = p->readInt32();

		auto machine = p->readInt32();

		if (machine == selfserverid)
			m_self_server.push_back(info);

		m_all_server[info.type].push_back(info);
	}


	auto& allser = m_trans_mod->getAllServer();

	for (auto& s:allser)
	{
		if (s.second->type != LOOP_MASTER)
		{
			sendConnInfo(s.second->type, s.second->serid);
		}
	}
}

void TestProcessModule::onServerState(NetMsg * msg)
{
	auto p = msg->m_buff;

	Int64Struct i64;

	i64.height32 = p->readInt32();
	i64.lower32 = p->readInt32();

	m_server_state[i64.i64] = p->readInt32();
}

void TestProcessModule::onGetMachineServerInfo(SHARE<BaseMsg>& msg)
{
	auto buf = LAYER_BUFF;

	buf->writeInt32((int32_t)m_server_state.size());

	Int64Struct i64;

	for (auto& i:m_server_state)
	{
		i64.i64 = i.first;

		buf->writeInt32(i64.height32);
		buf->writeInt32(i64.lower32);
		buf->writeInt32(i.second);
	}

	m_trans_mod->ResponseServerMsg(ServerNode{ LOOP_MASTER,1 }, msg, buf);
}

void TestProcessModule::onGetServerInfo(SHARE<BaseMsg>& msg, c_pull & pull, SHARE<BaseCoro>& coro)
{
	auto netmsg = dynamic_cast<NetMsg*>(msg->m_data);
	auto p = netmsg->m_buff;
	auto sertype = p->readInt32();
	auto serid = p->readInt32();

	if (m_trans_mod->GetServer(sertype,serid) == NULL)
	{
		auto buf = LAYER_BUFF;
		buf->writeInt32(0);
		m_net_mod->ResponseMsg(netmsg->socket, msg, buf);
		return;
	}

	auto resmsg = m_trans_mod->RequestServerAsynMsg(ServerNode{ (int8_t)sertype,(int16_t)serid }, N_GET_LINK_SERVER_INFO, NULL, pull, coro);
	{
		auto rp = resmsg->m_buff;

		std::map<int32_t, int32_t> link;
		auto num = rp->readInt32();
		Int32Struct i32;
		for (int32_t i = 0; i < num; i++)
		{
			i32.height16 = rp->readInt32();
			i32.lower16 = rp->readInt32();
			link[i32.i32] = 1;
		}

		auto conn = GetLayer()->GetLoopServer()->getConnectServer(sertype, serid, m_all_server);

		auto sendpack = LAYER_BUFF;
		sendpack->writeInt32((int32_t)conn.size());

		for (auto& c:conn)
		{
			sendpack->writeInt32(c.type);
			sendpack->writeInt32(c.server_id);
			sendpack->writeInt32(c.port);

			i32.height16 = c.type;
			i32.lower16 = c.server_id;

			sendpack->writeInt32(link[i32.i32]);
		}

		m_trans_mod->ResponseServerMsg(ServerNode{ LOOP_MASTER,1 }, msg, sendpack);
	}
}

void TestProcessModule::onServerOpt(NetMsg * msg)
{
	auto p = msg->m_buff;
	auto sert = p->readInt32();
	auto serid = p->readInt32();
	auto opt = p->readInt32();


	if (opt == 0)	//close
	{
		if (sert == 0 && serid == 0)
		{
			for (auto s:m_self_server)
				closeServer(s.type, s.server_id);
			
		}else
			closeServer(sert, serid);
	}
	else if (opt == 1) //open
	{
		if (sert == 0 && serid == 0)
		{
			for (auto s : m_self_server)
				openServer(s.type, s.server_id);
		}else
			openServer(sert, serid);
	}
	else if (opt == 2) //reopen
	{
		closeServer(sert, serid);
		openServer(sert, serid);
	}
}

std::string TestProcessModule::getServerPid(int32_t type, int32_t id)
{
	auto name = m_server_name[type];
	std::vector<std::string> arg;

#if PLATFORM == PLATFORM_WIN
	auto exec = LoopFile::GetExecutePath().append("batgetpid.bat");
	arg.push_back(name);
	arg.push_back(Loop::to_string(id));

#else
	auto exec = LoopFile::GetExecutePath().append("shgetpid.sh");
	arg.push_back(Loop::to_string(type));
	arg.push_back(Loop::to_string(id));
#endif

	auto res = m_processModule->runProcess(exec, arg);
	if (res.empty())
	{
		return res;
	}

#if PLATFORM != PLATFORM_WIN

	return res;
#else

	std::vector<std::string> strvec;
	Loop::Split(res, ",", strvec);
	if (strvec.size() < 2)
	{
		return "";
	}

	auto pid = strvec[1].substr(1, strvec[1].size()-2);
	return pid;
#endif
}

void TestProcessModule::closeServer(int32_t type, int32_t id)
{
	auto pid = getServerPid(type, id);
	if (pid.empty())
	{
		LP_INFO << "close server not open " << m_server_name[type] << " id:" << id;
		return;
	}

#if PLATFORM == PLATFORM_WIN
	auto exec = LoopFile::GetExecutePath().append("batkill.bat");
#else
	auto exec = LoopFile::GetExecutePath().append("shkill.sh");
#endif
	
	std::vector<std::string> arg;
	arg.push_back(pid);
	m_processModule->runProcess(exec, arg);
}

void TestProcessModule::openServer(int32_t type, int32_t id)
{
	auto pid = getServerPid(type, id);
	if (!pid.empty())
	{
		LP_INFO << "open server allready open " << m_server_name[type] << " id:" << id;
		return;
	}

#if PLATFORM == PLATFORM_WIN
	auto exec = LoopFile::GetExecutePath().append("batstart.bat");
#else
	auto exec = LoopFile::GetExecutePath().append("shstart.sh");
#endif
	std::vector<std::string> arg;
	arg.push_back(Loop::to_string(type));
	arg.push_back(Loop::to_string(id));
	arg.push_back(m_server_name[type]);
	m_processModule->runProcessAndDetach(exec, arg);
}

void TestProcessModule::sendConnInfo(int32_t type, int32_t id)
{
	if (m_all_server.empty())
		return;

	auto conn = GetLayer()->GetLoopServer()->getConnectServer(type, id, m_all_server);
	if (conn.empty())
		return;

	auto buf = LAYER_BUFF;
	buf->writeInt32((int32_t)conn.size());

	for (auto& s:conn)
	{
		buf->writeInt32(s.type);
		buf->writeInt32(s.server_id);
		buf->writeString(s.ip);
		buf->writeInt32(s.port);
	}

	m_trans_mod->SendToServer(ServerNode{ (int8_t)type,(int16_t)id }, N_CONN_SERVER_INFO, buf);
}
