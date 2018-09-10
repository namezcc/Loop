#ifndef PLAYER_MODULE_H
#define PLAYER_MODULE_H

#include "BaseModule.h"

#define ROOM_READY_OUT_TIME 60	//second

class MsgModule;
class TransMsgModule;
class NetObjectModule;
class SendProxyDbModule;
class EventModule;

class Player;
struct AccoutInfo;

enum READY_STATE
{
	RS_NONE,
	RS_SELECT,
	RS_CREATE,
};


struct ReadyInfo
{
	int64_t pid;
	int64_t roleId;
	int64_t outTime;
	int8_t state;
};

struct RoomPlayer
{
	int32_t sock;
	SHARE<Player> m_player;
	SHARE<AccoutInfo> m_account;
};

class PlayerModule:public BaseModule
{
public:
	PlayerModule(BaseLayer* l);
	~PlayerModule();

protected:
	virtual void Init() override;

	void OnReadyTakePlayer(SHARE<BaseMsg>& comsg);
	void OnPlayerEnter(NetMsg* msg);
	void OnCreatePlayer(SHARE<BaseMsg>& comsg, c_pull& pull, SHARE<BaseCoro>& coro);
	void OnClientClose(const int32_t& sock);

	void SendPlayerInfo(SHARE<RoomPlayer>& player);

	SHARE<RoomPlayer> AddPlayer(const int32_t& sock, SHARE<Player>& player);
	void RemovePlayer(const int64_t& pid);
	void KickChangePlayer(const int32_t& newSock, const int64_t& pid);

	SHARE<RoomPlayer> GetRoomPlayer(const int32_t& sock);
	SHARE<RoomPlayer> GetRoomPlayer(const int64_t& pid);
	SHARE<Player> GetPlayer(const int32_t& sock);
	SHARE<Player> GetPlayer(const int64_t& pid);

private:
	
	MsgModule* m_msgModule;
	TransMsgModule* m_transModule;
	NetObjectModule* m_netobjModule;
	SendProxyDbModule* m_sendProxyDb;
	EventModule* m_eventModule;

	std::unordered_map<int64_t, SHARE<ReadyInfo>> m_readyTable;
	std::unordered_map<int64_t, SHARE<RoomPlayer>> m_playerTable;
	std::unordered_map<int32_t, SHARE<RoomPlayer>> m_sockPlayer;
};

#endif
