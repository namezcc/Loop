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

struct ServerConfig
{
	NetServer addr;
	NetServer udpAddr;
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
		return m_msgPool[index].popMsg<T>();
	}
	void recycle(int32_t index, BaseData* msg);

	int m_port;
protected:
	void Init(const int& stype, const int& serid);
	void InitConfig();
	void InitLogLayer();//loglayerĬ�ϴ��� ������layer����
	void InitMsgPool();
private:
	SHARE<ThreadPool> m_pool;
	vector<SHARE<BaseLayer>> m_layers;
	ServerNode m_server;
	ServerConfig m_config;

	MsgPool* m_msgPool;
	RecyclePool* m_recycle;
	bool m_over;
};

#endif