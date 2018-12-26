#include <stdio.h>
#include "../utils/freelistallocator.h"
#include "../utils/allocwraps.h"
#include <vector>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#endif
#include "../utils/named_semaphores.h"
#include <atomic>

uintptr_t allocBase = 0;

generic_free_list_allocator<allocBase>* alloc = nullptr;
std::vector<int, stateful_allocator<int, alloc>>* vec = nullptr;
NamedSemaphore sem("/shm_test_sem");

constexpr unsigned long msz = 1 << 20;

bool MapSharedMemory(void*& addr);
void UnmapSharedMemory(void* addr);

int main()
{

	std::atomic_bool b;
	printf("LCK %d\n", (int)b.is_lock_free());
	void* addr = nullptr;
	bool firstTime = MapSharedMemory(addr);

	if (addr) {
		allocBase = (uintptr_t)addr;

		printf("Alloc base: %lx\n", allocBase);

		if (firstTime) {
			alloc = new((decltype(alloc))allocBase) generic_free_list_allocator<allocBase>((1 << 20) - 500, PlacementPolicy::FIND_FIRST, (void*)(allocBase + 200));
			vec = new((decltype(vec))(allocBase + sizeof(*alloc))) std::vector<int, stateful_allocator<int, alloc>>(10);
		} else {
			alloc = (decltype(alloc))allocBase;
			vec = (decltype(vec))(allocBase + sizeof(*alloc));
		}
		int n;

		scanf("%d", &n);

		if (firstTime) {
			for (int i = 0; i < n; i++) {
				int m;
				scanf("%d", &m);
				printf("PUSHING %d to vec\n", m);
				vec->push_back(m);
				printf("PUSHED %d to vec (%zu %d)\n", m, vec->size(), (*vec)[i]);
				sem.Post();
			}

			for (size_t i = 0; i < vec->size(); i++) {
				printf("%d\n", (*vec)[i]);
			}
		} else {
			for (int i = 0; i < n; i++) {
				sem.Wait();
				printf("INTERPUSHED to vec (%zu %d)\n", vec->size(), (*vec)[i]);
			}
		}

		UnmapSharedMemory(addr);
	}


	return 0;
}

bool MapSharedMemory(void*& addr)
{
	bool firstTime = false;

#ifdef _WIN32
	HANDLE mapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "shm_test");

	printf("OPEN FILE %p\n", mapFile);

	if (!(void*)mapFile) {
	    mapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, msz, "shm_test");
		firstTime = true;
	}

	printf("OPEN FILE %p\n", mapFile);

	if (mapFile) {
		addr = (void*)MapViewOfFile(mapFile, FILE_MAP_ALL_ACCESS, 0, 0, msz);
	}
	printf("MAP FILE %p\n", addr);

	if (!firstTime)
		CloseHandle(mapFile);

#else
	int fd = shm_open("shm_test", O_RDWR, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		firstTime = true;
		fd = shm_open("shm_test", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	}
	//firstTime = true;

	if (fd != -1 && ftruncate(fd, msz) != -2)
	    addr = mmap(nullptr, msz, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	if (addr == (void*)-1)
		addr = nullptr;

	if (!firstTime)
		shm_unlink("shm_test");
#endif

	return firstTime;
}

void UnmapSharedMemory(void* addr)
{
#ifdef _WIN32
	UnmapViewOfFile(addr);
#else
	munmap(addr, msz);
#endif
}
