#ifndef ROOM_LUA_MOD_H
#define ROOM_LUA_MOD_H

#include "BaseModule.h"
#include "LuaModule.h"

class RoomModuloe;

enum CTOL_EX
{
	CTOL_EX_BEG = CTOL_EXPAND,

	CTOL_SET_PLAYER_SOCK = 11,
	CTOL_PLAYER_OFFLINE = 12,


	CTOL_EX_MAX = CTOL_MAX,
};

enum LTOC_EX
{
	LTOC_EX_BEG = LTOC_MAX,

	LTOC_DO_SQL_OPERATION = 101,
	LTOC_SQL_UPDATE_PLAYER_DATA = 102,
	LTOC_SQL_DELETE_PLAYER_DATA = 103,


};

class RoomLuaModule:public BaseModule
{
public:
	RoomLuaModule(BaseLayer* l):BaseModule(l)
	{}

	~RoomLuaModule()
	{}

	int onLuaCallFunc(int32_t findex,LuaState* l);

	void setPlayerSock(int64_t uid, int32_t sock);
	void callLuaMsg(int32_t mid, LuaArgs& arg);
private:
	// Í¨¹ý BaseModule ¼Ì³Ð
	virtual void Init() override;
	virtual void AfterInit() override;

	LuaModule* m_lua_mod;
	RoomModuloe* m_room_mod;
};

#endif // !ROOM_LUA_MOD_H

