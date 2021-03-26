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

class BuffPool
{
	friend class Single;
public:
	
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
		for (size_t i = 0; i < 11; i++)
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
		}
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
#define GET_POOL_BUFF(nsize,rsize,looplist) Single::LocalInstance<BuffPool>()->GetLayerBuff(nsize,rsize,looplist)
//use local layer
#define GET_LOCAL_BUFF(nsize,rsize) Single::LocalInstance<BuffPool>()->GetLocalBuff(nsize,rsize)
#define RCY_LOCAL_BUFF(buff,size) Single::LocalInstance<BuffPool>()->RecycleLocalBuff(buff,size)

#endif
