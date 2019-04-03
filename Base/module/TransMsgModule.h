#ifndef TRANS_MSG_MODULE_H
#define TRANS_MSG_MODULE_H
#include "BaseModule.h"

class EventModule;
class NetObjectModule;
class MsgModule;
class ScheduleModule;

class LOOP_EXPORT TransMsgModule:public BaseModule
{
public:
	TransMsgModule(BaseLayer* l);
	~TransMsgModule();
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
	VecPath GetFromSelfPath(const int32_t& allSize, const int32_t& stype, const int32_t& sid = 0);
protected:
	virtual void Init() override;
	virtual void BeforExecute() override;
	virtual void Execute() override;

	void AddServerConn(const NetServer& ser);
	void OnServerConnect(SHARE<NetServer>& ser);
	void OnServerRegiste(NetMsg* msg);
	void OnServerClose(int32_t sock);
	void RemoveRand(const int32_t& stype, const int32_t& sid);

	void TransMsgToServer(vector<SHARE<ServerNode>>& sers,const int32_t& mid, google::protobuf::Message& pbmsg);
	void TransMsgToServer(vector<SHARE<ServerNode>>& sers, const int32_t& mid,BuffBlock* buffblock);

	int32_t GetPathSize(vector<SHARE<ServerNode>>& sers);

	void OnGetTransMsg(NetMsg* nmsg);

	NetServer* GetServer(const int32_t& type, const int32_t& serid);

	BuffBlock* PathToBuff(vector<SHARE<ServerNode>>& path,const int32_t& mid, const int32_t& toindex=1);
private:
	EventModule* m_eventModule;
	NetObjectModule* m_netObjMod;
	MsgModule* m_msgModule;
	ScheduleModule* m_schedule;

	std::unordered_map<int32_t, std::unordered_map<int32_t, SHARE<NetServer>>> m_serverList;
	std::unordered_map<int32_t, SHARE<NetServer>> m_allServer;//sock -> ser
	std::unordered_map<int32_t, std::vector<NetServer*>> m_randServer;

};

#endif