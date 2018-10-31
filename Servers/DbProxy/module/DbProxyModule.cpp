#include "DbProxyModule.h"
#include "NetObjectModule.h"
#include "MsgModule.h"
#include "EventModule.h"
#include "protoPB/base/LPSql.pb.h"
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

	m_msgModule->AddMsgCallBack(N_GET_MYSQL_GROUP, this, &DBProxyModule::OnGetMysqlGroup);
	m_msgModule->AddMsgCallBack(N_FORWARD_DB_PROXY, this, &DBProxyModule::OnForwardMsgHash);
	m_msgModule->AddMsgCallBack(N_FORWARD_DB_PROXY_GROUP, this, &DBProxyModule::OnForwardMsgGroup);

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
	m_getGrouppath[1]->type = LOOP_PROXY;
	m_getGrouppath[2]->serid = 0;
	m_getGrouppath[2]->type = LOOP_MYSQL;
}

void DBProxyModule::OnServerConnect(SHARE<NetServer>& ser)
{
	if (ser->type != LOOP_PROXY)
		return;

	m_tmpProxy[ser->socket] = ser;
}

void DBProxyModule::OnServerClose(SHARE<NetServer>& ser)
{
	if (ser->type != LOOP_PROXY)
		return;
	auto itg = m_groupMap.find(ser->socket);
	if (itg == m_groupMap.end())
	{
		m_tmpProxy.erase(ser->socket);
		return;
	}

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
	TRY_PARSEPB(LPMsg::UpdateTableGroup, msg);
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

void DBProxyModule::OnForwardMsgHash(NetMsg * msg)
{
	if (msg->getLen() < sizeof(int32_t)*2)
		return;
	if (m_groups.size() == 0)
	{
		LP_ERROR << "Proxy Db group Error";
		return;
	}
	int group = m_groups[0];
	auto forbeg = msg->getNetBuff();
	if (m_groups.size() > 1)
	{
		int crc = PB::GetInt(forbeg);
		int idx = crc % (m_groups.size() * 2);
		if (idx >= m_groups.size())
			idx = m_groups.size() - 1;
		group = m_groups[idx];
	}
	int32_t mid = PB::GetInt(forbeg + sizeof(int32_t));
	SendToProxy(group,mid ,msg);
}

void DBProxyModule::OnForwardMsgGroup(NetMsg * msg)
{
	if (msg->getLen() < sizeof(int32_t) * 2)
		return;
	auto forbeg = msg->getNetBuff();
	int group = PB::GetInt(forbeg);
	int32_t mid = PB::GetInt(forbeg +sizeof(int32_t));
	SendToProxy(group, mid, msg);
}

void DBProxyModule::SendToProxy(int group, const int& mid, NetMsg * msg)
{
	auto it = m_proxyGroup.find(group);
	if (it == m_proxyGroup.end())
	{
		LP_ERROR << "Proxy DB No group " << group;
		return;
	}
	if (it->second.size() == 0)
	{
		LP_ERROR << "Proxy DB group:" << group << " size == 0";
		return;
	}

	++m_hash;
	auto proxy = it->second[m_hash%it->second.size()];

	auto head = m_tranModule->GetServerConn(msg->socket);
	if (!head)
	{
		LP_ERROR << "Send To Mysql Error No Head";
		return;
	}

	m_path[0]->serid = head->serid;
	m_path[0]->type = head->type;
	*m_path[2] = *proxy;
	
	auto buff = msg->getNetBuff();
	auto sendbuff = GET_LAYER_MSG(BuffBlock);
	sendbuff->write(buff + sizeof(int32_t) * 2, msg->getLen() - sizeof(int32_t) * 2);
	m_tranModule->SendToServer(m_path, mid, sendbuff);
}