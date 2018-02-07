#include "LoopServer.h"
#include "TcpNetLayer.h"
#include "LogicLayer.h"
#include "ProcessModule.h"
#include "TestProcModule.h"

EXPORT void DLL_START_NAME(int argc,char* args[])
{
	LoopServer ser;
	ser.InitServer(argc, args);

	auto nl = ser.CreateLayer<TcpNetLayer>(ser.m_port);
	auto ll = ser.CreateLayer<LogicLayer>(LY_LOGIC);

	ll->CreateModule<ProcessModule>();
	ll->CreateModule<TestProcModule>();

	ser.BuildPipe(nl, ll);
	ser.Run();

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}