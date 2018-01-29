#include <iostream>
#include "LoopServer.h"
#include "TcpNetLayer.h"
#include "LogicLayer.h"
#include "HttpNetLayer.h"
#include "HttpLogicModule.h"
#include "HttpCgiModule.h"
//#include "Test/module/TestModule.h"
//#include "Test/layer/TestLayer.h"

int main(int argc,char* args[])
{
	LoopServer ser;

	ser.InitServer(argc, args);

	auto nl = ser.CreateLayer<HttpNetLayer>(ser.m_port);
	auto ll = ser.CreateLayer<LogicLayer>();

	ll->CreateModule<HttpLogicModule>()->SetWebRoot("web");
	ll->CreateModule<HttpCgiModule>()->ConnectCgi("127.0.0.1", 9000);

	ser.BuildPipe(nl, ll);
	ser.Run();

	while (1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
    return 0;
}