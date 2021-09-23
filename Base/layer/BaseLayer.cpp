#include "BaseLayer.h"
#include "BaseModule.h"
#include <thread>

BaseLayer::~BaseLayer() {
}

void BaseLayer::StartRun()
{
	auto defltype = 0;
	auto deflid = 0;
	GetDefaultTrans(defltype, deflid);

	auto it = m_pipes.find(defltype);
	if (it != m_pipes.end())
	{
		if (deflid < it->second.size())
			m_defPipe = &it->second[deflid];
	}

	init();
	for (auto& it : m_modules)
		it.second->Init();

	afterInit();
	for (auto& it : m_modules)
		it.second->AfterInit();

	for (auto& it : m_modules)
		it.second->BeforExecute();

	while (true)
	{
		for(auto& itv: m_pipes)
			for (auto& it : itv.second)
				startRead(it);

		loop();
		for (auto& it : m_modules)
			it.second->Execute();

		if (m_server->stopServer())
		{
			bool over = true;
			for (auto& it : m_modules)
			{
				if (!it.second->isOver())
				{
					over = false;
					break;
				}
			}
			if (over)
			{
				m_over = true;
				break;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void BaseLayer::RecycleLayerMsg(BaseData* msg)
{
	msg->recycleMsg();
	//m_server->recycle(m_lsindex,msg);
	//msg->recycleCheck();
	//msg->recycleMsg();
}