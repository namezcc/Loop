#ifndef TCP_SERVER_H
#define TCP_SERVER_H
#include "Define.h"
#include "BaseModule.h"
#include <uv.h>

class NetModule;

class TcpServer:public BaseModule
{
public:
	TcpServer(BaseLayer* l):BaseModule(l)
	{};
	~TcpServer() {};

	void Init();
	void Execute() {};

	void start(const int& port)
	{
		struct sockaddr_in addr;
		ASSERT(0 == uv_ip4_addr("0.0.0.0", port, &addr));
		int r;
		r = uv_tcp_init(uv_default_loop(), &m_hand);
		ASSERT(r == 0);
		r = uv_tcp_bind(&m_hand, (const struct sockaddr*) &addr, 0);
		ASSERT(r == 0);
		m_hand.data = this;
		r = uv_listen((uv_stream_t*)&m_hand, 128, connection_cb);
		ASSERT(r == 0);
		std::cout << "start listen port:" << port << std::endl;
	}
protected:
	static void connection_cb(uv_stream_t* serhand, int status);

	

private:
	NetModule* m_netModule;
	uv_tcp_t m_hand;
};

#endif