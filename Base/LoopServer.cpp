#include "LoopServer.h"
#include "cmdline.h"
#include "LogLayer.h"
#include "json/json.h"
#include "LPFile.h"
#include "DataDefine.h"

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
}

void LoopServer::InitConfig()
{
	string file;
	LoopFile::GetRootPath(file);
	if(m_server.type== SERVER_TYPE::LOOP_MASTER)
		file.append("commonconf/Master.json");
	else if(m_server.type == SERVER_TYPE::LOOP_CONSOLE)
		file.append("commonconf/Console.json");
	else
		file.append("commonconf/ServerConfig.json");

	ifstream ifs;
	try
	{
		ifs.open(file);
		ifs.is_open();
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
	}

	//Json::Reader reader;
	Json::Value root;

	/*if (!reader.parse(ifs, root, false))
		cout << reader.getFormattedErrorMessages() << endl;*/

	Json::CharReaderBuilder readerBuilder;
	std::string err;
	if (!Json::parseFromStream(readerBuilder, ifs, &root, &err))
		cout << err << endl;

	Json::Value config;
	if (m_server.type == SERVER_TYPE::LOOP_MASTER || m_server.type == SERVER_TYPE::LOOP_CONSOLE)
		config = root;
	else
	{
		auto type = std::to_string(m_server.type);
		auto id = std::to_string(m_server.serid);
		if (root.isMember(type))
			if (root[type].isMember(id))
				config = root[type][id];
	}

	if (config.isMember("addr"))
	{
		m_config.addr.ip = config["addr"]["ip"].asString();
		m_config.addr.port = config["addr"]["port"].asInt();
		m_port = m_config.addr.port;
	}

	if (config.isMember("udpaddr"))
	{
		m_config.udpAddr.ip = config["udpaddr"]["ip"].asString();
		m_config.udpAddr.port = config["udpaddr"]["port"].asInt();
	}

	if (config.isMember("connect"))
	{
		for (auto& v:config["connect"])
		{
			NetServer s;
			s.ip = v["ip"].asString();
			s.port = v["port"].asInt();
			s.type = v["type"].asInt();
			s.serid = v["id"].asInt();
			m_config.connect.push_back(s);
		}
	}

	if (config.isMember("sql"))
	{
		m_config.sql.ip = config["sql"]["ip"].asString();
		m_config.sql.port = config["sql"]["port"].asInt();
		m_config.sql.db = config["sql"]["database"].asString();
		m_config.sql.user = config["sql"]["user"].asString();
		m_config.sql.pass = config["sql"]["pass"].asString();
		m_config.sql.dbGroup = config["sql"]["group"].asInt();
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
	m_msgPool = new MsgPool[m_layers.size()];
	m_recycle = new RecyclePool[m_layers.size()];
}

void LoopServer::BuildPipe(BaseLayer * l1, BaseLayer * l2)
{
	//auto p1 = Single::LocalInstance<FactorManager>()->getLoopObj<PIPE>();
	//auto p2 = Single::LocalInstance<FactorManager>()->getLoopObj<PIPE>();
	auto p1 = GET_LOOP(PIPE);
	auto p2 = GET_LOOP(PIPE);
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
		while (msg= pool->pop())
		{
			msg->recycleMsg();
			if (++num >= 5000)
				break;
		}
	}
}

void LoopServer::recycle(int32_t index, BaseData* msg)
{
	assert(msg->m_looplist);
	msg->recycleCheck();
	m_recycle[index].recycle(msg);
}