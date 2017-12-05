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

	N_END,
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

//msg ��Ҫ�ֶ� delete
struct NetMsg
{
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

struct NetServer
{
	int serid;
	int socket;
	int state;
	std::string ip;
	int port;
};

#endif