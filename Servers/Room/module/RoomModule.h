﻿#ifndef ROOM_MODULE_H
#define ROOM_MODULE_H

#include "BaseModule.h"

class MsgModule;
class NetObjectModule;
class TransMsgModule;
class EventModule;

typedef std::pair<int32_t, int32_t> int32Pair;

class RoomModuloe:public BaseModule
{
public:
	RoomModuloe(BaseLayer* l);
	~RoomModuloe();

private:
	virtual void Init() override;

	void onServerConnect(SHARE<NetServer>& ser);
	ServerPath& getDbPath(int64_t uid);

public:

	void doSqlOperation(int64_t uid, int32_t opt, google::protobuf::Message& pb, int32_t ackId = 0);
	void updatePlayerData(int64_t uid, int32_t rid, google::protobuf::Message& pb, int32_t table, int32_t key1 = 0, int32_t key2 = 0);
	void deletePlayerData(int64_t uid, int32_t rid, int32_t table, int32_t key1 = 0, int32_t key2 = 0);
	void updatePlayerData(int64_t uid, int32_t rid, int32_t table,std::vector<int32Pair>& keys,std::vector<gpb::Message>& pb);
	void deletePlayerData(int64_t uid, int32_t rid, int32_t table, std::vector<int32Pair>& keys);


private:

	MsgModule * m_msg_mod;
	TransMsgModule* m_trans_mod;
	NetObjectModule* m_net_mod;
	EventModule* m_event_mod;

	ServerPath m_db_path;
};


#endif