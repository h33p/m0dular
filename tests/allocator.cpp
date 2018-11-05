#include <stdio.h>
#include <time.h>
#include "../utils/packed_heap.h"

#include <chrono>
#include <algorithm>

const int TEST_SIZE = 400000;
const int RTABLE_SIZE = TEST_SIZE / 100;
const int RAND_RANGE = 10;
const int HEAP_TRASH_SIZE = 1000000;
const int HEAP_TRASH_RAND_RANGE = 50;

typedef std::chrono::high_resolution_clock Clock;

struct TempStruct
{
	uintptr_t a, b, c;
	bool enable;
	char data[20];
	float x, y, z;
	void* datap;

	TempStruct()
	{
		//Something ASAN would catch if constructing/destructing was not clean
		datap = malloc(10);
	}

	TempStruct(const TempStruct& o)
		: TempStruct() {}

	~TempStruct()
	{
		if (datap)
			free(datap);
	}
};

TempStruct* mallocedPtrs[TEST_SIZE];
idx_t allocedPtrs[TEST_SIZE];
int freeOrder[TEST_SIZE];
int randTable[RTABLE_SIZE];
void* heapTrash[HEAP_TRASH_SIZE];

PackedAllocator alloc(sizeof(TempStruct) * TEST_SIZE / 2);
PackedHeap<TempStruct> allocT(TEST_SIZE / 2);
PackedHeapL<TempStruct> allocL;

