#ifndef PACKED_HEAP_H
#define PACKED_HEAP_H

//At the current moment region merge causes higher overall average memory difference, this is to be investigated
#ifndef PACKED_HEAP_MERGE_REGIONS
#define PACKED_HEAP_MERGE_REGIONS 0
#endif

#include <vector>
#include <map>
#include <set>

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

template<typename T>
class PackedHeap
{
  private:

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

	std::vector<char> buf;
	std::map<idx_t, std::set<idx_t>> freeRegionsTree;

	bool FillHole(idx_t holeStart, idx_t size)
	{
		if (size) {
			if (size == 1) {
				*(unsigned char*)&buf[holeStart] = HOLE_REGION;
				return true;
			} else if (size <= 2 * sizeof(MetaData) + sizeof(T)) {
				*(unsigned char*)&buf[holeStart] = HOLE_START;
				for (idx_t i = 2; i < size; i++)
					*(unsigned char*)&buf[holeStart + i - 1] = 0xff;
				*(unsigned char*)&buf[holeStart + size - 1] = HOLE_END;
				return true;
			}
		}

		return false;
	}

  public:

	idx_t totalAllocations = 0;
	idx_t totalFrees = 0;
	idx_t totalResizes = 0;
	idx_t totalReallocations = 0;

	PackedHeap(size_t sz = 10)
	{
		buf.reserve(sz * sizeof(T));
	}

	idx_t Alloc(idx_t sz)
	{
		sz *= sizeof(T);

		sz = sz + (sz % 4 ? 4 - sz % 4 : 0);

		totalAllocations++;

		size_t allocSize = sz + sizeof(MetaData) * 2;
		if (!freeRegionsTree.empty()) {
			auto reg = freeRegionsTree.lower_bound(sz);
			if (reg != freeRegionsTree.end()) {
				idx_t address = *reg->second.rbegin();
				idx_t ret = address + sizeof(MetaData);
				reg->second.erase(address);
				idx_t delta = ((MetaData*)&buf[address])->size - sz;

#ifdef PACKED_HEAP_DEBUG
				if (reg->first != ((MetaData*)&buf[address])->size)
					throw std::runtime_error("PackedHeap corruption");
#endif

				*(MetaData*)&buf[ret - sizeof(MetaData)] = {USED_REGION, sz};
				*(MetaData*)&buf[ret + sz] = {USED_REGION, sz};

				if (!reg->second.size())
					freeRegionsTree.erase(reg);

				idx_t holeStart = ret + allocSize - sizeof(MetaData);

				//Check if the place is small enough for a unallocatable hole
				if (delta && !FillHole(holeStart, delta)) {
					idx_t holeSpotSize = delta - sizeof(MetaData) * 2;
					*(MetaData*)&buf[holeStart] = {FREE_REGION, holeSpotSize};
					*(MetaData*)&buf[holeStart + holeSpotSize + sizeof(MetaData)] = {FREE_REGION, holeSpotSize};
				    freeRegionsTree[holeSpotSize].insert(holeStart);
				}

				return ret;
			}
		}

		totalResizes++;

		idx_t holeStart = buf.size();
		idx_t holeEnd = holeStart + (holeStart % 4 ? 4 - holeStart % 4 : 0);

		if (holeEnd - holeStart)
			FillHole(holeStart, holeEnd - holeStart);

		idx_t baseIdx = holeEnd + sizeof(MetaData);

		if (buf.capacity() < baseIdx + sizeof(MetaData) + sz)
			totalReallocations++;

		buf.resize(baseIdx + sizeof(MetaData) + sz);
		*(MetaData*)&buf[baseIdx - sizeof(MetaData)] = {USED_REGION, sz};
		*(MetaData*)&buf[baseIdx + sz] = {USED_REGION, sz};

		return baseIdx;
	}

	idx_t Alloc()
	{
		return Alloc(1);
	}

	void Free(idx_t idx)
	{
		if (!idx)
			return;

		totalFrees++;

		MetaData* metaData = (MetaData*)&buf[idx - sizeof(MetaData)];

		if (metaData->used != USED_REGION) {
			if (metaData->used == FREE_REGION)
#ifdef PACKED_HEAP_DEBUG
				throw std::runtime_error("Double free");
#else
				return;
#endif
			else
#if PACKED_HEAP_DEBUG
				throw std::runtime_error("PackedHeap corruption");
#else
				return;
#endif
		}

		idx_t start = idx - sizeof(MetaData);
		idx_t end = idx + sizeof(MetaData) + metaData->size;

		if (*metaData != *(MetaData*)&buf[end - sizeof(MetaData)])
#if PACKED_HEAP_DEBUG
			throw std::runtime_error("PackedHeap corruption");
#else
			return;
#endif

		//Check for a memory hole above the region (this can never occur below)
		if ((unsigned char)buf[end] == HOLE_START)
			while ((unsigned char)buf[end++] != HOLE_END)
				;
		else if ((unsigned char)buf[end] == HOLE_REGION)
			end++;

		MetaData* upperMetaData = (MetaData*)&buf[end - sizeof(MetaData)];

		MetaData* aboveRegion = end + sizeof(MetaData) < buf.size() ? (MetaData*)&buf[end] : nullptr;
		MetaData* belowRegion = start >= sizeof(MetaData) * 2 ? (MetaData*)&buf[start - sizeof(MetaData)] : nullptr;

		//Join the nearby free regions
		if (PACKED_HEAP_MERGE_REGIONS && aboveRegion && aboveRegion->used == FREE_REGION) {
			[[maybe_unused]]
			size_t ret = freeRegionsTree[aboveRegion->size].erase(end);

#ifdef PACKED_HEAP_DEBUG
			if (!ret)
				throw std::runtime_error("PackedHeap corruption");

			if (!freeRegionsTree[aboveRegion->size].size())
				freeRegionsTree.erase(aboveRegion->size);
#endif

			upperMetaData = aboveRegion->WalkUp();
		}

		if (PACKED_HEAP_MERGE_REGIONS && belowRegion && belowRegion->used == FREE_REGION) {
			[[maybe_unused]]
			size_t ret = freeRegionsTree[belowRegion->size].erase(start - 2 * sizeof(MetaData) - belowRegion->size);

#ifdef PACKED_HEAP_DEBUG
			if (!ret)
				throw std::runtime_error("PackedHeap corruption");

			if (!freeRegionsTree[belowRegion->size].size())
				freeRegionsTree.erase(belowRegion->size);
#endif

			metaData = belowRegion->WalkDown();
		}

		metaData->used = FREE_REGION;
		metaData->size = (uintptr_t)upperMetaData - (uintptr_t)metaData - sizeof(MetaData);
		*upperMetaData = *metaData;

		freeRegionsTree[metaData->size].insert((idx_t)((uintptr_t)metaData - (uintptr_t)&buf[0]));
	}

	void Free(T* ptr)
	{
		idx_t idx = (char*)ptr - &buf[0];
		if (idx < buf.size())
			Free(idx);
	}

	inline T& operator[](idx_t idx)
	{
		return *(T*)&buf[idx];
	}

	inline T* operator+(idx_t idx)
	{
		return (T*)&buf[idx];
	}
};

#endif
