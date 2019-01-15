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
	AddInterValTask([this](int64_t&dt) { m_plate.Run(dt); } , 1000);
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

void ScheduleModule::RemoveTimePointTask(const uint32_t& mid)
{
	m_plate.RemoveTask(mid);
}

uint32_t ScheduleModule::GetTimerIndex()
{
	return ++m_timerIndex;
}

void ScheduleModule::AddPlate(SHARE<Plate>& plate)
{
	m_plate.AddPlate(plate);
}