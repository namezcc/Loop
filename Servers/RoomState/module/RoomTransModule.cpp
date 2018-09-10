#include "RoomTransModule.h"
#include "MsgModule.h"
#include "TransMsgModule.h"
#include "EventModule.h"
#include "NetObjectModule.h"
#include "Servers/Login/module/RoomStateModule.h"
#include "protoPB/server/server.pb.h"


RoomTransModule::RoomTransModule(BaseLayer * l):BaseModule(l)
{
}

RoomTransModule::~RoomTransModule()
{
}

void RoomTransModule::Init()
{
	m_msgModule = GET_MODULE(MsgModule);
	m_transModule = GET_MODULE(TransMsgModule);
	m_eventModule = GET_MODULE(EventModule);
	m_netobjModule = GET_MODULE(NetObjectModule);

	m_msgModule->AddMsgCallBack(N_REQ_ROOM_LIST, this, &RoomTransModule::OnReqRoomList);

	m_eventModule->AddEventCallBack(E_SERVER_CONNECT, this, &RoomTransModule::OnServerConnect);
	m_eventModule->AddEventCallBack(E_SERVER_CLOSE, this, &RoomTransModule::OnServerClose);
}

void RoomTransModule::OnReqRoomList(NetMsg * msg)
{
	auto& serlist = m_transModule->GetServerList();

	LPMsg::RoomStateList ackmsg;

	auto it = serlist.find(SERVER_TYPE::LOOP_ROOM);
	if (it != serlist.end())
	{
		for (auto room:it->second)
		{
			auto pbrm = ackmsg.add_list();
			pbrm->set_id(room.second->serid);
			pbrm->set_ip(room.second->ip);
			pbrm->set_port(room.second->port);
		}
	}
	m_netobjModule->SendNetMsg(msg->socket, N_ACK_ROOM_LIST, ackmsg);
}

void RoomTransModule::OnServerConnect(SHARE<NetServer>& ser)
{
	if (ser->type == SERVER_TYPE::LOOP_ROOM)
	{
		LPMsg::RoomState msg;
		msg.set_id(ser->serid);
		msg.set_ip(ser->ip);
		msg.set_port(ser->port);
		msg.set_state(ROOM_STATE::ROOM_OPEN);

		m_transModule->SendToAllServer(LOOP_LOGIN, N_ROOM_STATE, msg);
	}
}

void RoomTransModule::OnServerClose(SHARE<NetServer>& ser)
{
	if (ser->type == SERVER_TYPE::LOOP_ROOM)
	{
		LPMsg::RoomState msg;
		msg.set_id(ser->serid);
		msg.set_ip(ser->ip);
		msg.set_port(ser->port);
		msg.set_state(ROOM_STATE::ROOM_CLOSE);

		m_transModule->SendToAllServer(LOOP_LOGIN, N_ROOM_STATE, msg);
	}
}
