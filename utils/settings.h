#ifndef SETTINGS_H
#define SETTINGS_H

#include "crc32.h"
#include "shared_utils.h"
#include <vector>
//For shared memory supporting hashmap
#include <boost/unordered_map.hpp>

#define OPTION(type, name, ...) Option<type, CCRC32(#name), __VA_ARGS__> name
#define OPTIONDEF(name) decltype(name) name

template<typename Alloc = std::allocator<unsigned char>>
class SettingsGroupBase
{
  public:

	using pointer = typename Alloc::pointer;
	template<typename T>
	using pointer_t = typename std::pointer_traits<pointer>::template rebind<T>;
	using size_type = unsigned int;

	//Very basic check used to verify the type of config
	static constexpr unsigned char HEADER_MAGIC = 0xde;

	struct MapEntry
	{
		pointer ptr;
		size_type size;

		template<typename T>
		constexpr MapEntry(T p, size_type sz)
			: ptr((pointer)p), size(sz) {}

		constexpr MapEntry()
			: ptr(), size() {}
	};

	SettingsGroupBase()
	{
		reloadCount = 0;
	}

	SettingsGroupBase(const std::vector<unsigned char>& buf, size_t i = 0)
	{
		reloadCount = 0;
		Initialize(buf, i);
		reloadCount = 0;
	}

	inline size_t Initialize(const std::vector<unsigned char>& buf, size_t idx = 0)
	{
		//Possibly throw an exception
		if (buf[idx++] != HEADER_MAGIC)
		    return ~0;

		size_type sz = buf.size();

		for (size_t i = 0; i < sizeof(size_type); i++)
		    ((unsigned char*)&sz)[i] = buf[idx++];

		while (idx < sz) {
			size_type sz = 0;
			crcs_t crc = 0;

			for (size_t o = 0; o < sizeof(sz); o++)
				((unsigned char*)&sz)[o] = buf[idx + o];
			idx += sizeof(sz);

			for (size_t o = 0; o < sizeof(crc); o++)
				((unsigned char*)&crc)[o] = buf[idx + o];
			idx += sizeof(crc);

			pointer a = alloc.allocate(sz);

			for (size_t o = 0; o < sz; o++)
			    a[o] = buf[idx++];

			map[crc] = MapEntry(a, sz);
		}

		reloadCount++;
		return idx;
	}

	inline std::vector<unsigned char> Serialize(std::vector<unsigned char>& ret, unsigned char pushHeader = HEADER_MAGIC)
	{
		ret.push_back(pushHeader);

		size_type endSize = ret.size();

		ret.resize(ret.size() + sizeof(size_type));

		for (auto& i : map) {
			size_type sz = i.second.size;
			crcs_t crc = i.first;

			for (size_t o = 0; o < sizeof(sz); o++)
				ret.push_back(((unsigned char*)&sz)[o]);

			for (size_t o = 0; o < sizeof(crc); o++)
				ret.push_back(((unsigned char*)&crc)[o]);

			for (size_t o = 0; o < sz; o++)
				ret.push_back(i.second.ptr[o]);
		}

		size_type sz = ret.size();

		for (size_t i = 0; i < sizeof(size_type); i++)
			ret[endSize + i] = ((unsigned char*)&sz)[i];

		return ret;
	}

	inline size_t ReloadCount()
	{
		return reloadCount;
	}

	template<typename T>
	inline pointer_t<T> RegisterOption(crcs_t crc, const T& val)
	{
		pointer_t<T> idx = ReserveOption<T>(crc, T());
		*(T*)&*idx = val;
		return idx;
	}

	template<typename T>
	inline pointer_t<T> ReserveOption(crcs_t crc, const T& val)
	{
		if (map.find(crc) == map.end()) {
			pointer_t<T> idx = (pointer_t<T>)alloc.allocate(sizeof(T));
			*idx = val;
			map[crc] = MapEntry(idx, sizeof(T));
		}
		return (pointer_t<T>)map[crc].ptr;
	}

	inline pointer TryGetAlloc(crcs_t crc)
	{
		if (map.find(crc) == map.end())
			return 0;
		return map[crc].ptr;
	}

	//IsBlocked can be overloaded to add in custom settings chain functionality
	template<typename T>
	inline bool IsBlocked(T ptr)
	{
		return false;
	}

	template<crcs_t CRC, typename T>
	inline auto& Set(const T& val)
	{
		pointer_t<T> idx = RegisterOption(CRC, val);
		*idx = val;
		return *this;
	}

