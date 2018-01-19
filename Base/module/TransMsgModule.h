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

	void SendToServer(ServerNode& ser, const int& mid, const int& len, char* msg);
protected:
	virtual void Init() override;
	virtual void Execute() override;
	//gen server net struct
	void InitServerNet();

	void OnServerConnect(const int& nEvent, NetServer* ser);
	void OnServerClose(const int& nEvent, NetServer* ser);

	void TransMsgToServer(vector<SHARE<ServerNode>>& sers,const int& mid,const int& len, char* msg);

	void OnGetTransMsg(NetMsg* nmsg);

	NetServer* GetServer(const int& type, const int& serid);

	void GetTransPath(ServerNode& beg, ServerNode& end, vector<SHARE<ServerNode>>& path);
	bool GetToPath(vector<SHARE<ServerNode>>& path);
private:
	EventModule* m_eventModule;
	NetObjectModule* m_netObjMod;
	MsgModule* m_msgModule;

	map<int, map<int, NetServer*>> m_serverList;

	map<string, int> m_serverType;
	map<string, list<vector<int>>> m_serverPath;
	map<int, map<int, int>> m_serverLink;
};

#endif