#include "vfhook.h"
#include "assert.h"
#include "string.h"
#include "stdint.h"

VFuncHook::VFuncHook()
{
	overridePointers = false;
	vtableLength = 0;
	curVTable = nullptr;
	oldVTable = nullptr;
	indexes = nullptr;
}

VFuncHook::VFuncHook(void* base, bool overrideMode, int minSize)
{
	classBase = (uintptr_t**)base;

	indexes = new std::unordered_map<void*, size_t>();

	oldVTable = *classBase;
	overridePointers = overrideMode;

	vtableLength = EstimateVTableLength(oldVTable, minSize);

#ifdef _WIN32
	curVTable = (uintptr_t*)malloc(sizeof(uintptr_t*) * (vtableLength + 1));
	curVTable++;
	memcpy((void*)(curVTable - 1), (void*)(oldVTable - 1), sizeof(uintptr_t*) * (vtableLength + 1));
#else
	curVTable = (uintptr_t*)malloc(sizeof(uintptr_t*) * vtableLength);
	memcpy((void*)curVTable, (void*)oldVTable, sizeof(uintptr_t*) * vtableLength);
#endif

	if (overridePointers) {
		oldVTable = curVTable;
		curVTable = *classBase;
	} else
		*classBase = curVTable;
}

VFuncHook::~VFuncHook()
{
	UnhookAll();
	uintptr_t* vtbl = overridePointers ? oldVTable : curVTable;
#ifdef _WIN32
	vtbl--;
#endif
	free(vtbl);
}

void VFuncHook::UpdateBase(void* base)
{
	classBase = (uintptr_t**)base;
}

void VFuncHook::UnhookID(size_t index)
{
	assert(index < vtableLength);
	curVTable[index] = oldVTable[index];
}

void VFuncHook::UnhookAll()
{
	if (overridePointers)
		memcpy((void*)curVTable, (void*)oldVTable, sizeof(intptr_t) * vtableLength);
	else
		*classBase = oldVTable;
}

size_t VFuncHook::EstimateVTableLength(uintptr_t* vtable, int minSize)
{
	size_t len = 0;
	while(*vtable++ || len < minSize) len++;
	return len;
}
