#ifndef LOOP_SERVER_H
#define LOOP_SERVER_H
#include "BaseLayer.h"
#include "ThreadPool.h"
#include "DataDefine.h"

class LoopServer
{
public:
	LoopServer();
	~LoopServer();

	void Init(const int& stype, const int& serid);

	template<typename T,typename... Args>
	enable_if_t<is_base_of_v<BaseLayer,T>,T*> CreateLayer(Args&&... args)
	{
		auto l = SHARE<T>(new T(std::forward<Args>(args)...));
		m_layers.push_back(l);
		return l.get();
	}

	void BuildPipe(BaseLayer* l1, BaseLayer* l2)
	{
		auto p1 = GET_LOOP(PIPE);
		auto p2 = GET_LOOP(PIPE);

		l1->regPipe(l2->GetID(), p1, p2);
		l2->regPipe(l1->GetID(), p2, p1);
	}

	void Run();

private:
	SHARE<ThreadPool> m_pool;
	vector<SHARE<BaseLayer>> m_layers;
	ServerNode m_serverNode;
};

#endif