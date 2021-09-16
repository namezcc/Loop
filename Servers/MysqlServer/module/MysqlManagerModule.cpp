#include "MysqlManagerModule.h"
#include "GameReflectData.h"
#include "MysqlModule.h"
#include "MsgModule.h"
#include "TransMsgModule.h"
#include "help_function.h"

#include "ServerMsgDefine.h"
#include "mysqlDefine.h"

#include "protoPB/client/login.pb.h"
#include "protoPB/server/dbdata.pb.h"

#include "proto_sql.h"

#include <algorithm>

#define SQL_BUFF m_sql_buff, sizeof(m_sql_buff)

MysqlManagerModule::MysqlManagerModule(BaseLayer * l):BaseModule(l)
{

}

MysqlManagerModule::~MysqlManagerModule()
{
}

void MysqlManagerModule::Init()
{
	m_mysqlmodule = GET_MODULE(MysqlModule);
	m_msgmodule = GET_MODULE(MsgModule);
	m_transModule = GET_MODULE(TransMsgModule);

	
	m_msgmodule->AddMsgCall(N_TDB_SQL_OPERATION, BIND_SERVER_MSG(onSqlOperation));
	m_msgmodule->AddMsgCall(L_DB_SQL_OPERATION, BIND_CALL(onSqlOperationRes, SqlOperation));

	m_msgmodule->AddAsynMsgCall(N_GET_ACCOUNT_INFO, BIND_ASYN_NETMSG(OnGetAccountInfo));
	

	m_lock_server.type = SERVER_TYPE::LOOP_LOGIN_LOCK;
	m_lock_server.serid = 0;

	m_index = 0;
	auto it = GetLayer()->GetPipes().find(LY_MYSQL);
	if (it != GetLayer()->GetPipes().end())
		m_sqlLayerNum = (int32_t)it->second.size();
	else
		m_sqlLayerNum = 0;
}

void MysqlManagerModule::AfterInit()
{
	if(GetLayer()->GetServer()->type == LOOP_MYSQL)
		m_msgmodule->SendMsg(LY_MYSQL, 0, L_DB_SAVE_ALL_PLAYER, NULL);
}

void MysqlManagerModule::BeforExecute()
{
	
}

void MysqlManagerModule::OnGetAccountInfo(NetMsg* msg, c_pull& pull, SHARE<BaseCoro>& coro)
{
	TRY_PARSEPB(LPMsg::ReqLogin, msg);
	SHARE<BaseMsg> frommsg = pull.get();

	LPMsg::DB_account pbaccount;
	select_account(pbaccount, m_mysqlmodule, m_sql_buff, sizeof(m_sql_buff), pbMsg.account());

	if (pbaccount.platform_uid().empty())
	{
		auto ackmsg = m_transModule->RequestServerAsynMsg(m_lock_server, N_GET_DBINDEX, LPMsg::EmptyPB{}, pull, coro);
		auto pack = ackmsg->m_buff;

		auto dbindex = pack->readInt32();
		auto uid = pack->readInt64();
		auto hashindex = getPlayerHashIndex(pbMsg.account());

		pbaccount.set_platform_uid(pbMsg.account());
		pbaccount.set_dbindex(dbindex);
		pbaccount.set_game_uid(uid);
		pbaccount.set_hash_index(hashindex);

		if (insert_account(pbaccount, m_mysqlmodule, SQL_BUFF))
		{
			auto sendpack = GET_LAYER_MSG(BuffBlock);
			sendpack->writeInt32(dbindex);
			m_transModule->SendToServer(m_lock_server, N_ADD_DBINDEX_NUM, sendpack);
		}
	}

	auto pathmsg = dynamic_cast<NetServerMsg*>(msg);
	if(pathmsg)
		m_transModule->ResponseBackServerMsg(pathmsg->path, frommsg, pbaccount);
	else
	{
		LP_ERROR << "OnGetAccountInfo dynamic_cast<NetServerMsg*> error ";
	}
}

void MysqlManagerModule::onSqlOperation(NetServerMsg * msg)
{
	auto pack = msg->m_buff;
	auto opt = GET_LAYER_MSG(SqlOperation);


	opt->optId = pack->readInt32();
	opt->ackId = pack->readInt32();
	opt->pid = pack->readInt64();
	opt->buff = GET_LAYER_MSG(BuffBlock);
	int32_t bufsize;
	auto msgbuff = pack->readBuff(bufsize);
	opt->buff->writeBuff(msgbuff, bufsize);

	opt->path = std::move(msg->path);

	auto sendid = getSendLayerId(opt->pid);

	opt->buff->setOffect(0);
	m_msgmodule->SendMsg(LY_MYSQL, sendid, L_DB_SQL_OPERATION, opt);
}

void MysqlManagerModule::onSqlOperationRes(SqlOperation * msg)
{
	BuffBlock* spack = NULL;
	std::swap(spack, msg->buff);

	m_transModule->SendBackServer(msg->path, msg->ackId, spack);
}

int32_t MysqlManagerModule::getSendLayerId(int64_t uid)
{
	return uid %m_sqlLayerNum;
}
