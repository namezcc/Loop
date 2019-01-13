#ifndef LUA_STATE_H
#define LUA_STATE_H
#include "Utils.h"
#include "LuaUtil.h"
#include "Define.h"
#include <iostream>
#include <unordered_map>

class LuaModule;

class LOOP_EXPORT LuaState
{
public:
	struct ExitLuaClearStack
	{
		lua_State* m_l;
		ExitLuaClearStack(lua_State* L) :m_l(L)
		{}

		~ExitLuaClearStack()
		{
			lua_settop(m_l, 0);
		}
	};
public:
	LuaState();
	~LuaState();

	void Run(int64_t dt);
	inline void Init(LuaModule* m) { m_module = m; };
	void RunScript(const std::string& file);
	void RunString(const std::string& trunk);
	lua_State* GetLuaState() { return m_L; };

	int32_t RegistGlobalFunc(const std::string& fname,bool loop=true)
	{
		ExitLuaClearStack _call(m_L);
		lua_getglobal(m_L, fname.c_str());
		if (!lua_isfunction(m_L, -1))
		{
			return -1;
		}
		int32_t ref = luaL_ref(m_L, LUA_REGISTRYINDEX);
		if(loop)
			m_loopRef.push_back(ref);
		return ref;
	}

	template<typename ...Args>
	void CallGlobalLuaFunc(const std::string& funcstr, Args&&... args)
	{
		ExitLuaClearStack _call(m_L);
		lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_errIndex);
		lua_getglobal(m_L, funcstr.c_str());
		if (!lua_isfunction(m_L, -1))
		{
			return;
		}
		if (sizeof...(Args)>0)
			PushLuaArgs::PushVal(m_L, std::forward<Args>(args)...);
		auto es = -2 - (sizeof...(Args));
		if (lua_pcall(m_L, sizeof...(Args), 0, es) != LUA_OK)
			PrintError();
	}

	template<typename T, typename ...Args>
	typename std::enable_if<!LuaReflect<T>::have_type, T>::type
		CallGlobalLuaFuncRes(const std::string& funcstr, Args&&... args)
	{
		ExitLuaClearStack _call(m_L);
		lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_errIndex);
		lua_getglobal(m_L, funcstr.c_str());
		if (!lua_isfunction(m_L, -1))
		{
			return T();
		}
		if (sizeof...(Args)>0)
			PushLuaArgs::PushVal(m_L, std::forward<Args>(args)...);
		auto es = -2 - (sizeof...(Args));
		if (lua_pcall(m_L, sizeof...(Args), PullLuaArgs<T>::P_SIZE, es) != LUA_OK)
			PrintError();
		return PullLuaArgs<T>::PullVal(m_L);
	}

	template<typename T, typename ...Args>
	typename std::enable_if<LuaReflect<T>::have_type, T>::type
		CallGlobalLuaFuncRes(const std::string& funcstr, Args&&... args)
	{
		ExitLuaClearStack _call(m_L);
		lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_errIndex);
		lua_getglobal(m_L, funcstr.c_str());
		if (!lua_isfunction(m_L, -1))
		{
			return T();
		}
		if (sizeof...(Args)>0)
			PushLuaArgs::PushVal(m_L, std::forward<Args>(args)...);
		auto es = -2 - (sizeof...(Args));
		if (lua_pcall(m_L, sizeof...(Args), PullLuaArgs<LuaReflect<T>>::P_SIZE, es) != LUA_OK)
			PrintError();
		return PullLuaArgs<LuaReflect<T>>::PullVal(m_L);
	}

	template<typename ...Args>
	void CallRegistFunc(const int32_t& ref, Args&&... args)
	{
		ExitLuaClearStack _call(m_L);
		lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_errIndex);
		lua_rawgeti(m_L, LUA_REGISTRYINDEX, ref);
		if (!lua_isfunction(m_L, -1))
		{
			return;
		}
		if (sizeof...(Args)>0)
			PushLuaArgs::PushVal(m_L, std::forward<Args>(args)...);
		int es = -2 - (int)(sizeof...(Args));
		if (lua_pcall(m_L, sizeof...(Args), 0, es) != LUA_OK)
			PrintError();
	}

	template<typename T, typename ...Args>
	typename std::enable_if<!LuaReflect<T>::have_type, T>::type
		CallRegistFuncRes(const int32_t& ref, Args&&... args)
	{
		ExitLuaClearStack _call(m_L);
		lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_errIndex);
		lua_rawgeti(m_L, LUA_REGISTRYINDEX, ref);
		if (!lua_isfunction(m_L, -1))
		{
			return T();
		}
		if (sizeof...(Args)>0)
			PushLuaArgs::PushVal(m_L, std::forward<Args>(args)...);
		auto es = -2 - (sizeof...(Args));
		if (lua_pcall(m_L, sizeof...(Args), PullLuaArgs<T>::P_SIZE, es) != LUA_OK)
			PrintError();
		return PullLuaArgs<T>::PullVal(m_L);
	}

	template<typename T, typename ...Args>
	typename std::enable_if<LuaReflect<T>::have_type, T>::type
		CallRegistFuncRes(const int32_t& ref, Args&&... args)
	{
		ExitLuaClearStack _call(m_L);
		lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_errIndex);
		lua_rawgeti(m_L, LUA_REGISTRYINDEX, ref);
		if (!lua_isfunction(m_L, -1))
		{
			return T();
		}
		if (sizeof...(Args)>0)
			PushLuaArgs::PushVal(m_L, std::forward<Args>(args)...);
		auto es = -2 - (sizeof...(Args));
		if (lua_pcall(m_L, sizeof...(Args), PullLuaArgs<LuaReflect<T>>::P_SIZE, es) != LUA_OK)
			PrintError();
		return PullLuaArgs<LuaReflect<T>>::PullVal(m_L);
	}

	static void l_message(const char *pname, const char *msg) {
		if (pname) lua_writestringerror("%s: ", pname);
		lua_writestringerror("%s\n", msg);
	}

	static int report(lua_State *L, int status) {
		if (status != LUA_OK) {
			const char *msg = lua_tostring(L, -1);
			l_message("server", msg);
			lua_pop(L, 1);  /* remove message */
		}
		return status;
	}

	template<typename T, typename F>
	void BindLuaCall(const std::string& fname, T&&t, F&&f)
	{
		auto call = ANY_BIND(t, f);
		m_luaCallFunc[fname] = [call](lua_State* L) {
			if (lua_gettop(L) - 2 != FuncArgsType<F>::SIZE)
			{
				std::cout << "error args num expect:" << FuncArgsType<F>::SIZE << std::endl;
				return 0;
			}
			return CallTool<FuncArgsType<F>::SIZE>::Call<FuncArgsType<F>::typeR, FuncArgsType<F>::tupleArgs>(L, call);
		};
	}

	template<typename ...Args>
	int32_t PushArgs(Args&&... args)
	{
		PushLuaArgs::PushVal(m_L, std::forward<Args>(args)...);
		return sizeof...(Args);
	}

	void printLuaStack();
protected:
	void PrintError()
	{
		std::cout << "Lua error:" << lua_tostring(m_L, -1) << std::endl;
	}

	static int callFunction(lua_State* L);
	static int open_callFunc(lua_State* L);
	static int traceback(lua_State *L);
	void open_libs();
	void RegistCallFunc();
	void PushSelf();

private:
	lua_State * m_L;
	int32_t m_errIndex;
	LuaModule* m_module;
	std::vector<int32_t> m_loopRef;
	std::unordered_map<std::string, std::function<int(lua_State*)>> m_luaCallFunc;
};

#endif
