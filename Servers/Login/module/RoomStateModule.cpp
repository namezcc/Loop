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

ServerInfoState * RoomStateModule::GetRandRoom()
{
	if (m_roomArray.empty())
		return NULL;

	auto idx = rand() % m_roomArray.size();
	return &m_roomArray[idx];
}

ServerInfoState * RoomStateModule::getRoom(int32_t roomid)
{
	if (!isRoomOpen(roomid))
		return NULL;

	for (size_t i = 0; i < m_roomArray.size(); i++)
	{
		if (m_roomArray[i].server_id == roomid)
			return &m_roomArray[i];
	}
	return NULL;
}

ServerPath& RoomStateModule::GetRoomPath(const int32_t & roomId)
{
	m_roomPath[2].serid = roomId;
	return m_roomPath;
}

bool RoomStateModule::isRoomOpen(int32_t roomId)
{
	return m_opemRoom.find(roomId) != m_opemRoom.end();
}

void RoomStateModule::Init()
{
	m_msgModule = GET_MODULE(MsgModule);
	m_transModule = GET_MODULE(TransMsgModule);
	m_eventModule = GET_MODULE(EventModule);
	m_netobjModule = GET_MODULE(NetObjectModule);

	m_msgModule->AddMsgCall(N_ROOM_STATE,BIND_NETMSG(OnRoomState));


	m_roomPath.push_back(*GetLayer()->GetServer());
	m_roomPath.push_back(ServerNode{ SERVER_TYPE::LOOP_ROOM_MANAGER,1});
	m_roomPath.push_back(ServerNode{ SERVER_TYPE::LOOP_ROOM,1 });
}

void RoomStateModule::OnRoomState(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::RoomStateList, msg);

	for (auto& m : pbMsg.list())
	{
		if (m.state() != SBS_NORMAL)
		{
			RemoveRoom(m.id());
			if (m.state() == SBS_CLOSE)
				m_opemRoom.erase(m.id());
			else
				m_opemRoom[m.id()] = SBS_BUSY;
		}
		else
		{
			auto it = m_opemRoom.find(m.id());
			if (it != m_opemRoom.end() && it->second == SBS_NORMAL)
				continue;

			ServerInfoState s = {};
			s.ip = m.ip();
			s.port = m.port();
			s.server_id = m.id();
			m_roomArray.push_back(s);
			m_opemRoom[s.server_id] = SBS_NORMAL;
		}
	}
}

void RoomStateModule::RemoveRoom(const int32_t & rid)
{
	for (size_t i = 0; i < m_roomArray.size(); i++)
	{
		if (m_roomArray[i].server_id == rid)
		{
			std::swap(m_roomArray[i], m_roomArray[m_roomArray.size() - 1]);
			m_roomArray.pop_back();
			break;
		}
	}
}
