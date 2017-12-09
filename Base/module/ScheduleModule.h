#ifndef SCHEDULE_MODULE_H
#define SCHEDULE_MODULE_H
#include "BaseModule.h"

enum DAY_TYPE
{
	Y_DAY,
	M_DAT,
	W_DAY,
	A_DAY,
};

enum TIME_TYPE
{
	TIME_POINT,
	TIME_INTERVAL,
};

typedef std::function<void(int64_t)> TimeTask;

struct Timer
{
	int index;
	int timeType;
	int count;
	int dayType;
	int day;
	int hour;
	int min;
	int sec;
	int64_t begtime;
	TimeTask task;
};

class ScheduleModule:public BaseModule
{
public:
	ScheduleModule(BaseLayer* l);
	~ScheduleModule();

	template<typename T,typename F>
	size_t AddInterValTask(T&&t, F&&f, const int& interval,const int& count = -1,const int& delay = 0)
	{
		auto timer = GetLayer()->GetLoopObj<Timer>();
		timer->count = count;
		timer->timeType = TIME_INTERVAL;
		timer->sec = interval;
		timer->task = move(bind(forward<F>(f),forward<T>(t),placeholders::_1));
		timer->index = GetTimerIndex();
		timer->begtime = GetSecend() + delay;
		m_timers[timer->index] = timer;
		return timer->index;
	}
protected:
	virtual void Init() override;
	virtual void Execute() override;

	size_t GetTimerIndex();

private:

	size_t m_timerIndex;
	unordered_map<size_t, Timer*> m_timers;
};

#endif