#include "LoopServer.h"


LoopServer::LoopServer()
{
}


LoopServer::~LoopServer()
{
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