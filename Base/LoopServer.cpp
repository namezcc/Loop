#include "LoopServer.h"
#include "cmdline.h"
#include "LogLayer.h"

LoopServer::LoopServer()
{
	m_factor.reset(Single::NewLocal<FactorManager>());
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
}

void LoopServer::InitLogLayer()
{
	auto l = CreateLayer<LogLayer>();

	for (size_t i = 0; i < m_layers.size()-1; i++)
		BuildPipe(l, m_layers[i].get());
}

void LoopServer::BuildPipe(BaseLayer * l1, BaseLayer * l2)
{
	auto p1 = m_factor->getLoopObj<PIPE>();
	auto p2 = m_factor->getLoopObj<PIPE>();
	l1->regPipe(l2->GetType(), p1, p2);
	l2->regPipe(l1->GetType(), p2, p1);
}

void LoopServer::Run()
{
	//´´½¨loglayer
	InitLogLayer();

	m_pool = SHARE<ThreadPool>(new ThreadPool(m_layers.size()));
	for (auto& l : m_layers)
	{
		l->SetServer(&m_server);
		m_pool->Add_Task([&l]() {
			l->StartRun();
		});
	}
}