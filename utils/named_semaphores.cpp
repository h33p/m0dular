#include "named_semaphores.h"

#if defined(__linux__)
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

NamedSemaphore::NamedSemaphore(const char* name) {
	_name = name;
	sm = sem_open(name, O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
	if (sm == (sem_t*)SEM_FAILED) {
		sem_unlink(name);
		sm = sem_open(name, O_CREAT | O_EXCL, S_IRWXU | S_IRWXG);
	}
	if (sm == (sem_t*)SEM_FAILED) {
		printf("ERROR %d\n", errno);
		throw;
	}
}

NamedSemaphore::~NamedSemaphore() {
	if (sm) {
		sem_unlink(_name);
		sem_close(sm);
	}
}

int NamedSemaphore::TimedWait(size_t milliseconds)
{
	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
		return 1;
	ts.tv_nsec += 1000000ull * milliseconds;
	return sem_timedwait(sm, &ts);
}

void NamedSemaphore::Wait() {
	sem_wait(sm);
}

void NamedSemaphore::Post() {
	sem_post(sm);
}

unsigned long NamedSemaphore::Count()
{
	int val = 0;
	sem_getvalue(sm, &val);
	return val;
}

#else

NamedSemaphore::NamedSemaphore(const char* name) {
	sm = CreateSemaphoreA(nullptr, 0, 0xffff, name);
}

NamedSemaphore::~NamedSemaphore() {
	CloseHandle(sm);
}

void NamedSemaphore::Wait() {
	WaitForSingleObject(sm, INFINITE);
}

int NamedSemaphore::TimedWait(size_t milliseconds)
{
	if (WaitForSingleObject(sm, milliseconds) == WAIT_OBJECT_0)
		return 0;
	return 1;
}

void NamedSemaphore::Post() {
	ReleaseSemaphore(sm, 1, NULL);
}

unsigned long NamedSemaphore::Count()
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
