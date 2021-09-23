#include <iostream>
#include "LoopServer.h"
#include "TcpNetLayer.h"
#include "UdpNetLayer.h"
#include "LogicLayer.h"
#include "TransMsgModule.h"
#include "BattleSceneInfoModule.h"
#include "module/ProxyNodeModule.h"
#include "UdpNetSockModule.h"

EXPORT void DLL_START_NAME(int argc, char* args[], int* stop)
{
	LoopServer& ser = *(new LoopServer);
	ser.InitServer(argc, args);
	ser.setStop(stop);

	auto& conf = ser.GetConfig();

	auto nl = ser.CreateLayer<TcpNetLayer>(ser.m_port);
	auto ll = ser.CreateLayer<LogicLayer>(LY_LOGIC);
	auto udpl = ser.CreateLayer<UdpNetLayer>(conf.udpAddr.port);

	ll->CreateModule<BattleSceneInfoModule>();
	ll->CreateModule<ProxyNodeModule>();
	ll->CreateModule<UdpNetSockModule>();

	ser.BuildPipe(nl, ll);
	ser.BuildPipe(udpl, ll);
	ser.Run();
}