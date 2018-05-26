#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#include <stdint.h>
#if defined(__linux__) || defined(__APPLE__)
#include <stddef.h>
#endif

class Semaphore
{
	public:
	Semaphore();
	~Semaphore();
	void Wait();
	int TimedWait(size_t milliseconds);
	void Post();
	unsigned long Count();
	private:
	class Impl;
	Impl* impl;
};

#endif
