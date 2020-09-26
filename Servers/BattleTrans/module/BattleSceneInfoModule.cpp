#include "BattleSceneInfoModule.h"
#include "MsgModule.h"
#include "EventModule.h"
#include "TransMsgModule.h"
#include "module/ProxyNodeModule.h"
#include "UdpNetSockModule.h"

#include "protoPB/server/common.pb.h"
#include "protoPB/server/sroom.pb.h"
#include "protoPB/client/battle.pb.h"
#include "protoPB/client/define.pb.h"

#include "ServerMsgDefine.h"

BattleSceneInfoModule::BattleSceneInfoModule(BaseLayer * l):BaseModule(l)
{
	m_match.proxyId = 0;
	m_sceneServer.type = SERVER_TYPE::LOOP_BATTLE_SCENE;
	m_sceneServer.serid = 0;
	m_lastMsIndex = 0;
	m_perFrameMs = 1000 / FRAME_NUM_PER_SECOND;
}

BattleSceneInfoModule::~BattleSceneInfoModule()
{
}

void BattleSceneInfoModule::Init()
{
	m_msgModule = GET_MODULE(MsgModule);
	m_eventModule = GET_MODULE(EventModule);
	m_proxyNode = GET_MODULE(ProxyNodeModule);
	m_transModule = GET_MODULE(TransMsgModule);
	m_udpSockModule = GET_MODULE(UdpNetSockModule);

	m_eventModule->AddEventCall(E_SERVER_CONNECT, BIND_EVENT(OnServerConnect,SHARE<NetServer>));
	m_eventModule->AddEventCall(E_UDP_SOCKET_CLOSE,BIND_EVENT(OnClientClose,int32_t));

	m_msgModule->AddMsgCallBack(N_ACK_BATTLE_FREE_SCENE, this, &BattleSceneInfoModule::OnAckBattleFreeScene);
	m_msgModule->AddMsgCallBack(N_ACK_SERVER_ONLINE, this, &BattleSceneInfoModule::OnAckMatchOnLine);
	m_msgModule->AddMsgCallBack(N_ACK_SERVER_OFFLINE, this, &BattleSceneInfoModule::OnAckMatchOffLine);
	m_msgModule->AddMsgCallBack(N_REQ_BEGIN_BATTLE_SCENE, this, &BattleSceneInfoModule::OnReqBeginBattleScene);
	m_msgModule->AddMsgCallBack(N_REQ_BATTLE_ADD_PLAYER, this, &BattleSceneInfoModule::OnReqAddPlayer);
	m_msgModule->AddMsgCallBack(N_ACK_OBJECT_INFO_ENTER_VIEW, this, &BattleSceneInfoModule::OnAckObjectInfoEnterView);
	
	m_msgModule->AddMsgCallBack(N_ACK_SELF_ROLE_INFO, this, &BattleSceneInfoModule::OnAckSelfRoleInfo);
	m_msgModule->AddMsgCallBack(LPMsg::CM_ENTER_BATTLE_SCENE, this, &BattleSceneInfoModule::OnReqEnterBattleScene);
	m_msgModule->AddMsgCallBack(LPMsg::CM_PLAYER_OPERATION, this, &BattleSceneInfoModule::OnReqPlayerOperation);
	m_msgModule->AddMsgCallBack(LPMsg::CM_FIX_FRAME, this, &BattleSceneInfoModule::OnReqFixFrame);


	auto config = GetLayer()->GetLoopServer()->GetConfig();
	m_udpip = config.udpAddr.ip;
	m_udpPort = config.udpAddr.port;
	m_selfSerid = GetLayer()->GetServer()->serid;
}

void BattleSceneInfoModule::Execute()
{
	auto nowms = GetMilliSecend();
	int32_t index = nowms % 1000;
	RunBattleSceneFrame(index);
}

void BattleSceneInfoModule::OnServerConnect(SHARE<NetServer>& ser)
{
	if (ser->type == SERVER_TYPE::LOOP_BATTLE_SCENE)
	{
		m_sceneServer.serid = ser->serid;
	}
}

void BattleSceneInfoModule::OnClientClose(const int32_t & sock)
{
	auto it = m_sockPlayer.find(sock);
	if (it == m_sockPlayer.end())
		return;

	LP_INFO << "Player udp close " << sock;
	it->second->m_sock = -1;
	m_sockPlayer.erase(sock);
}

