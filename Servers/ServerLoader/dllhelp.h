#ifndef DLL_HELP_H
#define DLL_HELP_H

#include "Define.h"
#include <iostream>

#if PLATFORM == PLATFORM_WIN
#define DYNLIB_HANDLE HINSTANCE
#define DYNLIB_LOAD( a ) LoadLibraryExA( a, NULL, LOAD_WITH_ALTERED_SEARCH_PATH )
#define DYNLIB_GETSYM( a, b ) GetProcAddress( a, b )
#define DYNLIB_UNLOAD( a ) FreeLibrary( a )
#else
#include <dlfcn.h>
#define DYNLIB_HANDLE void*
#define DYNLIB_LOAD( a ) dlopen( a, RTLD_LAZY | RTLD_GLOBAL)
#define DYNLIB_GETSYM( a, b ) dlsym( a, b )
#define DYNLIB_UNLOAD( a ) dlclose( a )
#endif

using namespace std;

class dllhelp
{
public:
	dllhelp(const string& dll)
	{
		name = dll;
#if PLATFORM == PLATFORM_WIN
		name.append(".dll");
#else
    name = "lib"+name;
		name.append(".so");
#endif // PLATFROM

	}
	~dllhelp() 
	{}

	bool Load()
	{
		string path = "./";
		path.append(name);
		mInst = (DYNLIB_HANDLE)DYNLIB_LOAD(path.c_str());
		return mInst != NULL;
	}

	void* GetSymbol(const char* nFuncName)
	{
		return (DYNLIB_HANDLE)DYNLIB_GETSYM(mInst, nFuncName);
	}
private:
	string name;
	DYNLIB_HANDLE mInst;
};

#endif