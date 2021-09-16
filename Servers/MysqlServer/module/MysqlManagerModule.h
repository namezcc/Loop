#ifndef MYSQL_MANAGER_MODULE
#define MYSQL_MANAGER_MODULE

#include "BaseModule.h"
#include "protoPB/base/LPSql.pb.h"

class MysqlModule;
class MsgModule;
class TransMsgModule;

struct LMsgSqlParam;
struct SqlOperation;

namespace LPMsg
{
	class PBSqlParam;
}

struct SqlReply:public LoopObject
{
	LPMsg::PBSqlParam pbMsg;
	ServerPath path;

	// 通过 LoopObject 继承
	virtual void init(FactorManager * fm) override {};
	virtual void recycle(FactorManager * fm) override
	{
		path.clear();
	};
};

class MysqlManagerModule:public BaseModule
{
public:
	MysqlManagerModule(BaseLayer* l);
	~MysqlManagerModule();

private:
	// 通过 BaseModule 继承
	virtual void Init() override;
	virtual void AfterInit() override;
	virtual void BeforExecute() override;

	void OnGetAccountInfo(NetMsg* msg, c_pull& pull, SHARE<BaseCoro>& coro);
	void onSqlOperation(NetServerMsg* msg);
	void onSqlOperationRes(SqlOperation* msg);
	int32_t getSendLayerId(int64_t uid);

protected:
	MysqlModule* m_mysqlmodule;
	MsgModule*	m_msgmodule;
	TransMsgModule* m_transModule;

	char m_sql_buff[4096];
	ServerNode m_lock_server;

	uint32_t m_index;
	int m_sqlLayerNum;
};

#endif // !MYSQL_MANAGER_MODULE