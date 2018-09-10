#ifndef ROOM_TRANS_MODULE_H
#define ROOM_TRANS_MODULE_H

#include "BaseModule.h"

class MsgModule;
class TransMsgModule;
class EventModule;
class NetObjectModule;

class RoomTransModule:public BaseModule
{
public:
	RoomTransModule(BaseLayer* l);
	~RoomTransModule();

protected:
	virtual void Init() override;

	void OnReqRoomList(NetMsg* msg);
	void OnServerConnect(SHARE<NetServer>& ser);
	void OnServerClose(SHARE<NetServer>& ser);

private:
	MsgModule * m_msgModule;
	TransMsgModule* m_transModule;
	EventModule* m_eventModule;
	NetObjectModule* m_netobjModule;
};

#endif
