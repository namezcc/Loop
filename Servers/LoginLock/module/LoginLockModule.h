#ifndef LOGIN_LOCK_MODULE_H
#define LOGIN_LOCK_MODULE_H

#include "BaseModule.h"

class MsgModule;
class NetObjectModule;
class ScheduleModule;

#define LOGIN_LOCK_OUT_TIME 5	//µ•Œª√Î

class LoginLockModule:public BaseModule
{
public:
	LoginLockModule(BaseLayer* l);
	~LoginLockModule();

protected:
	virtual void Init() override;

	void OnLoginLock(SHARE<BaseMsg>& msg);
	void OnLoginUnlock(NetMsg* msg);

	void CheckOutTime(int64_t& dt);
private:
	MsgModule * m_msgModule;
	NetObjectModule* m_netObjModule;
	ScheduleModule* m_schedulModule;

	std::unordered_map<int64_t, int64_t> m_lockPid;
		
};

#endif
