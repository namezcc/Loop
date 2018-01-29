#ifndef MSG_DEFINE_H
#define MSG_DEFINE_H
#include <string>
#include "protoPB/LPBase.pb.h"
#include "protoPB/LPDefine.pb.h"

enum MSG_FROM_LAYER
{
	L_BEGAN = 1,
	L_SOCKET_CONNET,
	L_SOCKET_CLOSE,
	L_SOCKET_SEND_DATA,
	L_SOCKET_SEND_HTTP_DATA,

	L_TO_CONNET_SERVER,
	L_SERVER_CONNECTED,

	L_CONNECT_PHP_CGI,
	L_PHP_CGI_CONNECTED,

	L_END,
};

enum MSG_FROM_NET
{
	N_BEGAN = MSG_FROM_LAYER::L_END,

	N_REGISTE_SERVER,
	N_TRANS_SERVER_MSG,
	N_TRANS_TEST,

	N_RECV_HTTP_MSG,
	N_RECV_PHP_CGI_MSG,

	N_END,
};

enum BASE_EVENT
{
	E_SOCKEK_CONNECT,

	E_SERVER_CONNECT,
	E_SERVER_CLOSE,

	E_CLIENT_HTTP_CONNECT,
	E_CLIENT_HTTP_CLOSE,

	E_PHP_CGI_CONNECT,
	E_PHP_CGI_CLOSE,
};

struct BaseData
{
	virtual ~BaseData()
	{}
};

typedef struct _BaseMsg
{
	virtual ~_BaseMsg()
	{
		delete data;
	}
	int msgId;
	BaseData* data;
}BaseMsg;

struct NetMsg:public BaseData
{
	~NetMsg()
	{
		if (msg)
			delete[] msg;
	};
	int socket;
	int mid;
	int len;
	char* msg;
};

struct NetSocket:public BaseData
{
	NetSocket(int sk) :socket(sk) {};
	int socket;
};

struct NetServer:public BaseData
{
	int type;
	int serid;
	int socket;
	int state;
	std::string ip;
	int port;
};

struct PB
{
	static char* PBToChar(google::protobuf::Message& msg,int& msize)
	{
		msize = msg.ByteSize();
		assert(msize != 0);
		char* buff = new char[msize];
		assert(msg.SerializeToArray(buff, msize));
		return buff;
	}
};

#endif