#ifndef LOOP_SERVER_H
#define LOOP_SERVER_H
#include "BaseLayer.h"
#include "ThreadPool.h"

class LoopServer
{
public:
	LoopServer();
	~LoopServer();

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

	void Run()
	{
		m_pool = SHARE<ThreadPool>(new ThreadPool(m_layers.size()));
		for (auto& l:m_layers)
		{
			m_pool->Add_Task([&l]() {
				l->StartRun();
			});
		}
	}

private:
	SHARE<ThreadPool> m_pool;
	vector<SHARE<BaseLayer>> m_layers;
};

#endif