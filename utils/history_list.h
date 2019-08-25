#ifndef HISTORY_LIST_H
#define HISTORY_LIST_H

#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <math.h>

template <typename T, size_t C>
struct HistoryList
{

	//id 0 is the current item, and higher up go previous items
	inline T& GetLastItem(size_t id)
	{
		return list[(counter - id) % C];
	}

	inline T& operator[](size_t id)
	{
		return GetLastItem(id);
	}

	HistoryList()
	{
		counter = 0;
	}

	~HistoryList()
	{
		for (size_t i = 0; i < Count(); i++)
			operator[](i).~T();
	}

	auto& Push()
	{
		counter++;

		if (counter >= C)
			list[counter % C].~T();

		new(&list[counter % C]) T();
		return list[counter % C];
	}

	void UndoPush()
	{
		counter--;
	}

	auto& Push(const T& item)
	{
		list[++counter % C] = item;
		return list[counter % C];
	}

	auto& GetItem(size_t id)
	{
		return list[id % C];
	}

	size_t Count()
	{
		return std::min(counter, C);
	}

	void Reset()
	{
		for (size_t i = 0; i < Count(); i++)
			operator[](i).~T();

		counter = 0;
	}

  private:
	T list[C];
	size_t counter;
};

#endif
