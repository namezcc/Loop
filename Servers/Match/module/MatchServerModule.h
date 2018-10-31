#ifndef MATCH_SERVER_MODULE_H
#define MATCH_SERVER_MODULE_H

#include "BaseModule.h"
#include "CommonDefine.h"

class MsgModule;
class TransMsgModule;
class ProxyNodeModule;

struct SceneInfo
{
	int32_t	m_id;
	int32_t m_num;
};

struct BattleInfo
{
	std::string m_ip;
	int32_t m_port;
	ProxyNode m_battle;
	std::list<SHARE<SceneInfo>> m_scene;
};

class MatchServerModule:public BaseModule
{
public:
	MatchServerModule(BaseLayer* l);
	~MatchServerModule();

private:
	virtual void Init() override;

	void OnAckBattleFreeScene(NetMsg* msg);
	void OnAckServerOffLine(NetMsg* msg);

	void OnReqMatchBattle(NetServerMsg* msg);


	SHARE<SceneInfo> GetUseBattleScene();
private:
	int32_t m_freeSceneNum;
	SHARE<BattleInfo> m_useBattle;
	SHARE<SceneInfo> m_useScene;

	std::unordered_map<int32_t, SHARE<BattleInfo>> m_battle;

	MsgModule* m_msgModule;
	TransMsgModule* m_transModule;
	ProxyNodeModule* m_proxyNode;
};

#endif
