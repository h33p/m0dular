#ifndef SETTINGS_H
#define SETTINGS_H

#include "crc32.h"
#include "packed_heap.h"
#include "shared_utils.h"
#include <unordered_map>
#include <vector>

#define OPTION(type, name, ...) Option<type, CCRC32(#name), __VA_ARGS__> name
#define OPTIONDEF(name) decltype(name) name

template<typename Alloc>
class SettingsGroupBase
{
  public:

    SettingsGroupBase() {}

	SettingsGroupBase(const std::vector<unsigned char>& buf)
	{
		size_t i = 0;
		while (i < buf.size()) {
			idx_t sz = 0;
			crcs_t crc = 0;

			for (size_t o = 0; o < sizeof(sz); o++)
				((unsigned char*)&sz)[o] = buf[i + o];
			i += sizeof(sz);

			for (size_t o = 0; o < sizeof(crc); o++)
				((unsigned char*)&crc)[o] = buf[i + o];
			i += sizeof(crc);

			idx_t a = alloc.Alloc(sz);

			for (size_t o = 0; o < sz; o++)
				alloc[a + o] = buf[i++];

			map[crc] = a;
		}

		map.rehash(0);
	}

	inline std::vector<unsigned char> Serialize()
	{
		std::vector<unsigned char> ret;
		for (auto& i : map) {
			idx_t sz = *(idx_t*)(alloc + i.second - sizeof(idx_t));
			crcs_t crc = i.first;

			for (size_t o = 0; o < sizeof(sz); o++)
				ret.push_back(((unsigned char*)&sz)[o]);

			for (size_t o = 0; o < sizeof(crc); o++)
				ret.push_back(((unsigned char*)&crc)[o]);

			for (size_t o = 0; o < sz; o++)
				ret.push_back(alloc[i.second + o]);
		}
		return ret;
	}

	template<typename T>
	inline idx_t RegisterOption(crcs_t crc, const T& val)
	{
		idx_t idx = ReserveOption<T>(crc, T());
		*(T*)(alloc + idx) = val;
		return idx;
	}

	template<typename T>
	inline idx_t ReserveOption(crcs_t crc, const T& val)
	{
		if (map.find(crc) == map.end()) {
			idx_t idx = alloc.Alloc(sizeof(T));
			*(T*)(alloc + idx) = val;
			map[crc] = idx;
		}
		return map[crc];
	}

	inline idx_t TryGetAlloc(crcs_t crc)
	{
		if (map.find(crc) == map.end())
			return 0;
		return map[crc];
	}

	template<typename T>
	constexpr T* RetreivePtrFast(idx_t idx)
	{
		return (T*)(alloc + idx);
	}

	template<crcs_t CRC, typename T>
	inline auto& Set(const T& val)
	{
		idx_t idx = RegisterOption(CRC, val);
		*RetreivePtrFast<T>(idx) = val;
		return *this;
	}

	template<typename T, crcs_t CRC>
	inline T Get()
	{
		idx_t idx = ReserveOption(CRC, T());
		return *RetreivePtrFast<T>(idx);
	}

  protected:
	std::unordered_map<crcs_t, idx_t> map;
	Alloc alloc;
};

using SettingsGroup = SettingsGroupBase<PackedAllocator>;

template<typename T, crcs_t CRC, auto& G>
struct OptionDataRef
{
	int allocID;
	T* val;

	constexpr OptionDataRef(const T& v)
		: allocID(G.RegisterOption(CRC, v)),
		val(G.template RetreivePtrFast<T>(allocID)) {}

	constexpr OptionDataRef()
		: allocID(0),
		val(nullptr) {}

	constexpr void Refresh()
	{
		if (!allocID)
			allocID = G.ReserveOption(CRC, T());

		val = G.template RetreivePtrFast<T>(allocID);
	}

	constexpr bool TryRefresh()
	{
		if (allocID)
			val = G.template RetreivePtrFast<T>(allocID);
		else
			val = nullptr;

		return !!val;
	}
};

template<typename T, crcs_t CRC, auto& G>
struct OptionDataPtr
{
	typename std::remove_reference<decltype(G)>::type g;
	int allocID;
	T* val;

	constexpr OptionDataPtr(const T& v)
		: g(G),
		allocID(G->RegisterOption(CRC, v)),
		val(G->template RetreivePtrFast<T>(allocID)) {}

	constexpr OptionDataPtr()
		: g(G),
		allocID(0),
		val(nullptr) {}

