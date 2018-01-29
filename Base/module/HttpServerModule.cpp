#include "HttpServerModule.h"
#include "HttpNetModule.h"

void HttpServerModule::Init()
{
	m_netModule = GetLayer()->GetModule<HttpNetModule>();
}