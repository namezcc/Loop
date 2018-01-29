#ifndef LP_FILE_H
#define LP_FILE_H
#include <fstream>

using namespace std;

class NetBuffer;

class LoopFile
{
public:
	LoopFile();
	~LoopFile();

	static bool ExistFile(string file);
	static int GetContent(const string& file, NetBuffer& context);

private:

};

#endif 