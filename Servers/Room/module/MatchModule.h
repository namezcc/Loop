#ifndef MATCH_MODULE_H
#define MATCH_MODULE_H

#include "BaseModule.h"

class MsgModule;
class NetObjectModule;
class TransMsgModule;
class PlayerModule;

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
	TransMsgModule* m_transModule;
	NetObjectModule* m_netObjModule;
	PlayerModule* m_playerModule;

	ServerPath m_p_matchState;
	
};
#endif
