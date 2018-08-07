#ifndef LUA_UTIL_H
#define LUA_UTIL_H
#include "Reflection.h"
#include "lua5.3/lua.hpp"

struct LuaPush
{
	static void PushInt(lua_State* L,const int64_t& val)
	{
		lua_pushinteger(L, val);
	}

	static void PushBool(lua_State* L, const bool& val)
	{
		lua_pushboolean(L, val);
	}

	static void PushString(lua_State* L, const std::string& val)
	{
		lua_pushlstring(L, val.c_str(), val.size());
	}

	static void PushNumber(lua_State* L,const double& val)
	{
		lua_pushnumber(L, val);
	}

	static void PushUserData(lua_State* L, void* val)
	{
		lua_pushlightuserdata(L, val);
	}

	static int64_t LuaToInt(lua_State* L, const int32_t& index)
	{
		if (lua_isinteger(L, index))
			return lua_tointeger(L, index);
		else
			return 0;
	}

	static bool LuaToBool(lua_State* L, const int32_t& index)
	{
		if (lua_isboolean(L, index))
			return lua_toboolean(L, index);
		else
			return 0;
	}

	static std::string LuaToString(lua_State* L, const int32_t& index)
	{
		if (lua_isstring(L, index))
			return lua_tostring(L, index);
		else
			return "";
	}

	static double LuaToNumber(lua_State* L, const int32_t& index)
	{
		if (lua_isnumber(L, index))
			return lua_tonumber(L, index);
		else
			return 0.0;
	}

	static void* LuaToUserData(lua_State* L, const int32_t& index)
	{
		if (lua_islightuserdata(L, index))
			return lua_touserdata(L, index);
		else
			return NULL;
	}
};

template<typename T>
struct LuaPushType {
	static void PushVal(lua_State* L, T&& val) { LuaPush::PushString(L, val); }
};

#define SET_PUSH_FUNC(T,F) \
template<>	\
struct LuaPushType<T>{		\
	static void PushVal(lua_State* L,T&& val){ F(L, val); }	\
};	\

SET_PUSH_FUNC(int16_t, LuaPush::PushInt)
SET_PUSH_FUNC(uint16_t, LuaPush::PushInt)
SET_PUSH_FUNC(int32_t, LuaPush::PushInt)
SET_PUSH_FUNC(uint32_t, LuaPush::PushInt)
SET_PUSH_FUNC(int64_t, LuaPush::PushInt)
SET_PUSH_FUNC(uint64_t, LuaPush::PushInt)
SET_PUSH_FUNC(bool, LuaPush::PushBool)
SET_PUSH_FUNC(const char*, LuaPush::PushString)
SET_PUSH_FUNC(std::string, LuaPush::PushString)
SET_PUSH_FUNC(float, LuaPush::PushNumber)
SET_PUSH_FUNC(double, LuaPush::PushNumber)
SET_PUSH_FUNC(void*, LuaPush::PushUserData)

template<typename T>
struct LuaPullType;
#define SET_PULL_FUNC(T,F) \
template<>	\
struct LuaPullType<T>{		\
	static T PullVal(lua_State* L,const int32_t& idx){ return F(L, idx); }	\
};	\

SET_PULL_FUNC(int16_t, LuaPush::LuaToInt)
SET_PULL_FUNC(uint16_t, LuaPush::LuaToInt)
SET_PULL_FUNC(int32_t, LuaPush::LuaToInt)
SET_PULL_FUNC(uint32_t, LuaPush::LuaToInt)
SET_PULL_FUNC(int64_t, LuaPush::LuaToInt)
SET_PULL_FUNC(uint64_t, LuaPush::LuaToInt)
SET_PULL_FUNC(bool, LuaPush::LuaToBool)
SET_PULL_FUNC(std::string, LuaPush::LuaToString)
SET_PULL_FUNC(float, LuaPush::LuaToNumber)
SET_PULL_FUNC(double, LuaPush::LuaToNumber)
SET_PULL_FUNC(void*, LuaPush::LuaToUserData)

template<typename T>
struct LuaReflect
{
	static constexpr bool have_type = false;
};

template<typename T, bool cond>
struct PushTool
{
	static void PushVal(lua_State* L, T&& val)
	{
		LuaPushType<typename std::decay<T>::type>::PushVal(L, std::forward<T>(val));
	}
};

template<typename T>
struct PushTool<T, true>
{
	using _T = typename std::decay<T>::type;
	static void PushVal(lua_State* L, T&& val)
	{
		lua_newtable(L);
		for (size_t i = 0; i < LuaReflect<_T>::SIZE; i++)
		{
			LuaReflect<_T>::arr_pushfunc[i]((char*)&val, LuaReflect<_T>::arr_offset[i], L);
			lua_setfield(L, -2, LuaReflect<_T>::arr_fields[i]);
		}
	}
};

struct PushLuaArgs
{
	template<typename T, typename ...Args>
	static void PushVal(lua_State* L, T&& val, Args&&... args)
	{
		PushLuaArgs::PushVal(L, std::forward<T>(val));
		PushLuaArgs::PushVal(L, std::forward<Args>(args)...);
	}

	template<typename T>
	static void PushVal(lua_State* L, T&& val)
	{
		PushTool<T, LuaReflect<typename std::decay<T>::type>::have_type>::PushVal(L, std::forward<T>(val));
	}

