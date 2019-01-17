#include "RoomStateModule.h"
#include "MsgModule.h"
#include "TransMsgModule.h"
#include "EventModule.h"
#include "NetObjectModule.h"

#include "protoPB/server/server.pb.h"
#include "protoPB/base/LPSql.pb.h"

RoomStateModule::RoomStateModule(BaseLayer * l):BaseModule(l)
{
}

RoomStateModule::~RoomStateModule()
{
}

RoomServer * RoomStateModule::GetRandRoom()
{
	if(m_roomArray.size()<=0)
		return NULL;

	auto randidx = rand() % m_roomArray.size();
	return m_roomArray[randidx];
}

RoomServer * RoomStateModule::GetRandRoom(const int32_t & roomId)
{
	auto it = m_roomTable.find(roomId);
	if (it == m_roomTable.end())
		return NULL;
	return it->second.get();
}

std::vector<SHARE<ServerNode>>& RoomStateModule::GetRoomPath(const int32_t & roomId)
{
	m_roomPath[2]->serid = roomId;
	return m_roomPath;
}

void RoomStateModule::Init()
{
	m_msgModule = GET_MODULE(MsgModule);
	m_transModule = GET_MODULE(TransMsgModule);
	m_eventModule = GET_MODULE(EventModule);
	m_netobjModule = GET_MODULE(NetObjectModule);

	m_msgModule->AddMsgCallBack(N_ROOM_STATE, this, &RoomStateModule::OnRoomState);
	m_msgModule->AddMsgCallBack(N_ACK_ROOM_LIST, this, &RoomStateModule::OnAckRoomList);

	m_eventModule->AddEventCall(E_SERVER_CONNECT,BIND_EVENT(OnServerConnect,SHARE<NetServer>));

	for (size_t i = 0; i < 3; i++)
	{
		m_roomPath.push_back(GET_SHARE(ServerNode));
	}
	*m_roomPath[0] = *GetLayer()->GetServer();
	m_roomPath[1]->type = SERVER_TYPE::LOOP_ROOM_STATE;
	m_roomPath[1]->serid = 1;
	m_roomPath[2]->type = SERVER_TYPE::LOOP_ROOM;
}

void RoomStateModule::OnRoomState(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::RoomState, msg);

	if (pbMsg.state() == ROOM_CLOSE || pbMsg.state() == ROOM_FULL)
		RemoveRoom(pbMsg.id());
	else
	{
		if(m_roomTable.find(pbMsg.id()) != m_roomTable.end())
			RemoveRoom(pbMsg.id());

		auto room = GET_SHARE(RoomServer);
		room->id = pbMsg.id();
		room->ip = pbMsg.ip();
		room->port = pbMsg.port();
		room->index = m_roomArray.size();

		m_roomTable[room->id] = room;
		m_roomArray.push_back(room.get());
	}
}

void RoomStateModule::OnAckRoomList(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::RoomStateList, msg);

	m_roomArray.clear();
	m_roomTable.clear();

	for (auto rm:pbMsg.list())
	{
		auto room = GET_SHARE(RoomServer);
		room->id = rm.id();
		room->ip = rm.ip();
		room->port = rm.port();
		room->index = m_roomArray.size();

		m_roomTable[room->id] = room;
		m_roomArray.push_back(room.get());
	}
}

void RoomStateModule::OnServerConnect(SHARE<NetServer>& ser)
{
	if (ser->type == SERVER_TYPE::LOOP_ROOM_STATE)
	{
		LPMsg::EmptyPB msg;
		ServerNode toser{ ser->type ,ser->serid};
		
		//m_transModule->SendToServer(toser, N_REQ_ROOM_LIST, msg);
		m_netobjModule->SendNetMsg(ser->socket, N_REQ_ROOM_LIST, msg);
	}
}

void RoomStateModule::RemoveRoom(const int32_t & rid)
{
	auto it = m_roomTable.find(rid);
	if (it == m_roomTable.end())
		return;
	if (m_roomArray.size() > 1)
	{
		m_roomArray.back()->index = it->second->index;
		m_roomArray[it->second->index] = m_roomArray.back();
	}
	m_roomArray.pop_back();
	m_roomTable.erase(it);
}
