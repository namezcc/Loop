#include <iostream>
#include "LoopServer.h"
#include "TcpNetLayer.h"
#include "LogicLayer.h"

typedef std::function<void()> Call;

template<typename F,typename... Args>
decltype(auto) Pack(F&& f, Args&&... args)
{
	auto functor = std::bind(std::forward<F>(f),std::forward<Args>(args)...);
	return functor();
}

using namespace std;

struct Test
{
	int _1;
	int _2;
	int _3;
	int _4;
	int _5;
	Lock lock;

	Test()
	{
		cout << "new test" << endl;
		_1 = _2 = _3 = _4 = _5 = 0;
		lock_init(&lock);
	}

	~Test()
	{
		cout << "delete test" << endl;
	}

	Test(const Test& t)
	{
		cout << "copy new test" << endl;
	}

	Test& operator&=(const Test& t)
	{
		cout << "= new test" << endl;
	}

	int call(int a)
	{
		cout<<"Test::call->"<< a << endl;
		return a;
	}

	static void call2(Test* t)
	{
		cout << "Test::call2 --- " <<t->_1<< endl;
	}
};

#define tag1 0x1
#define tag2 0x2
#define tag3 0x4
#define tag4 0x8

void check(int state,int must,int option)
{
	int bres = ((state^must) | option)^option;
	bool res = !bres;
	std::cout << "check state:" << state <<"res:"<< bres<<":"<<res<< endl;
}

#define IP "127.0.0.1"
#define PORT 15001

int main()
{
	/*check(tag1 | tag2 | tag3, tag1 | tag2,0);
	Pack(&check, tag1 | tag2 | tag3, tag1 | tag2, 0);
	Test t;
	int a = Pack(&Test::call, &t, 0x70f0f0f0);
	t._1 = 2;
	Pack(Test::call2, &t);*/

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