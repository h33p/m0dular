#ifndef ATOMIC_LOCK_H
#define ATOMIC_LOCK_H

#include <atomic>

class AtomicLock
{
  public:
	AtomicLock();
	~AtomicLock();
	void lock();
	bool trylock();
	void unlock();
  private:
	std::atomic_flag lck;
};

#endif
