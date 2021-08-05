#ifndef LOOP_LIST_H
#define LOOP_LIST_H

namespace Loop {

	struct mnode
	{
		mnode() :m_next(NULL), m_prev(NULL)
		{};

		virtual ~mnode()
		{}
		mnode* next() { return m_next; }

		mnode* prev() { return m_prev; }
		bool isInList() { return m_next != NULL && m_prev != NULL; }

		mnode* m_next;
		mnode* m_prev;
	};

template<typename T>
class mlist {
public:

	mlist()
	{
		m_head.m_next = &m_tail;
		m_tail.m_prev = &m_head;
	}

	T * begin()
	{
		if (m_head.next()->isInList())
			return dynamic_cast<T*>(m_head.next());
		return NULL;
	}

	T* erase(T* it)
	{
		if (it)
		{
			auto p = it->prev();
			auto n = it->next();

			p->m_next = n;
			n->m_prev = p;
			if(n->isInList())
				return dynamic_cast<T*>(n);
			return NULL;
		}
		else
			return NULL;
	}

	void push_back(T* t)
	{
		auto p = m_tail.m_prev;
		t->m_prev = p;
		t->m_next = p->m_next;

		p->m_next = t;
		m_tail.m_prev = t;
	}

private:

	T m_head;
	T m_tail;
};

};

#endif // !LOOP_LIST_H

