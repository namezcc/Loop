#ifndef TCP_NET_LAYER_H
#define TCP_NET_LAYER_H
#include "TcpBaseLayer.h"
#include "MsgModule.h"
#include "NetModule.h"
#include <iostream>

class LOOP_EXPORT TcpNetLayer:public TcpBaseLayer
{
public:
	TcpNetLayer(const int& port,ProtoType _ptype=PT_MY_PROTO) :TcpBaseLayer(port, _ptype)
	{
		m_uvloop = (uv_loop_t*)malloc(sizeof(uv_loop_t));
		uv_loop_init(m_uvloop);
	};
	virtual ~TcpNetLayer() {
		uv_loop_close(m_uvloop);
	};


protected:
	virtual void init() {
		auto netmod = CreateModule<NetModule>();
		netmod->SetProtoType(m_protoType);
		netmod->SetBind(m_port, m_uvloop);
	};
	void loop() {
		//std::cout << "TcpNetLayer loop..." << endl;
		uv_run(m_uvloop,uv_run_mode::UV_RUN_NOWAIT);
	};
	void close() {
		
	};
	
protected:
	uv_loop_t* m_uvloop;
};

#endif