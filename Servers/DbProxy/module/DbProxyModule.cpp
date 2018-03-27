#include "DbProxyModule.h"
#include "NetObjectModule.h"
#include "MsgModule.h"
#include "EventModule.h"
#include "protoPB/server/LPSql.pb.h"
#include "TransMsgModule.h"
#include "ScheduleModule.h"

DBProxyModule::DBProxyModule(BaseLayer * l):BaseModule(l)
{
}

DBProxyModule::~DBProxyModule()
{
}

void DBProxyModule::Init()
{
	m_netObjModule = GET_MODULE(NetObjectModule);
	m_msgModule = GET_MODULE(MsgModule);
	m_eventModule = GET_MODULE(EventModule);
	m_tranModule = GET_MODULE(TransMsgModule);
	m_scheduleModule = GET_MODULE(ScheduleModule);

	m_scheduleModule->AddInterValTask(this, &DBProxyModule::OnCheckProxy, 1000);
	
	m_eventModule->AddEventCallBack(E_SERVER_CONNECT, this, &DBProxyModule::OnServerConnect);
	m_eventModule->AddEventCallBack(E_SERVER_CLOSE, this, &DBProxyModule::OnServerClose);

	m_msgModule->AddMsgCallBack<NetMsg>(N_GET_MYSQL_GROUP, this, &DBProxyModule::OnGetMysqlGroup);
	m_msgModule->AddMsgCallBack<NetMsg>(N_CREATE_ACCOUNT, this, &DBProxyModule::OnCreateAccount);
	m_msgModule->AddMsgCallBack<NetMsg>(N_MYSQL_MSG, this, &DBProxyModule::OnMysqlMsg);

	m_hash = 0;

	auto myser = GetLayer()->GetServer();
	for (size_t i = 0; i < 4; i++)
	{
		auto node = GetLayer()->GetSharedLoop<ServerNode>();
		m_path.push_back(node);
	}
	//self
	*m_path[1] = *myser;
	//sql
	m_path[3]->serid = 0;
	m_path[3]->type = LOOP_MYSQL;

	for (size_t i = 0; i < 3; i++)
	{
		auto node = GetLayer()->GetSharedLoop<ServerNode>();
		m_getGrouppath.push_back(node);
	}
	*m_getGrouppath[0] = *myser;
	m_getGrouppath[1]->type = LOOP_PROXY_SQL;
	m_getGrouppath[2]->serid = 0;
	m_getGrouppath[2]->type = LOOP_MYSQL;
}

void DBProxyModule::OnServerConnect(SHARE<NetServer>& ser)
{
	if (ser->type != LOOP_PROXY_SQL)
		return;

	m_tmpProxy[ser->socket] = ser;
	/*LPMsg::EmptyPB msg;
	m_netObjModule->SendNetMsg(ser->socket, N_GET_MYSQL_GROUP, msg);*/
}

void DBProxyModule::OnServerClose(SHARE<NetServer>& ser)
{
	if (ser->type != LOOP_PROXY_SQL)
		return;
	auto itg = m_groupMap.find(ser->socket);
	if (itg == m_groupMap.end())
		return;
	else
		m_tmpProxy.erase(ser->socket);

	auto vec = m_proxyGroup[itg->second];
	for (auto it = vec.begin();it!=vec.end();it++)
	{
		if ((*it)->serid == ser->serid)
		{
			vec.erase(it);
			break;
		}
	}
	if (vec.size() == 0)
	{
		for (auto it = m_groups.begin(); it != m_groups.end(); it++)
		{
			if (*it == itg->second)
			{
				m_groups.erase(it);
				break;
			}
		}
	}
	m_groupMap.erase(itg);
}

void DBProxyModule::OnCheckProxy(int64_t & dt)
{
	for (auto& it:m_tmpProxy)
	{
		m_getGrouppath[1]->serid = it.second->serid;
		LPMsg::EmptyPB msg;
		m_tranModule->SendToServer(m_getGrouppath, N_GET_MYSQL_GROUP, msg);
	}
}

void DBProxyModule::OnGetMysqlGroup(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::UpdateTableGroup, msg, m_msgModule);
	auto ser = m_tranModule->GetServerConn(msg->socket);
	if (!ser)
		return;
	auto it = m_tmpProxy.find(msg->socket);
	if (it == m_tmpProxy.end())
		return;
	m_tmpProxy.erase(msg->socket);

	auto snode = GetLayer()->GetSharedLoop<ServerNode>();
	snode->serid = ser->serid;
	snode->type = ser->type;

	m_proxyGroup[pbMsg.groupcount()].push_back(snode);
	m_groupMap[msg->socket] = pbMsg.groupcount();
	if (m_proxyGroup[pbMsg.groupcount()].size() == 1)
		m_groups.push_back(pbMsg.groupcount());
	sort(m_groups.begin(), m_groups.end());
}

void DBProxyModule::OnCreateAccount(NetMsg * msg)
{
	if (msg->len < 4)
		return;
	if (m_groups.size() == 0)
	{
		LP_ERROR(m_msgModule) << "Proxy Db group Error";
		return;
	}
	int group = m_groups[0];
	if (m_groups.size() > 1)
	{
		int crc = *(int*)msg->msg;
		int idx = crc % (m_groups.size() * 2);
		if (idx >= m_groups.size())
			idx = m_groups.size() - 1;
		group = m_groups[idx];
	}
	SendToProxy(group,N_CREATE_ACCOUNT ,msg);
}

void DBProxyModule::OnMysqlMsg(NetMsg * msg)
{
	if (msg->len < 4)
		return;
	int group = *(int*)msg->msg;
	SendToProxy(group, N_MYSQL_MSG, msg);
}

void DBProxyModule::SendToProxy(int group, const int& mid, NetMsg * msg)
{
	auto it = m_proxyGroup.find(group);
	if (it == m_proxyGroup.end())
	{
		LP_ERROR(m_msgModule) << "Proxy DB No group " << group;
		return;
	}
	if (it->second.size() == 0)
	{
		LP_ERROR(m_msgModule) << "Proxy DB group:" << group << " size == 0";
		return;
	}

	++m_hash;
	auto proxy = it->second[m_hash%it->second.size()];

	auto head = m_tranModule->GetServerConn(msg->socket);
	if (!head)
	{
		LP_ERROR(m_msgModule) << "Send To Mysql Error No Head";
		return;
	}

	m_path[0]->serid = head->serid;
	m_path[0]->type = head->type;

	*m_path[2] = *proxy;
	
	auto len = msg->len - sizeof(int);
	char * nmsg = new char[len];
	memcpy(nmsg, msg->msg + 4, len);
	m_tranModule->SendToServer(m_path, mid, nmsg, len,2);
}