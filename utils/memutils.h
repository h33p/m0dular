#ifndef MEMUTILS_H
#define MEMUTILS_H

#include "pattern_scan.h"
#include "handles.h"

inline uintptr_t GetAbsoluteAddress(uintptr_t addr, intptr_t offset, intptr_t instructionSize)
{
	return addr + *(int*)(addr + offset) + instructionSize;
}

template<typename T, size_t idx, typename N>
inline T GetVFunc(N* inst)
{
	return (*(T**)inst)[idx];
}

#endif
