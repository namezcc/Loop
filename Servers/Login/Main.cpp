﻿#include <iostream>
#include "LoopServer.h"
#include "TcpNetLayer.h"
#include "LogicLayer.h"
#include "LoginModule.h"
#include "RedisModule.h"
#include "Coro3Module.h"
#include "RoomStateModule.h"
#include "TcpAsioLayer.h"

EXPORT void DLL_START_NAME(int argc,char* args[],int* stop)
{
	LoopServer& ser = *(new LoopServer);
	ser.InitServer(argc, args);
	ser.setStop(stop);

	auto nl = ser.CreateLayer<TcpNetLayer>(ser.m_port);
	//auto nl = ser.CreateLayer<TcpAsioLayer>(ser.m_port);
	auto ll = ser.CreateLayer<LogicLayer>(LY_LOGIC);
	ll->CreateModule<LoginModule>();
	ll->CreateModule<RedisModule>();
	ll->CreateModule<RoomStateModule>();



	ser.BuildPipe(nl, ll);
	ser.Run();
}