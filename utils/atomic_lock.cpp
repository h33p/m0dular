#include "atomic_lock.h"

AtomicLock::AtomicLock()
	: 	lck(ATOMIC_FLAG_INIT)
{
}

AtomicLock::~AtomicLock()
{
}

void AtomicLock::lock()
{
	while (lck.test_and_set(std::memory_order_acquire))
		;
}

bool AtomicLock::trylock()
{
	return !lck.test_and_set(std::memory_order_acquire);
}

void AtomicLock::unlock()
{
	lck.clear(std::memory_order_release);
}
