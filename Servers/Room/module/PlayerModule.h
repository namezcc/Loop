#ifndef PLAYER_MODULE_H
#define PLAYER_MODULE_H

#include "BaseModule.h"
#include "GameReflectData.h"
#include "ConfigObjects.h"

#define ROOM_READY_OUT_TIME 60	//second

class MsgModule;
class TransMsgModule;
class NetObjectModule;
class SendProxyDbModule;
class EventModule;

enum READY_STATE
{
	RS_NONE,
	RS_SELECT,
	RS_CREATE,
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
	int64_t outTime;
	int8_t state;
};

struct MatchInfo
{
	int8_t m_state;	//0: none 1: in match 2: in Battle
	int16_t m_proxyId;
	int16_t m_matchSerId;
	int32_t m_sceneId;
	std::string	m_battleIp;
	int32_t m_battlePort;
};

struct RoomPlayer:public LoopObject
{
	int32_t sock;
	SHARE<Player> m_player;
	SHARE<AccoutInfo> m_account;
	SHARE<MatchInfo> m_matchInfo;

	virtual void init(FactorManager*f)
	{
	}

	virtual void recycle(FactorManager*f)
	{
		m_player = NULL;
		m_account = NULL;
		m_matchInfo = NULL;
	}
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

public:
	SHARE<RoomPlayer> GetRoomPlayer(const int32_t& sock);
	SHARE<RoomPlayer> GetRoomPlayer(const int64_t& pid);
	SHARE<Player> GetPlayer(const int32_t& sock);
	SHARE<Player> GetPlayer(const int64_t& pid);
	SHARE<RoomPlayer> CheckRoomPlayer(const int32_t& sock);

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
