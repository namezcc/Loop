#include "MatchStateModule.h"
#include "MsgModule.h"
#include "EventModule.h"
#include "TransMsgModule.h"

#include "ServerMsgDefine.h"

#include "protoPB/server/common.pb.h"
#include "protoPB/server/sroom.pb.h"

MatchStateModule::MatchStateModule(BaseLayer * l):BaseModule(l),m_curMatchTable(&m_matchTable1)
{
}

MatchStateModule::~MatchStateModule()
{
}

void MatchStateModule::Init()
{
	m_msgModule = GET_MODULE(MsgModule);
	m_eventModule = GET_MODULE(EventModule);
	m_transModule = GET_MODULE(TransMsgModule);


	m_eventModule->AddEventCall(E_SERVER_CONNECT_AFTER,BIND_EVENT(OnServerConnect,SHARE<NetServer>));
	m_eventModule->AddEventCall(E_SERVER_CLOSE,BIND_EVENT(OnServerClose, SHARE<NetServer>));

	m_msgModule->AddMsgCallBack(N_ACK_NOTICE_SERVER, this, &MatchStateModule::OnAckNoticeServer);
	m_msgModule->AddMsgCallBack(N_REQ_GET_MATCH_SERVER, this, &MatchStateModule::OnReqGetMatchServer);



	m_noticePath = m_transModule->GetFromSelfPath(2,SERVER_TYPE::LOOP_PROXY);
	m_nodePath = m_transModule->GetFromSelfPath(3, SERVER_TYPE::LOOP_PROXY);
	m_nodePath[1]->type = SERVER_TYPE::LOOP_PROXY;
}

void MatchStateModule::OnServerConnect(SHARE<NetServer>& ser)
{
	if (ser->type == SERVER_TYPE::LOOP_PROXY)
	{
		LPMsg::ReqNoticeServer msg1;
		msg1.set_servertype(SERVER_TYPE::LOOP_MATCH);

		LPMsg::ReqNoticeServer msg2;
		msg2.set_servertype(SERVER_TYPE::LOOP_BATTLE_TRANS);
		
		auto pn1 = msg1.add_path();
		auto pn2 = msg1.add_path();

		pn1->set_serid(ser->serid);
		pn1->set_sertype(ser->type);

		pn2->set_sertype(SERVER_TYPE::LOOP_MATCH_STATE);
		pn2->set_serid(-1);

		auto pn3 = msg2.add_path();
		auto pn4 = msg2.add_path();
		pn3->CopyFrom(*pn1);
		pn4->CopyFrom(*pn2);

		m_noticePath[1]->serid = ser->serid;

		m_transModule->SendToServer(m_noticePath, N_REQ_NOTICE_SERVER, msg1);
		m_transModule->SendToServer(m_noticePath, N_REQ_NOTICE_SERVER, msg2);
	}
}

void MatchStateModule::OnServerClose(SHARE<NetServer>& ser)
{
	if (ser->type == SERVER_TYPE::LOOP_PROXY)
	{
		//remove all in this proxy id

		for (auto it=m_matchTable.begin();it!= m_matchTable.end();)
		{
			if (it->second->m_match.proxyId == ser->serid)
			{
				if (it->second->m_state == MATCH_ABLE)
				{
					m_matchTable1.erase(it->first);
					m_matchTable2.erase(it->first);
				}
				BattleResetMatch(it->second);
				m_matchTable.erase(it++);
			}
			else
				++it;
		}

		for (auto it = m_battleTable.begin();it!=m_battleTable.end();)
		{
			if (it->second->m_battle.proxyId == ser->serid)
			{
				if (it->second->m_matchId > 0)
					MatchRemoveBattle(it->second->m_matchId, it->first);
				m_battleTable.erase(it++);
			}
			else
				++it;
		}
	}
}

