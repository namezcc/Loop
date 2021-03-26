#include "LoopServer.h"
#include "cmdline.h"
#include "LogLayer.h"
#include "LPFile.h"
#include "DataDefine.h"
#include "JsonHelp.h"
#include "LPStringUtil.h"

enum ServerConnectType
{
	SCT_NONE,
	SCT_ALL,			//链接所有
	SCT_AVG_NUM,		//平均分配n个
	SCT_ID,				//固定id
};

LoopServer::LoopServer():m_over(false)
{
}


LoopServer::~LoopServer()
{
}

void LoopServer::InitServer(int argc, char** args)
{
	cmdline::parser param;

	param.add<int>("port", 'p', "server port", true, 0, cmdline::range(1, 65535));
	param.add<int>("type", 't', "server type", true, 0);
	param.add<int>("id", 'n', "server port", true, 0);
	param.set_program_name("server");
	param.parse_check(argc, args);

	m_port = param.get<int>("port");
	auto st = param.get<int>("type");
	auto serid = param.get<int>("id");
	Init(st, serid);
}

void LoopServer::Init(const int& stype, const int& serid)
{
	m_server.serid = serid;
	m_server.type = stype;
	InitConfig();
	InitServerConfig();
	InitConnectRule();
}

void LoopServer::InitConfig()
{
	string file = LoopFile::GetRootPath();
	if(m_server.type== SERVER_TYPE::LOOP_MASTER)
		file.append("commonconf/Master.json");
	else if(m_server.type == SERVER_TYPE::LOOP_CONSOLE)
		file.append("commonconf/Console.json");
	else
		file.append("commonconf/ServerConfig.json");

	JsonHelp jhelp;
	if (!jhelp.ParseFile(file))
		exit(-1);

	Value root = jhelp.GetDocument().GetObject();
	Value config;
	if (m_server.type == SERVER_TYPE::LOOP_MASTER || m_server.type == SERVER_TYPE::LOOP_CONSOLE)
		config = root;
	else
	{
		auto type = std::to_string(m_server.type);
		auto id = std::to_string(m_server.serid);
		if (root.HasMember(type.c_str()))
			if (root[type.c_str()].HasMember(id.c_str()))
				config = root[type.c_str()][id.c_str()];
	}

	if (config.HasMember("addr"))
	{
		m_config.addr.ip = config["addr"]["ip"].GetString();
		m_config.addr.port = config["addr"]["port"].GetInt();
		m_port = m_config.addr.port;
	}

	if (config.HasMember("udpaddr"))
	{
		m_config.udpAddr.ip = config["udpaddr"]["ip"].GetString();
		m_config.udpAddr.port = config["udpaddr"]["port"].GetInt();
	}

	if (config.HasMember("sql"))
	{
		m_config.sql.ip = config["sql"]["ip"].GetString();
		m_config.sql.port = config["sql"]["port"].GetInt();
		m_config.sql.db = config["sql"]["database"].GetString();
		m_config.sql.user = config["sql"]["user"].GetString();
		m_config.sql.pass = config["sql"]["pass"].GetString();
		m_config.sql.dbGroup = config["sql"]["group"].GetInt();
	}
}

void LoopServer::InitServerConfig()
{
	string file = LoopFile::GetRootPath();
	file.append("commonconf/ServerConfig.json");
	JsonHelp jhelp;
	if (!jhelp.ParseFile(file))
		exit(-1);
	Value root = jhelp.GetDocument().GetObject();

	for (auto it=root.MemberBegin();it!=root.MemberEnd();++it)
	{
		if (it->value.IsObject())
		{
			auto type = Loop::Cvto<int>(it->name.GetString());
			auto& vec = m_all_server[type];

			for (auto its=it->value.MemberBegin();its != it->value.MemberEnd();++its)
			{
				ServerConfigInfo info = {};
				info.server_id = Loop::Cvto<int>(its->name.GetString());
				info.ip = its->value["addr"]["ip"].GetString();
				info.port = its->value["addr"]["port"].GetInt();
				info.type = type;
				vec.push_back(info);
			}
		}
	}
}

void LoopServer::InitConnectRule()
{
	string file = LoopFile::GetRootPath();
	file.append("commonconf/connect_rule.json");
	JsonHelp jhelp;
	if (!jhelp.ParseFile(file))
		exit(-1);
	auto& root = jhelp.GetDocument().GetArray();

	for (auto& v:root)
	{
		auto server_type = v["server"].GetInt();

		if (server_type == m_server.type)
		{
			auto toserver = v["to_server"].GetInt();
			auto conn_type = v["type"].GetInt();
			auto param = v["param"].GetInt();
			m_connect_rule[toserver] = std::make_pair(conn_type, param);
		}
	}
}

void LoopServer::InitLogLayer()
{
	auto l = CreateLayer<LogLayer>();

	for (size_t i = 0; i < m_layers.size()-1; i++)
		BuildPipe(l, m_layers[i].get());
}

void LoopServer::InitMsgPool()
{
	m_recycle = new RecyclePool[m_layers.size()];
}

void LoopServer::BuildPipe(BaseLayer * l1, BaseLayer * l2)
{
	auto p1 = new PIPE();
	auto p2 = new PIPE();
	l1->regPipe(l2->GetType(), p1, p2);
	l2->regPipe(l1->GetType(), p2, p1);
}

void LoopServer::Run()
{
	//����loglayer
	InitLogLayer();
	InitMsgPool();
	m_pool = SHARE<ThreadPool>(new ThreadPool(m_layers.size()));
	for (auto& l : m_layers)
	{
		l->SetServer(&m_server);
		l->SetLoopServer(this);
		m_pool->Add_Task([&l]() {
			l->StartRun();
		});
	}

	while (!m_over)
	{
		Loop();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void LoopServer::Loop()
{
	for (size_t i = 0; i < m_layers.size(); i++)
	{
		BaseData* msg = NULL;
		auto pool = m_recycle + i;
		int num = 0;
		while (msg = pool->pop())
		{
			msg->recycleMsg();
			if (++num >= 5000)
				break;
		}
	}
}

std::vector<ServerConfigInfo> LoopServer::getConnectServer()
{
	std::vector<ServerConfigInfo> res;
	for (auto p:m_connect_rule)
	{
		auto server = p.first;
		auto type = p.second.first;
		auto param = p.second.second;
		auto svec = m_all_server[server];
		auto msvec = m_all_server[m_server.type];
		if (svec.empty())
			continue;

		if (type == SCT_ALL)
		{
			res.insert(res.end(), svec.begin(),svec.end());
		}
		else if (type == SCT_AVG_NUM)
		{
			auto to_snum = svec.size();
			auto my_snum = msvec.size();
			auto mindex = 0;

			for (size_t i = 0; i < msvec.size(); i++)
			{
				if (msvec[i].server_id == m_server.serid) 
					mindex = (int)i;
			}

			auto to_index = mindex * to_snum / my_snum;
			if (to_index >= to_snum) to_index = 0;

			if (param > to_snum) param = (int32_t)to_snum;

			for (size_t i = 0; i < param; i++)
			{
				res.push_back(svec[to_index]);
				if ((++to_index) >= to_snum)
					to_index = 0;
			}
		}
		else if (type == SCT_ID)
		{
			for (auto it:svec)
			{
				if (it.server_id == param)
				{
					res.push_back(it);
					break;
				}
			}
		}
	}
	return res;
}
void LoopServer::recycle(int32_t index, BaseData* msg)
{
	msg->recycleCheck();
	m_recycle[index].recycle(msg);
}