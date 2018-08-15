#ifndef MEMUTILS_H
#define MEMUTILS_H

#include "pattern_scan.h"
#include "handles.h"

//External programs might want to use custom RPM/WPM functions
#ifndef MEMUTILS_CUSTOM_RW
template<typename T, typename N>
inline T Read(N addr)
{
	return *(T*)addr;
}

template<typename T, typename N>
inline void Write(N addr, T value)
{
	return *(T*)addr = value;
}

#endif

inline uintptr_t GetAbsoluteAddress(uintptr_t addr, intptr_t offset, intptr_t instructionSize)
{
	return addr + Read<int>(addr + offset) + instructionSize;
}

template<typename T, size_t idx, typename N>
inline T GetVFunc(N* inst)
{
	return Read<T>(Read<T*>(inst) + idx);
}

#endif
