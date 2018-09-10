#ifndef ROOM_STATE_MODULE_H
#define ROOM_STATE_MODULE_H

#include "BaseModule.h"

class MsgModule;
class TransMsgModule;
class EventModule;
class NetObjectModule;

struct RoomServer
{
	int32_t id;
	int32_t index;
	int32_t port;
	std::string ip;
};

enum ROOM_STATE
{
	ROOM_OPEN,
	ROOM_CLOSE,
	ROOM_FULL,
	ROMM_FREE,
};

class RoomStateModule:public BaseModule
{
public:
	RoomStateModule(BaseLayer* l);
	~RoomStateModule();

	RoomServer* GetRandRoom();
	RoomServer* GetRandRoom(const int32_t& roomId);

	std::vector<SHARE<ServerNode>>& GetRoomPath(const int32_t& roomId);
protected:
	virtual void Init() override;

	void OnRoomState(NetMsg* msg);
	void OnAckRoomList(NetMsg* msg);
	void OnServerConnect(SHARE<NetServer>& ser);

	void RemoveRoom(const int32_t& rid);

private:
	MsgModule * m_msgModule;
	TransMsgModule* m_transModule;
	EventModule* m_eventModule;
	NetObjectModule* m_netobjModule;

	std::unordered_map<int32_t, SHARE<RoomServer>> m_roomTable;
	std::vector<RoomServer*> m_roomArray;

	std::vector<SHARE<ServerNode>> m_roomPath;
};

#endif
