#include "LogModule.h"
#include "MsgModule.h"
#include "ScheduleModule.h"
#include "LPFile.h"
#include "spdlog/spdlog.h"

LogModule::LogModule(BaseLayer * l):BaseModule(l),m_showlog(true)
{
}

LogModule::~LogModule()
{
}

void LogModule::Init()
{
	m_msgModule = GetLayer()->GetModule<MsgModule>();
	m_schedule = GET_MODULE(ScheduleModule);

	m_msgModule->AddMsgCall(L_LOG_INFO, BIND_CALL(OnLog,LogInfo));
	m_schedule->AddTimePointTask(BIND_TIME(OnFlush), -1, 0);

	string dir = LoopFile::GetExecutePath();
	dir.append("logs");
	LoopFile::MakeDir(dir);

	Json::Value jroot;
	LoopFile::ReadJsonInRoot(jroot,"commonconf/Common.json");
	m_showlog = jroot["showlog"].asBool();
}

void LogModule::AfterInit()
{
	auto ser = GetLayer()->GetServer();
	stringstream cons;
	cons <<server_name[ser->type] << "-" << ser->serid;
	m_console = spdlog::stdout_color_st(cons.str());
	string dir = LoopFile::GetExecutePath();
	dir.append("logs/");

	for (size_t i = 0; i < LOG_LEVEL_N; i++)
	{
		stringstream log, file;
		file << dir;
		if (i == 0)
		{
			log << server_name[ser->type] << "-" << ser->serid << "-" << spdlog::level::level_names[spdlog::level::info];
			file << server_name[ser->type] << "-" << ser->serid << "-" << spdlog::level::level_names[spdlog::level::info] << ".txt";
			auto loger = spdlog::daily_date_logger_st(log.str(), file.str());
			m_daily.push_back(loger);
		}
		else if (i == spdlog::level::err)
		{
			log << server_name[ser->type] << "-" << ser->serid << "-" << spdlog::level::level_names[spdlog::level::err];
			file << server_name[ser->type] << "-" << ser->serid << "-" << spdlog::level::level_names[spdlog::level::err] << ".txt";
			auto loger = spdlog::daily_date_logger_st(log.str(), file.str());
			loger->flush_on(spdlog::level::err);
			m_daily.push_back(loger);
		}else
			m_daily.push_back(m_daily[0]);
	}
}

void LogModule::Execute()
{
}

void LogModule::OnLog(LogInfo * info)
{
	if (info->level < 0 || info->level >= spdlog::level::off)
		return;
	if (m_showlog)
		m_console->log((spdlog::level::level_enum)info->level, info->log.str());
	m_daily[info->level]->log((spdlog::level::level_enum)info->level, info->log.str());
}

void LogModule::OnFlush(int64_t & dt)
{
	m_daily[spdlog::level::info]->flush();
}