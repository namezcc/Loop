#ifndef HTTP_NET_LAYER_H
#define HTTP_NET_LAYER_H

#include "TcpNetLayer.h"
#include "HttpNetModule.h"
#include "HttpServerModule.h"

class HttpNetLayer:public TcpNetLayer
{
public:
	HttpNetLayer(const int& port):TcpNetLayer(port)
	{};
	~HttpNetLayer() 
	{};

protected:
	virtual void init() {
		auto msgmd = CreateModule<MsgModule>();
		CreateModule<HttpServerModule>()->start(m_port, m_uvloop);
		CreateModule<HttpNetModule>()->Setuvloop(m_uvloop);

		msgmd->SetGetLayerFunc([this]() {
			auto it = GetPipes().begin();
			return it->first;
		});
	};
};


#endif