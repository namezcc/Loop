#include "stdafx.h"
#include "ScheduleModule.h"


ScheduleModule::ScheduleModule(BaseLayer* l):BaseModule(l)
{
	m_timerIndex = 0;
}


ScheduleModule::~ScheduleModule()
{
}

void ScheduleModule::Init()
{
}

void ScheduleModule::Execute()
{
	auto nt = GetSecend();

	for (auto it = m_timers.begin();it!= m_timers.end();)
	{
		if (nt >= it->second->begtime)
		{
			it->second->task(nt);
			it->second->begtime = nt + it->second->sec;
			if (it->second->count > 0)
				--it->second->count;
			if (it->second->count == 0)
				m_timers.erase(it++);
		}
		else
			it++;
	}
}

size_t ScheduleModule::GetTimerIndex()
{
	return ++m_timerIndex;
}