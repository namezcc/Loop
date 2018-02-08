#ifndef LP_FILE_H
#define LP_FILE_H
#include <fstream>
#include "Define.h"

using namespace std;

class NetBuffer;

class LOOP_EXPORT LoopFile
{
public:
	LoopFile();
	~LoopFile();

	static bool ExistFile(string file);
	static int GetContent(const string& file, NetBuffer& context);
	static void GetRootPath(string& res);
	static string GetExecutePath();
	static void MakeDir(const string& path);
private:

};

#endif 