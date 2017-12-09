#ifndef TCP_NET_LAYER_H
#define TCP_NET_LAYER_H
#include "BaseLayer.h"
#include "TcpServerModule.h"
#include "MsgModule.h"
#include "NetModule.h"
#include "TcpClientModule.h"
#include <iostream>

class TcpNetLayer:public BaseLayer
{
public:
	TcpNetLayer(const std::string& ip,const int& port) :m_ip(ip),m_port(port)
	{
	};
	virtual ~TcpNetLayer() {
	
	};


protected:
	void init() {
		auto msgmd = CreateModule<MsgModule>();
		CreateModule<TcpServer>()->start(m_ip.c_str(),m_port);
		CreateModule<TcpClientModule>();
		CreateModule<NetModule>();

		msgmd->SetGetLayerFunc([this]() {
			auto it = GetPipes().begin();
			return it->first;
		});
	};
	void loop() {
		//std::cout << "TcpNetLayer loop..." << endl;
		uv_run(uv_default_loop(),uv_run_mode::UV_RUN_NOWAIT);
	};
	void close() {
		
	};
	
private:
	std::string m_ip;
	int m_port;
};

#endif