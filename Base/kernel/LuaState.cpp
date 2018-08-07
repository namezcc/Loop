#include "LuaState.h"
#include "LuaModule.h"
#include <iostream>

LuaState::LuaState():m_module(NULL)
{
	m_L = luaL_newstate();
	luaL_openlibs(m_L);
	open_libs();
	RegistCallFunc();
	PushSelf();
}

LuaState::~LuaState()
{
	lua_close(m_L);
}

void LuaState::Run(int64_t dt)
{
	for (auto& ref: m_loopRef)
	{
		CallRegistFunc(ref,std::forward<decltype(dt)>(dt));
	}
}

void LuaState::RunScript(const std::string & file)
{
	auto status = luaL_dofile(m_L, file.c_str());
	if (status != 0)
	{
		report(m_L, status);
	}
}

void LuaState::RunString(const std::string & trunk)
{
	auto status = luaL_dostring(m_L, trunk.c_str());
	if (status != 0)
	{
		report(m_L, status);
	}
}

int LuaState::callFunction(lua_State* L)
{
	if (lua_gettop(L) < 2)
	{
		std::cout << "args num < 2 " << std::endl;
		return 0;
	}
	if (!lua_islightuserdata(L, 1))
	{
		std::cout << "get null " << std::endl;
		return 0;
	}
	auto ptr = (LuaState*)lua_touserdata(L, 1);
	if (ptr->m_module == NULL)
		return 0;
	if (!lua_isstring(L, 2))
	{
		std::cout << "error arg 2 not string " << std::endl;
		return 0;
	}
	auto fname = lua_tostring(L, 2);
	/*auto it = ptr->m_luaCallFunc.find(fname);
	if (it != ptr->m_luaCallFunc.end())
		return it->second(L);*/

	return ptr->m_module->CallFunc(ptr,fname);
	return 0;
}

int LuaState::open_callFunc(lua_State* L)
{
	luaL_Reg lib_s[] = {
		{ "callFunction",callFunction },
	{ NULL,NULL }
	};

	luaL_newlib(L, lib_s);
	return 1;
}

void LuaState::open_libs()
{
	luaL_Reg lib_s[] = {
		{ "Loop",open_callFunc },
	{ NULL,NULL }
	};
	const luaL_Reg* lib = lib_s;
	for (; lib->func != NULL; lib++)
	{
		luaL_requiref(m_L, lib->name, lib->func, 1);
		lua_settop(m_L, 0);
	}
}

void LuaState::RegistCallFunc()
{
	luaL_Reg lib_s[] = {
		{ "callFunction",callFunction },
	{ NULL,NULL }
	};

	luaL_newmetatable(m_L, "Module");
	lua_pushvalue(m_L, -1);
	lua_setfield(m_L, -2, "__index");
	luaL_setfuncs(m_L, lib_s, 0);
}

void LuaState::PushSelf()
{
	auto self = (void*)this;
	lua_pushlightuserdata(m_L, (void*)this);
	lua_setglobal(m_L, "LoopState");

	lua_getglobal(m_L, "LoopState");
	luaL_getmetatable(m_L, "Module");
	lua_setmetatable(m_L, -2);

}

void LuaState::printLuaStack(lua_State *L)
{
	int nIndex;
	int nType;
	fprintf(stderr, "================栈顶================\n");
	fprintf(stderr, "   索引  类型          值\n");
	for (nIndex = lua_gettop(L); nIndex > 0; --nIndex) {
		nType = lua_type(L, nIndex);
		fprintf(stderr, "   (%d)  %s         %s\n", nIndex,
			lua_typename(L, nType), lua_tostring(L, nIndex));
	}
	fprintf(stderr, "================栈底================\n");
}