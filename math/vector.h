#ifndef VECTOR_H
#define VECTOR_H

/*
 * This vector library is focuesed about high efficiency
 * by laying out the data in a very easily vectorizable way.
 * A compiler that does automatic SIMD code generation should
 * be able to work more easily on this layout.
 *
 * Clang does a really good job at generating SIMD code, while
 * MSVC does not always work. Some more tricky parts, such as
 * square root calculation have manual "hand written" SSE/AVX
 * implementations, while addition, multiplication, etc. usually
 * have correct code generation.
 *
 * All the code logic should focus around the data layout of the
 * SOA vectors, even though accessors (in vec3soa case) are implemented
 * in case you need to conveniently access all dimensions in one go.
 * The data is laid out like this for a reason.
*/

#include "stddef.h"
#include "mmath.h"
#include "math.h"

#include "vector_operators.h"
#include "soa_accessor.h"
#include <stdlib.h>
#include <functional>

template<typename T, size_t Q>
inline void VSqrt(T val[Q])
{
	for (size_t i = 0; i < Q; i++)
		val[i] = sqrt(val[i]);
}

#if PSIMD >= 4
template<>
inline void VSqrt<float, 4>(float val[4])
{
	__m128 x = _mm_loadu_ps(val);
	x = _mm_sqrt_ps(x);
	_mm_storeu_ps(val, x);
}
#endif

#if PSIMD >= 8
template<>
inline void VSqrt<float, 8>(float val[8])
{
	__m256 x = _mm256_loadu_ps(val);
	x = _mm256_sqrt_ps(x);
	_mm256_storeu_ps(val, x);
}
#endif

#if PSIMD >= 16
template<>
inline void VSqrt<float, 16>(float val[16])
{
	__m512 x = _mm512_loadu_ps(val);
	x = _mm512_sqrt_ps(x);
	_mm512_storeu_ps(val, x);
}
#endif

template<typename T, size_t N>
struct vecp;

template<typename T, size_t Y>
struct vec3soa;

template<typename T, size_t X, size_t Y>
struct vecSoa;

template<size_t X, size_t Y>
struct matrix;

template<typename T, size_t N>
struct vecb
{
	T v[N];

	DEFINE_VEC_OPS(vecb);

#define VEC_TYPE vecb
#include "vec_funcs.h"

	constexpr bool operator==(const vecb& o)
	{
		for (size_t i = 0; i < N; i++)
			if (v[i] != o.v[i])
				return false;
		return true;
	}

	constexpr bool operator!=(const vecb& o)
	{
		return !operator==(o);
	}

	constexpr T& operator[](int idx)
	{
		return v[idx];
	}

	constexpr const T& operator[](int idx) const
	{
		return v[idx];
	}

	template<size_t B>
	constexpr operator vecp<T, B>()
	{
		constexpr size_t mv = B < 4 ? B : 4;
		vecp<T, B> vec = {};
		for (size_t i = 0; i < mv; i++)
			vec[i] = v[i];
		return vec;
	}

	template<size_t B>
	constexpr operator vec3soa<T, B>()
	{
		vec3soa<T, B> ret = {};
		for (size_t i = 0; i < 3; i++)
			for (size_t o = 0; o < B; o++)
				ret[i][o] = v[i];
		return ret;
	}

};

template<typename T, size_t N>
struct vecp
{
	union {
		struct {
			float x, y, z, w;
		};
		T v[4];
	};

	DEFINE_VEC_OPS(vecp);

#define VEC_TYPE vecp
#include "vec_funcs.h"


	constexpr bool operator==(const vecp& o)
	{
		for (size_t i = 0; i < N; i++)
			if (v[i] != o.v[i])
				return false;
		return true;
	}

	constexpr bool operator!=(const vecp& o)
	{
		return !operator==(o);
	}

	constexpr T& operator[](int idx)
	{
		return v[idx];
	}

	constexpr const T& operator[](int idx) const
	{
		return v[idx];
	}

	template<size_t B>
	constexpr auto& operator=(vecb<float, B>& vec)
	{
		constexpr size_t mv = B < 4 ? B : 4;
		for (size_t i = 0; i < mv; i++)
			v[i] = vec[i];
		return *this;
	}

	template<size_t B>
	constexpr operator vecb<T, B>()
	{
		constexpr size_t mv = B < 4 ? B : 4;
		vecb<T, B> vec = {};
		for (size_t i = 0; i < mv; i++)
			vec[i] = v[i];
		return vec;
	}

