#include "LuaModule.h"

#include "MsgModule.h"
#include "NetObjectModule.h"
#include "TransMsgModule.h"

LuaModule::LuaModule(BaseLayer * l):BaseModule(l)
{
}

LuaModule::~LuaModule()
{
}

void LuaModule::Init()
{
	COM_MOD_INIT;

	CreateLuaState();
	m_curState->initCToLIndex(CTOL_NONE, CTOL_MAX);
	m_curState->initLToCIndex(LTOC_NONE,LTOC_MAX);
	//m_curState->bindLToCFunc(LTOC_SEND_MSG, AnyFuncBind::Bind(&LuaModule::onLuaSendMsg,this));
	m_curState->bindLToCFunc(LTOC_SEND_MSG, std::bind(&LuaModule::onLuaSendMsg, this, HOLD_1));
	m_curState->bindLToCFunc(LTOC_SEND_SERVER, std::bind(&LuaModule::onLuaSendServerMsg, this, HOLD_1));
}

void LuaModule::Execute()
{
	auto dt = Loop::GetMilliSecend();
	for (auto& ls:m_stats)
	{
		ls->Run(dt);
	}

	if (!m_buff_send_cash.empty())
	{
		LP_WARN << "buff cash need recycle size:" << m_buff_send_cash.size();
		for (auto p:m_buff_send_cash)
		{
			RECYCLE_LAYER_MSG(p);
		}
		m_buff_send_cash.clear();
	}
}

void LuaModule::runScript(const std::string & f)
{
	m_curState->RunScript(f);
}

void LuaModule::setUpdateFunc(const std::string & f)
{
	m_curState->RegistGlobalFunc(f);
}

void LuaModule::callGlobalFunc(const std::string& f, LuaArgs& arg)
{
	m_curState->callGloableFunc(f, arg);
}

void LuaModule::setLuaCallFunc(const std::function<int32_t(int32_t, LuaState*)>& f)
{
	m_curState->setLuaCallFunc(f);
}

bool LuaModule::callLuaFunc(int32_t findex, LuaArgs & arg)
{
	return m_curState->callLuaFunc(findex,arg);
}

bool LuaModule::callLuaMsg(LuaArgs & arg)
{
	return m_curState->callLuaFunc(CTOL_MSG,arg);
}

void LuaModule::addCashSendBuff(BuffBlock * b)
{
	m_buff_send_cash.insert(b);
}

void LuaModule::removeSendBuff(BuffBlock * b)
{
	m_buff_send_cash.erase(b);
}

LuaState* LuaModule::CreateLuaState()
{
	auto state = GET_SHARE(LuaState);
	state->Init(this);
	m_stats.push_back(state);
	m_curState = state.get();
	return m_curState;
}

void LuaModule::onNetMsg(NetMsg * msg)
{
	LuaArgs arg;

	arg.pushArg(msg->mid);
	arg.pushArg(msg->socket);
	arg.pushArg(msg);

	m_curState->callLuaFunc(CTOL_NET_MSG, arg);
}

int LuaModule::onLuaSendMsg(LuaState * l)
{
	auto sock = l->PullInt32();
	auto mid = l->PullInt32();
	
	auto pack = (BuffBlock*)l->PullUserData();
	if (pack == NULL)
	{
		LP_ERROR << "onLuaSendMsg pack = NULL";
		return 0;
	}

	removeSendBuff(pack);
	m_net_mod->SendNetMsg(sock, mid, pack);
	return 0;
}

int LuaModule::onLuaSendServerMsg(LuaState * l)
{
	LuaTValue<std::vector<int32_t>> path;
	path.pullValue(l->GetLuaState(), l->nextArgIndex());

	if (path.m_val.empty() || path.m_val.size() % 2 != 0)
	{
		LP_ERROR << "onLuaSendServerMsg path error";
		return 0;
	}

	m_send_path.clear();
	m_send_path.push_back(*GetLayer()->GetServer());
	ServerNode node = {};
	for (size_t i = 0; i < path.m_val.size(); i++)
	{
		node.type = path.m_val[i++];
		node.serid = path.m_val[i];
		m_send_path.push_back(node);
	}

	auto mid = l->PullInt32();
	
	auto pack = (BuffBlock*)l->PullUserData();
	if (pack == NULL)
	{
		LP_ERROR << "onLuaSendServerMsg pack = NULL";
		return 0;
	}

	removeSendBuff(pack);
	m_trans_mod->SendToServer(m_send_path, mid, pack);
	return 0;
}


