#include "LPFile.h"
#include "DataDefine.h"
#include <boost/filesystem.hpp>

LoopFile::LoopFile()
{
}

LoopFile::~LoopFile()
{
}

bool LoopFile::ExistFile(string file)
{
	return boost::filesystem::exists(file);
}

int LoopFile::GetContent(const string& file, NetBuffer & context)
{
	ifstream ifs;
	try
	{
		ifs.open(file.c_str(), ifstream::in | ifstream::binary | ifstream::ate);
		if (ifs.is_open())
		{
			uint32_t len = static_cast<uint32_t>(ifs.tellg());
			ifs.seekg(0, ios::beg);
			context.MakeRoome(len);
			ifs.read(context.buf, len);
			context.use = len;
			ifs.close();
			return 0;
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	return -1;
}

void LoopFile::GetRootPath(string & res)
{
	try
	{
		boost::filesystem::path p = boost::filesystem::initial_path();
		while (p.has_parent_path())
		{
			if (p.leaf() == "Loop")
				break;
			p.remove_leaf();
		}	
		res = move(p.string());
		res.append("/");
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
}

std::string LoopFile::GetRootPath2()
{
	try
	{
		boost::filesystem::path p = boost::filesystem::initial_path();
		while (p.has_parent_path())
		{
			if (p.leaf() == "Loop")
				break;
			p.remove_leaf();
		}
		auto res = p.string();
		res.append("/");
		return res;
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	return "";
}

string LoopFile::GetExecutePath()
{
	std::string path;
	GetRootPath(path);
	path.append("_out/");
#if PLATFORM == PLATFORM_WIN
	path.append("Debug/");
#endif
	return path;
}

void LoopFile::MakeDir(const string & path)
{
	try
	{
		boost::filesystem::create_directories(boost::filesystem::path(path));
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
}

void LoopFile::ReadJson(Json::Value& root,const std::string& file)
{
	ifstream ifs;
	try
	{
		ifs.open(file);
		ifs.is_open();
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
		return;
	}
	Json::CharReaderBuilder readerBuilder;
	std::string err;
	if (!Json::parseFromStream(readerBuilder, ifs, &root, &err))
		cout << err << endl;
}

void LoopFile::ReadJsonInRoot(Json::Value& root,const std::string& file)
{
	string rpath;
	LoopFile::GetRootPath(rpath);
	rpath.append(file);
	ReadJson(root,rpath);
}