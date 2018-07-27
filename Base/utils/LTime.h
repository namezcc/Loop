#ifndef L_TIME_H
#define L_TIME_H
#include <chrono>
#include <string>
using namespace std;

#if PLATFORM == PLATFORM_LINUX
#define localtime_s(tm,tt) localtime_r(tt,tm)
#endif

static int64_t GetMilliSecend()
{
	return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
}

static int64_t GetSecend()
{
	return chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count();
}

static std::string GetStringTime(const std::string& fmt="%Y-%m-%d %H:%M:%S")
{
	char tbuff[256];
	time_t ttm = time(NULL);
	tm tmm;
	localtime_s(&tmm, &ttm);
	strftime(tbuff, 256, fmt.c_str(), &tmm);
	return std::string(tbuff);
}

#endif