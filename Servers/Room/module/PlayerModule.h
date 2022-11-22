#ifndef PLAYER_MODULE_H
#define PLAYER_MODULE_H

#include "BaseModule.h"

class MsgModule;
class TransMsgModule;
class NetObjectModule;
class EventModule;
class RoomModuloe;
class RoomLuaModule;
class LuaModule;

struct RoleInfo
{
	int32_t cid;
	std::string name;
	int32_t level;
};

struct PlayerInfo
{
	int32_t uid;
	int32_t gate_sid;
	int32_t enter_cid;
	std::map<int32_t, RoleInfo> m_roles;
};

class PlayerModule:public BaseModule
{
public:
	PlayerModule(BaseLayer* l);
	~PlayerModule();

	void sendToPlayer(int32_t gateid,int32_t uid,int32_t mid,gpb::Message& msg);
	void sendToPlayer(int32_t gateid, int32_t uid, int32_t mid, const char* msg,const int32_t& msglen);
	void sendToPlayer(int32_t gateid, int32_t uid, int32_t mid, BuffBlock* msg);
	void broadToPlayer(int32_t gateid,std::vector<int32_t>& vec, int32_t mid, gpb::Message& msg);
	void broadToPlayer(int32_t gateid, std::vector<int32_t>& vec, int32_t mid, BuffBlock* msg);
protected:
	virtual void Init() override;

	void onServerConnect(SHARE<NetServer>& ser);
	void onServerClose(SHARE<NetServer>& ser);
	void sendRoomMgrPlayerNum();

	void onPlayerLogin(NetMsg* msg);
	void onPlayerLogout(NetMsg* msg);
	void onCreatePlayer(NetMsg* msg);
	void onEnterGame(NetMsg* msg);
	void onGetRoleList(NetMsg* msg);

	void onNetMsg(NetMsg * msg);

public:
	

private:
	
	MsgModule* m_msgModule;
	TransMsgModule* m_transModule;
	NetObjectModule* m_netobjModule;
	EventModule* m_eventModule;
	RoomModuloe* m_room_mod;
	RoomLuaModule* m_room_lua;
	LuaModule* m_lua_mod;

	std::unordered_map<int32_t, PlayerInfo> m_player_info;

};

#endif
