#include "threading.h"
#include <thread>

static std::condition_variable newJobNotify;

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

static void* ThreadLoop(void* t)
{
	struct JobThread* thread = (struct JobThread*)t;

	thread->isRunning = true;
	while (!thread->shouldQuit) {
		thread->jLock->lock();
		struct LList<struct Job>* tJobs = thread->jobs ? thread->jobs : &jobs;
		struct Job* job = tJobs->PopFront();
		std::mutex* lock = &tJobs->lock;
		std::condition_variable* notify = tJobs->notifyVar;
		thread->jLock->unlock();
		if (job) {
			thread->queueEmpty = false;
			job->flags |= IS_RUNNING;
			job->function(job->args);
			if (!job->ref)
				free(job->args);
			job->flags |= IS_FINISHED;
		} else {
			thread->queueEmpty = true;
			if (!thread->shouldQuit) {
				std::unique_lock<std::mutex> lk (*lock);
				notify->wait(lk);
				if (!thread->shouldQuit)
					thread->queueEmpty = false;
			}
		}
	}
	thread->isRunning = false;
	return nullptr;
}

unsigned int Threading::numThreads = 0;
static struct JobThread* threads = nullptr;

static void InitThread(struct JobThread* thread, int id)
{
	thread->id = id;
	thread->jLock = new std::mutex();
#ifdef _WIN32
	thread->handle = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ThreadLoop, thread, NULL, NULL);
#else
	pthread_attr_t tAttr;
	pthread_attr_init(&tAttr);
	pthread_t* t = (pthread_t*)malloc(sizeof(pthread_t));
	pthread_attr_setdetachstate(&tAttr, PTHREAD_CREATE_DETACHED);
	pthread_create(t, &tAttr, ThreadLoop, thread);
	thread->handle = t;
#endif
}

void Threading::InitThreads()
{
	numThreads = std::thread::hardware_concurrency();
	jobs.notifyVar = &newJobNotify;
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

	jobs.lock.lock();
	jobs.lock.unlock();
	newJobNotify.notify_all();

	for (unsigned int i = 0; i < numThreads; i++) {
		threads[i].jLock->lock();
		if (threads[i].jobs) {
			threads[i].jobs->lock.lock();
			threads[i].jobs->lock.unlock();
			threads[i].jobs->notifyVar->notify_all();
		}
		threads[i].jLock->unlock();
		while (threads[i].isRunning);
	}

#ifdef __linux__
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
				while(threads[i].jobs->count > 0);
			else
				while(jobs.count > 0);
			if (!threads[i].queueEmpty)
				empty = false;
		}
	}
}

JobThread* Threading::BindThread(LList<struct Job>* jobsQueue)
{
	for (int i = 0; i < numThreads; i++) {
		threads[i].jLock->lock();
		if (!threads[i].jobs) {
			jobs.lock.lock();
			jobs.lock.unlock();
			jobs.notifyVar->notify_all();
			threads[i].jobs = jobsQueue;
			threads[i].jLock->unlock();
			return threads + i;
		}
		threads[i].jLock->unlock();
	}
	return nullptr;
}

void Threading::UnbindThread(LList<struct Job>* jobsQueue)
{
	for (int i = 0; i < numThreads; i++) {
		threads[i].jLock->lock();
		if (threads[i].jobs == jobsQueue)
			threads[i].jobs = nullptr;
		threads[i].jLock->unlock();
	}
}