void MatchStateModule::OnAckNoticeServer(NetServerMsg * msg)
{
	TRY_PARSEPB(LPMsg::AckNoticeServer, msg);

	if (pbMsg.state() == CONN_STATE::CLOSE)
	{
		for (auto sid:pbMsg.serid())
		{
			if (pbMsg.sertype() == SERVER_TYPE::LOOP_MATCH)
				RemoveMatch(sid);
			else if(pbMsg.sertype() == SERVER_TYPE::LOOP_BATTLE_TRANS)
				RemoveBattle(sid);
		}
	}
	else if(pbMsg.state() == CONN_STATE::CONNECT)
	{
		for (auto sid : pbMsg.serid())
		{
			if (pbMsg.sertype() == SERVER_TYPE::LOOP_MATCH)
				AddMatchServer(sid, msg->path.front()->serid);
			else if (pbMsg.sertype() == SERVER_TYPE::LOOP_BATTLE_TRANS)
				AddBattleServer(sid, msg->path.front()->serid);
		}
	}
}

void MatchStateModule::OnReqGetMatchServer(NetServerMsg * msg)
{
	TRY_PARSEPB(LPMsg::ReqGetMatchServer, msg);

	LPMsg::PlayerNode ackmsg;
	ackmsg.set_playerid(pbMsg.playerid());

	auto it = m_curMatchTable->begin();
	if (it == m_curMatchTable->end())
	{
		m_curMatchTable = GetDiffMatchTable();
		it = m_curMatchTable->begin();
		if (it == m_curMatchTable->end())
		{//no match server in user
			LP_INFO << "no match server in user";
			ackmsg.set_proxyid(0);
			m_transModule->SendBackServer(msg->path, N_ACK_GET_MATCH_SERVER, ackmsg);
			return;
		}
	}

	ackmsg.set_proxyid(it->second->m_match.proxyId);
	ackmsg.set_serid(it->second->m_match.serid);
	m_transModule->SendBackServer(msg->path, N_ACK_GET_MATCH_SERVER, ackmsg);

	it->second->m_useNum += pbMsg.num();

	if (it->second->m_useNum >= ONE_SCENE_NUM)
	{
		auto match = it->second;
		m_curMatchTable->erase(it);

		auto diff = GetDiffMatchTable();
		(*diff)[match->m_match.serid] = match;
	}
}

void MatchStateModule::AddMatchServer(const int16_t & serid, const int16_t& proxyid)
{
	auto it = m_matchTable.find(serid);
	if (it != m_matchTable.end())
		return;

	auto match = GET_SHARE(MatchNode);
	match->m_battle.clear();
	match->m_match.proxyId = proxyid;
	match->m_match.serid = serid;
	match->m_state = MATCH_NONE;
	match->m_useNum = 0;
	
	while (true)
	{
		auto battle = GetFreeBattle();
		if (battle)
		{
			match->m_battle.push_back(battle->m_battle.serid);
			battle->m_matchId = serid;
			battle->m_matchProxyId = proxyid;
			//notice battle server
			SendOnLine(LOOP_BATTLE_TRANS, battle->m_battle, serid,proxyid);
		}
		else
			break;

		if (match->m_battle.size() >= MATCH_BATTLE_NUM)
			break;
	}

	if (match->m_battle.size() > 0)
		AddMatchToAble(match);
	m_matchTable[serid] = match;
}

void MatchStateModule::AddBattleServer(const int16_t & serid, const int16_t& proxyid)
{
	auto it = m_battleTable.find(serid);
	if (it != m_battleTable.end())
		return;

	auto battle = GET_SHARE(BattleNode);
	battle->m_matchId = 0;
	battle->m_battle.proxyId = proxyid;
	battle->m_battle.serid = serid;

	auto match = GetFreeMatch();
	if (match)
	{
		match->m_battle.push_back(serid);
		if (match->m_state != MATCH_ABLE)
			AddMatchToAble(match);
		battle->m_matchId = match->m_match.serid;
		battle->m_matchProxyId = match->m_match.proxyId;
		//notice battle server
		SendOnLine(LOOP_BATTLE_TRANS, battle->m_battle, battle->m_matchId, battle->m_matchProxyId);
	}
	m_battleTable[serid] = battle;
}

