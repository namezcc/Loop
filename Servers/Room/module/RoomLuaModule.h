#ifndef ROOM_LUA_MOD_H
#define ROOM_LUA_MOD_H

#include "BaseModule.h"
#include "LuaModule.h"

class RoomModuloe;
class PlayerModule;
class ScheduleModule;

enum CTOL_EX
{
	CTOL_EX_BEG = CTOL_EXPAND,

	CTOL_SET_PLAYER_GATE_INFO = 11,
	CTOL_PLAYER_OFFLINE = 12,
	CTOL_FRAME_UPDATE = 13,


	CTOL_EX_MAX = CTOL_MAX,
};

enum LTOC_EX
{
	LTOC_EX_BEG = LTOC_MAX,

	LTOC_DO_SQL_OPERATION = 101,
	LTOC_SQL_UPDATE_PLAYER_DATA = 102,
	LTOC_SQL_DELETE_PLAYER_DATA = 103,
	LTOC_SEND_MSG_TO_PLAYER = 104,
	LTOC_DO_SQL_OPERATION_PROTO = 105,

};

class RoomLuaModule:public BaseModule
{
public:
	RoomLuaModule(BaseLayer* l):BaseModule(l)
	{}

	~RoomLuaModule()
	{}

	int onLuaCallFunc(int32_t findex,LuaState* l);
	void onLuaSendMsgToPlayer(LuaState* l);

	void callLuaMsg(int32_t mid, LuaArgs& arg);
private:
	// Í¨¹ý BaseModule ¼Ì³Ð
	virtual void Init() override;
	virtual void AfterInit() override;

	LuaModule* m_lua_mod;
	RoomModuloe* m_room_mod;
	PlayerModule* m_player_mod;
	ScheduleModule* m_schedule_mod;
};

#endif // !ROOM_LUA_MOD_H

