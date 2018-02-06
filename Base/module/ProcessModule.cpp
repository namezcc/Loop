#include "ProcessModule.h"
#include <direct.h>

ProcessModule::ProcessModule(BaseLayer * l):BaseModule(l)
{
}

ProcessModule::~ProcessModule()
{
}

bool ProcessModule::CreateServer(const string & name, const string & line)
{
	bool ret=true;
	char curpath[MAX_PATH];
#ifdef _WIN32
// _WIN32
	GetModuleFileName(NULL, curpath,MAX_PATH);
	string tmp(curpath);
 	auto pos = tmp.find_last_of("\\");

	string path(tmp.data(), tmp.data()+pos);
	path.append("/").append(name).append(".exe ");

	path.append(line);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));

	if (!CreateProcess(NULL,(LPSTR)path.data(),NULL,NULL,false,CREATE_NEW_CONSOLE,NULL,NULL,&si,&pi))
	{
		ret = false;
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
#else
//linux


#endif
	return ret;
}

void ProcessModule::Init()
{
}

void ProcessModule::Execute()
{
}
