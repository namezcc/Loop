#ifndef LOCK_H
#define LOCK_H
#ifdef _WIN32
#include <Windows.h>
#endif
struct Lock
{
	volatile unsigned int lock;
};

static void lock_init(Lock* lock)
{
	lock->lock = 0;
}

static void atomic_lock(Lock* lock)
{
	while (InterlockedExchange(&lock->lock,1))
	{
		Sleep(2);
	}
}

static void atomic_unlock(Lock* lock)
{
	InterlockedExchange(&lock->lock, 0);
}

#endif