	template<size_t B>
	constexpr operator vec3soa<T, B>()
	{
		vec3soa<T, B> ret = {};
		for (size_t i = 0; i < 3; i++)
			for (size_t o = 0; o < B; o++)
				ret[i][o] = v[i];
		return ret;
	}
};

template<typename T, size_t Y>
struct vec3soa
{
	static constexpr size_t X = 3;
	static constexpr size_t Xt = X;
	static constexpr size_t Yt = Y;
	union {
		struct {
			T x[Y];
			T y[Y];
			T z[Y];
		};
		T v[X][Y];
		DEFINE_SOA_ACCESSOR;
	};

	DEFINE_SOA_OPS(vec3soa);
	DEFINE_SOA_VEC_OPS(vec3soa);

#define SOA_TYPE vec3soa
#include "vecsoa_funcs.h"

	constexpr bool operator==(const vec3soa& ov) const
	{
		for (size_t i = 0; i < X; i++)
			for (size_t o = 0; o < Y; o++)
				if (v[i][o] != ov.v[i][o])
					return false;
		return true;
	}

	constexpr bool operator!=(const vec3soa& o) const
	{
		return !operator==(o);
	}

	inline void ToAngles()
	{
		T y[Y], x[Y], len[Y];
		for (size_t o = 0; o < Y; o++)
			y[o] = atan2(v[1][o], v[0][o]);

		Length<2>(len);

		for (size_t o = 0; o < Y; o++)
			x[o] = atan2(-v[1][o], len[o]);

		for (size_t o = 0; o < Y; o++)
			v[0][o] = x[o];

		for (size_t o = 0; o < Y; o++)
			v[1][o] = y[o];

		for (size_t o = 0; o < Y; o++)
			v[2][o] = 0;

	}

	constexpr auto GetAngles()
	{
		auto ret = *this;
		ret.ToAngles();
		return ret;
	}

	constexpr T* operator[](int idx)
	{
		return v[idx];
	}

	constexpr const T* operator[](int idx) const
	{
		return v[idx];
	}

};

template<typename T, size_t X, size_t Y>
struct vecSoa
{
	static constexpr size_t Xt = X;
	static constexpr size_t Yt = Y;
	union
	{
		T v[X][Y];
		DEFINE_SOA_ACCESSOR;
	};

	DEFINE_SOA_OPS(vecSoa);
	DEFINE_SOA_VEC_OPS(vecSoa);

#define SOA_TYPE vecSoa
#include "vecsoa_funcs.h"

	constexpr bool operator==(const vecSoa& ov) const
	{
		for (size_t i = 0; i < X; i++)
			for (size_t o = 0; o < Y; o++)
				if (v[i][o] != ov.v[i][o])
					return false;
		return true;
	}

	constexpr bool operator!=(const vecSoa& o) const
	{
		return !operator==(o);
	}

    constexpr T* operator[](int idx)
	{
		return v[idx];
	}

    constexpr const T* operator[](int idx) const
	{
		return v[idx];
	}

	template<size_t B>
	constexpr operator vec3soa<T, B>()
	{
		constexpr int mv = X < 3 ? X : 3;
		constexpr int mb = Y < B ? Y : B;
		vec3soa<T, B> ret;
		for (size_t i = 0; i < mv; i++)
			for (size_t o = 0; o < mb; o++)
				ret[i][o] = v[i][o];
		return ret;
	}
};

template<size_t N>
using vec = vecb<float, N>;
using vec2 = vec<2>;
using vec3 = vec<3>;
using vec4 = vec<4>;

using vec3_t = vecp<float, 3>;
using vec4_t = vecp<float, 4>;

using xvec3 = vec3soa<float, 4>;
using yvec3 = vec3soa<float, 8>;
using zvec3 = vec3soa<float, 16>;
using nvec3 = vec3soa<float, SIMD_COUNT>;
template<size_t Y>
using svec3 = vec3soa<float, Y>;

template<size_t X>
using xvec = vecSoa<float, X, 4>;
template<size_t X>
using yvec = vecSoa<float, X, 8>;
template<size_t X>
using zvec = vecSoa<float, X, 16>;
template<size_t X>
using nvec = vecSoa<float, X, SIMD_COUNT>;

template <size_t N>
using veci = vecb<int, N>;

#include "matrix.h"

#endif
