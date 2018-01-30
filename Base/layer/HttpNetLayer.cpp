#include "HttpNetLayer.h"
#include "HttpNetModule.h"
#include "HttpServerModule.h"

void HttpNetLayer::init()
{
	auto msgmd = CreateModule<MsgModule>();
	CreateModule<HttpServerModule>()->start(m_port, m_uvloop);
	CreateModule<HttpNetModule>()->Setuvloop(m_uvloop);

	msgmd->SetGetLayerFunc([this]() {
		auto it = GetPipes().begin();
		return it->first;
	});
}
