#include "semaphores.h"

#if defined(__linux__)
#include <fcntl.h>

Semaphore::Semaphore(bool shared) {
	if (sem_init(&sm, (int)shared, 0) == -1)
		throw;
}

Semaphore::~Semaphore() {
}

int Semaphore::TimedWait(size_t milliseconds)
{
	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
		return 1;
	ts.tv_nsec += 1000000ull * milliseconds;
	return sem_timedwait(&sm, &ts);
}

void Semaphore::Wait() {
	sem_wait(&sm);
}

void Semaphore::Post() {
	sem_post(&sm);
}

unsigned long Semaphore::Count()
{
	int val = 0;
	sem_getvalue(&sm, &val);
	return val;
}

#elif defined(__APPLE__)

Semaphore::Semaphore(bool shared) {
	sm = dispatch_semaphore_create(0);
}

Semaphore::~Semaphore() {
	dispatch_release(sm);
}

void Semaphore::Wait() {
	dispatch_semaphore_wait(sm, DISPATCH_TIME_FOREVER);
}

int Semaphore::TimedWait(size_t milliseconds) {
	return dispatch_semaphore_wait(sm, dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_MSEC * milliseconds));
}

void Semaphore::Post() {
	dispatch_semaphore_signal(sm);
}

unsigned long Semaphore::Count()
{
	int val = 0;
	return val;
}

#else

Semaphore::Semaphore(bool shared) {
	//Unnamed shared semaphores do not work on windows
	if (shared)
		throw;
	sm = CreateSemaphoreA(nullptr, 0, 0xffff, nullptr);
}

Semaphore::~Semaphore() {
	CloseHandle(sm);
}

void Semaphore::Wait() {
	WaitForSingleObject(sm, INFINITE);
}

int Semaphore::TimedWait(size_t milliseconds)
{
	if (WaitForSingleObject(sm, milliseconds) == WAIT_OBJECT_0)
		return 0;
	return 1;
}

void Semaphore::Post() {
	ReleaseSemaphore(sm, 1, NULL);
}

unsigned long Semaphore::Count()
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
#endif
