#ifndef EXECUTE_MYSQL_MODULE_H
#define EXECUTE_MYSQL_MODULE_H

#include "BaseModule.h"

class MsgModule;
class MysqlModule;

struct LMsgSqlParam;

class ExecuteMysqlModule:public BaseModule
{
public:
	ExecuteMysqlModule(BaseLayer* l);
	~ExecuteMysqlModule();

private:
	// Í¨¹ý BaseModule ¼Ì³Ð
	virtual void Init() override;
	virtual void AfterInit() override;

	void OnGetMysqlMsg(LMsgSqlParam* msg);
	void OnRequestMysqlMsg(SHARE<BaseMsg>& comsg);

	void OnUpdateTableGroup(NetSocket* num);

protected:
	MsgModule * m_msgModule;
	MysqlModule* m_mysqlModule;

	int m_tableGroup;
};

#endif