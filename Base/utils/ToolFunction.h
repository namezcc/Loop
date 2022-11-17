#ifndef TOOL_FUNCTION_H
#define TOOL_FUNCTION_H

#include "Utils.h"
#include <vector>

struct CoroTool
{
	static void RequestFailCall(const std::function<void()>& cofunc,const std::function<void()>& failcall)
	{
		bool ret = false;
		ExitCall call([&ret,failcall]() {
			if (!ret)
				failcall();
		});
		cofunc();
		ret = true;
	}
};

std::string getLocalIp();

template<typename T>
int32_t randWeightPair(const std::vector<std::pair<int32_t, T>>& vec)
{
	auto rval = rand() % vec[vec.size() - 1].first;
	for (size_t i = 0; i < vec.size(); i++)
	{
		if (vec[i].first > rval)
			return i;
	}
	return 0;
}

#endif
