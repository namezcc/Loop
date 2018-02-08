#ifndef TCP_SERVER_H
#define TCP_SERVER_H
#include "Define.h"
#include "BaseModule.h"
#include <uv.h>

class NetModule;

class LOOP_EXPORT TcpServer:public BaseModule
{
public:
	TcpServer(BaseLayer* l):BaseModule(l)
	{};
	~TcpServer() {};

	virtual void Init();
	virtual void AfterInit();

	void SetBind(const int& port, uv_loop_t* loop);
protected:
	void start();
	static void connection_cb(uv_stream_t* serhand, int status);

protected:
	NetModule* m_netModule;
	uv_tcp_t m_hand;
	uv_loop_t* m_uvloop;
	int m_port;
};

#endif