#ifndef PLAYER_MODULE_H
#define PLAYER_MODULE_H

#include "BaseModule.h"
//#include "GameReflectData.h"
//#include "ConfigObjects.h"

#define ROOM_READY_OUT_TIME 60	//second

class MsgModule;
class TransMsgModule;
class NetObjectModule;
class EventModule;
class RoomModuloe;
class RoomLuaModule;

enum READY_STATE
{
	RS_NONE,
	RS_SELECT,
	RS_CREATE,
	RS_IN_GAME,
};

enum PlayerState
{
	PS_NONE,
	PS_IN_MATCH,
	PS_IN_BATTLE,
};

struct ReadyInfo
{
	int64_t pid;
	int64_t roleId;
	int32_t key;
	int32_t sock;
	int32_t state;
};

struct MatchInfo
{
	int8_t m_state;	//0: none 1: in match 2: in Battle
	int16_t m_proxyId;
	int16_t m_matchSerId;
	int32_t m_sceneId;
	std::string	m_battleIp;
	int32_t m_battlePort;
	int64_t m_key;
};

struct RoleInfo
{
	int64_t pid;
	std::string name;
	int32_t level;
};

struct RoomPlayer:public LoopObject
{
	virtual void init(FactorManager*f)
	{
		sock = 0;
		uid = 0;
		enter_pid = 0;
		m_roles.clear();
	}

	virtual void recycle(FactorManager*f)
	{
		
	}

	int32_t sock;
	int64_t uid;
	int64_t enter_pid;
	std::map<int64_t,RoleInfo> m_roles;

};

class PlayerModule:public BaseModule
{
public:
	PlayerModule(BaseLayer* l);
	~PlayerModule();

protected:
	virtual void Init() override;

	void OnPlayerEnter(NetMsg* msg);
	void OnCreatePlayer(NetMsg* msg);
	void onEnterGame(NetMsg* msg);

	void onGetRoleList(NetMsg* msg);


	RoomPlayer* getPlayerData(int32_t sock);
	void removePlayer(int32_t sock);
public:
	

private:
	
	MsgModule* m_msgModule;
	TransMsgModule* m_transModule;
	NetObjectModule* m_netobjModule;
	EventModule* m_eventModule;
	RoomModuloe* m_room_mod;
	RoomLuaModule* m_room_lua;

	std::unordered_map<int64_t, ReadyInfo> m_readyTable;
	
	RoomPlayer* m_player[MAX_CLIENT_CONN];

};

#endif
