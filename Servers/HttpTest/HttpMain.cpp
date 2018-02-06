#include "LoopServer.h"
#include "TcpNetLayer.h"
#include "LogicLayer.h"
#include "ProcessModule.h"
#include "TestProcModule.h"

int main(int argc,char* args[])
{
	LoopServer ser;
	ser.InitServer(argc, args);

	auto nl = ser.CreateLayer<TcpNetLayer>(ser.m_port);
	auto ll = ser.CreateLayer<LogicLayer>(LY_LOGIC);

	ll->CreateModule<ProcessModule>();
	ll->CreateModule<TestProcModule>();

	ser.BuildPipe(nl, ll);
	ser.Run();

	while (1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	return 0;
}