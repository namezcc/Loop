#ifndef MSG_DEFINE_H
#define MSG_DEFINE_H
#include <string>
#include <sstream>
#include <vector>
#include "protoPB/server/LPBase.pb.h"

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

	L_LOG_INFO,

	//http
	L_HL_GET_MACHINE_LIST,	//获取console服

	L_MYSQL_MSG,
	L_UPDATE_TABLE_GROUP,

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

	N_MYSQL_MSG,
	N_UPDATE_TABLE_GROUP,
	N_ADD_TABLE_GROUP,
	N_CREATE_ACCOUNT,

	N_GET_MYSQL_GROUP,	//获取mysql 数据库 组id

	N_ML_CREATE_ACCOUNT,	//mysql -> login
	N_ML_GET_ACCOUNT,

	N_END,
};
//客户端与服务器之间消息id > 10000
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

struct ServerNode
{
	int type;
	int serid;
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
	std::vector<SHARE<ServerNode>> path;
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

struct LogInfo:public BaseData
{
	int level;
	std::stringstream log;
};

struct PB
{
	static char* PBToChar(google::protobuf::Message& msg,int& msize)
	{
		msize = msg.ByteSize();
		char* buff = new char[msize];
		msg.SerializeToArray(buff, msize);
		return buff;
	}

	static char* PBToChar(google::protobuf::Message& msg, int& msize, const int& expand)
	{
		msize = msg.ByteSize()+expand;
		char* buff = new char[msize];
		msg.SerializeToArray(buff+expand, msize);
		return buff;
	}
};

#endif