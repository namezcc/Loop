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

#define PLAYER_CHANGE "player_change"

enum DB_OPT
{
	DO_INSTERT = 1,
	DO_UPDATE = 2,
	DO_DELETE = 3,
};

ExecuteMysqlModule::ExecuteMysqlModule(BaseLayer * l):BaseModule(l)
{
}

ExecuteMysqlModule::~ExecuteMysqlModule()
{
}

std::string ExecuteMysqlModule::getRedisPlayerKey(int32_t & cid)
{
	std::string res("player:");
	res.append(Loop::Cvto<std::string>(cid));
	return res;
}

std::string ExecuteMysqlModule::getRedisFieldKey(int32_t table)
{
	return std::to_string(table);
}

std::string ExecuteMysqlModule::getRedisFieldKey(int32_t table, std::string key1)
{
	if (key1 == "") key1 = "0";

	std::string res;
	res.append(Loop::to_string(table));
	res.append("_").append(key1);
	return res;
}

std::string ExecuteMysqlModule::getRedisFieldKey(int32_t table, std::string key1, std::string key2)
{
	if (key1 == "") key1 = "0";
	if (key2 == "") key2 = "0";

	std::string res;
	res.append(Loop::to_string(table));
	res.append("_").append(key1);
	res.append("_").append(key2);
	return res;
}

std::string ExecuteMysqlModule::getRedisFieldKey(int32_t table, const google::protobuf::RepeatedPtrField<std::string>& keys)
{
	auto fk = std::to_string(table);
	for (auto &k : keys)
	{
		if (k.empty())
			fk.append("_0");
		else
			fk.append("_").append(k);
	}
	return fk;
}

int32_t ExecuteMysqlModule::getTableIndexFromKey(const std::string & key)
{
	return Loop::Cvto<int32_t>(key);
}

void ExecuteMysqlModule::Init()
{
	m_msgModule = GET_MODULE(MsgModule);
	m_mysqlModule = GET_MODULE(MysqlModule);
	m_redis_mod = GET_MODULE(RedisModule);

	//m_msgModule->AddMsgCallBack(L_MYSQL_MSG, this, &ExecuteMysqlModule::OnGetMysqlMsg);
	//m_msgModule->AddMsgCallBack(L_MYSQL_CORO_MSG, this, &ExecuteMysqlModule::OnRequestMysqlMsg);

	m_msgModule->AddMsgCall(L_DB_SQL_OPERATION, BIND_CALL(onSqlOperation, SqlOperation));
	m_msgModule->AddMsgCall(L_DB_SAVE_ALL_PLAYER, BIND_SHARE_CALL(onSaveAllPlayer));
	

}

void ExecuteMysqlModule::AfterInit()
{
	
}

void ExecuteMysqlModule::saveAllPlayerChange()
{
	vec_str vec;
	m_redis_mod->SMembers(PLAYER_CHANGE, vec);
	if (vec.empty())
		return;

	auto start = Loop::GetMilliSecend();
	LP_WARN << "satrt save All change player tm:" << start;

	for (size_t i = 0; i < vec.size(); i++)
	{
		auto cid = Loop::Cvto<int32_t>(vec[i]);
		savePlayerDataToDb(cid);
	}

	LP_WARN << "save All change player use tm:" << (Loop::GetMilliSecend() - start) << " over ..............";
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
	case SOP_SEARCH_PLAYER:opSearchPlayer(msg); break;
	case SOP_UPDATE_PLAYER_DATA:opUpdatePlayerData(msg); break;
	case SOP_SAVE_PlAYER_TO_DB:opSavePlayerToDB(msg); break;
		
	default:
		break;
	}
}

void ExecuteMysqlModule::onSaveAllPlayer(SHARE<BaseMsg>& msg)
{
	saveAllPlayerChange();
}

