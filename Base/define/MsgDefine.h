#ifndef MSG_DEFINE_H
#define MSG_DEFINE_H
#include <string>

enum MSG_FROM_LAYER
{
	L_BEGAN = 1,
	L_SOCKET_CONNET,
	L_SOCKET_CLOSE,
	L_SOCKET_SEND_DATA,

	L_TO_CONNET_SERVER,
	L_SERVER_CONNECTED,
	L_SERVER_CLOSE,

	L_END,
};

enum MSG_FROM_NET
{
	N_BEGAN = MSG_FROM_LAYER::L_END,

	N_REGISTE_SERVER,
	N_TRANS_SERVER_MSG,

	N_END,
};

enum BASE_EVENT
{
	E_SERVER_CONNECT,
	E_SERVER_CLOSE,

};

typedef struct _BaseMsg
{
	int msgId;
	void* data;
}BaseMsg;

struct NetSocket
{
	NetSocket(int sk) :socket(sk) {};
	int socket;
};

struct NetMsg
{
	~NetMsg()
	{
		if (msg)
			delete msg;
	};
	int socket;
	int mid;
	int len;
	char* msg;
};

enum CONN_STATE
{
	CONNECT,
	CLOSE,
};

enum SERVER_TYPE
{



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