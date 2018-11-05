#include "packed_heap.h"


PackedAllocator::PackedAllocator(size_t sz)
{
	buf = (char*)malloc(sz);
	bufCapacity = sz;
	bufSize = 0;
}

PackedAllocator::PackedAllocator(const PackedAllocator& o)
{
	*this = o;
}

PackedAllocator::PackedAllocator(const PackedAllocator&& o)
{
	*this = o;
}

PackedAllocator::~PackedAllocator()
{
	if (buf)
		free(buf);
	buf = nullptr;
	bufCapacity = 0;
	bufSize = 0;
}

PackedAllocator& PackedAllocator::operator=(const PackedAllocator& o)
{
	totalAllocations = o.totalAllocations;
	totalFrees = o.totalFrees;
	totalResizes = o.totalResizes;
	totalReallocations = o.totalReallocations;

	freeRegionsTree = o.freeRegionsTree;

	if (o.bufSize <= bufCapacity) {
		bufSize = o.bufSize;
	} else {
		if (buf)
			free(buf);
		bufSize = o.bufSize;
		bufCapacity = o.bufCapacity;
		buf = (char*)malloc(bufCapacity);
	}
	memcpy(buf, o.buf, bufSize);

	return *this;
}

PackedAllocator& PackedAllocator::operator=(PackedAllocator&& o)
{
	totalAllocations = o.totalAllocations;
	totalFrees = o.totalFrees;
	totalResizes = o.totalResizes;
	totalReallocations = o.totalReallocations;

	freeRegionsTree = std::move(o.freeRegionsTree);

	if (buf)
		free(buf);

	buf = o.buf;
	o.buf = nullptr;
	bufSize = o.bufSize;
	bufCapacity = o.bufCapacity;

	return *this;
}

idx_t PackedAllocator::_Alloc(idx_t sz)
{
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

	idx_t holeStart = bufSize;
	idx_t holeEnd = holeStart + (holeStart % 4 ? 4 - holeStart % 4 : 0);

	if (holeEnd - holeStart)
		FillHole(holeStart, holeEnd - holeStart);

	idx_t baseIdx = holeEnd + sizeof(MetaData);

	if (bufCapacity < baseIdx + sizeof(MetaData) + sz) {
		totalReallocations++;
		bufCapacity *= 2;
		buf = (char*)malloc(bufCapacity);
	}

	bufSize = baseIdx + sizeof(MetaData) + sz;
	*(MetaData*)&buf[baseIdx - sizeof(MetaData)] = {USED_REGION, sz};
	*(MetaData*)&buf[baseIdx + sz] = {USED_REGION, sz};

	return baseIdx;
}

idx_t PackedAllocator::Alloc(idx_t sz)
{
	if (!buf) {
		bufCapacity = sz + 2 * sizeof(MetaData);
		buf = (char*)malloc(bufCapacity);
	}

	char* prevBuf = buf;

	idx_t ret = _Alloc(sz);

	if (buf != prevBuf) {
		memcpy(buf, prevBuf, ret - sizeof(MetaData));
		free(prevBuf);
	}

	return ret;
}

void PackedAllocator::Free(idx_t idx)
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

	MetaData* aboveRegion = end + sizeof(MetaData) < bufSize ? (MetaData*)&buf[end] : nullptr;
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
