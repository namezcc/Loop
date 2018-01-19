#include "TransMsgModule.h"
#include "EventModule.h"
#include "NetObjectModule.h"
#include "MsgModule.h"
#include "json/json.h"
#include <fstream>
#include <sstream>

TransMsgModule::TransMsgModule(BaseLayer* l):BaseModule(l)
{
	m_serverType["game"] = SERVER_TYPE::LOOP_GAME;
	m_serverType["pgs"] = SERVER_TYPE::LOOP_PROXY_GS;
	m_serverType["pg"] = SERVER_TYPE::LOOP_PROXY_G;
	m_serverType["pp"] = SERVER_TYPE::LOOP_PROXY_PP;
	m_serverType["sql"] = SERVER_TYPE::LOOP_MYSQL;
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

	InitServerNet();
}

void TransMsgModule::Execute()
{

}

void TransMsgModule::InitServerNet()
{
	string file = "../../commonconf/serverPath.json";
	ifstream ifs;
	ifs.open(file);
	assert(ifs.is_open());

	Json::Reader reader;
	Json::Value root;
	assert(reader.parse(ifs, root, false));

	auto serstruct = root["struct"];
	auto serpath = root["path"];

	for (auto it = serstruct.begin();it!=serstruct.end();it++)
	{
		map<int, int> link;
		auto mems = it->getMemberNames();
		for (auto& s:mems)
		{
			auto stype = m_serverType[s];
			link[stype] = (*it)[s]["ltype"].asInt();
		}
		auto stype = m_serverType[it.key().asString()];
		m_serverLink[stype] = move(link);
	}

	for (auto it= serpath.begin();it!=serpath.end();it++)
	{
		list<vector<int>> path;
		for (int i = 0; i < it->size(); i++)
		{
			vector<int> vec;
			int n = (*it)[i].size();
			for (int j = 0; j < n; j++)
			{
				auto stype = m_serverType[(*it)[i][j].asString()];
				vec.push_back(stype);
			}
			path.push_back(move(vec));
		}
		vector<string> key;
		Loop::Split(it.key().asString(), "-", key);
		auto s1 = m_serverType[key[0]];
		auto s2 = m_serverType[key[1]];

		stringstream stream;
		stream << s1 << "-" << s2;
		m_serverPath[stream.str()] = move(path);
	}
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

void TransMsgModule::SendToServer(ServerNode& ser, const int& mid, const int& len, char* msg)
{
	auto myser = GetLayer()->GetServer();
	vector<SHARE<ServerNode>> path;
	GetTransPath(*myser, ser, path);
	TransMsgToServer(path, mid, len, msg);
}

void TransMsgModule::TransMsgToServer(vector<SHARE<ServerNode>>& sers, const int& mid, const int& len, char* msg)
{
	//because copy so delete here
	ExitCall exitcall([msg](){
		delete[] msg;
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
		auto s = sers[i].get();
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
		if (ser->type == myser->type && (ser->serid == myser->serid ||ser->serid==0))
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

void TransMsgModule::GetTransPath(ServerNode& beg, ServerNode& end, vector<SHARE<ServerNode>>& path)
{
	stringstream stream;
	stream << beg.type << "-" << end.type;
	auto it = m_serverPath.find(stream.str());
	if (it == m_serverPath.end())
		return;

	for (auto& itv:it->second)
	{
		path.clear();
		for (auto& t:itv)
		{
			auto sn = GetLayer()->GetSharedLoop<ServerNode>();
			sn->type = t;
			sn->serid = -1;
			path.push_back(sn);
		}
		(*path.begin())->serid = beg.serid;
		(*path.rbegin())->serid = end.serid;
		if (GetToPath(path))
			return;
	}
}

bool TransMsgModule::GetToPath(vector<SHARE<ServerNode>>& path)
{
	int idx = path.size();
	for (size_t i = 0; i < path.size()-1; i++)
	{
		auto firn = path[i].get();
		auto senn = path[i + 1].get();
		auto lftos = m_serverLink[firn->type][senn->type];
		if (lftos == 1)
		{
			auto lstof = m_serverLink[senn->type][firn->type];
			senn->serid = ceil(float(firn->serid)/ lstof);
		}
		else if (lftos < 0)
			senn->serid = 0;
		else
		{
			idx = i;
			break;
		}
	}
	if (idx == path.size())
		return true;

	for (int i = path.size() - 1; i > idx; i--)
	{
		auto firn = path[i].get();
		auto senn = path[i - 1].get();

		auto lftos = m_serverLink[firn->type][senn->type];
		if (lftos == 1)
		{
			auto lstof = m_serverLink[senn->type][firn->type];
			int id = ceil(float(firn->serid) / lstof);
			if (senn->serid >= 0 && id != senn->serid)
				return false;
			else
				senn->serid = id;
		}
		else if (lftos < 0)
			senn->serid = 0;
		else
			if (senn->serid < 0)
				return false;
	}
	return true;
}