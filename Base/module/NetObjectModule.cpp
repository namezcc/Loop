#include "NetObjectModule.h"
#include "MsgModule.h"
#include "EventModule.h"
#include "LoopServer.h"
#include "TransMsgModule.h"

#include "protoPB/base/LPBase.pb.h"

NetObjectModule::NetObjectModule(BaseLayer* l):BaseModule(l), m_acceptNoCheck(false)
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

	m_msgModule->AddMsgCall(L_SOCKET_CONNET, BIND_CALL(OnSocketConnet,NetSocket));
	m_msgModule->AddMsgCall(L_SOCKET_CLOSE, BIND_CALL(OnSocketClose,NetSocket));
	m_msgModule->AddMsgCall(L_SERVER_CONNECTED, BIND_CALL(OnServerConnet,NetServer));
	m_msgModule->AddMsgCall(N_REGISTE_SERVER, BIND_CALL(OnServerRegiste,NetMsg));
	
	m_msgModule->AddMsgCall(L_PHP_CGI_CONNECTED, BIND_CALL(OnPHPCgiConnect,NetServer));

	m_eventModule->AddEventCallBack(E_CLIENT_HTTP_CONNECT, this, &NetObjectModule::OnHttpClientConnect);
	
	m_outTime = 5;
}

void NetObjectModule::BeforExecute()
{
	auto& config = GetLayer()->GetLoopServer()->GetConfig();

	for (auto& ser:config.connect)
	{
		AddServerConn(ser.type, ser.serid, ser.ip, ser.port);
	}
}

void NetObjectModule::Execute()
{
	CheckOutTime();
	CheckReconnect();
}

void NetObjectModule::OnSocketConnet(NetSocket * sock)
{
	auto netobj = GetLayer()->GetSharedLoop<NetObject>();
	netobj->socket = sock->socket;
	netobj->ctime = GetSecend()+m_outTime;
	if (m_acceptNoCheck)
		m_objects[sock->socket] = netobj;
	else
		m_objects_tmp[sock->socket] = netobj;
	m_eventModule->SendEvent(E_SOCKEK_CONNECT, sock->socket);
}

void NetObjectModule::OnSocketClose(NetSocket * sock)
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
		ServerClose(obj->socket);
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
	if (it == m_objects_tmp.end())
		return;

	it->second->type = connType;
	m_objects[socket] = it->second;
	m_objects_tmp.erase(it);
}

