#ifndef THREADING_H
#define THREADING_H

#include "../g_defines.h"
#include "mutex.h"
#include "semaphores.h"
#include "packed_heap.h"
#include <atomic>

#if defined(__posix__)
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

typedef pthread_t thread_t;

#else
#define NOMINMAX
#include "../wincludes.h"
#include <Psapi.h>
#include "stdint.h"

typedef unsigned long thread_t;

#endif

typedef void(*JobFn)(void*);
typedef void*(__stdcall*threadFn)(void*);

struct Job
{
	JobFn function;
	void* args;
	bool ref;
	uint64_t id;

	Job()
	{
		function = nullptr;
		args = nullptr;
		ref = true;
		id = ~0ull;
	}
};

template <typename T>
struct LList
{

	struct LEntry
	{
		T entry;
		idx_t prev;
		idx_t next;
	};

	PackedHeap<LEntry> entries;

	Mutex lock;
	bool quit;
	idx_t front;
	idx_t back;
	uint64_t lastID;
	uint64_t lastPopID;

	Semaphore sem;

	LList() {
		front = 0;
		back = 0;
		lastID = 0;
		lastPopID = 0;
	}

	uint64_t Enqueue(const T& data) {
		lock.lock();
		idx_t entry = entries.Alloc();
		entries[entry] = { data, 0, back };
		entries[entry].entry.id = lastID;
		uint64_t id = lastID++;
		if (back)
			entries[back].prev = entry;
		if (!front) {
			front = entry;
			entries[front].prev = 0;
		}
		entries[entry].next = back;
		back = entry;
		lock.unlock();
		sem.Post();
		return id;
	}

	T PopFront(Mutex* lck = nullptr) {
		sem.Wait();
		if (quit) {
			sem.Post();
			return Job();
		}
		lock.lock();
		if (!front) {
			lock.unlock();
			return Job();
		}
		if (lck)
			lck->lock();
		LEntry* entry = entries + front;
		front = entry->prev;
		if (front)
			entries[front].next = 0;
		else
			back = 0;
		T ret = entry->entry;
		lastPopID = ret.id;
		entries.Free(entry);
		lock.unlock();
		return ret;
	}

#ifdef _MSC_VER
	__declspec(noinline)
#else
	__attribute__((noinline))
#endif
	bool IsEmpty()
	{
		static volatile short cnt = 0;
		cnt++;
		return !front;
	}
};

struct JobThread
{
	std::atomic_bool shouldQuit;
	std::atomic_bool isRunning;
	std::atomic_bool queueEmpty;
	Mutex* jLock;
	LList<struct Job>* jobs;
	int id;
	void* handle;
};

namespace Threading
{
	extern unsigned int numThreads;
	uint64_t _QueueJob(JobFn function, void* data, bool ref = false);
	void InitThreads();
	int EndThreads();
	void FinishQueue();
	JobThread* BindThread(LList<struct Job>* jobsQueue);
	void UnbindThread(LList<struct Job>* jobsQueue);
	thread_t StartThread(threadFn start, void* param, bool detached = true);
	thread_t StartThread(threadFn start, void* param, bool detached, thread_t* thread);
	void JoinThread(thread_t thread, void** returnVal);

	template<typename N, typename T>
	uint64_t QueueJob(N function, T data) {
		void* d = malloc(sizeof(T));
		memcpy(d, (void*)&data, sizeof(T));
		return _QueueJob((JobFn)function, d, false);
	}

	template<typename N, typename T>
	uint64_t QueueJobRef(N function, T* data) {
		return _QueueJob((JobFn)function, (void*)data, true);
	}
}

#endif
