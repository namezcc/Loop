#ifndef LP_FILE_H
#define LP_FILE_H
#include <iostream>
#include <fstream>
#include "Define.h"

using namespace std;

struct NetBuffer;

class LOOP_EXPORT LoopFile
{
public:
	LoopFile();
	~LoopFile();

	static bool ExistFile(string file);
	static int GetContent(const string& file, NetBuffer& context);
	static std::string GetRootPath();
	static string GetExecutePath();
	static void MakeDir(const string& path);
private:

};

#endif 