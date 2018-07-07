#ifndef MMATH_H
#define MMATH_H

#include "../wincludes.h"
#include <stddef.h>
#include <type_traits>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>

#ifndef _MSC_VER
#define __ALIGNED(x) __attribute__((__aligned__(x)))
#else
#define __ALIGNED(x) __declspec(align(x))
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
#elif defined(__AVX__) || defined(__AVX2__)
#define PSIMD 8
const int SIMD_COUNT = 8;
#elif defined(__SSE__) || defined(__SSE2__) || defined(__SSE2_MATH__) || defined(_M_IX86_FP) || (defined(_M_AMD64) || defined(_M_X64))
#define PSIMD 4
const int SIMD_COUNT = 4;
#else
const int SIMD_COUNT = 1;
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

template<typename T>
inline T TMod(T val, T lim)
{
	return val % lim;
}

template<>
inline float TMod<float>(float val, float lim)
{
	return fmodf(val, lim);
}

inline float NormalizeFloat(float result, float start, float end)
{
	result = fmodf(result - start, end - start);

	if (result < start)
		result += end - start;

	return result + start;
}

constexpr float RAD2DEG = 180.0 / M_PI;
constexpr float DEG2RAD = M_PI / 180.0;

#include "vector.h"

#endif
