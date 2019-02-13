/*
	These functions should only be included inside SoA vector structures.
	Define SOA_TYPE before including, it will be undefined afterwards.
*/


#ifdef SOA_TYPE

constexpr SOA_TYPE() = default;

template<typename F, typename = typename std::enable_if<AllArithmetic<F>::value>::type>
constexpr SOA_TYPE(F arg) : v()
{
	for (size_t i = 0; i < Xt; i++)
		for (size_t o = 0; o < Yt; o++)
			v[i][o] = arg;
}

template<typename... F, typename = typename std::enable_if<AllArithmetic<F...>::value>::type>
constexpr SOA_TYPE(F... args) : v()
{
	constexpr size_t elementCount = sizeof...(args);

	for (size_t i = 0; i < Xt; i++)
		for (size_t o = 0; o < Yt; o++)
			v[i][o] = GetElementAt(o % elementCount, args...);
}

template<size_t Y2>
inline auto& ColumnVec(int col) const
{
	return *(vecb<T, Y2>*)v[col];
}

inline auto& ColumnVec(int col) const
{
	return ColumnVec<Y>(col);
}

//Micro-optimized version for 4 sized vector chunks since
//Clang did not want to generate SIMD code on a normal loop
template<size_t SWidth, typename F = T, size_t Q = Y, size_t D = X>
inline void AddUpDimSIMD(int dim, F vv[D][Q])
{
	static constexpr size_t Elems = SWidth / (8 * sizeof(F));

	if (!dim)
		return;

	for (size_t i = 0; i < Q / Elems; i++) {
		_m<SWidth, F> a = _mm_loadu<SWidth, F>(v[dim-1] + i * Elems);
		_m<SWidth, F> b = _mm_loadu<SWidth, F>(vv[dim] + i * Elems);
		a = _mm_add<SWidth, F>(a, b);
		_mm_storeu<SWidth, F>(vv[dim-1] + i * Elems, a);
	}

	AddUpDimSIMD<SWidth, T, Y, X>(--dim, vv);
}


template<typename F = T, size_t Q = Y, size_t D = X>
inline typename std::enable_if<do_sse<F, Q>::value, void>::type AddUpDim(int dim, F vv[D][Q])
{
	AddUpDimSIMD<128, F, Q, D>(dim, vv);
}

template<typename F = T, size_t Q = Y, size_t D = X>
inline typename std::enable_if<do_avx<F, Q>::value, void>::type AddUpDim(int dim, F vv[D][Q])
{
	AddUpDimSIMD<256, F, Q, D>(dim, vv);
}

template<typename F = T, size_t Q = Y, size_t D = X>
inline typename std::enable_if<do_avx512<F, Q>::value, void>::type AddUpDim(int dim, F vv[D][Q])
{
	AddUpDimSIMD<512, F, Q, D>(dim, vv);
}

template<typename F = T, size_t Q = Y, size_t D = X>
inline typename std::enable_if<!do_simd<F, Q>::value, void>::type AddUpDim(int dim, F vv[X][Q])
{
	if (!dim)
		return;

	for(; dim > 0; dim--)
		for (size_t o = 0; o < Y; o++)
			vv[dim-1][o] = v[dim-1][o] + vv[dim][o];
}

template <size_t D>
inline auto& AddUp()
{
	AddUpDim<T, Y, X>(D-1, v);
	return *this;
}

template <size_t D>
inline auto& AddUpTotal()
{
	AddUpDim<T, Y, X>(D-1, v);

	for (size_t i = D - 1; i > 0; i--)
		v[0][i - 1] += v[0][i];

	return *this;
}

template <size_t D>
inline T AddedUpTotal()
{
	T temp[D][Y];

	for (size_t o = 0; o < Y; o++)
		temp[D - 1][o] = 0;

	AddUpDim<T, Y, X>(D - 1, temp);

	for (size_t i = Y - 1; i > 0; i--)
		temp[0][i - 1] += temp[0][i];

	return temp[0][0];
}

inline T AddedUpTotal()
{
	return AddedUpTotal<X>();
}

//Constant array functions
template <typename F, size_t D, size_t Y2>
inline void Dot(const F& ov, T val[Y2]) const
{
	SOA_TYPE nv = *this * ov;
	nv.AddUp<D>();

	for (size_t i = 0; i < Y2; i++)
		val[i] = nv[0][i];
}

