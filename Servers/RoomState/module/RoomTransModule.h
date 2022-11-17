#ifndef ROOM_TRANS_MODULE_H
#define ROOM_TRANS_MODULE_H

#include "BaseModule.h"
#include "CommonDefine.h"

class MsgModule;
class TransMsgModule;
class EventModule;
class NetObjectModule;
class RedisModule;
class PlayerOnlineModule;
struct PlayerRoomInfo;

class RoomTransModule:public BaseModule
{
public:
	RoomTransModule(BaseLayer* l);
	~RoomTransModule();

	bool chooseGateAndRoom(PlayerRoomInfo& info);
protected:
	virtual void Init() override;

	void onRoomBusyState(NetMsg* msg);
	void onRoomPlayerLogout(NetMsg* msg);
	void onPlayerNum(NetMsg* msg);
	void OnServerConnect(SHARE<NetServer>& ser);
	void OnServerClose(SHARE<NetServer>& ser);

private:
	MsgModule * m_msgModule;
	TransMsgModule* m_transModule;
	EventModule* m_eventModule;
	NetObjectModule* m_netobjModule;
	RedisModule* m_redisModule;
	PlayerOnlineModule* m_ply_online;

	std::map<int32_t, ServerInfoState> m_room_state;
	
	std::map<int32_t, int32_t> m_gate_player_num;
	std::map<int32_t, int32_t> m_game_player_num;

};

#endif
