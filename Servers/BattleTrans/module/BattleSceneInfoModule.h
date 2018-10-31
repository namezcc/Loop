#ifndef BATTLE_SCENE_INFO_MODULE_H
#define BATTLE_SCENE_INFO_MODULE_H

#include "BaseModule.h"

#include "ForwardScene.h"
#include "CommonDefine.h"

#define FRAME_NUM_PER_SECOND	30		//30

class MsgModule;
class EventModule;
class ProxyNodeModule;
class TransMsgModule;
class UdpNetSockModule;

class BattleSceneInfoModule:public BaseModule
{
public:
	BattleSceneInfoModule(BaseLayer* l);
	~BattleSceneInfoModule();

private:
	virtual void Init() override;
	virtual void Execute() override;

	void OnServerConnect(SHARE<NetServer>& ser);
	void OnClientClose(const int32_t& sock);

	void OnAckBattleFreeScene(NetMsg* msg);
	void OnAckMatchOnLine(NetMsg* msg);
	void OnAckMatchOffLine(NetMsg* msg);

	void OnReqBeginBattleScene(NetMsg* msg);
	void OnReqAddPlayer(NetServerMsg* msg);
	
	//from battle scene
	void OnAckObjectInfoEnterView(NetMsg* msg);
	void OnAckSelfRoleInfo(NetMsg* msg);

	//udp client msg
	void OnReqEnterBattleScene(NetMsg* msg);
	void OnReqPlayerOperation(NetMsg* msg);


	void SendFreeSceneToMatch();

	void AddSceneToRunFrame(const SHARE<ForwardScene>& scene);
	void RunBattleSceneFrame(const int32_t& msidx);
	void ForwardPlayerCmd(const SHARE<ForwardScene>& scene, LPMsg::SceneCmd& scecmd);

private:
	MsgModule* m_msgModule;
	EventModule* m_eventModule;
	ProxyNodeModule* m_proxyNode;
	TransMsgModule* m_transModule;
	UdpNetSockModule* m_udpSockModule;

	std::string m_udpip;
	int32_t m_udpPort;
	int32_t m_selfSerid;
	ProxyNode m_match;

	ServerNode m_sceneServer;

	std::unordered_map<int32_t, SHARE<ForwardScene>> m_freeScene;
	std::unordered_map<int32_t, SHARE<ForwardScene>> m_runingScene;

	std::unordered_map<int64_t, SHARE<ForwardPlayer>> m_runingPlayer;
	std::unordered_map<int32_t, SHARE<ForwardPlayer>> m_sockPlayer;


	int32_t m_lastMsIndex;
	int32_t m_perFrameMs;
	std::vector<SHARE<ForwardScene>>	m_runFrameScene[1000];	// 1000 ms
};

#endif
