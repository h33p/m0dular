#ifndef VECTOR_H
#define VECTOR_H
#include "stddef.h"
#include "mmath.h"
#include "math.h"

#include "vector_operators.h"
#include <stdlib.h>

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

	//Constant array functions
	template <size_t D>
	inline void Dot(vec3soa& ov, T val[Y])
	{
		for (int i = 0; i < Y; i++) {
			T tv = 0;
			for (int o = 0; o < D; o++)
				tv += v[o][i] * ov.v[o][i];
			val[i] = tv;
		}
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
		for (int i = 0; i < Y; i++)
			val[i] = sqrt(val[i]);
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
		for (int i = 0; i < Y; i++)
			val[i] = 0;

		for (int i = 0; i < D; i++)
			for (int o = 0; o < Y; o++)
				val[o] += v[i][o] * ov.v[i][o];
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
		for (int i = 0; i < Y; i++)
			val[i] = sqrt(val[i]);
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

	//Constant array functions
	template <size_t D>
	inline void Dot(vecSoa& ov, T val[Y])
	{
		for (int i = 0; i < Y; i++) {
			T tv = 0;
			for (int o = 0; o < D; o++)
				tv += v[o][i] * ov.v[o][i];
			val[i] = tv;
		}
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
		for (int i = 0; i < Y; i++)
			val[i] = sqrt(val[i]);
	}

	inline void Dot(vecSoa& o, T val[Y])
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
	inline void DistTo(vecSoa& o, T val[Y])
	{
		(*this - o).template Length<D>(val);
	}

	inline void DistTo(vecSoa& o, T val[Y])
	{
		DistTo<Y>(o, val);
	}

	//Pointer returning functions
	template <size_t D>
	inline T* Dot(vecSoa& ov)
	{
		T val[Y];
		for (int i = 0; i < Y; i++)
			val[i] = 0;

		for (int i = 0; i < D; i++)
			for (int o = 0; o < Y; o++)
				val[o] += v[i][o] * ov.v[i][o];
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
		for (int i = 0; i < Y; i++)
			val[i] = sqrt(val[i]);
		return val;
	}

	inline T* Dot(vecSoa& o)
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
	T* DistTo(vecSoa& o)
	{
		return (*this - o).template Length<D>();
	}

	T* DistTo(vecSoa& o)
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

template<size_t N>
using vec = vecb<float, N>;
using vec2 = vec<2>;
using vec3 = vec<3>;
using vec4 = vec<4>;

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
#endif
