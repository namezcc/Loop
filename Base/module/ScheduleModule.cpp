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
	AddInterValTask(&m_plate, &TimerPlate::Run, 1000);
}

void ScheduleModule::Execute()
{
	if (m_timers.size() == 0)
		return;
	auto nt = GetMilliSecend();
	if (nt < m_checkTime)
		return;
	m_checkTime = nt + 100;

	for (auto it = m_timers.begin();it!= m_timers.end();)
	{
		if (nt >= it->second->begtime)
		{
			it->second->task(nt);
			it->second->begtime = nt + it->second->interval;
			if (it->second->count > 0)
				--it->second->count;
			if (it->second->count == 0)
				m_timers.erase(it++);
		}
		else
			it++;
	}
}

void ScheduleModule::RemoveTimePointTask(const size_t& mid)
{
	m_plate.RemoveTask(mid);
}

size_t ScheduleModule::GetTimerIndex()
{
	return ++m_timerIndex;
}