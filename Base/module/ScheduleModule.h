#ifndef SCHEDULE_MODULE_H
#define SCHEDULE_MODULE_H
#include "BaseModule.h"
#include "TimerPlate.h"

struct Timer
{
	int index;
	int count;
	int interval;
	int64_t begtime;
	TimeTask task;
};

class LOOP_EXPORT ScheduleModule:public BaseModule
{
public:
	ScheduleModule(BaseLayer* l);
	~ScheduleModule();

	//∫¡√Î µ•Œª
	template<typename T,typename F>
	size_t AddInterValTask(T&&t, F&&f, const int& interval,const int& count = -1,const int& delay = 0)
	{
		auto timer = GetLayer()->GetSharedLoop<Timer>();
		timer->count = count;
		timer->interval = interval;
		timer->task = move(bind(forward<F>(f),forward<T>(t),placeholders::_1));
		timer->index = GetTimerIndex();
		timer->begtime = GetMilliSecend() + delay;
		m_timers[timer->index] = timer;
		return timer->index;
	}

	template<typename T,typename F>
	size_t AddTimePointTask(T&&t,F&&f,const int& repeat, const int& sec, const int& min=-1, const int& hour=-1, const int& week=-1, const int& mday=-1)
	{
		auto timer = GetLayer()->GetSharedLoop<Plate>();
		timer->rep = repeat;
		timer->mid = GetTimerIndex();
		timer->task = AnyFuncBind::Bind(forward<F>(f), forward<T>(t));
		timer->plate[PLATE::P_SEC] = sec;
		timer->plate[PLATE::P_MIN] = min;
		timer->plate[PLATE::P_HOUR] = hour;
		timer->plate[PLATE::P_WEEK] = week;
		timer->plate[PLATE::P_MDAY] = mday;
		m_plate.AddPlate(timer);
		return timer->mid;
	}

	void RemoveTimePointTask(const size_t& mid);
protected:
	virtual void Init() override;
	virtual void Execute() override;

	size_t GetTimerIndex();

private:

	size_t m_timerIndex;
	int64_t m_checkTime;
	unordered_map<size_t, SHARE<Timer>> m_timers;
	TimerPlate m_plate;
};

#endif