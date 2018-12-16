#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#include <stdint.h>
#if defined(__linux__) || defined(__APPLE__)
#include <stddef.h>
#endif

#if defined(__linux__) || defined(__APPLE__)
#include <semaphore.h>
#include <time.h>
#else
#include "windows.h"
#endif

class NamedSemaphore
{
	public:
	NamedSemaphore(const char* name);
	~NamedSemaphore();
	void Wait();
	int TimedWait(size_t milliseconds);
	void Post();
	unsigned long Count();
	private:

#if defined(__linux__)
	sem_t* sm;
	const char* _name;
#else
	HANDLE sm;
#endif
};

#endif
