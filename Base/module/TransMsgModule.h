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
	void SendToServer(const ServerNode& ser, const int32_t& mid, BuffBlock* buff);
	void SendToServer(const ServerNode& ser, const int32_t& mid, const google::protobuf::Message& msg);
	void SendToAllServer(const int32_t& stype, const int32_t& mid, const google::protobuf::Message& msg);
	void SendToAllServer(const int32_t& stype, const int32_t& mid, BuffBlock* buff);
	void SendToServer(ServerPath& path, const int32_t& mid, const google::protobuf::Message& msg);
	void SendToServer(ServerPath& path, const int32_t& mid,BuffBlock* buff);
	void SendBackServer(ServerPath& path, const int32_t& mid,const google::protobuf::Message& msg);
	void SendBackServer(ServerPath& path, const int32_t& mid, BuffBlock* buff);

	NetMsg* RequestServerAsynMsg(const ServerNode& ser, const int32_t& mid, BuffBlock* buff,c_pull& pull,SHARE<BaseCoro>& coro);
	NetMsg* RequestServerAsynMsg(const ServerNode& ser, const int32_t& mid, const gpb::Message& msg,c_pull& pull,SHARE<BaseCoro>& coro);
	NetMsg* RequestServerAsynMsg(ServerPath& path, const int32_t& mid, BuffBlock* buff,c_pull& pull,SHARE<BaseCoro>& coro);
	NetMsg* RequestServerAsynMsg(ServerPath& path, const int32_t& mid, const gpb::Message& msg,c_pull& pull,SHARE<BaseCoro>& coro);
	NetMsg* RequestBackServerAsynMsg(ServerPath& path, const int32_t& mid, const gpb::Message& msg,c_pull& pull,SHARE<BaseCoro>& coro);

	NetMsg* RequestServerAsynMsg(const ServerNode& ser, const int32_t& mid, const gpb::Message& msg, c_pull& pull, SHARE<BaseCoro>& coro, const ReqFail& failCall);
	NetMsg* RequestServerAsynMsg(ServerPath& path, const int32_t& mid, const gpb::Message& msg, c_pull& pull, SHARE<BaseCoro>& coro,const ReqFail& failCall);

	NetMsg* ResponseServerAsynMsg(const ServerNode& ser, SHARE<BaseMsg>& comsg, BuffBlock* buff,c_pull& pull,SHARE<BaseCoro>& coro);
	NetMsg* ResponseServerAsynMsg(const ServerNode& ser, SHARE<BaseMsg>& comsg, const gpb::Message& msg,c_pull& pull,SHARE<BaseCoro>& coro);
	NetMsg* ResponseServerAsynMsg(ServerPath& path, SHARE<BaseMsg>& comsg, BuffBlock* buff,c_pull& pull,SHARE<BaseCoro>& coro);
	NetMsg* ResponseServerAsynMsg(ServerPath& path, SHARE<BaseMsg>& comsg, const gpb::Message& msg,c_pull& pull,SHARE<BaseCoro>& coro);
	NetMsg* ResponseBackServerAsynMsg(ServerPath& path, SHARE<BaseMsg>& comsg, const gpb::Message& msg,c_pull& pull,SHARE<BaseCoro>& coro);

	void ResponseServerMsg(const ServerNode& ser, SHARE<BaseMsg>& comsg, BuffBlock* buff);
	void ResponseServerMsg(const ServerNode& ser, SHARE<BaseMsg>& comsg, const gpb::Message& msg);
	void ResponseServerMsg(ServerPath& path, SHARE<BaseMsg>& comsg, BuffBlock* buff);
	void ResponseServerMsg(ServerPath& path, SHARE<BaseMsg>& comsg, const gpb::Message& msg);
	void ResponseBackServerMsg(ServerPath& path, SHARE<BaseMsg>& comsg, const gpb::Message& msg);

	BuffBlock* EncodeCoroMsg(BuffBlock* buff, const int32_t& mid, const int32_t& coid, const int32_t& mycoid = 0);
	inline std::unordered_map<int32_t, std::unordered_map<int32_t, SHARE<NetServer>>>& GetServerList() { return m_serverList; };
	ServerPath GetFromSelfPath(const int32_t& allSize, const int32_t& stype, const int32_t& sid = 0);
	NetServer* GetServer(const int32_t& type, const int32_t& serid);
	std::unordered_map<int32_t, SHARE<NetServer>>& getAllServer() { return m_allServer; }
protected:
	virtual void Init() override;
	virtual void BeforExecute() override;
	virtual void Execute() override;

	void AddServerConn(const NetServer& ser);
	void OnServerConnect(SHARE<NetServer>& ser);
	void OnServerRegiste(NetMsg* msg);
	void OnServerClose(int32_t sock);
	void RemoveRand(const int32_t& stype, const int32_t& sid);

	void TransMsgToServer(ServerPath& sers,const int32_t& mid, const google::protobuf::Message& pbmsg);
	void TransMsgToServer(ServerPath& sers, const int32_t& mid,BuffBlock* buffblock);

	int32_t GetPathSize(ServerPath& sers);

	void OnGetTransMsg(NetMsg* nmsg);
	void onConnInfo(NetMsg* nmsg);
	void onGetLinkInfo(SHARE<BaseMsg>& msg);

	BuffBlock* PathToBuff(ServerPath& path,const int32_t& mid, const int32_t& toindex=1);
	void checkSendServerState(int64_t dt);

private:
	EventModule* m_eventModule;
	NetObjectModule* m_netObjMod;
	MsgModule* m_msgModule;
	ScheduleModule* m_schedule;

	std::unordered_map<int32_t, std::unordered_map<int32_t, SHARE<NetServer>>> m_serverList;
	std::unordered_map<int32_t, SHARE<NetServer>> m_allServer;//sock -> ser
	std::unordered_map<int32_t, std::vector<NetServer*>> m_randServer;
	int32_t m_old_state;
};

#endif