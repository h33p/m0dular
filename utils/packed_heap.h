#ifndef PACKED_HEAP_H
#define PACKED_HEAP_H

//At the current moment region merge causes higher overall average memory difference, this is to be investigated
#ifndef PACKED_HEAP_MERGE_REGIONS
#define PACKED_HEAP_MERGE_REGIONS 0
#endif

#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <stdlib.h>
#include <string.h>

using idx_t = unsigned int;

//A simple, faster than malloc heap method using less memory, but only supports single element allocation
template<typename T>
class PackedHeapL;

//A more complicated heap method with support for continious sized buffer allocation. At the moment 1 / 4th the speed of (glibc) malloc
template<typename T>
class PackedHeap;

struct MemRegion
{
	idx_t start, end;

	inline bool operator < (const idx_t& o) const
	{
		return start < o;
	}

	inline bool operator > (const idx_t& o) const
	{
		return end > o;
	}

	inline bool operator == (const idx_t& o) const
	{
		return start <= o && end >= o;
	}

	inline bool operator < (const MemRegion& o) const
	{
		return end < o.start;
	}

	inline bool operator > (const MemRegion& o) const
	{
		return start > o.end;
	}

	inline bool operator == (const MemRegion& o) const
	{
		return start == o.start && end == o.end;
	}

	inline bool operator != (const MemRegion& o) const
	{
		return start != o.start || end != o.end;
	}

};

template<typename T>
class PackedHeapL
{
  private:
	std::vector<T> buf;
	std::vector<MemRegion> freeRegions;

  public:
	idx_t Alloc()
	{
		if (freeRegions.size()) {
		    auto end = freeRegions.rbegin();
			idx_t ret = end->end;
			if (end->start == end->end)
				freeRegions.pop_back();
			else
				end->end--;
			return ret;
		}
		buf.resize(buf.size() + 1);
		return buf.size();
	}

	void Free(idx_t idx)
	{
		auto upperReg = std::lower_bound(freeRegions.begin(), freeRegions.end(), idx);
		//Double free!
		if (upperReg != freeRegions.end() && upperReg->start <= idx && upperReg->end <= idx)
#ifdef PACKED_HEAP_DEBUG
			throw std::runtime_error("Double free");
#else
			return;
#endif

		auto lowerReg = upperReg == freeRegions.begin() ? upperReg : upperReg - 1;

		//Join the regions
		if (upperReg != freeRegions.end() && lowerReg->end == idx - 1 && upperReg->start == idx + 1) {
			lowerReg->end = upperReg->end;
			freeRegions.erase(upperReg);
		} else if (upperReg != freeRegions.end() && upperReg->start == idx + 1)
			upperReg->start--;
		else if (lowerReg != freeRegions.end() && lowerReg->end == idx - 1)
			lowerReg->end++;
		else {
			//List was empty
			freeRegions.push_back({idx, idx});
		}
	}

	void Free(T* ptr)
	{
		idx_t idx = ptr - &buf[0];
		if (++idx < buf.size())
			Free(idx);
	}

	inline T& operator[](idx_t idx)
	{
		return buf[idx - 1];
	}

	inline T* operator+(idx_t idx)
	{
		return &buf[idx - 1];
	}
};

class PackedAllocator
{
  protected:

	static const unsigned char USED_REGION = 0xaa;
	static const unsigned char FREE_REGION = 0x88;
	static const unsigned char HOLE_START = 0xba;
	static const unsigned char HOLE_END = 0xab;
	static const unsigned char HOLE_REGION = 0xbb;

	struct MetaData
	{
	    uint8_t used;
		idx_t size;

		inline bool operator==(const MetaData& o) const
		{
			return used == o.used && size == o.size;
		}

		inline bool operator!=(const MetaData& o) const
		{
			return !operator==(o);
		}

		inline MetaData* WalkUp()
		{
			return (MetaData*)((uintptr_t)this + sizeof(MetaData) + size);
		}

