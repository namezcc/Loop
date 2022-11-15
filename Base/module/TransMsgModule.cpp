#include "TransMsgModule.h"
#include "EventModule.h"
#include "NetObjectModule.h"
#include "MsgModule.h"
#include "ScheduleModule.h"
#include <fstream>
#include <sstream>
#include "protoPB/base/LPBase.pb.h"
#include "TcpAsioSessionModule.h"
#include "LPStringUtil.h"

TransMsgModule::TransMsgModule(BaseLayer* l):BaseModule(l), m_old_state(-1), m_find_service_idx(0)
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
	m_msgModule->AddMsgCall(N_CONN_SERVER_INFO, BIND_NETMSG(onConnInfo));
	m_msgModule->AddMsgCall(N_GET_LINK_SERVER_INFO, BIND_SHARE_CALL(onGetLinkInfo));

	m_eventModule->AddEventCall(E_SERVER_SOCKET_CLOSE,BIND_EVENT(OnServerClose, int32_t));

	/*auto sertype = GetLayer()->GetServer()->type;
	if (sertype != LOOP_CONSOLE && sertype != LOOP_MASTER)
	{
		m_schedule->AddInterValTask(BIND_TIME(checkSendServerState), 5000, -1, 3000);
	}*/
}

void TransMsgModule::BeforExecute()
{
	auto config = GetLayer()->GetLoopServer()->getConnectServer();

	for (auto& scfg : config)
	{
		auto ser = GET_SHARE(NetServer);
		ser->type = scfg.type;
		ser->ip = scfg.ip;
		ser->port = scfg.port;
		ser->serid = scfg.server_id;
		ser->state = CONN_STATE::CLOSE;
		ser->socket = -1;
		ser->activeLink = true;
		AddServerConn(*ser);
	}

	setFindService();
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
			//registe server
			auto myser = GetLayer()->GetServer();
			auto config = GetLayer()->GetLoopServer()->GetConfig();
			LPMsg::ServerInfo xMsg;
			xMsg.set_id(myser->serid);
			xMsg.set_type(myser->type);
			xMsg.set_ip(config.addr.ip);
			xMsg.set_port(config.addr.port);
			m_netObjMod->SendNetMsg(_ser.socket, N_REGISTE_SERVER, xMsg);


			*connSer = _ser;
			OnServerConnect(connSer);
		}
		else
		{
			if (!GetServer(_ser.type,_ser.serid))
			{
				NetServer tmp = _ser;
				m_schedule->AddInterValTask([this,tmp](int64_t&) {
					AddServerConn(tmp);
				},5000,1,5000);
			}

		}
	});
}

