#ifndef SCENE_MODULE_H
#define SCENE_MODULE_H

#include "BaseModule.h"

class TransMsgModule;
class MsgModule;
class EventModule;
class LuaModule;
class ScheduleModule;
struct lua_State;
class LuaState;

#define MAX_SCENE_SIZE	300

struct BattleScene
{
	int32_t m_id;
};

class SceneModule:public BaseModule
{
public:
	SceneModule(BaseLayer* l);
	~SceneModule();

private:

	// Í¨¹ý BaseModule ¼Ì³Ð
	virtual void Init() override;
	void InitScene();

	void OnShowLuaStack(int64_t& dt);
	void OnServerConnect(SHARE<NetServer>& ser);
	void OnClearGC(int64_t& dt);

	void SendFreeScene();

	int BindLuaNetCall(lua_State * L);
	int LuaBindNetMsg(const int32_t& mid);
	int OnGetNetMsgToLua(NetMsg* msg);
	int SendStreamData(lua_State * L);
	int GetScriptPath();
	int GetProtoPath();
	int RunScene(const int32_t& scid);
	int FreeScene(const int32_t& scid);

private:

	ServerNode m_battle;
	SHARE<LuaState> m_luaState;
	int32_t m_netCallRegIndex;
	int32_t m_gcRegIndex;

	std::unordered_map<int32_t, SHARE<BattleScene>> m_runingScene;
	std::unordered_map<int32_t, SHARE<BattleScene>> m_freeScene;


	TransMsgModule* m_transModule;
	MsgModule* m_msgModule;
	EventModule* m_eventModule;
	LuaModule* m_luaModule;
	ScheduleModule* m_scheduleModule;
};

#endif
