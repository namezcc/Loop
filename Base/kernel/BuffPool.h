#ifndef BUFF_POOL_H
#define BUFF_POOL_H

#include "LoopArray.h"

#define BASE_BUFF_SIZE 64
#define BASE_INIT_SIZE 1024

class BuffPool
{
	friend class Single;
public:
	
	char* GetLayerBuff(const int32_t& nsize,int32_t& rsize,LoopList<char*>* &llist)
	{
		int32_t base = BASE_BUFF_SIZE;
		int8_t idx = 0;
		while (base < nsize)
		{
			base <<= 1;
			++idx;
		}

		llist = &m_layerArr[idx];
		rsize = base;
		char* buf = NULL;
		if (m_layerArr[idx].pop(buf))
			return buf;
		buf = (char*)malloc(base);
		return buf;
	}

	char* GetLocalBuff(const int32_t& nsize, int32_t& rsize)
	{
		int32_t base = BASE_BUFF_SIZE;
		int8_t idx = 0;
		while (base < nsize)
		{
			base <<= 1;
			++idx;
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
		int32_t base = BASE_BUFF_SIZE;
		int8_t idx = 0;
		while (base < size)
		{
			base <<= 1;
			++idx;
		}
		m_localArr[idx].push_back(buff);
	}

private:
	BuffPool() 
	{
		for (size_t i = 0; i < 4; i++)
		{
			auto bsize = BASE_BUFF_SIZE << i;
			auto bnum = BASE_INIT_SIZE >> i;
			char* beg = (char*)malloc(bsize * bnum * 2);
			auto ptr = beg;
			for (size_t j = 0; j < bnum; j++)
			{
				m_layerArr[i].write(ptr);
				ptr += bsize;
			}
			for (size_t j = 0; j < bnum; j++)
			{
				m_localArr[i].push_back(ptr);
				ptr += bsize;
			}
		}
	}

	LoopList<char*> m_layerArr[32];
	std::vector<char*> m_localArr[32];
};

#define GET_POOL_BUFF(nsize,rsize,looplist) Single::LocalInstance<BuffPool>()->GetLayerBuff(nsize,rsize,looplist)
#define GET_LOCAL_BUFF(nsize,rsize) Single::LocalInstance<BuffPool>()->GetLocalBuff(nsize,rsize)
#define RCY_LOCAL_BUFF(buff,size) Single::LocalInstance<BuffPool>()->RecycleLocalBuff(buff,size)

#endif
