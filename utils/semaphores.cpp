#include "semaphores.h"

#if defined(__linux__)
#include <semaphore.h>
#include <time.h>

class Semaphore::Impl {
  public:
	Impl() {
		sem_init(&sm, 0, 0);
  }

  ~Impl() {
	}

	int TimedWait(size_t milliseconds)
	{
		struct timespec ts;
		if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
			return 1;
		ts.tv_nsec += 1000000ull * milliseconds;
		return sem_timedwait(&sm, &ts);
	}

	void Wait() {
		sem_wait(&sm);
	}

	void Post() {
  	sem_post(&sm);
	}

	unsigned long Count()
	{
		int val = 0;
		sem_getvalue(&sm, &val);
		return val;
	}

  private:
	sem_t sm;
};

#elif defined(__APPLE__)
#include <dispatch/dispatch.h>

class Semaphore::Impl {
  public:
	Impl() {
		sm = dispatch_semaphore_create(0);
  }

  ~Impl() {
		dispatch_release(sm);
	}

	void Wait() {
		dispatch_semaphore_wait(sm, DISPATCH_TIME_FOREVER);
	}

	int TimedWait(size_t milliseconds) {
		return dispatch_semaphore_wait(sm, dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_MSEC * milliseconds));
	}

	void Post() {
  	dispatch_semaphore_signal(sm);
	}

	unsigned long Count()
	{
		int val = 0;
		return val;
	}

  private:
	dispatch_semaphore_t sm;
};
#else
#include "windows.h"

class Semaphore::Impl {
  public:
	Impl() {
		sm = CreateSemaphoreA(nullptr, 1, 0xffffffff, nullptr);
  }

  ~Impl() {
		CloseHandle(sm);
	}

	void Wait() {
		WaitForSingleObject(sm, INFINITE);
	}

	int TimedWait(size_t milliseconds)
	{
		if (WaitForSingleObject(sm, milliseconds) == WAIT_OBJECT_0)
			return 0;
		return 1;
	}

	void Post() {
  	ReleaseSemaphore(sm, 1, NULL);
	}

	unsigned long Count()
	{
		long previous;
		switch (WaitForSingleObject(sm, 0)) {
			case WAIT_OBJECT_0:
				ReleaseSemaphore(sm, 1, &previous);
				return previous + 1;
			case WAIT_TIMEOUT:
				return 0;
		}
		return 0;
	}

  private:
	HANDLE sm;
};

#endif
Semaphore::Semaphore() : impl(new Semaphore::Impl()) {}
Semaphore::~Semaphore() { delete impl; }
void Semaphore::Wait() { impl->Wait(); }
void Semaphore::Post() { impl->Post(); }
int Semaphore::TimedWait(size_t milliseconds) { return impl->TimedWait(milliseconds); }
unsigned long Semaphore::Count() { return impl->Count(); }
