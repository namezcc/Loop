#include "LuaValue.h"

void LuaVInt64::pushValue(lua_State * L)
{
	lua_pushinteger(L, m_val);
}

bool LuaVInt64::pullValue(lua_State * L, int index)
{
	if (lua_isinteger(L, index))
	{
		m_val = lua_tointeger(L, index);
		return true;
	}
	return false;
}

void LuaVInt32::pushValue(lua_State * L)
{
	lua_pushinteger(L, m_val);
}

bool LuaVInt32::pullValue(lua_State * L, int index)
{
	if (lua_isinteger(L, index))
	{
		m_val = (int32_t)lua_tointeger(L, index);
		return true;
	}
	return false;
}

void LuaVString::pushValue(lua_State * L)
{
	lua_pushlstring(L, m_val.data(), m_val.size());
}

bool LuaVString::pullValue(lua_State * L, int index)
{
	if (lua_isstring(L, index))
	{
		size_t len;
		auto c = lua_tolstring(L, index, &len);
		m_val.assign(c, len);
		return true;
	}
	return false;
}

void LuaVNumber::pushValue(lua_State * L)
{
	lua_pushnumber(L, m_val);
}

bool LuaVNumber::pullValue(lua_State * L, int index)
{
	if (lua_isnumber(L, index))
	{
		m_val = lua_tonumber(L, index);
		return true;
	}
	return false;
}

void LuaVUdata::pushValue(lua_State * L)
{
	lua_pushlightuserdata(L, m_val);
}

bool LuaVUdata::pullValue(lua_State * L, int index)
{
	if (lua_isuserdata(L, index))
	{
		m_val = lua_touserdata(L, index);
		return true;
	}
	return false;
}

void LuaVBool::pushValue(lua_State * L)
{
	lua_pushboolean(L, m_val);
}

bool LuaVBool::pullValue(lua_State * L, int index)
{
	if (lua_isboolean(L, index))
	{
		m_val = lua_toboolean(L, index);
		return true;
	}
	return false;
}

void LuaVCharString::pushValue(lua_State * L)
{
	lua_pushlstring(L, m_val, m_size);
}

bool LuaVCharString::pullValue(lua_State * L, int index)
{
	if (lua_isstring(L, index))
	{
		size_t len;
		m_val = (char*)lua_tolstring(L, index, &len);
		m_size = (int32_t)len;
		return true;
	}
	return false;
}

#define PUSH_LUA_ARG(T) \
auto arg = new T();\
arg->m_val = val; \
m_arg.push_back(arg);

void LuaArgs::pushArg(int32_t val, int32_t index)
{
	auto arg = new LuaVInt32();
	arg->m_val = val;
	if (index >= 0)
		m_arg.insert(m_arg.begin() + index, arg);
	else
		m_arg.push_back(arg);
}

void LuaArgs::pushArg(int64_t val)
{
	PUSH_LUA_ARG(LuaVInt64);
}

void LuaArgs::pushArg(bool val)
{
	PUSH_LUA_ARG(LuaVBool);
}

void LuaArgs::pushArg(double val)
{
	PUSH_LUA_ARG(LuaVNumber);
}

void LuaArgs::pushArg(const std::string & val)
{
	PUSH_LUA_ARG(LuaVString);
}

void LuaArgs::pushArg(void * val)
{
	PUSH_LUA_ARG(LuaVUdata);
}

void LuaArgs::pushArg(const char * val, int32_t _size)
{
	auto arg = new LuaVCharString();
	arg->m_val = val;
	arg->m_size = _size;
	m_arg.push_back(arg);
}

#define GET_LUA_RES(T)\
auto res = new T();\
m_res.push_back(res);\
return res;

LuaVInt32 * LuaArgs::getInt32Res()
{
	GET_LUA_RES(LuaVInt32);
}

LuaVInt64 * LuaArgs::getInt64Res()
{
	GET_LUA_RES(LuaVInt64);
}

LuaVBool * LuaArgs::getBoolRes()
{
	GET_LUA_RES(LuaVBool);
}

LuaVString * LuaArgs::getStringRes()
{
	GET_LUA_RES(LuaVString);
}

LuaVCharString * LuaArgs::getCStringRes()
{
	GET_LUA_RES(LuaVCharString);
}

LuaVNumber * LuaArgs::getNumberRes()
{
	GET_LUA_RES(LuaVNumber);
}

LuaArgs::~LuaArgs()
{
	for (auto p : m_arg)
	{
		delete p;
	}
	for (auto p : m_res)
	{
		delete p;
	}
	m_arg.clear();
	m_res.clear();
}

