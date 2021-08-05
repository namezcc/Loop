#ifndef HELP_FUNCTION_H
#define HELP_FUNCTION_H

#include <stdint.h>
#include "Crypto/crchash.h"

uint32_t getPlayerHashIndex(const std::string& str)
{
	return common::Hash32(str) % 65536;
}

//起始时间戳 2019/11/18 00:00:00
#define UID_BEGIN_TIME_STAMP 1574006400

//16位dbid 30位time 12位num
int64_t createPlayerUid(int32_t dbid,int64_t stamp,int32_t addnum)
{
	stamp -= UID_BEGIN_TIME_STAMP;

	int64_t res = ((int64_t)(dbid & 0xFFFF) << 42) | ((stamp & 0x3FFFFFFF) << 12) | (addnum & 0xFFF);
	return res;
}

int32_t getPlayerDbIndexFromUid(int64_t uid)
{
	return (uid >> 42) & 0xFFFF;
}


#endif // !HELP_FUNCTION_H
