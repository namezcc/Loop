#include "dllhelp.h"
#include "LPFile.h"
#include <fstream>
#include <assert.h>
#include <map>
#include <thread>
#include "cmdline.h"
//#include "dump.h"
#include "JsonHelp.h"
#include <csignal>

#if PLATFORM != PLATFORM_WIN
#include <errno.h>
#else
#include <strsafe.h>
#endif

using namespace std;

typedef void(*DLL_START)(int, char*[],int*);

#define ARG_NUM 5
#define ARG_USE_NUM 4

void LoadServerConf(map<int,string>& conf)
{
	JsonHelp jhelp;
	if (!jhelp.ParseFile(LoopFile::GetRootPath().append("commonconf/Server.json")))
		exit(-1);
	for (auto& v : jhelp.GetDocument().GetArray())
		conf[v["type"].GetInt()] = v["dll"].GetString();
}

void StartInitCrash(const std::string& proname,const int32_t& nid)
{
	/*auto path = LoopFile::GetExecutePath();
	path.append("dump/");
	InitCrash(path, proname+"_"+std::to_string(nid));*/

#if PLATFORM == PLATFORM_WIN
	//set title

	char title[MAX_PATH] = { 0 };
	sprintf_s(title, "server %s-%d", proname.c_str(), nid);
	//StringCchPrintf(title,MAX_PATH,"%s-%d",)
	::SetConsoleTitle(title);
#endif // PLATFORM == PLATFORM_WIN
}

int32_t stopserver = 0;

void signalHandle(int signum)
{
	printf("signum %d\n", signum);
	stopserver = 1;
}

int main(int argc, char* args[])
{
	if (argc <ARG_NUM || ((argc - 1) % ARG_USE_NUM) != 0)
	{
		cout << "argc num error" << endl;
		assert(0);
	}

	signal(SIGINT, signalHandle);
	signal(SIGTERM, signalHandle);

	LoopFile::setRootPath(args[0]);

	map<int, string> serverConf;
	LoadServerConf(serverConf);

	vector<thread> thrs;
	int num = argc / ARG_USE_NUM;

	for (int i = 0; i < num; i++)
	{
		int idx = i*ARG_USE_NUM;
		char* nargs[ARG_NUM];
		nargs[0] = args[0];
		for (size_t i = 1; i < ARG_NUM; i++)
			nargs[i] = args[idx + i];

		cmdline::parser param;
		//param.add<int>("port", 'p', "server port", true, 0, cmdline::range(1, 65535));
		param.add<int>("type", 't', "server type", true, 0);
		param.add<int>("id", 'n', "server port", true, 0);
		param.parse_check(ARG_NUM, nargs);
		auto type = param.get<int>("type");
		auto nid = param.get<int>("id");
		string dllname = serverConf[type];
		assert(dllname.size() > 0);
		if (i==0)
		{
			StartInitCrash(dllname,nid);
		}

		dllhelp dll(dllname);
		if (dll.Load())
		{
			DLL_START func = (DLL_START)dll.GetSymbol(TOSTR(DLL_START_NAME));
			if (func)
			{
				if (num == 1)
				{
					func(ARG_NUM, args, &stopserver);
					continue;
				}

				thrs.emplace_back([i,argc,&args,func]() {
					int idx = i*ARG_USE_NUM;
					char* nargs[ARG_NUM];
					nargs[0] = args[0];
					for (size_t i = 1; i < ARG_NUM; i++)
						nargs[i] = args[idx + i];
					func(ARG_NUM, nargs, &stopserver);
				});
			}
			else
				cout << "dll get func Name error" << endl;
		}
		else
		{
#if PLATFORM == PLATFORM_WIN
    cout << "load dll error: " << GetLastError() << dllname << endl;
#else
    cout << "load dll error: " << strerror(errno) << dllname << endl;
#endif
    }
	}
	for (auto& t:thrs)
		t.join();
	return 0;
}