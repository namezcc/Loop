﻿#include "BaseMsg.h"
#include "LoopArray.h"
#include "BaseLayer.h"
#include "BuffPool.h"

void BaseMsg::recycleMsg()
{
	if (m_data)
	{
		m_data->recycleMsg();
		m_data = NULL;
	}
	if (!m_looplist)
		delete this;
	else
		((LoopList<BaseMsg*>*)m_looplist)->write(this);
}

BuffBlock::BuffBlock():m_size(0),m_allsize(0), m_next(NULL),m_buff(NULL),m_recylist(NULL)
{
}

void BuffBlock::makeRoom(const int32_t & size)
{
	if (m_buff)
		assert(0);
	LoopList<char*>* tlist = NULL;
	m_buff = GET_POOL_BUFF(size, m_allsize, tlist);
	m_recylist = tlist;
	if(!m_recylist)
		assert(0);
}

void BuffBlock::write(char * buf, const int32_t & size)
{
	if (size <= 0)
		return;

	if (m_buff == NULL)
	{
		makeRoom(size);
	}

	if (m_offect + size > m_allsize)
		assert(0);

	memcpy(m_buff + m_offect, buf, size);
	if(m_offect == m_size)
		m_size += size;
	m_offect += size;
}

void BuffBlock::recycleMsg()
{
	while (m_next)
	{
		auto tmp = m_next;
		m_next = tmp->m_next;
		tmp->m_next = NULL;
		tmp->recycleMsg();
	}
	if (m_buff)
	{
		((LoopList<char*>*)m_recylist)->write(m_buff);
		m_recylist = NULL;
		m_buff = NULL;
	}
	if (!m_looplist)
		delete this;
	else
		((LoopList<BuffBlock*>*)m_looplist)->write(this);
}

void BuffBlock::recycleCheck()
{
	assert(m_recylist);
}

LocalBuffBlock::LocalBuffBlock():BuffBlock()
{
}

void LocalBuffBlock::makeRoom(const int32_t & size)
{
	if (m_buff)
	{
		if (size > m_allsize)
			RCY_LOCAL_BUFF(m_buff, m_allsize);
		else
			return;
	}
	m_buff = GET_LOCAL_BUFF(size, m_allsize);
}

void LocalBuffBlock::init(FactorManager * fm)
{
	initMsg();
}

void LocalBuffBlock::recycle(FactorManager * fm)
{
	if (m_next)
		assert(0);
	if (m_buff)
	{
		RCY_LOCAL_BUFF(m_buff, m_allsize);
		m_buff = NULL;
	}
}

NetMsg::NetMsg():m_buff(NULL)
{
}

void NetMsg::initMsg()
{
	len = 0;
	mid = 0;
	socket = 0;
}

void NetMsg::push_front(BuffBlock* buff)
{
	if (buff == NULL)
		return;
	auto tmp = buff;
	while(tmp){
		len += tmp->getSize();
		if (tmp->m_next == NULL)
		{
			tmp->m_next = m_buff;
			break;
		}
		tmp = tmp->m_next;
	}
	m_buff = buff;
}

void NetMsg::push_front(BaseLayer* l,const char* buf,const int32_t& size)
{
	auto buff = l->GetLayerMsg<BuffBlock>();
	buff->makeRoom(size);
	buff->write(const_cast<char*>(buf),size);
	buff->setOffect(0);
	push_front(buff);
}

char * NetMsg::getNetBuff()
{
	if (m_buff)
		return m_buff->m_buff + m_buff->getOffect();
	return NULL;
}

SHARE<LocalBuffBlock> NetMsg::getCombinBuff()
{
	auto buff = GET_SHARE(LocalBuffBlock);
	if (len == 0)
		return buff;
	buff->makeRoom(len);
	auto mbf = m_buff;
	while (mbf) {
		if (mbf->m_buff)
			buff->write(mbf->m_buff, mbf->getSize());
		mbf = mbf->m_next;
	}
	return buff;
}

void NetMsg::getCombinBuff(LocalBuffBlock* buff)
{
	auto mbf = m_buff;
	while (mbf) {
		if (mbf->m_buff)
			buff->write(mbf->m_buff, mbf->getSize());
		mbf = mbf->m_next;
	}
}

BuffBlock * NetMsg::popBuffBlock()
{
	BuffBlock * pop = NULL;
	if (m_buff)
	{
		pop = m_buff;
		m_buff = m_buff->m_next;
		pop->m_next = NULL;
		len -= pop->getSize();
	}
	return pop;
}

void NetMsg::recycleMsg()
{
	if(m_buff)
	{
		m_buff->recycleMsg();
		m_buff = NULL;
	}
	if (!m_looplist)
		delete this;
	else
		((LoopList<NetMsg*>*)m_looplist)->write(this);
}

void NetServerMsg::recycleMsg()
{
	//can't enter this
	assert(0);
}

void NetServerMsg::init(FactorManager * fm)
{
	initMsg();
}

void NetServerMsg::recycle(FactorManager * fm)
{
	if(m_buff)
	{
		LOOP_RECYCLE(m_buff);
		m_buff = NULL;
	}
}

void NetServerMsg::push_front(BaseLayer * l, const char * buf, const int32_t & size)
{
	auto buff = GET_LOOP(LocalBuffBlock);
	buff->makeRoom(size);
	buff->write(const_cast<char*>(buf), size);
	buff->setOffect(0);
	NetMsg::push_front(buff);
}

void NetServer::recycleMsg()
{
	if (!m_looplist)
		delete this;
	else
		((LoopList<NetServer*>*)m_looplist)->write(this);
}

void LogInfo::recycleMsg()
{
	log.str("");
	if (!m_looplist)
		delete this;
	else
		((LoopList<LogInfo*>*)m_looplist)->write(this);
}