SqlOperation * ExecuteMysqlModule::getOperation(SqlOperation * msg, const size_t & len)
{
	auto opt = GET_LAYER_MSG(SqlOperation);
	opt->ackId = msg->ackId;
	opt->server_sock = msg->server_sock;
	opt->buff = GET_LAYER_MSG(BuffBlock);
	opt->buff->makeRoom(len);
	return opt;
}

SqlOperation * ExecuteMysqlModule::getPlayerOperation(SqlOperation * msg, const size_t & len)
{
	auto opt = GET_LAYER_MSG(SqlOperation);
	opt->ackId = msg->ackId;
	opt->server_sock = msg->server_sock;
	opt->buff = GET_LAYER_MSG(BuffBlock);
	opt->buff->makeRoom(len + sizeof(int64_t) + 2*sizeof(int32_t));
	opt->buff->writeInt64(msg->cid);
	opt->buff->writeInt32(msg->optId);
	opt->buff->writeInt32(msg->ackId);
	return opt;
}

void ExecuteMysqlModule::sendOperation(SqlOperation * opt)
{
	m_msgModule->SendMsg(L_DB_SQL_OPERATION, opt);
}

void ExecuteMysqlModule::opRoleSelect(SqlOperation * msg)
{
	LPMsg::DB_roleList pb;
	auto& role = *pb.mutable_roles();
	select_lp_player(role, m_mysqlModule, SQL_BUFF, msg->cid);

	auto opt = GET_LAYER_MSG(SqlOperation);
	opt->ackId = msg->ackId;
	opt->server_sock = msg->server_sock;
	opt->buff = GET_LAYER_MSG(BuffBlock);
	opt->buff->makeRoom(pb.ByteSize() + sizeof(int64_t));
	opt->buff->writeInt32(msg->cid);
	opt->buff->writeProto(pb);

	m_msgModule->SendMsg(L_DB_SQL_OPERATION, opt);
}

void ExecuteMysqlModule::opCreateRole(SqlOperation * msg)
{
	TRY_PARSE_SQL_PB(LPMsg::DB_player, msg->buff);

	LPMsg::DB_player old;
	if (select_lp_player(old, m_mysqlModule, SQL_BUFF, msg->cid))
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
	TRY_PARSE_SQL_PB(LPMsg::DB_sql_data_list, msg->buff);
	auto key = getRedisPlayerKey(msg->cid);

	if (m_redis_mod->haveKey(key))
	{
		std::vector<std::string> field;
		std::vector<std::string> optfield;
		std::vector<std::string> vals;
		std::vector<std::string> opts;

		for (auto& val : pbMsg.datas())
		{
			auto fk = getRedisFieldKey(val.table(), val.keys());
			if (val.opt() == DO_DELETE)
			{
				optfield.emplace_back(fk);
				opts.emplace_back("d");
			}
			else
			{
				field.emplace_back(fk);
				vals.emplace_back(val.data());
				optfield.emplace_back(fk);
				opts.emplace_back("u");
			}
		}
		m_redis_mod->HMSet(key, field, vals);
		m_redis_mod->HMSet("opt:" + key, optfield, opts);
		m_redis_mod->SAdd(PLAYER_CHANGE, Loop::to_string(msg->cid));
	}
	else
	{
		for (auto& val : pbMsg.datas())
		{
			if (val.opt() == DO_UPDATE || val.opt() == DO_INSTERT)
				updatePlayerData(val.table(), val.data());
			else
				deletePlayerData(val.table(), val.data());
		}
	}
}

