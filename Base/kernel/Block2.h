#ifndef BLOCK_H_2
#define BLOCK_H_2
//#include <iostream>

template<size_t Size, size_t Bsize = 4096>
struct MultSize
{
	static constexpr size_t value = Size + ((Bsize - Size) % Bsize);
};

template<typename T>
struct TSB2
{
	enum { bsize = 1000, };
};
#define SET_T_BLOCK_SIZE_2(T,S)	\
template<>						\
struct TSB2<T>					\
{								\
	enum{bsize = S,};			\
};	

template<typename T, size_t Tsize = TSB2<T>::bsize>
class Block2
{
private:
	struct Slot_
	{
		T mdata;
		bool isNew;
		Slot_* next;
	};

public:
	static constexpr size_t BlockSize = MultSize<Tsize * sizeof(Slot_)>::value;

	Block2() {
		m_curBlock = NULL;
		m_curSlot = NULL;
		m_freeSlot = NULL;
		m_lastSlot = NULL;
	};

	~Block2() {
		Slot_* curr = m_curBlock;
		while (curr != NULL)
		{
			Slot_* prev = *reinterpret_cast<Slot_**>(curr);
			//std::cout << "delete block:" << (void*)curr << std::endl;
			operator delete(reinterpret_cast<void*>(curr));
			curr = prev;
		}
	};

	inline T* allocate()
	{
		if (m_freeSlot != NULL) {
			T* res = reinterpret_cast<T*>(m_freeSlot);
			m_freeSlot = m_freeSlot->next;
			return res;
		}
		else {
			if (m_curSlot >= m_lastSlot)
				allocBlock();
			return reinterpret_cast<T*>(m_curSlot++);
		}
	}

	template<typename... Args>
	inline void construct(T* p, Args&&... args)
	{
		new (p) T(std::forward<Args>(args)...);
	}

	void destroy(T* p)
	{
		p->~T();
	}

	template<typename... Args>
	inline T* allocateNewOnceLimitNum(Args&&... args)
	{
		if (m_freeSlot != NULL) {
			T* res = reinterpret_cast<T*>(m_freeSlot);
			m_freeSlot = m_freeSlot->next;
			return res;
		}
		else {
			if (m_curSlot >= m_lastSlot)
			{
				if(m_curSlot == NULL)
					allocBlock();
				else
					return NULL;
			}
			T* res = reinterpret_cast<T*>(m_curSlot++);
			construct(res, std::forward<Args>(args)...);
			return res;
		}
	}

	template<typename... Args>
	inline T* allocateNewOnce(Args&&... args)
	{
		if (m_freeSlot != NULL) {
			T* res = reinterpret_cast<T*>(m_freeSlot);
			m_freeSlot->isNew = false;
			m_freeSlot = m_freeSlot->next;
			return res;
		}
		else {
			if (m_curSlot >= m_lastSlot)
			{
				if (m_curSlot == NULL)
					allocBlock();
				else
				{
					auto s = new Slot_();
					s->isNew = true;
					return reinterpret_cast<T*>(s);
				}
			}
			m_curSlot->isNew = false;
			T* res = reinterpret_cast<T*>(m_curSlot++);
			construct(res, std::forward<Args>(args)...);
			return res;
		}
	}

	inline void deallcate(T* p)
	{
		if (p != NULL) {
			if (reinterpret_cast<Slot_*>(p)->isNew)
			{
				delete reinterpret_cast<Slot_*>(p);
			}
			else
			{
				reinterpret_cast<Slot_*>(p)->next = m_freeSlot;
				m_freeSlot = reinterpret_cast<Slot_*>(p);
			}
		}
	}

	template<typename... Args>
	inline T* newElement(Args&&... args)
	{
		T* res = allocate();
		construct(res, std::forward<Args>(args)...);
		return res;
	}

	inline void deleteElement(T* p)
	{
		if (p != NULL) {
			p->~T();
			deallcate(p);
		}
	}

private:

	inline size_t padPointer(char*p, size_t align) const noexcept
	{
		uintptr_t res = reinterpret_cast<uintptr_t>(p);
		return ((align - res) % align);
	}

	inline void allocBlock()
	{
		char* newBlock = reinterpret_cast<char*>(operator new(BlockSize));
		//std::cout << "new block:" << (void*)newBlock << std::endl;
		*reinterpret_cast<Slot_**>(newBlock) = m_curBlock;
		m_curBlock = reinterpret_cast<Slot_*>(newBlock);

		char* body = newBlock + sizeof(Slot_*);
		size_t bodypad = padPointer(body, alignof(Slot_));
		m_curSlot = reinterpret_cast<Slot_*>(body + bodypad);
		m_lastSlot = reinterpret_cast<Slot_*>(newBlock + BlockSize - sizeof(Slot_) + 1);
	}

	Slot_ * m_curBlock;
	Slot_ * m_curSlot;
	Slot_ * m_lastSlot;
	Slot_ * m_freeSlot;

	static_assert(Tsize >= 2, "BSize too small.");
};

#endif