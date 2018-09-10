#include <iostream>
#include "LoopServer.h"
#include "TcpNetLayer.h"
#include "LogicLayer.h"
#include "LoginModule.h"
#include "SendProxyDbModule.h"
#include "RedisModule.h"
#include "Coro3Module.h"
#include "RoomStateModule.h"

EXPORT void DLL_START_NAME(int argc,char* args[])
{
	LoopServer ser;
	ser.InitServer(argc, args);

	auto nl = ser.CreateLayer<TcpNetLayer>(ser.m_port);
	auto ll = ser.CreateLayer<LogicLayer>(LY_LOGIC);
	ll->CreateModule<LoginModule>();
	ll->CreateModule<SendProxyDbModule>();
	ll->CreateModule<RedisModule>()->SetConnect("127.0.0.1","",6379);
	ll->CreateModule<RoomStateModule>();

	ser.BuildPipe(nl, ll);
	ser.Run();
}