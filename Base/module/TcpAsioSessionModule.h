#ifndef TCP_ASIO_SESSION_MODULE
#define TCP_ASIO_SESSION_MODULE

#include "BaseModule.h"
#include <boost/asio.hpp>
#include "ProtoDefine.h"
#include "io_pool.h"

using boost::asio::ip::tcp;
namespace as = boost::asio;

class MsgModule;
class Protocol;

template<typename T, typename... Args>
std::unique_ptr<T> make_unique_m(Args&&... args) {
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

struct AsioSession:public LoopObject
{
	AsioSession()
	{
		m_sockId = 0;
		m_active = false;
	}

	virtual void init(FactorManager* fm)
	{
		m_active = false;
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
	bool m_active;
};

class TcpAsioSessionModule:public BaseModule
{
public:
	TcpAsioSessionModule(BaseLayer*l);
	~TcpAsioSessionModule();

	static std::string getLocalIp();

	int32_t AddNewSession(const std::shared_ptr<tcp::socket>& sock,bool clien = true);
	void SetProtoType(ProtoType ptype);
	void SetBind(int port)
	{
		m_accptor = make_unique_m<tcp::acceptor>(m_context, tcp::endpoint(tcp::v4(), port));
	}
private:

	// 通过 BaseModule 继承
	virtual void Init() override;
	virtual void AfterInit() override;
	virtual void Execute() override;
	void DoAccept();
	void testHandle(boost::system::error_code ec,SHARE<AsioSession>& ss);

	void OnCloseSocket(NetMsg* msg);
	void OnSocketSendData(NetMsg* nMsg);
	void OnBroadData(BroadMsg* nMsg);
	void OnConnectServer(NetServer * ser);

	void DoReadData(SHARE<AsioSession> session);
	void CloseSession(const int32_t& sock,bool active = false);
	void pushMsg(NetMsg* msg);
	void sendMsgToLayer();

	void pushCloseSock(int32_t sock);
	void extureCloseSock();
private:

	Protocol * m_proto;

	MsgModule* m_msgModule;

	as::io_context m_context;
	std::unique_ptr<tcp::acceptor> m_accptor;
	//std::unordered_map<int32_t, SHARE<AsioSession>> m_session;
	AsioSession* m_session[MAX_CLIENT_CONN];
	std::list<int32_t> m_sock_pool;
	io_service_pool m_io_pool;
	NetMsg* m_send_msg_head;
	NetMsg* m_send_msg_tail;
	std::mutex m_msg_mutex;

	NetMsg* m_close_list;
	std::mutex m_close_mutex;
};

#endif