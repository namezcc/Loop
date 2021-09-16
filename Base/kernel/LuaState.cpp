#include "LuaState.h"
#include "LuaModule.h"
#include "MsgModule.h"
extern "C" {
#include "lua5.3/mpb.h"
}
#include <iostream>

LuaState::LuaState():m_module(NULL)
{
	m_L = luaL_newstate();
	luaL_openlibs(m_L);
	luaopen_pb(m_L);
	lua_settop(m_L, 0);
	open_libs();
	RegistCallFunc();
	PushSelf();
	lua_settop(m_L, 0);

	m_dt = new LuaVInt64();
	m_update_arg.m_arg.push_back(m_dt);
}

LuaState::~LuaState()
{
	lua_close(m_L);
}

void LuaState::Run(int64_t dt)
{
	m_dt->m_val = dt;
	for (auto& ref: m_loopRef)
	{
		callRegistFunc(ref, m_update_arg);
		//CallRegistFunc(ref,dt);
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

void LuaState::initLuaCommonFunc(const std::string & name)
{
	ExitLuaClearStack _call(m_L);
	lua_getglobal(m_L, name.c_str());
	if (!lua_isfunction(m_L, -1))
	{
		LP_ERROR<<"initLuaCommonFunc error can not find func " << name;
		return;
	}
	m_common_ref = luaL_ref(m_L, LUA_REGISTRYINDEX);
}

void LuaState::initLToCIndex(uint32_t _began, uint32_t _end)
{
	if (_began > _end)
		std::swap(_began, _end);

	m_beginIndex = _began;
	m_endIndex = _end;
	m_commonCall = new LuaCallHandle[_end - _began];
}

void LuaState::initCToLIndex(uint32_t _began, uint32_t _end)
{
	if (_began > _end)
		std::swap(_began, _end);

	m_cToLBegIndex = _began;
	m_cToLEndIndex = _end;
	m_cTolRef = new int32_t[_end - _began];
	memset(m_cTolRef, 0, sizeof(int32_t)*(_end - _began));
}

void LuaState::bindLToCFunc(int32_t findex, const LuaCallHandle & func)
{
	if (m_commonCall == NULL)
	{
		LP_ERROR << "need call initLToCIndex  first";
		return;
	}

	if (findex < m_beginIndex || findex >= m_endIndex)
	{
		LP_ERROR << "bind LToc index out of rang" << findex;
		return;
	}
	m_commonCall[findex] = func;
}

bool LuaState::callLuaCommonFunc(LuaArgs & arg)
{
	return callLuaFunc(m_common_ref, arg);
}

bool LuaState::callLuaFunc(int32_t findex, LuaArgs & arg)
{
	if (m_cTolRef == NULL)
	{
		LP_ERROR << "need init initCToLIndex or callRegistFunc";
		return false;
	}

	if (findex < m_cToLBegIndex || findex >= m_cToLEndIndex)
	{
		LP_ERROR << "findex out of range " << findex;
		return false;
	}

	return callRegistFunc(m_cTolRef[findex - m_cToLBegIndex], arg);
}

bool LuaState::callRegistFunc(int32_t ref, LuaArgs & arg)
{
	ExitLuaClearStack _call(m_L);

	if (!getRefFunc(ref))
		return false;

	int32_t rnum = (int32_t)arg.m_res.size();
	int32_t anum = (int32_t)arg.m_arg.size();

	int es = -2 - anum;

	for (auto& v : arg.m_arg)
	{
		v->pushValue(m_L);
	}
	if (lua_pcall(m_L, anum, rnum, es) != LUA_OK)
	{
		PrintError();
		return false;
	}
	for (int32_t i = 0; i < rnum; i++)
	{
		arg.m_res[i]->pullValue(m_L, -rnum + i);
	}
	return true;
}

bool LuaState::callGloableFunc(const std::string & f, LuaArgs & arg)
{
	lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_errIndex);
	lua_getglobal(m_L, f.c_str());
	if (!lua_isfunction(m_L, -1))
	{
		return false;
	}

	int32_t rnum = (int32_t)arg.m_res.size();
	int32_t anum = (int32_t)arg.m_arg.size();

	int es = -2 - anum;

	for (auto& v : arg.m_arg)
	{
		v->pushValue(m_L);
	}
	if (lua_pcall(m_L, anum, rnum, es) != LUA_OK)
	{
		PrintError();
		return false;
	}
	for (int32_t i = 0; i < rnum; i++)
	{
		arg.m_res[i]->pullValue(m_L, -rnum + i);
	}
	return false;
}

bool LuaState::getRefFunc(int32_t ref)
{
	lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_errIndex);
	lua_rawgeti(m_L, LUA_REGISTRYINDEX, ref);
	if (!lua_isfunction(m_L, -1))
	{
		LP_ERROR << "lua func not a function";
		return false;
	}
	return true;
}

