#ifndef HANDLES_H
#define HANDLES_H

#if defined(__linux__) || defined(__APPLE__)
typedef void* MHandle;
#else
typedef HModule MHandle;
#endif

namespace Handles
{
	MHandle GetModuleHandle(const char* module);
}

#endif
