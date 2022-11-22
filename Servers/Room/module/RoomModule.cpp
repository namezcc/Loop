#include "RoomModule.h"
#include "MsgModule.h"
#include "TransMsgModule.h"
#include "NetObjectModule.h"
#include "EventModule.h"
#include "CommonDefine.h"
#include "ServerMsgDefine.h"

#include "help_function.h"

#include "protoPB/server/server_msgid.pb.h"

RoomModuloe::RoomModuloe(BaseLayer * l) :BaseModule(l)
{
}

RoomModuloe::~RoomModuloe()
{
}

void RoomModuloe::setRoomState(int32_t state)
{
	m_room_state = state;

	auto pack = GET_LAYER_MSG(BuffBlock);
	pack->writeInt32(GetLayer()->GetServer()->serid);
	pack->writeInt32(m_room_state);
	ServerNode sernode{ LOOP_ROOM_MANAGER,1 };
	m_trans_mod->SendToServer(sernode, N_ROOM_STATE, pack);
}

void RoomModuloe::Init()
{
	m_msg_mod = GET_MODULE(MsgModule);
	m_trans_mod = GET_MODULE(TransMsgModule);
	m_net_mod = GET_MODULE(NetObjectModule);
	m_event_mod = GET_MODULE(EventModule);

	m_room_state = SBS_NORMAL;

	m_event_mod->AddEventCall(E_SERVER_CLOSE, BIND_EVENT(onServerClose, SHARE<NetServer>&));
	m_msg_mod->AddMsgCall(LPMsg::IM_ROOM_REG_DBID, BIND_NETMSG(onRegDbidToSerid));
	
	m_db_path.push_back(*GetLayer()->GetServer());
	m_db_path.push_back(ServerNode{ SERVER_TYPE::LOOP_PROXY_DB,0 });
	m_db_path.push_back(ServerNode{ SERVER_TYPE::LOOP_MYSQL,1 });
}

void RoomModuloe::onServerClose(SHARE<NetServer>& ser)
{
	if (ser->type == LOOP_MYSQL)
	{
		for (auto it = m_dbid_to_dbser.begin();it!=m_dbid_to_dbser.end();it++)
		{
			if (it->second == ser->serid)
			{
				m_dbid_to_dbser.erase(it);
				return;
			}
		}
	}
}

void RoomModuloe::onRegDbidToSerid(NetMsg * msg)
{
	auto pack = msg->m_buff;
	auto dbid = pack->readInt32();
	auto serid = pack->readInt32();
	m_dbid_to_dbser[dbid] = serid;
}

ServerPath & RoomModuloe::getDbPath(int64_t uid)
{
	auto dbid = getPlayerDbIndexFromUid(uid);
	m_db_path[1].serid = 0;
	m_db_path[2].serid = dbid;
	return m_db_path;
}

int32_t RoomModuloe::getDbserid(int32_t cid)
{
	auto dbid = getDbidFromCid32(cid);
	auto it = m_dbid_to_dbser.find(dbid);
	if(it == m_dbid_to_dbser.end())
		return 0;
	return it->second;
}

void RoomModuloe::doSqlOperation(int32_t cid, int32_t opt, const google::protobuf::Message & pb, int32_t ackId)
{
	auto serid = getDbserid(cid);
	if (serid <= 0)
	{
		LP_ERROR << "get dbserid error cid:" << cid;
		return;
	}

	auto pack = GET_LAYER_MSG(BuffBlock);
	pack->makeRoom(pb.ByteSize() + sizeof(int32_t) * 2 + sizeof(int64_t));
	pack->writeInt32(opt);
	pack->writeInt32(ackId);
	pack->writeInt32(cid);
	pack->write(pb);

	m_trans_mod->SendToServer(LOOP_MYSQL, serid, LPMsg::IM_DB_SQL_OPERATION, pack);
}

