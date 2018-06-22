#include "threading.h"
//#include <thread>

//static std::condition_variable newJobNotify;
static Semaphore newJobSem;

static LList<struct Job> jobs;

struct Job* Threading::_QueueJob(JobFn function, void* data, bool ref)
{
	struct Job* job = new Job();
	job->args = data;
	job->function = function;
	job->flags.store(0);
	job->ref = ref;
	return jobs.Enqueue(job);
}

static void* __stdcall ThreadLoop(void* t)
{
	struct JobThread* thread = (struct JobThread*)t;

	struct Job* job = nullptr;
	thread->isRunning = true;
	while (!thread->shouldQuit) {
		if (job) {
			thread->queueEmpty = false;
			job->flags |= IS_RUNNING;
			job->function(job->args);
			if (!job->ref)
				free(job->args);
			job->flags |= IS_FINISHED;
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
	Threading::StartThread(ThreadLoop, thread);
}

void Threading::InitThreads()
{
	//numThreads = std::thread::hardware_concurrency();
	numThreads = 4;
	if (numThreads < 2)
		numThreads = 2;
	if (numThreads >= 8)
		numThreads -= 2;
	threads = (struct JobThread*)calloc(numThreads, sizeof(struct JobThread));

	for (unsigned int i = 0; i < numThreads; i++)
		InitThread(threads + i, i);
}

void Threading::EndThreads()
{
	if (!threads)
		return;

	for (unsigned int i = 0; i < numThreads; i++)
		threads[i].shouldQuit = true;

	for (int o = 0; o < 2; o++)
		for (unsigned int i = 0; i < numThreads; i++)
			if (threads[i].jobs)
				threads[i].jobs->sem.Post();
			else
				jobs.sem.Post();

	for (unsigned int i = 0; i < numThreads; i++)
		while (threads[i].isRunning);

#if defined(__linux__) || defined(__APPLE__)
	for (int i = 0; i < numThreads; i++)
		free(threads[i].handle);
#endif
	free(threads);
	threads = nullptr;
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
	for (int i = 0; i < numThreads; i++) {
		if (threads[i].jobs == &jobs || !threads[i].jobs) {
			threads[i].jobs = jobsQueue;
			for (int o = 0; o < numThreads; o++)
				jobs.sem.Post();
			return threads + i;
		}
	}
	return nullptr;
}

void Threading::UnbindThread(LList<struct Job>* jobsQueue)
{
	for (int i = 0; i < numThreads; i++) {
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
