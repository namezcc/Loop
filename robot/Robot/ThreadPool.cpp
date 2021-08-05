#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t threads) :stop(false)
{
	for (size_t i = 0; i < threads; i++)
	{
		m_threads.emplace_back(
			[this,i]() {
			while (true)
			{
				task t;
				{
					unique_lock<mutex> lock(this->m_qmut);
					this->m_cond.wait(lock, [this] { return this->stop || !this->m_task.empty(); });
					if (this->stop && this->m_task.empty())
						return;
					t = std::move(this->m_task.front());
					this->m_task.pop();
				}
				t(i);
			}
		}
		);
	}
}

ThreadPool::~ThreadPool()
{
	{
		std::unique_lock<std::mutex> lock(m_qmut);
		stop = true;
	}
	m_cond.notify_all();
	for (std::thread &worker : m_threads)
		worker.join();
}