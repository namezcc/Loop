#ifndef GATE_MODULE_H
#define GATE_MODULE_H

#include "BaseModule.h"

struct ReadyInfo
{
	int32_t uid;
	int32_t roomsid;
	int32_t key;
	int32_t sock;
};

#define SOCK_ROLE_PLAYER 1

class EventModule;
COM_MOD_CLASS;

class GateModule:public BaseModule
{
public:
	GateModule(BaseLayer* l);
	~GateModule();

private:

	// Í¨¹ý BaseModule ¼Ì³Ð
	virtual void Init() override;


	void onServerConnect(SHARE<NetServer>& ser);
	void onServerClose(SHARE<NetServer>& ser);
	void onClientClose(const int32_t& sock);

	void onNetMsg(NetMsg* msg);
	void onPlayerReadyInfo(NetMsg* msg);
	void onPlayerLogin(NetMsg* msg);

	void sendRoomMgrPlayerNum();


	EventModule* m_event_mod;
	COM_MOD_OBJ;

	std::vector<int32_t> m_broad_vec;
	std::unordered_map<int32_t, ReadyInfo> m_readyInfo;
	ReadyInfo* m_player_sock[MAX_CLIENT_CONN];
	int32_t m_player_num;

};

#endif GATE_MODULE_H
