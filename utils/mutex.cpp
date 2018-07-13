#include "mutex.h"

#if defined(__linux__) || defined(__APPLE__)
#include <pthread.h>

class Mutex::Impl {
  public:
	Impl() {
		int ret = pthread_mutex_init(&cs, nullptr);
		if (ret) {
		}
  }

  ~Impl() {
	}

	void lock() {
		pthread_mutex_lock(&cs);
	}

	void unlock() {
  	pthread_mutex_unlock(&cs);
	}

  private:
	pthread_mutex_t cs;
};

Mutex::Mutex() : impl(new Mutex::Impl()) {}
Mutex::~Mutex() { delete impl; }
void Mutex::lock() { impl->lock(); }
void Mutex::unlock() { impl->unlock(); }
#else
#include <windows.h>

class Mutex::Impl {
  public:
	Impl() {
		::InitializeCriticalSection(&cs);
  }

  ~Impl() {
  	::DeleteCriticalSection(&cs);
	}

	void lock() {
  	::EnterCriticalSection(&cs);
	}

	void unlock() {
  	::LeaveCriticalSection(&cs);
	}

  private:
	CRITICAL_SECTION cs;
};

Mutex::Mutex() : impl(new Mutex::Impl()) {}
Mutex::~Mutex() { delete impl; }
void Mutex::lock() { impl->lock(); }
void Mutex::unlock() { impl->unlock(); }
#endif
