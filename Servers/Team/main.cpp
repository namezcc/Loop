#include <iostream>
#include "LoopServer.h"
#include "TcpNetLayer.h"
#include "LogicLayer.h"
#include "TransMsgModule.h"
#include "TeamModule.h"

EXPORT void DLL_START_NAME(int argc, char* args[], int* stop)
{
	LoopServer& ser = *(new LoopServer);
	ser.InitServer(argc, args);
	ser.setStop(stop);

	auto nl = ser.CreateLayer<TcpNetLayer>(ser.m_port);
	auto ll = ser.CreateLayer<LogicLayer>(LY_LOGIC);
	ll->CreateModule<TeamModule>();


	ser.BuildPipe(nl, ll);

	ser.Run();
}