#ifndef SHARED_MUTEX_H
#define SHARED_MUTEX_H

#if defined(__linux__) || defined(__APPLE__)
#include <pthread.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

class SharedMutex {
  public:
	SharedMutex();
	~SharedMutex();
	void rlock();
	void runlock();
	void wlock();
	void wunlock();
  private:
#if defined(__linux__) || defined(__APPLE__)
	pthread_rwlock_t lock;
#else
	SRWLOCK lock;
#endif
};

#endif
