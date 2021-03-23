#ifndef MET_OBJECT_MODULE_H
#define MET_OBJECT_MODULE_H
#include "BaseModule.h"
class MsgModule;
class EventModule;
class TransMsgModule;

typedef std::function<void(bool, NetServer&)> ConnectServerRes;

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
	void BroadNetMsg(const std::vector<int32_t>& socks, const int32_t & mid, gpb::Message& pbmsg);
	void BroadNetMsg(const std::vector<int32_t>& socks, const int32_t & mid, BuffBlock* buff);

	SHARE<BaseMsg> ResponseAsynMsg(const int32_t& socket, SHARE<BaseMsg>& comsg, gpb::Message& pbmsg, c_pull& pull, SHARE<BaseCoro>& coro);
	SHARE<BaseMsg> ResponseAsynMsg(const int32_t& socket, SHARE<BaseMsg>& comsg, BuffBlock* buff, c_pull& pull, SHARE<BaseCoro>& coro);
	void ResponseMsg(const int32_t& socket, SHARE<BaseMsg>& comsg, gpb::Message& pbmsg);
	void ResponseMsg(const int32_t& socket, SHARE<BaseMsg>& comsg, BuffBlock* buff);

	void SendHttpMsg(const int& socket,NetBuffer& buf);
	//void AddServerConn(const int& sType,const int& sid, const std::string& ip, const int& port);
	void ConnectServer(const NetServer& ser, const ConnectServerRes& call); //can not in ConnectServerRes Call ConnectServer again !!!
	void CloseNetObject(const int& socket);
	inline void SetAccept(bool _accept, int32_t _type) { m_acceptNoCheck = _accept; m_noCheckType = _type; };
protected:

	void OnSocketConnet(NetMsg* sock);
	void OnSocketClose(NetMsg* sock);
	void OnServerConnet(NetServer* ser);

	void NoticeSocketClose(NetObject* obj);

	void CheckOutTime();

private:
	MsgModule* m_msgModule;
	EventModule* m_eventModule;
	TransMsgModule* m_transModule;

	std::unordered_map<int, SHARE<NetObject>> m_objects_tmp;
	std::unordered_map<int, SHARE<NetObject>> m_objects;

	int64_t m_lastTime;
	int64_t m_tmpObjTime;
	int m_outTime;
	bool m_acceptNoCheck;
	int32_t m_noCheckType;
	std::map<std::string, ConnectServerRes> m_tempServer;
};

#endif