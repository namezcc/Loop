#include "TransMsgModule.h"
#include "EventModule.h"
#include "NetObjectModule.h"
#include "MsgModule.h"
#include "json/json.h"
#include <fstream>
#include <sstream>
#include "LPFile.h"

TransMsgModule::TransMsgModule(BaseLayer* l):BaseModule(l)
{
	//need fix
	/*m_serverType["game"] = SERVER_TYPE::LOOP_GAME;
	m_serverType["pgs"] = SERVER_TYPE::LOOP_PROXY_GS;
	m_serverType["pg"] = SERVER_TYPE::LOOP_PROXY_G;
	m_serverType["pp"] = SERVER_TYPE::LOOP_PROXY_PP;
	m_serverType["sql"] = SERVER_TYPE::LOOP_MYSQL;*/
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
	string file;
	LoopFile::GetRootPath(file);
	file.append("commonconf/serverPath.json");

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

		path.sort([](vector<int>& v1, vector<int>& v2) {
			return v1.size() < v2.size();
		});

		vector<string> key;
		Loop::Split(it.key().asString(), "-", key);
		auto s1 = m_serverType[key[0]];
		auto s2 = m_serverType[key[1]];

		stringstream stream;
		stream << s1 << "-" << s2;
		m_serverPath[stream.str()] = move(path);
		if (s1 != s2)
		{
			stringstream p2;
			p2 << s2 << "-" << s1;
			m_serverPath[p2.str()] = m_serverPath[stream.str()];
			std::reverse(m_serverPath[p2.str()].begin(), m_serverPath[p2.str()].end());
		}
	}
}

void TransMsgModule::OnServerConnect(SHARE<NetServer>& ser)
{
	//判断如果已有,则关掉老的
	auto its = m_serverList[ser->type].find(ser->serid);
	if (its != m_serverList[ser->type].end())
	{
		m_netObjMod->CloseNetObject(its->second->socket);
	}
	m_serverList[ser->type][ser->serid] = ser;
	m_allServer[ser->socket] = ser;
}

void TransMsgModule::OnServerClose(SHARE<NetServer>& ser)
{
	m_serverList[ser->type].erase(ser->serid);
	m_allServer.erase(ser->socket);
}

SHARE<NetServer> TransMsgModule::GetServerConn(const int & sock)
{
	auto it = m_allServer.find(sock);
	if (it == m_allServer.end())
		return nullptr;
	return it->second;
}

void TransMsgModule::SendToServer(ServerNode & ser, const int & mid, char * msg, const int & len)
{
	auto toser = GetServer(ser.type, ser.serid);
	if (toser)
	{
		m_netObjMod->SendNetMsg(toser->socket, msg, mid, len);
		return;
	}
	auto myser = GetLayer()->GetServer();
	vector<SHARE<ServerNode>> path;
	GetTransPath(*myser, ser, path);
	TransMsgToServer(path, mid, msg,len);
}

void TransMsgModule::SendToServer(ServerNode & ser, const int & mid, google::protobuf::Message & msg)
{
	auto toser = GetServer(ser.type, ser.serid);
	if (toser)
	{
		m_netObjMod->SendNetMsg(toser->socket, mid, msg);
		return;
	}
	auto myser = GetLayer()->GetServer();
	vector<SHARE<ServerNode>> path;
	GetTransPath(*myser, ser, path);
	TransMsgToServer(path, mid, msg);
}

void TransMsgModule::SendToAllServer(const int& stype, const int & mid, google::protobuf::Message & msg)
{
	auto it = m_serverList.find(stype);
	if (it != m_serverList.end())
	{
		for (auto& s:it->second)
			m_netObjMod->SendNetMsg(s.second->socket, mid, msg);
		return;
	}
	
	ServerNode ser{stype,0};
	auto myser = GetLayer()->GetServer();
	vector<SHARE<ServerNode>> path;
	GetTransPath(*myser, ser, path);
	path.back()->serid = -1;	//-1 表示全发送
	TransMsgToServer(path, mid, msg);

}

