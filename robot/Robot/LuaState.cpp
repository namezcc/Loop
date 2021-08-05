#include "LuaState.h"
#include <iostream>
extern "C" {
#include "mpb.h"
}

LuaState::LuaState()
{
	m_L = luaL_newstate();
	luaL_openlibs(m_L);
	luaopen_pb(m_L);
	lua_settop(m_L, 0);
	//open_libs();
	RegistCallFunc();
	PushSelf();
	lua_settop(m_L, 0);
}

LuaState::~LuaState()
{
	lua_close(m_L);
}

void LuaState::RunUpdateFunc(const int64_t & dt)
{
	for (auto& ref : m_loopRef)
	{
		CallRegistFunc(ref, dt);
	}
}

void LuaState::RunScript(const std::string & file)
{
	int status = luaL_dofile(m_L, file.c_str());
	if (status != 0)
	{
		report(m_L, status);
	}
}

void LuaState::RunString(const std::string & trunk)
{
	int status = luaL_dostring(m_L, trunk.c_str());
	if (status != 0)
	{
		report(m_L, status);
	}
}

void LuaState::PrintError()
{
	std::cout << "Lua error:" << lua_tostring(m_L, -1) << std::endl;
}

void LuaState::l_message(const char *pname, const char *msg)
{
	if (pname) lua_writestringerror("%s: ", pname);
	lua_writestringerror("%s\n", msg);
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
	if (!lua_isstring(L, 2))
	{
		std::cout << "error arg 2 not string " << std::endl;
		return 0;
	}
	auto fname = lua_tostring(L, 2);
	auto it = ptr->m_luaCallFunc.find(fname);
	if (it != ptr->m_luaCallFunc.end())
		return it->second(L);
	return 0;
}

#include <wchar.h>
#include <windows.h>

static char* U2G(const char* utf8) {
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	return str;
}

int LuaState::printFunction(lua_State * L)
{
	int n = lua_gettop(L);  /* number of arguments */
	if (n < 2)
	{
		printLuaFuncStack(L, "print func args num error");
		return 0;
	}

	if (!lua_isinteger(L, 1))
	{
		printLuaFuncStack(L, "print func level type error");
		return 0;
	}

	int i;
	lua_getglobal(L, "tostring");
	std::string log;
	for (i = 2; i <= n; i++) {
		const char *s;
		size_t l;
		lua_pushvalue(L, -1);  /* function to be called */
		lua_pushvalue(L, i);   /* value to print */
		lua_call(L, 1, 1);
		s = lua_tolstring(L, -1, &l);  /* get result */
		if (s == NULL)
		{
			printLuaFuncStack(L, "'tostring' must return a string to 'print'");
			return 0;
		}
		log.append(s, l);
		lua_pop(L, 1);  /* pop result */
	}

	auto chrlog = U2G(log.data());
	std::cout << chrlog << std::endl;
	delete[] chrlog;
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

int LuaState::traceback(lua_State * L)
{
	const char *msg = lua_tostring(L, -1);
	if (msg)
		luaL_traceback(L, L, msg, 1);
	else
		lua_pushliteral(L, "no message");
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
		{ "callPrint",printFunction },
	{ NULL,NULL }
	};

	luaL_newmetatable(m_L, "Module");
	lua_pushvalue(m_L, -1);
	lua_setfield(m_L, -2, "__index");
	luaL_setfuncs(m_L, lib_s, 0);
}

#define LuaStateName "CLuaState"

void LuaState::PushSelf()
{
	auto self = (void*)this;
	lua_pushlightuserdata(m_L, (void*)this);
	lua_setglobal(m_L, LuaStateName);

	lua_getglobal(m_L, LuaStateName);
	luaL_getmetatable(m_L, "Module");
	lua_setmetatable(m_L, -2);

	lua_pushcfunction(m_L, traceback);
	m_errIndex = luaL_ref(m_L, LUA_REGISTRYINDEX);
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

void LuaState::printLuaFuncStack(lua_State * L,const char* msg)
{
	luaL_traceback(L, L, msg, 1);
	std::cout << "Lua error:" << lua_tostring(L, -1) << std::endl;
	lua_pop(L, 1);
}
