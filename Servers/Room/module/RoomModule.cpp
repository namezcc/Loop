#include "RoomModule.h"
#include "MsgModule.h"
#include "TransMsgModule.h"
#include "NetObjectModule.h"
#include "EventModule.h"
#include "CommonDefine.h"
#include "ServerMsgDefine.h"

#include "help_function.h"

RoomModuloe::RoomModuloe(BaseLayer * l) :BaseModule(l)
{
}

RoomModuloe::~RoomModuloe()
{
}

void RoomModuloe::Init()
{
	m_msg_mod = GET_MODULE(MsgModule);
	m_trans_mod = GET_MODULE(TransMsgModule);
	m_net_mod = GET_MODULE(NetObjectModule);
	m_event_mod = GET_MODULE(EventModule);


	m_event_mod->AddEventCall(E_SERVER_CONNECT, BIND_EVENT(onServerConnect, SHARE<NetServer>&));


	m_db_path.push_back(*GetLayer()->GetServer());
	m_db_path.push_back(ServerNode{ SERVER_TYPE::LOOP_PROXY_DB,0 });
	m_db_path.push_back(ServerNode{ SERVER_TYPE::LOOP_MYSQL,1 });
}

void RoomModuloe::onServerConnect(SHARE<NetServer>& ser)
{
	if (ser->type == LOOP_ROOM_MANAGER)
	{
		auto pack = GET_LAYER_MSG(BuffBlock);
		pack->writeInt32(ser->serid);
		pack->writeInt32(SBS_NORMAL);
		ServerNode sernode{ LOOP_ROOM_MANAGER,1 };
		m_trans_mod->SendToServer(sernode, N_ROOM_STATE, pack);
	}
}

ServerPath & RoomModuloe::getDbPath(int64_t uid)
{
	auto dbid = getPlayerDbIndexFromUid(uid);
	m_db_path[1].serid = 0;
	m_db_path[2].serid = dbid;
	return m_db_path;
}

void RoomModuloe::doSqlOperation(int64_t uid, int32_t opt, google::protobuf::Message & pb, int32_t ackId)
{
	auto pack = GET_LAYER_MSG(BuffBlock);
	pack->makeRoom(pb.ByteSize() + sizeof(int32_t) * 2 + sizeof(int64_t));
	pack->writeInt32(opt);
	pack->writeInt32(ackId);
	pack->writeInt64(uid);
	pack->write(pb);

	m_trans_mod->SendToServer(getDbPath(uid), N_TDB_SQL_OPERATION, pack);
}

void RoomModuloe::updatePlayerData(int64_t uid, int32_t rid, google::protobuf::Message & pb, int32_t table, int32_t key1, int32_t key2)
{
	auto pack = GET_LAYER_MSG(BuffBlock);
	pack->makeRoom(sizeof(int32_t) * 2 + sizeof(int64_t) + sizeof(int32_t)*6 + pb.ByteSize());
	pack->writeInt32(SOP_UPDATE_PLAYER_DATA);
	pack->writeInt32(0);
	pack->writeInt64(uid);
	pack->writeInt32(rid);
	pack->writeInt32(table);

	pack->writeInt32(1);
	pack->writeInt32(key1);
	pack->writeInt32(key2);
	pack->writeString(pb);

	m_trans_mod->SendToServer(getDbPath(uid), N_TDB_SQL_OPERATION, pack);
}

void RoomModuloe::deletePlayerData(int64_t uid, int32_t rid, int32_t table, int32_t key1, int32_t key2)
{
	auto pack = GET_LAYER_MSG(BuffBlock);
	pack->makeRoom(sizeof(int32_t) * 2 + sizeof(int64_t) + sizeof(int32_t) * 5);
	pack->writeInt32(SOP_DELETE_PLAYER_DATA);
	pack->writeInt32(0);
	pack->writeInt64(uid);
	pack->writeInt32(rid);
	pack->writeInt32(table);

	pack->writeInt32(1);
	pack->writeInt32(key1);
	pack->writeInt32(key2);

	m_trans_mod->SendToServer(getDbPath(uid), N_TDB_SQL_OPERATION, pack);
}

void RoomModuloe::updatePlayerData(int64_t uid, int32_t rid, int32_t table, std::vector<int32Pair>& keys, std::vector<gpb::Message>& pb)
{
	if (keys.empty() || keys.size() != pb.size())
		return;

	auto pbsize = 0;
	for (auto& m:pb)
	{
		pbsize += sizeof(int32_t) + m.ByteSize();
	}

	auto pack = GET_LAYER_MSG(BuffBlock);
	pack->makeRoom(sizeof(int32_t) * 2 + sizeof(int64_t) + sizeof(int32_t) * 3 + keys.size()*2*sizeof(int32_t) + pbsize);
	pack->writeInt32(SOP_UPDATE_PLAYER_DATA);
	pack->writeInt32(0);
	pack->writeInt64(uid);
	pack->writeInt32(rid);
	pack->writeInt32(table);
	pack->writeInt32(keys.size());

	for (size_t i = 0; i < keys.size(); i++)
	{
		pack->writeInt32(keys[i].first);
		pack->writeInt32(keys[i].second);
		pack->writeString(pb[i]);
	}

	m_trans_mod->SendToServer(getDbPath(uid), N_TDB_SQL_OPERATION, pack);
}

void RoomModuloe::deletePlayerData(int64_t uid, int32_t rid, int32_t table, std::vector<int32Pair>& keys)
{
	if (keys.empty())
		return;

	auto pack = GET_LAYER_MSG(BuffBlock);
	pack->makeRoom(sizeof(int32_t) * 2 + sizeof(int64_t) + sizeof(int32_t) * 3 + keys.size() * 2 * sizeof(int32_t));
	pack->writeInt32(SOP_DELETE_PLAYER_DATA);
	pack->writeInt32(0);
	pack->writeInt64(uid);
	pack->writeInt32(rid);
	pack->writeInt32(table);
	pack->writeInt32(keys.size());

	for (size_t i = 0; i < keys.size(); i++)
	{
		pack->writeInt32(keys[i].first);
		pack->writeInt32(keys[i].second);
	}

	m_trans_mod->SendToServer(getDbPath(uid), N_TDB_SQL_OPERATION, pack);
}
