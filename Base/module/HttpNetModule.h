#ifndef HTTP_NET_MODULE_H
#define HTTP_NET_MODULE_H
#include "NetModule.h"

class HttpNetModule:public NetModule
{
public:
	HttpNetModule(BaseLayer* l) :NetModule(l) 
	{};
	~HttpNetModule()
	{};

	static void After_read_CGI(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf);
protected:
	virtual void Init();
	virtual void Execute();
	virtual bool ReadPack(int socket, char* buf, int len);

	bool ReadPackMid(int socket, char* buf, int len, int mid);

	void OnSendHttpMsg(NetMsg* msg);
	void OnConnectPHPCgi(NetServer* ser);

	static void Connect_cb(uv_connect_t* req, int status);
	

};


#endif