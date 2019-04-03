#include "TimerPlate.h"

TimerPlate::~TimerPlate()
{
}

void TimerPlate::AddPlate(SHARE<Plate>& t)
{
	m_timers[t->mid] = t;
	AddTimer(t);
}

void TimerPlate::AddTimer(SHARE<Plate>& t)
{
	if (t->plate[P_MDAY] >= 0 && (m_date.tm_mday != t->plate[P_MDAY] || t->plate[P_HOUR]<m_date.tm_hour))
		AddToPlate(P_MDAY, t);
	else if(t->plate[P_WEEK] >= 0 && (m_date.tm_wday != t->plate[P_WEEK] || t->plate[P_HOUR]<m_date.tm_hour))
		AddToPlate(P_WEEK, t);
	else if (t->plate[P_HOUR] >= 0 && (m_date.tm_hour != t->plate[P_HOUR] || t->plate[P_MIN]<m_date.tm_min))
		AddToPlate(P_HOUR, t);
	else if (t->plate[P_MIN] >= 0 && (m_date.tm_min != t->plate[P_MIN] || t->plate[P_SEC]<m_date.tm_sec))
		AddToPlate(P_MIN, t);
	else if (t->plate[P_SEC] >= 0)
		AddToPlate(P_SEC, t);
}

void TimerPlate::AddNextTimer(SHARE<Plate>& t)
{
	if (t->plate[P_MDAY] >= 0)
		AddToPlate(P_MDAY, t);
	else if (t->plate[P_WEEK] >= 0)
		AddToPlate(P_WEEK, t);
	else if (t->plate[P_HOUR] >= 0)
		AddToPlate(P_HOUR, t);
	else if (t->plate[P_MIN] >= 0)
		AddToPlate(P_MIN, t);
	else if (t->plate[P_SEC] >= 0)
		AddToPlate(P_SEC, t);
}

std::vector<SHARE<Plate>>* TimerPlate::GetPlate(int pt, int n)
{
	if (pt == P_SEC)
		return &m_sec[n];
	else if(pt == P_MIN)
		return &m_min[n];
	else if (pt == P_HOUR)
		return &m_hour[n];
	else if (pt == P_WEEK)
		return &m_week[n];
	else
		return &m_day[n-1];
}

void TimerPlate::AddToPlate(int pt, SHARE<Plate>& tn)
{
	auto pl = GetPlate(pt, tn->plate[pt]);
	pl->push_back(tn);
}

void TimerPlate::RunPlate(int pt, int n, int64_t& nt)
{
	auto pl = GetPlate(pt, n);
	if (pl->size() <= 0)
		return;
	switch (pt)
	{
	case P_SEC:
	{
		//auto tml = std::move(pl);
		std::vector<SHARE<Plate>> tmp = *pl;
		pl->clear();
		RunTask(tmp,nt);
		return;
	}
		break;
	case P_MIN:
		for (auto& t:*pl)
		{
			if (t->plate[P_SEC] >= 0)
				AddToPlate(P_SEC, t);
		}
		break;
	case P_HOUR:
		for (auto& t : *pl)
		{
			if (t->plate[P_MIN] >= 0)
				AddToPlate(P_MIN, t);
		}
		break;
	case P_WEEK:
		for (auto& t : *pl)
		{
			if (t->plate[P_HOUR] >= 0)
				AddToPlate(P_HOUR, t);
		}
		break;
	case P_MDAY:
		for (auto& t : *pl)
		{
			if (t->plate[P_HOUR] >= 0)
				AddToPlate(P_HOUR, t);
		}
		break;
	}
	pl->clear();
}

void TimerPlate::RunTask(std::vector<SHARE<Plate>>& tl, int64_t& nt)
{
	for (auto& t:tl)
	{
		if (t->rep > 0)
		{
			t->task(nt);
			--t->rep;
		}
		else if (t->rep < 0)
			t->task(nt);

		if(t->rep==0)
			m_timers.erase(t->mid);
		else
			AddNextTimer(t);
	}
}

void TimerPlate::Run(int64_t& nt)
{
	auto t = nt/1000;
	tm tmt;
	localtime_s(&tmt,&t);
	auto tmp = m_date;
	localtime_s(&m_date, &t);
	if (tmt.tm_sec != tmp.tm_sec)
	{
		if (tmt.tm_min != tmp.tm_min)
		{
			if (tmt.tm_hour != tmp.tm_hour)
			{
				if (tmt.tm_wday != tmp.tm_wday)
					RunPlate(P_WEEK, tmt.tm_wday,nt);
				if (tmt.tm_mday != tmp.tm_mday)
					RunPlate(P_MDAY, tmt.tm_mday,nt);
				RunPlate(P_HOUR, tmt.tm_hour, nt);
			}
			RunPlate(P_MIN, tmt.tm_min, nt);
		}
		int sec = tmp.tm_sec;
		for (size_t i = 1; i < 60; i++)
		{
			++sec;
			if (sec >= 60)
				sec = 0;
			if (sec == tmt.tm_sec)
			{
				RunPlate(P_SEC, tmt.tm_sec, nt);
				break;
			}else
				RunPlate(P_SEC, tmt.tm_sec, nt);
		}
	}
}

void TimerPlate::RemoveTask(const size_t& mid)
{
	auto it = m_timers.find(mid);
	if (it != m_timers.end())
		it->second->rep = 0;
}