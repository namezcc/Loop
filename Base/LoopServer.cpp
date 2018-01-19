#include "LoopServer.h"
#include "cmdline.h"

LoopServer::LoopServer()
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
	m_serverNode.type = stype;
	m_serverNode.serid = serid;
}

void LoopServer::Run()
{
	m_pool = SHARE<ThreadPool>(new ThreadPool(m_layers.size()));
	for (auto& l : m_layers)
	{
		l->SetServer(&m_serverNode);
		m_pool->Add_Task([&l]() {
			l->StartRun();
		});
	}
}