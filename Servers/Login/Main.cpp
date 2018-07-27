#include <iostream>
#include "LoopServer.h"
#include "TcpNetLayer.h"
#include "LogicLayer.h"
#include "LoginModule.h"
#include "SendProxyDbModule.h"
#include "RedisModule.h"

EXPORT void DLL_START_NAME(int argc,char* args[])
{
	LoopServer ser;
	ser.InitServer(argc, args);

	auto nl = ser.CreateLayer<TcpNetLayer>(ser.m_port);
	auto ll = ser.CreateLayer<LogicLayer>(LY_LOGIC);
	ll->CreateModule<LoginModule>();
	ll->CreateModule<SendProxyDbModule>();
	ll->CreateModule<RedisModule>()->SetConnect("127.0.0.1","",6379);


	ser.BuildPipe(nl, ll);
	ser.Run();
	/*while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}*/
}