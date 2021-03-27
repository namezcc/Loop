#ifndef BUFF_POOL_H
#define BUFF_POOL_H

#include "LoopArray.h"

#define BASE_SIZE_INDEX 6
#define BASE_BUFF_SIZE 64
#define BASE_INIT_SIZE 1024

static const int32_t bitIndexMap[256] = {
	0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,
	4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
	6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
	6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
	6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
	6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
};

static const int32_t pool_size[32] = {
	1000,1000,1000,1000,1000,1000,1000,100,
	10,10,10,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0
};

inline int32_t hightestBitIndex(const uint32_t& value)
{
	if (value <= 0xFF) return bitIndexMap[value];
	else if (value <= 0xFFFF) return bitIndexMap[(value >> 8) & 0xFF] + 8;
	else if (value <= 0xFFFFFF) return bitIndexMap[(value >> 16) & 0xFF] + 16;
	else return bitIndexMap[(value >> 24) & 0xFF] + 24;
}

template <size_t I>
struct IndexSize
{
	static constexpr size_t value = BASE_BUFF_SIZE << I;
};

template <size_t S>
struct BuffNode
{
	char m_buff[S];
};

#define GET_BUFF_BLOCK(i,n) Single::GetInstence<Block2<BuffNode<IndexSize<i>::value>, n>>()->allocateNewOnce()->m_buff
#define PUSH_BUFF_BLOCK(i,n,b) Single::GetInstence<Block2<BuffNode<IndexSize<i>::value>, n>>()->deallcate((BuffNode<IndexSize<i>::value>*)b)

class BuffPool
{
	friend class Single;
public:
	
	char* getBuff(const int32_t& nsize, int32_t& rsize)
	{
		int32_t base = BASE_BUFF_SIZE;
		int32_t idx = 0;
		if (nsize > base)
		{
			idx = hightestBitIndex(static_cast<uint32_t>(nsize)) - BASE_SIZE_INDEX;
			base <<= idx;
			if (base < nsize)
			{
				++idx;
				base <<= 1;
			}
		}

		rsize = base;

		switch (idx)
		{
		case 0: return GET_BUFF_BLOCK(0,1000);
		case 1: return GET_BUFF_BLOCK(1, 1000);
		case 2: return GET_BUFF_BLOCK(2, 1000);
		case 3: return GET_BUFF_BLOCK(3, 1000);
		case 4: return GET_BUFF_BLOCK(4, 1000);
		case 5: return GET_BUFF_BLOCK(5, 1000);
		case 6: return GET_BUFF_BLOCK(6, 1000);
		case 7: return GET_BUFF_BLOCK(7, 1000);
		case 8: return GET_BUFF_BLOCK(8, 1000);
		case 9: return GET_BUFF_BLOCK(9, 1000);
		case 10: return GET_BUFF_BLOCK(10, 1000);
		default:
			rsize = nsize;
			return (char*)malloc(nsize);
			break;
		}
	}

	void pushBuff(char* buf, int32_t size)
	{
		int32_t idx = 0;
		if (size > BASE_BUFF_SIZE)
		{
			idx = hightestBitIndex(static_cast<uint32_t>(size)) - BASE_SIZE_INDEX;
			int32_t base = BASE_BUFF_SIZE << idx;
			if (base < size)
				++idx;
		}

		switch (idx)
		{
		case 0: PUSH_BUFF_BLOCK(0, 1000, buf); break;
		case 1: PUSH_BUFF_BLOCK(1, 1000, buf); break;
		case 2: PUSH_BUFF_BLOCK(2, 1000, buf); break;
		case 3: PUSH_BUFF_BLOCK(3, 1000, buf); break;
		case 4: PUSH_BUFF_BLOCK(4, 1000, buf); break;
		case 5: PUSH_BUFF_BLOCK(5, 1000, buf); break;
		case 6: PUSH_BUFF_BLOCK(6, 1000, buf); break;
		case 7: PUSH_BUFF_BLOCK(7, 1000, buf); break;
		case 8: PUSH_BUFF_BLOCK(8, 1000, buf); break;
		case 9: PUSH_BUFF_BLOCK(9, 1000, buf); break;
		case 10: PUSH_BUFF_BLOCK(10, 1000, buf); break;
		default:
			free(buf);
			break;
		}
	}

	char* GetLayerBuff(const int32_t& nsize,int32_t& rsize,LoopArray<char*>* &llist)
	{
		int32_t base = BASE_BUFF_SIZE;
		int32_t idx = 0;
		if (nsize > base)
		{
			idx = hightestBitIndex(static_cast<uint32_t>(nsize)) - BASE_SIZE_INDEX;
			base <<= idx;
			if (base < nsize)
			{
				++idx;
				base <<= 1;
			}
		}

		char* buf = NULL;
		if (m_layerArr[idx].pop(buf))
		{
			rsize = base;
			llist = &m_layerArr[idx];
			return buf;
		}
		rsize = nsize;
		buf = (char*)malloc(nsize);
		return buf;
	}

	char* GetLocalBuff(const int32_t& nsize, int32_t& rsize)
	{
		int32_t base = BASE_BUFF_SIZE;
		int32_t idx = 0;
		if (nsize > base)
		{
			idx = hightestBitIndex(static_cast<uint32_t>(nsize)) - BASE_SIZE_INDEX;
			base <<= idx;
			if (base < nsize)
			{
				++idx;
				base <<= 1;
			}
		}

		rsize = base;
		char* buf = NULL;
		if (m_localArr[idx].size() > 0)
		{
			buf = m_localArr[idx].back();
			m_localArr[idx].pop_back();
		}else
			buf = (char*)malloc(base);
		return buf;
	}

	void RecycleLocalBuff(char* buff, const int32_t& size)
	{
		if (!buff)
			assert(0);
		int32_t idx = 0;
		if (size > BASE_BUFF_SIZE)
		{
			idx = hightestBitIndex(static_cast<uint32_t>(size)) - BASE_SIZE_INDEX;
			int32_t base = BASE_BUFF_SIZE << idx;
			if (base < size)
				++idx;
		}
		if (idx > 6)
			free(buff);
		else
		{
			if (m_localArr[idx].size() >= 10000)
				free(buff);
			else
				m_localArr[idx].push_back(buff);
		}
	}

private:
	BuffPool() 
	{
		/*for (size_t i = 0; i < 11; i++)
		{
			auto num = pool_size[i];
			if (num == 0) continue;
			auto bsize = BASE_BUFF_SIZE << i;

			char* beg = (char*)malloc(bsize * num);
			m_delete_pool.push_back(beg);

			for (size_t j = 0; j < num; j++)
			{
				m_layerArr[i].write(beg);
				beg += bsize;
			}
		}*/
	}

	~BuffPool()
	{
		for (size_t i = 0; i < m_delete_pool.size(); i++)
		{
			free(m_delete_pool[i]);
		}
	}

	LoopArray<char*> m_layerArr[32];
	std::vector<char*> m_localArr[32];
	std::vector<char*> m_delete_pool;
};
//use for layers
#define GET_POOL_BUFF(nsize,rsize) Single::GetInstence<BuffPool>()->getBuff(nsize,rsize)
#define PUSH_POOL_BUFF(buf,size) Single::GetInstence<BuffPool>()->pushBuff(buf,size)

//use local layer
#define GET_LOCAL_BUFF(nsize,rsize) Single::LocalInstance<BuffPool>()->GetLocalBuff(nsize,rsize)
#define RCY_LOCAL_BUFF(buff,size) Single::LocalInstance<BuffPool>()->RecycleLocalBuff(buff,size)

#endif
