#include "MatchServerModule.h"
#include "MsgModule.h"
#include "TransMsgModule.h"
#include "module/ProxyNodeModule.h"

#include "ServerMsgDefine.h"

#include "protoPB/server/sroom.pb.h"
#include "protoPB/server/common.pb.h"

MatchServerModule::MatchServerModule(BaseLayer * l):BaseModule(l), m_freeSceneNum(0)
{
}

MatchServerModule::~MatchServerModule()
{
}

void MatchServerModule::Init()
{
	m_msgModule = GET_MODULE(MsgModule);
	m_transModule = GET_MODULE(TransMsgModule);
	m_proxyNode = GET_MODULE(ProxyNodeModule);

	m_msgModule->AddMsgCallBack(N_ACK_BATTLE_FREE_SCENE, this, &MatchServerModule::OnAckBattleFreeScene);
	m_msgModule->AddMsgCallBack(N_ACK_SERVER_OFFLINE, this, &MatchServerModule::OnAckServerOffLine);
	m_msgModule->AddMsgCallBack(N_REQ_MATCH_BATTLE, this, &MatchServerModule::OnReqMatchBattle);


}

void MatchServerModule::OnAckBattleFreeScene(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::AckFreeScene, msg);

	auto it = m_battle.find(pbMsg.battle().serid());
	SHARE<BattleInfo> bat = NULL;
	if (it == m_battle.end())
	{
		bat = GET_SHARE(BattleInfo);
		bat->m_scene.clear();
		bat->m_battle.proxyId = pbMsg.battle().proxyid();
		bat->m_battle.serid = pbMsg.battle().serid();
		bat->m_ip = pbMsg.ip();
		bat->m_port = pbMsg.port();
		m_battle[bat->m_battle.serid] = bat;
	}
	else
		bat = it->second;

	for (auto& id:pbMsg.scene())
	{
		auto sinfo = GET_SHARE(SceneInfo);
		sinfo->m_id = id;
		sinfo->m_num = 0;
		bat->m_scene.push_back(sinfo);
	}
	m_freeSceneNum += pbMsg.scene_size();
}

void MatchServerModule::OnAckServerOffLine(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::ProxyNode, msg);

	auto it = m_battle.find(pbMsg.serid());
	if (it == m_battle.end())
		return;

	m_freeSceneNum -= it->second->m_scene.size();
	it->second->m_scene.clear();
	if (it->second->m_battle.serid == m_useBattle->m_battle.serid)
	{
		m_useBattle = NULL;
		m_useScene = NULL;
	}
	m_battle.erase(it);
}

void MatchServerModule::OnReqMatchBattle(NetServerMsg * msg)
{
	TRY_PARSEPB(LPMsg::ReqMatch, msg);
	if (pbMsg.players_size() == 0)
		return;

	LPMsg::AckMatchRes ackmsg;
	ackmsg.set_playerid(*pbMsg.players().begin());

	auto scene = GetUseBattleScene();
	if (scene)
	{
		auto node = ackmsg.mutable_battlenode();
		node->set_proxyid(m_useBattle->m_battle.proxyId);
		node->set_serid(m_useBattle->m_battle.serid);

		ackmsg.set_battleip(m_useBattle->m_ip);
		ackmsg.set_battleport(m_useBattle->m_port);
		ackmsg.set_sceneid(scene->m_id);
		if (scene->m_num == 0)
		{//notice battle start a scene
			//m_battlePath[3]->serid = m_useBattle->m_battle.proxyId;
			//m_battlePath[4]->serid = m_useBattle->m_battle.serid;

			LPMsg::Int32Value reqmsg;
			reqmsg.set_value(scene->m_id);

			//m_transModule->SendToServer(m_battlePath, N_REQ_BEGIN_BATTLE_SCENE, reqmsg);
			m_proxyNode->SendToNode(m_useBattle->m_battle.proxyId, SERVER_TYPE::LOOP_BATTLE_TRANS, m_useBattle->m_battle.serid, N_REQ_BEGIN_BATTLE_SCENE, reqmsg);
		}

		scene->m_num += pbMsg.players_size();
	}
	else
	{
		ackmsg.set_sceneid(0);
	}
	m_transModule->SendBackServer(msg->path, N_ACK_MATCH_BATTLE, ackmsg);
}

SHARE<SceneInfo> MatchServerModule::GetUseBattleScene()
{
	if (m_useScene && m_useScene->m_num < ONE_SCENE_NUM)
		return m_useScene;

	if (m_battle.size() == 0)
		return NULL;

	for (auto& b:m_battle)
	{
		if (b.second->m_scene.size() > 0)
		{
			m_useScene = b.second->m_scene.front();
			b.second->m_scene.pop_front();
			m_useBattle = b.second;
			--m_freeSceneNum;
			return m_useScene;
		}
	}
	return NULL;
}
