#include "GateModule.h"
#include "MsgModule.h"
#include "TransMsgModule.h"
#include "NetObjectModule.h"
#include "EventModule.h"

#include "protoPB/client/define.pb.h"
#include "protoPB/client/client.pb.h"
#include "protoPB/server/server_msgid.pb.h"

GateModule::GateModule(BaseLayer * l):BaseModule(l), m_player_num(0)
{
	memset(m_player_sock, 0, sizeof(m_player_sock));
}

GateModule::~GateModule()
{
}

void GateModule::Init()
{
	COM_MOD_INIT;

	m_event_mod = GET_MODULE(EventModule);
	m_event_mod->AddEventCall(E_SOCKET_CLOSE, BIND_EVENT(onClientClose, int32_t));
	m_event_mod->AddEventCall(E_SERVER_CONNECT, BIND_EVENT(onServerConnect, SHARE<NetServer>&));

	m_msg_mod->setCommonCall(BIND_NETMSG(onNetMsg));

	m_msg_mod->AddMsgCall(LPMsg::IM_GATE_PLAYER_LOGIN, BIND_NETMSG(onPlayerReadyInfo));
	m_msg_mod->AddMsgCall(LPMsg::CM_ENTER_ROOM, BIND_NETMSG(onPlayerLogin));
}

void GateModule::onServerConnect(SHARE<NetServer>& ser)
{
	if (ser->type == LOOP_ROOM_MANAGER)
	{
		sendRoomMgrPlayerNum();
	}
}

void GateModule::onServerClose(SHARE<NetServer>& ser)
{

}

void GateModule::onClientClose(const int32_t & sock)
{
	auto rinfo = m_player_sock[sock];
	if (rinfo == NULL)
		return;

	m_player_sock[sock] = NULL;
	m_player_num--;

	auto pack = LAYER_BUFF;
	pack->writeInt32(rinfo->uid);
	pack->writeInt32(0);
	m_trans_mod->SendToServer(LOOP_ROOM, rinfo->roomsid, LPMsg::IM_ROOM_PLAYER_LOGOUT, pack);
	sendRoomMgrPlayerLogout(rinfo->uid);
	sendRoomMgrPlayerNum();
	m_readyInfo.erase(rinfo->uid);
}

void GateModule::onNetMsg(NetMsg * msg)
{
	if (msg->role == SOCK_ROLE_PLAYER)
	{
		auto rinfo = m_player_sock[msg->socket];
		if (rinfo == NULL)
		{
			m_net_mod->CloseNetObject(msg->socket);
			return;
		}

		if (msg->mid < LPMsg::CM_BEGAN || msg->mid > LPMsg::CM_END)
		{
			m_player_sock[msg->socket] = NULL;
			m_net_mod->CloseNetObject(msg->socket);
			m_readyInfo.erase(rinfo->uid);
			return;
		}

		auto buff = LAYER_BUFF;
		buff->makeRoom(msg->getLen() + sizeof(int32_t));
		buff->writeInt32(rinfo->uid);
		buff->writeBuff(msg->getNetBuff(), msg->getLen());
		m_trans_mod->SendToServer(LOOP_ROOM, rinfo->roomsid, msg->mid,buff);
	}
	else {
		if (msg->mid >= LPMsg::SM_BEGIN && msg->mid <= LPMsg::SM_END)
		{
			auto pack = msg->m_buff;
			auto usize = pack->readInt32();
			if (usize == 1)
			{
				auto uid = pack->readInt32();
				auto it = m_readyInfo.find(uid);
				if (it == m_readyInfo.end() || it->second.sock < 0)
					return;

				auto sendbuff = LAYER_BUFF;
				sendbuff->makeRoom(pack->getUnReadSize());
				int32_t blen = 0;
				auto buff = pack->readBuff(blen);
				sendbuff->writeBuff(buff,blen);
				m_net_mod->SendNetMsg(it->second.sock, msg->mid, sendbuff);
			}
			else {
				m_broad_vec.clear();
				for (int32_t i = 0; i <= usize; i++)
				{
					auto uid = pack->readInt32();
					auto it = m_readyInfo.find(uid);
					if (it == m_readyInfo.end())
						continue;
					if(it->second.sock >= 0)
						m_broad_vec.push_back(it->second.sock);
				}

				auto sendbuff = LAYER_BUFF;
				sendbuff->makeRoom(pack->getUnReadSize());
				int32_t blen = 0;
				auto buff = pack->readBuff(blen);
				sendbuff->writeBuff(buff, blen);
				m_net_mod->BroadNetMsg(m_broad_vec, msg->mid, sendbuff);
			}
		}
	}
}

