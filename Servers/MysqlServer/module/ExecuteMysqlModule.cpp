#include "ExecuteMysqlModule.h"
#include "MsgModule.h"
#include "MysqlModule.h"
#include "RedisModule.h"

#include "mysqlDefine.h"
#include "ServerMsgDefine.h"
#include "CommonDefine.h"

#include "protoPB/server/dbdata.pb.h"

#include "proto_sql.h"
#include "proto_table.h"

#define SQL_BUFF m_sql,(size_t)sizeof(m_sql)

#define TRY_PARSE_SQL_PB(T,msg) \
	T pbMsg; \
	if(!pbMsg.ParseFromArray(msg->m_buff + msg->getOffect() , msg->getUnReadSize())){	\
		LP_ERROR<<"parse "<< #T << "error";	\
	return;}

ExecuteMysqlModule::ExecuteMysqlModule(BaseLayer * l):BaseModule(l)
{
}

ExecuteMysqlModule::~ExecuteMysqlModule()
{
}

std::string ExecuteMysqlModule::getRedisPlayerKey(int64_t & uid, int32_t rid)
{
	std::string res;
	res.append(Loop::Cvto<std::string>(uid)).append(":").append(Loop::Cvto<std::string>(rid));
	return res;
}

std::string ExecuteMysqlModule::getRedisFieldKey(int32_t table, int32_t key1, int32_t key2)
{
	std::string res;
	Int32Struct i32(key2);
	res.append((char*)&i32.bit8, 4).append("_");
	i32.i32 = key1;
	res.append((char*)&i32.bit8, 4).append("_");
	i32.i32 = table;
	res.append((char*)&i32.bit8, 4);
	std::reverse(res.begin(), res.end());
	return res;
}

int32_t ExecuteMysqlModule::getTableIndexFromKey(const std::string & key)
{
	if (key.size() < 4)
		return 0;

	Int32Struct i32;
	i32.bit8[0] = key.at(3);
	i32.bit8[1] = key.at(2);
	i32.bit8[2] = key.at(1);
	i32.bit8[3] = key.at(0);
	return i32.i32;
}

void ExecuteMysqlModule::Init()
{
	m_msgModule = GET_MODULE(MsgModule);
	m_mysqlModule = GET_MODULE(MysqlModule);
	m_redis_mod = GET_MODULE(RedisModule);

	m_msgModule->AddMsgCallBack(L_MYSQL_MSG, this, &ExecuteMysqlModule::OnGetMysqlMsg);
	m_msgModule->AddMsgCallBack(L_MYSQL_CORO_MSG, this, &ExecuteMysqlModule::OnRequestMysqlMsg);

	m_msgModule->AddMsgCall(L_DB_SQL_OPERATION, BIND_CALL(onSqlOperation, SqlOperation));

}

void ExecuteMysqlModule::AfterInit()
{
}

void ExecuteMysqlModule::OnGetMysqlMsg(LMsgSqlParam * msg)
{
	switch (msg->param->opt)
	{
	case SQL_SELECT:
		msg->param->ret = m_mysqlModule->Select(*msg->param);
		break;
	case SQL_INSERT:
		msg->param->ret = m_mysqlModule->Insert(*msg->param);
		break;
	case SQL_DELETE:
		msg->param->ret = m_mysqlModule->Delete(*msg->param);
		break;
	case SQL_UPDATE:
		msg->param->ret = m_mysqlModule->Update(*msg->param);
		break;
	case SQL_INSERT_SELECT:
		msg->param->ret = false;
		if (m_mysqlModule->Insert(*msg->param))
		{
			msg->param->field.clear();
			msg->param->value.clear();
			msg->param->ret = m_mysqlModule->Select(*msg->param);
		}
		break;
	}

	if (msg->index > 0)
	{
		auto lmsg = GET_LAYER_MSG(LMsgSqlParam);
		lmsg->index = msg->index;
		std::swap(lmsg->param, msg->param);
		m_msgModule->SendMsg(LY_LOGIC, 0, L_MYSQL_MSG, lmsg);
	}
}

