#include "NetObjectModule.h"
#include "MsgModule.h"
#include "EventModule.h"

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
	m_msgModule->AddMsgCallBack<NetSocket>(L_SERVER_CLOSE, this, &NetObjectModule::OnServerClose);
	m_msgModule->AddMsgCallBack<NetMsg>(N_REGISTE_SERVER, this, &NetObjectModule::OnServerRegiste);
	
}

void NetObjectModule::Execute()
{
	CheckReconnect();
}

void NetObjectModule::OnSocketConnet(NetSocket * sock)
{
	auto netobj = GetLayer()->GetLoopObj<NetObject>();
	netobj->socket = sock->socket;
	
	m_objects_tmp[sock->socket] = netobj;
}

void NetObjectModule::OnSocketClose(NetSocket * sock)
{
	auto it = m_objects.find(sock->socket);
	if (it != m_objects.end())
	{
		if (it->second->type == SERVER)
		{
			m_eventModule->SendEvent(E_SERVER_CLOSE, (NetServer*)it->second->data);
			GetLayer()->Recycle((NetServer*)it->second->data);
		}
		GetLayer()->Recycle(it->second);
		m_objects.erase(it);
		return;
	}

	it = m_objects_tmp.find(sock->socket);
	if (it != m_objects_tmp.end())
	{
		GetLayer()->Recycle(it->second);
		m_objects_tmp.erase(it);
	}
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

void NetObjectModule::CloseNetObject(const int& socket)
{
	auto it = m_objects.find(socket);
	if (it == m_objects.end())
		return;

	NetSocket* sock = new NetSocket(socket);

	m_msgModule->SendMsg(L_SOCKET_CLOSE, sock);

	GetLayer()->Recycle(it->second);
	m_objects.erase(it);
}

void NetObjectModule::AddServerConn(const int& sType, const int& sid, const std::string& ip, const int& port)
{
	auto ser = GetLayer()->GetLoopObj<NetServer>();
	ser->type = sType;
	ser->ip = ip;
	ser->port = port;
	ser->serid = sid;
	ser->state = CONN_STATE::CLOSE;
	ser->socket = -1;
	m_serverTmp[ser->serid] = ser;
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
	auto netobj = GetLayer()->GetLoopObj<NetObject>();
	netobj->socket = ser->socket;
	m_objects[netobj->socket] = netobj;
	//通知连接成功
	m_eventModule->SendEvent(E_SERVER_CONNECT, it->second);
	m_serverTmp.erase(it);

	//发送注册消息
	LPMsg::ServerInfo xMsg;
	xMsg.set_id(ser->serid);
	xMsg.set_type(ser->type);
	int msize;
	auto msg = PB::PBToChar(xMsg, msize);
	//ServerNode* xMsg = new ServerNode();
	//xMsg->serid = ser->serid;
	//xMsg->type = ser->type;
	SendNetMsg(ser->socket, msg, N_REGISTE_SERVER, msize);
}

void NetObjectModule::OnServerClose(NetSocket* ser)
{
	auto it = m_serverConn.find(ser->socket);
	if (it == m_serverConn.end())
		return;
	it->second->socket = -1;
	it->second->state = CONN_STATE::CLOSE;
	m_serverTmp[it->second->serid] = it->second;
	//从 m_object里删除
	m_objects.erase(ser->socket);
	//事件通知
	m_eventModule->SendEvent(E_SERVER_CLOSE, it->second);
	m_serverConn.erase(it);
}

void NetObjectModule::OnServerRegiste(NetMsg* msg)
{
	auto it = m_objects_tmp.find(msg->socket);
	if (it == m_objects_tmp.end())
		return;

	m_objects[msg->socket] = it->second;
	m_objects_tmp.erase(it);
	//用 proto 替换
	LPMsg::ServerInfo xMsg;
	xMsg.ParseFromArray(msg->msg, msg->len);
	//ServerNode* sernode = (ServerNode*)msg->msg;
	NetServer* server = GetLayer()->GetLoopObj<NetServer>();

	server->serid = xMsg.id();
	server->type = xMsg.type();
	server->socket = msg->socket;
	server->state = CONN_STATE::CONNECT;
	m_eventModule->SendEvent(E_SERVER_CONNECT, server);

	cout << "Server registe type:" << server->type << " ID:" << server->serid << endl;
}

void NetObjectModule::CheckReconnect()
{
	auto nt = GetSecend();
	if (nt <= m_lastTime)
		return;
	m_lastTime = nt;
	for (auto& it:m_serverTmp)
	{
		auto msg = new NetServer(*it.second);
		m_msgModule->SendMsg(L_TO_CONNET_SERVER, msg);
	}
}