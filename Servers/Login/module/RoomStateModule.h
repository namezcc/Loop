#ifndef ROOM_STATE_MODULE_H
#define ROOM_STATE_MODULE_H

#include "BaseModule.h"
#include "CommonDefine.h"

class MsgModule;
class TransMsgModule;
class EventModule;
class NetObjectModule;

class RoomStateModule:public BaseModule
{
public:
	RoomStateModule(BaseLayer* l);
	~RoomStateModule();

	ServerInfoState* GetRandRoom();
	ServerInfoState* getRoom(int32_t roomid);

	ServerPath& GetRoomPath(const int32_t& roomId);
	bool isRoomOpen(int32_t roomId);
protected:
	virtual void Init() override;

	void OnRoomState(NetMsg* msg);

	void RemoveRoom(const int32_t& rid);

private:
	MsgModule * m_msgModule;
	TransMsgModule* m_transModule;
	EventModule* m_eventModule;
	NetObjectModule* m_netobjModule;

	std::vector<ServerInfoState> m_roomArray;
	std::map<int32_t, int32_t> m_opemRoom;

	ServerPath m_roomPath;
};

#endif
