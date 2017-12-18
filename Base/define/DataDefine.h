#ifndef DATA_DEFINE_H
#define DATA_DEFINE_H
#include "FactorManager.h"
#include "LTime.h"

enum CONN_TYPE
{
	CLIENT,
	SERVER,
};

enum CONN_STATE
{
	CONNECT,
	CLOSE,
};

enum SERVER_TYPE
{
	LOOP_PROXY,
	LOOP_GAME,
	LOOP_GATE,
};

struct NetSocket
{
	NetSocket(int sk) :socket(sk) {};
	int socket;
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

struct NetServer
{
	int type;
	int serid;
	int socket;
	int state;
	std::string ip;
	int port;
};

struct TransHead
{
	int size;
	int index;
};

struct ServerNode
{
	int type;
	int serid;
};



#endif