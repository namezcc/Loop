#include "LoginModule.h"
#include "NetObjectModule.h"
#include "MsgModule.h"
#include "Crypto/crchash.h"
#include "GameReflectData.h"
#include "MysqlModule.h"
#include "protoPB/server/LPSql.pb.h"
#include "SendProxyDbModule.h"
#include "RedisModule.h"
#include "protoPB/client/login.pb.h"
#include "protoPB/client/define.pb.h"


LoginModule::LoginModule(BaseLayer* l):BaseModule(l)
{
}

LoginModule::~LoginModule()
{
}

void LoginModule::Init()
{
	m_msgModule= GET_MODULE(MsgModule);
	m_netModule= GET_MODULE(NetObjectModule);
	m_sendProxyDb = GET_MODULE(SendProxyDbModule);
	m_redisModule = GET_MODULE(RedisModule);

	m_msgModule->AddMsgCallBack<NetMsg>(LPMsg::N_REQ_LOGIN, this, &LoginModule::OnClientLogin);
	m_msgModule->AddMsgCallBack<NetMsg>(N_ML_GET_ACCOUNT, this, &LoginModule::OnGetAccountInfo);
	m_msgModule->AddMsgCallBack<NetMsg>(N_ML_CREATE_ACCOUNT, this, &LoginModule::OnCreateAccount);
}

void LoginModule::OnClientLogin(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::ReqLogin, msg, m_msgModule);

	string sid;
	m_redisModule->HGet("Account:" + pbMsg.account(), "id", sid);
	if (sid.empty())
	{
		CreateAccount(pbMsg.account(), pbMsg.password());
		return;
	}

	int64_t id = loop_cast<int64_t>(sid);

	LPMsg::PBSqlParam param;
	param.set_uid(id);
	param.set_opt(SQL_OPT::SQL_SELECT);
	param.set_table(Reflect<AccoutInfo>::Name());
	param.set_reply(N_ML_GET_ACCOUNT);

	param.add_kname("id");
	param.add_kval(sid);

	m_sendProxyDb->SendToProxyDb(pbMsg, id >> 56, N_CREATE_ACCOUNT);

	auto client = GET_SHARE(ClientObj);
	
	client->sock = msg->socket;
	client->pass = pbMsg.password();
	m_tmpClient[pbMsg.account()] = client;
}

void LoginModule::OnGetAccountInfo(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::PBSqlParam, msg, m_msgModule);

	vector<string> field(pbMsg.field().begin(), pbMsg.field().end());
	vector<string> value(pbMsg.value().begin(), pbMsg.value().end());

	SHARE<AccoutInfo> account = m_sendProxyDb->CreateObject<AccoutInfo>(field,value);

	auto it = m_tmpClient.find(account->name);
	if (it == m_tmpClient.end())
		return;

	if (it->second->pass == account->pass)
		LP_WARN(m_msgModule) << "Account:" << account->name << "  Login success";
	else
		LP_WARN(m_msgModule)<< "Account:" << account->name << "  Login pass Error";
	m_tmpClient.erase(it);
}

void LoginModule::CreateAccount(const string & name, const string & pass)
{
	int hash = common::Hash32(name);

	LPMsg::PBSqlParam param;

	param.set_uid(hash);
	param.set_opt(SQL_OPT::SQL_INSERT_SELECT);
	param.set_reply(N_ML_CREATE_ACCOUNT);

	param.set_table(Reflect<AccoutInfo>::Name());

	param.add_field("name");
	param.add_value(name);
	param.add_field("pass");
	param.add_value(pass);

	param.add_kname("name");
	param.add_kval(name);

	m_sendProxyDb->SendToProxyDb(param, hash, N_CREATE_ACCOUNT);
	LP_WARN(m_msgModule) << "Account:" << name << "Begin Create";
}

void LoginModule::OnCreateAccount(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::PBSqlParam, msg, m_msgModule);

	vector<string> field(pbMsg.field().begin(), pbMsg.field().end());
	vector<string> value(pbMsg.value().begin(), pbMsg.value().end());

	SHARE<AccoutInfo> account = m_sendProxyDb->CreateObject<AccoutInfo>(field, value);

	m_redisModule->HMSet("Account:" + account->name, field, value);

	LP_WARN(m_msgModule) << "Account:" << account->name << " Create Success";
}