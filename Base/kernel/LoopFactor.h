#ifndef LOOP_FACTOR_H
#define LOOP_FACTOR_H
#include <list>
#include "Block.h"
#include "Single.h"

#define BLOCK_SIZE 10

template<typename T>
struct TSB
{
	enum{bsize = BLOCK_SIZE,};
};
#define SET_T_BLOCK_SIZE(T,S)	\
template<>						\
struct TSB<T>					\
{								\
	enum{bsize = S,};			\
};			

template<typename T>
class LoopFactor
{
public:

	T* get() {
		if (m_pool.size()<=0)
		{
			newBlock();
		}
		auto t = m_pool.front();
		m_pool.pop_front();
		return t;
	}

	void recycle(T* t)
	{
		assert(t!=NULL);
		m_pool.push_back(t);
	}

	friend class Single;
	~LoopFactor() {};
private:
	LoopFactor():m_block(TSB<T>::bsize)
	{
		//std::cout << "new LoopFactor T=" << typeid(T).name() << std::endl;
	};

	void newBlock() {
		T* t = m_block.newBlock();
		for (size_t i = 0; i < m_block.bsize(); i++)
		{
			m_pool.push_back(t++);
		}
	}

	Block<T> m_block;
	std::list<T*> m_pool;
};

#endif