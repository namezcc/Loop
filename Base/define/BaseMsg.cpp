#include "BaseMsg.h"
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
		makeRoom(size);

	if (size + m_size > m_allsize)
		assert(0);

	memcpy(m_buff + m_size, buf, size);
	m_size += size;
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
	((LoopList<BuffBlock*>*)m_looplist)->write(this);
}

void BuffBlock::recycleCheck()
{
	assert(m_recylist);
}

//void BuffBlock::init(FactorManager * fm)
//{
//	SAFE_FREE(m_buff);
//}
//
//void BuffBlock::recycle(FactorManager * fm)
//{
//	SAFE_FREE(m_buff);
//	while (m_next)
//	{
//		auto tmp = m_next;
//		m_next = tmp->m_next;
//		tmp->m_next = NULL;
//		fm->recycle(tmp);
//	}
//}

LocalBuffBlock::LocalBuffBlock():BuffBlock()
{
}

void LocalBuffBlock::makeRoom(const int32_t & size)
{
	if (m_buff)
		assert(0);
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
		len += tmp->m_size;
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
	buff->write(const_cast<char*>(buf),size);
	push_front(buff);
}

char * NetMsg::getNetBuff()
{
	if (m_buff)
		return m_buff->m_buff;
	return NULL;
}

SHARE<LocalBuffBlock> NetMsg::getCombinBuff(BaseLayer * l)
{
	auto buff = l->GetSharedLoop<LocalBuffBlock>();
	if (len == 0)
		return buff;
	buff->makeRoom(len);
	auto mbf = m_buff;
	while (mbf) {
		if (mbf->m_buff)
			buff->write(mbf->m_buff, mbf->m_size);
		mbf = mbf->m_next;
	}
	return buff;
}

BuffBlock * NetMsg::popBuffBlock()
{
	BuffBlock * pop = NULL;
	if (m_buff)
	{
		pop = m_buff;
		m_buff = m_buff->m_next;
		pop->m_next = NULL;
		len -= pop->m_size;
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
		fm->recycle(dynamic_cast<LocalBuffBlock*>(m_buff));
		m_buff = NULL;
	}
}

void NetServerMsg::push_front(BaseLayer * l, const char * buf, const int32_t & size)
{
	auto buff = l->GetLoopObj<LocalBuffBlock>();
	buff->write(const_cast<char*>(buf), size);
	NetMsg::push_front(buff);
}

void NetSocket::recycleMsg()
{
	((LoopList<NetSocket*>*)m_looplist)->write(this);
}

void NetServer::recycleMsg()
{
	((LoopList<NetServer*>*)m_looplist)->write(this);
}

void LogInfo::recycleMsg()
{
	log.str("");
	((LoopList<LogInfo*>*)m_looplist)->write(this);
}

BuffBlock* PB::PBToBuffBlock(BaseLayer* l,const google::protobuf::Message& msg)
{
	auto buff = l->GetLayerMsg<BuffBlock>();
	auto pbsize = msg.ByteSize();
	buff->makeRoom(pbsize);
	buff->m_size = pbsize;
	msg.SerializeToArray(buff->m_buff, pbsize);
	return buff;
}

BuffBlock * PB::PBToBuffBlock(BaseLayer * l, const google::protobuf::Message & msg, const int32_t & append)
{
	auto buff = l->GetLayerMsg<BuffBlock>();
	auto pbsize = msg.ByteSize();
	buff->makeRoom(pbsize + append);
	buff->m_size = pbsize + append;
	msg.SerializeToArray(buff->m_buff + append, pbsize);
	return buff;
}