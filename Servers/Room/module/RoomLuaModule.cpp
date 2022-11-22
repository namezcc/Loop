#include "RoomLuaModule.h"
#include "RoomModule.h"
#include "PlayerModule.h"
#include "ScheduleModule.h"


void RoomLuaModule::Init()
{
	m_lua_mod = GET_MODULE(LuaModule);
	m_room_mod = GET_MODULE(RoomModuloe);
	m_player_mod = GET_MODULE(PlayerModule);
	m_schedule_mod = GET_MODULE(ScheduleModule);
}

void RoomLuaModule::AfterInit()
{
	m_lua_mod->runScript("lua/room/main.lua");
	m_lua_mod->setLuaCallFunc(std::bind(&RoomLuaModule::onLuaCallFunc, this, HOLD_2));
	m_lua_mod->getLuaState()->setDealErrorFunc("luaErrorCall");

	m_schedule_mod->AddInterValTask([this](int64_t& dt) {
		LuaArgs arg;
		arg.pushArg(dt);
		arg.pushArg(int32_t(dt / 1000));
		m_lua_mod->callLuaFunc(CTOL_FRAME_UPDATE, arg);
	}, 1000);

	LuaArgs arg;
	arg.pushArg(GetLayer()->GetServer()->serid);
	m_lua_mod->callGlobalFunc("main", arg);

}

int RoomLuaModule::onLuaCallFunc(int32_t findex, LuaState * l)
{
	switch (findex)
	{
	case LTOC_DO_SQL_OPERATION:
	{
		auto pid = l->PullInt32();
		auto opt = l->PullInt32();
		auto buf = (BuffBlock*)l->PullUserData();
		auto ack = l->PullInt32();
		if(buf)
			m_lua_mod->removeSendBuff(buf);
		m_room_mod->doSqlOperation(pid, opt, buf, ack);
		break;
	}
	case LTOC_DO_SQL_OPERATION_PROTO:
	{
		auto cid = l->PullInt32();
		auto opt = l->PullInt32();
		int32_t buflen = 0;
		auto buf = l->PullCString(buflen);
		auto ack = l->PullInt32();
		m_room_mod->doSqlOperation(cid, opt, buf,buflen, ack);
		break;
	}
	case LTOC_SQL_UPDATE_PLAYER_DATA:
	{
		auto pid = l->PullInt32();
		auto table = l->PullInt32();
		auto k1 = l->PullString();
		auto k2 = l->PullString();
		auto pb = l->PullString();

		m_room_mod->updatePlayerData(pid, pb, table, k1, k2);
		break;
	}
	case LTOC_SQL_DELETE_PLAYER_DATA:
	{
		auto pid = l->PullInt32();
		auto table = l->PullInt32();
		auto k1 = l->PullString();
		auto k2 = l->PullString();
		auto pb = l->PullString();

		m_room_mod->deletePlayerData(pid, pb, table, k1, k2);
		break;
	}
	case LTOC_SEND_MSG_TO_PLAYER:
		onLuaSendMsgToPlayer(l);
	default:
		break;
	}
	return 0;
}

void RoomLuaModule::onLuaSendMsgToPlayer(LuaState * l)
{
	auto gateid = l->PullInt32();
	auto uid = l->PullInt32();
	auto mid = l->PullInt32();
	int32_t blen = 0;
	auto buff = l->PullCString(blen);
	m_player_mod->sendToPlayer(gateid,uid,mid,buff,blen);
}

void RoomLuaModule::callLuaMsg(int32_t mid, LuaArgs & arg)
{
	arg.pushArg(mid, 0);
	m_lua_mod->callLuaMsg(arg);
}
