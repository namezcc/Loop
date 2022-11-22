#ifndef ROOM_MODULE_H
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

	void setRoomState(int32_t state);

private:
	virtual void Init() override;

	void onServerClose(SHARE<NetServer>& ser);
	void onRegDbidToSerid(NetMsg* msg);

	ServerPath& getDbPath(int64_t uid);
	int32_t getDbserid(int32_t cid);

public:

	void doSqlOperation(int32_t cid, int32_t opt, const google::protobuf::Message& pb, int32_t ackId = 0);
	void doSqlOperation(int32_t cid, int32_t opt, const char* buf,int32_t buflen, int32_t ackId = 0);
	void doSqlOperation(int32_t cid, int32_t opt, BuffBlock* buf, int32_t ackId = 0);
	void updatePlayerData(int64_t pid, const google::protobuf::Message& pb, int32_t table, std::string key1 = "0", std::string key2 = "0");
	void updatePlayerData(int64_t pid,const std::string& pb, int32_t table, std::string key1 = "0", std::string key2 = "0");

	void deletePlayerData(int64_t pid, const std::string& pb, int32_t table, std::string key1 = "0", std::string key2 = "0");
	void deletePlayerData(int64_t uid, int32_t rid, int32_t table, int32_t key1 = 0, int32_t key2 = 0);
	void updatePlayerData(int64_t uid, int32_t rid, int32_t table,std::vector<int32Pair>& keys,std::vector<gpb::Message>& pb);
	void deletePlayerData(int64_t uid, int32_t rid, int32_t table, std::vector<int32Pair>& keys);


private:

	MsgModule * m_msg_mod;
	TransMsgModule* m_trans_mod;
	NetObjectModule* m_net_mod;
	EventModule* m_event_mod;

	ServerPath m_db_path;
	int32_t m_room_state;
	std::map<int32_t, int32_t> m_dbid_to_dbser;
};


#endif
