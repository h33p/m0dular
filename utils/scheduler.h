#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stddef.h>

template <typename T, size_t size>
struct Scheduler
{
	T data[size];
	int burstTime[size];
	int waitTime[size];
	int priority[size];
	int sid[size];
	size_t cid;

	void Sort()
	{
		for (size_t i = 0; i < size; i++) {
			size_t pos = i;
			for (size_t o = i + 1; o < size; o++)
				if (priority[pos] >= priority[o])
					pos = o;

			int temp = priority[i];
			priority[i] = priority[pos];
			priority[pos] = temp;

			temp = burstTime[i];
			burstTime[i] = burstTime[pos];
			burstTime[pos] = temp;

			T temp2 = data[i];
			data[i] = data[pos];
			data[pos] = temp2;

			sid[pos] = i;
		}
		cid = 0;
	}

	T* Run(int time)
	{
		for (; cid < size; cid++) {
			if (burstTime[cid] >= time) {
				burstTime[cid] -= time;
				return data + (cid++);
			}
			time -= burstTime[cid];
			burstTime[cid] = 0;
		}
		return nullptr;
	}
};

#endif
