#ifndef LUA_MODULE_H
#define LUA_MODULE_H

#include "BaseModule.h"
#include "LuaState.h"

class LOOP_EXPORT LuaModule:public BaseModule
{
public:
	LuaModule(BaseLayer* l);
	~LuaModule();

	// ͨ�� BaseModule �̳�
	virtual void Init() override;
	virtual void Execute() override;

	int32_t CreateLuaState();
	SHARE<LuaState> GetLuaState(const int32_t& index);
	
	template<typename T,typename F>
	void BindLuaCall(const std::string& fname, T&&t, F&&f)
	{
		auto call = ANY_BIND(t, f);
		m_luaCallFunc[fname] = [this,call](LuaState* ls,lua_State* L) {
			if (lua_gettop(L) - 2 != FuncArgsType<F>::SIZE)
			{
				std::cout << "error args num expect:" << FuncArgsType<F>::SIZE << std::endl;
				return 0;
			}
			m_curState = ls;
			return CallTool<FuncArgsType<F>::SIZE>::Call<FuncArgsType<F>::typeR, FuncArgsType<F>::tupleArgs>(L, call);
		};
	}

	template<typename T, typename F>
	void BindLuaOrgCall(const std::string& fname, T&&t, F&&f)
	{
		auto call = ANY_BIND(t, f);
		m_luaCallFunc[fname] = [this, call](LuaState* ls,lua_State* L) {
			m_curState = ls;
			return call(L);
		};
	}

	template<typename ...Args>
	int32_t PushArgs(Args&&... args)
	{
		if(m_curState)
			return m_curState->PushArgs(std::forward<Args>(args)...);
		return 0;
	}

	int32_t CallFunc(LuaState* ls,const std::string& fname);
	void SetLoopFunc(const int32_t& index, const std::string& fname);

private:
	LuaState* m_curState;
	std::vector<SHARE<LuaState>> m_stats;
	std::unordered_map<std::string, std::function<int(LuaState*,lua_State*)>> m_luaCallFunc;
};

#endif
