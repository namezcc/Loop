#include "EventModule.h"


EventModule::EventModule(BaseLayer* l):BaseModule(l)
{
}


EventModule::~EventModule()
{
	/*for (auto& it:m_events)
	{
		for (auto& call:it.second)
		{
			delete call;
		}
	}*/
}

void EventModule::Init()
{
}

void EventModule::Execute()
{
}
