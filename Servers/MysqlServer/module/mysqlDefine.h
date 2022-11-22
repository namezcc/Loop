#ifndef MYSQL_DEFINE_H
#define MYSQL_DEFINE_H

#include "BaseMsg.h"
#include "MsgPool.h"

struct SqlOperation:public BaseData
{
	SqlOperation() :optId(0),ackId(0), cid(0), server_sock(0), buff(NULL)
	{

	}

	virtual void initMsg() override
	{

	}

	virtual void recycleMsg() override
	{
		ackId = 0;
		optId = 0;
		cid = 0;
		server_sock = 0;
		if (buff)
			buff->recycleMsg();
		buff = NULL;
		MsgPool::pushMsg(this);
	}

	int32_t optId;
	int32_t ackId;
	int32_t cid;
	int32_t server_sock;
	BuffBlock* buff;
};


#endif