template <size_t D>
inline void Dot(const SOA_TYPE& ov, T val[Y]) const
{
    Dot<SOA_TYPE, D, Y>(ov, val);
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

template <size_t D>
inline auto& Sqrt()
{
	for (size_t i = 0; i < D; i++)
		VSqrt<T, Y>(v[i]);
	return *this;
}

template<typename F, size_t Y2>
inline void Dot(const F& o, T val[Y2]) const
{
	Dot<F, X, Y2>(o, val);
}

inline void Dot(const SOA_TYPE& o, T val[Y]) const
{
	Dot<SOA_TYPE, X, Y>(o, val);
}

inline void LengthSqr(T val[Y]) const
{
	LengthSqr<X>(val);
}

inline void Length(T val[Y]) const
{
	Length<X>(val);
}

inline auto& Sqrt()
{
	return Sqrt<X>();
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

constexpr auto Abs() const
{
	auto ret = *this;

	for (size_t i = 0; i < X; i++)
		for (size_t o = 0; o < Y; o++)
			ret[i][o] = std::abs(ret[i][o]);

	return ret;
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

inline auto DirToRay(const SOA_TYPE& a, const SOA_TYPE& b) const
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

inline auto DirToLine(const SOA_TYPE& a, const SOA_TYPE& b) const
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

inline auto Normalized() const
{
	auto val = *this;
	float l[Y];
	val.Length(l);
	for (size_t i = 0; i < Y; i++)
		l[i] = l[i] ? 1.f / l[i] : (T)0;
	val *= l;
	return val;
}

inline void Normalize()
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

constexpr auto& AssignRow(int row, const vecb<T, X>& vec)
{
	for (size_t i = 0; i < X; i++)
		v[i][row] = vec[i];

	return *this;
}

constexpr auto& AddRow(int row, const vecb<T, X>& vec)
{
	for (size_t i = 0; i < X; i++)
		v[i][row] += vec[i];

	return *this;
}

constexpr auto& AddRow(int row, const vecp<T, X>& vec)
{
	return AddRow(row, *(vecb<T, X>*)&vec);
}

constexpr auto& AddRow(int row, T val)
{
	for (int i = 0; i < X; i++)
		v[i][row] += val;
}

constexpr auto& AddRow(int row, const T* val)
{
	for (int i = 0; i < X; i++)
		v[i][row] *= val[i];
}

constexpr auto& AssignCol(int col, const vecb<T, Y>& vec)
{
	for (size_t i = 0; i < Y; i++)
		v[col][i] = vec[i];

	return *this;
}

constexpr auto& AssignCol(int col, const vecp<T, Y>& vec)
{
	return AssignCol(col, (const vecb<T, Y>&)vec);
}

constexpr auto& AssignCol(int col, T val)
{
	vecb<T, Y> vec;
	return AssignCol(col, vec.Assign(val));
}


constexpr auto& MulCol(int col, const vecb<T, Y>& vec)
{
	for (size_t i = 0; i < Y; i++)
		v[col][i] *= vec[i];

	return *this;
}

constexpr auto& MulCol(int col, const vecp<T, Y>& vec)
{
	return MulCol(col, (vecb<T, Y>)vec);
}

constexpr auto& MulCol(int col, T val)
{
	for (int i = 0; i < Y; i++)
		v[col][i] *= val;
}

constexpr auto& MulCol(int col, const T* val)
{
	for (int i = 0; i < Y; i++)
		v[col][i] *= val[i];
}


constexpr auto& AddCol(int col, const vecb<T, Y>& vec)
{
	for (size_t i = 0; i < Y; i++)
		v[col][i] += vec[i];

	return *this;
}

constexpr auto& AddCol(int col, const vecp<T, Y>& vec)
{
	return AddCol(col, (vecb<T, Y>)vec);
}

constexpr auto& AddCol(int col, T val)
{
	for (int i = 0; i < Y; i++)
		v[col][i] += val;
}

constexpr auto& AddCol(int col, const T* val)
{
	for (int i = 0; i < Y; i++)
		v[col][i] += val[i];
}


template<size_t Q = Xt>
constexpr typename std::enable_if<comp_if<Q, 3>::value, SOA_TYPE>::type
Cross(const SOA_TYPE& o) const
{
	SOA_TYPE ret;

	for (size_t i = 0; i < Yt; i++) {
		ret[0][i] = v[1][i] * o[2][i] - v[2][i] * o[1][i];
		ret[1][i] = v[2][i] * o[0][i] - v[0][i] * o[2][i];
		ret[2][i] = v[0][i] * o[1][i] - v[1][i] * o[0][i];
	}
	return ret;
}



inline auto Rotate() const
{
	vecSoa<T, Y, X> ret;

	for (size_t i = 0; i < X; i++)
		for (size_t o = 0; o < Y; o++)
			ret[o][i] = v[i][o];

	return ret;
}

constexpr auto Min(const SOA_TYPE& ov)
{
	SOA_TYPE ret;

	for (size_t i = 0; i < Xt; i++)
		for (size_t o = 0; o < Yt; o++)
			ret[i][o] = ::Min(v[i][o], ov[i][o]);

	return ret;
}

constexpr auto Max(const SOA_TYPE& ov)
{
	SOA_TYPE ret;

	for (size_t i = 0; i < Xt; i++)
		for (size_t o = 0; o < Yt; o++)
			ret[i][o] = ::Min(v[i][o], ov[i][o]);

	return ret;
}

constexpr auto MinUp()
{
	vecb<T, Xt> ret(std::numeric_limits<T>::max());

	for (size_t i = 0; i < Xt; i++)
		for(size_t o = 0; o < Yt; o++)
			ret[i] = ::Min(ret[i], v[i][o]);

	return ret;
}

constexpr auto MaxUp()
{
	vecb<T, Xt> ret(std::numeric_limits<T>::min());

	for (size_t i = 0; i < Xt; i++)
		for(size_t o = 0; o < Yt; o++)
			ret[i] = ::Max(ret[i], v[i][o]);

	return ret;
}

constexpr auto Lerp(const SOA_TYPE& ov, float time)
{
	return *this + time * (ov - *this);
}

constexpr auto LerpClamped(const SOA_TYPE& ov, float time)
{
	return *this + Min(1.f, Max(0.f, time)) * (ov - *this);
}

#undef SOA_TYPE
#endif
