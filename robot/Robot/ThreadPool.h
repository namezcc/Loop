#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <thread>
#include <functional>
#include <vector>
#include <queue>
#include <condition_variable>
#include <mutex>

using namespace std;

typedef function<void(const int&)> task;
class ThreadPool
{
public:
	ThreadPool(size_t threads);
	~ThreadPool();

	template<typename F,typename... Args>
	void Add_Task(F&& f, Args&&... args)
	{
		if (stop)
			return;
		auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...,std::placeholders::_1);
		{
			std::unique_lock<std::mutex> lock(m_qmut);
			m_task.emplace(task);
		}
		m_cond.notify_one();
	}

private:
	int m_size;
	std::vector<std::thread> m_threads;
	queue<task> m_task;
	mutex m_qmut;
	condition_variable m_cond;
	bool stop;
};
#endif