#include "semaphores.h"

#if defined(__linux__) || defined(__APPLE__)
#include <semaphore.h>

class Semaphore::Impl {
  public:
	Impl() {
  }

  ~Impl() {
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
unsigned long Semaphore::Count() { return impl->Count(); }
