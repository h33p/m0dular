#ifndef PACKED_HEAP_H
#define PACKED_HEAP_H

#include <vector>

using idx_t = unsigned int;

template<typename T>
class PackedHeap
{
  private:
	std::vector<T> buf;
	std::vector<idx_t> free_indexes;

  public:
	idx_t Alloc()
	{
		if (free_indexes.size()) {
		    idx_t ret = *free_indexes.rbegin();
			free_indexes.pop_back();
			return ret;
		}
		buf.resize(buf.size() + 1);
		return buf.size();
	}

	void Free(idx_t idx)
	{
		free_indexes.push_back(idx);
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

#endif
