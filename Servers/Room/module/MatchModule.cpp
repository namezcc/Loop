#include "MatchModule.h"
#include "MsgModule.h"
#include "ProxyNodeModule.h"
#include "TransMsgModule.h"
#include "PlayerModule.h"
#include "NetObjectModule.h"

#include "ServerMsgDefine.h"

#include "protoPB/server/sroom.pb.h"
#include "protoPB/client/room.pb.h"
#include "protoPB/client/define.pb.h"

MatchModule::MatchModule(BaseLayer * l):BaseModule(l)
{

}

MatchModule::~MatchModule()
{
}

void MatchModule::Init()
{
	m_msgModule = GET_MODULE(MsgModule);
	m_proxyNode = GET_MODULE(ProxyNodeModule);
	m_transModule = GET_MODULE(TransMsgModule);
	m_playerModule = GET_MODULE(PlayerModule);
	m_netObjModule = GET_MODULE(NetObjectModule);


	m_msgModule->AddMsgCallBack(LPMsg::CM_BEGIN_MATCH, this, &MatchModule::OnPlayerBeginBattle);
	m_msgModule->AddMsgCallBack(LPMsg::CM_STOP_MATCH, this, &MatchModule::OnReqStopMatch);

	m_msgModule->AddMsgCallBack(N_ACK_GET_MATCH_SERVER, this, &MatchModule::OnAckGetMatchServer);
	m_msgModule->AddMsgCallBack(N_ACK_MATCH_BATTLE, this, &MatchModule::OnAckMatchBattle);
	m_msgModule->AddMsgCallBack(N_ACK_BATTLE_ADD_PLAYER, this, &MatchModule::OnAckBattleAddPlayer);

	
	InitPath();
}

void MatchModule::InitPath()
{
	m_p_matchState = m_transModule->GetFromSelfPath(3, SERVER_TYPE::LOOP_MATCH_STATE);
	m_p_matchState[1]->type = SERVER_TYPE::LOOP_PROXY;
	m_p_matchState[1]->serid = m_proxyNode->GetProxyId();
	
	
}

void MatchModule::OnPlayerBeginBattle(NetMsg * msg)
{
	auto player = m_playerModule->CheckRoomPlayer(msg->socket);
	if (!player)
		return;

	if (player->m_matchInfo->m_state == PlayerState::PS_IN_MATCH)
		return;

	player->m_matchInfo->m_state = PlayerState::PS_IN_MATCH;
	player->m_matchInfo->m_key = 0;

	LPMsg::ReqGetMatchServer reqmsg;
	reqmsg.set_playerid(player->m_player->id);
	reqmsg.set_num(1);
	m_transModule->SendToServer(m_p_matchState, N_REQ_GET_MATCH_SERVER, reqmsg);

	//send player
	LPMsg::AckMatchState ackmsg;
	ackmsg.set_state(1);
	
	m_netObjModule->SendNetMsg(player->sock, LPMsg::SM_MATCH_STATE, ackmsg);
}

void MatchModule::OnReqStopMatch(NetMsg * msg)
{
	auto player = m_playerModule->CheckRoomPlayer(msg->socket);
	if (!player)
		return;

	player->m_matchInfo->m_state = PlayerState::PS_NONE;
}

void MatchModule::OnAckGetMatchServer(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::PlayerNode, msg);

	auto player = m_playerModule->GetRoomPlayer(pbMsg.playerid());
	if (!player)
		return;

	if (player->m_matchInfo->m_state != PlayerState::PS_IN_MATCH)
		return;

	LPMsg::ReqMatch reqmsg;
	reqmsg.add_players(player->m_player->id);

	m_proxyNode->SendToNode(pbMsg.proxyid(), SERVER_TYPE::LOOP_MATCH, pbMsg.serid(), N_REQ_MATCH_BATTLE, reqmsg);
}

void MatchModule::OnAckMatchBattle(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::AckMatchRes, msg);
	auto player = m_playerModule->GetRoomPlayer(pbMsg.playerid());
	if (!player)
		return;
	if (player->m_matchInfo->m_state != PlayerState::PS_IN_MATCH)
	{
		//notice stop match
		return;
	}

	auto match = player->m_matchInfo;

	match->m_state = PlayerState::PS_IN_BATTLE;
	match->m_proxyId = pbMsg.battlenode().proxyid();
	match->m_matchSerId = pbMsg.battlenode().serid();
	match->m_sceneId = pbMsg.sceneid();
	match->m_battleIp = pbMsg.battleip();
	match->m_battlePort = pbMsg.battleport();

	LPMsg::BatPlayerInfo reqmsg;
	auto pnode = reqmsg.mutable_pnode();
	pnode->set_playerid(player->m_player->id);
	pnode->set_proxyid(m_proxyNode->GetProxyId());
	pnode->set_serid(GetLayer()->GetServer()->serid);

	reqmsg.set_name(player->m_player->Get_name());
	reqmsg.set_sceneid(match->m_sceneId);

	m_proxyNode->SendToNode(match->m_proxyId, SERVER_TYPE::LOOP_BATTLE_TRANS, match->m_matchSerId, N_REQ_BATTLE_ADD_PLAYER, reqmsg);
}

void MatchModule::OnAckBattleAddPlayer(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::BatPlayerRes, msg);
	auto player = m_playerModule->GetRoomPlayer(pbMsg.playerid());
	if (!player)
		return;

	if (pbMsg.key() == 0)
	{
		//add player fail
		player->m_matchInfo->m_state = PlayerState::PS_NONE;
		return;
	}

	auto match = player->m_matchInfo;
	LPMsg::AckEnterBattle ackmsg;
	match->m_key = pbMsg.key();

	ackmsg.set_ip(match->m_battleIp);
	ackmsg.set_port(match->m_battlePort);
	ackmsg.set_sceneid(match->m_sceneId);
	ackmsg.set_key(pbMsg.key());
	m_netObjModule->SendNetMsg(player->sock, LPMsg::SM_ENTER_BATTLE, ackmsg);
}
