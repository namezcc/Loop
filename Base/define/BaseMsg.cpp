#include "BaseMsg.h"
#include "LoopArray.h"
#include "BaseLayer.h"

void BaseMsg::recycleMsg()
{
	if (m_data)
	{
		m_data->recycleMsg();
		m_data = NULL;
	}
	((LoopList<BaseMsg*>*)m_looplist)->write(this);
}

BuffBlock::BuffBlock():m_size(0), m_next(NULL),m_buff(NULL)
{
}

void BuffBlock::makeRoom(const int32_t & size)
{
	SAFE_FREE(m_buff);
	if(size>0)
		m_buff = (char*)malloc(size);
	m_size = size;
}

void BuffBlock::write(char * buf, const int32_t & size)
{
	if (size <= 0)
		return;
	if (m_buff == NULL || m_size != size)
		makeRoom(size);
	if(m_buff)
		memcpy(m_buff, buf, size);
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
	SAFE_FREE(m_buff);
	((LoopList<BuffBlock*>*)m_looplist)->write(this);
}

void BuffBlock::init(FactorManager * fm)
{
	SAFE_FREE(m_buff);
}

void BuffBlock::recycle(FactorManager * fm)
{
	SAFE_FREE(m_buff);
	while (m_next)
	{
		auto tmp = m_next;
		m_next = tmp->m_next;
		tmp->m_next = NULL;
		fm->recycle(tmp);
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

SHARE<BuffBlock> NetMsg::getCombinBuff(BaseLayer * l)
{
	auto buff = l->GetSharedLoop<BuffBlock>();
	char* buf = NULL;
	if (m_buff->m_next == NULL)
	{
		buf = m_buff->m_buff;
		m_buff->m_buff = NULL;
		buff->m_buff = buf;
		buff->m_size = len;
		return buff;
	}
	if (len == 0)
		return buff;
	buf = (char*)malloc(len);
	int32_t idx = 0;
	auto mbf = m_buff;
	while (mbf) {
		memcpy(buf + idx, mbf->m_buff, mbf->m_size);
		idx += mbf->m_size;
		mbf = mbf->m_next;
	}
	buff->m_buff = buf;
	buff->m_size = len;
	return buff;
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
	/*if (m_buff)
	{
		m_buff->recycleMsg();
		m_buff = NULL;
	}
	((LoopList<NetServerMsg*>*)m_looplist)->write(this);*/
}

void NetServerMsg::init(FactorManager * fm)
{
	initMsg();
}

void NetServerMsg::recycle(FactorManager * fm)
{
	if(m_buff)
	{
		fm->recycle(m_buff);
		m_buff = NULL;
	}
}

void NetServerMsg::push_front(BaseLayer * l, const char * buf, const int32_t & size)
{
	auto buff = l->GetLoopObj<BuffBlock>();
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

BuffBlock* PB::PBToBuffBlock(BaseLayer* l,google::protobuf::Message& msg)
{
	auto buff = l->GetLayerMsg<BuffBlock>();
	auto pbsize = msg.ByteSize();
	buff->makeRoom(pbsize);
	msg.SerializeToArray(buff->m_buff, pbsize);
	return buff;
}

BuffBlock * PB::PBToBuffBlock(BaseLayer * l, google::protobuf::Message & msg, const int32_t & append)
{
	auto buff = l->GetLayerMsg<BuffBlock>();
	auto pbsize = msg.ByteSize();
	buff->makeRoom(pbsize + append);
	msg.SerializeToArray(buff->m_buff + append, pbsize);
	return buff;
}
