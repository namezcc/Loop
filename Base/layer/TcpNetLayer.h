#ifndef TCP_NET_LAYER_H
#define TCP_NET_LAYER_H
#include "BaseLayer.h"
#include "TcpServerModule.h"
#include "MsgModule.h"
#include "NetModule.h"
#include "TcpClientModule.h"
#include <iostream>

class LOOP_EXPORT TcpNetLayer:public BaseLayer
{
public:
	TcpNetLayer(const int& port) :m_port(port)
	{
		m_uvloop = (uv_loop_t*)malloc(sizeof(uv_loop_t));
		uv_loop_init(m_uvloop);
	};
	virtual ~TcpNetLayer() {
		uv_loop_close(m_uvloop);
	};


protected:
	virtual void init() {
		auto msgmd = CreateModule<MsgModule>();
		CreateModule<TcpServer>()->start(m_port,m_uvloop);
		CreateModule<TcpClientModule>()->Setuvloop(m_uvloop);
		CreateModule<NetModule>()->Setuvloop(m_uvloop);

		msgmd->SetGetLayerFunc([this]() {
			auto it = GetPipes().begin();
			return it->first;
		});
	};
	void loop() {
		//std::cout << "TcpNetLayer loop..." << endl;
		uv_run(m_uvloop,uv_run_mode::UV_RUN_NOWAIT);
	};
	void close() {
		
	};
	
protected:
	int m_port;
	uv_loop_t* m_uvloop;
};

#endif