SHARE<MatchNode> MatchStateModule::GetFreeMatch()
{
	for (auto& n:m_matchTable)
	{
		if (n.second->m_battle.size() < MATCH_BATTLE_NUM)
			return n.second;
	}
	return NULL;
}

SHARE<BattleNode> MatchStateModule::GetFreeBattle()
{
	for (auto& n:m_battleTable)
	{
		if (n.second->m_matchId == 0)
			return n.second;
	}
	return NULL;
}

void MatchStateModule::RemoveMatch(const int16_t & serid)
{
	auto it = m_matchTable.find(serid);
	if (it == m_matchTable.end())
		return;

	if (it->second->m_state == MATCH_ABLE)
	{
		m_matchTable1.erase(serid);
		m_matchTable2.erase(serid);
	}

	BattleResetMatch(it->second);
	m_matchTable.erase(it);
}

void MatchStateModule::BattleResetMatch(SHARE<MatchNode>& match)
{
	for (auto bid:match->m_battle)
	{
		auto itb = m_battleTable.find(bid);
		if (itb != m_battleTable.end())
		{
			//notice battle server
			SendOffLine(LOOP_BATTLE_TRANS, itb->second->m_battle, itb->second->m_matchId, itb->second->m_matchProxyId);
			itb->second->m_matchId = 0;
		}
	}
}

void MatchStateModule::RemoveBattle(const int16_t & battleid)
{
	auto it = m_battleTable.find(battleid);
	if (it == m_battleTable.end())
		return;
	if (it->second->m_matchId > 0)
	{
		MatchRemoveBattle(it->second->m_matchId, battleid);
	}
	m_battleTable.erase(battleid);
}

void MatchStateModule::MatchRemoveBattle(const int16_t & matchid, const int16_t & battleid)
{
	auto it = m_matchTable.find(matchid);
	if (it == m_matchTable.end())
		return;

	for (auto itb=it->second->m_battle.begin(); itb!= it->second->m_battle.end();++itb)
	{
		if (*itb == battleid)
		{
			it->second->m_battle.erase(itb);
			if (it->second->m_state == MATCH_BUSY)
				AddMatchToAble(it->second);
			//notice match server
			SendOffLine(LOOP_MATCH, it->second->m_match, battleid,0);
			return;
		}
	}
}

void MatchStateModule::AddMatchToAble(SHARE<MatchNode>& match)
{
	match->m_state = MATCH_ABLE;
	match->m_useNum = 0;

	auto tab = GetDiffMatchTable();
	(*tab)[match->m_match.serid] = match;
}

void MatchStateModule::SendOffLine(const int16_t & tostype, ProxyNode & pnode, const int16_t & nserid, const int16_t& proxyId)
{
	// just one this server send
	m_nodePath[1]->serid = pnode.proxyId;
	m_nodePath[2]->type = tostype;
	m_nodePath[2]->serid = pnode.serid;

	LPMsg::ProxyNode msg;
	msg.set_proxyid(proxyId);
	msg.set_serid(nserid);
	m_transModule->SendToServer(m_nodePath, N_ACK_SERVER_OFFLINE, msg);
}

void MatchStateModule::SendOnLine(const int16_t & tostype, ProxyNode & pnode, const int16_t & nserid, const int16_t& proxyId)
{
	// just one this server send
	m_nodePath[1]->serid = pnode.proxyId;
	m_nodePath[2]->type = tostype;
	m_nodePath[2]->serid = pnode.serid;

	LPMsg::ProxyNode msg;
	msg.set_proxyid(proxyId);
	msg.set_serid(nserid);
	m_transModule->SendToServer(m_nodePath, N_ACK_SERVER_ONLINE, msg);
}
