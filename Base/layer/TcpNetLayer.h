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
	TcpNetLayer(const int& port) :m_port(port)
	{
	};
	virtual ~TcpNetLayer() {
	
	};


protected:
	void init() {
		auto msgmd = CreateModule<MsgModule>();
		CreateModule<TcpServer>()->start(m_port);
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
	int m_port;
};

#endif