void ExecuteMysqlModule::opDeletePlayerData(SqlOperation * msg)
{
	auto table = msg->buff->readInt32();
	auto num = msg->buff->readInt32();
	auto key = getRedisPlayerKey(msg->cid);

	std::vector<std::string> field;
	std::vector<std::string> vals;
	std::vector<std::string> opts;

	for (size_t i = 0; i < num; i++)
	{
		auto key1 = msg->buff->readString();
		auto key2 = msg->buff->readString();
		int32_t slen = 0;
		auto str = msg->buff->readString(slen);

		field.emplace_back(std::move(getRedisFieldKey(table, key1, key2)));
		vals.emplace_back(str, slen);
		opts.push_back("d");
	}

	if (m_redis_mod->haveKey(key))
	{
		m_redis_mod->HMSet("opt:" + key, field,opts);
		m_redis_mod->SAdd(PLAYER_CHANGE, Loop::to_string(msg->cid));
	}
	else
	{
		for (size_t i = 0; i < field.size(); i++)
		{
			auto table = getTableIndexFromKey(field[i]);
			deletePlayerData(table, vals[i]);
		}
	}
}

void ExecuteMysqlModule::opLoadPlayerData(SqlOperation * msg)
{
	auto key = getRedisPlayerKey(msg->cid);

	if (m_redis_mod->haveKey("opt:" + key))
	{
		savePlayerDataToDb(msg->cid);
	}

	LPMsg::DB_player_all_data playerdata;
	std::map<std::string, std::string> hval;
	m_redis_mod->HGetAll(key, hval);

	if (hval.empty())
	{
		loadPlayerDataFromDb(playerdata, hval,msg->cid);
		m_redis_mod->HMSet(key, hval);
	}
	else
	{
		loadPlayerDataFromRedis(playerdata, hval);
	}

	auto opt = GET_LAYER_MSG(SqlOperation);
	opt->ackId = msg->ackId;
	opt->server_sock = msg->server_sock;
	opt->buff = GET_LAYER_MSG(BuffBlock);
	opt->buff->makeRoom(playerdata.ByteSize());
	opt->buff->writeProto(playerdata);
	m_msgModule->SendMsg(L_DB_SQL_OPERATION, opt);
}
void ExecuteMysqlModule::opSearchPlayer(SqlOperation * msg)
{
	auto cid = msg->buff->readInt32();

	auto key = getRedisPlayerKey(cid);
	auto fkey = getRedisFieldKey(TAB_lp_player);
	std::string redisdata;
	m_redis_mod->HGet(key, fkey, redisdata);

	LPMsg::DB_player player;

	if (redisdata.empty())
	{
		select_lp_player(player, m_mysqlModule, SQL_BUFF,cid);
	}
	else
	{
		player.ParseFromString(redisdata);
	}

	auto opt = getPlayerOperation(msg, player.ByteSize());
	opt->buff->writeProto(player);
	sendOperation(opt);
}

void ExecuteMysqlModule::opSavePlayerToDB(SqlOperation * msg)
{
	savePlayerDataToDb(msg->cid);
}
//每行sql数据作为每个hashval
void ExecuteMysqlModule::loadPlayerDataFromDb(LPMsg::DB_player_all_data & pdata, std::map<std::string, std::string>& hval, int32_t& cid)
{
	{
		auto pmsg = pdata.mutable_player();
		select_lp_player(*pmsg, m_mysqlModule, SQL_BUFF, cid);
		auto k = getRedisFieldKey(TAB_lp_player);
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

void ExecuteMysqlModule::savePlayerDataToDb(int32_t cid)
{
	auto key = getRedisPlayerKey(cid);

	std::map<std::string, std::string> pdata;
	std::map<std::string, std::string> opts;
	vec_str dels;

	m_redis_mod->HGetAll(key, pdata);
	m_redis_mod->HGetAll("opt:" + key, opts);
	
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
			dels.push_back(itd->first);
		}
		else
		{
			LP_ERROR << "error opt " << it.second << " key " << it.first;
		}
	}

	if(!dels.empty())
		m_redis_mod->HMDel(key, dels);

	m_redis_mod->Del("opt:" + key);
	m_redis_mod->SRem(PLAYER_CHANGE, Loop::to_string(cid));
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
		update_lp_player(pb, m_mysqlModule, SQL_BUFF);
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

