#include "LPFile.h"
#include "DataDefine.h"
#include <io.h>

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
		streamsize read;
		ifs.read(context.buf, len);
		context.use = len;
		ifs.close();
		return 0;
	}
	return -1;
}
