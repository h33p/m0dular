#ifndef FREELISTALLOCATOR_ENUMS
#define FREELISTALLOCATOR_ENUMS
enum PlacementPolicy {
	FIND_FIRST,
	FIND_BEST
};
#endif

#ifndef FREELISTALLOCATOR_H
#define FREELISTALLOCATOR_H

#include <stdint.h>
#include <stddef.h>
#include <type_traits>
#include "shared_utils.h"
#include <iterator>
#include "allocwraps.h"
#include <stdlib.h>
#include <algorithm>
#include <string.h>

/*
  Credits: mtrebi
  Added purpose based modifications.

  This allocator is usable by all stl containers and is usable in shared memory, it requires a global uintptr variable base which provides the necessary offset base for use in shared memory
*/

static inline size_t CalculatePadding(size_t baseAddress, size_t alignment) {
	size_t multiplier = (baseAddress / alignment) + 1;
	size_t alignedAddress = multiplier * alignment;
	size_t padding = alignedAddress - baseAddress;
	return padding;
}

static inline size_t CalculatePaddingWithHeader(size_t baseAddress, size_t alignment, size_t headerSize) {
	size_t padding = CalculatePadding(baseAddress, alignment);
	size_t neededSpace = headerSize;

	if (padding < neededSpace){
		// Header does not fit - Calculate next aligned address that header fits
		neededSpace -= padding;

		// How many alignments I need to fit the header
		if (neededSpace % alignment > 0)
			padding += alignment * (1 + (neededSpace / alignment));
		else
			padding += alignment * (neededSpace / alignment);
	}

	return padding;
}

template <class T, auto& BASE>
class SinglyLinkedList {
  public:
	struct Node {
		T data;
		offset_pointer_t<Node, BASE> next;
	};

	using NodePtr = offset_pointer_t<Node, BASE>;

	offset_pointer_t<Node, BASE> head;

  public:
	SinglyLinkedList()
	{

	}

	void insert(NodePtr previousNode, NodePtr newNode){
		if (!previousNode) {
			newNode->next = head;
			head = newNode;
		} else {
			newNode->next = previousNode->next;
			previousNode->next = newNode;
		}
	}

	void remove(NodePtr previousNode, NodePtr deleteNode){
		if (!previousNode)
			head = deleteNode->next;
		else
			previousNode->next = deleteNode->next;
	}
};

class Allocator {
  protected:
	size_t totalSize;
	size_t used;
	size_t peak;
  public:
	Allocator(size_t inTotalSize)
		: totalSize(inTotalSize), used(), peak() {}

	~Allocator()
	{
		totalSize = 0;
	}
};

struct FreeHeader {
	size_t blockSize;
};

struct AllocationHeader {
	size_t blockSize;
	char padding;
};

template<auto& BASE, bool REALLOCATABLE>
class FreeListAllocator : public Allocator {
  protected:

	using Node = typename SinglyLinkedList<FreeHeader, BASE>::Node;
	using NodePtr = typename SinglyLinkedList<FreeHeader, BASE>::NodePtr;
	template<typename T>
	using pointer_t = offset_pointer_t<T, BASE>;
	using AllocationHeaderPtr = pointer_t<AllocationHeader>;

	uintptr_t baseOffset;
	offset_pointer_t<void*, BASE> start_ptr = 0;
	PlacementPolicy pPolicy;
	SinglyLinkedList<FreeHeader, BASE> freeList;
	bool freeOnExit;

  public:
	FreeListAllocator(size_t totalSize, const PlacementPolicy inpPolicy, void* startPtr = nullptr)
		: Allocator(totalSize) {
		pPolicy = inpPolicy;
		start_ptr = (void**)startPtr;
		freeOnExit = false;

		Init(startPtr);
	}

	~FreeListAllocator()
	{
		if (freeOnExit && start_ptr)
			free((void*)&*start_ptr);
		start_ptr = nullptr;
	}

	pointer_t<void*> Allocate(size_t size, size_t alignment = 0)
	{
		size_t allocationHeaderSize = sizeof(AllocationHeader);
		//size_t freeHeaderSize = sizeof(FreeHeader);

		size = std::max(size, sizeof(Node));
		alignment = std::max(alignment, size_t(8));

		// Search through the free list for a free block that has enough space to allocate our data
		size_t padding = 0;
		NodePtr affectedNode, previousNode;
		this->Find(size, alignment, padding, previousNode, affectedNode);

		if (!affectedNode)
			if constexpr (REALLOCATABLE) {
				Reallocate(size);
				this->Find(size, alignment, padding, previousNode, affectedNode);
			}

		if (!affectedNode)
#if defined(__cpp_exceptions) || defined(_CPPUNWIND)
			throw;
#else
		return nullptr;
#endif

		size_t alignmentPadding = padding - allocationHeaderSize;
		size_t requiredSize = size + padding;

		size_t rest = affectedNode->data.blockSize - requiredSize;

		if (rest > 0) {
			// We have to split the block into the data block and a free block of size 'rest'
			NodePtr newFreeNode = NodePtr((size_t) affectedNode + requiredSize);
			newFreeNode->data.blockSize = rest;
			freeList.insert(affectedNode, newFreeNode);
		}
		freeList.remove(previousNode, affectedNode);

		// Setup data block
		size_t headerAddress = (size_t) affectedNode + alignmentPadding;
		size_t dataAddress = headerAddress + allocationHeaderSize;
		AllocationHeaderPtr(headerAddress)->blockSize = requiredSize;
		AllocationHeaderPtr(headerAddress)->padding = alignmentPadding;

		used += requiredSize;
		peak = std::max(peak, used);

		return (void**)dataAddress;
	}

