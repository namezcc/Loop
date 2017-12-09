#include "TransMsgModule.h"
#include "EventModule.h"
#include "NetObjectModule.h"
#include "MsgModule.h"

TransMsgModule::TransMsgModule(BaseLayer* l):BaseModule(l)
{
}


TransMsgModule::~TransMsgModule()
{
}

void TransMsgModule::Init()
{
	m_eventModule = GetLayer()->GetModule<EventModule>();
	m_netObjMod = GetLayer()->GetModule<NetObjectModule>();
	m_msgModule = GetLayer()->GetModule<MsgModule>();

	m_eventModule->AddEventCallBack(E_SERVER_CONNECT, this, &TransMsgModule::OnServerConnect);
	m_eventModule->AddEventCallBack(E_SERVER_CLOSE, this, &TransMsgModule::OnServerClose);

	m_msgModule->AddMsgCallBack<NetMsg>(N_TRANS_SERVER_MSG, this, &TransMsgModule::OnGetTransMsg);
}

void TransMsgModule::Execute()
{

}

void TransMsgModule::OnServerConnect(const int& nEvent, NetServer* ser)
{
	auto it = m_serverList.find(ser->type);
	if (it == m_serverList.end())
	{
		map<int, NetServer*> sermap;
		sermap[ser->serid] = ser;
		m_serverList[ser->type] = move(sermap);
		return;
	}
	//判断如果已有,则关掉老的
	auto its = it->second.find(ser->serid);
	if (its != it->second.end())
	{
		m_netObjMod->CloseNetObject(its->second->socket);
		GetLayer()->Recycle(its->second);
	}
	it->second[ser->serid] = ser;
}

void TransMsgModule::OnServerClose(const int& nEvent, NetServer* ser)
{
	auto it = m_serverList.find(ser->type);
	if (it == m_serverList.end())
		return;
	it->second.erase(ser->serid);
}

void TransMsgModule::TransMsgToServer(vector<ServerNode*>& sers, const int& mid, const int& len, char* msg)
{
	ExitCall exitcall([msg](){
		delete msg;
	});

	if (sers.size() < 2)
		return;
	auto toser = sers[1];
	auto it = m_serverList.find(toser->type);
	if (it == m_serverList.end())
		return;
	auto its = it->second.find(toser->serid);
	if (its == it->second.end())
		return;
	
	TransHead head;
	head.size = sers.size();
	head.index = 1;

	int nsize = sizeof(head) + head.size * sizeof(ServerNode) + sizeof(mid) + len;
	char* newmsg = new char[nsize];

	memcpy(newmsg, &head, sizeof(head));
	char* tag = newmsg + sizeof(head);
	for (size_t i = 0; i < head.size; i++)
	{
		auto s = sers[i];
		memcpy(tag, s, sizeof(ServerNode));
		tag += sizeof(ServerNode);
	}
	memcpy(tag, &mid, sizeof(mid));
	tag += sizeof(mid);
	memcpy(tag, msg, len);

	m_netObjMod->SendNetMsg(its->second->socket, newmsg, N_TRANS_SERVER_MSG, nsize);
}

void TransMsgModule::OnGetTransMsg(NetMsg* nmsg)
{
	TransHead* head = (TransHead*)nmsg->msg;
	ServerNode* first = (ServerNode*)(head+1);
	if (head->size <= head->index)
		return;
	if (head->size == head->index + 1)
	{
		auto ser = first + head->index;
		//终点
		auto myser = GetLayer()->GetServer();
		if (ser->type == myser->type && ser->serid == myser->serid)
		{
			int mlen = nmsg->len - sizeof(int) - sizeof(ServerNode)*head->size - sizeof(TransHead);
			int* mid = (int*)(nmsg->msg + (nmsg->len - mlen - sizeof(int)));
			NetMsg* xMsg = new NetMsg();
			xMsg->len = mlen;
			xMsg->socket = nmsg->socket;
			xMsg->msg = new char[mlen];
			xMsg->mid = *mid;

			memcpy(xMsg->msg, nmsg->msg + (nmsg->len - mlen), mlen);
			m_msgModule->TransMsgCall(xMsg);
		}
	}
	else {
		++head->index;
		auto next = first + head->index;
		auto server = GetServer(next->type, next->serid);
		if (server)
		{
			m_netObjMod->SendNetMsg(server->socket, nmsg->msg, N_TRANS_SERVER_MSG, nmsg->len);
			nmsg->msg = nullptr;//注意
		}
	}
}

NetServer* TransMsgModule::GetServer(const int& type, const int& serid)
{
	auto it = m_serverList.find(type);
	if (it == m_serverList.end() || it->second.size()==0)
		return nullptr;
	
	if (serid == 0)
	{
		auto fir = it->second.begin();
		auto last = it->second.rbegin();
		auto ser = GetLayer()->GetServer();
		int slot = (ser->serid + last->first) % (last->first - fir->first + 1) + fir->first;
		auto its = it->second.lower_bound(slot);
		return its->second;
	}
	auto its = it->second.find(serid);
	if (its == it->second.end())
		return nullptr;
	return its->second;
}