void TransMsgModule::setFindService()
{
	auto selfinfo = getLoopServer()->GetConfig();
	auto service = getLoopServer()->getServerInfo(LOOP_SERVICE_FIND);
	if (service.empty())
	{
		LP_ERROR << "service find empty";
		getLoopServer()->closeServer();
		return;
	}
	auto idx = 0;
	if (m_find_service_idx > 0)
	{
		idx = m_find_service_idx % service.size();
	}
	else
	{
		idx = selfinfo.addr.serid%service.size();
		m_find_service_idx = idx;
	}
	auto& scfg = service[idx];
	auto ser = GET_SHARE(NetServer);
	ser->type = scfg.type;
	ser->ip = scfg.ip;
	ser->port = scfg.port;
	ser->serid = scfg.server_id;
	ser->state = CONN_STATE::CLOSE;
	ser->socket = -1;
	ser->activeLink = false;
	
	m_netObjMod->ConnectServer(*ser, [this](bool res, NetServer& _ser) {
		if (res)
		{
			auto connSer = GET_SHARE(NetServer);
			//registe server
			auto server = getLoopServer();
			auto config = server->GetConfig();
			/*LPMsg::ServerInfo xMsg;
			xMsg.set_id(config.addr.serid);
			xMsg.set_type(config.addr.type);
			xMsg.set_ip(ip);
			xMsg.set_port(config.addr.port);*/


			auto sendbuff = GET_LAYER_MSG(BuffBlock);
			sendbuff->makeRoom(512);

			sendbuff->writeString(server->getServerAddrKey());
			sendbuff->writeString(server->getServerAddrInfo());

			std::vector<std::string> connkey,watchkey;
			server->getConnectKey(connkey, watchkey);
			auto noticekey = server->getNoticKey();

			sendbuff->writeInt8((int8_t)connkey.size());
			for (auto& s : connkey)
				sendbuff->writeString(s);

			sendbuff->writeInt8((int8_t)watchkey.size());
			for (auto& s : watchkey)
				sendbuff->writeString(s);

			sendbuff->writeInt8((int8_t)noticekey.size());
			for (auto& s : noticekey)
				sendbuff->writeString(s);

			m_netObjMod->SendNetMsg(_ser.socket, N_REGISTE_SERVER, sendbuff);

			*connSer = _ser;
			OnServerConnect(connSer);
		}
		else {
			m_find_service_idx++;
			m_schedule->AddInterValTask([this](int64_t&) {
				setFindService();
			}, 5000, 1, 5000);
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

	if (ser->type == LOOP_CONSOLE)
	{
		m_old_state = -1;
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

	LP_INFO << "Server registe type:" << server->type << " ID:" << server->serid;
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
	m_eventModule->SendEvent(E_SERVER_CLOSE, ser);
	//reconnect server
	LP_ERROR << "Server close type:" << ser->type << " ID:" << ser->serid;
	if (ser->activeLink)
	{
		AddServerConn(*ser);
	}

	if (ser->type == LOOP_SERVICE_FIND)
	{
		m_schedule->AddInterValTask([this](int64_t&) {
			setFindService();
		}, 5000, 1, 5000);
	}
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

void TransMsgModule::SendToServer(const ServerNode & ser, const int32_t & mid, BuffBlock* buff)
{
	auto toser = GetServer(ser.type, ser.serid);
	if (toser)
	{
		m_netObjMod->SendNetMsg(toser->socket, mid, buff);
		return;
	}
	if (buff)
	{
		RECYCLE_LAYER_MSG(buff);
	}
}

void TransMsgModule::SendToServer(const ServerNode & ser, const int32_t & mid, const google::protobuf::Message & msg)
{
	auto toser = GetServer(ser.type, ser.serid);
	if (toser)
	{
		m_netObjMod->SendNetMsg(toser->socket, mid, msg);
		return;
	}
}

void TransMsgModule::SendToAllServer(const int32_t& stype, const int32_t & mid, const google::protobuf::Message & msg)
{
	auto it = m_serverList.find(stype);
	if (it != m_serverList.end())
	{
		if (it->second.empty())
			return;
		std::vector<int32_t> socks;
		for (auto& s : it->second)
			socks.push_back(s.second->socket);
		m_netObjMod->BroadNetMsg(socks, mid, msg);
	}
}

void TransMsgModule::SendToAllServer(const int32_t & stype, const int32_t & mid, BuffBlock * buff)
{
	auto it = m_serverList.find(stype);
	if (it != m_serverList.end())
	{
		if (it->second.empty())
		{
			RECYCLE_LAYER_MSG(buff);
			return;
		}
		std::vector<int32_t> socks;
		for (auto& s : it->second)
			socks.push_back(s.second->socket);
		m_netObjMod->BroadNetMsg(socks, mid, buff);
	}
}

void TransMsgModule::SendToServer(ServerPath& path, const int32_t & mid, const google::protobuf::Message & msg)
{
	TransMsgToServer(path, mid, msg);
}

void TransMsgModule::SendToServer(ServerPath& path, const int32_t & mid, BuffBlock* buff)
{
	TransMsgToServer(path, mid, buff);
}

void TransMsgModule::SendBackServer(ServerPath& path, const int32_t & mid, const google::protobuf::Message& msg)
{
	std::reverse(path.begin(), path.end());
	TransMsgToServer(path, mid, msg);
}

void TransMsgModule::SendBackServer(ServerPath & path, const int32_t & mid, BuffBlock * buff)
{
	std::reverse(path.begin(), path.end());
	TransMsgToServer(path, mid, buff);
}

void TransMsgModule::TransMsgToServer(ServerPath& sers, const int32_t& mid, const google::protobuf::Message& pbmsg)
{
	auto buff = GET_LAYER_MSG(BuffBlock);
	buff->makeRoom(pbmsg.ByteSize());
	buff->write(pbmsg);
	TransMsgToServer(sers, mid, buff);
}

void TransMsgModule::TransMsgToServer(ServerPath& sers, const int32_t & mid,BuffBlock* buffblock)
{
	bool res = false;
	ExitCall _call([this,&buffblock,&res]() {
		if(!res)
			RECYCLE_LAYER_MSG(buffblock);
	});

	if (sers.size() < 2)	//must >= 2
		return;
	auto myser = GetLayer()->GetServer();
	int32_t toindex = 1;
	for (size_t i = 0; i < sers.size()-1; i++)
	{
		if (sers[i].serid == myser->serid && sers[i].type == myser->type)
		{
			toindex = (int32_t)i+1;
			break;
		}
	}

	auto toser = sers[toindex];

	if (toser.serid != -1)
	{
		auto toserver = GetServer(toser.type, toser.serid);
		if (!toserver)
			return;
		
		sers[toindex].serid = toserver->serid;
		auto pathbuff = PathToBuff(sers, mid, toindex);
		pathbuff->m_next = buffblock;
		m_netObjMod->SendNetMsg(toserver->socket, N_TRANS_SERVER_MSG, pathbuff);
	}
	else {
		auto it = m_serverList.find(toser.type);
		if (it == m_serverList.end())
			return;

		sers[toindex].serid = 0;
		auto pathbuff = PathToBuff(sers, mid, toindex);
		pathbuff->m_next = buffblock;

		std::vector<int32_t> broad;
		for (auto& ser:it->second)
			broad.push_back(ser.second->socket);
		m_netObjMod->BroadNetMsg(broad, N_TRANS_SERVER_MSG, pathbuff);
	}
	res = true;
}

int32_t TransMsgModule::GetPathSize(ServerPath& sers)
{
	//headsize + nodesize + mid
	return static_cast<int32_t>(TransHead::SIZE + sers.size()*ServerNode::SIZE + sizeof(int32_t));
}

void TransMsgModule::OnGetTransMsg(NetMsg* nmsg)
{
	auto buffblock = nmsg->m_buff;

	auto nmsgbuff = nmsg->getNetBuff();
	auto msglen = nmsg->getLen();
	char hsize = buffblock->readInt8();
	char hindex = buffblock->readInt8();

	char* first = nmsgbuff + TransHead::SIZE;
	if (hsize <= hindex)
		return;

	if (hsize == hindex + 1)
	{
		buffblock->setOffect(buffblock->getOffect() + hindex * ServerNode::SIZE);
		int8_t sertype = buffblock->readInt8();
		int16_t serid = buffblock->readInt16();

		auto myser = GetLayer()->GetServer();
		if (sertype == myser->type && (serid == myser->serid || serid ==0))
		{
			auto xMsg = GET_SHARE(NetServerMsg);
			xMsg->socket = nmsg->socket;	

			buffblock->setOffect(TransHead::SIZE);
			for (int8_t i = 0; i < hsize; i++)
			{
				ServerNode snode;
				snode.type = buffblock->readInt8();
				snode.serid = buffblock->readInt16();
				xMsg->path.push_back(snode);
			}

			xMsg->mid = buffblock->readInt32();
			int32_t bufflen;
			auto buff = buffblock->readBuff(bufflen);
			xMsg->push_front(GetLayer(),buff,bufflen);
			m_msgModule->TransMsgCall(xMsg);
		}
	}
	else {
		++hindex;
		buffblock->setOffect(1);
		buffblock->writeInt8(hindex);
		buffblock->setOffect(TransHead::SIZE + hindex*ServerNode::SIZE);

		int8_t nxtype = buffblock->readInt8();
		int16_t nxid = buffblock->readInt16();
		// -1 send to all server  0 hash server
		if (nxid == -1)
		{
			auto it = m_serverList.find(nxtype);
			if (it == m_serverList.end())
				return;

			std::vector<int32_t> broad;
			for (auto& s:it->second)
				broad.push_back(s.second->socket);

			auto sendbuff = GET_LAYER_MSG(BuffBlock);
			sendbuff->writeBuff(buffblock->m_buff, buffblock->getSize());
			sendbuff->setOffect(TransHead::SIZE + hindex * ServerNode::SIZE);
			sendbuff->readInt8();
			sendbuff->writeInt16(0);
			m_netObjMod->BroadNetMsg(broad, N_TRANS_SERVER_MSG, sendbuff);
			return;
		}
		auto server = GetServer(nxtype, nxid);
		if (server)
		{
			auto sendbuff = GET_LAYER_MSG(BuffBlock);
			sendbuff->writeBuff(buffblock->m_buff, buffblock->getSize());
			sendbuff->setOffect(TransHead::SIZE + hindex * ServerNode::SIZE);
			sendbuff->readInt8();
			sendbuff->writeInt16(server->serid);
			m_netObjMod->SendNetMsg(server->socket, N_TRANS_SERVER_MSG, sendbuff);
		}
	}
}

void TransMsgModule::onConnInfo(NetMsg * nmsg)
{
	std::vector<NetServer> server;

	auto p = nmsg->m_buff;
	auto num = p->readInt32();

	for (int32_t i = 0; i < num; i++)
	{
		auto info = p->readString();
		std::vector<std::string> vec;
		Loop::Split(info, ":", vec);

		if (vec.size() != 5)
		{
			LP_ERROR << "conn info error :" << info;
			continue;
		}

		NetServer ser;
		ser.type = Loop::Cvto<int>(vec[1]);
		ser.serid = Loop::Cvto<int>(vec[2]);
		ser.ip = vec[3];
		ser.port = Loop::Cvto<int>(vec[4]);
		ser.state = CONN_STATE::CLOSE;
		ser.socket = -1;
		ser.activeLink = true;
		
		server.push_back(ser);
	}

	for (auto& s:server)
	{
		if (GetServer(s.type,s.serid) != NULL)
		{
			continue;
		}

		AddServerConn(s);
	}
}

void TransMsgModule::onGetLinkInfo(SHARE<BaseMsg>& msg)
{
	auto buf = LAYER_BUFF;
	auto begindex = buf->getOffect();
	int32_t num = 0;
	buf->writeInt32(num);

	for (auto i:m_allServer)
	{
		if (i.second->activeLink)
		{
			num++;

			buf->writeInt32(i.second->type);
			buf->writeInt32(i.second->serid);
		}
	}

	auto endindex = buf->getOffect();
	buf->setOffect(begindex);
	buf->writeInt32(num);
	buf->setOffect(endindex);
	ResponseServerMsg(ServerNode{ LOOP_CONSOLE,0 }, msg, buf);
}

NetServer* TransMsgModule::GetServer(const int32_t& type, const int32_t& serid)
{
	auto it = m_serverList.find(type);
	if (it == m_serverList.end() || it->second.size() == 0)
	{
		LP_INFO << "GetServer NULL type:" << type << " serid:" << serid;
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

BuffBlock* TransMsgModule::PathToBuff(ServerPath& path,const int32_t& mid, const int32_t& toindex)
{
	auto buff = GetLayer()->GetLayerMsg<BuffBlock>();
	buff->makeRoom(GetPathSize(path));

	buff->writeInt8((unsigned char)path.size());
	buff->writeInt8((unsigned char)toindex);

	for (size_t i = 0; i < path.size(); i++)
	{
		auto s = path[i];
		buff->writeInt8(s.type);
		buff->writeInt16(s.serid);
	}
	buff->writeInt32(mid);
	return buff;
}

void TransMsgModule::checkSendServerState(int64_t dt)
{
	if (m_old_state == GetLayer()->GetLoopServer()->getServerState())
		return;

	if (GetServer(LOOP_CONSOLE,0) == NULL)
		return;

	auto node = GetLayer()->GetServer();
	m_old_state = GetLayer()->GetLoopServer()->getServerState();

	auto buf = LAYER_BUFF;

	buf->writeInt32(node->type);
	buf->writeInt32(node->serid);
	buf->writeInt32(m_old_state);

	SendToServer(ServerNode{ LOOP_CONSOLE,0 }, N_SEND_SERVER_STATE, buf);
}

BuffBlock* TransMsgModule::EncodeCoroMsg(BuffBlock* buff,const int32_t& mid,const int32_t& coid,const int32_t& mycoid)
{
	auto corobuff = GET_LAYER_MSG(BuffBlock);
	corobuff->makeRoom(sizeof(int32_t)*3);
	corobuff->writeInt32(mid);
	corobuff->writeInt32(coid);
	corobuff->writeInt32(mycoid);
	corobuff->m_next = buff;
	return corobuff;
}

NetMsg* TransMsgModule::RequestServerAsynMsg(const ServerNode& ser, const int32_t& mid, BuffBlock* buff,c_pull& pull,SHARE<BaseCoro>& coro)
{
	auto coid = m_msgModule->GenCoroIndex();
	auto corobuff = EncodeCoroMsg(buff,mid,coid);
	SendToServer(ser,N_REQUEST_CORO_MSG,corobuff);
	return dynamic_cast<NetMsg*>(m_msgModule->PullWait(coid,coro,pull)->m_data);
}

NetMsg* TransMsgModule::RequestServerAsynMsg(const ServerNode& ser, const int32_t& mid, const gpb::Message& msg,c_pull& pull,SHARE<BaseCoro>& coro)
{
	auto buff = GET_LAYER_MSG(BuffBlock);
	buff->makeRoom(msg.ByteSize());
	buff->write(msg);
	return RequestServerAsynMsg(ser,mid,buff,pull,coro);
}

NetMsg* TransMsgModule::RequestServerAsynMsg(ServerPath& path, const int32_t& mid, BuffBlock* buff,c_pull& pull,SHARE<BaseCoro>& coro)
{
	auto coid = m_msgModule->GenCoroIndex();
	auto corobuff = EncodeCoroMsg(buff,mid,coid);
	SendToServer(path,N_REQUEST_CORO_MSG,corobuff);
	return dynamic_cast<NetMsg*>(m_msgModule->PullWait(coid,coro,pull)->m_data);
}

NetMsg* TransMsgModule::RequestServerAsynMsg(ServerPath& path, const int32_t& mid, const gpb::Message& msg,c_pull& pull,SHARE<BaseCoro>& coro)
{
	auto buff = GET_LAYER_MSG(BuffBlock);
	buff->makeRoom(msg.ByteSize());
	buff->write(msg);
	return RequestServerAsynMsg(path,mid,buff,pull,coro);
}

NetMsg* TransMsgModule::RequestBackServerAsynMsg(ServerPath& path, const int32_t& mid, const gpb::Message& msg,c_pull& pull,SHARE<BaseCoro>& coro)
{
	std::reverse(path.begin(), path.end());
	return RequestServerAsynMsg(path,mid,msg,pull,coro);
}

NetMsg* TransMsgModule::RequestServerAsynMsg(const ServerNode & ser, const int32_t & mid, const gpb::Message & msg, c_pull & pull, SHARE<BaseCoro>& coro, const ReqFail & failCall)
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

NetMsg* TransMsgModule::RequestServerAsynMsg(ServerPath & path, const int32_t & mid, const gpb::Message & msg, c_pull & pull, SHARE<BaseCoro>& coro, const ReqFail & failCall)
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

NetMsg* TransMsgModule::ResponseServerAsynMsg(const ServerNode& ser, SHARE<BaseMsg>& comsg, BuffBlock* buff,c_pull& pull,SHARE<BaseCoro>& coro)
{
	auto coroMsg = (CoroMsg*)comsg.get();
	auto coid = coroMsg->m_mycoid >0 ? coroMsg->m_mycoid : coroMsg->m_coroId;
	auto mycoid = m_msgModule->GenCoroIndex();
	auto corobuff = EncodeCoroMsg(buff,0,coid,mycoid);
	SendToServer(ser,N_RESPONSE_CORO_MSG,corobuff);
	return dynamic_cast<NetMsg*>(m_msgModule->PullWait(mycoid,coro,pull)->m_data);
}

NetMsg* TransMsgModule::ResponseServerAsynMsg(const ServerNode& ser, SHARE<BaseMsg>& comsg, const gpb::Message& msg,c_pull& pull,SHARE<BaseCoro>& coro)
{
	auto buff = GET_LAYER_MSG(BuffBlock);
	buff->makeRoom(msg.ByteSize());
	buff->write(msg);
	return ResponseServerAsynMsg(ser,comsg,buff,pull,coro);
}

NetMsg* TransMsgModule::ResponseServerAsynMsg(ServerPath& path, SHARE<BaseMsg>& comsg, BuffBlock* buff,c_pull& pull,SHARE<BaseCoro>& coro)
{
	auto coroMsg = (CoroMsg*)comsg.get();
	auto coid = coroMsg->m_mycoid >0 ? coroMsg->m_mycoid : coroMsg->m_coroId;
	auto mycoid = m_msgModule->GenCoroIndex();
	auto corobuff = EncodeCoroMsg(buff,0,coid,mycoid);
	SendToServer(path,N_RESPONSE_CORO_MSG,corobuff);
	return dynamic_cast<NetMsg*>(m_msgModule->PullWait(mycoid,coro,pull)->m_data);
}

NetMsg* TransMsgModule::ResponseServerAsynMsg(ServerPath& path, SHARE<BaseMsg>& comsg, const gpb::Message& msg,c_pull& pull,SHARE<BaseCoro>& coro)
{
	auto buff = GET_LAYER_MSG(BuffBlock);
	buff->makeRoom(msg.ByteSize());
	buff->write(msg);
	return ResponseServerAsynMsg(path,comsg,buff,pull,coro);
}

NetMsg* TransMsgModule::ResponseBackServerAsynMsg(ServerPath& path, SHARE<BaseMsg>& comsg, const gpb::Message& msg,c_pull& pull,SHARE<BaseCoro>& coro)
{
	std::reverse(path.begin(), path.end());
	return ResponseServerAsynMsg(path,comsg,msg,pull,coro);
}

void TransMsgModule::ResponseServerMsg(const ServerNode& ser, SHARE<BaseMsg>& comsg, BuffBlock* buff)
{
	auto coroMsg = (CoroMsg*)comsg.get();
	auto coid = coroMsg->m_mycoid >0 ? coroMsg->m_mycoid : coroMsg->m_coroId;
	auto corobuff = EncodeCoroMsg(buff,0,coid,0);
	SendToServer(ser,N_RESPONSE_CORO_MSG,corobuff);
}

void TransMsgModule::ResponseServerMsg(const ServerNode& ser, SHARE<BaseMsg>& comsg, const gpb::Message& msg)
{
	auto buff = GET_LAYER_MSG(BuffBlock);
	buff->makeRoom(msg.ByteSize());
	buff->write(msg);
	ResponseServerMsg(ser,comsg,buff);
}

void TransMsgModule::ResponseServerMsg(ServerPath& path, SHARE<BaseMsg>& comsg, BuffBlock* buff)
{
	auto coroMsg = (CoroMsg*)comsg.get();
	auto coid = coroMsg->m_mycoid >0 ? coroMsg->m_mycoid : coroMsg->m_coroId;
	auto corobuff = EncodeCoroMsg(buff,0,coid,0);
	SendToServer(path,N_RESPONSE_CORO_MSG,corobuff);
}

void TransMsgModule::ResponseServerMsg(ServerPath& path, SHARE<BaseMsg>& comsg, const gpb::Message& msg)
{
	auto buff = GET_LAYER_MSG(BuffBlock);
	buff->makeRoom(msg.ByteSize());
	buff->write(msg);
	ResponseServerMsg(path,comsg,buff);
}

void TransMsgModule::ResponseBackServerMsg(ServerPath& path, SHARE<BaseMsg>& comsg, const gpb::Message& msg)
{
	std::reverse(path.begin(), path.end());
	ResponseServerMsg(path,comsg,msg);
}

ServerPath TransMsgModule::GetFromSelfPath(const int32_t& allSize, const int32_t& stype, const int32_t& sid)
{
	ServerPath path;
	for (int32_t i = 0; i < allSize; i++)
		path.push_back(ServerNode{});
	
	path[0] = *GetLayer()->GetServer();
	path.back().type = stype;
	path.back().serid = sid;
	return path;
}