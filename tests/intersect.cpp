#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <chrono>
#include "../utils/intersect_impl.h"
#include "../utils/intersect_box_impl.h"

//This is more of a speedtest, not a unit test

static constexpr long INTERSECTC = 640000;
static constexpr long SOA_SIZE = SIMD_COUNT;
static constexpr long SOA_INTERSECTC = INTERSECTC / SOA_SIZE;
static constexpr long REPEAT_C = 50;

typedef std::chrono::high_resolution_clock Clock;

vec3_t rays[INTERSECTC][2];
svec3<SOA_SIZE> raysSoa[INTERSECTC][2];

int main()
{
	srand(time(nullptr));

	volatile float rad = 1.f;
	volatile vec3_t startV(0);
	volatile vec3_t endV(10);

	vec3_t start = *(vec3_t*)&startV;
	vec3_t end = *(vec3_t*)&endV;

	volatile svec3<SOA_SIZE> startSoaV(0);
	volatile svec3<SOA_SIZE> endSoaV(10);

	svec3<SOA_SIZE> startSoa = *(svec3<SOA_SIZE>*)&startSoaV;
	svec3<SOA_SIZE> endSoa = *(svec3<SOA_SIZE>*)&endSoaV;

	[[maybe_unused]] CapsuleCollider testCollider;
	testCollider.start = start;
	testCollider.end = end;
	testCollider.radius = rad;

	[[maybe_unused]] CapsuleColliderSOA<SOA_SIZE> testColliderSOA;
	testColliderSOA.start = startSoa;
	testColliderSOA.end = endSoa;

	AABBCollider testBox(start, end);
	AABBColliderSOA testBoxSOA(startSoa, endSoa);

	for (long i = 0; i < SOA_SIZE; i++)
		testColliderSOA.radius[i] = rad + 0.01f * i;

	for (long i = 0; i < INTERSECTC; i++)
		for (long o = 0; o < 2; o++)
			for (long u = 0; u < 3; u++)
				rays[i][o][u] = (rand() % 10000) * (((rand() % 2) * 2) - 1);

	for (long i = 0; i < SOA_INTERSECTC; i++)
		for (long o = 0; o < 2; o++)
			for (long u = 0; u < 3; u++)
				for (int p = 0; p < SOA_SIZE; p++)
					raysSoa[i][o][u][p] = (rand() % 10000) * (((rand() % 2) * 2) - 1);


	{
		printf("Intersecting 1ray - 1box... %d times\n", INTERSECTC * REPEAT_C);

		auto t1 = Clock::now();

		for (volatile long r = 0; r < REPEAT_C; r++)
			for (volatile long i = 0; i < INTERSECTC; i++)
				[[maybe_unused]] volatile bool res = testBox.Intersect(rays[i][0], rays[i][1]);

		auto t2 = Clock::now();

		printf("Finished intersecting in %lu.\n", (unsigned long)std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count());
	}

	{
		printf("Intersecting %drays - 1capsule... %d times\n", SOA_SIZE, SOA_INTERSECTC * REPEAT_C);

		auto t1 = Clock::now();

		for (volatile long r = 0; r < REPEAT_C; r++)
			for (volatile long i = 0; i < SOA_INTERSECTC; i++)
				[[maybe_unused]] volatile auto res = testBox.IntersectSOA(raysSoa[i][0], raysSoa[i][1]);

		auto t2 = Clock::now();

		printf("Finished intersecting in %lu.\n", (unsigned long)std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count());
	}

	{
		printf("Intersecting 1ray - %dboxes... %d times\n", SOA_SIZE, SOA_INTERSECTC * REPEAT_C);

		auto t1 = Clock::now();

		for (volatile long r = 0; r < REPEAT_C; r++)
			for (volatile long i = 0; i < SOA_INTERSECTC; i++)
				[[maybe_unused]] volatile auto res = testBoxSOA.Intersect(rays[i][0], rays[i][1]);

		auto t2 = Clock::now();

		printf("Finished intersecting in %lu.\n", (unsigned long)std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count());
	}

	{
		printf("Intersecting 1rays - 1box (x%d)... %d times\n", SOA_SIZE, SOA_INTERSECTC * REPEAT_C);

		auto t1 = Clock::now();

		for (volatile long r = 0; r < REPEAT_C; r++)
			for (volatile long i = 0; i < SOA_INTERSECTC; i++)
				[[maybe_unused]] volatile auto res = testBoxSOA.IntersectSSOA(raysSoa[i][0], raysSoa[i][1]);

		auto t2 = Clock::now();

		printf("Finished intersecting in %lu.\n", (unsigned long)std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count());
	}

	{
		printf("Intersecting 1ray - 1capsule... %d times\n", INTERSECTC * REPEAT_C);

		auto t1 = Clock::now();

		for (volatile long r = 0; r < REPEAT_C; r++)
			for (volatile long i = 0; i < INTERSECTC; i++)
				[[maybe_unused]] volatile bool res = testCollider.Intersect(rays[i][0], rays[i][1]);

		auto t2 = Clock::now();

		printf("Finished intersecting in %lu.\n", (unsigned long)std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count());
	}

	{
		printf("Intersecting %drays - 1capsule... %d times\n", SOA_SIZE, SOA_INTERSECTC * REPEAT_C);

		auto t1 = Clock::now();

		for (volatile long r = 0; r < REPEAT_C; r++)
			for (volatile long i = 0; i < SOA_INTERSECTC; i++)
				[[maybe_unused]] volatile auto res = testCollider.IntersectSOA(raysSoa[i][0], raysSoa[i][1]);

		auto t2 = Clock::now();

		printf("Finished intersecting in %lu.\n", (unsigned long)std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count());
	}

	{
		printf("Intersecting 1ray - %dcapsules... %d times\n", SOA_SIZE, SOA_INTERSECTC * REPEAT_C);

		auto t1 = Clock::now();

		for (volatile long r = 0; r < REPEAT_C; r++)
			for (volatile long i = 0; i < SOA_INTERSECTC; i++)
				[[maybe_unused]] volatile auto res = testColliderSOA.Intersect(rays[i][0], rays[i][1]);

		auto t2 = Clock::now();

		printf("Finished intersecting in %lu.\n", (unsigned long)std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count());
	}

	return 0;
}
