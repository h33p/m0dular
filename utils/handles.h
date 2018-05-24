#ifndef HANDLES_H
#define HANDLES_H

#if defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
typedef void* MHandle;
#else
#include "../wincludes.h"
typedef HMODULE MHandle;
#endif

typedef struct
{
	MHandle handle;
	uintptr_t address;
	size_t size;
} ModuleInfo;

namespace Handles
{
	MHandle GetModuleHandle(const char* module);
	ModuleInfo GetModuleInfo(const char* module);
}

#endif
