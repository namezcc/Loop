#include "HttpNetLayer.h"
#include "NetModule.h"

void HttpNetLayer::init()
{
	CreateModule<MsgModule>();
	auto netmod = CreateModule<NetModule>();
	netmod->SetProtoType(m_protoType);
	netmod->SetBind(m_port, m_uvloop,m_role);
}
