#include "LoopServer.h"
#include "cmdline.h"
#include "LogLayer.h"
#include "LPFile.h"
#include "DataDefine.h"
#include "JsonHelp.h"
#include "LPStringUtil.h"
#include "httpclient.h"
#include "ToolFunction.h"

enum ServerConnectType
{
	SCT_NONE,
	SCT_ALL,			//链接所有
	SCT_AVG_NUM,		//平均分配n个
	SCT_ID,				//固定id
};

LoopServer::LoopServer():m_over(false), m_stop(NULL), m_server_state(0), m_machine_id(0)
{
}


LoopServer::~LoopServer()
{
}

void LoopServer::InitServer(int argc, char** args)
{
	cmdline::parser param;

	//param.add<int>("port", 'p', "server port", true, 0, cmdline::range(1, 65535));
	param.add<int>("type", 't', "server type", true, 0);
	param.add<int>("id", 'n', "server port", true, 0);
	param.set_program_name("server");
	param.parse_check(argc, args);

	//m_port = param.get<int>("port");
	auto st = param.get<int>("type");
	auto serid = param.get<int>("id");
	Init(st, serid);
}

void LoopServer::setServerState(int32_t bit, bool err)
{
	if (err)
	{
		SET_BIT_1(m_server_state, bit);
	}
	else
	{
		SET_BIT_0(m_server_state, bit);
	}
}

void LoopServer::Init(const int& stype, const int& serid)
{
	m_server.serid = serid;
	m_server.type = stype;
	JsonHelp jhelp2;
	if (jhelp2.ParseFile(LoopFile::GetRootPath().append("commonconf/Server.json")))
	{
		for (auto& v : jhelp2.GetDocument().GetArray())
		{
			m_server_name[v["type"].GetInt()] = v["name"].GetString();
			m_server_type[v["name"].GetString()] = v["type"].GetInt();
		}
	}
	InitConfig();
	InitServerConfig();
	InitConnectRule();
}

