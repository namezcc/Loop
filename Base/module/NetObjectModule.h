#ifndef MET_OBJECT_MODULE_H
#define MET_OBJECT_MODULE_H
#include "BaseModule.h"
class MsgModule;
class EventModule;
class TransMsgModule;

class LOOP_EXPORT NetObjectModule:public BaseModule
{
public:
	NetObjectModule(BaseLayer* l);
	~NetObjectModule();

	// ͨ�� BaseModule �̳�
	virtual void Init() override;
	virtual void BeforExecute() override;
	virtual void Execute() override;

	void AcceptConn(const int& socket,const int32_t& connType = CONN_CLIENT);
	void SendNetMsg(const int& socket,const int& mid,google::protobuf::Message& pbmsg);
	void SendNetMsg(const int& socket,const int32_t & mid, BuffBlock* buff);

	SHARE<BaseMsg> ResponseAsynMsg(const int32_t& socket, SHARE<BaseMsg>& comsg, gpb::Message& pbmsg, c_pull& pull, SHARE<BaseCoro>& coro);
	SHARE<BaseMsg> ResponseAsynMsg(const int32_t& socket, SHARE<BaseMsg>& comsg, BuffBlock* buff, c_pull& pull, SHARE<BaseCoro>& coro);
	void ResponseMsg(const int32_t& socket, SHARE<BaseMsg>& comsg, gpb::Message& pbmsg);
	void ResponseMsg(const int32_t& socket, SHARE<BaseMsg>& comsg, BuffBlock* buff);

	void SendHttpMsg(const int& socket,NetBuffer& buf);
	void AddServerConn(const int& sType,const int& sid, const std::string& ip, const int& port);
	void ConnectPHPCgi(NetServer& cgi);
	void CloseNetObject(const int& socket);
	inline void SetAccept(bool _accept) { m_acceptNoCheck = _accept; };
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
	int64_t GetSerTypeId64(const int32_t& stype, const int32_t& sid);

private:
	MsgModule* m_msgModule;
	EventModule* m_eventModule;
	TransMsgModule* m_transModule;

	std::unordered_map<int, SHARE<NetObject>> m_objects_tmp;
	std::unordered_map<int, SHARE<NetObject>> m_objects;

	std::unordered_map<int64_t, SHARE<NetServer>> m_serverTmp;//stype|sid ->
	std::unordered_map<int, SHARE<NetServer>> m_serverConn;//sock->
	int64_t m_lastTime;
	int64_t m_tmpObjTime;
	int m_outTime;
	bool m_acceptNoCheck;
};

#endif