		inline MetaData* WalkDown()
		{
			return (MetaData*)((uintptr_t)this - sizeof(MetaData) - size);
		}
	};

	char* buf = nullptr;
	idx_t bufSize = 0;
	idx_t bufCapacity = 0;

	std::map<idx_t, std::set<idx_t>> freeRegionsTree;

	constexpr bool FillHole(idx_t holeStart, idx_t size)
	{
		if (size) {
			if (size == 1) {
				*(unsigned char*)&buf[holeStart] = HOLE_REGION;
				return true;
			} else if (size < 2 * sizeof(MetaData)) {
				*(unsigned char*)&buf[holeStart] = HOLE_START;
				for (idx_t i = 2; i < size; i++)
					*(unsigned char*)&buf[holeStart + i - 1] = 0xff;
				*(unsigned char*)&buf[holeStart + size - 1] = HOLE_END;
				return true;
			}
		}

		return false;
	}

	idx_t _Alloc(idx_t sz);
  public:

	idx_t totalAllocations = 0;
	idx_t totalFrees = 0;
	idx_t totalResizes = 0;
	idx_t totalReallocations = 0;

	PackedAllocator(size_t sz = 10);
	PackedAllocator(const PackedAllocator& o);
	PackedAllocator(const PackedAllocator&& o);
	~PackedAllocator();

	PackedAllocator& operator=(const PackedAllocator& o);
	PackedAllocator& operator=(PackedAllocator&& o);

	idx_t Alloc(idx_t sz = 1);
	void Free(idx_t idx);

	constexpr auto& operator[](idx_t idx) const
	{
		return buf[idx];
	}

	constexpr auto operator+(idx_t idx) const
	{
		return &buf[idx];
	}
};

template<typename T>
class PackedHeap : PackedAllocator
{
  private:

	template<typename U, typename F>
	void WalkBuffer(char* prevBuf, idx_t limit, U holeHandler, F chunkHandler)
	{
		idx_t idx = 0;
		while (idx < limit) {
			MetaData* meta = (MetaData*)&prevBuf[idx];

			if ((unsigned char)meta->used == HOLE_START) {
				idx_t holeStart = idx;
				while ((unsigned char)prevBuf[idx++] != HOLE_END)
					;
				holeHandler(buf, prevBuf, holeStart, idx);
				continue;
			} else if ((unsigned char)meta->used == HOLE_REGION) {
				idx++;
				holeHandler(buf, prevBuf, idx - 1, idx);
				continue;
			}

			chunkHandler(buf, prevBuf, idx, meta);

			idx += meta->size + sizeof(MetaData) * 2;
		}
	}

	static void HoleCopy(char* buf, char* prevBuf, idx_t start, idx_t end)
	{
		memcpy(buf + start, prevBuf + start, end - start);
	}

	static void HoleNull(char* buf, char* prevBuf, idx_t start, idx_t end) {}

	static void MoveChunk(char* buf, char* prevBuf, idx_t idx, MetaData* meta)
	{
		*(MetaData*)&buf[idx] = *meta;
		*((MetaData*)&buf[idx])->WalkUp() = *meta->WalkUp();

		if ((unsigned char)meta->used == USED_REGION) {
			size_t cnt = meta->size / sizeof(T);
			for (size_t i = 0; i < cnt; i++) {
				new(buf + idx + sizeof(MetaData) + i * sizeof(T)) T(*(T*)&prevBuf[idx + sizeof(MetaData) + i * sizeof(T)]);
				((T*)&prevBuf[idx + sizeof(MetaData) + i * sizeof(T)])->~T();
			}
		}
	}

	static void ConstructChunk(char* buf, char* prevBuf, idx_t idx, MetaData* meta)
	{
		*(MetaData*)&buf[idx] = *meta;
		*((MetaData*)&buf[idx])->WalkUp() = *meta->WalkUp();

		if ((unsigned char)meta->used == USED_REGION) {
			size_t cnt = meta->size / sizeof(T);
			for (size_t i = 0; i < cnt; i++)
				new(buf + idx + sizeof(MetaData) + i * sizeof(T)) T(*(T*)&prevBuf[idx + sizeof(MetaData) + i * sizeof(T)]);
		}
	}

