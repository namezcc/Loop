#ifndef EVENT_MODULE_H
#define EVENT_MODULE_H
#include "BaseModule.h"

template<typename... Args>
struct Event
{
	using FuncType = function<void(Args...)>;
};

template<typename... Args>
using FuncType = typename Event<Args...>::FuncType;

class EventModule:public BaseModule
{
public:
	EventModule(BaseLayer* l);
	~EventModule();

	template<typename T,typename F>
	void AddEventCallBack(const int& ev,T&&t, F&&f)
	{
		auto call = AnyFuncBind::Bind(forward<F>(f),forward<T>(t));
		auto callpter = new decltype(call)(call);
		auto it = m_events.find(ev);
		if (it == m_events.end())
		{
			vector<void*> calls;
			calls.push_back((void*)callpter);
			m_events[ev] = move(calls);
		}
		else
			it->second.push_back((void*)callpter);
	}

	template<typename... Args>
	void SendEvent(const int& ev, Args... args)
	{
		auto it = m_events.find(ev);
		if (it == m_events.end())
			return;
		
		for (auto& call:it->second)
		{
			auto callpter = static_cast<FuncType<int, Args...>*>(call);
			callpter->operator()(ev, forward<Args>(args)...);
		}
	}
protected:
	virtual void Init() override;
	virtual void Execute() override;

private:
	std::unordered_map<int, vector<void*>> m_events;
};

#endif