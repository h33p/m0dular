#ifndef MMATH_H
#define MMATH_H

#define OVERRIDE 4

#if defined(OVERRIDE)
const int SIMD_COUNT = OVERRIDE;
#elif defined(__AVX512F__) || defined(__AVX512CD__) || defined(__AVX512ER__)
const int SIMD_COUNT = 16;
#elif defined(__AVX__) || defined(__AVX2__)
const int SIMD_COUNT = 8;
#elif defined(__SSE__) || defined(__SSE2__) || defined(__SSE2_MATH__) || defined(_M_IX86_FP )
const int SIMD_COUNT = 4;
#else
const int SIMD_COUNT = 1;
#endif

constexpr int NumOfSIMD(const int val)
{
	return (val - 1) / SIMD_COUNT + 1;
}

#include "vector.h"

#endif
