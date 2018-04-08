#ifndef LOOP_ARRAY_H
#define LOOP_ARRAY_H

#if defined( __WIN32__ ) || defined( _WIN32 ) || defined(_WINDOWS) || defined(WIN) || defined(_WIN64) || defined( __WIN64__ )
#define l_sleep(s) Sleep(s)
#else
#include <unistd.h>
#define l_sleep(s) usleep(s*1000)
#endif
template<typename T>
class LoopArray
{
public:

	LoopArray(size_t size):m_ridx(0),m_widx(0)
	{
		m_data = new T[size];
		m_size = size;
	}

	~LoopArray()
	{
		delete[] m_data;
	}

	T read() {
		lockRead();
		m_ridx = IncIndex(m_ridx);
		return m_data[m_ridx];
	}

	void write(T t)
	{
		lockWrite();
		m_data[IncIndex(m_widx)] = t;
		m_widx = IncIndex(m_widx);
	}

protected:
	size_t IncIndex(size_t idx)
	{
		return ++idx >= m_size ? 0:idx;
	}

	bool tryRead() {
		return m_ridx != m_widx;
	};
	bool tryWrite() {
		return IncIndex(m_widx) != m_ridx;
	};

	void lockRead()
	{
		while (!tryRead())
			l_sleep(2);
	}

	void lockWrite()
	{
		while (!tryWrite())
			l_sleep(2);
	}

private:
	T* m_data;
	size_t m_size;
	size_t m_ridx;
	size_t m_widx;
};

#define DEF_LOOP_SIZE 1000

template<typename T>
class LoopList
{
public:
	typedef struct LN
	{
		LN(T t):data(t),next(NULL)
		{
		}
		~LN()
		{
		}
		T data;
		LN* next;
	}LPNode;

	typedef struct LP
	{
		LPNode* head;
		LPNode* tail;

		LP():head(NULL),tail(NULL)
		{
		}

		~LP()
		{
			while (head)
			{
				LPNode* n = head;
				head = head->next;
				delete n;
			}
		}
		
		void push(T t)
		{
			if (tail == NULL)
				head = tail = new LPNode(t);
			else
			{
				LPNode* n = new LPNode(t);
				tail->next = n;
				tail = n;
			}
		}

		LPNode* getHead()
		{
			return head;
		}
	}LPList;

	LoopList():m_size(DEF_LOOP_SIZE), m_widx(0), m_ridx(0)
	{
		m_data = new LPList[DEF_LOOP_SIZE];
	}

	LoopList(size_t size):m_size(size),m_widx(0),m_ridx(0)
	{
		m_data = new LPList[size];
	}

	~LoopList()
	{
		delete[] m_data;
	}

	void write(T t)
	{
		if (tryWrite())
		{
			LPList* lw = &m_data[IncIndex(m_widx)];
			lw->push(t);
			m_widx = IncIndex(m_widx);
		}
		else
		{
			LPList* lw = &m_data[m_widx];
			lw->push(t);
		}
	}

	bool read(LPList& l)
	{
		if (!tryRead())
			return false;
		m_ridx = IncIndex(m_ridx);

		LPList* rl = &m_data[m_ridx];
		l.head = rl->head;
		l.tail = rl->tail;
		rl->head = NULL;
		rl->tail = NULL;
		return true;
	}

	bool readTimeOut(LPList& l,size_t mt,size_t count=0)
	{
		while (!read(l))
		{
			l_sleep(mt);
			--count;
			if (count <= 0)
				return false;
		}
		return true;
	}

protected:
	size_t IncIndex(size_t idx)
	{
		return ++idx == m_size ? 0 : idx;
	}

	bool tryRead() {
		return m_ridx != m_widx;
	};
	bool tryWrite() {
		return IncIndex(m_widx) != m_ridx;
	};

private:
	LPList* m_data;
	size_t m_size;
	size_t m_ridx;
	size_t m_widx;
};

#endif