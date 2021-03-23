#ifndef TCP_ASIO_SESSION_MODULE
#define TCP_ASIO_SESSION_MODULE

#include "BaseModule.h"
#include <boost/asio.hpp>
#include "ProtoDefine.h"

using boost::asio::ip::tcp;
namespace as = boost::asio;

class MsgModule;
class Protocol;

struct AsioSession:public LoopObject
{
	AsioSession()
	{
		m_sockId = ++AsioSession::SOCKET;
	}

	virtual void init(FactorManager* fm)
	{
		m_close = false;
	}

	virtual void recycle(FactorManager* fm)
	{
		m_buff.recycle(fm);
		m_decodeBuff.Clear();
	}

	SHARE<tcp::socket> m_sock;
	int32_t m_sockId;
	LocalBuffBlock m_buff;
	NetBuffer m_decodeBuff;
	bool m_close;
	

	static thread_local int32_t SOCKET;
};

class TcpAsioSessionModule:public BaseModule
{
public:
	TcpAsioSessionModule(BaseLayer*l);
	~TcpAsioSessionModule();

	int32_t AddNewSession(tcp::socket& sock,bool clien = true);
	void SetProtoType(ProtoType ptype);
	void SetBind(int port)
	{
		m_accptor = std::make_unique<tcp::acceptor>(m_context, tcp::endpoint(tcp::v4(), port));
	}
private:

	// 通过 BaseModule 继承
	virtual void Init() override;
	virtual void AfterInit() override;
	virtual void Execute() override;
	void DoAccept();

	void OnCloseSocket(NetMsg* msg);
	void OnSocketSendData(NetMsg* nMsg);
	void OnBroadData(BroadMsg* nMsg);
	void OnConnectServer(NetServer * ser);

	void CombinBuff(NetMsg * nMsg);

	void DoReadData(AsioSession* session);
	void CloseSession(const int32_t& sock,bool active = false);

private:

	Protocol * m_proto;

	MsgModule* m_msgModule;
	LocalBuffBlock m_sendBuff;

	as::io_context m_context;
	std::unique_ptr<tcp::acceptor> m_accptor;
	//std::unordered_map<int32_t, SHARE<AsioSession>> m_session;
	AsioSession* m_session[MAX_CLIENT_CONN];
	std::list<int32_t> m_sock_pool;
};

#endif