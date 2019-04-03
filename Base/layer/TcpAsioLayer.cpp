#include "TcpAsioLayer.h"
#include "MsgModule.h"

void TcpAsioLayer::init()
{
	auto asmod = CreateModule<TcpAsioSessionModule>();
	asmod->SetBind(m_port);
	asmod->SetProtoType(m_protoType);
}

void TcpAsioLayer::loop()
{
}

void TcpAsioLayer::close()
{
}
