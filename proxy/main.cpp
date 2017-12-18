#include <iostream>
#include "LoopServer.h"
#include "TcpNetLayer.h"
#include "LogicLayer.h"

using namespace std;

#define IP "127.0.0.1"
#define PORT 15001

int main()
{
	LoopServer ser;

	auto nl = ser.CreateLayer<TcpNetLayer>(IP,PORT);
	auto ll = ser.CreateLayer<LogicLayer>();

	ser.BuildPipe(nl, ll);

	ser.Run();

	while (1)
	{
		//cout << "Sleep 1 sec ..." << endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
    return 0;
}