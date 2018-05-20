#include "handles.h"
#include <vector>
#include <mutex>
#include "string.h"

#if defined(_WIN32)
#include <Psapi.h>
#elif defined(__linux__)
#include <dlfcn.h>
#include <link.h>
#include <sys/mman.h>
#elif defined(__APPLE__)
#include <dlfcn.h>
#include <sys/mman.h>
#include <mach/mach_traps.h>
#include <mach/mach_init.h>
#include <mach/mach_error.h>
#include <mach/mach.h>
#include <mach-o/dyld_images.h>
#include <mach-o/loader.h>
#endif

#if defined(__linux__) || defined(__APPLE__)
struct dlinfo_t
{
	const char* library = nullptr;
	uintptr_t address = 0;
	size_t size = 0;
};

std::vector<dlinfo_t> libraries;
std::mutex lInfoLock;

#endif

#ifdef __linux__
int DlIterateCallback(struct dl_phdr_info* info, size_t, void*)
{
	dlinfo_t libraryInfo;
	libraryInfo.library = info->dlpi_name;
	libraryInfo.address = info->dlpi_addr + info->dlpi_phdr[0].p_vaddr;
	libraryInfo.size = info->dlpi_phdr[0].p_memsz;

	libraries.push_back(libraryInfo);

	return 0;
}
#endif

static void InitializeLibraries()
{
#if defined(__APPLE__)

	struct task_dyld_info dyldInfo;
	vm_address_t address = 0;

	mach_msg_type_number_t count = TASK_DYLD_INFO_COUNT;
	if (task_info(current_task(), TASK_DYLD_INFO, (task_info_t)&dyldInfo, &count) == KERN_SUCCESS)
		address = (vm_address_t)dyldInfo.all_image_info_addr;

	struct dyld_all_image_infos* dyldaii;
	mach_msg_type_number_t size = sizeof(dyld_all_image_infos);
	vm_offset_t readMem;
	kern_return_t kr = vm_read(current_task(), address, size, &readMem, &size);
	if (kr != KERN_SUCCESS)
		return;

	dyldaii = (dyld_all_image_infos*) readMem;
	int imageCount = dyldaii->infoArrayCount;
	mach_msg_type_number_t dataCnt = imageCount * 24;
	struct dyld_image_info* gdii = nullptr;
	dii = (struct dyld_image_info*) malloc (dataCnt);
	// 32bit bs 64bit
	kern_return_t kr2 = vm_read(current_task(), (vm_address_t)dyldaii->infoArray, dataCnt, &readMem, &dataCnt);
	if (kr2) {
		free(gdii);
		return;
	}
	struct dyld_image_info* dii = (struct dyld_image_info*) readMem;
	for (int i = 0; i < imageCount; i++) {
		dataCnt = 1024;
		vm_read(current_task(), (vm_address_t)dii[i].imageFilePath, dataCnt, &readMem, &dataCnt);
		char *imageName = (char *)readMem;

		if (imageName)
			gdii[i].imageFilePath = strdup(imageName);
		else
			gdii[i].imageFilePath = NULL;
		gdii[i].imageLoadAddress = dii[i].imageLoadAddress;

		dlinfo_t libraryInfo;
		struct stat st;
		stat(imageName, &st);

		libraryInfo.address = (vm_address_t)dii[i].imageLoadAddress;
		libraryInfo.size = st.st_size;
		libraryInfo.library = gdii[i].imageFilePath;

		libraries.push_back(libraryInfo);
	}

	free(gdii);
#elif defined(__linux__)
	dl_iterate_phdr(DlIterateCallback, nullptr);
#endif
}

MHandle Handles::GetModuleHandle(const char* module)
{
#if defined(__APPLE__) || defined(__linux__)
	lInfoLock.lock();
	if (!libraries.size())
		InitializeLibraries();
	lInfoLock.unlock();

	for (dlinfo_t& i : libraries)
		if (strstr(i.library, module))
			return dlopen(i.library, RTLD_NOLOAD | RTLD_NOW);
	return nullptr;
#else
	return GetModuleHandleA(module);
#endif
}
