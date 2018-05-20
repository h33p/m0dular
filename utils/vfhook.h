#ifndef VFHOOK_H
#define VFHOOK_H

#include "math.h"
#include <unordered_map>

class VFuncHook
{
  public:
	VFuncHook();
	VFuncHook(void* base, bool overrideMode = false);
	~VFuncHook();

	void UpdateBase(void* base);

	template<typename T>
	void Hook(size_t index, T function);

	template<typename T>
	void Unhook(T function);
	void UnhookID(size_t index);
	void UnhookAll();

	template<typename T, typename F>
	T GetOriginal(F func);
	template<typename T>
	T GetOriginalByIndex(size_t index);

  private:
	size_t EstimateVTableLength(uintptr_t* vtable);

	uintptr_t** classBase;

	bool overridePointers;
	size_t vtableLength;

	uintptr_t* curVTable;
	uintptr_t* oldVTable;

	std::unordered_map<void*, size_t>* indexes;
};

#endif