void GateModule::onPlayerReadyInfo(NetMsg * msg)
{
	auto pack = msg->m_buff;
	auto uid = pack->readInt32();
	auto key = pack->readInt32();
	auto roomid = pack->readInt32();

	auto it = m_readyInfo.find(uid);
	if (it == m_readyInfo.end())
	{
		auto ser = m_trans_mod->GetServer(LOOP_ROOM, roomid);
		if (ser == NULL)
		{
			LP_ERROR << "error roomid get null roomid:" << roomid;
			return;
		}

		ReadyInfo info = {};
		info.sock = -1;
		info.uid = uid;
		info.roomsid = roomid;
		info.key = key;
		m_readyInfo[uid] = info;
		
	}
	else {
		it->second.key = key;
		if (roomid != it->second.roomsid)
		{
			LP_WARN << "ready info roomid diff " << it->second.roomsid << " --> " << roomid;
		}
	}
}

void GateModule::onPlayerLogin(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::CmEnterGame, msg);
	auto it = m_readyInfo.find(pbMsg.uid());
	if (it == m_readyInfo.end() || it->second.key != pbMsg.key())
	{
		m_net_mod->CloseNetObject(msg->socket);
		return;
	}

	if (it->second.sock > 0 && it->second.sock != msg->socket)
	{
		//¶¥ºÅ
		auto rinfo = m_player_sock[it->second.sock];
		if (rinfo)
		{
			auto pack = LAYER_BUFF;
			auto server = GetLayer()->GetServer();
			pack->writeInt32(pbMsg.uid());
			pack->writeInt32(1);
			m_trans_mod->SendToServer(LOOP_ROOM, rinfo->roomsid, LPMsg::IM_ROOM_PLAYER_LOGOUT, pack);
			m_player_num--;
			m_net_mod->CloseNetObject(it->second.sock);
		}
		it->second.sock = -1;
	}
	
	auto pack = LAYER_BUFF;
	auto server = GetLayer()->GetServer();
	pack->writeInt32(server->serid);
	pack->writeInt32(pbMsg.uid());

	auto roomser = m_trans_mod->GetServer(LOOP_ROOM, it->second.roomsid);
	if (roomser == NULL)
	{
		LP_ERROR << "player login roomserver close id:" << it->second.roomsid;
		m_net_mod->CloseNetObject(msg->socket);
		sendRoomMgrPlayerLogout(pbMsg.uid());
		return;
	}
	m_net_mod->SendNetMsg(roomser->socket, LPMsg::IM_ROOM_PLAYER_LOGIN, pack);
	m_player_sock[msg->socket] = &it->second;
	if(it->second.sock < 0)
		m_player_num++;

	it->second.sock = msg->socket;
	sendRoomMgrPlayerNum();
	m_net_mod->AcceptConn(msg->socket);
}

void GateModule::sendRoomMgrPlayerNum()
{
	auto server = GetLayer()->GetServer();
	auto pack = LAYER_BUFF;
	pack->writeInt32(LOOP_GATE);
	pack->writeInt32(server->serid);
	pack->writeInt32(m_player_num);
	m_trans_mod->SendToServer(LOOP_ROOM_MANAGER, 0, LPMsg::IM_RMGR_PLAYER_ONLINE_NUM, pack);
}

void GateModule::sendRoomMgrPlayerLogout(int32_t uid)
{
	auto server = GetLayer()->GetServer();
	auto pack = LAYER_BUFF;
	pack->writeInt32(uid);
	pack->writeInt32(server->serid);
	m_trans_mod->SendToServer(LOOP_ROOM_MANAGER, 0, LPMsg::IM_RMGR_PLAYER_LOGOUT, pack);
}
