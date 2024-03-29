#include "SceneModule.h"
#include "TransMsgModule.h"
#include "MsgModule.h"
#include "EventModule.h"
#include "LuaModule.h"
#include "ScheduleModule.h"

#include "lua5.3/mpb.h"

#include "LPFile.h"

#include "ServerMsgDefine.h"

#include "protoPB/server/sroom.pb.h"

SceneModule::SceneModule(BaseLayer * l):BaseModule(l),m_netCallRegIndex(-1), m_gcRegIndex(-1)
{
}

SceneModule::~SceneModule()
{
}

void SceneModule::Init()
{
	m_transModule = GET_MODULE(TransMsgModule);
	m_msgModule = GET_MODULE(MsgModule);
	m_eventModule = GET_MODULE(EventModule);
	m_luaModule = GET_MODULE(LuaModule);
	m_scheduleModule = GET_MODULE(ScheduleModule);


	m_eventModule->AddEventCall(E_SERVER_CONNECT, BIND_EVENT(OnServerConnect,SHARE<NetServer>));
	//m_scheduleModule->AddTimePointTask(this, &SceneModule::OnShowLuaStack, -1);

}

void SceneModule::InitScene()
{
	for (int32_t i = 0; i < MAX_SCENE_SIZE; i++)
	{
		auto scene = GET_SHARE(BattleScene);
		scene->m_id = i + 1;
		m_freeScene[scene->m_id] = scene;
	}
}

void SceneModule::OnShowLuaStack(int64_t & dt)
{
	if (m_luaState)
		m_luaState->printLuaStack();
}

void SceneModule::OnServerConnect(SHARE<NetServer>& ser)
{
	if (ser->type == SERVER_TYPE::LOOP_BATTLE_TRANS)
	{
		m_battle.type = ser->type;
		m_battle.serid = ser->serid;
		SendFreeScene();
	}
}

void SceneModule::OnClearGC(int64_t & dt)
{
	m_luaState->CallRegistFunc(m_gcRegIndex);
}

void SceneModule::SendFreeScene()
{
	if (m_freeScene.size() == 0)
		return;

	LPMsg::AckFreeScene ackmsg;
	for (auto& s : m_freeScene)
	{
		ackmsg.add_scene(s.second->m_id);
	}
	m_transModule->SendToServer(m_battle, N_ACK_BATTLE_FREE_SCENE, ackmsg);
}

int SceneModule::BindLuaNetCall(lua_State * L)
{
	if (lua_gettop(L) - 2 != 1)
		return m_luaState->PushArgs(false);

	if (!lua_isfunction(L, -1))
		return m_luaState->PushArgs(false);

	int32_t ref = luaL_ref(L, LUA_REGISTRYINDEX);
	m_netCallRegIndex = ref;
	return m_luaState->PushArgs(true);
}

int SceneModule::LuaBindNetMsg(const int32_t & mid)
{
	m_msgModule->AddMsgCallBack(mid, this, &SceneModule::OnGetNetMsgToLua);
	return 0;
}

int SceneModule::OnGetNetMsgToLua(NetMsg * msg)
{
	if (m_luaState)
	{
		auto buff = msg->getNetBuff();
		string str(buff, msg->getLen());
		m_luaState->CallRegistFunc(m_netCallRegIndex, msg->mid, str);
	}
	return 0;
}

int SceneModule::SendStreamData(lua_State * L)
{
	return 0;
}

int SceneModule::GetScriptPath()
{
	auto rootp = LoopFile::GetRootPath();
	rootp.append("Servers/BattleScene/script/");
	return m_luaState->PushArgs(rootp);
}

int SceneModule::GetProtoPath()
{
	auto rootp = LoopFile::GetRootPath();
	rootp.append("proto/");
	return m_luaState->PushArgs(rootp);
}

int SceneModule::RunScene(const int32_t & scid)
{
	auto it = m_freeScene.find(scid);
	if (it == m_freeScene.end())
		return 0;

	m_runingScene[it->second->m_id] = it->second;
	m_freeScene.erase(it);
	return 0;
}

int SceneModule::FreeScene(const int32_t & scid)
{
	auto it = m_runingScene.find(scid);
	if (it == m_runingScene.end())
		return 0;

	m_freeScene[it->second->m_id] = it->second;

	LPMsg::AckFreeScene ackmsg;
	ackmsg.add_scene(it->second->m_id);
	m_transModule->SendToServer(m_battle, N_ACK_BATTLE_FREE_SCENE, ackmsg);

	m_runingScene.erase(it);
	return 0;
}


