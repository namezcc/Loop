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

	m_msgModule->AddMsgCallBack(N_TRANS_SERVER_MSG, this, &TransMsgModule::OnGetTransMsg);

	//InitServerNet();
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
		map<int32_t, int32_t> link;
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
		list<vector<int32_t>> path;
		for (int32_t i = 0; i < it->size(); i++)
		{
			vector<int32_t> vec;
			int32_t n = (*it)[i].size();
			for (int32_t j = 0; j < n; j++)
			{
				auto stype = m_serverType[(*it)[i][j].asString()];
				vec.push_back(stype);
			}
			path.push_back(move(vec));
		}

		path.sort([](vector<int32_t>& v1, vector<int32_t>& v2) {
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
	//�ж��������,��ص��ϵ�
	auto its = m_serverList[ser->type].find(ser->serid);
	if (its != m_serverList[ser->type].end())
	{
		m_netObjMod->CloseNetObject(its->second->socket);
		RemoveRand(ser->type, ser->serid);
	}
	m_serverList[ser->type][ser->serid] = ser;
	m_randServer[ser->type].push_back(ser.get());
	m_allServer[ser->socket] = ser;
}

void TransMsgModule::OnServerClose(SHARE<NetServer>& ser)
{
	m_serverList[ser->type].erase(ser->serid);
	m_allServer.erase(ser->socket);
	RemoveRand(ser->type, ser->serid);
}

void TransMsgModule::RemoveRand(const int32_t & stype, const int32_t & sid)
{
	auto it = m_randServer.find(stype);
	if (it == m_randServer.end())
		return;
	for (size_t i = 0; i < it->second.size(); i++)
	{
		if (it->second[i]->serid == sid)
		{
			it->second[i] = it->second[it->second.size() - 1];
			it->second.pop_back();
		}
	}
}

SHARE<NetServer> TransMsgModule::GetServerConn(const int32_t & sock)
{
	auto it = m_allServer.find(sock);
	if (it == m_allServer.end())
		return NULL;
	return it->second;
}

void TransMsgModule::SendToServer(ServerNode & ser, const int32_t & mid, BuffBlock* buff)
{
	auto toser = GetServer(ser.type, ser.serid);
	if (toser)
	{
		m_netObjMod->SendNetMsg(toser->socket, mid, buff);
		return;
	}
	RECYCLE_LAYER_MSG(buff);
	/*auto myser = GetLayer()->GetServer();
	vector<SHARE<ServerNode>> path;
	GetTransPath(*myser, ser, path);
	SendToServer(path, mid,buff);*/
}

void TransMsgModule::SendToServer(ServerNode & ser, const int32_t & mid, google::protobuf::Message & msg)
{
	auto toser = GetServer(ser.type, ser.serid);
	if (toser)
	{
		m_netObjMod->SendNetMsg(toser->socket, mid, msg);
		return;
	}
	/*auto myser = GetLayer()->GetServer();
	vector<SHARE<ServerNode>> path;
	GetTransPath(*myser, ser, path);
	TransMsgToServer(path, mid, msg);*/
}

void TransMsgModule::SendToAllServer(const int32_t& stype, const int32_t & mid, google::protobuf::Message & msg)
{
	auto it = m_serverList.find(stype);
	if (it != m_serverList.end())
	{
		for (auto& s:it->second)
			m_netObjMod->SendNetMsg(s.second->socket, mid, msg);
		return;
	}
	//
	//ServerNode ser{stype,0};
	//auto myser = GetLayer()->GetServer();
	//vector<SHARE<ServerNode>> path;
	//GetTransPath(*myser, ser, path);
	//path.back()->serid = -1;	//-1 ��ʾȫ����
	//TransMsgToServer(path, mid, msg);

}

void TransMsgModule::SendToServer(vector<SHARE<ServerNode>>& path, const int32_t & mid, google::protobuf::Message & msg)
{
	TransMsgToServer(path, mid, msg);
}

void TransMsgModule::SendToServer(vector<SHARE<ServerNode>>& path, const int32_t & mid, BuffBlock* buff)
{
	TransMsgToServer(path, mid, buff);
}

void TransMsgModule::SendBackServer(vector<SHARE<ServerNode>>& path, const int32_t & mid, google::protobuf::Message& msg)
{
	std::reverse(path.begin(), path.end());
	TransMsgToServer(path, mid, msg);
}

void TransMsgModule::TransMsgToServer(vector<SHARE<ServerNode>>& sers, const int32_t& mid, google::protobuf::Message& pbmsg)
{
	auto buff = PB::PBToBuffBlock(GetLayer(),pbmsg);
	TransMsgToServer(sers, mid, buff);
}

void TransMsgModule::TransMsgToServer(vector<SHARE<ServerNode>>& sers, const int32_t & mid,BuffBlock* buffblock)
{
	bool res = false;
	ExitCall _call([this,&buffblock,&res]() {
		if(!res)
			GetLayer()->RecycleLayerMsg(buffblock);
	});

	if (sers.size() < 2)	//must >= 2
		return;
	auto myser = GetLayer()->GetServer();
	int32_t toindex = 1;
	for (size_t i = 0; i < sers.size()-1; i++)
	{
		if (sers[i]->serid == myser->serid && sers[i]->type == myser->type)
		{
			toindex = i+1;
			break;
		}
	}

	auto toser = sers[toindex];
	auto it = m_serverList.find(toser->type);
	if (it == m_serverList.end())
		return;
	auto its = it->second.find(toser->serid);
	if (its == it->second.end())
		return;

	auto pathbuff = PathToBuff(sers,mid,toindex);
	pathbuff->m_next = buffblock;
	m_netObjMod->SendNetMsg(its->second->socket, N_TRANS_SERVER_MSG, pathbuff);
	res = true;
}

int32_t TransMsgModule::GetPathSize(vector<SHARE<ServerNode>>& sers)
{
	return TransHead::SIZE+sers.size()*ServerNode::SIZE+4;
}

void TransMsgModule::OnGetTransMsg(NetMsg* nmsg)
{
	auto buffblock = nmsg->getCombinBuff(GetLayer());
	auto nmsgbuff = buffblock->m_buff;
	auto msglen = buffblock->m_size;
	char hsize = nmsgbuff[0];
	char hindex = nmsgbuff[1];
	char* first = nmsgbuff + TransHead::SIZE;
	
	if (hsize <= hindex)
		return;
	if (hsize == hindex + 1)
	{
		char* ser = first + hindex*ServerNode::SIZE;
		int8_t sertype = ser[0];
		int16_t serid = ser[1]&0xff;
		serid |= (ser[2] & 0xff) << 8;
		//�յ�
		auto myser = GetLayer()->GetServer();
		if (sertype == myser->type && (serid == myser->serid || serid ==0))
		{
			//sizeof(int32_t) ת����msg Id
			int32_t mlen = msglen - 4 - ServerNode::SIZE*hsize - TransHead::SIZE;
			char* midp = nmsgbuff + (msglen - mlen - 4);

			int32_t mid = PB::GetInt(midp);

			auto xMsg = GET_SHARE(NetServerMsg);
			xMsg->socket = nmsg->socket;
			xMsg->mid = mid;

			for (size_t i = 0; i < hsize; i++)
			{
				char* node = first + i*ServerNode::SIZE;
				auto snode = GET_SHARE(ServerNode);

				snode->type = node[0];
				snode->serid = node[1];
				snode->serid |= (int32_t)node[2]<<8;
				xMsg->path.push_back(snode);
			}
			xMsg->push_front(GetLayer(),nmsgbuff+(msglen - mlen),mlen);
			m_msgModule->TransMsgCall(xMsg);
		}
	}
	else {
		++hindex;
		nmsgbuff[1] = hindex;
		char* next = first + hindex*ServerNode::SIZE;

		int8_t nxtype = next[0];
		int16_t nxid = next[1]&0xff;
		nxid |= (next[2] &0xff)<<8;

		if (nxid == -1)
		{//ȫת��
			auto it = m_serverList.find(nxtype);
			if (it == m_serverList.end())
				return;
			int32_t nextFn = hindex*ServerNode::SIZE+TransHead::SIZE;
			for (auto& s:it->second)
			{
				// auto tmsg = new char[nmsg->len+PACK_HEAD_SIZE];
				// memcpy(tmsg+ PACK_HEAD_SIZE, nmsg->msg, nmsg->len);
				auto buffblock = GetLayer()->GetLayerMsg<BuffBlock>();
				buffblock->write(nmsgbuff, msglen);
				char* tnext = buffblock->m_buff + nextFn;
				tnext[1] = (unsigned char)s.second->serid;
				tnext[2] = (unsigned char)(s.second->serid >> 8);
				m_netObjMod->SendNetMsg(s.second->socket, N_TRANS_SERVER_MSG, buffblock);
			}
			return;
		}
		auto server = GetServer(nxtype, nxid);
		if (server)
		{
			next[1] = (unsigned char)server->serid;
			next[2] = (unsigned char)(server->serid >> 8);

			// char* tmsg = new char[nmsg->len + PACK_HEAD_SIZE];
			// memcpy(tmsg + PACK_HEAD_SIZE, nmsg->msg, nmsg->len);
			auto buffblock = GetLayer()->GetLayerMsg<BuffBlock>();
			buffblock->write(nmsgbuff, msglen);
			m_netObjMod->SendNetMsg(server->socket, N_TRANS_SERVER_MSG, buffblock);
		}
	}
}

NetServer* TransMsgModule::GetServer(const int32_t& type, const int32_t& serid)
{
	auto it = m_serverList.find(type);
	if (it == m_serverList.end() || it->second.size() == 0)
	{
		LP_ERROR(m_msgModule) << "GetServer NULL type:" << type << " serid:" << serid;
		return NULL;
	}
	
	if (serid == 0)
	{//hash һ��
		auto sit = m_randServer.find(type);
		if (sit == m_randServer.end())
			return it->second.begin()->second.get();
		auto res = rand() % sit->second.size();
		return sit->second[res];
	}
	auto its = it->second.find(serid);
	if (its == it->second.end())
		return NULL;
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
	int32_t idx = path.size();
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

	for (int32_t i = path.size() - 1; i > idx; i--)
	{
		auto firn = path[i].get();
		auto senn = path[i - 1].get();

		auto lftos = m_serverLink[firn->type][senn->type];
		if (lftos == 1)
		{
			auto lstof = m_serverLink[senn->type][firn->type];
			int32_t id = ceil(float(firn->serid) / lstof);
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

BuffBlock* TransMsgModule::PathToBuff(vector<SHARE<ServerNode>>& path,const int32_t& mid, const int32_t& toindex)
{
	auto buff = GetLayer()->GetLayerMsg<BuffBlock>();
	buff->makeRoom(GetPathSize(path));
	char* began = buff->m_buff;

	began[0] = (unsigned char)path.size();
	began[1] = (unsigned char)toindex;

	char* tag = began + TransHead::SIZE;
	for (size_t i = 0; i < path.size(); i++)
	{
		auto s = path[i].get();
		tag[0] = (unsigned char)s->type;
		tag[1] = (unsigned char)s->serid;
		tag[2] = (unsigned char)(s->serid >> 8);
		tag += ServerNode::SIZE;
	}
	PB::WriteInt(tag, mid);
	return buff;
}

BuffBlock* TransMsgModule::EncodeCoroMsg(BuffBlock* buff,const int32_t& mid,const int32_t& coid,const int32_t& mycoid)
{
	auto corobuff = GET_LAYER_MSG(BuffBlock);
	corobuff->makeRoom(sizeof(int32_t)*3);
	PB::WriteInt(corobuff->m_buff,mid);
	PB::WriteInt(corobuff->m_buff+sizeof(int32_t),coid);
	PB::WriteInt(corobuff->m_buff+sizeof(int32_t)*2,mycoid);
	corobuff->m_next = buff;
	return corobuff;
}

SHARE<BaseMsg> TransMsgModule::RequestServerAsynMsg(ServerNode& ser, const int32_t& mid, BuffBlock* buff,c_pull& pull,SHARE<BaseCoro>& coro)
{
	auto coid = m_msgModule->GenCoroIndex();
	auto corobuff = EncodeCoroMsg(buff,mid,coid);
	SendToServer(ser,N_REQUEST_CORO_MSG,corobuff);
	return m_msgModule->PullWait(coid,coro,pull);
}

SHARE<BaseMsg> TransMsgModule::RequestServerAsynMsg(ServerNode& ser, const int32_t& mid, gpb::Message& msg,c_pull& pull,SHARE<BaseCoro>& coro)
{
	auto buff = PB::PBToBuffBlock(GetLayer(),msg);
	return RequestServerAsynMsg(ser,mid,buff,pull,coro);
}

SHARE<BaseMsg> TransMsgModule::RequestServerAsynMsg(VecPath& path, const int32_t& mid, BuffBlock* buff,c_pull& pull,SHARE<BaseCoro>& coro)
{
	auto coid = m_msgModule->GenCoroIndex();
	auto corobuff = EncodeCoroMsg(buff,mid,coid);
	SendToServer(path,N_REQUEST_CORO_MSG,corobuff);
	return m_msgModule->PullWait(coid,coro,pull);
}

SHARE<BaseMsg> TransMsgModule::RequestServerAsynMsg(VecPath& path, const int32_t& mid, gpb::Message& msg,c_pull& pull,SHARE<BaseCoro>& coro)
{
	auto buff = PB::PBToBuffBlock(GetLayer(),msg);
	return RequestServerAsynMsg(path,mid,buff,pull,coro);
}

SHARE<BaseMsg> TransMsgModule::RequestBackServerAsynMsg(VecPath& path, const int32_t& mid, gpb::Message& msg,c_pull& pull,SHARE<BaseCoro>& coro)
{
	std::reverse(path.begin(), path.end());
	return RequestServerAsynMsg(path,mid,msg,pull,coro);
}

SHARE<BaseMsg> TransMsgModule::RequestServerAsynMsg(ServerNode & ser, const int32_t & mid, gpb::Message & msg, c_pull & pull, SHARE<BaseCoro>& coro, const ReqFail & failCall)
{
	bool res = false;
	ExitCall call([&res, failCall]() {
		if (!res)
			failCall();
	});
	auto resmsg = RequestServerAsynMsg(ser, mid, msg, pull, coro);
	res = true;
	return resmsg;
}

SHARE<BaseMsg> TransMsgModule::RequestServerAsynMsg(VecPath & path, const int32_t & mid, gpb::Message & msg, c_pull & pull, SHARE<BaseCoro>& coro, const ReqFail & failCall)
{
	bool res = false;
	ExitCall call([&res,failCall]() {
		if (!res)
			failCall;
	});
	auto resmsg = RequestServerAsynMsg(path,mid,msg,pull,coro);
	res = true;
	return resmsg;
}

SHARE<BaseMsg> TransMsgModule::ResponseServerAsynMsg(ServerNode& ser, SHARE<BaseMsg>& comsg, BuffBlock* buff,c_pull& pull,SHARE<BaseCoro>& coro)
{
	auto coroMsg = (CoroMsg*)comsg.get();
	auto coid = coroMsg->m_mycoid >0 ? coroMsg->m_mycoid : coroMsg->m_coroId;
	auto mycoid = m_msgModule->GenCoroIndex();
	auto corobuff = EncodeCoroMsg(buff,0,coid,mycoid);
	SendToServer(ser,N_RESPONSE_CORO_MSG,corobuff);
	return m_msgModule->PullWait(mycoid,coro,pull);
}

SHARE<BaseMsg> TransMsgModule::ResponseServerAsynMsg(ServerNode& ser, SHARE<BaseMsg>& comsg, gpb::Message& msg,c_pull& pull,SHARE<BaseCoro>& coro)
{
	auto buff = PB::PBToBuffBlock(GetLayer(),msg);
	return ResponseServerAsynMsg(ser,comsg,buff,pull,coro);
}

SHARE<BaseMsg> TransMsgModule::ResponseServerAsynMsg(VecPath& path, SHARE<BaseMsg>& comsg, BuffBlock* buff,c_pull& pull,SHARE<BaseCoro>& coro)
{
	auto coroMsg = (CoroMsg*)comsg.get();
	auto coid = coroMsg->m_mycoid >0 ? coroMsg->m_mycoid : coroMsg->m_coroId;
	auto mycoid = m_msgModule->GenCoroIndex();
	auto corobuff = EncodeCoroMsg(buff,0,coid,mycoid);
	SendToServer(path,N_RESPONSE_CORO_MSG,corobuff);
	return m_msgModule->PullWait(mycoid,coro,pull);
}

SHARE<BaseMsg> TransMsgModule::ResponseServerAsynMsg(VecPath& path, SHARE<BaseMsg>& comsg, gpb::Message& msg,c_pull& pull,SHARE<BaseCoro>& coro)
{
	auto buff = PB::PBToBuffBlock(GetLayer(),msg);
	return ResponseServerAsynMsg(path,comsg,buff,pull,coro);
}

SHARE<BaseMsg> TransMsgModule::ResponseBackServerAsynMsg(VecPath& path, SHARE<BaseMsg>& comsg, gpb::Message& msg,c_pull& pull,SHARE<BaseCoro>& coro)
{
	std::reverse(path.begin(), path.end());
	return ResponseServerAsynMsg(path,comsg,msg,pull,coro);
}

void TransMsgModule::ResponseServerMsg(ServerNode& ser, SHARE<BaseMsg>& comsg, BuffBlock* buff)
{
	auto coroMsg = (CoroMsg*)comsg.get();
	auto coid = coroMsg->m_mycoid >0 ? coroMsg->m_mycoid : coroMsg->m_coroId;
	auto corobuff = EncodeCoroMsg(buff,0,coid,0);
	SendToServer(ser,N_RESPONSE_CORO_MSG,corobuff);
}

void TransMsgModule::ResponseServerMsg(ServerNode& ser, SHARE<BaseMsg>& comsg, gpb::Message& msg)
{
	auto buff = PB::PBToBuffBlock(GetLayer(), msg);
	ResponseServerMsg(ser,comsg,buff);
}

void TransMsgModule::ResponseServerMsg(VecPath& path, SHARE<BaseMsg>& comsg, BuffBlock* buff)
{
	auto coroMsg = (CoroMsg*)comsg.get();
	auto coid = coroMsg->m_mycoid >0 ? coroMsg->m_mycoid : coroMsg->m_coroId;
	auto corobuff = EncodeCoroMsg(buff,0,coid,0);
	SendToServer(path,N_RESPONSE_CORO_MSG,corobuff);
}

void TransMsgModule::ResponseServerMsg(VecPath& path, SHARE<BaseMsg>& comsg, gpb::Message& msg)
{
	auto buff = PB::PBToBuffBlock(GetLayer(),msg);
	ResponseServerMsg(path,comsg,buff);
}

void TransMsgModule::ResponseBackServerMsg(VecPath& path, SHARE<BaseMsg>& comsg, gpb::Message& msg)
{
	std::reverse(path.begin(), path.end());
	ResponseServerMsg(path,comsg,msg);
}