void ExecuteMysqlModule::OnRequestMysqlMsg(SHARE<BaseMsg>& comsg)
{
	auto msg = (LMsgSqlParam*)comsg->m_data;
	switch (msg->param->opt)
	{
	case SQL_SELECT:
		msg->param->ret = m_mysqlModule->Select(*msg->param);
		break;
	case SQL_INSERT:
		msg->param->ret = m_mysqlModule->Insert(*msg->param);
		break;
	case SQL_DELETE:
		msg->param->ret = m_mysqlModule->Delete(*msg->param);
		break;
	case SQL_UPDATE:
		msg->param->ret = m_mysqlModule->Update(*msg->param);
		break;
	case SQL_INSERT_SELECT:
		msg->param->ret = false;
		if (m_mysqlModule->Insert(*msg->param))
		{
			msg->param->field.clear();
			msg->param->value.clear();
			msg->param->ret = m_mysqlModule->Select(*msg->param);
		}
		break;
	}
	auto lmsg = GET_LAYER_MSG(LMsgSqlParam);
	std::swap(lmsg->param, msg->param);
	m_msgModule->ResponseMsg(comsg, lmsg, LY_LOGIC, 0);
}

void ExecuteMysqlModule::onSqlOperation(SqlOperation * msg)
{
	switch (msg->optId)
	{
	case SOP_ROLE_SELET:opRoleSelect(msg); break;
	case SOP_CREATE_ROLE:opCreateRole(msg); break;
	case SOP_LOAD_PLAYER_DATA:opLoadPlayerData(msg); break;
	default:
		break;
	}
}

void ExecuteMysqlModule::opRoleSelect(SqlOperation * msg)
{
	LPMsg::DB_roleList pb;
	auto& role = *pb.mutable_roles();
	select_lp_player(role, m_mysqlModule, SQL_BUFF, msg->uid);

	auto opt = GET_LAYER_MSG(SqlOperation);
	opt->ackId = msg->ackId;
	opt->path = std::move(msg->path);
	opt->buff = GET_LAYER_MSG(BuffBlock);
	opt->buff->makeRoom(pb.ByteSize() + sizeof(int64_t));
	opt->buff->writeInt64(msg->uid);
	opt->buff->writeProto(pb);

	m_msgModule->SendMsg(L_DB_SQL_OPERATION, opt);
}

void ExecuteMysqlModule::opCreateRole(SqlOperation * msg)
{
	TRY_PARSE_SQL_PB(LPMsg::DB_player, msg->buff);

	LPMsg::DB_player old;
	if (select_lp_player(old, m_mysqlModule, SQL_BUFF, msg->uid,pbMsg.rid()))
	{
		return opRoleSelect(msg);
	}

	if (insert_lp_player(pbMsg, m_mysqlModule, SQL_BUFF))
	{
		opRoleSelect(msg);
	}
}

void ExecuteMysqlModule::opUpdatePlayerData(SqlOperation * msg)
{
	auto rid = msg->buff->readInt32();
	auto table = msg->buff->readInt32();
	auto num = msg->buff->readInt32();

	auto key = getRedisPlayerKey(msg->uid, rid);
	std::vector<std::string> field;
	std::vector<std::string> vals;
	std::vector<std::string> opts;

	for (size_t i = 0; i < num; i++)
	{
		auto key1 = msg->buff->readInt32();
		auto key2 = msg->buff->readInt32();
		int32_t slen = 0;
		auto str = msg->buff->readString(slen);

		field.emplace_back(std::move(getRedisFieldKey(table, key1, key2)));
		vals.emplace_back(str, slen);
		opts.push_back("u");
	}

	m_redis_mod->HMSet(key, field, vals);
	m_redis_mod->HMSet(key + ":opt", field, opts);
}

