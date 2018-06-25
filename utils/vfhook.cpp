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

	curVTable = (uintptr_t*)malloc(sizeof(uintptr_t*) * (vtableLength + 2));
	curVTable += 2;
	memcpy((void*)(curVTable - 2), (void*)(oldVTable - 2), sizeof(uintptr_t*) * (vtableLength + 2));

	if (overridePointers) {
		oldVTable = curVTable;
		curVTable = *classBase;
	} else
		*classBase = curVTable;
}

VFuncHook::~VFuncHook()
{
	if (*classBase == curVTable)
		UnhookAll();
	uintptr_t* vtbl = overridePointers ? oldVTable : curVTable;
	vtbl -= 2;
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
	while(*vtable++ || (int)len < minSize) len++;
	return len;
}
