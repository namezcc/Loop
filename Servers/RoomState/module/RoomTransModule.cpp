#include "RoomTransModule.h"
#include "MsgModule.h"
#include "TransMsgModule.h"
#include "EventModule.h"
#include "NetObjectModule.h"

#include "protoPB/server/server.pb.h"


RoomTransModule::RoomTransModule(BaseLayer * l):BaseModule(l)
{
}

RoomTransModule::~RoomTransModule()
{
}

void RoomTransModule::Init()
{
	m_msgModule = GET_MODULE(MsgModule);
	m_transModule = GET_MODULE(TransMsgModule);
	m_eventModule = GET_MODULE(EventModule);
	m_netobjModule = GET_MODULE(NetObjectModule);

	m_msgModule->AddMsgCall(N_ROOM_STATE, BIND_NETMSG(onRoomBusyState));

	m_eventModule->AddEventCall(E_SERVER_CONNECT,BIND_EVENT(OnServerConnect,SHARE<NetServer>));
	m_eventModule->AddEventCall(E_SERVER_CLOSE,BIND_EVENT(OnServerClose, SHARE<NetServer>));
}

void RoomTransModule::onRoomBusyState(NetMsg * msg)
{
	auto pack = msg->m_buff;

	auto id = pack->readInt32();
	auto state = pack->readInt32();

	auto& sinfo = m_room_state[id];
	sinfo.state = state;

	LPMsg::RoomStateList pb;
	auto& pbmsg = *pb.add_list();
	pbmsg.set_id(sinfo.server_id);
	pbmsg.set_ip(sinfo.ip);
	pbmsg.set_port(sinfo.port);
	pbmsg.set_state(state);

	LP_INFO << "room state id:" << id << "ip:" << sinfo.ip << "port:" << sinfo.port << "state:"<<state;

	m_transModule->SendToAllServer(SERVER_TYPE::LOOP_LOGIN, N_ROOM_STATE, pb);
}

void RoomTransModule::OnServerConnect(SHARE<NetServer>& ser)
{
	if (ser->type == SERVER_TYPE::LOOP_LOGIN)
	{
		if (m_room_state.empty())
			return;

		LPMsg::RoomStateList pb;
		
		for (auto& s:m_room_state)
		{
			if (s.second.state == SBS_NORMAL)
			{
				auto& msg = *pb.add_list();
				msg.set_id(s.second.server_id);
				msg.set_ip(s.second.ip);
				msg.set_port(s.second.port);
				msg.set_state(SBS_NORMAL);
			}
		}

		if (pb.list_size() > 0)
			m_transModule->SendToAllServer(SERVER_TYPE::LOOP_LOGIN, N_ROOM_STATE, pb);
	}
	else if (ser->type == SERVER_TYPE::LOOP_ROOM)
	{
		ServerInfoState s = {};
		s.ip = ser->ip;
		s.port = ser->port;
		s.server_id = ser->serid;
		s.state = ServerBusyState::SBS_BUSY;
		m_room_state[s.server_id] = s;
	}
}

void RoomTransModule::OnServerClose(SHARE<NetServer>& ser)
{
	if (ser->type == SERVER_TYPE::LOOP_ROOM)
	{
		LPMsg::RoomStateList pb;
		auto& msg = *pb.add_list();
		msg.set_id(ser->serid);
		msg.set_ip(ser->ip);
		msg.set_port(ser->port);
		msg.set_state(ServerBusyState::SBS_CLOSE);

		m_transModule->SendToAllServer(LOOP_LOGIN, N_ROOM_STATE, pb);
	}
}
