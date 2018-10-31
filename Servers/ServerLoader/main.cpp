#include "dllhelp.h"
#include "json/json.h"
#include "LPFile.h"
#include <fstream>
#include <assert.h>
#include <map>
#include <thread>
#include "cmdline.h"
#include "dump.h"

#if PLATFORM != PLATFORM_WIN
#include <errno.h>
#else
#include <strsafe.h>
#endif

using namespace std;

typedef void(*DLL_START)(int, char*[]);

#define ARG_NUM 7
#define ARG_USE_NUM 6

void LoadServerConf(map<int,string>& conf)
{
	string file;
	LoopFile::GetRootPath(file);
	file.append("commonconf/Server.json");

	ifstream ifs;
	ifs.open(file);

	try
	{
		ifs.is_open();
	}
	catch (const std::exception& e)
	{
		cout << e.what() <<endl;
	}

	Json::Reader reader;
	Json::Value root;
	if (!reader.parse(ifs, root, false))
		cout << reader.getFormattedErrorMessages();

	for (auto& v : root)
		conf[v["type"].asInt()] = v["name"].asString();
}

void StartInitCrash(const std::string& proname,const int32_t& nid)
{
	auto path = LoopFile::GetExecutePath();
	path.append("dump/");
	InitCrash(path, proname+"_"+std::to_string(nid));

#if PLATFORM == PLATFORM_WIN
	//set title

	char title[MAX_PATH] = { 0 };
	sprintf_s(title, "server %s-%d", proname.c_str(), nid);
	//StringCchPrintf(title,MAX_PATH,"%s-%d",)
	::SetConsoleTitle(title);
#endif // PLATFORM == PLATFORM_WIN
}

int main(int argc, char* args[])
{
	if (argc <ARG_NUM || ((argc - 1) % ARG_USE_NUM) != 0)
	{
		cout << "argc num error" << endl;
		assert(0);
	}

	map<int, string> serverConf;
	LoadServerConf(serverConf);

	vector<thread> thrs;
	int num = argc / ARG_USE_NUM;

	for (size_t i = 0; i < num; i++)
	{
		int idx = i*ARG_USE_NUM;
		char* nargs[ARG_NUM];
		nargs[0] = args[0];
		for (size_t i = 1; i < ARG_NUM; i++)
			nargs[i] = args[idx + i];

		cmdline::parser param;
		param.add<int>("port", 'p', "server port", true, 0, cmdline::range(1, 65535));
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
				thrs.emplace_back([i,argc,&args,func]() {
					int idx = i*ARG_USE_NUM;
					char* nargs[ARG_NUM];
					nargs[0] = args[0];
					for (size_t i = 1; i < ARG_NUM; i++)
						nargs[i] = args[idx + i];
					func(ARG_NUM, nargs);
				});
			}
			else
				cout << "dll get func Name error" << endl;
		}
		else
		{
#if PLATFORM == PLATFORM_WIN
    cout << "load dll error: " << GetLastError() << endl;
#else
    cout << "load dll error: " << strerror(errno) << endl;
#endif
    }
	}
	for (auto& t:thrs)
		t.join();
	return 0;
}