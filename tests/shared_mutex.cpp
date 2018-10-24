#include <stdio.h>
#include "../utils/threading.h"
#include "../utils/shared_mutex.h"

#include <atomic>
#include <thread>
#include <chrono>

std::atomic_int expected_value(0);
int value = 0;
bool cont = true;
SharedMutex mtx;
std::atomic_int global_return_status(0);

void* __stdcall WriteFunc(void*)
{
	while (cont) {
		int newval = rand();
		mtx.wlock();
		value = newval;
		std::this_thread::sleep_for(std::chrono::microseconds(100 + rand() % 10000));
		expected_value.store(value);
		mtx.wunlock();
		std::this_thread::sleep_for(std::chrono::microseconds(100 + rand() % 1000));
	}
	return nullptr;
}

void* __stdcall ReadFunc(void*)
{
	int local = -1;
	while (cont) {
		mtx.rlock();
		if (value != local) {
			if (value != expected_value.load())
				global_return_status++;
			local = value;
		}
		mtx.runlock();
		std::this_thread::sleep_for(std::chrono::microseconds(rand() % 100));
	}
	return nullptr;
}

int main()
{
	srand(time(nullptr));
	thread_t write_thread = Threading::StartThread(WriteFunc, nullptr, false);
	thread_t read_thread1 = Threading::StartThread(ReadFunc, nullptr, false);
	thread_t read_thread2 = Threading::StartThread(ReadFunc, nullptr, false);
	thread_t read_thread3 = Threading::StartThread(ReadFunc, nullptr, false);

	std::this_thread::sleep_for(std::chrono::microseconds(500000));

	cont = false;
	void* ret = nullptr;
	Threading::JoinThread(write_thread, &ret);
	Threading::JoinThread(read_thread1, &ret);
	Threading::JoinThread(read_thread2, &ret);
	Threading::JoinThread(read_thread3, &ret);

	return -global_return_status.load();
}
