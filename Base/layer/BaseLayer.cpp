#include "BaseLayer.h"
#include "BaseModule.h"
#include <thread>

BaseLayer::~BaseLayer() {
}

void BaseLayer::StartRun()
{
	init();
	for (auto& it : m_modules)
		it.second->Init();

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

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}