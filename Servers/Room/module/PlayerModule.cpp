#include "PlayerModule.h"
#include "MsgModule.h"
#include "TransMsgModule.h"
#include "NetObjectModule.h"
#include "SendProxyDbModule.h"
#include "EventModule.h"
#include "RoomMsgDefine.h"

#include "protoPB/server/server.pb.h"
#include "protoPB/client/define.pb.h"
#include "protoPB/client/room.pb.h"

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
	m_sendProxyDb = GET_MODULE(SendProxyDbModule);
	m_eventModule = GET_MODULE(EventModule);

	m_msgModule->AddMsgCallBack(N_ROOM_READY_TAKE_PLAYER, this, &PlayerModule::OnReadyTakePlayer);
	m_msgModule->AddMsgCallBack(LPMsg::N_REQ_ENTER_ROOM, this, &PlayerModule::OnPlayerEnter);
	m_msgModule->AddAsynMsgCallBack(LPMsg::N_REQ_CREATE_ROLE, this, &PlayerModule::OnCreatePlayer);

	m_eventModule->AddEventCall(E_SOCKET_CLOSE,BIND_EVENT(OnClientClose,int32_t));
}

void PlayerModule::OnReadyTakePlayer(SHARE<BaseMsg>& comsg)
{
	auto msg = (NetServerMsg*)comsg->m_data;
	TRY_PARSEPB(LPMsg::RoomReadyInfo, msg);

	auto it = m_readyTable.find(pbMsg.pid());
	if (it == m_readyTable.end())
	{
		auto ready = GET_SHARE(ReadyInfo);
		ready->pid = pbMsg.pid();
		ready->outTime = GetSecend() + ROOM_READY_OUT_TIME;
		ready->roleId = pbMsg.roleid();
		ready->state = RS_NONE,
		m_readyTable[pbMsg.pid()] = ready;
	}
	m_transModule->ResponseBackServerMsg(msg->path, comsg, pbMsg);
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

	m_netobjModule->AcceptConn(msg->socket);

	if (it->second->state != READY_STATE::RS_NONE)
	{
		LP_WARN << "in doing select or create pid:"<< pbMsg.pid();
		return;
	}

	auto account = GET_SHARE(AccoutInfo);
	account->id = pbMsg.pid();
	Reflect<AccoutInfo> rf(account.get());
	rf.Set_lastLoginTime(GetStringTime());
	m_sendProxyDb->UpdateDbGroup(rf, account->id);

	//have role 
	//no create role
	if (it->second->roleId == 0)
	{
		LPMsg::EmptyPB ackpb;
		m_netobjModule->SendNetMsg(msg->socket, LPMsg::N_ACK_CREATE_ROLE, ackpb);
	}
	else
	{//yes select
		auto itrmp = m_playerTable.find(it->second->roleId);
		if (itrmp != m_playerTable.end())
		{
			//kick old
			if(itrmp->second->sock != msg->socket)
				KickChangePlayer(msg->socket, it->second->roleId);
			SendPlayerInfo(itrmp->second);
			return;
		}

		m_msgModule->DoCoroFunc([this,&it,&msg,account](c_pull& pull,SHARE<BaseCoro>& coro) {
			auto player = GET_SHARE(Player);
			auto rdinfo = it->second;
			player->Set_id(rdinfo->roleId);
			auto socket = msg->socket;
			rdinfo->state = READY_STATE::RS_SELECT;

			ExitCall failCall([this,&rdinfo,&socket,&coro]() {
				if (!coro->IsFail())
					return;
				LP_ERROR << "Select Role Faile";
				m_netobjModule->CloseNetObject(socket);
				rdinfo->state = READY_STATE::RS_NONE;
			});

			auto ackres = m_sendProxyDb->RequestSelectDbGroup(*player,*player->m_sql, it->second->roleId, pull, coro);
			if (!ackres)
			{
				coro->SetFail();
				return;
			}
			auto rmplayer = AddPlayer(socket, player);
			rmplayer->m_account = account;
			SendPlayerInfo(rmplayer);
			m_readyTable.erase(rdinfo->pid);
		});
	}
}

void PlayerModule::OnCreatePlayer(SHARE<BaseMsg>& comsg, c_pull & pull, SHARE<BaseCoro>& coro)
{
	auto msg = (NetMsg*)comsg->m_data;
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

	if (it->second->state != READY_STATE::RS_NONE)
	{
		LP_WARN << "onCreate in doing select or create pid:" << pbMsg.pid();
		return;
	}

	it->second->state = READY_STATE::RS_CREATE;
	auto player = GET_SHARE(Player);
	player->Set_name(pbMsg.name());

	auto rdinfo = it->second;
	ExitCall failCall([this,&rdinfo,&msg,&coro]() {
		if (!coro->IsFail())
			return;
		LP_ERROR << "Create Role Faile";
		m_netobjModule->CloseNetObject(msg->socket);
		rdinfo->state = READY_STATE::RS_NONE;
	});
	//check have same name
	auto checkres = m_sendProxyDb->RequestSelectDbGroup(*player,*player->m_sql, pbMsg.pid(), pull, coro,true);
	if (checkres)
	{
		LP_WARN << "create role fail same Name:" << pbMsg.name();
		rdinfo->state = READY_STATE::RS_NONE;
		return;
	}

	auto ackres = m_sendProxyDb->RequestInsertSelectDbGroup(*player,*player->m_sql, pbMsg.pid(), pull, coro,true);
	if (!ackres)
	{
		LP_ERROR << "Create Insert player Error";
		coro->SetFail();
		return;
	}

	//save role id
	auto account = GET_SHARE(AccoutInfo);
	account->id = pbMsg.pid();
	Reflect<AccoutInfo> rf(account.get());
	rf.Set_roleId(player->Get_id());
	m_sendProxyDb->UpdateDbGroup(rf, account->id);

	auto rmplayer = AddPlayer(msg->socket, player);
	rmplayer->m_account = account;
	SendPlayerInfo(rmplayer);
	m_readyTable.erase(it);
}

void PlayerModule::OnClientClose(const int32_t & sock)
{
	auto it = m_sockPlayer.find(sock);
	if (it == m_sockPlayer.end())
		return;

	auto server = GetLayer()->GetServer();
	
	Reflect<AccoutInfo> rf(it->second->m_account.get());
	rf.Set_lastLogoutTime(GetStringTime());
	rf.Set_lastRoomId(server->serid);

	m_sendProxyDb->UpdateDbGroup(rf, it->second->m_account->id);

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
	m_netobjModule->SendNetMsg(player->sock, LPMsg::N_ACK_ENTER_ROOM, msg);

	if (player->m_matchInfo->m_key > 0)
	{
		//send reEnter scene

		LPMsg::AckEnterBattle ackmsg;
		auto match = player->m_matchInfo;

		ackmsg.set_ip(match->m_battleIp);
		ackmsg.set_port(match->m_battlePort);
		ackmsg.set_sceneid(match->m_sceneId);
		ackmsg.set_key(match->m_key);
		m_netobjModule->SendNetMsg(player->sock, LPMsg::N_ACK_OLD_BATTLE_INFO, ackmsg);
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
