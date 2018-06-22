#ifndef MMATH_H
#define MMATH_H

#include "../wincludes.h"
#include <stddef.h>
#include <type_traits>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>

#if defined(_MSC_VER)
#define __ALIGNED(x) __declspec(align(x))
#else
#define __ALIGNED(x) __attribute__((__aligned__(x)))
#endif

#include <xmmintrin.h>
#include <immintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>

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

constexpr int NumOfSIMD(const int val)
{
	return (val - 1) / SIMD_COUNT + 1;
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

constexpr float RAD2DEG = 180.0 / M_PI;
constexpr float DEG2RAD = M_PI / 180.0;

#include "vector.h"

#endif
