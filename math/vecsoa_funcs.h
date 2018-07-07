/*
	These functions should only be included inside SoA vector structures.
	Define SOA_TYPE before including, it will be undefined afterwards.
*/


#ifdef SOA_TYPE

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
inline void Dot(SOA_TYPE& ov, T val[Y])
{
	SOA_TYPE nv = *this * ov;
	nv.AddUp<D>();

	for (size_t i = 0; i < Y; i++)
		val[i] = nv.x[i];
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

inline void Dot(SOA_TYPE& o, T val[Y])
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
inline void DistTo(SOA_TYPE& o, T val[Y])
{
	(*this - o).template Length<D>(val);
}

inline void DistTo(SOA_TYPE& o, T val[Y])
{
	DistTo<X>(o, val);
}

//Pointer returning functions
template <size_t D>
inline T* Dot(SOA_TYPE& ov)
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
	VSqrt<X>(val);
	return val;
}

inline T* Dot(SOA_TYPE& o)
{
	return Dot<X>(o);
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
T* DistTo(SOA_TYPE& o)
{
	return (*this - o).template Length<D>();
}

T* DistTo(SOA_TYPE& o)
{
	return DistTo<X>(o);
}

auto Normalized()
{
	auto val = *this;
	float l[Y];
	val.Length(l);
	for (size_t i = 0; i < Y; i++)
		l[i] = l[i] ? 1.f / l[i] : (T)0;
	val *= l;
	return val;
}

void Normalize()
{
	*this = Normalized();
}

template<typename Q>
inline void TransformInPlace(Q* inp)
{
	Q dot[Y];
	for (size_t o = 0; o < Y; o++)
		for (size_t i = 0; i < X; i++)
			v[o][i] = Dot(inp[o].vec[i]) + inp[o].vec[i][3];
}

template<typename Q>
inline auto Transform(Q* inp)
{
	auto ret = *this;
	ret.TransformInPlace(inp);
	return ret;
}

inline auto& AssignCol(int col, vecb<T, Y> vec)
{
	for (size_t i = 0; i < Y; i++)
		v[col][i] = vec[i];

	return *this;
}

inline auto& AssignCol(int col, vecp<T, Y> vec)
{
	return AssignCol(col, (vecb<T, Y>)vec);
}

inline auto& AssignCol(int col, T val)
{
	vecb<T, Y> vec;
	return AssignCol(col, vec.Assign(val));
}

inline auto Rotate()
{
	vecSoa<T, Y, X> ret;

	for (size_t i = 0; i < X; i++)
		for (size_t o = 0; o < Y; o++)
			ret[o][i] = v[i][o];

	return ret;
}

#undef SOA_TYPE
#endif
