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

	//���� ��λ
	template<typename T>
	size_t AddInterValTask(T*&&t,void(T::*&&f)(int64_t&), const int& interval,const int& count = -1,const int& delay = 0)
	{
		auto timer = GetLayer()->GetSharedLoop<Timer>();
		timer->count = count;
		timer->interval = interval;
		timer->task = std::move(std::bind(f,t,placeholders::_1));
		timer->index = GetTimerIndex();
		timer->begtime = GetMilliSecend() + delay;
		m_timers[timer->index] = timer;
		return timer->index;
	}

	//@repeat -1 ����
	template<typename T>
	size_t AddTimePointTask(T*&&t, void(T::*&&f)(int64_t&),const int& repeat, const int& sec=0, const int& min=-1, const int& hour=-1, const int& week=-1, const int& mday=-1)
	{
		auto timer = GetLayer()->GetSharedLoop<Plate>();
		timer->rep = repeat;
		timer->mid = GetTimerIndex();
		timer->task = std::move(std::bind(f, t, placeholders::_1));
		timer->plate[PLATE::P_SEC] = sec;
		timer->plate[PLATE::P_MIN] = min;
		timer->plate[PLATE::P_HOUR] = hour;
		timer->plate[PLATE::P_WEEK] = week;
		timer->plate[PLATE::P_MDAY] = mday;
		AddPlate(timer);
		return timer->mid;
	}

	void RemoveTimePointTask(const size_t& mid);
protected:
	virtual void Init() override;
	virtual void Execute() override;

	size_t GetTimerIndex();
	void AddPlate(SHARE<Plate>& plate);

private:

	size_t m_timerIndex;
	int64_t m_checkTime;
	unordered_map<size_t, SHARE<Timer>> m_timers;
	TimerPlate m_plate;
};

#endif