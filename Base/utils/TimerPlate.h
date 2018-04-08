#ifndef TIMER_PLATE_H
#define TIMER_PLATE_H

#include <functional>
#include <list>
#include <map>
#include <time.h>
#include <memory>
#include "Define.h"

using namespace std;

#if PLATFORM == PLATFORM_WIN
#define localtime_s localtime_s
#else
#define localtime_s(tm,tt) localtime_r(tt,tm)
#endif

typedef std::function<void(int64_t&)> TimeTask;

enum PLATE
{
	P_SEC,
	P_MIN,
	P_HOUR,
	P_WEEK,
	P_MDAY,
	P_END,
};

struct Plate
{
	size_t mid;
	char plate[P_END];
	char rep;
	TimeTask task;
};

class TimerPlate
{
public:
	TimerPlate() 
	{
		auto t = time(NULL);
		localtime_s(&m_date, &t);
	};
	~TimerPlate();

	void Run(int64_t& nt);
	void AddPlate(SHARE<Plate>& t);
	void RemoveTask(const size_t& mid);
protected:
	void AddTimer(SHARE<Plate>& t);

	list<SHARE<Plate>>& GetPlate(int pt, int n);
	void AddToPlate(int pt, SHARE<Plate>& tn);
	void RunPlate(int pt, int n,int64_t& nt);
	void RunTask(list<SHARE<Plate>>& tl,int64_t& nt);
	
private:
	list<SHARE<Plate>> m_sec[60];
	list<SHARE<Plate>> m_min[60];
	list<SHARE<Plate>> m_hour[24];
	list<SHARE<Plate>> m_week[7];
	list<SHARE<Plate>> m_day[31];
	tm m_date;

	map<size_t, SHARE<Plate>> m_timers;
};

#endif