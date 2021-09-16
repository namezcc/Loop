#include "PlayerOnlineModule.h"
#include "Common_mod.h"


void PlayerOnlineModule::Init()
{
	COM_MOD_INIT;

	m_msg_mod->AddMsgCall(N_TRMM_PLAYER_ONLINE, BIND_NETMSG(onPlayerOnline));
	m_msg_mod->AddMsgCall(N_TRMM_CHECK_PLAYER_ONLINE, BIND_NETMSG(onCheckPlayerOnline));
	m_msg_mod->AddMsgCall(N_TRMM_TRANS_MSG_TO_PLAYER, BIND_NETMSG(onPlayerTransMsg));
	

}

void PlayerOnlineModule::onPlayerOnline(NetMsg * msg)
{
	auto pack = msg->m_buff;
	auto num = pack->readInt32();

	for (int32_t i = 0; i < num; i++)
	{
		auto pid = pack->readInt64();
		m_online_info[pid] = msg->socket;
	}
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

