#include "NetObjectModule.h"
#include "MsgModule.h"

NetObjectModule::NetObjectModule(BaseLayer* l):BaseModule(l)
{
}


NetObjectModule::~NetObjectModule()
{
}

void NetObjectModule::Init()
{
	m_msgModule = GetLayer()->GetModule<MsgModule>();

	m_msgModule->AddMsgCallBack<NetSocket>(L_SOCKET_CONNET,this,&NetObjectModule::OnSocketConnet);
	m_msgModule->AddMsgCallBack<NetSocket>(L_SOCKET_CLOSE, this, &NetObjectModule::OnSocketClose);
	m_msgModule->AddMsgCallBack<NetServer>(L_SERVER_CONNECTED, this, &NetObjectModule::OnServerConnet);
	m_msgModule->AddMsgCallBack<NetSocket>(L_SERVER_CLOSE, this, &NetObjectModule::OnServerClose);
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
		GetLayer()->Recycle(it->second);
		return;
	}

	it = m_objects_tmp.find(sock->socket);
	if (it != m_objects_tmp.end())
	{
		GetLayer()->Recycle(it->second);
	}
}

void NetObjectModule::SendNetMsg(const int& socket, char* msg, const int& mid, const int& len)
{
	//m_object 没加判断
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

void NetObjectModule::AddServerConn(const int& sid, const std::string& ip, const int& port)
{
	auto ser = GetLayer()->GetLoopObj<NetServer>();
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
	assert(it != m_serverTmp.end());
	it->second->socket = ser->socket;
	it->second->state = CONN_STATE::CONNECT;
	m_serverConn[ser->socket] = it->second;
	//添加进 m_object 里
	auto netobj = GetLayer()->GetLoopObj<NetObject>();
	netobj->socket = ser->socket;
	m_objects[netobj->socket] = netobj;

	m_serverTmp.erase(it);
	//通知连接成功


}

void NetObjectModule::OnServerClose(NetSocket* ser)
{
	auto it = m_serverConn.find(ser->socket);
	assert(it != m_serverConn.end());
	it->second->socket = -1;
	it->second->state = CONN_STATE::CLOSE;
	m_serverTmp[it->second->serid] = it->second;
	//从 m_object里删除
	m_objects.erase(ser->socket);

	m_serverConn.erase(it);
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