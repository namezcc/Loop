#ifndef SESSION_MODULE_H
#define SESSION_MODULE_H

#include "BaseModule.h"

class HttpLogicModule;
class MysqlModule;
class HttpMsg;
class Admin;

struct Session
{
	int64_t id;
	int64_t loseTime;
};
//过期时间 秒
#define SESSION_LOSE_TIME 3600
#define COOKIE_NAME "Loop"

class SessionModule:public BaseModule
{
public:
	SessionModule(BaseLayer* l);
	~SessionModule();

private:
	// 通过 BaseModule 继承
	virtual void Init() override;
	virtual void BeforExecute()override;
	virtual void Execute() override;

	void LoadAdmin();

	bool OnCheckSession(HttpMsg* msg);
	void OnHttpLogin(HttpMsg* msg);

	bool CheckLogin(HttpMsg* msg);

	Session* GetSession(int64_t& sid);

private:
	HttpLogicModule* m_httpModule;
	MysqlModule* m_mysqlModule;

	map<int64_t, SHARE<Session>> m_session;
	map<string, SHARE<Admin>> m_admins;
};

#endif