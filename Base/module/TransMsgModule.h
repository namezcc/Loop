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
	typedef std::vector<SHARE<ServerNode>> VecPath;
	typedef std::function<void()> ReqFail;

	SHARE<NetServer> GetServerConn(const int32_t& sock);
	void SendToServer(ServerNode& ser, const int32_t& mid, BuffBlock* buff);
	void SendToServer(ServerNode& ser, const int32_t& mid, google::protobuf::Message& msg);
	void SendToAllServer(const int32_t& stype, const int32_t& mid, google::protobuf::Message& msg);
	void SendToServer(vector<SHARE<ServerNode>>& path, const int32_t& mid, google::protobuf::Message& msg);
	void SendToServer(vector<SHARE<ServerNode>>& path, const int32_t& mid,BuffBlock* buff);
	void SendBackServer(vector<SHARE<ServerNode>>& path, const int32_t& mid, google::protobuf::Message& msg);

	SHARE<BaseMsg> RequestServerAsynMsg(ServerNode& ser, const int32_t& mid, BuffBlock* buff,c_pull& pull,SHARE<BaseCoro>& coro);
	SHARE<BaseMsg> RequestServerAsynMsg(ServerNode& ser, const int32_t& mid, gpb::Message& msg,c_pull& pull,SHARE<BaseCoro>& coro);
	SHARE<BaseMsg> RequestServerAsynMsg(VecPath& path, const int32_t& mid, BuffBlock* buff,c_pull& pull,SHARE<BaseCoro>& coro);
	SHARE<BaseMsg> RequestServerAsynMsg(VecPath& path, const int32_t& mid, gpb::Message& msg,c_pull& pull,SHARE<BaseCoro>& coro);
	SHARE<BaseMsg> RequestBackServerAsynMsg(VecPath& path, const int32_t& mid, gpb::Message& msg,c_pull& pull,SHARE<BaseCoro>& coro);

	SHARE<BaseMsg> RequestServerAsynMsg(ServerNode& ser, const int32_t& mid, gpb::Message& msg, c_pull& pull, SHARE<BaseCoro>& coro, const ReqFail& failCall);
	SHARE<BaseMsg> RequestServerAsynMsg(VecPath& path, const int32_t& mid, gpb::Message& msg, c_pull& pull, SHARE<BaseCoro>& coro,const ReqFail& failCall);

	SHARE<BaseMsg> ResponseServerAsynMsg(ServerNode& ser, SHARE<BaseMsg>& comsg, BuffBlock* buff,c_pull& pull,SHARE<BaseCoro>& coro);
	SHARE<BaseMsg> ResponseServerAsynMsg(ServerNode& ser, SHARE<BaseMsg>& comsg, gpb::Message& msg,c_pull& pull,SHARE<BaseCoro>& coro);
	SHARE<BaseMsg> ResponseServerAsynMsg(VecPath& path, SHARE<BaseMsg>& comsg, BuffBlock* buff,c_pull& pull,SHARE<BaseCoro>& coro);
	SHARE<BaseMsg> ResponseServerAsynMsg(VecPath& path, SHARE<BaseMsg>& comsg, gpb::Message& msg,c_pull& pull,SHARE<BaseCoro>& coro);
	SHARE<BaseMsg> ResponseBackServerAsynMsg(VecPath& path, SHARE<BaseMsg>& comsg, gpb::Message& msg,c_pull& pull,SHARE<BaseCoro>& coro);

	void ResponseServerMsg(ServerNode& ser, SHARE<BaseMsg>& comsg, BuffBlock* buff);
	void ResponseServerMsg(ServerNode& ser, SHARE<BaseMsg>& comsg, gpb::Message& msg);
	void ResponseServerMsg(VecPath& path, SHARE<BaseMsg>& comsg, BuffBlock* buff);
	void ResponseServerMsg(VecPath& path, SHARE<BaseMsg>& comsg, gpb::Message& msg);
	void ResponseBackServerMsg(VecPath& path, SHARE<BaseMsg>& comsg, gpb::Message& msg);

	BuffBlock* EncodeCoroMsg(BuffBlock* buff, const int32_t& mid, const int32_t& coid, const int32_t& mycoid = 0);
	inline std::unordered_map<int32_t, std::unordered_map<int32_t, SHARE<NetServer>>>& GetServerList() { return m_serverList; };
protected:
	virtual void Init() override;
	virtual void Execute() override;
	//gen server net struct
	void InitServerNet();

	void OnServerConnect(SHARE<NetServer>& ser);
	void OnServerClose(SHARE<NetServer>& ser);
	void RemoveRand(const int32_t& stype, const int32_t& sid);

	void TransMsgToServer(vector<SHARE<ServerNode>>& sers,const int32_t& mid, google::protobuf::Message& pbmsg);
	void TransMsgToServer(vector<SHARE<ServerNode>>& sers, const int32_t& mid,BuffBlock* buffblock);

	int32_t GetPathSize(vector<SHARE<ServerNode>>& sers);

	void OnGetTransMsg(NetMsg* nmsg);

	NetServer* GetServer(const int32_t& type, const int32_t& serid);

	void GetTransPath(ServerNode& beg, ServerNode& end, vector<SHARE<ServerNode>>& path);
	bool GetToPath(vector<SHARE<ServerNode>>& path);
	BuffBlock* PathToBuff(vector<SHARE<ServerNode>>& path,const int32_t& mid, const int32_t& toindex=1);
private:
	EventModule* m_eventModule;
	NetObjectModule* m_netObjMod;
	MsgModule* m_msgModule;

	std::unordered_map<int32_t, std::unordered_map<int32_t, SHARE<NetServer>>> m_serverList;
	std::unordered_map<int32_t, SHARE<NetServer>> m_allServer;//sock -> ser
	std::unordered_map<int32_t, std::vector<NetServer*>> m_randServer;

	map<string, int32_t> m_serverType;
	map<string, list<vector<int32_t>>> m_serverPath;
	map<int32_t, map<int32_t, int32_t>> m_serverLink;
};

#endif