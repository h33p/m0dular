#ifndef SHARED_UTILS_H
#define SHARED_UTILS_H

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

#endif
