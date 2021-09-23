#include "LoopServer.h"
#include "LogicLayer.h"
#include "HttpNetLayer.h"
#include "HttpLogicModule.h"
#include "HttpCgiModule.h"
#include "TransMsgModule.h"
#include "SessionModule.h"
#include "MysqlModule.h"
#include "HServerInfoModule.h"
#include "ServerInfoModule.h"
#include "TcpNetLayer.h"

EXPORT void DLL_START_NAME(int argc, char* args[], int* stop)
{
	LoopServer& ser = *(new LoopServer);
	ser.InitServer(argc, args);
	ser.setStop(stop);

	auto nl = ser.CreateLayer<TcpNetLayer>(ser.m_port);
	//auto ll = ser.CreateLayer<LogicLayer>(LY_HTTP_LOGIC);

	/*ll->CreateModule<HttpLogicModule>()->SetWebRoot("Servers/Master/web");
	ll->CreateModule<HttpCgiModule>()->ConnectCgi("127.0.0.1", 9000);
	ll->CreateModule<SessionModule>();
	ll->CreateModule<MysqlModule>();
	ll->CreateModule<HServerInfoModule>();*/

	auto llogic = ser.CreateLayer<LogicLayer>(LY_LOGIC);
	llogic->CreateModule<MysqlModule>();
	llogic->CreateModule<ServerInfoModule>();

	ser.BuildPipe(nl, llogic);

	ser.Run();
}