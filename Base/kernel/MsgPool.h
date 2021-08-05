#ifndef MSG_POOL_H
#define MSG_POOL_H
#include "BaseMsg.h"
#include "LoopArray.h"
#include "FactorManager.h"
#include "Block2.h"

SET_T_BLOCK_SIZE_2(BaseMsg,20000)

struct MsgPool
{
	template<typename T>
	static typename std::enable_if<std::is_base_of<BaseData, T>::value, T*>::type popMsg()
	{
		T* msg = Single::GetInstence<Block2<T>>()->allocateNewOnce();
		msg->initMsg();
		return msg;
	}

	template<typename T>
	static typename std::enable_if<std::is_base_of<BaseData, T>::value>::type pushMsg(T* t)
	{
		Single::GetInstence<Block2<T>>()->deallcate(t);
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

	LoopArray<BaseData*> m_pool;
};

#endif
