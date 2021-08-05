#ifndef LOG_MODULE_H
#define LOG_MODULE_H
#include "BaseModule.h"

class MsgModule;
class ScheduleModule;

#define LOG_LEVEL_N 7

namespace spdlog
{
	class logger;
}

class LogModule:public BaseModule
{
public:
	LogModule(BaseLayer* l);
	~LogModule();

private:


	// ͨ�� BaseModule �̳�
	virtual void Init() override;
	virtual void AfterInit()override;
	virtual void Execute() override;

	void OnLog(LogInfo* info);
	void OnFlush(int64_t& dt);

private:
	MsgModule* m_msgModule;
	ScheduleModule* m_schedule;

	SHARE<spdlog::logger> m_console;
	vector<SHARE<spdlog::logger>> m_daily;

	std::map<int32_t, std::string> m_server_name;

	bool m_showlog;
};

#endif