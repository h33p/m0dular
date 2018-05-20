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

VFuncHook::VFuncHook(void* base, bool overrideMode)
{
	classBase = (uintptr_t**)base;

	oldVTable = *classBase;
	overridePointers = overrideMode;

	vtableLength = EstimateVTableLength(oldVTable);

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
	}
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

template<typename T>
void VFuncHook::Hook(size_t index, T function)
{
	assert(index < vtableLength && indexes);
	indexes->insert({(void*)function, index});
	curVTable[index] = (uintptr_t)function;
}

template<typename T>
void VFuncHook::Unhook(T function)
{
	assert(indexes->find(function) != indexes->end());
	size_t idx = indexes->at(function);
	curVTable[idx] = oldVTable[idx];
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

template<typename T, typename F>
T VFuncHook::GetOriginal(F func)
{
	assert(indexes->find(func) != indexes->end());
	return (T)indexes->at(func);
}

template<typename T>
T VFuncHook::GetOriginalByIndex(size_t index)
{
	return (T)oldVTable[index];
}

size_t VFuncHook::EstimateVTableLength(uintptr_t* vtable)
{
	size_t len = 0;
	while(*vtable++) len++;
	return len;
}
