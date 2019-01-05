#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#include <stdint.h>
#if defined(__linux__) || defined(__APPLE__)
#include <stddef.h>
#endif

#if defined(__linux__)
#include <semaphore.h>
#include <time.h>
#elif defined(__APPLE__)
#include <dispatch/dispatch.h>
#else
#include "../wincludes.h"
#endif

class Semaphore
{
	public:
	Semaphore(bool shared = false);
	~Semaphore();
	void Wait();
	int TimedWait(size_t milliseconds);
	void Post();
	unsigned long Count();
	private:

#if defined(__linux__)
	sem_t sm;
#elif defined(__APPLE__)
	dispatch_semaphore_t sm;
#else
	HANDLE sm;
#endif
};

#endif
