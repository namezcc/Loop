#ifndef LUA_VALUE_H
#define LUA_VALUE_H
#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include "Define.h"
#include "LuaUtil.h"

class LuaState;

struct LOOP_EXPORT LuaValue
{
	virtual void pushValue(lua_State* L) = 0;
	virtual bool pullValue(lua_State* L, int index) = 0;
};

struct LOOP_EXPORT LuaVInt64 :public LuaValue
{
	LuaVInt64() :m_val(0)
	{}
	// 通过 LuaValue 继承
	virtual void pushValue(lua_State * L) override;
	virtual bool pullValue(lua_State * L, int index) override;

	int64_t m_val;
};

struct LOOP_EXPORT LuaVInt32 :public LuaValue
{
	LuaVInt32() :m_val(0)
	{}
	LuaVInt32(int32_t v) :m_val(v)
	{}
	// 通过 LuaValue 继承
	virtual void pushValue(lua_State * L) override;
	virtual bool pullValue(lua_State * L, int index) override;
	int32_t m_val;
};

struct LOOP_EXPORT LuaVString :public LuaValue
{
	// 通过 LuaValue 继承
	virtual void pushValue(lua_State * L) override;
	virtual bool pullValue(lua_State * L, int index) override;

	std::string m_val;
};

struct LOOP_EXPORT LuaVCharString :public LuaValue
{
	LuaVCharString() :m_val(NULL), m_size(0)
	{}
	// 通过 LuaValue 继承
	virtual void pushValue(lua_State * L) override;
	virtual bool pullValue(lua_State * L, int index) override;

	const char* m_val;
	int32_t m_size;
};

struct LOOP_EXPORT LuaVNumber :public LuaValue
{
	LuaVNumber() :m_val(0)
	{}
	// 通过 LuaValue 继承
	virtual void pushValue(lua_State * L) override;
	virtual bool pullValue(lua_State * L, int index) override;

	double m_val;
};

struct LOOP_EXPORT LuaVUdata :public LuaValue
{
	LuaVUdata() :m_val(NULL)
	{}
	// 通过 LuaValue 继承
	virtual void pushValue(lua_State * L) override;
	virtual bool pullValue(lua_State * L, int index) override;

	void* m_val;
};

struct LOOP_EXPORT LuaVBool :public LuaValue
{
	LuaVBool() :m_val(false)
	{}
	// 通过 LuaValue 继承
	virtual void pushValue(lua_State * L) override;
	virtual bool pullValue(lua_State * L, int index) override;

	bool m_val;
};

template<typename T>
struct LOOP_EXPORT LuaTValue :public LuaValue
{
	T m_val;

	// 通过 LuaValue 继承
	virtual void pushValue(lua_State * L) override
	{
		PushLuaArgs::PushVal(L, m_val);
	}
	virtual bool pullValue(lua_State * L, int index) override
	{
		m_val = PullLuaArgs<T>::PullVal(L, index);
		return true;
	}
};

struct LOOP_EXPORT LuaArgs
{
	void pushArg(int32_t val, int32_t index = -1);
	void pushArg(int64_t val);
	void pushArg(bool val);
	void pushArg(double val);
	void pushArg(const std::string& val);
	void pushArg(void* val);
	void pushArg(const char* val, int32_t _size);

	LuaVInt32* getInt32Res();
	LuaVInt64* getInt64Res();
	LuaVBool* getBoolRes();
	LuaVString* getStringRes();
	LuaVCharString* getCStringRes();
	LuaVNumber* getNumberRes();

	~LuaArgs();

	template<typename T>
	void pushTypeArg(const T& val)
	{
		auto arg = new LuaTValue<T>();
		arg->m_val = val;
		m_arg.push_back(arg);
	}

	template<typename T>
	LuaTValue<T>* getRes()
	{
		auto res = new LuaTValue<T>();
		m_res.push_back(res);
		return res;
	}

	std::vector<LuaValue*> m_arg;
	std::vector<LuaValue*> m_res;
};

#endif
