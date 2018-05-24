#ifndef VFHOOK_H
#define VFHOOK_H

#include <assert.h>
#include "math.h"
#include <unordered_map>

class VFuncHook
{
  public:
	VFuncHook();
	VFuncHook(void* base, bool overrideMode = false, int minSize = 0);
	~VFuncHook();

	void UpdateBase(void* base);

	template<typename T>
	void Hook(size_t index, T function)
	{
		assert(index < vtableLength && indexes);
		indexes->insert({(void*)function, index});
		curVTable[index] = (uintptr_t)function;
	}

	template<typename T>
	void Unhook(T function)
	{
		assert(indexes->find(function) != indexes->end());
		size_t idx = indexes->at(function);
		curVTable[idx] = oldVTable[idx];
	}

	void UnhookID(size_t index);
	void UnhookAll();

	template<typename T, typename F>
	T GetOriginal(F func)
	{
		assert(indexes->find(func) != indexes->end());
		return (T)oldVTable[indexes->at(func)];
	}

	template<typename T>
	T GetOriginalByIndex(size_t index)
	{
		return (T)oldVTable[index];
	}

  private:
	size_t EstimateVTableLength(uintptr_t* vtable, int minSize = 0);

	uintptr_t** classBase;

	bool overridePointers;
	size_t vtableLength;

	uintptr_t* curVTable;
	uintptr_t* oldVTable;

	std::unordered_map<void*, size_t>* indexes;
};

#endif
