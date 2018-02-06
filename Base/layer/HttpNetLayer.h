#ifndef HTTP_NET_LAYER_H
#define HTTP_NET_LAYER_H
#include "TcpNetLayer.h"

class LOOP_EXPORT HttpNetLayer:public TcpNetLayer
{
public:
	HttpNetLayer(const int& port):TcpNetLayer(port)
	{
	};
	~HttpNetLayer() 
	{};

protected:
	virtual void init();
};


#endif