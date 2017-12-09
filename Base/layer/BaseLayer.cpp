#include "BaseLayer.h"
#include "BaseModule.h"
#include <thread>

int BaseLayer::LID = 0;

BaseLayer::~BaseLayer() {
	delete m_factor;
}

void BaseLayer::StartRun()
{
	init();
	for (auto& it : m_modules)
		it.second->Init();

	while (true)
	{
		for (auto& it : m_pipes)
			startRead(it.second);

		loop();
		for (auto& it : m_modules)
			it.second->Execute();

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}