#include "ProcessModule.h"
#include "MsgModule.h"
#include "LogModule.h"
#include "LPFile.h"
#include <boost/process.hpp>
#include <boost/filesystem.hpp>

namespace bp = boost::process;
namespace bf = boost::filesystem;

ProcessModule::ProcessModule(BaseLayer * l):BaseModule(l)
{
}

ProcessModule::~ProcessModule()
{
}

void ProcessModule::Init()
{
	m_msgModule = GET_MODULE(MsgModule);

	Json::Value jroot;
	LoopFile::ReadJsonInRoot(jroot,"commonconf/Common.json");
	m_logdir = jroot["logdir"].asString();

	std::string root;
	LoopFile::GetRootPath(root);
	root.append(m_logdir);
	m_logdir = move(root);
	LoopFile::MakeDir(m_logdir);
}

void ProcessModule::Execute()
{
}

void ProcessModule::CreateServer(const SERVER_TYPE& sertype, const int32_t& nid, const int32_t& port)
{
	if (sertype <= SERVER_TYPE::LOOP_SERVER_NONE || sertype >= SERVER_TYPE::LOOP_SERVER_END)
	{
		LP_ERROR << "CreateServer error sertype:" << sertype;
		return;
	}
	std::vector<std::string> args;
	args.push_back("-t");
	args.push_back(Loop::Cvto<std::string>((int32_t)sertype));
	args.push_back("-n");
	args.push_back(Loop::Cvto<std::string>(nid));
	args.push_back("-p");
	args.push_back(Loop::Cvto<std::string>(port));
	std::string logname = server_name[sertype];
	logname.append("_").append(Loop::Cvto<std::string>(nid));
#if PLATFORM == PLATFORM_WIN
	CreateLoopProcess("Server.exe", logname, args);
#else
	CreateLoopProcess("Server", logname, args);
#endif // PLA
}

void ProcessModule::CreateLoopProcess(const std::string& proname, const std::string& logname, const std::vector<std::string>& args)
{
	m_error.clear();
	auto exec = LoopFile::GetExecutePath();
	exec.append(proname);
	auto logfile = logname +"_"+GetStringTime("%Y-%m-%d_%H_%M_%S")+".log";
	std::error_code ec;
	try
	{
		bf::path p(m_logdir);
		p /= logfile;
		bp::spawn(exec,args,(bp::std_err & bp::std_out) > p,bp::std_in<bp::null,ec);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	if (ec) {
		std::cerr << "create process error code "<< ec << '\n';
	}
}