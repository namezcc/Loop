#ifndef EXECUTE_MYSQL_MODULE_H
#define EXECUTE_MYSQL_MODULE_H

#include "BaseModule.h"

class MsgModule;
class MysqlModule;
class RedisModule;

struct LMsgSqlParam;
struct SqlOperation;

namespace LPMsg
{
	class DB_player_all_data;
}

class ExecuteMysqlModule:public BaseModule
{
public:
	ExecuteMysqlModule(BaseLayer* l);
	~ExecuteMysqlModule();

	std::string getRedisPlayerKey(int32_t& cid);
	std::string getRedisFieldKey(int32_t table);
	std::string getRedisFieldKey(int32_t table, std::string key1);
	std::string getRedisFieldKey(int32_t table, std::string key1, std::string key2);
	std::string getRedisFieldKey(int32_t table, const google::protobuf::RepeatedPtrField<std::string>& keys);
	int32_t getTableIndexFromKey(const std::string& key);

private:
	// 通过 BaseModule 继承
	virtual void Init() override;
	virtual void AfterInit() override;
	void saveAllPlayerChange();

	void OnGetMysqlMsg(LMsgSqlParam* msg);
	void OnRequestMysqlMsg(SHARE<BaseMsg>& comsg);
	void onSqlOperation(SqlOperation* msg);
	void onSaveAllPlayer(SHARE<BaseMsg>& msg);


	SqlOperation* getOperation(SqlOperation* msg, const size_t& len);
	SqlOperation* getPlayerOperation(SqlOperation* msg, const size_t& len);
	void sendOperation(SqlOperation* opt);

	// ------
	void opRoleSelect(SqlOperation * msg);
	void opCreateRole(SqlOperation * msg);
	void opUpdatePlayerData(SqlOperation * msg);
	void opDeletePlayerData(SqlOperation * msg);
	void opLoadPlayerData(SqlOperation * msg);
	void opSearchPlayer(SqlOperation * msg);
	void opSavePlayerToDB(SqlOperation * msg);

	void loadPlayerDataFromDb(LPMsg::DB_player_all_data& pdata, std::map<std::string, std::string>& hval,int32_t& cid);
	void loadPlayerDataFromRedis(LPMsg::DB_player_all_data& pdata, std::map<std::string, std::string>& hval);
	void savePlayerDataToDb(int32_t cid);
	void updatePlayerData(int32_t table, const std::string& str);
	void deletePlayerData(int32_t table, const std::string& str);
protected:
	MsgModule * m_msgModule;
	MysqlModule* m_mysqlModule;
	RedisModule* m_redis_mod;

	int m_tableGroup;
	char m_sql[4096];

};

#endif