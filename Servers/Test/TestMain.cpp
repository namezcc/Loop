#include <iostream>
#include "LoopServer.h"
#include "TcpNetLayer.h"
#include "LogicLayer.h"
#include "HttpNetLayer.h"
#include "HttpLogicModule.h"
#include "HttpCgiModule.h"
#include "MysqlModule.h"
#include "RedisModule.h"
#include "TestModule.h"

EXPORT void DLL_START_NAME(int argc,char* args[])
{
	LoopServer ser;
	ser.InitServer(argc, args);

	auto nl = ser.CreateLayer<HttpNetLayer>(ser.m_port);
	auto ll = ser.CreateLayer<LogicLayer>(LY_LOGIC);
	ll->CreateModule<TestModule>();
	ll->CreateModule<MysqlModule>();
	ll->CreateModule<RedisModule>();

	ser.BuildPipe(nl, ll);
	ser.Run();
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}