#include "TransMsgModule.h"
#include "EventModule.h"
#include "NetObjectModule.h"
#include "MsgModule.h"
#include "ScheduleModule.h"
#include <fstream>
#include <sstream>
#include "protoPB/base/LPBase.pb.h"

TransMsgModule::TransMsgModule(BaseLayer* l):BaseModule(l)
{
	
}

TransMsgModule::~TransMsgModule()
{
}

void TransMsgModule::Init()
{
	m_eventModule = GET_MODULE(EventModule);
	m_netObjMod = GET_MODULE(NetObjectModule);
	m_msgModule = GET_MODULE(MsgModule);
	m_schedule = GET_MODULE(ScheduleModule);

	m_msgModule->AddMsgCall(N_REGISTE_SERVER, BIND_CALL(OnServerRegiste, NetMsg));
	m_msgModule->AddMsgCall(N_TRANS_SERVER_MSG, BIND_CALL(OnGetTransMsg,NetMsg));

	m_eventModule->AddEventCall(E_SERVER_SOCKET_CLOSE,BIND_EVENT(OnServerClose, int32_t));

}

void TransMsgModule::BeforExecute()
{
	auto& config = GetLayer()->GetLoopServer()->GetConfig();

	for (auto& scfg : config.connect)
	{
		auto ser = GET_SHARE(NetServer);
		ser->type = scfg.type;
		ser->ip = scfg.ip;
		ser->port = scfg.port;
		ser->serid = scfg.serid;
		ser->state = CONN_STATE::CLOSE;
		ser->socket = -1;
		ser->activeLink = true;
		AddServerConn(*ser);
	}
}

void TransMsgModule::Execute()
{

}

void TransMsgModule::AddServerConn(const NetServer & ser)
{
	m_netObjMod->ConnectServer(ser, [this](bool res, NetServer& _ser) {
		if (res)
		{
			auto connSer = GET_SHARE(NetServer);
			*connSer = _ser;
			OnServerConnect(connSer);
			//registe server
			auto myser = GetLayer()->GetServer();
			auto config = GetLayer()->GetLoopServer()->GetConfig();
			LPMsg::ServerInfo xMsg;
			xMsg.set_id(myser->serid);
			xMsg.set_type(myser->type);
			xMsg.set_ip(config.addr.ip);
			xMsg.set_port(config.addr.port);
			m_netObjMod->SendNetMsg(_ser.socket, N_REGISTE_SERVER, xMsg);
		}
		else
		{
			NetServer tmp = _ser;
			m_schedule->AddInterValTask([this,tmp](int64_t&) {
				AddServerConn(tmp);
			},5000,1);
		}
	});
}

void TransMsgModule::OnServerConnect(SHARE<NetServer>& ser)
{
	auto its = m_serverList[ser->type].find(ser->serid);
	if (its != m_serverList[ser->type].end())
	{
		m_netObjMod->CloseNetObject(its->second->socket);
		RemoveRand(ser->type, ser->serid);
	}
	m_serverList[ser->type][ser->serid] = ser;
	m_randServer[ser->type].push_back(ser.get());
	m_allServer[ser->socket] = ser;
	m_netObjMod->AcceptConn(ser->socket, CONN_SERVER);
	m_eventModule->SendEvent(E_SERVER_CONNECT, ser);
}

void TransMsgModule::OnServerRegiste(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::ServerInfo, msg);

	auto server = GET_SHARE(NetServer);
	server->serid = pbMsg.id();
	server->type = pbMsg.type();
	server->socket = msg->socket;
	server->state = CONN_STATE::CONNECT;
	server->ip = pbMsg.ip();
	server->port = pbMsg.port();
	server->activeLink = false;
	OnServerConnect(server);

	LP_WARN << "Server registe type:" << server->type << " ID:" << server->serid;
}

void TransMsgModule::OnServerClose(int32_t sock)
{
	auto it = m_allServer.find(sock);
	if (it == m_allServer.end())
		return;

	auto ser = it->second;
	m_serverList[ser->type].erase(ser->serid);
	m_allServer.erase(ser->socket);
	RemoveRand(ser->type, ser->serid);
	m_eventModule->SendEvent(E_SOCKET_CLOSE, ser);
	//reconnect server
	if(ser->activeLink)
		AddServerConn(*ser);
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
}