void RoomModuloe::doSqlOperation(int32_t cid, int32_t opt, const char * buf, int32_t buflen, int32_t ackId)
{
	auto serid = getDbserid(cid);
	if (serid <= 0)
	{
		LP_ERROR << "get dbserid error cid:" << cid;
		return;
	}

	auto pack = GET_LAYER_MSG(BuffBlock);
	pack->makeRoom(sizeof(int32_t) * 3 + buflen);
	pack->writeInt32(opt);
	pack->writeInt32(ackId);
	pack->writeInt32(cid);
	pack->writeBuff(buf, buflen);

	m_trans_mod->SendToServer(LOOP_MYSQL, serid, LPMsg::IM_DB_SQL_OPERATION, pack);
}

void RoomModuloe::doSqlOperation(int32_t cid, int32_t opt, BuffBlock * buf, int32_t ackId)
{
	auto serid = getDbserid(cid);
	if (serid <= 0)
	{
		LP_ERROR << "get dbserid error cid:" << cid;
		return;
	}

	auto pack = GET_LAYER_MSG(BuffBlock);
	pack->makeRoom(sizeof(int32_t) * 2 + sizeof(int64_t));
	pack->writeInt32(opt);
	pack->writeInt32(ackId);
	pack->writeInt32(cid);
	pack->m_next = buf;

	m_trans_mod->SendToServer(LOOP_MYSQL, serid, LPMsg::IM_DB_SQL_OPERATION, pack);
}

void RoomModuloe::updatePlayerData(int64_t pid, const google::protobuf::Message & pb, int32_t table, std::string key1, std::string key2)
{
	auto pack = GET_LAYER_MSG(BuffBlock);
	pack->makeRoom(pb.ByteSize() + 100);
	pack->writeInt32(SOP_UPDATE_PLAYER_DATA);
	pack->writeInt32(0);
	pack->writeInt64(pid);

	pack->writeInt32(table);
	pack->writeInt32(1);
	pack->writeString(key1);
	pack->writeString(key2);
	pack->writeString(pb);

	m_trans_mod->SendToServer(getDbPath(pid), LPMsg::IM_DB_SQL_OPERATION, pack);
}

void RoomModuloe::updatePlayerData(int64_t pid, const std::string & pb, int32_t table, std::string key1, std::string key2)
{
	auto pack = GET_LAYER_MSG(BuffBlock);
	pack->makeRoom(pb.size() + 100);
	pack->writeInt32(SOP_UPDATE_PLAYER_DATA);
	pack->writeInt32(0);
	pack->writeInt64(pid);

	pack->writeInt32(table);
	pack->writeInt32(1);
	pack->writeString(key1);
	pack->writeString(key2);
	pack->writeString(pb);

	m_trans_mod->SendToServer(getDbPath(pid), LPMsg::IM_DB_SQL_OPERATION, pack);
}

void RoomModuloe::deletePlayerData(int64_t pid, const std::string & pb, int32_t table, std::string key1, std::string key2)
{
	auto pack = GET_LAYER_MSG(BuffBlock);
	pack->makeRoom(pb.size() + 100);
	pack->writeInt32(SOP_DELETE_PLAYER_DATA);
	pack->writeInt32(0);
	pack->writeInt64(pid);

	pack->writeInt32(table);
	pack->writeInt32(1);
	pack->writeString(key1);
	pack->writeString(key2);
	pack->writeString(pb);

	m_trans_mod->SendToServer(getDbPath(pid), LPMsg::IM_DB_SQL_OPERATION, pack);
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

	m_trans_mod->SendToServer(getDbPath(uid), LPMsg::IM_DB_SQL_OPERATION, pack);
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
	pack->writeInt32((int32_t)keys.size());

	for (size_t i = 0; i < keys.size(); i++)
	{
		pack->writeInt32(keys[i].first);
		pack->writeInt32(keys[i].second);
		pack->writeString(pb[i]);
	}

	m_trans_mod->SendToServer(getDbPath(uid), LPMsg::IM_DB_SQL_OPERATION, pack);
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
	pack->writeInt32((int32_t)keys.size());

	for (size_t i = 0; i < keys.size(); i++)
	{
		pack->writeInt32(keys[i].first);
		pack->writeInt32(keys[i].second);
	}

	m_trans_mod->SendToServer(getDbPath(uid), LPMsg::IM_DB_SQL_OPERATION, pack);
}
