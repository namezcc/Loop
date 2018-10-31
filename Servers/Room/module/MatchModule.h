#ifndef MATCH_MODULE_H
#define MATCH_MODULE_H

#include "BaseModule.h"

class MsgModule;
class ProxyNodeModule;
class TransMsgModule;
class PlayerModule;
class NetObjectModule;

class MatchModule:public BaseModule
{
public:
	MatchModule(BaseLayer* l);
	~MatchModule();

private:
	virtual void Init() override;

	void InitPath();

	void OnPlayerBeginBattle(NetMsg* msg);
	void OnReqStopMatch(NetMsg* msg);
	void OnAckGetMatchServer(NetMsg* msg);
	void OnAckMatchBattle(NetMsg* msg);
	void OnAckBattleAddPlayer(NetMsg* msg);

private:
	MsgModule * m_msgModule;
	ProxyNodeModule* m_proxyNode;
	TransMsgModule* m_transModule;
	PlayerModule* m_playerModule;
	NetObjectModule* m_netObjModule;

	VecPath m_p_matchState;
	
};
#endif
