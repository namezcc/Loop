#ifndef DUMP_H
#define DUMP_H
#include "Define.h"
#include "LTime.h"
#include <boost/filesystem.hpp>
#include <locale>
#include <codecvt>
#include <iostream>
#if PLATFORM == PLATFORM_WIN
#include "client/windows/handler/exception_handler.h"
#else
#include "client/linux/handler/exception_handler.h"
#endif // PLAT

namespace bf = boost::filesystem;

struct CrashContext
{
	std::string path;
	std::string newName;
};

std::string wstos(const std::wstring& str)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.to_bytes(str);
}

std::wstring stows(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(str);
}

#if PLATFORM == PLATFORM_WIN
google_breakpad::ExceptionHandler* handle = NULL;

bool CrashCallBack(const wchar_t *dump_path, const wchar_t *id,
	void *context, EXCEPTION_POINTERS *exinfo,
	MDRawAssertionInfo *assertion,
	bool succeeded)
{
	if (succeeded) {
		printf("dump guid is %ws\n", id);
		CrashContext* ctx = (CrashContext*)context;
		bf::path dmppath(ctx->path);
		std::string wstr = wstos(id);
		wstr.append(".dmp");
		try
		{
			bf::rename(dmppath / wstr, dmppath / ctx->newName);
		}
		catch (const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
		}
	}
	else {
		printf("dump failed\n");
	}
	return succeeded;
}

static void initHandle(CrashContext* ctx)
{
	handle = new google_breakpad::ExceptionHandler(
		stows(ctx->path), NULL, CrashCallBack, ctx,
		google_breakpad::ExceptionHandler::HANDLER_ALL);
}
#else
google_breakpad::ExceptionHandler* handle = NULL;

bool CrashCallBack(const google_breakpad::MinidumpDescriptor& descriptor,
	void* context,
	bool succeeded)
{
	if (succeeded) {
		printf("dump guid is %ws\n", id);
		CrashContext* ctx = (CrashContext*)context;
		bf::path dmpfile(descriptor.path());
		bf::path dmppath(dmpfile.parent_path());
		try
		{
			bf::rename(dmpfile, dmppath / ctx->newName);
		}
		catch (const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
		}
	}
	else {
		printf("dump failed\n");
	}
	return succeeded;
}

static void initHandle(CrashContext* ctx)
{
	google_breakpad::MinidumpDescriptor desc(ctx->path);
	handle = new google_breakpad::ExceptionHandler(desc,NULL, CrashCallBack,ctx,true,-1);
}
#endif

static bool InitCrash(const std::string& dir, const std::string& name)
{
	auto ctx = new CrashContext();
	ctx->path = dir;
	ctx->newName = name +"_"+GetStringTime("%Y-%m-%d_%H_%M_%S")+ ".dmp";
	try
	{
		bf::create_directories(bf::path(dir));
		initHandle(ctx);
	}
	catch (const std::exception& ex)
	{
		std::cout<< "initCrash fail :" << ex.what() << std::endl;
		return false;
	}
	return true;
}

#endif
