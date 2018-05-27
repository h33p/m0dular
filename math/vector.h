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
#include <stdlib.h>

template<typename T, size_t Q>
inline void VSqrt(T val[Q])
{
	for (int i = 0; i < Q; i++)
		val[i] = sqrt(val[i]);
}

#if PSIMD >= 4
template<>
inline void VSqrt<float, 4>(float val[4])
{
	__m128 x = _mm_load_ps(val);
	x = _mm_sqrt_ps(x);
	_mm_store_ps(val, x);
}
#endif

#if PSIMD >= 8
template<>
inline void VSqrt<float, 8>(float val[8])
{
	__m256 x = _mm256_load_ps(val);
	x = _mm256_sqrt_ps(x);
	_mm256_store_ps(val, x);
}
#endif

#if PSIMD >= 16
template<>
inline void VSqrt<float, 16>(float val[16])
{
	__m512 x = _mm512_load_ps(val);
	x = _mm512_sqrt_ps(x);
	_mm512_store_ps(val, x);
}
#endif

template<typename T, size_t N>
struct vecp;

template<typename T, size_t N>
struct vecb
{
	T v[N];

	DEFINE_VEC_OPS(vecb,);
	DEFINE_VEC_OPS(vecb,const);

	bool operator==(vecb& o)
	{
		for (int i = 0; i < N; i++)
			if (v[i] != o.v[i])
				return false;
		return true;
	}

	bool operator!=(vecb& o)
	{
		return !operator==(o);
	}

	template <size_t D>
	inline T Dot(vecb& o)
	{
		T val = 0;
		for (int i = 0; i < D; i++)
			val += v[i] * o.v[i];
		return val;
	}

	template <size_t D>
	inline T LengthSqr()
	{
		return Dot<D>(*this);
	}

	template <size_t D>
	inline T Length()
	{
		return sqrt(Dot<D>(*this));
	}

	inline T Dot(vecb& o)
	{
		return Dot<N>(o);
	}

	inline T LengthSqr()
	{
		return LengthSqr<N>();
	}

	inline T Length()
	{
		return Length<N>();
	}

	auto Normalized()
	{
		auto val = *this;
		float l = val.Length();
		val = l ? val / l : 0;
		return val;
	}

	void Normalize()
	{
		*this = Normalized();
	}

	template <size_t D>
	T DistTo(vecb& o)
	{
		return (*this - o).template Length<D>();
	}

	T DistTo(vecb& o)
	{
		return DistTo<N>(o);
	}

	inline T& operator[](int idx)
	{
		return v[idx];
	}

	inline const T& operator[](int idx) const
	{
		return v[idx];
	}

	template<size_t B>
	inline operator vecp<T, B>()
	{
		constexpr size_t mv = B < 4 ? B : 4;
		vecp<T, B> vec;
		for (size_t i = 0; i < mv; i++)
			vec[i] = v[i];
		return vec;
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

	DEFINE_VEC_OPS(vecp,);
	DEFINE_VEC_OPS(vecp,const);

	bool operator==(vecp& o)
	{
		for (int i = 0; i < N; i++)
			if (v[i] != o.v[i])
				return false;
		return true;
	}

	bool operator!=(vecp& o)
	{
		return !operator==(o);
	}

	template <size_t D>
	inline T Dot(vecp& o)
	{
		T val = 0;
		for (int i = 0; i < D; i++)
			val += v[i] * o.v[i];
		return val;
	}

	template <size_t D>
	inline T LengthSqr()
	{
		return Dot<D>(*this);
	}

	template <size_t D>
	inline T Length()
	{
		return sqrt(Dot<D>(*this));
	}

	inline T Dot(vecp& o)
	{
		return Dot<N>(o);
	}

	inline T LengthSqr()
	{
		return LengthSqr<N>();
	}

	inline T Length()
	{
		return Length<N>();
	}

	auto Normalized()
	{
		auto val = *this;
		float l = val.Length();
		val = l ? val / l : 0;
		return val;
	}

	void Normalize()
	{
		*this = Normalized();
	}

	template <size_t D>
	T DistTo(vecp& o)
	{
		return (*this - o).template Length<D>();
	}

	T DistTo(vecp& o)
	{
		return DistTo<N>(o);
	}

	inline T& operator[](int idx)
	{
		return v[idx];
	}

	template<size_t B>
	inline auto& operator=(vecb<float, B>& vec)
	{
		constexpr size_t mv = B < 4 ? B : 4;
		for (size_t i = 0; i < mv; i++)
			v[i] = vec[i];
		return *this;
	}

	template<size_t B>
	inline operator vecb<T, B>()
	{
		constexpr size_t mv = B < 4 ? B : 4;
		vecb<T, B> vec;
		for (size_t i = 0; i < mv; i++)
			vec[i] = v[i];
		return vec;
	}
};

template<typename T, size_t Y>
struct vec3soa
{
	static constexpr size_t X = 3;
	union {
		struct {
			T x[Y];
			T y[Y];
			T z[Y];
		};
		T v[X][Y];
		struct {
			struct SoaAccessor {
				T x;
				T px[Y - 1];
				T y;
				T py[Y - 1];
				T z;

