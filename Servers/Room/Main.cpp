#include <iostream>
#include "LoopServer.h"
#include "TcpNetLayer.h"
#include "LogicLayer.h"
#include "PlayerModule.h"
#include "SendProxyDbModule.h"

EXPORT void DLL_START_NAME(int argc,char* args[])
{
	LoopServer ser;
	ser.InitServer(argc, args);

	auto nl = ser.CreateLayer<TcpNetLayer>(ser.m_port);
	auto ll = ser.CreateLayer<LogicLayer>(LY_LOGIC);

	ll->CreateModule<PlayerModule>();
	ll->CreateModule<SendProxyDbModule>();

	ser.BuildPipe(nl, ll);
	ser.Run();
}