#ifndef HISTORY_LIST_H
#define HISTORY_LIST_H

template <typename T, size_t C>
struct HistoryList
{
	HistoryList()
	{
		counter = 0;
	}

	auto& Push()
	{
		counter++;
		return list[counter % C];
	}

	void UndoPush()
	{
		counter--;
	}

	auto& Push(T& item)
	{
		list[++counter % C] = item;
		return list[counter % C];
	}

	auto& GetItem(size_t id)
	{
		return list[id % C];
	}

	//id 0 is the current item, and higher up go previous items
	auto& GetLastItem(size_t id)
	{
		return list[(counter - id) % C];
	}

	size_t Count()
	{
		return std::min(counter, C);
	}

	void Reset()
	{
		counter = 0;
	}

  private:
	T list[C];
	size_t counter;
};

#endif