	constexpr void Refresh()
	{
		if (g != G) {
			g = G;
			allocID = g->ReserveOption(CRC, T());
		}

		if (!allocID)
			allocID = g->ReserveOption(CRC, T());

		val = g->template RetreivePtrFast<T>(allocID);
	}

	constexpr bool TryRefresh()
	{
		if (g != G) {
			g = G;
			allocID = g->TryGetAlloc(CRC);
		}

		if (allocID)
			val = g->template RetreivePtrFast<T>(allocID);
		else
			val = nullptr;

		return !!val;
	}
};

template<typename T, crcs_t CRC, auto&... Args>
struct SettingsChain;

template<typename T, crcs_t CRC, auto& G>
struct SettingsChain<T, CRC, G>
	: std::conditional<IsPointer(G), OptionDataPtr<T, CRC, G>, OptionDataRef<T, CRC, G>>::type
{
	typedef typename std::conditional<IsPointer(G), OptionDataPtr<T, CRC, G>, OptionDataRef<T, CRC, G>>::type BaseType;

	SettingsChain(const T& v)
		: BaseType(v) {}

	SettingsChain()
		: BaseType() {}

	constexpr T Get()
	{
		BaseType::Refresh();
		return *BaseType::val;
	}

	constexpr void Set(const T& val)
	{
		BaseType::Refresh();
		*BaseType::val = val;
	}
};

template<typename T, crcs_t CRC, auto& G, auto&... Args>
struct SettingsChain<T, CRC, G, Args...>
	: std::conditional<IsPointer(G), OptionDataPtr<T, CRC, G>, OptionDataRef<T, CRC, G>>::type
{
	typedef typename std::conditional<IsPointer(G), OptionDataPtr<T, CRC, G>, OptionDataRef<T, CRC, G>>::type BaseType;

	SettingsChain(const T& v)
		: BaseType(v), next(v) {}

	SettingsChain()
		: BaseType(), next() {}

	constexpr T Get()
	{
		if (BaseType::TryRefresh())
			return *BaseType::val;
		return next.Get();
	}

	constexpr void Set(const T& val)
	{
		BaseType::Refresh();

		if (!BaseType::val) {
			next.Set(val);
			return;
		}

		*BaseType::val = val;
	}

  private:
	SettingsChain<T, CRC, Args...> next;
};

template<typename T, crcs_t CRC, auto&... Chain>
struct Option : public SettingsChain<T, CRC, Chain...>
{
	typedef SettingsChain<T, CRC, Chain...> Container;

	constexpr Option(const T& v)
		: Container(v) {}

	constexpr Option()
		: Container() {}

	constexpr operator T()
	{
		return Container::Get();
	}

	template<typename F>
	constexpr bool operator == (const F& o)
	{
		return Container::Get() == o;
	}

	template<typename F>
	constexpr bool operator != (const F& o)
	{
		return Container::Get() != o;
	}

	template<typename F>
	constexpr bool operator > (const F& o)
	{
		return Container::Get() > o;
	}

	template<typename F>
	constexpr bool operator < (const F& o)
	{
		return Container::Get() < o;
	}

	template<typename F>
	constexpr bool operator >= (const F& o)
	{
		return Container::Get() >= o;
	}

	template<typename F>
	constexpr bool operator <= (const F& o)
	{
		return Container::Get() <= o;
	}

	template<typename F>
	constexpr T operator + (const F& o)
	{
		return Container::Get() + o;
	}

	template<typename F>
	constexpr T operator - (const F& o)
	{
		return Container::Get() - o;
	}

	template<typename F>
	constexpr T operator * (const F& o)
	{
		return Container::Get() * o;
	}

	template<typename F>
	constexpr T operator / (const F& o)
	{
		return Container::Get() / o;
	}

	template<typename F>
	constexpr auto& operator += (const F& o)
	{
		Container::Set(Container::Get() + o);
		return *this;
	}

	template<typename F>
	constexpr auto& operator -= (const F& o)
	{
		Container::Set(Container::Get() - o);
		return *this;
	}

	template<typename F>
	constexpr auto& operator *= (const F& o)
	{
		Container::Set(Container::Get() * o);
		return *this;
	}

	template<typename F>
	constexpr auto& operator /= (const F& o)
	{
		Container::Set(Container::Get() / o);
		return *this;
	}

	template<typename F>
	constexpr auto& operator = (const F& o)
	{
		Container::Set(o);
		return *this;
	}

};

#endif