void BattleSceneInfoModule::OnAckBattleFreeScene(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::AckFreeScene, msg);

	for (auto& scid:pbMsg.scene())
	{
		auto scene = GET_SHARE(ForwardScene);
		scene->SetSceneId(scid);
		m_freeScene[scid] = scene;
	}

	if (m_match.proxyId > 0)
		SendFreeSceneToMatch();
}

void BattleSceneInfoModule::OnAckMatchOnLine(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::ProxyNode, msg);
	for (auto& sc:m_freeScene)
	{
		sc.second->m_sendMatch = false;
	}

	m_match.proxyId = pbMsg.proxyid();
	m_match.serid = pbMsg.serid();

	if (m_freeScene.size() > 0)
		SendFreeSceneToMatch();
}

void BattleSceneInfoModule::OnAckMatchOffLine(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::ProxyNode, msg);
	for (auto& sc : m_freeScene)
	{
		sc.second->m_sendMatch = false;
	}
	m_match.proxyId = 0;
	m_match.serid = 0;
}

void BattleSceneInfoModule::OnReqBeginBattleScene(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::Int32Value, msg);

	auto it = m_freeScene.find(pbMsg.value());
	if (it == m_freeScene.end())
		return;

	m_runingScene[it->second->GetSceneId()] = it->second;
	AddSceneToRunFrame(it->second);
	m_freeScene.erase(it);

	m_transModule->SendToServer(m_sceneServer, N_REQ_BEGIN_BATTLE_SCENE, pbMsg);
}

void BattleSceneInfoModule::OnReqAddPlayer(NetServerMsg * msg)
{
	TRY_PARSEPB(LPMsg::BatPlayerInfo, msg);
	auto pnode = pbMsg.pnode();

	int64_t key = 0;
	ExitCall call([this,&msg,&pnode,&key]() {
		LPMsg::BatPlayerRes ackmsg;
		ackmsg.set_playerid(pnode.playerid());
		ackmsg.set_key(key);
		m_transModule->SendBackServer(msg->path, N_ACK_BATTLE_ADD_PLAYER, ackmsg);
	});

	auto it = m_runingScene.find(pbMsg.sceneid());
	if (it == m_runingScene.end() || !it->second->AbleAddPlayer())
		return;
	//????有问题
	int32_t hval = std::hash<int64_t>()(pnode.playerid());
	key |= it->second->GetSceneId();
	key <<= 32;
	key |= hval;

	auto itp = m_runingPlayer.find(key);
	if (itp != m_runingPlayer.end())
		return;

	auto forply = GET_SHARE(ForwardPlayer);
	forply->m_playerId = pnode.playerid();
	forply->m_proxyId = pnode.proxyid();
	forply->m_roomSerId = pnode.serid();	
	it->second->AddPlayer(forply);

	m_runingPlayer[key] = forply;

	m_transModule->SendToServer(m_sceneServer, N_REQ_BATTLE_ADD_PLAYER, pbMsg);
}

void BattleSceneInfoModule::OnAckObjectInfoEnterView(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::AoiSceneEntityList, msg);
	std::vector<int32_t> broads;
	for (auto& slis:pbMsg.list())
	{
		auto it = m_runingScene.find(slis.sceneid());
		if (it == m_runingScene.end())
			continue;

		for (auto& ent : slis.list())
		{
			for (auto& idx : ent.views())
			{
				auto ply = it->second->GetPlayer(idx);
				if (ply && ply->OnLine())
					broads.push_back(ply->m_sock);
			}
			if(broads.size() > 0)
				m_udpSockModule->BroadNetMsg(broads, LPMsg::SM_OBJECT_INFO, ent.entity());
		}
	}
}

void BattleSceneInfoModule::OnAckSelfRoleInfo(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::EntityList, msg);

	auto it = m_runingScene.find(pbMsg.sceneid());
	if (it == m_runingScene.end())
		return;

	for (auto& pid:pbMsg.views())
	{
		auto ply = it->second->GetPlayer(pid);
		if (ply && ply->OnLine())
			m_udpSockModule->SendNetMsg(ply->m_sock, LPMsg::SM_SELF_ROLE_INFO, pbMsg);
	}
}