				inline T& operator[](int idx)
				{
					return px[idx * Y - 1];
				}

				inline auto& Set(SoaAccessor& acc)
				{
					x = acc.x;
					y = acc.y;
					z = acc.z;
					return *this;
				}

				template<size_t B>
				inline auto& operator=(vecb<float, B>& vec)
				{
					constexpr size_t mv = B < X ? B : X;
					auto& it = *this;
					for (size_t i = 0; i < mv; i++)
						it[i] = vec[i];
					return it;
				}
			} acc2;

			inline auto& operator[](int idx)
			{
				return *(SoaAccessor*)(((T*)&acc2)+idx);
			}

		} acc;
	};

	DEFINE_SOA_OPS(vec3soa,);
	DEFINE_SOA_OPS(vec3soa,const);

	bool operator==(vec3soa& ov)
	{
		for (int i = 0; i < X; i++)
			for (int o = 0; o < Y; o++)
				if (v[i][o] != ov.v[i][o])
					return false;
		return true;
	}

	bool operator!=(vec3soa& o)
	{
		return !operator==(o);
	}

	template <size_t D>
	inline auto& AddUp()
	{
		for (size_t i = D - 1; i > 0; i--)
			for (size_t o = 0; o < Y; o++)
			v[i-1][o] += v[i][o];
		return *this;
	}

	//Constant array functions
	template <size_t D>
	inline void Dot(vec3soa& ov, T val[Y])
	{
		vec3soa nv = *this * ov;
		nv.AddUp<D>();

		for (int i = 0; i < Y; i++)
			val[i] = nv.z[i];
	}

	template <size_t D>
	inline void LengthSqr(T val[Y])
	{
		Dot<D>(*this, val);
	}

	template <size_t D>
	inline void Length(T val[Y])
	{
	    Dot<D>(*this, val);
		VSqrt<Y>(val);
	}

	inline void Dot(vec3soa& o, T val[Y])
	{
		Dot<Y>(o, val);
	}

	inline void LengthSqr(T val[Y])
	{
		LengthSqr<Y>(val);
	}

	inline void Length(T val[Y])
	{
		Length<Y>(val);
	}

	template <size_t D>
	inline void DistTo(vec3soa& o, T val[Y])
	{
		(*this - o).template Length<D>(val);
	}

	inline void DistTo(vec3soa& o, T val[Y])
	{
		DistTo<Y>(o, val);
	}

	//Pointer returning functions
	template <size_t D>
	inline T* Dot(vec3soa& ov)
	{
		T val[Y];
	    Dot<D>(ov, val);
		return val;
	}

	template <size_t D>
	inline T* LengthSqr()
	{
		return Dot<D>(*this);
	}

	template <size_t D>
	inline T* Length()
	{
	    T* val = Dot<D>(*this);
		VSqrt<Y>(val);
		return val;
	}

	inline T* Dot(vec3soa& o)
	{
		return Dot<Y>(o);
	}

	inline T* LengthSqr()
	{
		return LengthSqr<Y>();
	}

	inline T* Length()
	{
		return Length<Y>();
	}

	template <size_t D>
	T* DistTo(vec3soa& o)
	{
		return (*this - o).template Length<D>();
	}

	T* DistTo(vec3soa& o)
	{
		return DistTo<Y>(o);
	}

	auto Normalized()
	{
		auto val = *this;
		float l[Y];
		val.Length(l);
		val = l ? val / l : 0;
		return val;
	}

	void Normalize()
	{
		*this = Normalized();
	}

	inline T* operator[](int idx)
	{
		return v[idx];
	}
};

template<typename T, size_t X, size_t Y>
struct vecSoa
{
	T v[X][Y];

	DEFINE_SOA_OPS(vecSoa,);
	DEFINE_SOA_OPS(vecSoa,const);

	bool operator==(vecSoa& ov)
	{
		for (int i = 0; i < X; i++)
			for (int o = 0; o < Y; o++)
				if (v[i][o] != ov.v[i][o])
					return false;
		return true;
	}

	bool operator!=(vecSoa& o)
	{
		return !operator==(o);
	}

