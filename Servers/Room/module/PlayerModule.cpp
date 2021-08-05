#include "PlayerModule.h"
#include "MsgModule.h"
#include "TransMsgModule.h"
#include "NetObjectModule.h"
#include "EventModule.h"
#include "RoomMsgDefine.h"
#include "RoomModule.h"

#include "CommonDefine.h"
#include "ServerMsgDefine.h"

#include "protoPB/server/server.pb.h"
#include "protoPB/client/define.pb.h"
#include "protoPB/client/room.pb.h"
#include "protoPB/client/login.pb.h"
#include "protoPB/server/dbdata.pb.h"

PlayerModule::PlayerModule(BaseLayer * l):BaseModule(l)
{
}

PlayerModule::~PlayerModule()
{
}

void PlayerModule::Init()
{
	m_msgModule = GET_MODULE(MsgModule);
	m_transModule = GET_MODULE(TransMsgModule);
	m_netobjModule = GET_MODULE(NetObjectModule);
	m_eventModule = GET_MODULE(EventModule);
	m_room_mod = GET_MODULE(RoomModuloe);

	m_msgModule->AddMsgCall(N_ROOM_READY_TAKE_PLAYER, BIND_NETMSG(OnReadyTakePlayer));
	m_msgModule->AddMsgCall(LPMsg::CM_ENTER_ROOM, BIND_NETMSG(OnPlayerEnter));
	m_msgModule->AddMsgCall(LPMsg::CM_CREATE_ROLE, BIND_NETMSG(OnCreatePlayer));

	m_msgModule->AddMsgCall(N_TROM_GET_ROLE_LIST, BIND_NETMSG(onGetRoleList));

	m_eventModule->AddEventCall(E_SOCKET_CLOSE,BIND_EVENT(OnClientClose,int32_t));
}

void PlayerModule::OnReadyTakePlayer(NetMsg* msg)
{
	TRY_PARSEPB(LPMsg::RoomInfo, msg);

	auto it = m_readyTable.find(pbMsg.pid());
	if (it == m_readyTable.end())
	{
		ReadyInfo ready = {};
		ready.pid = pbMsg.pid();
		ready.outTime = Loop::GetSecend() + ROOM_READY_OUT_TIME;
		m_readyTable[pbMsg.pid()] = ready;
	}
}

void PlayerModule::OnPlayerEnter(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::ReqEnterRoom, msg);

	auto it = m_readyTable.find(pbMsg.pid());
	if (it == m_readyTable.end())
	{
		m_netobjModule->CloseNetObject(msg->socket);
		return;
	}
	else if(it->second.sock != msg->socket)
	{//顶号

		if (it->second.sock > 0)
		{
			m_netobjModule->CloseNetObject(it->second.sock);

			
		}

		m_netobjModule->AcceptConn(msg->socket);
		it->second.sock = msg->socket;
	}

	if (it->second.state != READY_STATE::RS_NONE)
	{
		LP_WARN << "in doing select or create pid:"<< pbMsg.pid();
		return;
	}

	//get role list
	it->second.state = RS_SELECT;
	m_room_mod->doSqlOperation(pbMsg.pid(), SOP_ROLE_SELET, LPMsg::propertyInt32{}, N_TROM_GET_ROLE_LIST);

}

void PlayerModule::OnCreatePlayer(NetMsg* msg)
{
	TRY_PARSEPB(LPMsg::ReqCreateRole, msg);
	auto it = m_readyTable.find(pbMsg.pid());
	if (it == m_readyTable.end())
	{
		m_netobjModule->CloseNetObject(msg->socket);
		return;
	}

	if (pbMsg.name().size() >= 30)
	{
		LP_INFO << "name to lone name:" << pbMsg.name();
		return;
	}

	if (it->second.state != READY_STATE::RS_NONE)
	{
		LP_WARN << "onCreate in doing select or create pid:" << pbMsg.pid();
		return;
	}

	it->second.state = READY_STATE::RS_CREATE;
	LPMsg::DB_player spb;
	spb.set_uid(pbMsg.pid());
	spb.set_rid(1);
	spb.set_level(1);
	spb.set_name(pbMsg.name());
	m_room_mod->doSqlOperation(pbMsg.pid(), SOP_CREATE_ROLE, spb, N_TROM_GET_ROLE_LIST);
}

