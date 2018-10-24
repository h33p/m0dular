#include "threading.h"

static LList<struct Job> jobs;

uint64_t Threading::_QueueJob(JobFn function, void* data, bool ref)
{
	Job job;
	job.args = data;
	job.function = function;
	job.ref = ref;
	return jobs.Enqueue(job);
}

static void* __stdcall ThreadLoop(void* t)
{
	struct JobThread* thread = (struct JobThread*)t;

	struct Job job;
	thread->isRunning = true;
	while (!thread->shouldQuit) {
		if (job.id ^ ~0ull) {
			thread->queueEmpty = false;
			job.function(job.args);
			if (!job.ref)
				free(job.args);
		} else
			thread->queueEmpty = true;
		struct LList<struct Job>* tJobs = thread->jobs;
		thread->jLock->unlock();
		job = tJobs->PopFront(thread->jLock);
	}
	thread->isRunning = false;
	return nullptr;
}

unsigned int Threading::numThreads = 0;
static struct JobThread* threads = nullptr;

static void InitThread(struct JobThread* thread, int id)
{
	thread->id = id;
	thread->jLock = new Mutex();
	thread->jobs = &jobs;
	thread_t handle = Threading::StartThread(ThreadLoop, thread, false);
	thread->handle = malloc(sizeof(thread_t));
	*(thread_t*)thread->handle = handle;

}

void Threading::InitThreads()
{
	//numThreads = std::thread::hardware_concurrency();
	numThreads = NUM_THREADS;
	if (numThreads < 2)
		numThreads = 2;
	if (numThreads >= 8)
		numThreads -= 2;
	threads = (struct JobThread*)calloc(numThreads, sizeof(struct JobThread));

	for (unsigned int i = 0; i < numThreads; i++)
		InitThread(threads + i, i);
}

int Threading::EndThreads()
{
	int ret = 0;

	if (!threads)
		return ret;

	for (unsigned int i = 0; i < numThreads; i++)
		threads[i].shouldQuit = true;

	for (unsigned int i = 0; i < numThreads; i++)
		threads[i].jobs->quit = true;

	for (int o = 0; o < 4; o++)
		for (unsigned int i = 0; i < numThreads; i++)
			threads[i].jobs->sem.Post();

	for (size_t i = 0; i < numThreads; i++) {
#if defined(__linux__) || defined(__APPLE__)
		void* ret = nullptr;
		pthread_join(*(pthread_t*)threads[i].handle, &ret);
#else
		ResumeThread(*(HANDLE*)threads[i].handle);
		if (WaitForSingleObject(*(HANDLE*)threads[i].handle, 100) == WAIT_TIMEOUT && threads[i].isRunning)
			;
#endif
		delete threads[i].jLock;
		threads[i].jLock = nullptr;
		free(threads[i].handle);
	}
	free(threads);
	threads = nullptr;

	return ret;
}

void Threading::FinishQueue()
{
	bool empty = false;
	while (!empty) {
		empty = true;
		for (unsigned int i = 0; i < numThreads; i++) {
			if (threads[i].jobs)
				while (!threads[i].jobs->IsEmpty());
			threads[i].jLock->lock();
			threads[i].jLock->unlock();
		}
	}
}

JobThread* Threading::BindThread(LList<struct Job>* jobsQueue)
{
	for (size_t i = 0; i < numThreads; i++) {
		if (threads[i].jobs == &jobs || !threads[i].jobs) {
			threads[i].jobs = jobsQueue;
			for (size_t o = 0; o < numThreads; o++)
				jobs.sem.Post();
			return threads + i;
		}
	}
	return nullptr;
}

void Threading::UnbindThread(LList<struct Job>* jobsQueue)
{
	for (size_t i = 0; i < numThreads; i++) {
		threads[i].jLock->lock();
		if (threads[i].jobs == jobsQueue)
			threads[i].jobs = &jobs;
		threads[i].jLock->unlock();
	}
}

thread_t Threading::StartThread(threadFn start, void* arg, bool detached, thread_t* thread)
{
#ifdef _WIN32
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)start, arg, 0, thread);
#else
	pthread_attr_t* attr = NULL;
	pthread_attr_t tAttr;
	if (detached) {
		pthread_attr_init(&tAttr);
		pthread_attr_setdetachstate(&tAttr, PTHREAD_CREATE_DETACHED);
		attr = &tAttr;
	}
	pthread_create(thread, attr, start, arg);
#endif
	return *thread;
}

thread_t Threading::StartThread(threadFn start, void* arg, bool detached)
{
	thread_t thread;
	return StartThread(start, arg, detached, &thread);
}

void Threading::JoinThread(thread_t thread, void** returnVal)
{
#ifdef __posix__
	pthread_join(thread, returnVal);
#else
	WaitForSingleObject((void*)thread, INFINITE);
#endif
}
