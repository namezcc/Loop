#include "PlayerModule.h"
#include "MsgModule.h"
#include "TransMsgModule.h"
#include "NetObjectModule.h"
#include "EventModule.h"
#include "RoomMsgDefine.h"
#include "RoomModule.h"
#include "RoomLuaModule.h"

#include "CommonDefine.h"
#include "ServerMsgDefine.h"

#include "protoPB/server/server.pb.h"
#include "protoPB/client/define.pb.h"
#include "protoPB/client/room.pb.h"
#include "protoPB/client/login.pb.h"
#include "protoPB/server/dbdata.pb.h"

PlayerModule::PlayerModule(BaseLayer * l):BaseModule(l)
{
	memset(m_player, 0, sizeof(m_player));
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
	m_room_lua = GET_MODULE(RoomLuaModule);

	m_eventModule->AddEventCall(E_SOCKET_CLOSE,BIND_EVENT(OnClientClose,int32_t));

	m_msgModule->AddMsgCall(N_ROOM_READY_TAKE_PLAYER, BIND_NETMSG(OnReadyTakePlayer));
	m_msgModule->AddMsgCall(N_TROM_GET_ROLE_LIST, BIND_NETMSG(onGetRoleList));
	

	m_msgModule->AddMsgCall(LPMsg::CM_ENTER_ROOM, BIND_NETMSG(OnPlayerEnter));
	m_msgModule->AddMsgCall(LPMsg::CM_CREATE_ROLE, BIND_NETMSG(OnCreatePlayer));
	m_msgModule->AddMsgCall(LPMsg::CM_ENTER_GAME, BIND_NETMSG(onEnterGame));

}

void PlayerModule::OnReadyTakePlayer(NetMsg* msg)
{
	TRY_PARSEPB(LPMsg::RoomInfo, msg);

	LP_INFO << "take ready pid:" << pbMsg.pid();

	auto it = m_readyTable.find(pbMsg.pid());
	if (it == m_readyTable.end())
	{
		ReadyInfo ready = {};
		ready.pid = pbMsg.pid();
		ready.key = pbMsg.key();
		m_readyTable[pbMsg.pid()] = ready;
	}
	else
	{
		it->second.key = pbMsg.key();
	}
}

void PlayerModule::OnPlayerEnter(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::ReqEnterRoom, msg);

	auto it = m_readyTable.find(pbMsg.uid());
	if (it == m_readyTable.end())
	{
		LP_INFO << "player enter no ready uid:" << pbMsg.uid();
		m_netobjModule->CloseNetObject(msg->socket);
		return;
	}

	if (it->second.key != pbMsg.key())
	{
		m_netobjModule->CloseNetObject(msg->socket);
		return;
	}

	RoomPlayer* player = NULL;
	//顶号
	if (it->second.sock != msg->socket)
	{
		if (it->second.sock > 0)
		{
			auto oldply = m_player[it->second.sock];
			m_player[it->second.sock] = NULL;
			if (oldply == NULL)
			{
				LP_ERROR << "player enter oldplayer == NULL";
			}
			else
			{
				player = oldply;
			}
			m_netobjModule->CloseNetObject(it->second.sock);
		}

		if (m_player[msg->socket] != NULL)
		{
			LP_ERROR << "player enter error have player";
			LOOP_RECYCLE(m_player[msg->socket]);
			m_player[msg->socket] = NULL;
		}
		it->second.sock = msg->socket;
	}
	else
	{
		if (m_player[msg->socket] != NULL)		//已加载角色
			return;
	}

	if (player == NULL)
		player = GET_LOOP(RoomPlayer);

	player->uid = pbMsg.uid();
	player->sock = msg->socket;
	m_player[msg->socket] = player;

	it->second.state = RS_SELECT;
	m_netobjModule->AcceptConn(msg->socket);
	//get role list
	m_room_mod->doSqlOperation(pbMsg.uid(), SOP_ROLE_SELET, LPMsg::propertyInt32{}, N_TROM_GET_ROLE_LIST);

	//设置lua uid -> sock
	m_room_lua->setPlayerSock(pbMsg.uid(), msg->socket);
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

	if (it->second.sock != msg->socket)
		return;

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
	spb.set_pid(pbMsg.pid());
	spb.set_level(1);
	spb.set_name(pbMsg.name());
	m_room_mod->doSqlOperation(pbMsg.pid(), SOP_CREATE_ROLE, spb, N_TROM_GET_ROLE_LIST);
}

void PlayerModule::onEnterGame(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::propertyInt64, msg);

	auto player = getPlayerData(msg->socket);
	if (player == NULL)
		return;

	auto it = player->m_roles.find(pbMsg.data());
	if (it == player->m_roles.end())
		return;

	if (player->enter_pid > 0)
	{
		LP_ERROR << "have role in game uid:" << player->uid;
		return;
	}

	LP_INFO << "player enter rid:" << it->second.pid;

	player->enter_pid = pbMsg.data();
	m_room_mod->doSqlOperation(pbMsg.data(), SOP_LOAD_PLAYER_DATA, pbMsg, N_TROM_LOAD_ROLE_DATA);
}

void PlayerModule::onGetRoleList(NetMsg * msg)
{
	auto uid = msg->m_buff->readInt64();
	auto it = m_readyTable.find(uid);
	if (it == m_readyTable.end())
		return;

	it->second.state = RS_NONE;
	auto player = getPlayerData(it->second.sock);
	if (player == NULL)
	{
		LP_ERROR << "getrole player == null uid:" << it->second.pid;
		return;
	}

	TRY_PARSEPB(LPMsg::DB_roleList, msg);
	LPMsg::RoleList spb;

	for (auto r:pbMsg.roles())
	{
		auto role = spb.add_roles();
		role->set_pid(r.pid());
		role->set_name(r.name());
		role->set_level(r.level());

		RoleInfo rinfo = {};
		rinfo.pid = r.pid();
		rinfo.name = r.name();
		rinfo.level = r.level();
		player->m_roles[rinfo.pid] = rinfo;
	}

	m_netobjModule->SendNetMsg(it->second.sock, LPMsg::SM_SELF_ROLE_INFO, spb);
}

void PlayerModule::OnClientClose(const int32_t & sock)
{
	auto ply = getPlayerData(sock);
	if (ply == NULL)
		return;

	auto it = m_readyTable.find(ply->uid);
	if (it == m_readyTable.end() || it->second.sock != sock)
	{
		removePlayer(sock);
		LP_ERROR << "client close diff ready uid:" << ply->uid;
		return;
	}

	LP_INFO << "player offLine id:" << ply->uid;

	removePlayer(sock);
	m_readyTable.erase(it);
}

RoomPlayer * PlayerModule::getPlayerData(int32_t sock)
{
	return m_player[sock];
}

void PlayerModule::removePlayer(int32_t sock)
{
	auto ply = m_player[sock];
	if (ply)
	{
		LuaArgs arg;
		arg.pushArg(ply->uid);
		arg.pushArg(sock);
		m_room_lua->callLuaMsg(CTOL_PLAYER_OFFLINE, arg);

		LOOP_RECYCLE(ply);
		m_player[sock] = NULL;
	}
}
