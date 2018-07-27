#ifndef LP_FILE_H
#define LP_FILE_H
#include <iostream>
#include <fstream>
#include "Define.h"
#include "json/json.h"

using namespace std;

struct NetBuffer;

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
	static void ReadJson(Json::Value& root,const std::string& file);
	static void ReadJsonInRoot(Json::Value& root,const std::string& file);
private:

};

#endif 