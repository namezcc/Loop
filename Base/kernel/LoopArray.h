#ifndef LOOP_ARRAY_H
#define LOOP_ARRAY_H

#define DEF_LOOP_SIZE 1000
#include "Block2.h"
//#include "Single.h"

template<typename T>
class LoopList
{
public:
	typedef struct LN
	{
		LN(T t):data(t),next(NULL)
		{}
		LN():next(NULL)
		{}

		~LN()
		{}
		T data;
		LN* next;
	}LPNode;

	typedef struct LP
	{
		LPNode* head;
		LPNode* tail;

		LP():head(NULL),tail(NULL)
		{}

		~LP()
		{}

		LPNode* pop()
		{
			if (head == NULL)
				return NULL;

			auto tmp = head;
			head = head->next;
			if (head == NULL)
				tail = NULL;
			tmp->next = NULL;
			return tmp;
		}

		void push(LPNode* n)
		{
			if (tail == NULL)
				head = tail = n;
			else
			{
				tail->next = n;
				tail = n;
			}
		}

		void combin(LP& lp)
		{
			if (!lp.tail)
				return;
			if (tail)
			{
				tail->next = lp.head;
				tail = lp.tail;
			}
			else
			{
				head = lp.head;
				tail = lp.tail;
			}
			lp.head = NULL;
			lp.tail = NULL;
		}

		LPNode* getHead()
		{
			return head;
		}
	}LPList;

	LoopList():m_size(DEF_LOOP_SIZE), m_widx(0), m_ridx(0), m_lockindex(-1)
	{
		m_data = new LPList[DEF_LOOP_SIZE];
		m_cash = new LPList[DEF_LOOP_SIZE];
		m_nodePool = new Block2<LPNode>();
	}

	LoopList(size_t size):m_size(size),m_widx(0),m_ridx(0), m_lockindex(-1)
	{
		m_data = new LPList[size];
		m_cash = new LPList[size];
		m_nodePool = new Block2<LPNode>();
	}

	~LoopList()
	{
		delete[] m_data;
		delete[] m_cash;
	}

	void write(T t)
	{
		while (true)
		{
			if (tryWrite())
			{
				size_t idx = IncIndex(m_widx);
				recycleCash(idx);
				LPList* lw = &m_data[idx];
				auto n = getNode();
				n->data = t;
				lw->push(n);
				m_widx = idx;
				break;
			}
			else
			{
				m_lockindex = static_cast<int32_t>(m_widx);
				if (tryWrite())
					m_lockindex = -1;
				else
				{
					LPList* lw = &m_data[m_widx];
					auto n = getNode();
					n->data = t;
					lw->push(n);
					m_lockindex = -1;
					break;
				}
			}
		}
	}

	bool pop(T& t)
	{
		auto ln = m_cur.pop();
		if (ln)
		{
			t = ln->data;
			recycleNode(ln);
			return true;
		}
		if(!read(m_cur))
			return false;

		ln = m_cur.pop();
		t = ln->data;
		recycleNode(ln);
		return true;
	}

protected:
	bool read(LPList& l)
	{
		if (!tryRead())
			return false;
		if (m_lockindex >= 0 && m_lockindex == IncIndex(m_ridx))
			return false;
		m_ridx = IncIndex(m_ridx);

		LPList* rl = &m_data[m_ridx];
		l.head = rl->head;
		l.tail = rl->tail;
		rl->head = NULL;
		rl->tail = NULL;
		return true;
	}

	void recycleCash(size_t index)
	{
		m_pool.combin(m_cash[index]);
	}

	LPNode* getNode()
	{
		auto n = m_pool.pop();
		if (n)
			return n;
		else
			return m_nodePool->allocateNewOnce();
		//return GET_LOOP(LPNode);	//this well 2 thread call can not use LocalIncetance
	}

	void recycleNode(LPNode* n)
	{
		m_cash[m_ridx].push(n);
	}

	size_t IncIndex(size_t idx)
	{
		return ++idx == m_size ? 0 : idx;
	}

	bool tryRead() {
		return m_ridx != m_widx;
	};
	bool tryWrite() {
		return IncIndex(m_widx) != m_ridx;
	}
private:
	LPList* m_data;
	LPList m_cur;
	size_t m_size;
	volatile size_t m_ridx;
	volatile size_t m_widx;
	volatile int32_t m_lockindex;
	LPList* m_cash;
	LPList m_pool;
	Block2<LPNode>* m_nodePool;
};

#endif