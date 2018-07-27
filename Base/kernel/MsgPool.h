#ifndef MSG_POOL_H
#define MSG_POOL_H
#include "BaseMsg.h"
#include "LoopArray.h"
#include "FactorManager.h"

struct MsgPool
{
	template<typename T>
	typename std::enable_if<std::is_base_of<BaseData, T>::value, T*>::type popMsg()
	{
		auto llist = Single::LocalInstance<LoopList<T*>>();
		T* msg = NULL;
		if (!llist->pop(msg))
		{
			msg = Single::LocalInstance<LoopFactor<T>>()->get();
			msg->m_looplist = (void*)llist;
		}
		msg->initMsg();
		return msg;
	}
};

struct RecyclePool
{
	void recycle(BaseData* msg)
	{
		m_pool.write(msg);
	}

	BaseData* pop()
	{
		BaseData* msg = NULL;
		m_pool.pop(msg);
		return msg;
	}

	LoopList<BaseData*> m_pool;
};

#endif
