#include "ProcessModule.h"
#include "MsgModule.h"
#include "LogModule.h"
#include "LPFile.h"
#include <boost/process.hpp>
#include <boost/filesystem.hpp>

#include <boost/process/windows.hpp>

#include "JsonHelp.h"

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

	JsonHelp jhelp;
	if (!jhelp.ParseFile(LoopFile::GetRootPath().append("commonconf/Common.json")))
		exit(-1);
	m_logdir = LoopFile::GetRootPath().append(jhelp.GetMember("logdir")->GetString());
	LoopFile::MakeDir(m_logdir);

	JsonHelp jhelp2;
	if (jhelp2.ParseFile(LoopFile::GetRootPath().append("commonconf/Server.json")))
	{
		for (auto& v : jhelp2.GetDocument().GetArray())
			m_server_name[v["type"].GetInt()] = v["name"].GetString();
	}
}

void ProcessModule::Execute()
{
}

void ProcessModule::CreateServer(const int32_t& sertype, const int32_t& nid)
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
	
	std::string logname = m_server_name[sertype];
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
	auto logfile = logname +"_"+Loop::GetStringTime("%Y-%m-%d_%H_%M_%S")+".log";
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

std::string ProcessModule::runProcess(const std::string & cmd, const std::vector<std::string>& args)
{
	char buf[4096] = {};

	bp::ipstream p1;

	std::error_code ec;
	try
	{
		bp::system(cmd.c_str(), args, bp::std_out > p1, ec);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	if (ec) {
		std::cerr << "run process error code " << ec << ec.message() << '\n';
	}
	
	std::string res;

	while (!p1.eof())
	{
		p1.read(buf, sizeof(buf));
		res.append(buf);
	}

	return res;
}

void ProcessModule::runProcessAndDetach(const std::string & cmd, const std::vector<std::string>& args)
{
	std::error_code ec;
	try
	{
		bp::system(cmd.c_str(), args, ec);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	if (ec) {
		std::cerr << "run process error code " << ec << '\n';
	}
}