void NetObjectModule::SendNetMsg(const int & socket, const int & mid, google::protobuf::Message & pbmsg)
{
	NetMsg* nMsg = GetLayer()->GetLayerMsg<NetMsg>();
	nMsg->socket = socket;
	nMsg->mid = mid;
	auto buff = PB::PBToBuffBlock(GetLayer(),pbmsg);
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

void NetObjectModule::BroadNetMsg(const std::vector<int32_t>& socks, const int32_t & mid, gpb::Message & pbmsg)
{
	auto nMsg = GET_LAYER_MSG(BroadMsg);

	nMsg->m_socks = std::move(socks);
	nMsg->mid = mid;
	auto buff = PB::PBToBuffBlock(GetLayer(), pbmsg);
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
	auto buff = PB::PBToBuffBlock(GetLayer(), pbmsg);
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
	auto buff = PB::PBToBuffBlock(GetLayer(), pbmsg);
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
	buffblk->m_buff = buf.buf;
	buffblk->m_size = buf.use;
	buf.buf = NULL;
	nMsg->push_front(buffblk);
	m_msgModule->SendMsg(L_SOCKET_SEND_HTTP_DATA, nMsg);
}

void NetObjectModule::CloseNetObject(const int& socket)
{
	auto it = m_objects.find(socket);
	if (it == m_objects.end())
		return;

	NetSocket* sock = GetLayer()->GetLayerMsg<NetSocket>();
	sock->socket = socket;

	m_msgModule->SendMsg(L_SOCKET_CLOSE, sock);
	m_objects.erase(it);
}

void NetObjectModule::AddServerConn(const int& sType, const int& sid, const std::string& ip, const int& port)
{
	auto ser = GetLayer()->GetSharedLoop<NetServer>();
	ser->type = sType;
	ser->ip = ip;
	ser->port = port;
	ser->serid = sid;
	ser->state = CONN_STATE::CLOSE;
	ser->socket = -1;
	ser->activeLink = true;
	m_serverTmp[GetSerTypeId64(ser->type,ser->serid)] = ser;
}

void NetObjectModule::ConnectPHPCgi(NetServer& cgi)
{
	auto msg = GetLayer()->GetLayerMsg<NetServer>();
	*msg = cgi;
	m_msgModule->SendMsg(L_CONNECT_PHP_CGI, msg);
}

void NetObjectModule::OnServerConnet(NetServer* ser)
{
	auto it = m_serverTmp.find(GetSerTypeId64(ser->type, ser->serid));
	if (it == m_serverTmp.end())
		return;

	it->second->socket = ser->socket;
	it->second->state = CONN_STATE::CONNECT;

	m_serverConn[ser->socket] = it->second;
	//��ӽ� m_object ��
	auto netobj = GetLayer()->GetSharedLoop<NetObject>();
	netobj->socket = ser->socket;
	netobj->type = CONN_SERVER;
	m_objects[netobj->socket] = netobj;

	//����ע����Ϣ
	auto myser = GetLayer()->GetServer();
	auto config = GetLayer()->GetLoopServer()->GetConfig();

	LPMsg::ServerInfo xMsg;
	xMsg.set_id(myser->serid);
	xMsg.set_type(myser->type);
	xMsg.set_ip(config.addr.ip);
	xMsg.set_port(config.addr.port);

	SendNetMsg(ser->socket, N_REGISTE_SERVER, xMsg);

	//֪ͨ���ӳɹ�
	m_eventModule->SendEvent(E_SERVER_CONNECT, it->second);
	m_eventModule->SendEvent(E_SERVER_CONNECT_AFTER, it->second);
	m_serverTmp.erase(it);
}

void NetObjectModule::ServerClose(const int& socket)
{
	auto it = m_serverConn.find(socket);
	if (it == m_serverConn.end())
		return;
	it->second->socket = -1;
	it->second->state = CONN_STATE::CLOSE;
	if(it->second->activeLink)
		m_serverTmp[GetSerTypeId64(it->second->type, it->second->serid)] = it->second;
	//�¼�֪ͨ
	m_eventModule->SendEvent(E_SERVER_CLOSE, it->second);
	m_serverConn.erase(it);
}

void NetObjectModule::OnServerRegiste(NetMsg* msg)
{
	auto it = m_objects_tmp.find(msg->socket);
	if (it == m_objects_tmp.end())
		return;

	it->second->type = CONN_SERVER;
	m_objects[msg->socket] = it->second;
	m_objects_tmp.erase(it);
	
	TRY_PARSEPB(LPMsg::ServerInfo, msg);
	
	auto server = GetLayer()->GetSharedLoop<NetServer>();

	server->serid = pbMsg.id();
	server->type = pbMsg.type();
	server->socket = msg->socket;
	server->state = CONN_STATE::CONNECT;
	server->ip = pbMsg.ip();
	server->port = pbMsg.port();
	server->activeLink = false;

	m_serverConn[msg->socket] = server;
	m_eventModule->SendEvent(E_SERVER_CONNECT, server);
	m_eventModule->SendEvent(E_SERVER_CONNECT_AFTER, server);

	LP_WARN << "Server registe type:" << server->type << " ID:" << server->serid;
}

void NetObjectModule::OnPHPCgiConnect(NetServer* ser)
{
	//��ӽ� m_object ��
	auto netobj = GetLayer()->GetSharedLoop<NetObject>();
	netobj->socket = ser->socket;
	netobj->type = CONN_PHP_CGI;
	m_objects[netobj->socket] = netobj;
	//֪ͨ���ӳɹ�
	m_eventModule->SendEvent(E_PHP_CGI_CONNECT, ser);
}

void NetObjectModule::OnHttpClientConnect(const int& socket)
{
	auto it = m_objects_tmp.find(socket);
	if (it == m_objects_tmp.end())
		return;

	it->second->type = CONN_HTTP_CLIENT;
	m_objects[socket] = it->second;
	m_objects_tmp.erase(it);
}

void NetObjectModule::CheckOutTime()
{
	if (m_objects_tmp.size() == 0)
		return;
	auto nt = GetSecend();
	if (nt < m_tmpObjTime)
		return;
	m_tmpObjTime = nt + 5;
	for (auto it=m_objects_tmp.begin();it!=m_objects_tmp.end();)
	{
		if (it->second->ctime < nt)
		{
			NetSocket* sock = GetLayer()->GetLayerMsg<NetSocket>();
			sock->socket = it->second->socket;
			m_msgModule->SendMsg(L_SOCKET_CLOSE, sock);
			m_objects_tmp.erase(it++);
		}
		else
			it++;
	}
}

void NetObjectModule::CheckReconnect()
{
	if (m_serverTmp.size() == 0)
		return;
	auto nt = GetSecend();
	if (nt < m_lastTime)
		return;
	m_lastTime = nt+2;//2 secend check once
	for (auto& it:m_serverTmp)
	{
		auto msg = GetLayer()->GetLayerMsg<NetServer>();
		*msg = *it.second;
		m_msgModule->SendMsg(L_TO_CONNET_SERVER, msg);
	}
}

int64_t NetObjectModule::GetSerTypeId64(const int32_t & stype, const int32_t & sid)
{
	int64_t id = stype;
	id <<= 32;
	return id | sid;
}
