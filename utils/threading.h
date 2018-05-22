#ifndef THREADING_H
#define THREADING_H

#include "../g_defines.h"

#if defined(__posix__)
#include <unistd.h>
#include <pthread.h>
#include <atomic>
#include <stdlib.h>
#include <string.h>
#else
#define NOMINMAX
#include "../wincludes.h"
#include <Psapi.h>
#include <cstdint>
#endif

#pragma push_macro("if")
#undef if
#include <atomic>
#include <mutex>
#include <condition_variable>
#pragma pop_macro("if")

#define IS_RUNNING (1 << 0)
#define IS_FINISHED (1 << 1)

typedef void(*JobFn)(void*);

struct Job
{
	JobFn function;
	void* args;
	bool ref;
	std::atomic_char flags;

	void WaitForFinish()
	{
		while (!(flags & IS_FINISHED));
	}
};

template <typename T>
struct LList
{
	struct LEntry
	{
		T* entry;
		LEntry* prev;
		LEntry* next;
	};

	std::mutex lock;
	std::atomic_int count;
	struct LEntry* front;
	struct LEntry* back;
	std::condition_variable* notifyVar;

	T* Enqueue(T* data) {
		struct LEntry* entry = new LEntry({ data, nullptr, back });
		lock.lock();
		count++;
		if (back)
			back->prev = entry;
		if (!front)
			front = entry;
		entry->next = back;
		back = entry;
		lock.unlock();
		if (notifyVar)
			notifyVar->notify_one();
		return data;
	}

	T* PopFront() {
		lock.lock();
		if (!front) {
			lock.unlock();
			return nullptr;
		}
		struct LEntry* entry = front;
		front = entry->prev;
		if (front)
			front->next = nullptr;
		else
			back = nullptr;
		count--;
		T* ret = entry->entry;
		delete entry;
		lock.unlock();
		return ret;
	}
};

struct JobThread
{
	std::atomic_bool shouldQuit;
	std::atomic_bool isRunning;
	std::atomic_bool queueEmpty;
	std::mutex* jLock;
	LList<struct Job>* jobs;
	int id;
	void* handle;
};

namespace Threading
{
	extern unsigned int numThreads;
	struct Job* _QueueJob(JobFn function, void* data, bool ref = false);
	void InitThreads();
	void EndThreads();
	void FinishQueue();
	JobThread* BindThread(LList<struct Job>* jobsQueue);
	void UnbindThread(LList<struct Job>* jobsQueue);

	template<typename N, typename T>
	Job* QueueJob(N function, T data) {
		void* d = malloc(sizeof(T));
		memcpy(d, (void*)&data, sizeof(T));
		return _QueueJob((JobFn)function, d, false);
	}

	template<typename N, typename T>
	Job* QueueJobRef(N function, T* data) {
		return _QueueJob((JobFn)function, (void*)data, true);
	}
}

#endif
