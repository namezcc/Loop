#ifndef LOGIN_LOCK_MODULE_H
#define LOGIN_LOCK_MODULE_H

#include "BaseModule.h"
#include "protoPB/server/dbdata.pb.h"

class MsgModule;
class NetObjectModule;
class ScheduleModule;
class MysqlModule;
class TransMsgModule;

#define LOGIN_LOCK_OUT_TIME 5	//单位秒
#define MAX_PLAYER_NUM_PER_DB 10000

class LoginLockModule:public BaseModule
{
public:
	LoginLockModule(BaseLayer* l);
	~LoginLockModule();

protected:
	virtual void Init() override;
	virtual void AfterInit() override;

	void onPlayerLogin(SHARE<BaseMsg>& msg);

	void OnLoginLock(SHARE<BaseMsg>& msg);
	void OnLoginUnlock(NetMsg* msg);

	void onGetDbIndex(SHARE<BaseMsg>& msg);
	void onDbIndexAddNum(NetMsg* msg);

	void onNormalTest(NetMsg* msg);
	void onAsyncTest(NetMsg* msg, c_pull & pull, SHARE<BaseCoro>& coro);

	void CheckOutTime(int64_t& dt);
	void showTestNum(int64_t& dt);

	void loadDbPlayerNum();
	int32_t getDbIndex();
	int32_t genPlayerUid(const std::string& uuid);
private:
	MsgModule * m_msgModule;
	NetObjectModule* m_netObjModule;
	ScheduleModule* m_schedulModule;
	MysqlModule* m_mysql_module;
	TransMsgModule* m_trans_mod;

	int64_t m_now_stamp;
	int32_t m_add_num;

	int32_t m_pp_num;
	int64_t m_start_time;

	std::unordered_map<int64_t, int64_t> m_lockPid;
	std::vector<std::pair<int32_t, int32_t>> m_db_player_num;
	std::vector<LPMsg::DB_player_num_info> m_db_player_num_info;
	std::map<int32_t, size_t> m_id_index;
	std::map<std::string, int32_t> m_account_info;
	std::set<int32_t> m_uid_check;

	char m_sql_buff[4096];
};

#endif
