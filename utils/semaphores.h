#ifndef SEMAPHORES_H
#define SEMAPHORES_H

class Semaphore
{
	public:
	Semaphore();
	~Semaphore();
	void Wait();
	void Post();
	unsigned long Count();
	private:
	class Impl;
	Impl* impl;
};

#endif
