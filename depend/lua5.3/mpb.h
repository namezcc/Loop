#ifndef MPB_H
#define MPB_H

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

typedef struct 
{
	char* buf;
	int len;
	int pos;
	int lastTag;
	int nextTag;
	int curLimit;
	int hashNextTag;
	int type;
}OutPutStream;

LUALIB_API int (luaopen_mpb) (lua_State *L);

#endif