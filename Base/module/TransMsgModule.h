#ifndef TRANS_MSG_MODULE_H
#define TRANS_MSG_MODULE_H
#include "BaseModule.h"

class EventModule;
class NetObjectModule;
class MsgModule;

class LOOP_EXPORT TransMsgModule:public BaseModule
{
public:
	TransMsgModule(BaseLayer* l);
	~TransMsgModule();

	SHARE<NetServer> GetServerConn(const int& sock);
	void SendToServer(ServerNode& ser, const int& mid, char* msg,const int& len);
	void SendToServer(ServerNode& ser, const int& mid, google::protobuf::Message& msg);
	void SendToAllServer(const int& stype, const int& mid, google::protobuf::Message& msg);
	void SendToServer(vector<SHARE<ServerNode>>& path, const int& mid, google::protobuf::Message& msg,const int& toidx=1);
	void SendToServer(vector<SHARE<ServerNode>>& path, const int& mid,const char* msg,const int& len, const int& toidx = 1);
	void SendBackServer(vector<SHARE<ServerNode>>& path, const int& mid, google::protobuf::Message& msg);
protected:
	virtual void Init() override;
	virtual void Execute() override;
	//gen server net struct
	void InitServerNet();

	void OnServerConnect(SHARE<NetServer>& ser);
	void OnServerClose(SHARE<NetServer>& ser);

	void TransMsgToServer(vector<SHARE<ServerNode>>& sers,const int& mid, google::protobuf::Message& pbmsg, const int& toidx = 1);
	void TransMsgToServer(vector<SHARE<ServerNode>>& sers, const int& mid,char* msg,const int& len,const int& toidx=1);

	int GetPathSize(vector<SHARE<ServerNode>>& sers);

	void OnGetTransMsg(NetMsg* nmsg);

	NetServer* GetServer(const int& type, const int& serid);

	void GetTransPath(ServerNode& beg, ServerNode& end, vector<SHARE<ServerNode>>& path);
	bool GetToPath(vector<SHARE<ServerNode>>& path);
private:
	EventModule* m_eventModule;
	NetObjectModule* m_netObjMod;
	MsgModule* m_msgModule;

	map<int, map<int, SHARE<NetServer>>> m_serverList;
	map<int, SHARE<NetServer>> m_allServer;//sock -> ser

	map<string, int> m_serverType;
	map<string, list<vector<int>>> m_serverPath;
	map<int, map<int, int>> m_serverLink;
};

#endif