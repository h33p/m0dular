#include "shared_mutex.h"

#if defined(__linux__) || defined(__APPLE__)
#include <pthread.h>

SharedMutex::SharedMutex() {
	int ret = pthread_rwlock_init(&lock, nullptr);
	if (ret) {
		throw;
	}
}

SharedMutex::~SharedMutex() {
	pthread_rwlock_destroy(&lock);
}

void SharedMutex::rlock() {
	pthread_rwlock_rdlock(&lock);
}

void SharedMutex::runlock() {
  	pthread_rwlock_unlock(&lock);
}

void SharedMutex::wlock() {
	pthread_rwlock_wrlock(&lock);
}

void SharedMutex::wunlock() {
  	pthread_rwlock_unlock(&lock);
}

#else
#include <windows.h>

SharedMutex::SharedMutex() {
	::InitializeSRWLock(&lock);
}

SharedMutex::~SharedMutex() {
  	::DeleteCriticalSection(&lock);
}

void SharedMutex::rlock() {
	::AcquireSRWLockShared(&lock);
}

void SharedMutex::runlock() {
	::ReleaseSRWLockShared(&lock);
}

void SharedMutex::wlock() {
	::AcquireSRWLockExclusive(&lock);
}

void SharedMutex::wunlock() {
	::ReleaseSRWLockExclusive(&lock);
}
#endif