void TransMsgModule::SendToServer(vector<SHARE<ServerNode>>& path, const int & mid, google::protobuf::Message & msg, const int& toidx)
{
	TransMsgToServer(path, mid, msg,toidx);
}

void TransMsgModule::SendToServer(vector<SHARE<ServerNode>>& path, const int & mid, const char * msg, const int & len, const int& toidx)
{
	TransMsgToServer(path, mid, msg, len,toidx);
}

void TransMsgModule::SendBackServer(vector<SHARE<ServerNode>>& path, const int & mid, google::protobuf::Message& msg)
{
	std::reverse(path.begin(), path.end());
	/*vector<SHARE<ServerNode>> backpath(path.size());
	for (auto it=path.rbegin();it!=path.rend();it++)
		backpath.push_back(*it);*/

	TransMsgToServer(path, mid, msg);
}

void TransMsgModule::TransMsgToServer(vector<SHARE<ServerNode>>& sers, const int& mid, google::protobuf::Message& pbmsg, const int& toidx)
{
	int len;
	auto msg = PB::PBToChar(pbmsg, len);
	TransMsgToServer(sers, mid, msg, len);
}

void TransMsgModule::TransMsgToServer(vector<SHARE<ServerNode>>& sers, const int & mid, const char * msg, const int & len,const int& toidx)
{
	ExitCall _call([&msg]() {
		delete[] msg;
	});

	if (sers.size() < toidx)
		return;
	auto toser = sers[toidx];
	auto it = m_serverList.find(toser->type);
	if (it == m_serverList.end())
		return;
	auto its = it->second.find(toser->serid);
	if (its == it->second.end())
		return;

	TransHead head;
	head.size = sers.size();
	head.index = toidx;

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
			//sizeof(int) 转发的msg Id
			int mlen = nmsg->len - sizeof(int) - sizeof(ServerNode)*head->size - sizeof(TransHead);
			int* mid = (int*)(nmsg->msg + (nmsg->len - mlen - sizeof(int)));
			NetMsg* xMsg = new NetMsg();
			xMsg->len = mlen;
			xMsg->socket = nmsg->socket;
			xMsg->msg = new char[mlen];
			xMsg->mid = *mid;

			for (size_t i = 0; i < head->size; i++)
			{
				auto node = first + i;
				auto snode = GetLayer()->GetSharedLoop<ServerNode>();
				*snode = *node;
				xMsg->path.push_back(snode);
			}

			memcpy(xMsg->msg, nmsg->msg + (nmsg->len - mlen), mlen);
			m_msgModule->TransMsgCall(xMsg);//由函数内delete 待定
		}
	}
	else {
		++head->index;
		auto next = first + head->index;

		if (next->serid == -1)
		{//全转发
			auto it = m_serverList.find(next->type);
			if (it == m_serverList.end())
				return;
			auto nextFn = (char*)next - nmsg->msg;
			for (auto& s:it->second)
			{
				auto tmsg = new char[nmsg->len];
				memcpy(tmsg, nmsg->msg, nmsg->len);
				auto tnext = (ServerNode*)(tmsg + nextFn);
				tnext->serid = s.second->serid;
				m_netObjMod->SendNetMsg(s.second->socket, tmsg, N_TRANS_SERVER_MSG, nmsg->len);
			}
			return;
		}
		auto server = GetServer(next->type, next->serid);
		if (server)
		{
			next->serid = server->serid;
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
	{//hash 一个
		auto fir = it->second.begin();
		auto last = it->second.rbegin();
		auto ser = GetLayer()->GetServer();
		int slot = (ser->serid + last->first) % (last->first - fir->first + 1) + fir->first;
		auto its = it->second.lower_bound(slot);
		return its->second.get();
	}
	auto its = it->second.find(serid);
	if (its == it->second.end())
		return nullptr;
	return its->second.get();
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