void BattleSceneInfoModule::OnReqEnterBattleScene(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::ReqEnterBattleScene, msg);

	auto it = m_runingPlayer.find(pbMsg.playerid());
	if (it == m_runingPlayer.end())
	{
		m_udpSockModule->CloseNetObject(msg->socket);
		return;
	}

	//remove old
	if (it->second->OnLine())
	{
		m_udpSockModule->CloseNetObject(it->second->m_sock);
		m_sockPlayer.erase(it->second->m_sock);
	}

	it->second->m_sock = msg->socket;
	//pbMsg.set_playerid(it->second->m_objectId);
	m_sockPlayer[msg->socket] = it->second;

	LPMsg::PlayerNode reqmsg;
	reqmsg.set_playerid(it->second->m_objectId);
	reqmsg.set_proxyid(it->second->m_sceneId);
	m_transModule->SendToServer(m_sceneServer, LPMsg::CM_ENTER_BATTLE_SCENE, reqmsg);
}

void BattleSceneInfoModule::OnReqPlayerOperation(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::CmdList, msg);
	auto it = m_sockPlayer.find(msg->socket);
	if (it == m_sockPlayer.end())
	{
		m_udpSockModule->CloseNetObject(msg->socket);
		return;
	}
	auto ply = it->second;
	if (ply->m_scene == NULL)
	{
		return;
	}
	ply->CollectOpt(pbMsg);
}

void BattleSceneInfoModule::OnReqFixFrame(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::FixFrame, msg);
	auto it = m_sockPlayer.find(msg->socket);
	if (it == m_sockPlayer.end())
	{
		m_udpSockModule->CloseNetObject(msg->socket);
		return;
	}
	auto diff = pbMsg.frame() - it->second->m_scene->GetFrame();
	pbMsg.set_frame(diff);
	m_udpSockModule->SendNetMsg(msg->socket, LPMsg::SM_FIX_FRAME, pbMsg);
}

void BattleSceneInfoModule::SendFreeSceneToMatch()
{
	LPMsg::AckFreeScene ackmsg;
	ackmsg.set_ip(m_udpip);
	ackmsg.set_port(m_udpPort);
	auto selfnode = ackmsg.mutable_battle();
	selfnode->set_proxyid(m_proxyNode->GetProxyId());
	selfnode->set_serid(m_selfSerid);

	for (auto& sc:m_freeScene)
	{
		if (!sc.second->m_sendMatch)
		{
			ackmsg.add_scene(sc.first);
			sc.second->m_sendMatch = true;
		}
	}
	m_proxyNode->SendToNode(m_match.proxyId, SERVER_TYPE::LOOP_MATCH, m_match.serid, N_ACK_BATTLE_FREE_SCENE, ackmsg);
}

void BattleSceneInfoModule::AddSceneToRunFrame(const SHARE<ForwardScene>& scene)
{
	auto index = (m_lastMsIndex + m_perFrameMs) % 1000;
	m_runFrameScene[index].push_back(scene);
}

void BattleSceneInfoModule::RunBattleSceneFrame(const int32_t & msidx)
{
	LPMsg::SceneCmdList scenemsg;

	while (m_lastMsIndex != msidx)
	{
		++m_lastMsIndex;
		if (m_lastMsIndex >= 1000)
			m_lastMsIndex = 0;

		if (m_runFrameScene[m_lastMsIndex].size() > 0)
		{
			auto list = std::move(m_runFrameScene[m_lastMsIndex]);
			for (auto& scene: list)
			{
				scene->AddFrame();
				auto scenecmd = scenemsg.add_scenecmd();
				scenecmd->set_sceneid(scene->GetSceneId());
				if (scene->GetPlayerNum() > 0)
					ForwardPlayerCmd(scene, *scenecmd);
				AddSceneToRunFrame(scene);
			}
		}
	}
	if (scenemsg.scenecmd_size() > 0)
		m_transModule->SendToServer(m_sceneServer, LPMsg::SM_PLAYER_OPERATION, scenemsg);
}

void BattleSceneInfoModule::ForwardPlayerCmd(const SHARE<ForwardScene>& scene, LPMsg::SceneCmd& scecmd)
{
	for (int32_t i = 0; i < scene->GetPlayerNum(); i++)
	{
		auto ply = scene->GetPlayer(i);
		if (ply)
		{
			if (ply->m_cmdList.list_size() > 0)
			{
				auto cmd = scecmd.add_list();
				cmd->CopyFrom(ply->m_cmdList);
				ply->m_cmdList.clear_list();
			}
		}
	}
}
