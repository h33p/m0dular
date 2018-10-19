#include <stdio.h>
#include "../utils/threading.h"

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
	usleep(job->delay);
	global_return_status++;
}

int main()
{
	srand(time(nullptr));
	Threading::InitThreads();

	for (int i = 0; i < 100000; i++) {
		Threading::QueueJob(TJob, (threadjob){10 + rand() % 10});
		global_return_status--;
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
