#include "NetObjectModule.h"
#include "MsgModule.h"
#include "EventModule.h"
#include "LoopServer.h"
#include "TransMsgModule.h"

NetObjectModule::NetObjectModule(BaseLayer* l):BaseModule(l), m_acceptNoCheck(false), m_noCheckType(CONN_CLIENT)
{
}


NetObjectModule::~NetObjectModule()
{
}

void NetObjectModule::Init()
{
	m_msgModule = GetLayer()->GetModule<MsgModule>();
	m_eventModule = GetLayer()->GetModule<EventModule>();
	m_transModule = GET_MODULE(TransMsgModule);

	m_msgModule->AddMsgCall(L_SOCKET_CONNET, BIND_CALL(OnSocketConnet, NetMsg));
	m_msgModule->AddMsgCall(L_SOCKET_CLOSE, BIND_CALL(OnSocketClose, NetMsg));
	m_msgModule->AddMsgCall(L_SERVER_CONNECTED, BIND_CALL(OnServerConnet,NetServer));
	
	m_outTime = 5;
}

void NetObjectModule::BeforExecute()
{
}

void NetObjectModule::Execute()
{
	CheckOutTime();
}

void NetObjectModule::OnSocketConnet(NetMsg * sock)
{
	auto netobj = GET_SHARE(NetObject);
	netobj->socket = sock->socket;
	netobj->ctime = Loop::GetSecend()+m_outTime;
	if (m_acceptNoCheck)
	{
		netobj->type = m_noCheckType;
		m_objects[sock->socket] = netobj;
	}
	else
		m_objects_tmp[sock->socket] = netobj;
	m_eventModule->SendEvent(E_SOCKEK_CONNECT, sock->socket);
}

void NetObjectModule::OnSocketClose(NetMsg * sock)
{
	auto it = m_objects.find(sock->socket);
	if (it != m_objects.end())
	{
		NoticeSocketClose(it->second.get());
		m_objects.erase(it);
		return;
	}

	it = m_objects_tmp.find(sock->socket);
	if (it != m_objects_tmp.end())
	{
		m_objects_tmp.erase(it);
	}
}

void NetObjectModule::NoticeSocketClose(NetObject* obj)
{
	switch (obj->type)
	{
	case CONN_CLIENT:
		m_eventModule->SendEvent(E_SOCKET_CLOSE, obj->socket);
		break;
	case CONN_SERVER:
		m_eventModule->SendEvent(E_SERVER_SOCKET_CLOSE, obj->socket);
		break;
	case CONN_HTTP_CLIENT:
		m_eventModule->SendEvent(E_CLIENT_HTTP_CLOSE, obj->socket);
		break;
	case CONN_PHP_CGI:
		m_eventModule->SendEvent(E_PHP_CGI_CLOSE, obj->socket);
		break;
	}
}

void NetObjectModule::AcceptConn(const int & socket, const int32_t& connType)
{
	auto it = m_objects_tmp.find(socket);
	if (it != m_objects_tmp.end())
	{
		it->second->type = connType;
		m_objects[socket] = it->second;
		m_objects_tmp.erase(it);
	}
	else {
		auto it2 = m_objects.find(socket);
		if (it2 != m_objects.end())
			it2->second->type = connType;
	}
}

void NetObjectModule::SendNetMsg(const int & socket, const int & mid, const google::protobuf::Message & pbmsg)
{
	NetMsg* nMsg = GetLayer()->GetLayerMsg<NetMsg>();
	nMsg->socket = socket;
	nMsg->mid = mid;
	auto buff = GET_LAYER_MSG(BuffBlock);
	buff->makeRoom(pbmsg.ByteSize());
	buff->write(pbmsg);
	nMsg->push_front(buff);
	m_msgModule->SendMsg(L_SOCKET_SEND_DATA, nMsg);
}

void NetObjectModule::SendNetMsg(const int& socket,const int32_t & mid, BuffBlock* buff)
{
	//m_object û���ж� ... ����Ҫ
	NetMsg* nMsg = GetLayer()->GetLayerMsg<NetMsg>();
	nMsg->socket = socket;
	nMsg->mid = mid;
	nMsg->push_front(buff);
	m_msgModule->SendMsg(L_SOCKET_SEND_DATA, nMsg);
}

void NetObjectModule::BroadNetMsg(const std::vector<int32_t>& socks, const int32_t & mid, const gpb::Message & pbmsg)
{
	auto nMsg = GET_LAYER_MSG(BroadMsg);

	nMsg->m_socks = std::move(socks);
	nMsg->mid = mid;
	auto buff = GET_LAYER_MSG(BuffBlock);
	buff->makeRoom(pbmsg.ByteSize());
	buff->write(pbmsg);
	nMsg->push_front(buff);
	m_msgModule->SendMsg(L_SOCKET_BROAD_DATA, nMsg);
}

void NetObjectModule::BroadNetMsg(const std::vector<int32_t>& socks, const int32_t & mid, BuffBlock * buff)
{
	auto nMsg = GET_LAYER_MSG(BroadMsg);
	nMsg->m_socks = std::move(socks);
	nMsg->mid = mid;
	nMsg->push_front(buff);
	m_msgModule->SendMsg(L_SOCKET_BROAD_DATA, nMsg);
}

