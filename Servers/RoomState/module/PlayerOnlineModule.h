#ifndef PLAYER_ONLINE_MOD_H
#define PLAYER_ONLINE_MOD_H

#include "BaseModule.h"

class MsgModule;
class TransMsgModule;
class NetObjectModule;

class PlayerOnlineModule:public BaseModule
{
public:
	PlayerOnlineModule(BaseLayer* l):BaseModule(l)
	{}
	~PlayerOnlineModule()
	{}

	void onPlayerOnline(NetMsg* msg);
	void onCheckPlayerOnline(NetMsg* msg);
	void onPlayerTransMsg(NetMsg* msg);

	void onPlayerOffline(int64_t pid);

protected:
	// Í¨¹ý BaseModule ¼Ì³Ð
	virtual void Init() override;

	MsgModule* m_msg_mod;
	TransMsgModule* m_trans_mod;
	NetObjectModule* m_net_mod;


	std::map<int64_t, int32_t> m_online_info;

};

#endif // !PLAYER_ONLINE_MOD_H