int LuaState::open_callFunc(lua_State* L)
{
	/*luaL_Reg lib_s[] = {
		{ "callFunction",callFunction },
	{ NULL,NULL }
	};

	luaL_newlib(L, lib_s);*/
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

int net_readint8(lua_State * L)
{
	if (lua_islightuserdata(L, -1))
	{
		auto msg = (NetMsg*)lua_touserdata(L, -1);
		lua_pushinteger(L, msg->m_buff->readInt8());
	}
	else
	{
		LuaState::printLuaFuncStack(L, "readerror not netMsg*");
		lua_pushinteger(L, 0);
	}
	return 1;
}

int net_readint16(lua_State * L)
{
	if (lua_islightuserdata(L, -1))
	{
		auto msg = (NetMsg*)lua_touserdata(L, -1);
		lua_pushinteger(L, msg->m_buff->readInt16());
	}
	else
	{
		LuaState::printLuaFuncStack(L, "readerror not netMsg*");
		lua_pushinteger(L, 0);
	}
	return 1;
}

int net_readint32(lua_State * L)
{
	if (lua_islightuserdata(L, -1))
	{
		auto msg = (NetMsg*)lua_touserdata(L, -1);
		lua_pushinteger(L, msg->m_buff->readInt32());
	}
	else
	{
		LuaState::printLuaFuncStack(L, "readerror not netMsg*");
		lua_pushinteger(L, 0);
	}
	return 1;
}

int net_readint64(lua_State * L)
{
	if (lua_islightuserdata(L, -1))
	{
		auto msg = (NetMsg*)lua_touserdata(L, -1);
		lua_pushinteger(L, msg->m_buff->readInt64());
	}
	else
	{
		LuaState::printLuaFuncStack(L, "readerror not netMsg*");
		lua_pushinteger(L, 0);
	}
	return 1;
}

int net_readstring(lua_State * L)
{
	if (lua_islightuserdata(L, -1))
	{
		auto msg = (NetMsg*)lua_touserdata(L, -1);
		int32_t len = 0;
		auto buf = msg->m_buff->readString(len);
		lua_pushlstring(L, buf, len);
	}
	else
	{
		LuaState::printLuaFuncStack(L, "readerror not netMsg*");
		lua_pushstring(L, "");
	}
	return 1;
}

int net_readbuff(lua_State * L)
{
	if (lua_islightuserdata(L, -1))
	{
		auto msg = (NetMsg*)lua_touserdata(L, -1);
		int32_t len = 0;
		auto buf = msg->m_buff->readBuff(len);
		lua_pushlstring(L, buf, len);
	}
	else
	{
		LuaState::printLuaFuncStack(L, "readerror not netMsg*");
		lua_pushstring(L, "");
	}
	return 1;
}

int net_make_buff(lua_State * L)
{
	if (!lua_islightuserdata(L,-2))
		return 0;

	auto ptr = (LuaState*)lua_touserdata(L, -2);

	size_t len = 0;

	if (lua_isinteger(L, -1))
	{
		len = (size_t)lua_tointeger(L, -1);
		if (len > (1 << 20))
		{
			LuaState::printLuaFuncStack(L, "net_make_buff size error");
			return 0;
		}
	}

	auto pack = GET_LAYER_MSG(BuffBlock);
	ptr->luaModule()->addCashSendBuff(pack);
	if (len > 0)
		pack->makeRoom(len);
	lua_pushlightuserdata(L, pack);
	return 1;
}

int net_writeint8(lua_State * L)
{
	if (lua_islightuserdata(L, -2))
	{
		auto num = luaL_checkinteger(L, -1);
		auto pack = (BuffBlock*)lua_touserdata(L, -2);
		pack->writeInt8((int8_t)num);
	}
	else
	{
		LuaState::printLuaFuncStack(L, "write not BuffBlock*");
	}
	return 0;
}

int net_writeint16(lua_State * L)
{
	if (lua_islightuserdata(L, -2))
	{
		auto num = luaL_checkinteger(L, -1);
		auto pack = (BuffBlock*)lua_touserdata(L, -2);
		pack->writeInt16((int16_t)num);
	}
	else
	{
		LuaState::printLuaFuncStack(L, "write not BuffBlock*");
	}
	return 0;
}

int net_writeint32(lua_State * L)
{
	if (lua_islightuserdata(L, -2))
	{
		auto num = luaL_checkinteger(L, -1);
		auto pack = (BuffBlock*)lua_touserdata(L, -2);
		pack->writeInt32((int32_t)num);
	}
	else
	{
		LuaState::printLuaFuncStack(L, "write not BuffBlock*");
	}
	return 0;
}

int net_writeint64(lua_State * L)
{
	if (lua_islightuserdata(L, -2))
	{
		auto num = luaL_checkinteger(L, -1);
		auto pack = (BuffBlock*)lua_touserdata(L, -2);
		pack->writeInt64((int64_t)num);
	}
	else
	{
		LuaState::printLuaFuncStack(L, "write not BuffBlock*");
	}
	return 0;
}

int net_writestring(lua_State * L)
{
	if (lua_islightuserdata(L, -2))
	{
		if (!lua_isstring(L, -1))
		{
			LuaState::printLuaFuncStack(L, "write arg not string");
			return 0;
		}
		size_t len;
		auto buf = lua_tolstring(L, -1, &len);
		auto pack = (BuffBlock*)lua_touserdata(L, -2);
		pack->writeString(std::string(buf, len));
	}
	else
	{
		LuaState::printLuaFuncStack(L, "write not BuffBlock*");
	}
	return 0;
}

int net_writebuff(lua_State * L)
{
	if (lua_islightuserdata(L, -2))
	{
		if (!lua_isstring(L, -1))
		{
			LuaState::printLuaFuncStack(L, "write arg not string");
			return 0;
		}
		size_t len;
		auto buf = lua_tolstring(L, -1, &len);
		auto pack = (BuffBlock*)lua_touserdata(L, -2);
		pack->writeBuff(buf, (int32_t)len);
	}
	else
	{
		LuaState::printLuaFuncStack(L, "write not BuffBlock*");
	}
	return 0;
}

void LuaState::open_libs()
{
	luaL_Reg lib_s[] = {
		{ "readint8",net_readint8 },
		{ "readint16",net_readint16 },
		{ "readint32",net_readint32 },
		{ "readint64",net_readint64 },
		{ "readstring",net_readstring },
		{ "readbuff",net_readbuff },
		{ "makepack",net_make_buff},
		{ "writeint8",net_writeint8},
		{ "writeint16",net_writeint16},
		{ "writeint32",net_writeint32},
		{ "writeint64",net_writeint64},
		{ "writestring",net_writestring},
		{ "writebuff",net_writebuff},
	{ NULL,NULL }
	};

	lua_newtable(m_L);
	lua_setglobal(m_L, "NetMsg");
	lua_getglobal(m_L, "NetMsg");
	luaL_setfuncs(m_L, lib_s, 0);
	lua_settop(m_L,0);
}

void LuaState::RegistCallFunc()
{
	luaL_Reg lib_s[] = {
		{ "callPrint", printFunction },
		{ "luaCallC", callFunction2 },
		{ "bindLuaFunc", bindLuaFunc },
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

void LuaState::printLuaStack()
{
	int nIndex;
	int nType;
	fprintf(stderr, "================栈顶================\n");
	fprintf(stderr, "   索引  类型          值\n");
	for (nIndex = lua_gettop(m_L); nIndex > 0; --nIndex) {
		nType = lua_type(m_L, nIndex);
		fprintf(stderr, "   (%d)  %s         %s\n", nIndex,
			lua_typename(m_L, nType), lua_tostring(m_L, nIndex));
	}
	fprintf(stderr, "================栈底================\n");
}

void LuaState::printLuaFuncStack(lua_State * L, const char * msg)
{
	luaL_traceback(L, L, msg, 1);
	LP_INFO << "Lua error:" << lua_tostring(L, -1);
	lua_pop(L, 1);
}

void LuaState::PrintError()
{
	LuaVString str;
	str.pullValue(m_L, -1);
	LP_ERROR_SP << "Lua error:" << str.m_val;
}

int LuaState::callFunction2(lua_State * L)
{
	if (lua_gettop(L) < 2)
	{
		printLuaFuncStack(L, "args num < 2 ");
		return 0;
	}
	if (!lua_islightuserdata(L, 1))
	{
		printLuaFuncStack(L, "get null ");
		return 0;
	}
	auto ptr = (LuaState*)lua_touserdata(L, 1);
	if (!lua_isinteger(L, 2))
	{
		printLuaFuncStack(L, "error arg 2 not int ");
		return 0;
	}

	auto findex = (int32_t)lua_tointeger(L, 2);
	ptr->m_argIndex = 2;

	if (ptr->m_commonCall != NULL && findex >= ptr->m_beginIndex && findex < ptr->m_endIndex)
	{
		if (ptr->m_commonCall[findex])
		{
			return ptr->m_commonCall[findex](ptr);
		}
		else
		{
			LP_ERROR << "common func not bind " << findex;
		}
	}
	else {
		if (!ptr->m_luaCallFunc)
		{
			printLuaFuncStack(L, "not set lua call func");
			return 0;
		}
		return ptr->m_luaCallFunc(findex, ptr);
	}
	return 0;
}

int LuaState::bindLuaFunc(lua_State * L)
{
	if (lua_gettop(L) != 3)
	{
		printLuaFuncStack(L, "args num < 2 ");
		return 0;
	}
	if (!lua_islightuserdata(L, 1))
	{
		printLuaFuncStack(L, "get null ");
		return 0;
	}
	if (!lua_isinteger(L, 2))
	{
		printLuaFuncStack(L, "error arg 2 not int ");
		return 0;
	}
	if (!lua_isfunction(L, -1))
	{
		printLuaFuncStack(L, "error arg 3 not func ");
		return 0;
	}

	auto ptr = (LuaState*)lua_touserdata(L, 1);
	auto findex = (int32_t)lua_tointeger(L, 2);

	if (ptr == NULL)
	{
		printLuaFuncStack(L, "ptr == NULL ");
		return 0;
	}

	if (ptr->m_cTolRef && findex >= ptr->m_cToLBegIndex && findex < ptr->m_cToLEndIndex)
	{
		//解除老的引用
		if (ptr->m_cTolRef[findex] > 0)
			luaL_unref(L, LUA_REGISTRYINDEX, ptr->m_cTolRef[findex]);

		ptr->m_cTolRef[findex] = luaL_ref(L, LUA_REGISTRYINDEX);
	}
	else
	{
		if (!ptr->m_luaBindFunc)
			printLuaFuncStack(L, "not set lua bind func ");
		else
			ptr->m_luaBindFunc(findex, ptr);
	}
	return 0;
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


	int32_t level = (int32_t)lua_tointeger(L, 1);
	int i;
	lua_getglobal(L, "tostring");
	std::string log("LUA:");
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
		log.append(" ");
		lua_pop(L, 1);  /* pop result */
	}
	//log.append("\n");
	if (level == 1)
	{
		LP_DEBUG << log;
	}
	else if (level == 2)
	{
		LP_INFO << log;
	}
	else if (level == 3)
	{
		LP_WARN << log;
	}
	else if (level == 4)
	{
		LP_ERROR_SP << log;
	}
	return 0;
}

void LuaState::PushInt32(int32_t val)
{
	lua_pushinteger(m_L, val);
}

void LuaState::PushInt64(int64_t val)
{
	lua_pushinteger(m_L, val);
}

void LuaState::PushString(const std::string & val)
{
	lua_pushlstring(m_L, val.data(), val.size());
}

void LuaState::PushFloat(float val)
{
	lua_pushnumber(m_L, val);
}

void LuaState::PushTableBegin()
{
	lua_newtable(m_L);
}

void LuaState::PushTableBegin(int32_t tabkey)
{
	PushInt32(tabkey);
	lua_newtable(m_L);
}

void LuaState::PushTableBegin(const std::string & tabkey)
{
	PushString(tabkey);
	lua_newtable(m_L);
}

void LuaState::PushTableInt32(int32_t idx, int32_t val)
{
	lua_pushinteger(m_L, val);
	lua_rawseti(m_L, -2, idx);
}

void LuaState::PushTableInt64(int32_t idx, int64_t val)
{
	lua_pushinteger(m_L, val);
	lua_rawseti(m_L, -2, idx);
}

void LuaState::PushTableString(int32_t idx, const std::string & val)
{
	lua_pushlstring(m_L, val.data(), val.size());
	lua_rawseti(m_L, -2, idx);
}

void LuaState::PushTableFloat(int32_t idx, float val)
{
	lua_pushnumber(m_L, val);
	lua_rawseti(m_L, -2, idx);
}

void LuaState::PushTableEnd()
{
	lua_settable(m_L, -3);
}

void LuaState::PushTableSetValue()
{
	lua_settable(m_L, -3);
}

void LuaState::PushTableInt32(const std::string & key, int32_t val)
{
	lua_pushlstring(m_L, key.data(), key.size());
	lua_pushinteger(m_L, val);
	lua_settable(m_L, -3);
}

void LuaState::PushTableInt64(const std::string & key, int64_t val)
{
	lua_pushlstring(m_L, key.data(), key.size());
	lua_pushinteger(m_L, val);
	lua_settable(m_L, -3);
}

void LuaState::PushTableString(const std::string & key, const std::string & val)
{
	lua_pushlstring(m_L, key.data(), key.size());
	lua_pushlstring(m_L, val.data(), val.size());
	lua_settable(m_L, -3);
}

void LuaState::PushTableFloat(const std::string & key, float val)
{
	lua_pushlstring(m_L, key.data(), key.size());
	lua_pushnumber(m_L, val);
	lua_settable(m_L, -3);
}

int32_t LuaState::PullInt32()
{
	++m_argIndex;
	if (lua_isinteger(m_L, m_argIndex))
	{
		return (int32_t)lua_tointeger(m_L, m_argIndex);
	}
	else if (lua_isnumber(m_L, m_argIndex))
	{
		double tmpV = lua_tonumber(m_L, m_argIndex);
		return (int32_t)std::lround(tmpV);
	}
	else
	{
		LP_ERROR << "PullInt32 not a number: " << m_argIndex;
	}

	return 0;
}

int64_t LuaState::PullInt64()
{
	++m_argIndex;
	if (lua_isinteger(m_L, m_argIndex))
	{
		return lua_tointeger(m_L, m_argIndex);
	}
	else if (lua_isnumber(m_L, m_argIndex))
	{
		double tmpV = lua_tonumber(m_L, m_argIndex);
		return std::llroundl(tmpV);
	}
	else
	{
		LP_ERROR << "PullInt64 not a number: " << m_argIndex;
	}
	return 0;
}

std::string LuaState::PullString()
{
	++m_argIndex;
	if (lua_isstring(m_L, m_argIndex))
	{
		size_t len;
		auto c = lua_tolstring(m_L, m_argIndex, &len);
		return std::string(c, len);
	}
	return "";
}

void * LuaState::PullUserData()
{
	++m_argIndex;
	if (lua_islightuserdata(m_L,m_argIndex))
	{
		return lua_touserdata(m_L, m_argIndex);
	}
	return NULL;
}

const char * LuaState::PullCString(int32_t & size)
{
	++m_argIndex;
	if (lua_isstring(m_L, m_argIndex))
	{
		size_t len;
		auto c = lua_tolstring(m_L, m_argIndex, &len);
		size = (int32_t)len;
		return c;
	}
	return NULL;
}

float LuaState::Pullfloat()
{
	++m_argIndex;
	if (lua_isnumber(m_L, m_argIndex))
	{
		return (float)lua_tonumber(m_L, m_argIndex);
	}
	return 0.0f;
}

bool LuaState::IsTable(int32_t argIndex)
{
	return lua_istable(m_L, argIndex);
}

bool LuaState::PullTableBegin()
{
	if (!lua_istable(m_L, m_argIndex + 1))
		return false;
	++m_argIndex;
	lua_pushvalue(m_L, m_argIndex);
	return true;
}

int32_t LuaState::PullTableLength()
{
	lua_len(m_L, -1);
	auto val = (int32_t)lua_tointeger(m_L, -1);
	lua_pop(m_L, 1);
	return val;
}

int32_t LuaState::PullTableInt32(int32_t index)
{
	lua_rawgeti(m_L, -1, index);
	auto val = (int32_t)luaL_checkinteger(m_L, -1);
	lua_pop(m_L, 1);
	return val;
}

int64_t LuaState::PullTableInt64(int32_t index)
{
	lua_rawgeti(m_L, -1, index);
	auto val = (int64_t)luaL_checkinteger(m_L, -1);
	lua_pop(m_L, 1);
	return val;
}

std::string LuaState::PullTableString(int32_t index)
{
	lua_rawgeti(m_L, -1, index);
	size_t len;
	auto buff = lua_tolstring(m_L, -1, &len);
	lua_pop(m_L, 1);
	return std::string(buff, len);
}

float LuaState::PullTableFloat(int32_t index)
{
	lua_rawgeti(m_L, -1, index);
	auto val = (float)luaL_checknumber(m_L, -1);
	lua_pop(m_L, 1);
	return val;
}

bool LuaState::PullTableTable(int32_t index)
{
	lua_rawgeti(m_L, -1, index);
	if (lua_istable(m_L, -1))
		return true;
	lua_pop(m_L, 1);
	return false;
}

int32_t LuaState::PullTableInt32(const std::string & key)
{
	lua_getfield(m_L, -1, key.data());
	auto val = (int32_t)luaL_checkinteger(m_L, -1);
	lua_pop(m_L, 1);
	return val;
}

int64_t LuaState::PullTableInt64(const std::string & key)
{
	lua_getfield(m_L, -1, key.data());
	auto val = (int64_t)luaL_checkinteger(m_L, -1);
	lua_pop(m_L, 1);
	return val;
}

std::string LuaState::PullTableString(const std::string & key)
{
	lua_getfield(m_L, -1, key.data());
	size_t len;
	auto buff = lua_tolstring(m_L, -1, &len);
	lua_pop(m_L, 1);
	return std::string(buff, len);
}

float LuaState::PullTableFloat(const std::string & key)
{
	lua_getfield(m_L, -1, key.data());
	auto val = (float)luaL_checknumber(m_L, -1);
	lua_pop(m_L, 1);
	return val;
}

bool LuaState::PullTableTable(const std::string & key)
{
	lua_getfield(m_L, -1, key.data());
	if (lua_istable(m_L, -1))
		return true;
	lua_pop(m_L, 1);
	return false;
}

void LuaState::PullTableEnd()
{
	lua_pop(m_L, 1);
}