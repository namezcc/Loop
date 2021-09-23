#include <iostream>
#include "LoopServer.h"
#include "TcpNetLayer.h"
#include "LogicLayer.h"
#include "MysqlManagerModule.h"
#include "MysqlModule.h"
#include "MysqlLayer.h"
#include "Coro1Module.h"
#include "RedisModule.h"

EXPORT void DLL_START_NAME(int argc,char* args[], int* stop)
{
	LoopServer& ser = *(new LoopServer);
	ser.InitServer(argc, args);
	ser.setStop(stop);

	auto nl = ser.CreateLayer<TcpNetLayer>(ser.m_port);
	auto ll = ser.CreateLayer<LogicLayer>(LY_LOGIC);
	ll->CreateModule<MysqlManagerModule>();
	ll->CreateModule<MysqlModule>();
	//ll->CreateModule<GameTableModule>();
	//ll->CreateModule<Coro1Module>();

	ser.BuildPipe(nl, ll);

	
	if (ser.getServerNode().type == LOOP_MYSQL)
	{
		for (size_t i = 0; i < 4; i++)
		{
			auto ml = ser.CreateLayer<MysqlLayer>();
			ser.BuildPipe(ll, ml);
		}
	}

	ser.Run();
}