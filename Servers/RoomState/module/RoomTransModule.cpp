#include "RoomTransModule.h"
#include "MsgModule.h"
#include "TransMsgModule.h"
#include "EventModule.h"
#include "NetObjectModule.h"
#include "ServerMsgDefine.h"
#include "RedisModule.h"
#include "PlayerOnlineModule.h"
#include "ToolFunction.h"

#include "protoPB/server/server.pb.h"
#include "protoPB/server/server_msgid.pb.h"


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
	m_redisModule = GET_MODULE(RedisModule);
	m_ply_online = GET_MODULE(PlayerOnlineModule);

	m_msgModule->AddMsgCall(N_ROOM_STATE, BIND_NETMSG(onRoomBusyState));
	m_msgModule->AddMsgCall(N_TRMM_PLAYER_LOGOUT, BIND_NETMSG(onRoomPlayerLogout));
	m_msgModule->AddMsgCall(LPMsg::IM_RMGR_PLAYER_ONLINE_NUM, BIND_NETMSG(onPlayerNum));

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

void RoomTransModule::onRoomPlayerLogout(NetMsg * msg)
{
	auto pack = msg->m_buff;

	auto uid = pack->readInt64();
	auto pid = pack->readInt64();

	auto key = "login:" + Loop::to_string(uid);
	m_redisModule->HSet(key, "last_logout", Loop::to_string(Loop::GetSecend()));
	m_redisModule->expire(key, 1800);

	m_ply_online->onPlayerOffline(pid);
}

void RoomTransModule::onPlayerNum(NetMsg * msg)
{
	auto pack = msg->m_buff;
	auto stype = pack->readInt32();
	auto sid = pack->readInt32();
	auto num = pack->readInt32();

	if (stype == LOOP_GATE)
		m_gate_player_num[sid] = num;
	else if (stype == LOOP_ROOM)
		m_game_player_num[sid] = num;
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
			auto& msg = *pb.add_list();
			msg.set_id(s.second.server_id);
			msg.set_ip(s.second.ip);
			msg.set_port(s.second.port);
			msg.set_state(s.second.state);
		}

		m_transModule->SendToAllServer(SERVER_TYPE::LOOP_LOGIN, N_ROOM_STATE, pb);
	}
	else if (ser->type == SERVER_TYPE::LOOP_ROOM)
	{
		ServerInfoState s = {};
		s.ip = ser->ip;
		s.port = ser->port;
		s.server_id = ser->serid;
		s.state = ServerBusyState::SBS_NORMAL;
		m_room_state[s.server_id] = s;
	}
}

void RoomTransModule::OnServerClose(SHARE<NetServer>& ser)
{
	if (ser->type == SERVER_TYPE::LOOP_ROOM)
	{
		m_game_player_num.erase(ser->serid);
	}
	else if (ser->type == SERVER_TYPE::LOOP_GATE)
	{
		m_gate_player_num.erase(ser->serid);
	}
}

#define GATE_MAX_PLAYER 10000
#define GAME_MAX_PLAYER 20000

bool RoomTransModule::chooseGateAndRoom(PlayerRoomInfo & info)
{
	std::vector<std::pair<int32_t, int32_t>> randvec;
	for (auto i:m_gate_player_num)
	{
		if (i.second < GATE_MAX_PLAYER)
			randvec.push_back(std::make_pair(GATE_MAX_PLAYER - i.second, i.first));
	}

	if (randvec.size() == 0)
		return false;

	auto idx = randWeightPair(randvec);
	auto sid = randvec[idx].second;
	auto server = m_transModule->GetServer(LOOP_GATE, sid);
	if (server == NULL)
		return false;

	info.gate_id = sid;
	info.ip = server->ip;
	info.port = server->port;

	randvec.clear();
	for (auto i : m_game_player_num)
	{
		if (i.second < GAME_MAX_PLAYER)
			randvec.push_back(std::make_pair(GAME_MAX_PLAYER - i.second, i.first));
	}
	if (randvec.size() == 0) return false;

	idx = randWeightPair(randvec);
	sid = randvec[idx].second;
	info.room_id = sid;
	return true;
}
