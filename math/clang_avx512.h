
//A hack to improve compile times on clang
#if defined(__clang__) && !defined(__AVX512F__) && !defined(__AVX512CD__) && !defined(__AVX512ER__)
#define __AVX512BITALGINTRIN_H
#define __AVX512BWINTRIN_H
#define __AVX512CDINTRIN_H
#define __AVX512DQINTRIN_H
#define __AVX512ERINTRIN_H
#define __AVX512FINTRIN_H
#define __AVX512PFINTRIN_H
#define __AVX512VBMI2INTRIN_H
#define __AVX512VLBITALGINTRIN_H
#define __AVX512VLBWINTRIN_H
#define __AVX512VLCDINTRIN_H
#define __AVX512VLDQINTRIN_H
#define __AVX512VLINTRIN_H
#define __AVX512VLVBMI2INTRIN_H
#define __AVX512VLVNNIINTRIN_H
#define __AVX512VNNIINTRIN_H
#define __AVX512VPOPCNTDQINTRIN_H
#define __AVX512VPOPCNTDQVLINTRIN_H

#define __IFMAINTRIN_H
#define __IFMAVLINTRIN_H
#define __VBMIINTRIN_H
#define __VBMIVLINTRIN_H

typedef char __v64qi __attribute__((__vector_size__(64)));
typedef short __v32hi __attribute__((__vector_size__(64)));
typedef double __v8df __attribute__((__vector_size__(64)));
typedef float __v16sf __attribute__((__vector_size__(64)));
typedef long long __v8di __attribute__((__vector_size__(64)));
typedef int __v16si __attribute__((__vector_size__(64)));

/* Unsigned types */
typedef unsigned char __v64qu __attribute__((__vector_size__(64)));
typedef unsigned short __v32hu __attribute__((__vector_size__(64)));
typedef unsigned long long __v8du __attribute__((__vector_size__(64)));
typedef unsigned int __v16su __attribute__((__vector_size__(64)));

typedef float __m512 __attribute__((__vector_size__(64)));
typedef double __m512d __attribute__((__vector_size__(64)));
typedef long long __m512i __attribute__((__vector_size__(64)));

typedef unsigned char __mmask8;
typedef unsigned short __mmask16;

#define __DEFAULT_FN_ATTRS512 __attribute__((__always_inline__, __nodebug__, __target__("avx512f"), __min_vector_width__(512)))

static  __inline __m512i __DEFAULT_FN_ATTRS512
_mm512_setzero_si512(void)
{
	return __extension__ (__m512i)(__v8di){ 0, 0, 0, 0, 0, 0, 0, 0 };
}

typedef unsigned int __mmask32;
typedef unsigned long long __mmask64;
typedef unsigned short __v64hu __attribute__((__vector_size__(128)));
typedef unsigned int __v32su __attribute__((__vector_size__(128)));

typedef __v8di __v8di_aligned __attribute__((aligned(64)));
typedef __v8di __v8di_aligned __attribute__((aligned(64)));
typedef __v8df __v8df_aligned __attribute__((aligned(64)));
typedef __v16sf __v16sf_aligned __attribute__((aligned(64)));
typedef short __v2hi __attribute__((__vector_size__(4)));
typedef char __v4qi __attribute__((__vector_size__(4)));
typedef char __v2qi __attribute__((__vector_size__(2)));


#endif
