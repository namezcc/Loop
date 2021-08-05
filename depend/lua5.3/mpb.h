#ifndef MPB_H
#define MPB_H

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

LUALIB_API int (luaopen_pb)(lua_State *L);

#endif