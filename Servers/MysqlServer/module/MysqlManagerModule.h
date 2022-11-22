#ifndef MYSQL_MANAGER_MODULE
#define MYSQL_MANAGER_MODULE

#include "BaseModule.h"
#include "protoPB/base/LPSql.pb.h"

class MysqlModule;
class MsgModule;
class TransMsgModule;
class NetObjectModule;
class EventModule;

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

	void onServerConnect(SHARE<NetServer>& ser);

	void onSqlOperation(NetMsg* msg);
	void onSqlOperationRes(SqlOperation* msg);
	int32_t getSendLayerId(int32_t cid);

protected:
	MysqlModule* m_mysqlmodule;
	MsgModule*	m_msgmodule;
	TransMsgModule* m_transModule;
	NetObjectModule* m_net_module;
	EventModule* m_event_mod;

	char m_sql_buff[4096];

	uint32_t m_index;
	int m_sqlLayerNum;
};

#endif // !MYSQL_MANAGER_MODULE