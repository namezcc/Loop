#ifndef MET_OBJECT_MODULE_H
#define MET_OBJECT_MODULE_H
#include "BaseModule.h"
class MsgModule;

class NetObjectModule:public BaseModule
{
public:
	NetObjectModule(BaseLayer* l);
	~NetObjectModule();

	// Í¨¹ý BaseModule ¼Ì³Ð
	virtual void Init() override;
	virtual void Execute() override;

	void SendNetMsg(const int& socket, char* msg,const int& mid, const int& len);
	void CloseNetObject(const int& socket);

	void AddServerConn(const int& sid, const std::string& ip, const int& port);
protected:

	void OnSocketConnet(NetSocket* sock);
	void OnSocketClose(NetSocket* sock);
	void OnServerConnet(NetServer* ser);
	void OnServerClose(NetSocket* ser);

	void CheckReconnect();
private:
	MsgModule* m_msgModule;
	std::unordered_map<int, NetObject*> m_objects_tmp;
	std::unordered_map<int, NetObject*> m_objects;

	std::unordered_map<int, NetServer*> m_serverTmp;//sid->
	std::unordered_map<int, NetServer*> m_serverConn;//sock->
	int64_t m_lastTime;
};

#endif