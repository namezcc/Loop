#include "LPFile.h"
#include "DataDefine.h"
#include <io.h>
#include <direct.h>

LoopFile::LoopFile()
{
}

LoopFile::~LoopFile()
{
}

bool LoopFile::ExistFile(string file)
{
	return _access(file.c_str(), 0) == 0;
}

int LoopFile::GetContent(const string& file, NetBuffer & context)
{
	ifstream ifs;
	ifs.open(file.c_str(), ifstream::in | ifstream::binary | ifstream::ate);
	if (ifs.is_open())
	{
		auto len = ifs.tellg();
		ifs.seekg(0, ios::beg);
		context.MakeRoome(len);
		ifs.read(context.buf, len);
		context.use = len;
		ifs.close();
		return 0;
	}
	return -1;
}

void LoopFile::GetRootPath(string & res)
{
	char curpath[MAX_PATH];
	_getcwd(curpath, MAX_PATH);
	string path(curpath);
	auto pos = path.find_first_of("Loop");

	res.assign(path.data(), path.data() + pos);
	res.append("Loop/");
}

string LoopFile::GetExecutePath()
{
	char curpath[MAX_PATH];
#if PLATFORM == PLATFORM_WIN
	GetModuleFileName(NULL, curpath, MAX_PATH);
	string tmp(curpath);
	auto pos = tmp.find_last_of("\\");

	string path(tmp.data(), tmp.data() + pos);
	path.append("\\");
	return path;
#else
	return string();
#endif
}

void LoopFile::MakeDir(const string & path)
{
#if PLATFORM == PLATFORM_WIN
	string sys("if not exist ");
	sys.append(path).append(" mkdir ").append(path);
	system(sys.data());
#else
	string sys("mkdir -p ");
	sys.append(path);
	auto rv = system(sys.data());
	//rv = system("rm -f logs/*");
	(void)rv;
#endif
}