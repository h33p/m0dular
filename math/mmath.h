#ifndef MMATH_H
#define MMATH_H

#include "../utils/shared_utils.h"
#include "../wincludes.h"
#include <stddef.h>
#include <type_traits>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>
#include <algorithm>

#include "mm_funcs.h"

#ifndef _MSC_VER
#include <nmmintrin.h>
#endif

#if defined(__clang__) && defined(_MSC_VER)
#pragma push_macro("_MM_HINT_T0")
#undef _MM_HINT_T0
#pragma push_macro("_MM_HINT_T1")
#undef _MM_HINT_T1
#pragma push_macro("_MM_HINT_T2")
#undef _MM_HINT_T2
#endif
#include <xmmintrin.h>
#include <immintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#if defined(__clang__) && defined(_MSC_VER)
#pragma pop_macro("_MM_HINT_T0")
#pragma pop_macro("_MM_HINT_T1")
#pragma pop_macro("_MM_HINT_T2")
#endif

#if defined(OVERRIDE)
const int SIMD_COUNT = OVERRIDE;
#elif defined(__AVX512F__) || defined(__AVX512CD__) || defined(__AVX512ER__)
#define PSIMD 16
const int SIMD_COUNT = 16;
typedef short simdFlags;
DEFINE_MM(128);
DEFINE_MM(256);
DEFINE_MM(512);
#elif defined(__AVX__) || defined(__AVX2__)
#define PSIMD 8
const int SIMD_COUNT = 8;
typedef char simdFlags;
DEFINE_MM(128);
DEFINE_MM(256);
#elif defined(__SSE__) || defined(__SSE2__) || defined(__SSE2_MATH__) || defined(_M_IX86_FP) || (defined(_M_AMD64) || defined(_M_X64))
#define PSIMD 4
const int SIMD_COUNT = 4;
typedef char simdFlags;
DEFINE_MM(128);
#else
const int SIMD_COUNT = 1;
typedef char simdFlags;
#endif

template<typename W, size_t Q>
struct max_sse
{
	static const bool value = (Q * sizeof(W) == 16 && SIMD_COUNT >= 4);
};

template<typename W, size_t Q>
struct max_avx
{
	static const bool value = (Q * sizeof(W) == 32 && SIMD_COUNT >= 8);
};

template<typename W, size_t Q>
struct max_avx512
{
	static const bool value = (Q * sizeof(W) == 64 && SIMD_COUNT >= 16);
};

template<typename W, size_t Q>
struct do_avx512
{
	static const bool value = SIMD_COUNT >= 16 && !((Q * 8 * sizeof(W)) % (8 * 4 * 16));
};

template<typename W, size_t Q>
struct do_avx
{
	static const bool value = !do_avx512<W, Q>::value && SIMD_COUNT >= 8 && !((Q * 8 * sizeof(W)) % (8 * 4 * 8));
};

template<typename W, size_t Q>
struct do_sse
{
	static const bool value = !do_avx<W, Q>::value && !do_avx512<W, Q>::value && SIMD_COUNT >= 4 && !((Q * 8 * sizeof(W)) % (8 * 4 * 4));
};

template<typename W, size_t Q>
struct do_simd
{
	static const bool value = do_sse<W, Q>::value || do_avx<W, Q>::value || do_avx512<W, Q>::value;
};

template<size_t A, size_t B>
struct comp_if
{
	static const bool value = (A == B);
};

template<size_t N>
constexpr int NumOf(const int val)
{
	return (val - 1) / N + 1;
}

constexpr int NumOfSIMD(const int val)
{
	return NumOf<SIMD_COUNT>(val);
}

#include <type_traits>

template<typename T>
constexpr T PopCnt(T inp)
{
#ifdef __GNUC__
	if (sizeof(inp) == 8)
		return __builtin_popcountll(inp);
	return __builtin_popcount(inp);
#else
	T i = 0;
	for (i = 0; i < sizeof(inp) * 8; i++)
		if (~inp & (1 << i))
			break;
	return i;
#endif
}

constexpr size_t Clz(size_t inp)
{
#ifdef __GNUC__
	if (sizeof(inp) == 8)
		return __builtin_clzll(inp);
	return __builtin_clz(inp);
#else
	inp |= inp >> 1;
    inp |= inp >> 2;
    inp |= inp >> 4;
    inp |= inp >> 8;
    inp |= inp >> 16;
	if (sizeof(inp) == 8)
		inp |= inp >> 32;
    return sizeof(inp) * 8 - PopCnt(inp);
#endif
}

constexpr size_t AlignUp(size_t inp)
{
	if (inp <= 1)
		return 1;
	return size_t(1) << (sizeof(size_t) * 8 - Clz(inp));
}


template<typename T, typename std::enable_if<std::is_floating_point<T>::value>::type* p = nullptr>
constexpr T Modulo(const T x, const T y)
{
    return (x < T() ? T(-1) : T(1)) * (
		(x < T() ? -x : x) -
	    (long long)((x / y < T() ? -x / y : x / y)) * (y < T() ? -y : y));
}

// For non-floating point types

template<typename T>
using TypeToCast = typename std::conditional<std::is_floating_point<T>::value, int, T>::type;

template<typename T, typename std::enable_if<!std::is_floating_point<T>::value>::type* p = nullptr>
constexpr T Modulo(const T x, const T y)
{
    return (TypeToCast<T>)(x) % (TypeToCast<T>)(y);
}

template<typename T>
[[deprecated("Duplicate function")]]
inline T TMod(T val, T lim)
{
	return std::remainder(val, lim);
}

constexpr float NormalizeFloat(float result, float start, float end)
{
	result = Modulo(result - start, end - start);

	if (result < 0.f)
		result += end - start;

	return result + start;
}

template<typename T>
constexpr T NormalizeInRange(T result, T start, T end)
{
	result = Modulo(result - start, end - start);

	if (result < 0)
		result += end - start;

	return result + start;
}

//This should never be called in the first place, but it is required for the compile to take place
template<typename T>
constexpr T GetElementAt(size_t id)
{
	return T();
}

template<typename F, typename... T>
constexpr F GetElementAt(size_t id, F arg, T... args)
{
	constexpr size_t sz = sizeof...(args);
	return (id && sz) ? GetElementAt<F>(id - 1, args...) : arg;
}

template<typename T>
constexpr T Max(T a, T b)
{
	return a > b ? a : b;
}

template<typename T>
constexpr T Min(T a, T b)
{
	return a < b ? a : b;
}

template<typename T>
constexpr T Abs(T val)
{
	return val < 0 ? -val : val;
}

template <typename T>
constexpr T TrigSeries(T val, T sum, T n, int i, int s, T exp)
{
	return Abs(exp * s / n) > std::numeric_limits<T>::epsilon() ? TrigSeries(val, sum + exp * s / n, n * i * (i + 1), i + 2, -s, exp * val * val) : sum;
}

template<typename T>
constexpr T ConstSin(T val)
{
	val = NormalizeInRange(val, T(-M_PI), T(M_PI));
	return TrigSeries(val, val, T(6), 4, -1, val * val * val);
}

template<typename T>
constexpr T ConstCos(T val)
{
	return ConstSin(val + M_PI / 2);
}

constexpr float RAD2DEG = 180.0 / M_PI;
constexpr float DEG2RAD = M_PI / 180.0;

#include "vector.h"

#endif
