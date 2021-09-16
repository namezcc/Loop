#include "RoomLuaModule.h"
#include "RoomModule.h"


void RoomLuaModule::Init()
{
	m_lua_mod = GET_MODULE(LuaModule);
	m_room_mod = GET_MODULE(RoomModuloe);


}

void RoomLuaModule::AfterInit()
{
	m_lua_mod->runScript("./lua/room/main.lua");
	m_lua_mod->setLuaCallFunc(std::bind(&RoomLuaModule::onLuaCallFunc, this, HOLD_2));
	m_lua_mod->setUpdateFunc("update");


	LuaArgs arg;
	arg.pushArg(GetLayer()->GetServer()->serid);
	m_lua_mod->callGlobalFunc("set_self_server_id", arg);

}

int RoomLuaModule::onLuaCallFunc(int32_t findex, LuaState * l)
{
	switch (findex)
	{
	case LTOC_DO_SQL_OPERATION:
	{
		auto pid = l->PullInt64();
		auto opt = l->PullInt32();
		auto buf = (BuffBlock*)l->PullUserData();
		auto ack = l->PullInt32();
		if(buf)
			m_lua_mod->removeSendBuff(buf);
		m_room_mod->doSqlOperation(pid, opt, buf, ack);
		break;
	}
	case LTOC_SQL_UPDATE_PLAYER_DATA:
	{
		auto pid = l->PullInt64();
		auto table = l->PullInt32();
		auto k1 = l->PullString();
		auto k2 = l->PullString();
		auto pb = l->PullString();

		m_room_mod->updatePlayerData(pid, pb, table, k1, k2);
		break;
	}
	case LTOC_SQL_DELETE_PLAYER_DATA:
	{
		auto pid = l->PullInt64();
		auto table = l->PullInt32();
		auto k1 = l->PullString();
		auto k2 = l->PullString();
		auto pb = l->PullString();

		m_room_mod->deletePlayerData(pid, pb, table, k1, k2);
		break;
	}
	default:
		break;
	}
	return 0;
}

void RoomLuaModule::setPlayerSock(int64_t uid, int32_t sock)
{
	LuaArgs arg;
	arg.pushArg(uid);
	arg.pushArg(sock);
	//m_lua_mod->callLuaFunc(CTOL_SET_PLAYER_SOCK, arg);
	callLuaMsg(CTOL_SET_PLAYER_SOCK, arg);
}

void RoomLuaModule::callLuaMsg(int32_t mid, LuaArgs & arg)
{
	arg.pushArg(mid, 0);
	m_lua_mod->callLuaMsg(arg);
}
