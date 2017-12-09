#ifndef TRANS_MSG_MODULE_H
#define TRANS_MSG_MODULE_H
#include "BaseModule.h"

class EventModule;
class NetObjectModule;
class MsgModule;

class TransMsgModule:public BaseModule
{
public:
	TransMsgModule(BaseLayer* l);
	~TransMsgModule();

protected:
	virtual void Init() override;
	virtual void Execute() override;


	void OnServerConnect(const int& nEvent, NetServer* ser);
	void OnServerClose(const int& nEvent, NetServer* ser);

	void TransMsgToServer(vector<ServerNode*>& sers,const int& mid,const int& len, char* msg);

	void OnGetTransMsg(NetMsg* nmsg);

	NetServer* GetServer(const int& type, const int& serid);
private:
	EventModule* m_eventModule;
	NetObjectModule* m_netObjMod;
	MsgModule* m_msgModule;

	map<int, map<int, NetServer*>> m_serverList;
};

#endif