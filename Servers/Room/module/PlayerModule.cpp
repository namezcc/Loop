#include "PlayerModule.h"
#include "MsgModule.h"
#include "TransMsgModule.h"
#include "NetObjectModule.h"
#include "EventModule.h"
#include "RoomMsgDefine.h"
#include "RoomModule.h"
#include "RoomLuaModule.h"
#include "LuaModule.h"

#include "CommonDefine.h"
#include "ServerMsgDefine.h"

#include "protoPB/client/define.pb.h"
#include "protoPB/client/client.pb.h"
#include "protoPB/client/room.pb.h"

#include "protoPB/server/server.pb.h"
#include "protoPB/server/dbdata.pb.h"
#include "protoPB/server/server_msgid.pb.h"
#include "protoPB/server/proto_common.pb.h"

PlayerModule::PlayerModule(BaseLayer * l):BaseModule(l)
{
	
}

PlayerModule::~PlayerModule()
{
}

void PlayerModule::sendToPlayer(int32_t gateid, int32_t uid, int32_t mid, gpb::Message & msg)
{
	auto pack = LAYER_BUFF;
	pack->makeRoom(msg.ByteSize() + sizeof(int32_t) * 2);
	pack->writeInt32(1);
	pack->writeInt32(uid);
	pack->write(msg);

	m_transModule->SendToServer(LOOP_GATE, gateid, mid, pack);
}

void PlayerModule::sendToPlayer(int32_t gateid, int32_t uid, int32_t mid, const char * msg, const int32_t & msglen)
{
	auto pack = LAYER_BUFF;
	pack->makeRoom(msglen + sizeof(int32_t) * 2);
	pack->writeInt32(1);
	pack->writeInt32(uid);
	pack->writeBuff(msg, msglen);
	m_transModule->SendToServer(LOOP_GATE, gateid, mid, pack);
}

void PlayerModule::sendToPlayer(int32_t gateid, int32_t uid, int32_t mid, BuffBlock * msg)
{
	auto pack = LAYER_BUFF;
	pack->writeInt32(1);
	pack->writeInt32(uid);
	pack->m_next = msg;
	m_transModule->SendToServer(LOOP_GATE, gateid, mid, pack);
}

void PlayerModule::broadToPlayer(int32_t gateid, std::vector<int32_t>& vec, int32_t mid, gpb::Message & msg)
{
	if (vec.empty())
		return;

	auto pack = LAYER_BUFF;
	pack->makeRoom(msg.ByteSize() + sizeof(int32_t) * (vec.size()+1));
	pack->writeInt32((int32_t)vec.size());
	for (auto& uid:vec)
	{
		pack->writeInt32(uid);
	}
	pack->write(msg);
	m_transModule->SendToServer(LOOP_GATE, gateid, LPMsg::SM_SELF_ROLE_INFO, pack);
}

void PlayerModule::broadToPlayer(int32_t gateid, std::vector<int32_t>& vec, int32_t mid, BuffBlock * msg)
{
	if (vec.empty())
	{
		RECYCLE_LAYER_MSG(msg);
		return;
	}
	auto pack = LAYER_BUFF;
	pack->makeRoom(sizeof(int32_t) * (vec.size() + 1));
	pack->writeInt32((int32_t)vec.size());
	for (auto& uid : vec)
	{
		pack->writeInt32(uid);
	}
	pack->m_next = msg;
	m_transModule->SendToServer(LOOP_GATE, gateid, LPMsg::SM_SELF_ROLE_INFO, pack);
}

void PlayerModule::Init()
{
	m_msgModule = GET_MODULE(MsgModule);
	m_transModule = GET_MODULE(TransMsgModule);
	m_netobjModule = GET_MODULE(NetObjectModule);
	m_eventModule = GET_MODULE(EventModule);
	m_room_mod = GET_MODULE(RoomModuloe);
	m_room_lua = GET_MODULE(RoomLuaModule);
	m_lua_mod = GET_MODULE(LuaModule);

	m_msgModule->setCommonCall(BIND_NETMSG(onNetMsg));
	
	m_eventModule->AddEventCall(E_SERVER_CONNECT, BIND_EVENT(onServerConnect, SHARE<NetServer>&));
	m_eventModule->AddEventCall(E_SERVER_CLOSE, BIND_EVENT(onServerClose, SHARE<NetServer>&));

	m_msgModule->AddMsgCall(LPMsg::IM_ROOM_GET_ROLE_LIST, BIND_NETMSG(onGetRoleList));
	m_msgModule->AddMsgCall(LPMsg::IM_ROOM_PLAYER_LOGIN, BIND_NETMSG(onPlayerLogin));
	m_msgModule->AddMsgCall(LPMsg::IM_ROOM_PLAYER_LOGOUT, BIND_NETMSG(onPlayerLogout));
	m_msgModule->AddMsgCall(LPMsg::CM_CREATE_ROLE, BIND_NETMSG(onCreatePlayer));
	m_msgModule->AddMsgCall(LPMsg::CM_ENTER_GAME, BIND_NETMSG(onEnterGame));

}

void PlayerModule::onServerConnect(SHARE<NetServer>& ser)
{
	if (ser->type == LOOP_ROOM_MANAGER)
	{
		sendRoomMgrPlayerNum();
	}
}

void PlayerModule::onServerClose(SHARE<NetServer>& ser)
{

}

