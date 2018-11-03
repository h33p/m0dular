#include <stdio.h>
#include "../utils/threading.h"

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
	std::this_thread::sleep_for(std::chrono::microseconds(job->delay));
	global_return_status++;
}

typedef std::chrono::high_resolution_clock Clock;
long mtime = 0;
std::atomic_long mtimef(0);

int main()
{
	srand(time(nullptr));
	Threading::InitThreads();

	for (int i = 0; i < 1000000; i++) {
		auto t1 = Clock::now();
		Threading::QueueJob(TJob, (threadjob){0 * 1 + rand() % 1});
		auto t2 = Clock::now();
		mtime = std::max(mtime, std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count());
		global_return_status--;
		if (!(i % 100000)) {
			printf("%d... (%lu)\n", i, mtime);
			mtime = 0;
		}
	}

	threadjob* testdata[1000];

	for (int i = 0; i < 1000; i++) {
		testdata[i] = (threadjob*)malloc(sizeof(threadjob));
		testdata[i]->delay = 100000 + rand() % 100000;
	}

	printf("Queued the first batch of jobs\n");

	Threading::FinishQueue();

	printf("Finished the first batch of jobs (%d)\n", global_return_status.load());

	for (int i = 0; i < 100000; i++) {
		Threading::QueueJobRef(TJob, testdata[i % 1000]);
		global_return_status--;
		if (!(i % 10000))
			printf("%d...\n", i);
	}

	printf("Queued the second batch of jobs\n");

	Threading::EndThreads();

	printf("Finished the second batch of jobs (%d)\n", global_return_status.load());

	for (int i = 0; i < 1000; i++)
		free(testdata[i]);

	if (global_return_status < 0)
		global_return_status.store(0);

	return global_return_status.load();
}