int main()
{
	srand(time(nullptr));

	for (int i = 0; i < HEAP_TRASH_SIZE; i++)
		heapTrash[i] = malloc(1 + rand() % HEAP_TRASH_RAND_RANGE);

	std::random_shuffle(heapTrash, heapTrash + HEAP_TRASH_SIZE);

	for (int i = 0; i < HEAP_TRASH_SIZE / 4 * 3; i++)
		free(heapTrash[i]);

	for (int i = 0; i < RTABLE_SIZE; i++)
		randTable[i] = 1 + i % RAND_RANGE;

	std::random_shuffle(randTable, randTable + RTABLE_SIZE);

	for (int i = 0; i < TEST_SIZE; i++)
		freeOrder[i] = i;

	for (int i = 0; i < 4; i++)
		std::random_shuffle(freeOrder + i * (TEST_SIZE / 4), freeOrder + (i + 1) * (TEST_SIZE / 4));

	printf("Initialized!\n\nRunning randomized count allocations...\n");

	{
		auto t1 = Clock::now();

		int i = 0;

		for (i = 0; i < TEST_SIZE / 4; i++)
			allocedPtrs[i] = alloc.Alloc(sizeof(TempStruct) * randTable[i % RTABLE_SIZE]);

		for (; i < (TEST_SIZE / 4) * 2; i++) {
			alloc.Free(allocedPtrs[freeOrder[i - TEST_SIZE / 4]]);
			allocedPtrs[i] = alloc.Alloc(sizeof(TempStruct) * randTable[i % RTABLE_SIZE]);
		}

		for (; i < (TEST_SIZE / 4) * 3; i++) {
			alloc.Free(allocedPtrs[freeOrder[i - TEST_SIZE / 4]]);
			allocedPtrs[i] = alloc.Alloc(sizeof(TempStruct) * randTable[i % RTABLE_SIZE]);
		}

		for (; i < TEST_SIZE; i++) {
			alloc.Free(allocedPtrs[freeOrder[i - TEST_SIZE / 4]]);
			allocedPtrs[i] = alloc.Alloc(sizeof(TempStruct) * randTable[i % RTABLE_SIZE]);
		}

		for (; i < TEST_SIZE / 4 * 5; i++)
			alloc.Free(allocedPtrs[freeOrder[i - TEST_SIZE / 4]]);

		auto t2 = Clock::now();

		long diffAlloc = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

		uintptr_t minAddr = ~uintptr_t(0);
		uintptr_t maxAddr = 0;

		for (i = 0; i < TEST_SIZE; i++) {
			minAddr = std::min(minAddr, (uintptr_t)allocedPtrs[i]);
			maxAddr = std::max(maxAddr, (uintptr_t)allocedPtrs[i]);
		}

		printf("Alloc finished in %ld ms. Largest memory difference: %lx (%u %u %u %u)\n", diffAlloc, (maxAddr - minAddr) / sizeof(TempStruct), alloc.totalAllocations, alloc.totalResizes, alloc.totalFrees, alloc.totalReallocations);
	}

	{
		auto t1 = Clock::now();

		int i = 0;

		for (i = 0; i < TEST_SIZE / 4; i++)
			allocedPtrs[i] = allocT.New(randTable[i % RTABLE_SIZE]);

		for (; i < (TEST_SIZE / 4) * 2; i++) {
			allocT.Delete(allocedPtrs[freeOrder[i - TEST_SIZE / 4]]);
			allocedPtrs[i] = allocT.New(randTable[i % RTABLE_SIZE]);
		}

		for (; i < (TEST_SIZE / 4) * 3; i++) {
			allocT.Delete(allocedPtrs[freeOrder[i - TEST_SIZE / 4]]);
			allocedPtrs[i] = allocT.New(randTable[i % RTABLE_SIZE]);
		}

		for (; i < TEST_SIZE; i++) {
			allocT.Delete(allocedPtrs[freeOrder[i - TEST_SIZE / 4]]);
			allocedPtrs[i] = allocT.New(randTable[i % RTABLE_SIZE]);
		}

		for (; i < TEST_SIZE / 4 * 5; i++)
			allocT.Delete(allocedPtrs[freeOrder[i - TEST_SIZE / 4]]);

		auto t2 = Clock::now();

		long diffAlloc = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

		uintptr_t minAddr = ~uintptr_t(0);
		uintptr_t maxAddr = 0;

		for (i = 0; i < TEST_SIZE; i++) {
			minAddr = std::min(minAddr, (uintptr_t)allocedPtrs[i]);
			maxAddr = std::max(maxAddr, (uintptr_t)allocedPtrs[i]);
		}

		printf("AllocT finished in %ld ms. Largest memory difference: %lx (%u %u %u %u)\n", diffAlloc, (maxAddr - minAddr) / sizeof(TempStruct), alloc.totalAllocations, alloc.totalResizes, alloc.totalFrees, alloc.totalReallocations);
	}

	{
		auto t1 = Clock::now();

		int i = 0;

		for (i = 0; i < TEST_SIZE / 4; i++)
			mallocedPtrs[i] = new TempStruct[randTable[i % RTABLE_SIZE]];

		for (; i < (TEST_SIZE / 4) * 2; i++) {
			delete[] mallocedPtrs[freeOrder[i - TEST_SIZE / 4]];
			mallocedPtrs[i] = new TempStruct[randTable[i % RTABLE_SIZE]];
		}

		for (; i < (TEST_SIZE / 4) * 3; i++) {
			delete[] mallocedPtrs[freeOrder[i - TEST_SIZE / 4]];
			mallocedPtrs[i] = new TempStruct[randTable[i % RTABLE_SIZE]];
		}

		for (; i < TEST_SIZE; i++) {
			delete[] mallocedPtrs[freeOrder[i - TEST_SIZE / 4]];
			mallocedPtrs[i] = new TempStruct[randTable[i % RTABLE_SIZE]];
		}

		for (; i < TEST_SIZE / 4 * 5; i++)
			delete[] mallocedPtrs[freeOrder[i - TEST_SIZE / 4]];

		auto t2 = Clock::now();

		long diffMalloc = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

		uintptr_t minAddr = ~uintptr_t(0);
		uintptr_t maxAddr = 0;

		for (i = 0; i < TEST_SIZE; i++) {
			minAddr = std::min(minAddr, (uintptr_t)mallocedPtrs[i]);
			maxAddr = std::max(maxAddr, (uintptr_t)mallocedPtrs[i]);
		}

		printf("New[] finished in %ld ms. Largest memory difference: %lx\n", diffMalloc, (maxAddr - minAddr) / sizeof(TempStruct));
	}

	{
		auto t1 = Clock::now();

		int i = 0;

		for (i = 0; i < TEST_SIZE / 4; i++)
			mallocedPtrs[i] = (TempStruct*)malloc(sizeof(TempStruct) * randTable[i % RTABLE_SIZE]);

		for (; i < (TEST_SIZE / 4) * 2; i++) {
			free(mallocedPtrs[freeOrder[i - TEST_SIZE / 4]]);
			mallocedPtrs[i] = (TempStruct*)malloc(sizeof(TempStruct) * randTable[i % RTABLE_SIZE]);
		}

		for (; i < (TEST_SIZE / 4) * 3; i++) {
		    free(mallocedPtrs[freeOrder[i - TEST_SIZE / 4]]);
			mallocedPtrs[i] = (TempStruct*)malloc(sizeof(TempStruct) * randTable[i % RTABLE_SIZE]);
		}

		for (; i < TEST_SIZE; i++) {
		    free(mallocedPtrs[freeOrder[i - TEST_SIZE / 4]]);
			mallocedPtrs[i] = (TempStruct*)malloc(sizeof(TempStruct) * randTable[i % RTABLE_SIZE]);
		}

		for (; i < TEST_SIZE / 4 * 5; i++)
		    free(mallocedPtrs[freeOrder[i - TEST_SIZE / 4]]);

		auto t2 = Clock::now();

		long diffMalloc = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

		uintptr_t minAddr = ~uintptr_t(0);
		uintptr_t maxAddr = 0;

		for (i = 0; i < TEST_SIZE; i++) {
			minAddr = std::min(minAddr, (uintptr_t)mallocedPtrs[i]);
			maxAddr = std::max(maxAddr, (uintptr_t)mallocedPtrs[i]);
		}

		printf("Malloc finished in %ld ms. Largest memory difference: %lx\n", diffMalloc, (maxAddr - minAddr) / sizeof(TempStruct));
	}

	printf("\nRunning single element allocations\n");

	alloc = PackedAllocator(sizeof(TempStruct) * TEST_SIZE / 4);
	allocT = PackedHeap<TempStruct>(TEST_SIZE / 4);

	{
		auto t1 = Clock::now();

		int i = 0;

		for (i = 0; i < TEST_SIZE / 4; i++)
			allocedPtrs[i] = alloc.Alloc(sizeof(TempStruct));

		for (; i < (TEST_SIZE / 4) * 2; i++) {
			alloc.Free(allocedPtrs[freeOrder[i - TEST_SIZE / 4]]);
			allocedPtrs[i] = alloc.Alloc(sizeof(TempStruct));
		}

		for (; i < (TEST_SIZE / 4) * 3; i++) {
			alloc.Free(allocedPtrs[freeOrder[i - TEST_SIZE / 4]]);
			allocedPtrs[i] = alloc.Alloc(sizeof(TempStruct));
		}

		for (; i < TEST_SIZE; i++) {
			alloc.Free(allocedPtrs[freeOrder[i - TEST_SIZE / 4]]);
			allocedPtrs[i] = alloc.Alloc(sizeof(TempStruct));
		}

		for (; i < TEST_SIZE / 4 * 5; i++)
			alloc.Free(allocedPtrs[freeOrder[i - TEST_SIZE / 4]]);

		auto t2 = Clock::now();

		long diffAlloc = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

		uintptr_t minAddr = ~uintptr_t(0);
		uintptr_t maxAddr = 0;

		for (i = 0; i < TEST_SIZE; i++) {
			minAddr = std::min(minAddr, (uintptr_t)allocedPtrs[i]);
			maxAddr = std::max(maxAddr, (uintptr_t)allocedPtrs[i]);
		}

		printf("Alloc finished in %ld ms. Largest memory difference: %lx (%u %u %u %u)\n", diffAlloc, (maxAddr - minAddr) / sizeof(TempStruct), alloc.totalAllocations, alloc.totalResizes, alloc.totalFrees, alloc.totalReallocations);

	}

	{
		auto t1 = Clock::now();

		int i = 0;

		for (i = 0; i < TEST_SIZE / 4; i++)
			allocedPtrs[i] = allocT.New();

		for (; i < (TEST_SIZE / 4) * 2; i++) {
			allocT.Delete(allocedPtrs[freeOrder[i - TEST_SIZE / 4]]);
			allocedPtrs[i] = allocT.New();
		}

		for (; i < (TEST_SIZE / 4) * 3; i++) {
			allocT.Delete(allocedPtrs[freeOrder[i - TEST_SIZE / 4]]);
			allocedPtrs[i] = allocT.New();
		}

		for (; i < TEST_SIZE; i++) {
			allocT.Delete(allocedPtrs[freeOrder[i - TEST_SIZE / 4]]);
			allocedPtrs[i] = allocT.New();
		}

		for (; i < TEST_SIZE / 4 * 5; i++)
			allocT.Delete(allocedPtrs[freeOrder[i - TEST_SIZE / 4]]);

		auto t2 = Clock::now();

		long diffAlloc = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

		uintptr_t minAddr = ~uintptr_t(0);
		uintptr_t maxAddr = 0;

		for (i = 0; i < TEST_SIZE; i++) {
			minAddr = std::min(minAddr, (uintptr_t)allocedPtrs[i]);
			maxAddr = std::max(maxAddr, (uintptr_t)allocedPtrs[i]);
		}

		printf("Alloc finished in %ld ms. Largest memory difference: %lx (%u %u %u %u)\n", diffAlloc, (maxAddr - minAddr) / sizeof(TempStruct), alloc.totalAllocations, alloc.totalResizes, alloc.totalFrees, alloc.totalReallocations);

	}

	{
		auto t1 = Clock::now();

		int i = 0;

		for (i = 0; i < TEST_SIZE / 4; i++)
			allocedPtrs[i] = allocL.Alloc();

		for (; i < (TEST_SIZE / 4) * 2; i++) {
			allocL.Free(allocedPtrs[freeOrder[i - TEST_SIZE / 4]]);
			allocedPtrs[i] = allocL.Alloc();
		}

		for (; i < (TEST_SIZE / 4) * 3; i++) {
			allocL.Free(allocedPtrs[freeOrder[i - TEST_SIZE / 4]]);
			allocedPtrs[i] = allocL.Alloc();
		}

		for (; i < TEST_SIZE; i++) {
			allocL.Free(allocedPtrs[freeOrder[i - TEST_SIZE / 4]]);
			allocedPtrs[i] = allocL.Alloc();
		}

		for (; i < TEST_SIZE / 4 * 5; i++)
			allocL.Free(allocedPtrs[freeOrder[i - TEST_SIZE / 4]]);

		auto t2 = Clock::now();

		long diffAlloc = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

		uintptr_t minAddr = ~uintptr_t(0);
		uintptr_t maxAddr = 0;

		for (i = 0; i < TEST_SIZE; i++) {
			minAddr = std::min(minAddr, (uintptr_t)allocedPtrs[i]);
			maxAddr = std::max(maxAddr, (uintptr_t)allocedPtrs[i]);
		}

		printf("AllocL finished in %ld ms. Largest memory difference: %lx\n", diffAlloc, maxAddr - minAddr);

	}

	{
		auto t1 = Clock::now();

		int i = 0;

		for (i = 0; i < TEST_SIZE / 4; i++)
			mallocedPtrs[i] = new TempStruct();

		for (; i < (TEST_SIZE / 4) * 2; i++) {
			delete mallocedPtrs[freeOrder[i - TEST_SIZE / 4]];
			mallocedPtrs[i] = new TempStruct();
		}

		for (; i < (TEST_SIZE / 4) * 3; i++) {
		    delete mallocedPtrs[freeOrder[i - TEST_SIZE / 4]];
			mallocedPtrs[i] = new TempStruct();
		}

		for (; i < TEST_SIZE; i++) {
		    delete mallocedPtrs[freeOrder[i - TEST_SIZE / 4]];
			mallocedPtrs[i] = new TempStruct();
		}

		for (; i < TEST_SIZE / 4 * 5; i++)
		    delete mallocedPtrs[freeOrder[i - TEST_SIZE / 4]];

		auto t2 = Clock::now();

		long diffMalloc = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

		uintptr_t minAddr = ~uintptr_t(0);
		uintptr_t maxAddr = 0;

		for (i = 0; i < TEST_SIZE; i++) {
			minAddr = std::min(minAddr, (uintptr_t)mallocedPtrs[i]);
			maxAddr = std::max(maxAddr, (uintptr_t)mallocedPtrs[i]);
		}

		printf("New finished in %ld ms. Largest memory difference: %lx\n", diffMalloc, (maxAddr - minAddr) / sizeof(TempStruct));
	}

	{
		auto t1 = Clock::now();

		int i = 0;

		for (i = 0; i < TEST_SIZE / 4; i++)
			mallocedPtrs[i] = (TempStruct*)malloc(sizeof(TempStruct));

		for (; i < (TEST_SIZE / 4) * 2; i++) {
			free(mallocedPtrs[freeOrder[i - TEST_SIZE / 4]]);
			mallocedPtrs[i] = (TempStruct*)malloc(sizeof(TempStruct));
		}

		for (; i < (TEST_SIZE / 4) * 3; i++) {
		    free(mallocedPtrs[freeOrder[i - TEST_SIZE / 4]]);
			mallocedPtrs[i] = (TempStruct*)malloc(sizeof(TempStruct));
		}

		for (; i < TEST_SIZE; i++) {
		    free(mallocedPtrs[freeOrder[i - TEST_SIZE / 4]]);
			mallocedPtrs[i] = (TempStruct*)malloc(sizeof(TempStruct));
		}

		for (; i < TEST_SIZE / 4 * 5; i++)
		    free(mallocedPtrs[freeOrder[i - TEST_SIZE / 4]]);

		auto t2 = Clock::now();

		long diffMalloc = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

		uintptr_t minAddr = ~uintptr_t(0);
		uintptr_t maxAddr = 0;

		for (i = 0; i < TEST_SIZE; i++) {
			minAddr = std::min(minAddr, (uintptr_t)mallocedPtrs[i]);
			maxAddr = std::max(maxAddr, (uintptr_t)mallocedPtrs[i]);
		}

		printf("Malloc finished in %ld ms. Largest memory difference: %lx\n", diffMalloc, (maxAddr - minAddr) / sizeof(TempStruct));
	}

	for (int i = HEAP_TRASH_SIZE / 4 * 3; i < HEAP_TRASH_SIZE; i++)
		free(heapTrash[i]);

	return 0;
}
