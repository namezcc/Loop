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

class RoomTransModule:public BaseModule
{
public:
	RoomTransModule(BaseLayer* l);
	~RoomTransModule();

protected:
	virtual void Init() override;

	void onRoomBusyState(NetMsg* msg);
	void onRoomPlayerLogout(NetMsg* msg);
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
	
};

#endif
