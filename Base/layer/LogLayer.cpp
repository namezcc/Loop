#include "LogLayer.h"
#include "MsgModule.h"
#include "LogModule.h"
#include "ScheduleModule.h"

LogLayer::LogLayer():BaseLayer(LY_LOG)
{
}

LogLayer::~LogLayer()
{
}

void LogLayer::init()
{
	CreateModule<MsgModule>();
	CreateModule<LogModule>();
	CreateModule<ScheduleModule>();
}

void LogLayer::loop()
{
}

void LogLayer::close()
{
}

void LogLayer::GetDefaultTrans(int & ltype, int & lid)
{
}