	static void DestructChunk(char* buf, char* prevBuf, idx_t idx, MetaData* meta)
	{
		if ((unsigned char)meta->used == USED_REGION) {
			size_t cnt = meta->size / sizeof(T);
			for (size_t i = 0; i < cnt; i++)
				((T*)&prevBuf[idx + sizeof(MetaData) + i * sizeof(T)])->~T();
		}
	}

  public:

	inline auto& operator=(const PackedHeap& o) const
	{
		totalAllocations = o.totalAllocations;
		totalFrees = o.totalFrees;
		totalResizes = o.totalResizes;
		totalReallocations = o.totalReallocations;

		freeRegionsTree = o.freeRegionsTree;

		if (o.bufSize <= bufCapacity) {
			bufSize = o.bufSize;
		} else {
			if (buf) {
				WalkBuffer(buf, bufSize, HoleNull, DestructChunk);
				free(buf);
			}
			bufSize = o.bufSize;
			bufCapacity = o.bufCapacity;
			buf = (char*)malloc(bufCapacity);
		}

		WalkBuffer(o.buf, bufSize, HoleCopy, ConstructChunk);
		return *this;
	}

	constexpr auto& operator=(PackedHeap&& o)
	{
		totalAllocations = o.totalAllocations;
		totalFrees = o.totalFrees;
		totalResizes = o.totalResizes;
		totalReallocations = o.totalReallocations;

		freeRegionsTree = std::move(o.freeRegionsTree);

		if (buf) {
			WalkBuffer(buf, bufSize, HoleNull, DestructChunk);
			free(buf);
		}

		bufSize = o.bufSize;
		bufCapacity = o.bufCapacity;
		buf = o.buf;
		o.buf = nullptr;

		return *this;
	}

	PackedHeap(size_t sz = 10)
		: PackedAllocator(sz * sizeof(T)) {}

	constexpr PackedHeap(PackedHeap& o)
	{
		*this = o;
	}

	constexpr PackedHeap(PackedHeap&& o)
	{
		*this = o;
	}

	~PackedHeap()
	{
		if (buf) {
			idx_t idx = 0;
			while (idx < bufSize - sizeof(MetaData) && idx < bufSize) {
				MetaData* meta = (MetaData*)&buf[idx];

				if ((unsigned char)meta->used == HOLE_START) {
					while ((unsigned char)buf[idx++] != HOLE_END)
						;
					continue;
				} else if ((unsigned char)meta->used == HOLE_REGION) {
					idx++;
					continue;
				}

				if ((unsigned char)meta->used == USED_REGION)
					Delete(idx + sizeof(MetaData));

				idx += meta->size + sizeof(MetaData) * 2;
			}
		}
	}

	inline idx_t New(size_t sz = 1)
	{

		if (!buf) {
			bufCapacity = sz * sizeof(T) + 2 * sizeof(MetaData);
			buf = (char*)malloc(bufCapacity);
		}

		char* prevBuf = buf;

		idx_t ret = _Alloc(sz * sizeof(T));

		//If the buffer was reallocated, we need to call the move constructors on all members and destruct the members of previous buffer
		if (prevBuf != buf) {
			WalkBuffer(prevBuf, std::min(ret, ret - (idx_t)sizeof(MetaData)), HoleCopy, MoveChunk);
			free(prevBuf);
		}

		if (ret)
			for (size_t i = 0; i < sz; i++)
				new (&buf[ret + i * sizeof(T)]) T();

		return ret;
	}

	inline void Delete(idx_t idx)
	{
		MetaData* meta = (MetaData*)&buf[idx - sizeof(MetaData)];
		size_t cnt = meta->size / sizeof(T);

		for (size_t i = 0; i < cnt; i++)
			((T*)&buf[idx + i * sizeof(T)])->~T();

		Free(idx);
	}
};


#endif
