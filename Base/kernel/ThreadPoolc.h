#ifndef THREAD_POOL_C_H
#define THREAD_POOL_C_H
#include <malloc.h>
#include <pthread.h>
#include <sched.h>
typedef void(*work)(void* arg);

struct threadjob
{
	work fun;
	void* arg;
	threadjob* next;
};

struct threadpool;

struct Thread
{
	pthread_t tid;
	pthread_cond_t cond;
	threadpool* pool;
};

struct threadpool
{
	int size;
	int idle;
	bool quit;
	pthread_mutex_t mutex;
	threadjob* head;
	threadjob* tail;
	Thread* thrs;
	pthread_cond_t** idles;
};

static void* thread_run(void* arg)
{
	Thread* thd = (Thread*)arg;
	threadpool* pool = thd->pool;
	threadjob* job;
	int status;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	//pthread_detach(thd->tid);
	
	pthread_cleanup_push((work)pthread_mutex_unlock, &pool->mutex);

	pthread_mutex_lock(&pool->mutex);
	while (1)
	{
		if (pool->head)
		{
			job = pool->head;
			pool->head = job->next;
			if (pool->head == NULL)
				pool->tail = NULL;

			pthread_mutex_unlock(&pool->mutex);
			job->fun(job->arg);
			free(job);
			pthread_mutex_lock(&pool->mutex);
		}
		else
		{
			if (pool->quit)
			{
				pthread_mutex_unlock(&pool->mutex);
				break;
			}
			pool->idles[pool->idle] = &thd->cond;
			++pool->idle;
			status = pthread_cond_wait(&thd->cond, &pool->mutex);

			if (status == 0)
				continue;

			pthread_mutex_unlock(&pool->mutex);
			break;
		}
	}
	pthread_cleanup_pop(0);
	return NULL;
}

void addthread(Thread* thd)
{
	pthread_cond_init(&thd->cond,NULL);
	pthread_create(&thd->tid, NULL, thread_run, thd);
}

threadpool* create_pool(int size)
{
	threadpool* pool = (threadpool*)calloc(1,sizeof(threadpool));
	pool->thrs = (Thread*)calloc(size,sizeof(Thread));
	pool->idles = (pthread_cond_t**)calloc(size,sizeof(pthread_cond_t*));
	pool->size = size;
	pool->quit = false;

	pthread_mutex_init(&pool->mutex,NULL);
	pthread_mutex_lock(&pool->mutex);
	for (size_t i = 0; i < size; i++)
	{
		pool->thrs[i].pool = pool;
		addthread(&pool->thrs[i]);
	}
	pthread_mutex_unlock(&pool->mutex);
	return pool;
}

void pool_add_job(threadpool* pool,work fun,void* arg)
{
	threadjob* job = (threadjob*)calloc(1, sizeof(*job));
	job->arg = arg;
	job->fun = fun;

	pthread_mutex_lock(&pool->mutex);
	if (pool->head)
		pool->tail->next = job;
	else
		pool->head = job;
	pool->tail = job;

	if (pool->idle>0)
	{
		pthread_cond_t* cond = pool->idles[--pool->idle];
		pool->idles[pool->idle] = NULL;
		pthread_mutex_unlock(&pool->mutex);
		pthread_cond_signal(cond);
	}else
		pthread_mutex_unlock(&pool->mutex);
}

void distroy_pool(threadpool* &pool)
{
	pthread_mutex_lock(&pool->mutex);
	pool->quit = true;
	while (pool->head)
	{
		threadjob* next = pool->head->next;
		free(pool->head);
		pool->head = next;
	}
	pthread_mutex_unlock(&pool->mutex);
	void* status;
	for (size_t i = 0; i < pool->size; i++)
	{
		//std::cout<< pthread_cancel(pool->thrs[i].tid) <<std::endl;
		pthread_cond_signal(&pool->thrs[i].cond);
		pthread_join(pool->thrs[i].tid, &status);
		pthread_cond_destroy(&pool->thrs[i].cond);
	}

	pthread_mutex_destroy(&pool->mutex);

	free(pool->thrs);
	free(pool->idles);
	free(pool);
	pool = NULL;
}

#endif