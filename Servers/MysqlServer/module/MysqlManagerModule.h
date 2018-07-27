#ifndef MYSQL_MANAGER_MODULE
#define MYSQL_MANAGER_MODULE

#include "BaseModule.h"
#include "protoPB/server/LPSql.pb.h"

class MysqlModule;
class MsgModule;
class TransMsgModule;

struct LMsgSqlParam;

namespace LPMsg
{
	class PBSqlParam;
}

struct SqlReply:public LoopObject
{
	LPMsg::PBSqlParam pbMsg;
	vector<SHARE<ServerNode>> path;

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
	
	void InitTableGroupNum();
	void CreateMysqlTable(int group = 1);

	void OnGetGroupId(NetServerMsg* msg);

	void OnCreateAccount(NetServerMsg* msg);
	void OnGetMysqlMsg(NetServerMsg* msg);
	void OnGetMysqlRes(LMsgSqlParam* msg);
	void OnUpdateTableGroup(NetMsg* msg);
	void OnAddTableGroup(NetMsg* msg);

	void SendSqlReply(SHARE<SqlReply>& reply, const int& gid);
	int GetSendLayerId();


protected:
	MysqlModule* m_mysqlmodule;
	MsgModule*	m_msgmodule;
	TransMsgModule* m_transModule;
	uint32_t m_index;
	int m_sqlLayerNum;

	map<uint32_t, SHARE<SqlReply>> m_replay;

	int m_tableGroup;		//table group num
};

#endif // !MYSQL_MANAGER_MODULE