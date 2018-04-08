#ifndef LOOP_SERVER_H
#define LOOP_SERVER_H
#include "BaseLayer.h"
#include "ThreadPool.h"
#include "MsgDefine.h"

struct SqlInfo
{
	string ip;
	int port;
	string db;
	string user;
	string pass;
	int dbGroup;
};

struct ServerConfig
{
	NetServer addr;
	vector<NetServer> connect;
	SqlInfo sql;
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
		m_layers.push_back(l);
		return l.get();
	}

	void BuildPipe(BaseLayer* l1, BaseLayer* l2);

	void Run();

	inline ServerConfig& GetConfig() { return m_config; };

	int m_port;
protected:
	void Init(const int& stype, const int& serid);
	void InitConfig();
	void InitLogLayer();//loglayerĬ�ϴ��� ������layer����
private:
	SHARE<ThreadPool> m_pool;
	vector<SHARE<BaseLayer>> m_layers;
	SHARE<FactorManager> m_factor;
	ServerNode m_server;
	ServerConfig m_config;
};

#endif