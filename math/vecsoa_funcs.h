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

	__m128 a = _mm_loadu_ps(v[dim-1]);
	__m128 b = _mm_loadu_ps(v[dim]);
	a = _mm_add_ps(a, b);
	_mm_storeu_ps(v[dim-1], a);

	AddUpDim(--dim);
}

template<size_t Q = Y>
inline typename std::enable_if<max_avx<T, Q>::value, void>::type AddUpDim(int dim)
{
	if (!dim)
		return;

	__m256 a = _mm256_loadu_ps(v[dim-1]);
	__m256 b = _mm256_loadu_ps(v[dim]);
	a = _mm256_add_ps(a, b);
	_mm256_storeu_ps(v[dim-1], a);

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
inline void Dot(const SOA_TYPE& ov, T val[Y]) const
{
	SOA_TYPE nv = *this * ov;
	nv.AddUp<D>();

	for (size_t i = 0; i < Y; i++)
		val[i] = nv[0][i];
}

template <size_t D>
inline void LengthSqr(T val[Y]) const
{
	Dot<D>(*this, val);
}

template <size_t D>
inline void Length(T val[Y]) const
{
	Dot<D>(*this, val);
	VSqrt<T, Y>(val);
}

inline void Dot(const SOA_TYPE& o, T val[Y]) const
{
	Dot<X>(o, val);
}

inline void LengthSqr(T val[Y]) const
{
	LengthSqr<X>(val);
}

inline void Length(T val[Y])
{
	Length<X>(val);
}

template <size_t D>
inline void DistTo(const SOA_TYPE& o, T val[Y]) const
{
	(*this - o).template Length<D>(val);
}

inline void DistTo(const SOA_TYPE& o, T val[Y]) const
{
	DistTo<X>(o, val);
}

//Pointer returning functions
template <size_t D>
inline const T* Dot(const SOA_TYPE& ov) const
{
	T val[Y];
	Dot<D>(ov, val);
	return val;
}

template <size_t D>
inline const T* LengthSqr() const
{
	return Dot<D>(*this);
}

template <size_t D>
inline const T* Length() const
{
	T* val = Dot<D>(*this);
	VSqrt<X>(val);
	return val;
}

inline const T* Dot(const SOA_TYPE& o) const
{
	return Dot<X>(o);
}

inline const T* LengthSqr() const
{
	return LengthSqr<X>();
}

inline const T* Length() const
{
	return Length<X>();
}

template <size_t D>
inline const T* DistTo(const SOA_TYPE& o) const
{
	return (*this - o).template Length<D>();
}

inline const T* DistTo(const SOA_TYPE& o) const
{
	return DistTo<X>(o);
}

auto DirToRay(const SOA_TYPE& a, const SOA_TYPE& b) const
{
	auto c = *this - a;
	auto d = b - a;

	T t[Y], ls[Y];
	c.Dot(d, t);
	d.LengthSqr(ls);

	for (int i = 0; i < Y; i++)
		t[i] = t[i] / ls[i];

	return a + d * t;
}

auto DirToLine(const SOA_TYPE& a, const SOA_TYPE& b) const
{
	auto c = *this - a;
	auto d = b - a;

	T t[Y], ls[Y];
	c.Dot(d, t);
	d.LengthSqr(ls);

	for (int i = 0; i < Y; i++)
		t[i] = std::clamp(t[i] / ls[i], T(0), T(1));

	return a + d * t;
}

auto Normalized() const
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
inline void TransformInPlace(const Q* inp)
{
	Q dot[Y];
	for (size_t o = 0; o < Y; o++)
		for (size_t i = 0; i < X; i++)
			v[o][i] = Dot(inp[o].vec[i]) + inp[o].vec[i][3];
}

template<typename Q>
inline auto Transform(const Q* inp) const
{
	auto ret = *this;
	ret.TransformInPlace(inp);
	return ret;
}

inline auto& AssignCol(int col, const vecb<T, Y>& vec)
{
	for (size_t i = 0; i < Y; i++)
		v[col][i] = vec[i];

	return *this;
}

inline auto& AssignCol(int col, const vecp<T, Y>& vec)
{
	return AssignCol(col, (const vecb<T, Y>&)vec);
}

inline auto& AssignCol(int col, T val)
{
	vecb<T, Y> vec;
	return AssignCol(col, vec.Assign(val));
}


inline auto& MulCol(int col, const vecb<T, Y>& vec)
{
	for (size_t i = 0; i < Y; i++)
		v[col][i] *= vec[i];

	return *this;
}

inline auto& MulCol(int col, const vecp<T, Y>& vec)
{
	return MulCol(col, (vecb<T, Y>)vec);
}

inline auto& MulCol(int col, T val)
{
	for (int i = 0; i < Y; i++)
		v[col][i] *= val;
}

inline auto& MulCol(int col, const T* val)
{
	for (int i = 0; i < Y; i++)
		v[col][i] *= val[i];
}


inline auto& AddCol(int col, const vecb<T, Y>& vec)
{
	for (size_t i = 0; i < Y; i++)
		v[col][i] += vec[i];

	return *this;
}

inline auto& AddCol(int col, const vecp<T, Y>& vec)
{
	return MulCol(col, (vecb<T, Y>)vec);
}

inline auto& AddCol(int col, T val)
{
	for (int i = 0; i < Y; i++)
		v[col][i] *= val;
}
inline auto& AddCol(int col, const T* val)
{
	for (int i = 0; i < Y; i++)
		v[col][i] *= val[i];
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
