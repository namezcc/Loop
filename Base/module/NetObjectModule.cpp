#include "NetObjectModule.h"
#include "MsgModule.h"
#include "EventModule.h"
#include "LoopServer.h"

NetObjectModule::NetObjectModule(BaseLayer* l):BaseModule(l)
{
}


NetObjectModule::~NetObjectModule()
{
}

void NetObjectModule::Init()
{
	m_msgModule = GetLayer()->GetModule<MsgModule>();
	m_eventModule = GetLayer()->GetModule<EventModule>();

	m_msgModule->AddMsgCallBack<NetSocket>(L_SOCKET_CONNET,this,&NetObjectModule::OnSocketConnet);
	m_msgModule->AddMsgCallBack<NetSocket>(L_SOCKET_CLOSE, this, &NetObjectModule::OnSocketClose);
	m_msgModule->AddMsgCallBack<NetServer>(L_SERVER_CONNECTED, this, &NetObjectModule::OnServerConnet);
	m_msgModule->AddMsgCallBack<NetMsg>(N_REGISTE_SERVER, this, &NetObjectModule::OnServerRegiste);
	
	m_msgModule->AddMsgCallBack<NetServer>(L_PHP_CGI_CONNECTED, this, &NetObjectModule::OnPHPCgiConnect);

	m_eventModule->AddEventCallBack(E_CLIENT_HTTP_CONNECT, this, &NetObjectModule::OnHttpClientConnect);
	
	m_outTime = 5;
}

void NetObjectModule::BeforExecute()
{
	auto config = GetLayer()->GetLoopServer()->GetConfig();

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

void NetObjectModule::AcceptConn(const int & socket)
{
	auto it = m_objects_tmp.find(socket);
	if (it == m_objects_tmp.end())
		return;

	m_objects[socket] = it->second;
	m_objects_tmp.erase(it);
}

void NetObjectModule::SendNetMsg(const int & socket, const int & mid, google::protobuf::Message & pbmsg)
{
	NetMsg* nMsg = new NetMsg();
	nMsg->socket = socket;
	nMsg->mid = mid;
	nMsg->msg = PB::PBToChar(pbmsg, nMsg->len,PACK_HEAD_SIZE);
	m_msgModule->SendMsg(L_SOCKET_SEND_DATA, nMsg);
}

void NetObjectModule::SendNetMsg(const int& socket, char* msg, const int& mid, const int& len)
{
	//m_object 没加判断 ... 不需要
	NetMsg* nMsg = new NetMsg();
	nMsg->socket = socket;
	nMsg->mid = mid;
	nMsg->len = len;
	nMsg->msg = msg;

	m_msgModule->SendMsg(L_SOCKET_SEND_DATA, nMsg);
}

void NetObjectModule::SendHttpMsg(const int& socket, NetBuffer& buf)
{
	NetMsg* nMsg = new NetMsg();
	nMsg->socket = socket;
	nMsg->len = buf.use;
	nMsg->msg = buf.buf;

	buf.buf = nullptr;
	m_msgModule->SendMsg(L_SOCKET_SEND_HTTP_DATA, nMsg);
}

void NetObjectModule::CloseNetObject(const int& socket)
{
	auto it = m_objects.find(socket);
	if (it == m_objects.end())
		return;

	NetSocket* sock = new NetSocket(socket);

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
	m_serverTmp[ser->serid] = ser;
}

void NetObjectModule::ConnectPHPCgi(NetServer& cgi)
{
	auto msg = new NetServer(cgi);
	m_msgModule->SendMsg(L_CONNECT_PHP_CGI, msg);
}

void NetObjectModule::OnServerConnet(NetServer* ser)
{
	auto it = m_serverTmp.find(ser->serid);
	if (it == m_serverTmp.end())
		return;

	it->second->socket = ser->socket;
	it->second->state = CONN_STATE::CONNECT;

	m_serverConn[ser->socket] = it->second;
	//添加进 m_object 里
	auto netobj = GetLayer()->GetSharedLoop<NetObject>();
	netobj->socket = ser->socket;
	netobj->type = CONN_SERVER;
	m_objects[netobj->socket] = netobj;

	//发送注册消息
	auto myser = GetLayer()->GetServer();
	LPMsg::ServerInfo xMsg;
	xMsg.set_id(myser->serid);
	xMsg.set_type(myser->type);
	SendNetMsg(ser->socket, N_REGISTE_SERVER, xMsg);

	//通知连接成功
	m_eventModule->SendEvent(E_SERVER_CONNECT, it->second);
	m_serverTmp.erase(it);
}

void NetObjectModule::ServerClose(const int& socket)
{
	auto it = m_serverConn.find(socket);
	if (it == m_serverConn.end())
		return;
	it->second->socket = -1;
	it->second->state = CONN_STATE::CLOSE;
	m_serverTmp[it->second->serid] = it->second;
	//从 m_object里删除
	//m_objects.erase(socket); 已經刪了
	//事件通知
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
	
	//待优化
	//LPMsg::ServerInfo xMsg;
	//xMsg.ParseFromArray(msg->msg, msg->len);
	TRY_PARSEPB(LPMsg::ServerInfo,msg,m_msgModule)
	
	auto server = GetLayer()->GetSharedLoop<NetServer>();

	server->serid = pbMsg.id();
	server->type = pbMsg.type();
	server->socket = msg->socket;
	server->state = CONN_STATE::CONNECT;
	m_eventModule->SendEvent(E_SERVER_CONNECT, server);

	LP_WARN(m_msgModule) << "Server registe type:" << server->type << " ID:" << server->serid;
	//cout << "Server registe type:" << server->type << " ID:" << server->serid << endl;
}

void NetObjectModule::OnPHPCgiConnect(NetServer* ser)
{
	//添加进 m_object 里
	auto netobj = GetLayer()->GetSharedLoop<NetObject>();
	netobj->socket = ser->socket;
	netobj->type = CONN_PHP_CGI;
	m_objects[netobj->socket] = netobj;
	//通知连接成功
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
	m_tmpObjTime = nt + 1;
	for (auto it=m_objects_tmp.begin();it!=m_objects_tmp.end();)
	{
		if (it->second->ctime < nt)
		{
			NetSocket* sock = new NetSocket(it->second->socket);
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
		auto msg = new NetServer(*it.second);
		m_msgModule->SendMsg(L_TO_CONNET_SERVER, msg);
	}
}