#ifndef LOOP_SERVER_H
#define LOOP_SERVER_H
//#include "BaseLayer.h"
#include "ThreadPool.h"
#include "MsgDefine.h"
#include "MsgPool.h"

class BaseLayer;

struct SqlInfo
{
	string ip;
	int port;
	string db;
	string user;
	string pass;
	int dbGroup;
};

struct ServerConfigInfo
{
	int32_t type;
	int32_t server_id;
	std::string ip;
	int32_t port;
};

struct ServerConfig
{
	NetServer addr;
	NetServer udpAddr;
	SqlInfo sql;
	SqlInfo redis;
};

struct ConnRule
{
	int32_t server_type;
	int32_t to_server_type;
	int32_t conn_type;
	int32_t param;
};

enum ServerState
{
	SERR_LINK = 1,
	SERR_REDIS,
	SERR_SQL,
};

class LOOP_EXPORT LoopServer
{
public:
	LoopServer();
	~LoopServer();

	void InitServer(int argc, char** args);

	template<typename T,typename... Args>
	typename std::enable_if<std::is_base_of<BaseLayer,T>::value,T*>::type CreateLayer(Args&&... args)
	{
		auto l = SHARE<T>(new T(std::forward<Args>(args)...));
		l->SetlsIndex((int32_t)m_layers.size());
		m_layers.push_back(l);
		return l.get();
	}

	void BuildPipe(BaseLayer* l1, BaseLayer* l2);

	void Run();
	void Loop();

	inline ServerConfig& GetConfig() { return m_config; };

	template<typename T>
	T* popMsg(int32_t index)
	{
		return MsgPool::popMsg<T>();
	}
	//void recycle(int32_t index, BaseData* msg);
	std::vector<ServerConfigInfo> getConnectServer();
	std::vector<ServerConfigInfo> getConnectServer(int32_t type,int32_t id, std::map<int32_t, std::vector<ServerConfigInfo>>& allser);


	int m_port;
	const ServerNode& getServerNode() { return m_server; }
	void setStop(int32_t* stop) { m_stop = stop; }
	bool stopServer() { return (*m_stop) > 0; }
	int32_t getServerState() { return m_server_state; }
	void setServerState(int32_t bit, bool err);
protected:
	void Init(const int& stype, const int& serid);
	void InitConfig();
	void InitServerConfig();
	void InitConnectRule();
	void InitLogLayer();//
	//void InitMsgPool();
private:
	SHARE<ThreadPool> m_pool;
	vector<SHARE<BaseLayer>> m_layers;
	ServerNode m_server;
	ServerConfig m_config;

	//RecyclePool* m_recycle;
	std::map<int32_t, std::vector<ServerConfigInfo>> m_all_server;
	std::vector<ConnRule> m_connect_rule;

	bool m_over;
	int32_t* m_stop;

	int32_t m_server_state;
};

#endif