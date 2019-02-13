//Be sure to only include in a vector class and have VEC_TYPE defined (it gets automatically undefined)

#ifdef VEC_TYPE

constexpr VEC_TYPE() = default;

template<typename F, typename = typename std::enable_if<AllArithmetic<F>::value>::type>
constexpr VEC_TYPE(F arg) : v()
{
	for (size_t i = 0; i < N; i++)
		v[i] = arg;
}

template<typename... F, typename = typename std::enable_if<AllArithmetic<F...>::value>::type>
constexpr VEC_TYPE(F... args) : v()
{
	constexpr size_t elementCount = sizeof...(args);
	for (size_t i = 0; i < N; i++)
		v[i] = GetElementAt<T>(i % elementCount, args...);
}

inline auto& Assign(T val)
{
	for (size_t i = 0; i < N; i++)
		v[i] = val;
	return *this;
}


template <size_t D>
constexpr T Dot(const VEC_TYPE& o) const
{
	T val = 0;
	for (size_t i = 0; i < D; i++)
		val += v[i] * o.v[i];
	return val;
}

template <size_t D>
constexpr T Dot(const T* o) const
{
	T val = 0;
	for (size_t i = 0; i < D; i++)
		val += v[i] * o[i];
	return val;
}

template <size_t D>
constexpr T LengthSqr() const
{
	return Dot<D>(*this);
}

template <size_t D>
constexpr T Length() const
{
	return sqrt(Dot<D>(*this));
}

template <size_t D>
inline auto& Sqrt()
{
	constexpr size_t Md = D > N ? N : D;
	VSqrt<T, Md>(v);
	return *this;
}

template <size_t D>
inline auto& NormalizeAngles(T start, T end)
{
	for (size_t i = 0; i < D; i++)
		v[i] = std::fmod(std::fmod(v[i] - start, end - start) + (end - start), end - start) + start;
	return *this;
}

constexpr T Dot(const VEC_TYPE& o) const
{
	return Dot<N>(o);
}

constexpr T Dot(const T* o) const
{
	return Dot<N>(o);
}

constexpr T LengthSqr() const
{
	return LengthSqr<N>();
}

constexpr T Length() const
{
	return Length<N>();
}

inline auto& Sqrt()
{
	return Sqrt<N>();
}

inline auto Normalized() const
{
	auto val = *this;
	float l = val.Length();
	val *= l ? 1 / l : 0;
	return val;
}

inline auto& Normalize()
{
	*this = Normalized();
	return *this;
}

template <size_t D>
inline T DistTo(const VEC_TYPE& o) const
{
	return (*this - o).template Length<D>();
}

inline T DistTo(const VEC_TYPE& o) const
{
	return DistTo<N>(o);
}

template <size_t D>
constexpr T DistToSqr(const VEC_TYPE& o) const
{
	return (*this - o).template LengthSqr<D>();
}

constexpr T DistToSqr(const VEC_TYPE& o) const
{
	return DistToSqr<N>(o);
}

inline auto DirToRay(const VEC_TYPE& a, const VEC_TYPE& b) const
{
	auto c = *this - a;
	auto d = b - a;

	T t = c.Dot(d) / d.LengthSqr();

	return a + t * d;
}

inline auto DirToLine(const VEC_TYPE& a, const VEC_TYPE& b) const
{
	auto c = *this - a;
	auto d = b - a;

	T t = std::clamp(c.Dot(d) / d.LengthSqr(), T(0), T(1));

	return a + t * d;
}

constexpr auto GetRight() const
{
	if (v[0] == v[1] == 0)
		return VEC_TYPE(0, -1, 0);
	return this->Cross(VEC_TYPE(0, 0, 1));
}

constexpr auto GetUp() const
{
	if (v[0] == v[1] == 0)
		return VEC_TYPE(-v[2], 0, 0);
	return GetRight().Cross(*this);
}

