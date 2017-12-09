#ifndef DATA_DEFINE_H
#define DATA_DEFINE_H
#include "FactorManager.h"
#include "LTime.h"

enum CONN_TYPE
{
	CLIENT,
	SERVER,
};

struct NetObject:public LoopObject
{
	int socket;
	int64_t ctime;
	bool isVerifi;
	int type;
	int id;
	void* data;
	// Í¨¹ý LoopObject ¼Ì³Ð
	void init(FactorManager * fm)
	{
		ctime = GetSecend();
		isVerifi = false;
		type = 0;
		id = 0;
		data = nullptr;
	}

	void recycle(FactorManager * fm)
	{

	}
};

#endif