#include "LoopServer.h"
#include "LogicLayer.h"
#include "HttpNetLayer.h"
#include "HttpLogicModule.h"
#include "HttpCgiModule.h"
#include "TransMsgModule.h"
#include "SessionModule.h"
#include "MysqlModule.h"

int main(int argc, char* args[])
{
	LoopServer ser;
	ser.InitServer(argc, args);

	auto nl = ser.CreateLayer<HttpNetLayer>(ser.m_port);
	auto ll = ser.CreateLayer<LogicLayer>(LY_HTTP_LOGIC);

	ll->CreateModule<HttpLogicModule>()->SetWebRoot("Servers/Master/web");
	ll->CreateModule<HttpCgiModule>()->ConnectCgi("127.0.0.1", 9000);
	ll->CreateModule<SessionModule>();
	ll->CreateModule<MysqlModule>()->SetConnect("master","127.0.0.1","root","123456");

	ser.BuildPipe(nl, ll);
	ser.Run();
	while (1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	return 0;
}