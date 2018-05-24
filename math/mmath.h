#ifndef MMATH_H
#define MMATH_H

#include "stddef.h"

#if defined(_WIN32) && defined(__clang__)
#pragma push_macro("_MSC_VER")
#undef _MSC_VER
#endif

#include <xmmintrin.h>
#include <immintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>

#if defined(_WIN32) && defined(__clang__)
#pragma pop_macro("_MSC_VER")
#endif

#if defined(OVERRIDE)
const int SIMD_COUNT = OVERRIDE;
#elif defined(__AVX512F__) || defined(__AVX512CD__) || defined(__AVX512ER__)
const int SIMD_COUNT = 16;
#elif defined(__AVX__) || defined(__AVX2__)
const int SIMD_COUNT = 8;
#elif defined(__SSE__) || defined(__SSE2__) || defined(__SSE2_MATH__) || defined(_M_IX86_FP) || (defined(_M_AMD64) || defined(_M_X64))
const int SIMD_COUNT = 4;
#else
const int SIMD_COUNT = 1;
#endif

template<typename W, size_t Q>
struct max_sse
{
	static const bool value = (Q * sizeof(W) == 16);
};

template<typename W, size_t Q>
struct max_avx
{
	static const bool value = (Q * sizeof(W) == 32);
};

template<typename W, size_t Q>
struct max_avx512
{
	static const bool value = (Q * sizeof(W) == 64);
};

constexpr int NumOfSIMD(const int val)
{
	return (val - 1) / SIMD_COUNT + 1;
}

#include "vector.h"

#endif