	static void PushVal(lua_State* L)
	{}
};

template<typename T, bool cond>
struct PullTool
{
	static T PullVal(lua_State* L, const int32_t& idx)
	{
		return LuaPullType<T>::PullVal(L, idx);
	}
};

template<typename T>
struct PullLuaArgs
{
	enum
	{
		P_SIZE = 1,
	};
	static T PullVal(lua_State* L, const int32_t& idx = -1)
	{
		return PullTool<T, LuaReflect<typename std::decay<T>::type>::have_type>::PullVal(L, idx);
	}
};

template<typename T>
struct PullTool<T, true>
{
	static T PullVal(lua_State* L, const int32_t& idx)
	{
		return PullLuaArgs<LuaReflect<T>>::PullVal(L, idx);
	}
};


template<int32_t N, typename T, typename V, typename ...Args>
struct LuaToTuple
{
	enum
	{
		INDEX = sizeof...(Args),
	};
	static void PullVal(T& tp, lua_State* L)
	{
		std::get<N - INDEX - 1>(tp) = PullLuaArgs<V>::PullVal(L, -INDEX - 1);
		LuaToTuple<N, T, Args...>::PullVal(tp, L);
	}
};

template<int32_t N, typename T, typename V>
struct LuaToTuple<N, T, V>
{
	static void PullVal(T& tp, lua_State* L)
	{
		std::get<N - 1>(tp) = PullLuaArgs<V>::PullVal(L, -1);
	}
};

template<typename ...Args>
struct PullLuaArgs<std::tuple<Args...>>
{
	enum
	{
		P_SIZE = sizeof...(Args),
	};
	typedef std::tuple<Args...> R;
	static R PullVal(lua_State* L)
	{
		R tup;
		LuaToTuple<sizeof...(Args), R, Args...>::PullVal(tup, L);
		return tup;
	}
};

template<typename T>
struct PullLuaArgs<LuaReflect<T>>
{
	enum
	{
		P_SIZE = 1,
	};
	static T PullVal(lua_State* L, const int32_t& idx = -1)
	{
		T t;
		if (!lua_istable(L, idx))
			return t;

		for (size_t i = 0; i < LuaReflect<T>::SIZE; i++)
		{
			lua_getfield(L, idx, LuaReflect<T>::arr_fields[i]);
			if (!lua_isnil(L, -1))
				LuaReflect<T>::arr_func[i]((char*)&t, LuaReflect<T>::arr_offset[i], L, -1);
			lua_pop(L, 1);
		}
		return t;
	}
};

template<typename T>
struct MemFromLua {
	static void SetVal(char* p, const int32_t& offset, lua_State* L, const int32_t& idx)
	{
		*((T*)(p + offset)) = PullLuaArgs<T>::PullVal(L, idx);
	}
};

template<typename T>
struct MemToLua {
	static void SetVal(char* p, const int32_t& offset, lua_State* L)
	{
		PushLuaArgs::PushVal(L, std::forward<T>(*((T*)(p + offset))));
	}
};

#define FROM_LUA_PTR(t,m) &MemFromLua<ClassMember<decltype(&t::m)>::type>::SetVal
#define TO_LUA_PTR(t,m) &MemToLua<ClassMember<decltype(&t::m)>::type>::SetVal

#define MAKE_LUA_REFLECT(T,N,...)	\
template<> struct LuaReflect<T>{	\
	static constexpr bool have_type = true;	\
	enum{ SIZE = N,};	\
	static constexpr array<const char*, N> arr_fields = { MAKE_STR_LIST(__VA_ARGS__) }; \
	static constexpr array<int, N> arr_offset = { MAKE_ARG_LISTS(N,offsetof,T,__VA_ARGS__) }; \
	static constexpr array<decltype(&MemFromLua<int32_t>::SetVal),N> arr_func = {MAKE_ARG_LISTS(N,FROM_LUA_PTR,T,__VA_ARGS__)};	\
	static constexpr array<decltype(&MemToLua<int32_t>::SetVal),N> arr_pushfunc = {MAKE_ARG_LISTS(N,TO_LUA_PTR,T,__VA_ARGS__)};	\
};

#define LUA_REFLECT(T,...)	\
	MAKE_LUA_REFLECT(T,GET_ARG_N(__VA_ARGS__),__VA_ARGS__)

#define LUA_REFLECT_CPP_DEFINE(T) \
	constexpr array<const char*,LuaReflect<T>::SIZE> LuaReflect<T>::arr_fields; \
	constexpr array<int,LuaReflect<T>::SIZE> LuaReflect<T>::arr_offset; \
	constexpr array<int,LuaReflect<T>::SIZE> LuaReflect<T>::arr_func;	\
	constexpr array<int,LuaReflect<T>::SIZE> LuaReflect<T>::arr_pushfunc;	


template<int32_t N>
struct CallTool
{
	template<typename R, typename T, typename C>
	static R Call(lua_State* L, C&&f)
	{
		auto tp = PullLuaArgs<T>::PullVal(L);
		return ApplyFunc<N>::template apply<R>(std::forward<C>(f), std::forward<T>(tp));
	}
};

template<>
struct CallTool<0>
{
	template<typename R, typename T, typename C>
	static R Call(lua_State* L, C&&f)
	{
		return f();
	}
};

#endif