void ExecuteMysqlModule::opDeletePlayerData(SqlOperation * msg)
{
	auto rid = msg->buff->readInt32();
	auto table = msg->buff->readInt32();
	auto num = msg->buff->readInt32();
	auto key = getRedisPlayerKey(msg->uid, rid);

	std::vector<std::string> field;
	std::vector<std::string> opts;

	for (size_t i = 0; i < num; i++)
	{
		auto key1 = msg->buff->readInt32();
		auto key2 = msg->buff->readInt32();
		field.emplace_back(std::move(getRedisFieldKey(table, key1, key2)));
		opts.push_back("d");
	}

	m_redis_mod->HMSet(key+":opt", field,opts);
}

void ExecuteMysqlModule::opLoadPlayerData(SqlOperation * msg)
{
	auto rid = msg->buff->readInt32();
	auto key = getRedisPlayerKey(msg->uid, rid);

	LPMsg::DB_player_all_data playerdata;
	std::map<std::string, std::string> hval;
	m_redis_mod->HGetAll(key, hval);

	if (hval.empty())
	{
		loadPlayerDataFromDb(playerdata, hval,msg->uid,rid);
		m_redis_mod->HMSet(key, hval);
	}
	else
	{
		loadPlayerDataFromRedis(playerdata, hval);
	}

	auto opt = GET_LAYER_MSG(SqlOperation);
	opt->ackId = msg->ackId;
	opt->path = std::move(msg->path);
	opt->buff = GET_LAYER_MSG(BuffBlock);
	opt->buff->makeRoom(playerdata.ByteSize() + sizeof(int64_t));
	opt->buff->writeInt64(msg->uid);
	opt->buff->writeProto(playerdata);
	m_msgModule->SendMsg(L_DB_SQL_OPERATION, opt);
}

void ExecuteMysqlModule::loadPlayerDataFromDb(LPMsg::DB_player_all_data & pdata, std::map<std::string, std::string>& hval, int64_t& uid, int32_t& rid)
{
	{
		auto pmsg = pdata.mutable_player();
		select_lp_player(*pmsg, m_mysqlModule, SQL_BUFF, uid, rid);
		auto k = getRedisFieldKey(TAB_lp_player, 0, 0);
		hval[k] = std::move(pmsg->SerializeAsString());
	}


}

void ExecuteMysqlModule::loadPlayerDataFromRedis(LPMsg::DB_player_all_data & pdata, std::map<std::string, std::string>& hval)
{
	for (auto& it:hval)
	{
		auto table = getTableIndexFromKey(it.first);

		switch (table)
		{
		case TAB_lp_player:
		{
			auto pmsg = pdata.mutable_player();
			pmsg->ParseFromString(it.second);
		}
			break;
		default:
			break;
		}
	}
}

void ExecuteMysqlModule::savePlayerDataToDb(int64_t uid, int32_t rid)
{
	auto key = getRedisPlayerKey(uid, rid);

	std::map<std::string, std::string> pdata;
	std::map<std::string, std::string> opts;

	m_redis_mod->HGetAll(key, pdata);
	m_redis_mod->HGetAll(key + ":opt", opts);
	
	for (auto& it:opts)
	{
		auto itd = pdata.find(it.first);
		if (itd == pdata.end())
		{
			LP_ERROR << "opt key no data" << it.first;
			continue;
		}

		auto table = getTableIndexFromKey(it.first);

		if (it.second == "u")
		{
			updatePlayerData(table, itd->second);
		}
		else if(it.second == "d")
		{
			deletePlayerData(table, itd->second);
		}
		else
		{
			LP_ERROR << "error opt " << it.second << " key " << it.first;
		}
	}

	m_redis_mod->Del(key + ":opt");
}

#define PB_FROM_STR(t,s) t pb; \
if (!pb.ParseFromString(s)) \
{ \
	LP_ERROR << "save data parse error " << #t; \
	return; \
}

void ExecuteMysqlModule::updatePlayerData(int32_t table, const std::string & str)
{
	switch (table)
	{
	case TAB_lp_player:
	{
		PB_FROM_STR(LPMsg::DB_player, str);
		insert_lp_player(pb, m_mysqlModule, SQL_BUFF);
	}
		break;
	default:
		break;
	}
}

void ExecuteMysqlModule::deletePlayerData(int32_t table, const std::string & str)
{
	switch (table)
	{
	default:
		break;
	}
}