	//Micro-optimized version for 4 sized vector chunks since
	//Clang did not want to generate SIMD code on a normal loop
	template<size_t Q = Y>
	inline typename std::enable_if<max_sse<T, Q>::value, void>::type AddUpDim(int dim)
	{
		if (!dim)
			return;

		__m128 a = _mm_load_ps(v[dim-1]);
		__m128 b = _mm_load_ps(v[dim]);
		a = _mm_add_ps(a, b);
		_mm_store_ps(v[dim-1], a);

		AddUpDim(--dim);
	}

	template<size_t Q = Y>
	inline typename std::enable_if<max_avx<T, Q>::value, void>::type AddUpDim(int dim)
	{
		if (!dim)
			return;

		__m256 a = _mm256_load_ps(v[dim-1]);
		__m256 b = _mm256_load_ps(v[dim]);
		a = _mm256_add_ps(a, b);
		_mm256_store_ps(v[dim-1], a);

		AddUpDim(--dim);
	}

	template<size_t Q = Y>
	inline typename std::enable_if<(!max_sse<T, Q>::value && !max_avx<T, Q>::value), void>::type AddUpDim(int dim)
	{
		if (!dim)
			return;

		for(; dim > 0; dim--) {
			T* v1 = v[dim-1];
			T* v2 = v[dim];
			for (size_t o = 0; o < Y; o++)
				v1[o] += v2[o];
		}
	}

	template <size_t D>
	inline auto& AddUp()
	{
		AddUpDim(D-1);
		return *this;
	}

	//Constant array functions
	template <size_t D>
	inline void Dot(vecSoa& ov, T val[Y])
	{
		vecSoa nv = *this * ov;
		nv.AddUp<D>();

		for (int i = 0; i < Y; i++)
			val[i] = nv[0][i];
	}

	template <size_t D>
	inline void LengthSqr(T val[Y])
	{
		Dot<D>(*this, val);
	}

	template <size_t D>
	inline void Length(T val[Y])
	{
	    Dot<D>(*this, val);
		VSqrt<T, Y>(val);
	}

	inline void Dot(vecSoa& o, T val[Y])
	{
		Dot<X>(o, val);
	}

	inline void LengthSqr(T val[Y])
	{
		LengthSqr<X>(val);
	}

	inline void Length(T val[Y])
	{
		Length<X>(val);
	}

	template <size_t D>
	inline void DistTo(vecSoa& o, T val[Y])
	{
		(*this - o).template Length<D>(val);
	}

	inline void DistTo(vecSoa& o, T val[Y])
	{
		DistTo<X>(o, val);
	}

	//Pointer returning functions
	template <size_t D>
	inline T* Dot(vecSoa& ov)
	{
		T val[Y];
		Dot<D>(ov, val);
		return val;
	}

	template <size_t D>
	inline T* LengthSqr()
	{
		return Dot<D>(*this);
	}

	template <size_t D>
	inline T* Length()
	{
	    T* val = Dot<D>(*this);
		VSqrt<T, Y>(val);
		return val;
	}

	inline T* Dot(vecSoa& o)
	{
		return Dot<Y>(o);
	}

	inline T* LengthSqr()
	{
		return LengthSqr<X>();
	}

	inline T* Length()
	{
		return Length<X>();
	}

	template <size_t D>
	T* DistTo(vecSoa& o)
	{
		return (*this - o).template Length<D>();
	}

	T* DistTo(vecSoa& o)
	{
		return DistTo<X>(o);
	}

	auto Normalized()
	{
		auto val = *this;
		float l[Y];
		val.Length(l);
		val = l ? val / l : 0;
		return val;
	}

	void Normalize()
	{
		*this = Normalized();
	}

	inline T* operator[](int idx)
	{
		return v[idx];
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

template<size_t X, size_t Y>
struct matrix
{
	vecSoa<float, X, Y> vec;
	
	template <size_t X2, size_t Y2>
	inline auto& operator =(matrix<X2, Y2>& ov)
	{
		constexpr size_t MX = X2 < X ? X2 : X;
		constexpr size_t MY = Y2 < Y ? Y2 : Y;
		for (size_t i = 0; i < MY; i++)
			for (size_t o = 0; o < MX; o++)
				vec[i][o] = ov.vec[i][o];
		return *this;
	}

	template<typename T>
	inline auto Vector3Transform(T& inp)
	{
		T out;

		for (size_t i = 0; i < 3; i++)
			out[i] = inp.Dot(vec[i]) + vec[i][3];

		return out;
	}
};

typedef matrix<4,4> matrix4x4;
#endif
