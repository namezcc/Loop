#include <iostream>
#include "LoopServer.h"
#include "TcpNetLayer.h"
#include "Test/layer/TestLayer.h"

int main(int argc,char* args[])
{
	
	LoopServer ser;

	ser.InitServer(argc, args);

	auto nl = ser.CreateLayer<TcpNetLayer>(ser.m_port);
	auto ll = ser.CreateLayer<TestLayer>();

	ser.BuildPipe(nl, ll);

	ser.Run();

	while (1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
    return 0;
}