#ifndef TOOL_FUNCTION_H
#define TOOL_FUNCTION_H

#include "Utils.h"

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

#endif
