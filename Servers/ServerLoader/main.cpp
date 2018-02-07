#include "dllhelp.h"
#include "json/json.h"
#include "LPFile.h"
#include <fstream>
#include <assert.h>
#include <map>
#include <thread>
#include "cmdline.h"

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
	assert(ifs.is_open());

	Json::Reader reader;
	Json::Value root;
	assert(reader.parse(ifs, root, false));

	for (auto& v : root)
		conf[v["type"].asInt()] = v["name"].asString();
}

int main(int argc, char* args[])
{
	map<int, string> serverConf;
	LoadServerConf(serverConf);

	if (argc <ARG_NUM || ((argc - 1) % ARG_USE_NUM) != 0)
	{
		cout << "argc num error" << endl;
		assert(0);
	}

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
		
		string dllname = serverConf[type];
		assert(dllname.size() > 0);

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
			cout << "load dll error: " << GetLastError() << endl;
	}
	for (auto& t:thrs)
		t.join();
	return 0;
}