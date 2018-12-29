#ifndef SHARED_UTILS_H
#define SHARED_UTILS_H

#include <type_traits>

//We can force a constexpr to really be a compile-time expression
template <typename T, T K>
struct calc_constexpr
{
	static constexpr T value = K;
};

template<typename T>
struct is_pointer {
	static const bool value = false;
};

template<typename T>
struct is_pointer<T*> {
	static const bool value = true;
};

template<typename T>
constexpr auto& RemovePtr(T& arg)
{
	if constexpr(is_pointer<T>::value)
		return *arg;
	else
		return arg;
}

template<typename T>
constexpr bool IsPointer(T& arg)
{
	return is_pointer<T>::value;
}

template<auto& G>
class pointer_proxy
{
  public:
	constexpr auto& operator->()
	{
		if constexpr(IsPointer(G))
			return G;
		else
			return &G;
	}

	constexpr auto& operator*()
	{
		if constexpr(IsPointer(G))
			return *G;
		else
			return G;
	}
};

template<typename first, typename...more>
	struct AllArithmetic {
		static const bool value = std::is_arithmetic<first>::value &&
			AllArithmetic<more...>::value;
	};

template<typename first>
struct AllArithmetic<first> : std::is_arithmetic<first> {};

template<typename T>
constexpr T* AlignUp(T* ptr, std::size_t align = std::alignment_of<T>::value)
{
    return (T*)(((std::size_t)ptr + align - 1) & ~(align - 1));
}
#endif