SHARE<BaseMsg> NetObjectModule::ResponseAsynMsg(const int32_t & socket, SHARE<BaseMsg>& comsg, gpb::Message & pbmsg, c_pull& pull, SHARE<BaseCoro>& coro)
{
	auto buff = GET_LAYER_MSG(BuffBlock);
	buff->makeRoom(pbmsg.ByteSize());
	buff->write(pbmsg);
	return ResponseAsynMsg(socket,comsg,buff,pull,coro);
}

SHARE<BaseMsg> NetObjectModule::ResponseAsynMsg(const int32_t & socket, SHARE<BaseMsg>& comsg, BuffBlock * buff, c_pull& pull, SHARE<BaseCoro>& coro)
{
	auto coroMsg = (CoroMsg*)comsg.get();
	auto coid = coroMsg->m_mycoid >0 ? coroMsg->m_mycoid : coroMsg->m_coroId;
	auto mycoid = m_msgModule->GenCoroIndex();
	auto cobuf = m_transModule->EncodeCoroMsg(buff,0, coid, mycoid);
	SendNetMsg(socket, N_RESPONSE_CORO_MSG, cobuf);
	return m_msgModule->PullWait(mycoid,coro,pull);
}

void NetObjectModule::ResponseMsg(const int32_t & socket, SHARE<BaseMsg>& comsg, gpb::Message & pbmsg)
{
	auto buff = GET_LAYER_MSG(BuffBlock);
	buff->makeRoom(pbmsg.ByteSize());
	buff->write(pbmsg);
	ResponseMsg(socket, comsg, buff);
}

void NetObjectModule::ResponseMsg(const int32_t & socket, SHARE<BaseMsg>& comsg, BuffBlock * buff)
{
	auto coroMsg = (CoroMsg*)comsg.get();
	auto coid = coroMsg->m_mycoid >0 ? coroMsg->m_mycoid : coroMsg->m_coroId;
	auto corobuff = m_transModule->EncodeCoroMsg(buff, 0, coid, 0);
	SendNetMsg(socket, N_RESPONSE_CORO_MSG, corobuff);
}

void NetObjectModule::SendHttpMsg(const int& socket, NetBuffer& buf)
{
	NetMsg* nMsg = GetLayer()->GetLayerMsg<NetMsg>();
	auto buffblk = GetLayer()->GetLayerMsg<BuffBlock>();
	nMsg->socket = socket;
	buffblk->makeRoom(buf.use);
	buffblk->writeBuff(buf.buf,buf.use);
	buf.buf = NULL;
	nMsg->push_front(buffblk);
	m_msgModule->SendMsg(L_SOCKET_SEND_DATA, nMsg);
}

void NetObjectModule::CloseNetObject(const int& socket)
{
	auto it = m_objects.find(socket);
	if (it == m_objects.end())
		return;

	auto sock = GetLayer()->GetLayerMsg<NetMsg>();
	sock->socket = socket;

	m_msgModule->SendMsg(L_SOCKET_CLOSE, sock);
	m_objects.erase(it);
}

//can not in ConnectServerRes Call ConnectServer again !!!
void NetObjectModule::ConnectServer(const NetServer & ser, const ConnectServerRes & call)
{
	auto msg = GET_LAYER_MSG(NetServer);
	*msg = ser;
	m_tempServer[ser.ip + ":" + loop_cast<std::string>(ser.port)] = call;
	m_msgModule->SendMsg(L_TO_CONNET_SERVER,msg);
}

void NetObjectModule::OnServerConnet(NetServer* ser)
{
	auto key = ser->ip + ":" + loop_cast<std::string>(ser->port);
	auto it = m_tempServer.find(key);
	if (it == m_tempServer.end())
		return;

	if (ser->state == CONN_STATE::CONNECT)
	{
		LP_INFO << "connect server " << ser->serid << "success ip:" << ser->ip << " port: " << ser->port;
		auto netobj = GET_SHARE(NetObject);
		netobj->socket = ser->socket;
		m_objects_tmp[netobj->socket] = netobj;
		if (m_tempServer.size() == 1)
		{
			GetLayer()->GetLoopServer()->setServerState(SERR_LINK, false);
		}
	}
	else
	{
		LP_ERROR << "connect server " << ser->serid << "error ip:" << ser->ip << " port: " << ser->port;
		GetLayer()->GetLoopServer()->setServerState(SERR_LINK, true);
	}
	it->second(ser->socket >= 0, *ser);
	m_tempServer.erase(it);
}

void NetObjectModule::CheckOutTime()
{
	if (m_objects_tmp.size() == 0)
		return;
	auto nt = Loop::GetSecend();
	if (nt < m_tmpObjTime)
		return;
	m_tmpObjTime = nt + 5;
	for (auto it=m_objects_tmp.begin();it!=m_objects_tmp.end();)
	{
		if (it->second->ctime < nt)
		{
			auto sock = GetLayer()->GetLayerMsg<NetMsg>();
			sock->socket = it->second->socket;
			m_msgModule->SendMsg(L_SOCKET_CLOSE, sock);
			m_objects_tmp.erase(it++);
		}
		else
			it++;
	}
}