void LoopServer::InitConfig()
{
	string file = LoopFile::GetRootPath();
	if(m_server.type== SERVER_TYPE::LOOP_MASTER)
		file.append("commonconf/Master.json");
	else if(m_server.type == SERVER_TYPE::LOOP_CONSOLE){
		file.append("commonconf/ServerConfig.json");
	}
	else
	{
		JsonHelp confjs;
		confjs.ParseFile(file.append("commonconf/Common.json"));
		Value cfv = confjs.GetDocument().GetObject();

		std::string confhost = cfv["confighost"].GetString();
		confhost.append("/serverInfo?id=%d&type=%d");

		char url[256] = {};
		sprintf(url, confhost.c_str(), m_server.serid, m_server.type);
		auto res = Single::GetInstence<HttpClient>()->requestUrl(url);

		if (res.empty())
		{
			printf("requestUrl getconfig error %s", url);
			exit(-1);
		}

		JsonHelp jhelp;
		if (!jhelp.ParseString(res))
			exit(-1);

		Value sroot = jhelp.GetDocument().GetObject();
		Value rt = sroot["server"].GetObject();

		m_config.name = m_server_name[m_server.type];
		m_config.group = rt["group"].GetInt();
		m_config.addr.ip = rt["ip"].GetString();
		m_config.addr.port = rt["port"].GetInt();
		m_config.addr.serid = m_server.serid;
		m_config.addr.type = m_server.type;
		m_port = m_config.addr.port;
		m_machine_id = rt["Machine"].GetInt();

		auto sql = rt["mysql"].GetString();
		if (strlen(sql) > 0)
		{
			std::vector<std::string> res;
			Loop::Split(sql, "|", res);

			if (res.size() < 5)
			{
				printf("mysql conf error %s\n", sql);
				exit(-1);
			}

			m_config.sql.ip = res[0];
			m_config.sql.port = Loop::Cvto<int>(res[1]);
			m_config.sql.db = res[2];
			m_config.sql.user = res[3];
			m_config.sql.pass = res[4];
		}

		auto redis = rt["redis"].GetString();
		if (strlen(redis) > 0)
		{
			std::vector<std::string> res;
			Loop::Split(redis, "|", res);

			if (res.size() < 3)
			{
				printf("redis conf error %s\n", redis);
				exit(-1);
			}

			m_config.redis.ip = res[0];
			m_config.redis.port = Loop::Cvto<int>(res[1]);
			m_config.redis.pass = res[2];
		}

		auto service = sroot["service"].GetArray();
		for (auto& sv:service)
		{
			ServerConfigInfo info = {};
			info.server_id = sv["id"].GetInt();
			info.ip = sv["ip"].GetString();
			info.port = sv["port"].GetInt();
			info.type = LOOP_SERVICE_FIND;
			m_all_server[LOOP_SERVICE_FIND].push_back(info);
		}
		return;
	}

	JsonHelp jhelp;
	if (!jhelp.ParseFile(file))
		exit(-1);

	Value root = jhelp.GetDocument().GetObject();
	Value config;
	if (m_server.type == SERVER_TYPE::LOOP_MASTER)
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

	if (config.HasMember("redis"))
	{
		m_config.redis.ip = config["redis"]["ip"].GetString();
		m_config.redis.port = config["redis"]["port"].GetInt();
		m_config.redis.pass = config["redis"]["pass"].GetString();
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
			if (type != LOOP_MASTER && type != LOOP_CONSOLE)
			{
				continue;
			}

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
	auto root = jhelp.GetDocument().GetArray();

	for (auto& v:root)
	{
		ConnRule r = {};
		r.group = v["group"].GetBool();
		r.server_type = m_server_type[v["server"].GetString()];
		r.to_server_type = m_server_type[v["to_server"].GetString()];
		r.conn_type = v["type"].GetInt();
		r.param = v["param"].GetInt();

		if (r.conn_type > 0)
			m_connect_rule.push_back(r);
	}
}

void LoopServer::InitLogLayer()
{
	LOG_LAYER->start(&m_server,this);
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
	if (stopServer())
	{
		bool over = true;
		for (size_t i = 0; i < m_layers.size(); i++)
		{
			if (!m_layers[i]->isOver())
			{
				over = false;
				break;
			}
		}
		if (over)
		{
			m_pool.reset();
			m_over = true;
		}
	}
	/*for (size_t i = 0; i < m_layers.size(); i++)
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
	}*/
}

std::vector<ServerConfigInfo> LoopServer::getConnectServer()
{
	std::vector<ServerConfigInfo> res;
	for (auto p:m_connect_rule)
	{
		if (p.server_type != m_server.type)
			continue;

		auto server = p.to_server_type;
		auto type = p.conn_type;
		auto param = p.param;
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

	/*if (m_server.type != LOOP_CONSOLE && m_server.type != LOOP_MASTER)
	{
		auto svec = m_all_server[LOOP_CONSOLE];
		for (auto m:svec)
		{
			if (m.server_id == m_machine_id)
			{
				res.push_back(m);
				break;
			}
		}
	}*/
	return res;
}
std::vector<ServerConfigInfo> LoopServer::getConnectServer(int32_t type, int32_t id, std::map<int32_t, std::vector<ServerConfigInfo>>& allser)
{
	std::vector<ServerConfigInfo> res;
	for (auto p : m_connect_rule)
	{
		if (p.server_type != type)
			continue;

		auto server = p.to_server_type;
		auto type = p.conn_type;
		auto param = p.param;
		auto svec = allser[server];
		auto msvec = allser[type];
		if (svec.empty())
			continue;

		if (type == SCT_ALL)
		{
			res.insert(res.end(), svec.begin(), svec.end());
		}
		else if (type == SCT_AVG_NUM)
		{
			auto to_snum = svec.size();
			auto my_snum = msvec.size();
			auto mindex = 0;

			for (size_t i = 0; i < msvec.size(); i++)
			{
				if (msvec[i].server_id == id)
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
			for (auto it : svec)
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
std::vector<ServerConfigInfo>& LoopServer::getServerInfo(int32_t type)
{
	return m_all_server[type];
}

std::string LoopServer::getServerAddrKey()
{
	char key[64];
	sprintf(key, "serveraddr:g%d:%s-%d", m_config.group, m_config.name.c_str(), m_config.addr.serid);
	return key;
}

std::string LoopServer::getServerAddrInfo()
{
	char key[64];
	auto ip = getLocalIp();
	sprintf(key, "%s:%d:%d:%s:%d",m_config.name.c_str(),m_server.type,m_server.serid, ip.c_str(), m_config.addr.port);
	return key;
}

void LoopServer::getConnectKey(std::vector<std::string>& connkey, std::vector<std::string>& watchkey)
{
	for (auto& p:m_connect_rule)
	{
		if (p.server_type != m_server.type)
			continue;

		std::string ckey = ":g";
		if (p.group)
			ckey.append(std::to_string(m_config.group));
		else
			ckey.append("*");

		ckey.append(":").append(m_server_name[p.to_server_type]);

		if (p.conn_type == SCT_ALL)
			ckey.append("*");
		else if (p.conn_type == SCT_ID)
			ckey.append(std::to_string(m_server.serid));
		else if (p.conn_type == SCT_AVG_NUM)
		{

		}
		connkey.push_back("serveraddr"+ckey);
		watchkey.push_back("watch" + ckey);
	}
}
std::set<std::string> LoopServer::getNoticKey()
{
	std::set<std::string> res;
	for (auto& p : m_connect_rule)
	{
		if (p.to_server_type != m_server.type)
			continue;

		std::string key = "watch:g";
		if (p.group)
			key.append(std::to_string(m_config.group));
		else
			key.append("*");

		key.append(":").append(m_config.name);

		if (p.conn_type == SCT_ALL)
			key.append("*");
		else if (p.conn_type == SCT_ID)
			key.append(std::to_string(m_server.serid));
		else if (p.conn_type == SCT_AVG_NUM)
		{

		}
		res.insert(key);
	}
	return res;
}
//void LoopServer::recycle(int32_t index, BaseData* msg)
//{
//	msg->recycleCheck();
//	m_recycle[index].recycle(msg);
//}