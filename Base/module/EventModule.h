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

struct EventParam
{
	virtual ~EventParam() 
	{}
};

template<typename T>
struct EventData:public EventParam
{
	T m_data;
};

template<typename T>
struct EventData<SHARE<T>> :public EventParam
{
	SHARE<T> m_data;
};

typedef std::function<void(EventParam*)> EventCall;

class LOOP_EXPORT EventModule:public BaseModule
{
public:
	EventModule(BaseLayer* l);
	~EventModule();

#define BIND_EVENT(F,T) [this](EventParam* arg){ F(dynamic_cast<EventData<typename std::decay<T>::type>*>(arg)->m_data);}

	void AddEventCall(const int32_t& ev, const EventCall& call)
	{
		m_events[ev].push_back(call);
	}

	template<typename T>
	void SendEvent(const int& ev, T&& arg)
	{
		auto it = m_events.find(ev);
		if (it == m_events.end())
			return;

		EventData<typename std::decay<T>::type> param;
		param.m_data = arg;
		for (auto& call : it->second)
			call(&param);
	}

	/*template<typename T,typename F>
	void AddEventCallBack(const int& ev,T&&t, F&&f)
	{
		auto call = AnyFuncBind::Bind(forward<F>(f),forward<T>(t));
		assert(call);
		auto callpter = new decltype(call)(call);
		m_events2[ev].push_back((void*)callpter);
	}

	template<typename... Args>
	void SendEvent2(const int& ev, Args&&... args)
	{
		auto it = m_events2.find(ev);
		if (it == m_events2.end())
			return;
		
		for (auto& call:it->second)
		{
			auto callpter = static_cast<FuncType<Args...>*>(call);
			callpter->operator()(forward<Args>(args)...);
		}
	}*/
protected:
	virtual void Init() override;
	virtual void Execute() override;

private:
	std::unordered_map<int32_t, std::list<EventCall>> m_events;
	//std::unordered_map<int32_t, std::list<void*>> m_events2;
};

#endif