void PlayerModule::sendRoomMgrPlayerNum()
{
	auto server = GetLayer()->GetServer();
	auto pack = LAYER_BUFF;
	pack->writeInt32(LOOP_ROOM);
	pack->writeInt32(server->serid);
	pack->writeInt32(m_player_info.size());
	m_transModule->SendToServer(LOOP_ROOM_MANAGER, 0, LPMsg::IM_RMGR_PLAYER_ONLINE_NUM, pack);
}

void PlayerModule::onPlayerLogin(NetMsg * msg)
{
	auto pack = msg->m_buff;
	auto gatesid = pack->readInt32();
	auto uid = pack->readInt32();

	auto it = m_player_info.find(uid);
	if (it != m_player_info.end())
	{
		LuaArgs arg;
		arg.pushArg(uid);
		m_room_lua->callLuaMsg(CTOL_PLAYER_OFFLINE, arg);
		it->second.gate_sid = gatesid;
		it->second.enter_cid = 0;
		it->second.m_roles.clear();
	}
	else
	{
		PlayerInfo info;
		info.uid = uid;
		info.gate_sid = gatesid;
		info.enter_cid = 0;
		m_player_info[uid] = info;
	}
	//get role list
	m_room_mod->doSqlOperation(uid, SOP_ROLE_SELET, LPMsg::Int32Value{}, LPMsg::IM_ROOM_GET_ROLE_LIST);
	sendRoomMgrPlayerNum();

	LuaArgs arg;
	arg.pushArg(uid);
	arg.pushArg(gatesid);
	m_room_lua->callLuaMsg(CTOL_SET_PLAYER_GATE_INFO, arg);
}

void PlayerModule::onPlayerLogout(NetMsg * msg)
{
	auto pack = msg->m_buff;
	auto uid = pack->readInt32();
	auto type = pack->readInt32();
	LuaArgs arg;
	arg.pushArg(uid);
	m_room_lua->callLuaMsg(CTOL_PLAYER_OFFLINE, arg);
	m_player_info.erase(uid);
	sendRoomMgrPlayerNum();
}

void PlayerModule::onCreatePlayer(NetMsg* msg)
{
	auto uid = msg->m_buff->readInt32();
	TRY_PARSEPB(LPMsg::ReqCreateRole, msg);
	auto it = m_player_info.find(uid);
	if (it == m_player_info.end())
	{
		LP_ERROR << "create player not login uid:" << uid;
		return;
	}

	if (pbMsg.name().size() >= 30)
	{
		LP_INFO << "name to lone name:" << pbMsg.name();
		return;
	}

	LPMsg::DB_player spb;
	spb.set_uid(uid);
	spb.set_cid(uid);
	spb.set_level(1);
	spb.set_name(pbMsg.name());
	spb.set_create_time(Loop::GetSecend());
	m_room_mod->doSqlOperation(uid, SOP_CREATE_ROLE, spb, LPMsg::IM_ROOM_GET_ROLE_LIST);
}

void PlayerModule::onEnterGame(NetMsg * msg)
{
	auto uid = msg->m_buff->readInt32();
	TRY_PARSEPB(LPMsg::Int32Value, msg);

	auto it = m_player_info.find(uid);
	if (it == m_player_info.end())
		return;

	if (it->second.enter_cid > 0)
	{
		LP_ERROR << "have role in game uid:" << uid << "cid:"<<it->second.enter_cid;
		return;
	}


	auto itr = it->second.m_roles.find(pbMsg.value());
	if (itr == it->second.m_roles.end())
	{
		return;
	}

	LP_INFO << "player enter cid:" << pbMsg.value();
	it->second.enter_cid = pbMsg.value();
	m_room_mod->doSqlOperation(uid, SOP_LOAD_PLAYER_DATA, pbMsg, LPMsg::IM_ROOM_LOAD_ROLE_INFO);
}

void PlayerModule::onGetRoleList(NetMsg * msg)
{
	auto uid = msg->m_buff->readInt32();
	auto it = m_player_info.find(uid);
	if (it == m_player_info.end())
		return;

	TRY_PARSEPB(LPMsg::DB_roleList, msg);
	LPMsg::SmRoleList spb;

	for (auto r:pbMsg.roles())
	{
		auto role = spb.add_roles();
		role->set_cid(r.cid());
		role->set_name(r.name());
		role->set_level(r.level());

		RoleInfo rinfo = {};
		rinfo.cid = r.cid();
		rinfo.name = r.name();
		rinfo.level = r.level();
		it->second.m_roles[rinfo.cid] = rinfo;
	}

	sendToPlayer(it->second.gate_sid,it->second.uid, LPMsg::SM_SELF_ROLE_INFO, spb);
}

void PlayerModule::onNetMsg(NetMsg * msg)
{
	auto pack = msg->m_buff;
	if (msg->mid >= LPMsg::CM_BEGAN && msg->mid <= LPMsg::CM_END)
	{
		LuaArgs arg;
		arg.pushArg(msg->mid);
		arg.pushArg(pack->readInt32()); //uid
		int32_t bsize = 0;
		auto buff = pack->readBuff(bsize);
		pack->setOffect(pack->getOffect() - bsize);
		arg.pushArg(buff, bsize);
		m_lua_mod->callLuaFunc(CTOL_NET_MSG,arg);
	}
	else
	{
		LuaArgs arg;
		arg.pushArg(msg->mid);
		int32_t bsize = 0;
		auto buff = pack->readBuff(bsize);
		pack->setOffect(pack->getOffect() - bsize);
		arg.pushArg(buff, bsize);
		arg.pushArg(msg);
		m_lua_mod->callLuaFunc(CTOL_NET_MSG, arg);
	}
}
