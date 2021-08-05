#ifndef MATCH_STATE_MODULE_H
#define MATCH_STATE_MODULE_H

#include "BaseModule.h"
#include "CommonDefine.h"

class MsgModule;
class EventModule;
class TransMsgModule;

struct BattleNode
{
	ProxyNode m_battle;
	int16_t m_matchId;
	int16_t m_matchProxyId;
};

enum MATCH_STATE
{
	MATCH_NONE,
	MATCH_ABLE,
	MATCH_BUSY,
};

struct MatchNode
{
	int8_t m_state;
	int16_t	m_useNum;
	ProxyNode m_match;
	std::vector<int16_t> m_battle;
};

class MatchStateModule:public BaseModule
{
public:
	MatchStateModule(BaseLayer* l);
	~MatchStateModule();

private:
	virtual void Init() override;

	void OnServerConnect(SHARE<NetServer>& ser);
	void OnServerClose(SHARE<NetServer>& ser);

	void OnAckNoticeServer(NetServerMsg* msg);

	void OnReqGetMatchServer(NetServerMsg* msg);

	std::map<int32_t, SHARE<MatchNode>>* GetDiffMatchTable() { return m_curMatchTable == &m_matchTable1 ? &m_matchTable2 : &m_matchTable1; }


	void AddMatchServer(const int16_t& serid,const int16_t& proxyid);
	void AddBattleServer(const int16_t& serid, const int16_t& proxyid);

	SHARE<MatchNode> GetFreeMatch();
	SHARE<BattleNode> GetFreeBattle();

	void RemoveMatch(const int16_t& serid);
	void BattleResetMatch(SHARE<MatchNode>& match);
	void RemoveBattle(const int16_t& battleid);
	void MatchRemoveBattle(const int16_t& matchid, const int16_t& battleid);
	void AddMatchToAble(SHARE<MatchNode>& match);

	void SendOffLine(const int8_t& tostype, ProxyNode& pnode, const int16_t& nserid, const int16_t& proxyId);
	void SendOnLine(const int8_t& tostype, ProxyNode& pnode, const int16_t& nserid, const int16_t& proxyId);
private:

	MsgModule* m_msgModule;
	EventModule* m_eventModule;
	TransMsgModule* m_transModule;

	ServerPath m_noticePath;
	ServerPath m_nodePath;


	std::map<int32_t, SHARE<MatchNode>>* m_curMatchTable;

	std::map<int32_t, SHARE<MatchNode>> m_matchTable1;
	std::map<int32_t, SHARE<MatchNode>> m_matchTable2;

	std::unordered_map<int32_t, SHARE<MatchNode>> m_matchTable;
	std::unordered_map<int32_t, SHARE<BattleNode>> m_battleTable;
};

#endif
