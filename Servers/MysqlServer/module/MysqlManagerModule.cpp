#include "MysqlManagerModule.h"
//#include "GameReflectData.h"
#include "MysqlModule.h"
#include "MsgModule.h"
#include "TransMsgModule.h"
#include "NetObjectModule.h"
#include "EventModule.h"

#include "help_function.h"

#include "ServerMsgDefine.h"
#include "mysqlDefine.h"

#include "protoPB/client/client.pb.h"
#include "protoPB/server/dbdata.pb.h"
#include "protoPB/server/server_msgid.pb.h"

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
	m_net_module = GET_MODULE(NetObjectModule);
	m_event_mod = GET_MODULE(EventModule);

	m_event_mod->AddEventCall(E_SERVER_CONNECT, BIND_EVENT(onServerConnect, SHARE<NetServer>&));
	
	m_msgmodule->AddMsgCall(LPMsg::IM_DB_SQL_OPERATION, BIND_NETMSG(onSqlOperation));
	m_msgmodule->AddMsgCall(L_DB_SQL_OPERATION, BIND_CALL(onSqlOperationRes, SqlOperation));


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

void MysqlManagerModule::onServerConnect(SHARE<NetServer>& ser)
{
	if (ser->type == LOOP_ROOM)
	{
		auto conf = getLoopServer()->GetConfig();
		auto pack = LAYER_BUFF;
		pack->writeInt32(conf.sql.id);
		pack->writeInt32(conf.addr.serid);
		m_net_module->SendNetMsg(ser->socket, LPMsg::IM_ROOM_REG_DBID, pack);
	}
}

void MysqlManagerModule::onSqlOperation(NetMsg * msg)
{
	auto pack = msg->m_buff;
	auto opt = GET_LAYER_MSG(SqlOperation);

	opt->optId = pack->readInt32();
	opt->ackId = pack->readInt32();
	opt->cid = pack->readInt32();
	opt->buff = GET_LAYER_MSG(BuffBlock);
	int32_t bufsize;
	auto msgbuff = pack->readBuff(bufsize);
	opt->buff->writeBuff(msgbuff, bufsize);

	opt->server_sock = std::move(msg->socket);

	auto sendid = getSendLayerId(opt->cid);

	opt->buff->setOffect(0);
	m_msgmodule->SendMsg(LY_MYSQL, sendid, L_DB_SQL_OPERATION, opt);
}

void MysqlManagerModule::onSqlOperationRes(SqlOperation * msg)
{
	BuffBlock* spack = NULL;
	std::swap(spack, msg->buff);

	m_net_module->SendNetMsg(msg->server_sock, msg->ackId, spack);
}

int32_t MysqlManagerModule::getSendLayerId(int32_t cid)
{
	return cid %m_sqlLayerNum;
}
