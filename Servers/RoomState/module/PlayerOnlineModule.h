#ifndef PLAYER_ONLINE_MOD_H
#define PLAYER_ONLINE_MOD_H

#include "BaseModule.h"

class MsgModule;
class TransMsgModule;
class NetObjectModule;
class RedisModule;
class RoomTransModule;

struct PlayerRoomInfo
{
	int32_t uid;
	int32_t gate_id;
	std::string ip;
	int32_t port;
	int32_t room_id;

	std::string getString();
	bool parse(const std::string& str);
};

class PlayerOnlineModule:public BaseModule
{
public:
	PlayerOnlineModule(BaseLayer* l):BaseModule(l)
	{}
	~PlayerOnlineModule()
	{}

	void onPlayerOnline(SHARE<BaseMsg>& msg);
	void onPlayerLogout(NetMsg* msg);
	void onCheckPlayerOnline(NetMsg* msg);
	void onPlayerTransMsg(NetMsg* msg);

	void onPlayerOffline(int64_t pid);

protected:
	// Í¨¹ý BaseModule ¼Ì³Ð
	virtual void Init() override;
	virtual void AfterInit() override;

	void loadPlayerLogin();
	void sendGatePlayerLogin(PlayerRoomInfo& info,int32_t key);

	MsgModule* m_msg_mod;
	TransMsgModule* m_trans_mod;
	NetObjectModule* m_net_mod;
	RedisModule* m_redis_mod;
	RoomTransModule* m_room_mgr;

	std::map<int64_t, int32_t> m_online_info;

	std::map<int32_t, PlayerRoomInfo> m_player_online;
};

#endif // !PLAYER_ONLINE_MOD_H

