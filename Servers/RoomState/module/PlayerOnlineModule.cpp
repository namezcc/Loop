#include "PlayerOnlineModule.h"
#include "Common_mod.h"
#include "RedisModule.h"
#include "utils/LPStringUtil.h"
#include "RoomTransModule.h"

#include "Crypto/crchash.h"

#include "protoPB/server/proto_common.pb.h"
#include "protoPB/server/server_msgid.pb.h"
#include "protoPB/client/client.pb.h"

#define REDIS_PLAYER_ONLINE_KEY "player_online"

std::string PlayerRoomInfo::getString()
{
	char buff[32];
	sprintf(buff, "%d:%d:%s:%d:%d", uid, gate_id, ip.c_str(), port, room_id);
	return buff;
}

bool PlayerRoomInfo::parse(const std::string & str)
{
	std::vector<std::string> res;
	Loop::Split(str, ":", res);

	if (res.size() != 5)
		return false;

	uid = Loop::Cvto<int>(res[0]);
	gate_id = Loop::Cvto<int>(res[1]);
	ip = res[2];
	port = Loop::Cvto<int>(res[3]);
	room_id = Loop::Cvto<int>(res[4]);
	return true;
}

void PlayerOnlineModule::Init()
{
	COM_MOD_INIT;

	m_redis_mod = GET_MODULE(RedisModule);
	m_room_mgr = GET_MODULE(RoomTransModule);

	m_msg_mod->AddMsgCall(LPMsg::IM_RMGR_PLAYER_LOGIN, BIND_SHARE_CALL(onPlayerOnline));
	m_msg_mod->AddMsgCall(N_TRMM_CHECK_PLAYER_ONLINE, BIND_NETMSG(onCheckPlayerOnline));
	m_msg_mod->AddMsgCall(N_TRMM_TRANS_MSG_TO_PLAYER, BIND_NETMSG(onPlayerTransMsg));
	

}

void PlayerOnlineModule::AfterInit()
{
	loadPlayerLogin();
}

void PlayerOnlineModule::loadPlayerLogin()
{
	std::vector<pair_str> res;
	m_redis_mod->HGetAll(REDIS_PLAYER_ONLINE_KEY, res);
	PlayerRoomInfo rinfo;

	for (auto& i:res)
	{
		if (rinfo.parse(i.second))
			m_player_online[rinfo.uid] = rinfo;
	}
}

void PlayerOnlineModule::sendGatePlayerLogin(PlayerRoomInfo & info, int32_t key)
{
	auto pack = GET_LAYER_MSG(BuffBlock);
	pack->writeInt32(info.uid);
	pack->writeInt32(key);
	pack->writeInt32(info.room_id);
	m_trans_mod->SendToServer(LOOP_GATE, info.gate_id,LPMsg::IM_GATE_PLAYER_LOGIN, pack);
}

void PlayerOnlineModule::onPlayerOnline(SHARE<BaseMsg>& msg)
{
	auto netmsg = (NetMsg*)msg->m_data;
	TRY_PARSEPB(LPMsg::Int32Value, netmsg);
	LPMsg::RoomInfo pbsend;

	auto it = m_player_online.find(pbMsg.value());
	if (it != m_player_online.end())
	{
		auto ser = m_trans_mod->GetServer(LOOP_GATE, it->second.gate_id);
		auto rmser = m_trans_mod->GetServer(LOOP_ROOM, it->second.room_id);
		if (ser != NULL && rmser != NULL)
		{
			//服务器重启会导致ip port更改,不能用存下来的值		
			pbsend.set_uid(pbMsg.value());
			pbsend.set_ip(ser->ip);
			pbsend.set_port(ser->port);
			pbsend.set_key(common::Hash32(std::to_string(pbMsg.value()) + std::to_string(Loop::GetSecend())));

			m_net_mod->ResponseMsg(netmsg->socket, msg, pbsend);
			sendGatePlayerLogin(it->second,pbsend.key());
			return;
		}
	}

	PlayerRoomInfo info = {};
	info.uid = pbMsg.value();
	
	if (m_room_mgr->chooseGateAndRoom(info))
	{
		auto suid = std::to_string(info.uid);
		pbsend.set_uid(info.uid);
		pbsend.set_ip(info.ip);
		pbsend.set_port(info.port);
		pbsend.set_key(common::Hash32(suid + std::to_string(Loop::GetSecend())));

		m_redis_mod->HSet(REDIS_PLAYER_ONLINE_KEY, suid, info.getString());
		m_player_online[info.uid] = info;
		sendGatePlayerLogin(info, pbsend.key());
	}
	m_net_mod->ResponseMsg(netmsg->socket, msg, pbsend);
}

void PlayerOnlineModule::onPlayerLogout(NetMsg * msg)
{
	auto pack = msg->m_buff;
	auto uid = pack->readInt32();

	auto it = m_player_online.find(uid);
	if (it == m_player_online.end())
		return;

	m_redis_mod->HDel(REDIS_PLAYER_ONLINE_KEY, std::to_string(uid));
	m_player_online.erase(it);
}

void PlayerOnlineModule::onCheckPlayerOnline(NetMsg * msg)
{
	auto pack = msg->m_buff;
	auto pid = pack->readInt64();
	auto type = pack->readInt32();
	auto num = pack->readInt32();

	auto sendpack = GET_LAYER_MSG(BuffBlock);
	auto len = sizeof(int64_t) + sizeof(int32_t)*2 + num*(sizeof(int64_t)+sizeof(int8_t));
	sendpack->makeRoom(len);

	sendpack->writeInt64(pid);
	sendpack->writeInt32(type);
	sendpack->writeInt32(num);

	for (int32_t i = 0; i < num; i++)
	{
		auto pid = pack->readInt64();
		sendpack->writeInt64(pid);

		auto it = m_online_info.find(pid);
		if (it == m_online_info.end())
			sendpack->writeInt8(0);
		else
			sendpack->writeInt8(1);
	}

	m_net_mod->SendNetMsg(msg->socket, N_TRMM_CHECK_PLAYER_ONLINE, sendpack);
}

void PlayerOnlineModule::onPlayerTransMsg(NetMsg * msg)
{
	auto pack = msg->m_buff;
	auto pid = pack->readInt64();

	auto it = m_online_info.find(pid);
	if (it == m_online_info.end())
		return;

	auto mid = pack->readInt32();

	int32_t len;
	auto buf = pack->readBuff(len);

	auto sendpack = GET_LAYER_MSG(BuffBlock);
	sendpack->writeInt64(pid);
	sendpack->writeBuff(buf, len);
	m_net_mod->SendNetMsg(it->second, mid, sendpack);
}

void PlayerOnlineModule::onPlayerOffline(int64_t pid)
{
	m_online_info.erase(pid);
}