#include "HttpNetLayer.h"
#include "HttpNetModule.h"
#include "HttpServerModule.h"

void HttpNetLayer::init()
{
	auto msgmd = CreateModule<MsgModule>();

	CreateModule<HttpServerModule>()->SetBind(m_port, m_uvloop);
	CreateModule<HttpNetModule>()->Setuvloop(m_uvloop);
}
