#ifndef MYSQL_DEFINE_H
#define MYSQL_DEFINE_H

#include "BaseMsg.h"
#include "MsgPool.h"

struct SqlOperation:public BaseData
{
	SqlOperation() :optId(0),ackId(0), uid(0), buff(NULL)
	{

	}

	virtual void initMsg() override
	{

	}

	virtual void recycleMsg() override
	{
		ackId = 0;
		optId = 0;
		uid = 0;
		path.clear();
		if (buff)
			buff->recycleMsg();
		buff = NULL;
		MsgPool::pushMsg(this);
	}

	int32_t optId;
	int32_t ackId;
	int64_t uid;
	ServerPath path;
	BuffBlock* buff;
};


#endif
