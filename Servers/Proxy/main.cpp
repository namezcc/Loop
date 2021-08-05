#include <iostream>
#include "LoopServer.h"
#include "TcpNetLayer.h"
#include "LogicLayer.h"
#include "TransMsgModule.h"
#include "module/NoticeServerModule.h"
#include "TestCallModule.h"
#include "TcpAsioLayer.h"

EXPORT void DLL_START_NAME(int argc, char* args[])
{
	LoopServer ser;
	ser.InitServer(argc, args);
	auto nl = ser.CreateLayer<TcpAsioLayer>(ser.m_port);
	//auto nl = ser.CreateLayer<TcpNetLayer>(ser.m_port);
	auto ll = ser.CreateLayer<LogicLayer>(LY_LOGIC);

	ll->CreateModule<NoticeServerModule>();
	ll->CreateModule<TestCallModule>()->setSend();

	//nl->CreateModule<TestCallModule>();


	ser.BuildPipe(nl, ll);

	ser.Run();
}