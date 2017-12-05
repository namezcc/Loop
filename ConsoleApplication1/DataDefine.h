#ifndef DATA_DEFINE_H
#define DATA_DEFINE_H
#include "FactorManager.h"
#include "LTime.h"

struct NetObject:public LoopObject
{
	int socket;
	int64_t ctime;
	bool isVerifi;

	// Í¨¹ý LoopObject ¼Ì³Ð
	void init(FactorManager * fm)
	{
		ctime = GetSecend();
		isVerifi = false;
	}

	void recycle(FactorManager * fm)
	{

	}
};

#endif