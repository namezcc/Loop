#ifndef MET_OBJECT_MODULE_H
#define MET_OBJECT_MODULE_H
#include "BaseModule.h"
class MsgModule;
class EventModule;

class NetObjectModule:public BaseModule
{
public:
	NetObjectModule(BaseLayer* l);
	~NetObjectModule();

	// Í¨¹ý BaseModule ¼Ì³Ð
	virtual void Init() override;
	virtual void Execute() override;

	void SendNetMsg(const int& socket, char* msg,const int& mid, const int& len);
	void SendHttpMsg(const int& socket,NetBuffer& buf);
	void AddServerConn(const int& sType,const int& sid, const std::string& ip, const int& port);
	void ConnectPHPCgi(NetServer& cgi);
	void CloseNetObject(const int& socket);
protected:

	void OnSocketConnet(NetSocket* sock);
	void OnSocketClose(NetSocket* sock);
	void OnServerConnet(NetServer* ser);
	void OnServerRegiste(NetMsg* msg);

	void OnPHPCgiConnect(NetServer* ser);

	void OnHttpClientConnect(const int& socket);

	void NoticeSocketClose(NetObject* obj);
	void ServerClose(const int& socket);

	void CheckOutTime();
	void CheckReconnect();


private:
	MsgModule* m_msgModule;
	EventModule* m_eventModule;

	std::unordered_map<int, SHARE<NetObject>> m_objects_tmp;
	std::unordered_map<int, SHARE<NetObject>> m_objects;

	std::unordered_map<int, SHARE<NetServer>> m_serverTmp;//sid->
	std::unordered_map<int, SHARE<NetServer>> m_serverConn;//sock->
	int64_t m_lastTime;
	int64_t m_tmpObjTime;
	int m_outTime;
};

#endif