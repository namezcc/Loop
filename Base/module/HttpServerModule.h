#ifndef HTTP_SERVER_MODULE_H
#define HTTP_SERVER_MODULE_H

#include "TcpServerModule.h"

class LOOP_EXPORT HttpServerModule:public TcpServer
{
public:
	HttpServerModule(BaseLayer* l):TcpServer(l)
	{};
	~HttpServerModule() {};

protected:
	virtual void Init();
};


#endif