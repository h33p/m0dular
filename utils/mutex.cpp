#include "mutex.h"

#if defined(__linux__) || defined(__APPLE__)
#include <pthread.h>

Mutex::Mutex() {
	int ret = pthread_mutex_init(&lck, nullptr);
	if (ret) {
		throw;
	}
}

Mutex::~Mutex() {
	pthread_mutex_destroy(&lck);
}

void Mutex::lock() {
	pthread_mutex_lock(&lck);
}

bool Mutex::trylock() {
	return !pthread_mutex_trylock(&lck);
}

void Mutex::unlock() {
	pthread_mutex_unlock(&lck);
}

#else
#include <windows.h>

Mutex::Mutex() {
	::InitializeCriticalSection(&lck);
}

Mutex::~Mutex() {
	::DeleteCriticalSection(&lck);
}

void Mutex::lock() {
	::EnterCriticalSection(&lck);
}

void Mutex::unlock() {
	::LeaveCriticalSection(&lck);
}
#endif
