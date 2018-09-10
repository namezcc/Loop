#include "LoginLockModule.h"
#include "MsgModule.h"
#include "NetObjectModule.h"
#include "ScheduleModule.h"
#include "protoPB/server/server.pb.h"

LoginLockModule::LoginLockModule(BaseLayer * l):BaseModule(l)
{
}

LoginLockModule::~LoginLockModule()
{
}

void LoginLockModule::Init()
{
	m_msgModule = GET_MODULE(MsgModule);
	m_netObjModule = GET_MODULE(NetObjectModule);
	m_schedulModule = GET_MODULE(ScheduleModule);

	m_msgModule->AddMsgCallBack(N_LOGIN_LOCK, this, &LoginLockModule::OnLoginLock);
	m_msgModule->AddMsgCallBack(N_LOGIN_UNLOCK, this, &LoginLockModule::OnLoginUnlock);

	m_schedulModule->AddTimePointTask(this, &LoginLockModule::CheckOutTime, -1, 0);
}

void LoginLockModule::OnLoginLock(SHARE<BaseMsg>& msg)
{
	auto netmsg = (NetMsg*)msg->m_data;
	TRY_PARSEPB(LPMsg::LoginLock, netmsg,m_msgModule);

	auto it = m_lockPid.find(pbMsg.pid());
	auto now = GetSecend();
	if (it == m_lockPid.end() || it->second < now)
	{
		m_netObjModule->ResponseMsg(netmsg->socket, msg, pbMsg);
		m_lockPid[pbMsg.pid()] = now + LOGIN_LOCK_OUT_TIME;
	}
	else
	{
		pbMsg.set_pid(0);
		m_netObjModule->ResponseMsg(netmsg->socket, msg, pbMsg);
	}
}

void LoginLockModule::OnLoginUnlock(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::LoginLock, msg, m_msgModule);
	m_lockPid.erase(pbMsg.pid());
}

void LoginLockModule::CheckOutTime(int64_t & dt)
{
	auto now = dt / 1000;
	for (auto it = m_lockPid.begin();it!= m_lockPid.end();)
	{
		if (it->second > now)
			m_lockPid.erase(it++);
		else
			it++;
	}
}
