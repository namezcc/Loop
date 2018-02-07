#ifndef LOOP_SERVER_H
#define LOOP_SERVER_H
#include "BaseLayer.h"
#include "ThreadPool.h"
#include "DataDefine.h"

class LOOP_EXPORT LoopServer
{
public:
	LoopServer();
	~LoopServer();

	void InitServer(int argc, char** args);

	template<typename T,typename... Args>
	enable_if_t<is_base_of_v<BaseLayer,T>,T*> CreateLayer(Args&&... args)
	{
		auto l = SHARE<T>(new T(std::forward<Args>(args)...));
		m_layers.push_back(l);
		return l.get();
	}

	void BuildPipe(BaseLayer* l1, BaseLayer* l2)
	{
		m_factor.reset(Single::NewLocal<FactorManager>());

		auto p1 = m_factor->getLoopObj<PIPE>();
		auto p2 = m_factor->getLoopObj<PIPE>();

		l1->regPipe(l2->GetType(), p1, p2);
		l2->regPipe(l1->GetType(), p2, p1);
	}

	void Run();

	int m_port;
protected:
	void Init(const int& stype, const int& serid);

private:
	SHARE<ThreadPool> m_pool;
	vector<SHARE<BaseLayer>> m_layers;
	SHARE<FactorManager> m_factor;
	ServerNode m_server;
};

#endif