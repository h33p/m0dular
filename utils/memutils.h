#ifndef MEMUTILS_H
#define MEMUTILS_H

#include "pattern_scan.h"

inline uintptr_t GetAbsoluteAddress(uintptr_t addr, intptr_t offset, intptr_t instructionSize)
{
	return addr + *(intptr_t*)(addr + offset) + instructionSize;
}

#endif
