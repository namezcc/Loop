#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H
#include "Utils.h"
#include <stdint.h>
#include <functional>
#include <unordered_map>
#include <vector>
#include <set>
#include <memory>
#include "FactorManager.h"

#include "protoPB/client/object.pb.h"

struct BasePropertyCall
{
};

template<typename T, typename V>
struct PropertyCall :public BasePropertyCall
{
	typedef std::function<void(T, int32_t, V, V)> procall;
	PropertyCall(const procall& _call):m_func(_call)
	{
	}
	procall m_func;
};


struct PropertyEvent
{
	std::unordered_map<size_t, std::unordered_map<int32_t, std::list<std::shared_ptr<BasePropertyCall>>>> m_procall;

	template<typename T, typename F>
	void AddPropertyEvent(T&&t, int32_t pid, F&&f)
	{
		BasePropertyCall* call = new PropertyCall<typename FuncArgsType<F>::arg1, typename FuncArgsType<F>::arg3>(ANY_BIND(t, f));
		m_procall[typeid(PointType<typename FuncArgsType<F>::arg1>::type).hash_code()][pid].push_back(std::shared_ptr<BasePropertyCall>(call));
	}

	template<typename T, typename V>
	void SendEvent(T&&t, int32_t pid,const V& v1,const V& v2)
	{
		auto it = m_procall.find(typeid(PointType<T>::type).hash_code());
		if (it == m_procall.end())
			return;
		auto itf = it->second.find(pid);
		if (itf == it->second.end())
			return;
		for (auto& f : itf->second)
		{
			auto func = static_cast<PropertyCall<T, V>*>(f.get());
			func->m_func(t, pid, v1, v2);
		}
	}
};


struct BaseProperty
{
	enum
	{
		UP_DB = 1,
		UP_PRIVATE = 2,
		UP_PUBLIC = 4,
		UP_EVENT = 8,
	};

	void SetFlag(int32_t _flag)
	{
		flag = _flag;
	}

	bool UpDB() { return flag & UP_DB; }
	bool UpPrivate() { return flag & UP_PRIVATE; }
	bool UpPublic() { return flag & UP_PUBLIC; }
	bool UpEvent() { return flag & UP_EVENT; }

	int32_t flag;
};

template<typename T>
struct Property :public BaseProperty
{
	T data;
};


class GameObject:public LoopObject
{
public:
	GameObject() :m_event(NULL)
	{}

	virtual ~GameObject()
	{
		m_props.clear();
	}

	void SetEvent(PropertyEvent* nEvent)
	{
		m_event = nEvent;
	}

	void ParsePB(LPMsg::GameObject& pb)
	{
		for (int32_t i = 0; i < m_props.size(); i++)
		{
			if (m_props[i]->UpPrivate())
			{
				auto pfunc = GetPbFunc(i);
				if (!pfunc)
					continue;

				auto propb = pb.add_prolist();
				propb->set_proname(GetProName(i));
				pfunc(m_props[i].get(), *propb);
			}
		}
	}

	void Clear()
	{
		m_change.clear();
	}
	// 通过 LoopObject 继承
	virtual void init(FactorManager * fm) override;
	virtual void recycle(FactorManager * fm) override;
protected:
	typedef void (*PbFunc)(BaseProperty*, LPMsg::property&);

	template<typename T>
	void AddPropery(const int32_t& flag)
	{
		//auto pro = Single::LocalInstance<FactorManager>()->GetSharedLoop<Property<T>>();
		auto pro = GET_SHARE(Property<T>);
		pro->SetFlag(flag);
		m_props.push_back(pro);
	}

	template<typename T, typename V>
	void SetValue(int32_t pid,const V& v)
	{
		auto pro = static_cast<Property<V>*>(m_props[pid].get());
		auto old = pro->data;
		pro->data = v;
		if (pro->UpEvent() && m_event)
			m_event->SendEvent((T*)this, pid, old, v);
		m_change.insert(pid);
	}

	template<typename T>
	T GetValue(int32_t pid)
	{
		auto pro = static_cast<Property<T>*>(m_props[pid].get());
		return pro->data;
	}

	virtual PbFunc GetPbFunc(const int32_t& pid) = 0;
	virtual std::string GetProName(const int32_t& pid) = 0;
	virtual void CopySqlData() = 0;
	
private:
	std::vector<std::shared_ptr<BaseProperty>> m_props;
	std::set<int32_t> m_change;
	PropertyEvent* m_event;
};

enum PROPERTY_TYPE_ID
{
	PTI_INT32,
	PTI_INT64,
	PTI_STRING,
	PTI_FLOAT,
};

template<typename T>
struct PropertyID;

#define SET_PROPERTY_ID(T,ID)	\
template<>	\
struct PropertyID<T>:public std::integral_constant<int32_t,ID>{};

SET_PROPERTY_ID(bool,PTI_INT32)
SET_PROPERTY_ID(int8_t, PTI_INT32)
SET_PROPERTY_ID(int16_t, PTI_INT32)
SET_PROPERTY_ID(int32_t, PTI_INT32)
SET_PROPERTY_ID(uint8_t, PTI_INT32)
SET_PROPERTY_ID(uint16_t, PTI_INT32)
SET_PROPERTY_ID(uint32_t, PTI_INT32)
SET_PROPERTY_ID(int64_t, PTI_INT64)
SET_PROPERTY_ID(uint64_t, PTI_INT64)
SET_PROPERTY_ID(std::string, PTI_STRING)
SET_PROPERTY_ID(float, PTI_FLOAT)
SET_PROPERTY_ID(double, PTI_FLOAT)

#define SET_PB_FUNC(name,type,pbtype)	\
static void name(BaseProperty* basepro,LPMsg::property& pb)	\
{	\
	auto pro = static_cast<Property<type>*>(basepro);	\
	pbtype ppb;	\
	ppb.set_data(pro->data);	\
	pb.set_protype(PropertyID<type>::value);			\
	std::string outstr;									\
	if (ppb.SerializePartialToString(&outstr))			\
		pb.set_data(outstr);							\
}


struct PropertyToPB
{
	SET_PB_FUNC(ToBool, bool, LPMsg::propertyInt32);
	SET_PB_FUNC(ToInt8, int8_t, LPMsg::propertyInt32);
	SET_PB_FUNC(ToInt16, int16_t, LPMsg::propertyInt32);
	SET_PB_FUNC(ToInt32, int32_t, LPMsg::propertyInt32);
	SET_PB_FUNC(ToUInt8, uint8_t, LPMsg::propertyInt32);
	SET_PB_FUNC(ToUInt16, uint16_t, LPMsg::propertyInt32);
	SET_PB_FUNC(ToUInt32, uint32_t, LPMsg::propertyInt32);
	SET_PB_FUNC(ToInt64, uint64_t, LPMsg::propertyInt64);
	SET_PB_FUNC(ToUInt64, uint64_t, LPMsg::propertyInt64);
	SET_PB_FUNC(ToString, std::string, LPMsg::propertyString);
	SET_PB_FUNC(ToFloat, float, LPMsg::propertyFloat);
	SET_PB_FUNC(ToDouble, double, LPMsg::propertyDouble);
};


#endif