template<size_t Q = N>
constexpr typename std::enable_if<comp_if<Q, 3>::value, VEC_TYPE<T, 3>>::type
Cross(const VEC_TYPE& o) const
{
	VEC_TYPE<T, 3> ret;
	ret[0] = v[1] * o[2] - v[2] * o[1];
	ret[1] = v[2] * o[0] - v[0] * o[2];
	ret[2] = v[0] * o[1] - v[1] * o[0];
	return ret;
}


template<size_t Q = N>
inline typename std::enable_if<comp_if<Q, 3>::value, VEC_TYPE<T, 3>&>::type
ToAngles()
{
	T y, x, len;
	y = atan2(v[1], v[0]);

	len = Length<2>();

	x = atan2(-v[2], len);

	v[0] = x;
	v[1] = y;
	v[2] = 0;

	return *this;
}

template<size_t Q = N>
inline typename std::enable_if<comp_if<Q, 3>::value, VEC_TYPE<T, 3>>::type
GetAngles(bool toDegrees = false) const
{
	auto ret = *this;
	ret.ToAngles();
	if (toDegrees)
		ret *= RAD2DEG;
	return ret;
}

template<size_t Q = N>
inline typename std::enable_if<comp_if<Q, 3>::value, VEC_TYPE<T, 3>&>::type
GetVectors(VEC_TYPE& __restrict forward, VEC_TYPE& __restrict right, VEC_TYPE& __restrict up, bool fromDegrees = false)
{
	const int VP = 0;
	const int VY = 1;
	const int VR = 2;

	T s[3], c[3];

	auto it = *this;
	if (fromDegrees)
		it *= DEG2RAD;

	for (size_t i = 0; i < 3; i++)
		s[i] = std::sin(it[i]);

	for (size_t i = 0; i < 3; i++)
		c[i] = std::cos(it[i]);

	forward[0] = c[VP] * c[VY];
	forward[1] = c[VP] * s[VY];
	forward[2] = -s[VP];

	right[0] = -s[VR] * s[VP] * c[VY] + c[VR] * s[VY];
	right[1] = -s[VR] * s[VP] * s[VY] - c[VR] * c[VY];
	right[2] = -s[VR] * c[VP];

	up[0] = c[VR] * s[VP] * c[VY] + s[VR] * s[VY];
	up[1] = c[VR] * s[VP] * s[VY] - s[VR] * c[VY];
	up[2] = c[VR] * c[VP];

	return *this;
}

template<size_t dim, size_t Q = N>
inline typename std::enable_if<comp_if<Q, 3>::value, VEC_TYPE<T, 3>&>::type
Rotate(T angle)
{
	T s, c;
	s = std::sin(angle);
	c = std::cos(angle);

	constexpr size_t iX = (dim + 1) % 3;
	constexpr size_t iY = (dim + 2) % 3;

	T xn = v[iX] * c - v[iY] * s;
	T yn = v[iX] * s + v[iY] * c;

	v[iX] = xn;
	v[iY] = yn;

	return *this;
}

constexpr auto Min(const VEC_TYPE& ov)
{
	VEC_TYPE ret;

	for (size_t i = 0; i < N; i++)
		ret[i] = ::Min(v[i], ov[i]);

	return ret;
}

constexpr auto Max(const VEC_TYPE& ov)
{
	VEC_TYPE ret;

	for (size_t i = 0; i < N; i++)
		ret[i] = ::Max(v[i], ov[i]);

	return ret;
}

constexpr auto MinUp()
{
	T ret = std::numeric_limits<T>::max();

	for (size_t i = 0; i < N; i++)
		ret = ::Min(ret, v[i]);

	return ret;
}

constexpr auto MaxUp()
{
	T ret = std::numeric_limits<T>::min();

	for (size_t i = 0; i < N; i++)
		ret = ::Max(ret, v[i]);

	return ret;
}

constexpr auto Lerp(const VEC_TYPE& ov, float time)
{
	return *this + time * (ov - *this);
}

constexpr auto LerpClamped(const VEC_TYPE& ov, float time)
{
	return *this + Min(1.f, Max(0.f, time)) * (ov - *this);
}

#undef VEC_TYPE
#endif