void TransMsgModule::SendToServer(ServerNode & ser, const int32_t & mid, google::protobuf::Message & msg)
{
	auto toser = GetServer(ser.type, ser.serid);
	if (toser)
	{
		m_netObjMod->SendNetMsg(toser->socket, mid, msg);
		return;
	}
}

void TransMsgModule::SendToAllServer(const int32_t& stype, const int32_t & mid, google::protobuf::Message & msg)
{
	auto it = m_serverList.find(stype);
	if (it != m_serverList.end())
	{
		for (auto& s:it->second)
			m_netObjMod->SendNetMsg(s.second->socket, mid, msg);
	}
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
			toindex = (int32_t)i+1;
			break;
		}
	}

	auto toser = sers[toindex];

	if (toser->serid != -1)
	{
		auto sersock = GetServer(toser->type, toser->serid);
		if (!sersock)
			return;
		
		auto pathbuff = PathToBuff(sers, mid, toindex);
		pathbuff->m_next = buffblock;
		m_netObjMod->SendNetMsg(sersock->socket, N_TRANS_SERVER_MSG, pathbuff);
	}
	else {
		auto it = m_serverList.find(toser->type);
		if (it == m_serverList.end())
			return;

		sers[toindex]->serid = 0;
		auto pathbuff = PathToBuff(sers, mid, toindex);
		pathbuff->m_next = buffblock;

		std::vector<int32_t> broad;
		for (auto& ser:it->second)
			broad.push_back(ser.second->socket);
		m_netObjMod->BroadNetMsg(broad, N_TRANS_SERVER_MSG, pathbuff);
	}
	res = true;
}

int32_t TransMsgModule::GetPathSize(vector<SHARE<ServerNode>>& sers)
{
	return static_cast<int32_t>(TransHead::SIZE + sers.size()*ServerNode::SIZE + 4);
}

void TransMsgModule::OnGetTransMsg(NetMsg* nmsg)
{
	auto nmsgbuff = nmsg->getNetBuff();
	auto msglen = nmsg->getLen();
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
		// -1 send to all server  0 hash server
		if (nxid == -1)
		{//ȫת��
			auto it = m_serverList.find(nxtype);
			if (it == m_serverList.end())
				return;
			int32_t nextFn = hindex*ServerNode::SIZE+TransHead::SIZE;
			std::vector<int32_t> broad;
			for (auto& s:it->second)
				broad.push_back(s.second->socket);

			auto buffblock = GET_LAYER_MSG(BuffBlock);
			buffblock->write(nmsgbuff, msglen);
			char* tnext = buffblock->m_buff + nextFn;
			tnext[1] = (unsigned char)0;
			tnext[2] = (unsigned char)0;
			m_netObjMod->BroadNetMsg(broad, N_TRANS_SERVER_MSG, buffblock);
			return;
		}
		auto server = GetServer(nxtype, nxid);
		if (server)
		{
			next[1] = (unsigned char)server->serid;
			next[2] = (unsigned char)(server->serid >> 8);

			auto buffblock = GET_LAYER_MSG(BuffBlock);
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
		LP_ERROR << "GetServer NULL type:" << type << " serid:" << serid;
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

BuffBlock* TransMsgModule::PathToBuff(vector<SHARE<ServerNode>>& path,const int32_t& mid, const int32_t& toindex)
{
	auto buff = GetLayer()->GetLayerMsg<BuffBlock>();
	buff->m_size = GetPathSize(path);
	buff->makeRoom(buff->m_size);
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
	corobuff->m_size = sizeof(int32_t) * 3;
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

VecPath TransMsgModule::GetFromSelfPath(const int32_t& allSize, const int32_t& stype, const int32_t& sid)
{
	VecPath path;
	for (size_t i = 0; i < allSize; i++)
		path.push_back(GET_SHARE(ServerNode));
	
	*path.front() = *GetLayer()->GetServer();
	path.back()->type = stype;
	path.back()->serid = sid;
	return path;
}