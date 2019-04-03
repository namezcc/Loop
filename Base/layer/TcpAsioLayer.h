#ifndef TCP_ASIO_LAYER_H
#define TCP_ASIO_LAYER_H

#include "TcpBaseLayer.h"
#include "TcpAsioSessionModule.h"

class LOOP_EXPORT TcpAsioLayer:public TcpBaseLayer
{
public:
	TcpAsioLayer(int32_t port,ProtoType ptype = PT_MY_PROTO):TcpBaseLayer(port,ptype)
	{}

	~TcpAsioLayer()
	{}

private:

	// Í¨¹ý BaseLayer ¼Ì³Ð
	virtual void init() override;
	virtual void loop() override;
	virtual void close() override;

};

#endif
