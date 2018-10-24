#ifndef SHARED_UTILS_H
#define SHARED_UTILS_H

//We can force a constexpr to really be a compile-time expression
template <typename T, T K>
struct calc_constexpr
{
	static constexpr T value = K;
};

#endif