	void Free(pointer_t<void*> ptr)
	{
		// Insert it in a sorted position by the address number
		size_t currentAddress = (size_t) ptr;
		size_t headerAddress = currentAddress - sizeof (AllocationHeader);
		const AllocationHeaderPtr allocationHeader(headerAddress);

		NodePtr freeNode = NodePtr(headerAddress);
		freeNode->data.blockSize = allocationHeader->blockSize + allocationHeader->padding;
		freeNode->next = nullptr;

		NodePtr it = freeList.head;
		NodePtr itPrev = nullptr;
		while (it) {
			if (ptr < it) {
				freeList.insert(itPrev, freeNode);
				break;
			}
			itPrev = it;
			it = it->next;
		}

		used -= freeNode->data.blockSize;

		// Merge contiguous nodes
		Coalescence(itPrev, freeNode);
	}

	void Init(void* startPtr = nullptr)
	{
		if (freeOnExit && start_ptr) {
			free((void*)&*start_ptr);
			start_ptr = nullptr;
		}

		if (!startPtr) {
			void** startPtr2 = (void**)malloc(totalSize);
			if (!startPtr2)
				throw;
			if constexpr (REALLOCATABLE) {
				baseOffset = 0;
				BASE = (uintptr_t)startPtr2;
			}
			start_ptr = startPtr2;
			freeOnExit = true;
		} else {
			start_ptr = (void**)startPtr;
			baseOffset = (uintptr_t)&*start_ptr - (uintptr_t)BASE;
		}

		this->Reset();
	}

	void Reset()
	{
		used = 0;
		peak = 0;
		NodePtr firstNode = NodePtr(start_ptr);
		firstNode->data.blockSize = totalSize;
		firstNode->next = nullptr;
		freeList.head = nullptr;
		freeList.insert(nullptr, firstNode);
	}
  protected:
	FreeListAllocator(FreeListAllocator &freeListAllocator);

	void Coalescence(NodePtr previousNode, NodePtr freeNode)
	{
		if (freeNode->next &&
			(size_t) freeNode + freeNode->data.blockSize == (size_t) freeNode->next) {
			freeNode->data.blockSize += freeNode->next->data.blockSize;
			freeList.remove(freeNode, freeNode->next);
		}

		if (previousNode &&
			(size_t) previousNode + previousNode->data.blockSize == (size_t) freeNode) {
			previousNode->data.blockSize += freeNode->data.blockSize;
			freeList.remove(previousNode, freeNode);
		}
	}

	void Find(size_t size, size_t alignment, size_t& padding, NodePtr& previousNode, NodePtr& foundNode)
	{
		switch (pPolicy) {
		  case FIND_FIRST:
			  FindFirst(size, alignment, padding, previousNode, foundNode);
			  break;
		  case FIND_BEST:
			  FindBest(size, alignment, padding, previousNode, foundNode);
			  break;
		}
	}

	void FindBest(size_t size, size_t alignment, size_t& padding, NodePtr& previousNode, NodePtr& foundNode)
	{
		// Iterate WHOLE list keeping a pointer to the best fit
		size_t smallestDiff = std::numeric_limits<size_t>::max();
		NodePtr bestBlock = nullptr;
		NodePtr it = freeList.head, itPrev = nullptr;
		while (it) {
			padding = CalculatePaddingWithHeader((size_t)it, alignment, sizeof (AllocationHeader));
			size_t requiredSpace = size + padding;
			if (it->data.blockSize >= requiredSpace && (it->data.blockSize - requiredSpace < smallestDiff)) {
				bestBlock = it;
			}
			itPrev = it;
			it = it->next;
		}
		previousNode = itPrev;
		foundNode = bestBlock;
	}

	void FindFirst(size_t size, size_t alignment, size_t& padding, NodePtr& previousNode, NodePtr& foundNode)
	{
		//Iterate list and return the first free block with a size >= than given size
		NodePtr it = freeList.head, itPrev = nullptr;

		while (it) {
			padding = CalculatePaddingWithHeader((size_t)it, alignment, sizeof (AllocationHeader));
			size_t requiredSpace = size + padding;
			if (it->data.blockSize >= requiredSpace) {
				break;
			}
			itPrev = it;
			it = it->next;
		}
		previousNode = itPrev;
		foundNode = it;
	}

	void Reallocate(size_t neededSize)
	{
		if constexpr (!REALLOCATABLE)
#if defined(__cpp_exceptions) || defined(_CPPUNWIND)
			throw;
#else
		return;
#endif
		else {
			size_t newSize = (totalSize + neededSize) * 2;
			size_t oldSize = totalSize;
			//Possibly change this to realloc
			void* newPtr = malloc(newSize);
			void* oldPtr = (void*)&*start_ptr;
			memcpy(newPtr, oldPtr, totalSize);
			free(oldPtr);
			totalSize = newSize;

			BASE = (uintptr_t)newPtr - baseOffset;

			NodePtr it = freeList.head, itPrev = nullptr;

			while (it) {
				itPrev = it;
				it = it->next;
			}

			NodePtr lastNode = NodePtr(BASE + (uintptr_t)oldSize);
			lastNode->data.blockSize = totalSize - oldSize;
		    lastNode->next = nullptr;
			freeList.insert(itPrev, lastNode);
			Coalescence(itPrev, lastNode);
		}
	}
};

#endif
