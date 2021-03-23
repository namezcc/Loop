#ifndef SCHEDULE_MODULE_H
#define SCHEDULE_MODULE_H
#include "BaseModule.h"
#include "TimerPlate.h"

struct Timer
{
	uint32_t index;
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


#define BIND_TIME(F) [this](int64_t&dt) { F(dt);}
	//���� ��λ
	uint32_t AddInterValTask(const TimeTask& nTask, const int& interval,const int& count = -1,const int& delay = 0)
	{
		//auto timer = GetLayer()->GetSharedLoop<Timer>();
		auto timer = NEW_SHARE(Timer);
		timer->count = count;
		timer->interval = interval;
		timer->task = nTask;
		timer->index = GetTimerIndex();
		timer->begtime = GetMilliSecend() + delay;
		m_timers[timer->index] = timer;
		return timer->index;
	}

	//@repeat -1 ����
	uint32_t AddTimePointTask(const TimeTask& nTask,const int& repeat, const int& sec=0, const int& min=-1, const int& hour=-1, const int& week=-1, const int& mday=-1)
	{
		//auto timer = GetLayer()->GetSharedLoop<Plate>();
		auto timer = NEW_SHARE(Plate);
		timer->rep = repeat;
		timer->mid = GetTimerIndex();
		timer->task = nTask;
		timer->plate[PLATE::P_SEC] = sec;
		timer->plate[PLATE::P_MIN] = min;
		timer->plate[PLATE::P_HOUR] = hour;
		timer->plate[PLATE::P_WEEK] = week;
		timer->plate[PLATE::P_MDAY] = mday;
		AddPlate(timer);
		return timer->mid;
	}

	void RemoveTimePointTask(const uint32_t& mid);
protected:
	virtual void Init() override;
	virtual void Execute() override;

	uint32_t GetTimerIndex();
	void AddPlate(SHARE<Plate>& plate);

private:

	uint32_t m_timerIndex;
	int64_t m_checkTime;
	unordered_map<uint32_t, SHARE<Timer>> m_timers;
	TimerPlate m_plate;
};

#endif