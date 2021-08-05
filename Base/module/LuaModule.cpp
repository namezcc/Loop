#include "LuaModule.h"

LuaModule::LuaModule(BaseLayer * l):BaseModule(l)
{
}

LuaModule::~LuaModule()
{
}

void LuaModule::Init()
{

}

void LuaModule::Execute()
{
	auto dt = Loop::GetMilliSecend();
	for (auto& ls:m_stats)
	{
		ls->Run(dt);
	}
}

int32_t LuaModule::CreateLuaState()
{
	auto state = GET_SHARE(LuaState);
	state->Init(this);
	m_stats.push_back(state);
	return (int32_t)m_stats.size()-1;
}

SHARE<LuaState> LuaModule::GetLuaState(const int32_t & index)
{
	if (index >= 0 && index < m_stats.size())
		return m_stats[index];
	return SHARE<LuaState>();
}

int32_t LuaModule::CallFunc(LuaState * ls, const std::string & fname)
{
	auto it = m_luaCallFunc.find(fname);
	if (it != m_luaCallFunc.end())
		return it->second(ls,ls->GetLuaState());
	return 0;
}

void LuaModule::SetLoopFunc(const int32_t & index, const std::string & fname)
{
	if (index > 0 && index < m_stats.size())
		m_stats[index]->RegistGlobalFunc(fname);
}
