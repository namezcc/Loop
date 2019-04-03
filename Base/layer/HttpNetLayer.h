#ifndef HTTP_NET_LAYER_H
#define HTTP_NET_LAYER_H
#include "TcpNetLayer.h"

class LOOP_EXPORT HttpNetLayer:public TcpNetLayer
{
public:
	HttpNetLayer(const int32_t& port):TcpNetLayer(port, PT_HTTP)
	{
	};
	~HttpNetLayer() 
	{};

protected:
	virtual void init();
};


#endif