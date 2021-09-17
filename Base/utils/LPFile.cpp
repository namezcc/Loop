#include "LPFile.h"
#include "DataDefine.h"
#include <boost/filesystem.hpp>

std::string LoopFile::root_path;

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

std::string LoopFile::GetRootPath()
{
	/*try
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
	}*/
	//return "";
	return root_path;
}

string LoopFile::GetExecutePath()
{
	std::string path = GetRootPath();
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

void LoopFile::setRootPath(const string & path)
{
	boost::filesystem::path p(path);

	while (p.has_parent_path())
	{
		if (p.leaf() == "_out")
		{
			p.remove_leaf();
			break;
		}
		p.remove_leaf();
	}
	auto res = p.string();
	res.append("/");

	root_path = res;
}