	template<typename T>
	inline auto& SetRuntime(const T& val, crcs_t CRC)
	{
		pointer_t<T> idx = RegisterOption(CRC, val);
		*idx = val;
		return *this;
	}

	template<typename T, crcs_t CRC>
	inline T Get()
	{
		pointer_t<T> idx = ReserveOption(CRC, T());
		return *idx;
	}

	template<typename T>
	inline T GetRuntime(crcs_t CRC)
	{
		pointer_t<T> idx = ReserveOption(CRC, T());
		return *idx;
	}

	constexpr auto operator->()
	{
		return this;
	}

  protected:
	using MapAlloc = typename Alloc::template rebind<std::pair<const crcs_t, MapEntry>>::other;
	boost::unordered_map<crcs_t, MapEntry, boost::hash<crcs_t>, std::equal_to<crcs_t>, MapAlloc> map;
	Alloc alloc;
	size_t reloadCount;
};

using SettingsGroup = SettingsGroupBase<std::allocator<unsigned char>>;

template<typename T, crcs_t CRC, auto& G>
struct OptionDataRef
{

	using parent_type_t = decltype(*(G.operator->()));
	using parent_type = typename std::decay<parent_type_t>::type;
	using pointer = typename parent_type::template pointer_t<T>;

	pointer allocID;
	size_t reloadCnt;

	constexpr OptionDataRef(const T& v)
		: allocID(G->ReserveOption(CRC, v)), reloadCnt(G->ReloadCount()) {}

	constexpr OptionDataRef()
		: allocID() {}

	constexpr void Init(const T& v)
	{
		allocID = G->ReserveOption(CRC, v);
		reloadCnt = G->ReloadCount();
	}

	constexpr void CheckReloadCnt()
	{
		size_t gCnt = G->ReloadCount();
		if (reloadCnt != gCnt)
			allocID = (pointer)0;
		reloadCnt = gCnt;
	}

	constexpr void Refresh()
	{
		CheckReloadCnt();
		if (!allocID)
			allocID = G->ReserveOption(CRC, T());
	}

	constexpr bool TryRefresh()
	{
		CheckReloadCnt();
		return allocID && !G->IsBlocked(allocID);
	}
};

template<typename T, crcs_t CRC, auto& G>
struct OptionDataPtr
{

	using parent_type = typename std::decay<decltype(*G)>::type;
	using pointer = typename parent_type::template pointer_t<T>;


	typename std::remove_reference<decltype(G)>::type g;
	pointer allocID;
	size_t reloadCnt;

	constexpr OptionDataPtr(const T& v)
		: g(G),
		allocID(G->ReserveOption(CRC, v)), reloadCnt(G->ReloadCount()) {}

	constexpr OptionDataPtr()
		: g(G),
		allocID(), reloadCnt(0) {}

	constexpr void Init(const T& v)
	{
		g = G;
		allocID = G->ReserverOption(CRC, v);
		reloadCnt = G->ReloadCount();
	}

	constexpr void CheckReloadCnt()
	{
		size_t gCnt = G->ReloadCount();
		if (reloadCnt != gCnt)
			allocID = (pointer)0;
		reloadCnt = gCnt;
	}

	constexpr void Refresh()
	{
		if (g != G) {
			g = G;
			allocID = g->ReserveOption(CRC, T());
			reloadCnt = G->ReloadCount();
		} else
			CheckReloadCnt();

		if (!allocID)
			allocID = g->ReserveOption(CRC, T());
	}

	constexpr bool TryRefresh()
	{
		if (g != G) {
			g = G;
			allocID = (pointer)g->TryGetAlloc(CRC);
			reloadCnt = G->ReloadCount();
		} else
			CheckReloadCnt();

		return allocID && !G->IsBlocked(allocID);
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
		return *BaseType::allocID;
	}

	constexpr void Init(const T& v)
	{
		BaseType::Init(v);
	}

	constexpr void Set(const T& val)
	{
		BaseType::Refresh();
		*BaseType::allocID = val;
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
			return *BaseType::allocID;
		return next.Get();
	}

	constexpr void Init(const T& v)
	{
		BaseType::Init(v);
		next.Init(v);
	}

	constexpr void Set(const T& val)
	{
		BaseType::Refresh();

		if (!BaseType::allocID) {
			next.Set(val);
			return;
		}

		*BaseType::allocID = val;
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

	constexpr void Init(const T& v)
	{
		Container::Init(v);
	}

	constexpr operator T()
	{
		return Container::Get();
	}

	static constexpr T Get(Option& in)
	{
		return in.operator T();
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