void PlayerModule::onGetRoleList(NetMsg * msg)
{
	auto uid = msg->m_buff->readInt64();
	auto it = m_readyTable.find(uid);
	if (it == m_readyTable.end())
		return;

	it->second.state = RS_NONE;
	TRY_PARSEPB(LPMsg::DB_roleList, msg);

	LPMsg::RoleList spb;

	for (auto r:pbMsg.roles())
	{
		auto role = spb.add_roles();
		role->set_rid(r.rid());
		role->set_name(r.name());
		role->set_level(r.level());
	}

	m_netobjModule->SendNetMsg(it->second.sock, LPMsg::SM_SELF_ROLE_INFO, spb);
}

void PlayerModule::OnClientClose(const int32_t & sock)
{
	auto it = m_sockPlayer.find(sock);
	if (it == m_sockPlayer.end())
		return;

	auto server = GetLayer()->GetServer();
	
	Reflect<AccoutInfo> rf(it->second->m_account.get());
	rf.Set_lastLogoutTime(Loop::GetStringTime());
	rf.Set_lastRoomId(server->serid);

	LP_INFO << "player offLine id:" << it->second->m_player->id << " name:" << it->second->m_player->Get_name();

	it->second->sock = -1;
	if(it->second->m_matchInfo->m_state == PlayerState::PS_NONE)
		m_playerTable.erase(it->second->m_player->Get_id());
	m_sockPlayer.erase(it);
}

void PlayerModule::SendPlayerInfo(SHARE<RoomPlayer>& player)
{
	LPMsg::GameObject msg;
	player->m_player->ParsePB(msg);
	m_netobjModule->SendNetMsg(player->sock, LPMsg::SM_ENTER_ROOM, msg);

	if (player->m_matchInfo->m_key > 0)
	{
		//send reEnter scene

		LPMsg::AckEnterBattle ackmsg;
		auto match = player->m_matchInfo;

		ackmsg.set_ip(match->m_battleIp);
		ackmsg.set_port(match->m_battlePort);
		ackmsg.set_sceneid(match->m_sceneId);
		ackmsg.set_key(match->m_key);
		m_netobjModule->SendNetMsg(player->sock, LPMsg::SM_OLD_BATTLE_INFO, ackmsg);
	}
}

SHARE<RoomPlayer> PlayerModule::AddPlayer(const int32_t & sock, SHARE<Player>& player)
{
	auto rmplayer = GET_SHARE(RoomPlayer);
	rmplayer->m_player = player;
	rmplayer->sock = sock;
	rmplayer->m_matchInfo = GET_SHARE(MatchInfo);
	rmplayer->m_matchInfo->m_state = PlayerState::PS_NONE;
	rmplayer->m_matchInfo->m_key = 0;

	m_playerTable[player->Get_id()] = rmplayer;
	m_sockPlayer[sock] = rmplayer;
	return rmplayer;
}

void PlayerModule::RemovePlayer(const int64_t & pid)
{
	auto it = m_playerTable.find(pid);
	if (it == m_playerTable.end())
		return;

	m_netobjModule->CloseNetObject(it->second->sock);
	m_sockPlayer.erase(it->second->sock);
	m_playerTable.erase(it);
}

void PlayerModule::KickChangePlayer(const int32_t & newSock, const int64_t & pid)
{
	auto ply = GetRoomPlayer(pid);
	if (!ply || ply->sock == newSock)
		return;
	if (ply->sock != -1)
	{
		m_sockPlayer.erase(ply->sock);
		m_netobjModule->CloseNetObject(ply->sock);
	}

	ply->sock = newSock;
	m_sockPlayer[newSock] = ply;
}

SHARE<RoomPlayer> PlayerModule::GetRoomPlayer(const int32_t & sock)
{
	auto it = m_sockPlayer.find(sock);
	if (it == m_sockPlayer.end())
		return NULL;
	return it->second;
}

SHARE<RoomPlayer> PlayerModule::GetRoomPlayer(const int64_t & pid)
{
	auto it = m_playerTable.find(pid);
	if (it == m_playerTable.end())
		return NULL;
	return it->second;
}

SHARE<Player> PlayerModule::GetPlayer(const int32_t & sock)
{
	auto it = m_sockPlayer.find(sock);
	if (it == m_sockPlayer.end())
		return NULL;
	return it->second->m_player;
}

SHARE<Player> PlayerModule::GetPlayer(const int64_t & pid)
{
	auto it = m_playerTable.find(pid);
	if (it == m_playerTable.end())
		return NULL;
	return it->second->m_player;
}

SHARE<RoomPlayer> PlayerModule::CheckRoomPlayer(const int32_t & sock)
{
	auto player = GetRoomPlayer(sock);
	if (!player)
	{
		m_netobjModule->CloseNetObject(sock);
		return NULL;
	}
	return player;
}
