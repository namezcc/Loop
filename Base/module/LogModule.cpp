#include "LogModule.h"
#include "MsgModule.h"
#include "ScheduleModule.h"
#include "LPFile.h"
#include "spdlog/spdlog.h"
#include "JsonHelp.h"
#include "LTime.h"

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

	JsonHelp jhelp;
	if(jhelp.ParseFile(LoopFile::GetRootPath().append("commonconf/Common.json")))
		m_showlog = jhelp.GetMember("showlog")->GetBool();
}

void LogModule::AfterInit()
{
	auto ser = GetLayer()->GetServer();
	auto loopser = getLoopServer();
	auto sername = loopser->getServerName(ser->type);
	stringstream cons;
	cons << "std" << sername << "-" << ser->serid;
	m_console = spdlog::stdout_color_st(cons.str());
	string dir = LoopFile::GetExecutePath();
	dir.append("logs/");

	stringstream log, file;
	file << dir;
	log << sername << "-" << ser->serid;
	file << sername << "-" << ser->serid << "-" << Loop::GetStringTime("%Y.%m.%d.%H.%M.%S") << ".log";
	try
	{
		m_loger = spdlog::basic_logger_st(log.str(), file.str(), true);
		m_loger->flush_on(spdlog::level::err);
	}
	catch (const spdlog::spdlog_ex& e)
	{
		std::cout << e.what() << std::endl;
		loopser->closeServer();
	}

	m_console->log(spdlog::level::level_enum::info, "log init ...");
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
	m_loger->log((spdlog::level::level_enum)info->level, info->log.str());
}

void LogModule::OnFlush(int64_t & dt)
{
	m_loger->flush();
}