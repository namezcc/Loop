#ifndef BLOCK_H
#define BLOCK_H
#include <vector>
#include <assert.h>
template<typename T>
class Block
{
public:
	Block(const unsigned int bsize):m_bsize(bsize){
		assert(bsize>0);
	};

	~Block() {
		for (auto t:m_stack)
		{
			delete[] t;
		}
	};

	T* newBlock() {
		T* t = new T[m_bsize];
		m_stack.push_back(t);
		return t;
	}

	inline size_t bsize() {
		return m_bsize;
	}
private:
	const unsigned int m_bsize;
	std::vector<T*> m_stack;
};

#endif