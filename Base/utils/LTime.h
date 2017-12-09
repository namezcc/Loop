#ifndef L_TIME_H
#define L_TIME_H
#include <chrono>
using namespace std;

static int64_t GetMilliSecend()
{
	return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
}

static int64_t GetSecend()
{
	return chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count();
}

#endif