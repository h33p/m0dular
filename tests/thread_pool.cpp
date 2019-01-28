#include <stdio.h>
#include "../utils/threading.h"
#include "../submodules/minitrace/minitrace.h"

#include <thread>
#include <chrono>

int value = 0;
bool cont = true;
Mutex mtx;
std::atomic_int global_return_status(0);

struct threadjob
{
	int delay;
};

void TJob(threadjob* job)
{
	MTR_BEGIN("test", "sleep_job");
	std::this_thread::sleep_for(std::chrono::microseconds(job->delay));
	MTR_END("test", "sleep_job");
	global_return_status++;
}

typedef std::chrono::high_resolution_clock Clock;
long mtime = 0;
std::atomic_long mtimef(0);

int main()
{
	mtr_init("trace.json");
	MTR_META_PROCESS_NAME("thread_pool_test");

	srand(time(nullptr));
	Threading::InitThreads();

	MTR_BEGIN("test", "queue_jobs");
	for (int i = 0; i < 10000; i++) {
		//auto t1 = Clock::now();
		Threading::QueueJob(TJob, (threadjob){0 * 1 + rand() % 10});
		//auto t2 = Clock::now();
		//mtime = std::max((long long)mtime, (long long)std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count());
		global_return_status--;
		/*if (!(i % 100000)) {
			printf("%d... (%lu)\n", i, mtime);
			mtime = 0;
			}*/
	}
	MTR_END("test", "queue_jobs");

	threadjob* testdata[1000];

	MTR_BEGIN("test", "genrerate_test_data");
	for (int i = 0; i < 1000; i++) {
		testdata[i] = (threadjob*)malloc(sizeof(threadjob));
		testdata[i]->delay = rand() % 10;
	}
	MTR_END("test", "genrerate_test_data");

	printf("Queued the first batch of jobs\n");

	Threading::FinishQueue();

	printf("Finished the first batch of jobs (%d)\n", global_return_status.load());

	MTR_BEGIN("test", "queue_jobs_ref");
	for (int i = 0; i < 10000; i++) {
		Threading::QueueJobRef(TJob, testdata[i % 1000]);
		global_return_status--;
		/*if (!(i % 10000))
		  printf("%d...\n", i);*/
	}
	MTR_END("test", "queue_jobs_ref");

	printf("Queued the second batch of jobs\n");

	Threading::FinishQueue();
	Threading::EndThreads();

	printf("Finished the second batch of jobs (%d)\n", global_return_status.load());

	for (int i = 0; i < 1000; i++)
		free(testdata[i]);

	if (global_return_status < 0)
		global_return_status.store(0);

	mtr_flush();
	mtr_shutdown();

	return global_return_status.load();
}
