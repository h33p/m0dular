#ifndef MATRIX_H
#define MATRIX_H

#include "vector.h"

template<size_t X, size_t Y>
struct matrix
{
	vecSoa<float, X, Y> vec;

	template <size_t X2, size_t Y2>
	inline auto& operator=(const matrix<X2, Y2>& ov)
	{
		constexpr size_t MX = X2 < X ? X2 : X;
		constexpr size_t MY = Y2 < Y ? Y2 : Y;
		for (size_t i = 0; i < MX; i++)
			for (size_t o = 0; o < MY; o++)
				vec[i][o] = ov.vec[i][o];
		return *this;
	}

	template <size_t X2, size_t Y2>
	inline auto operator*(const matrix<X2, Y2>& ov)
	{
		constexpr size_t MinX = Y2 < X ? Y2 : X;
		constexpr size_t MinY = X2 < Y ? X2 : Y;
		constexpr size_t CompS = MinX < MinY ? MinX : MinY;

		constexpr size_t MX = X2 < X ? X2 : X;
		constexpr size_t MY = Y2 < Y ? Y2 : Y;

		constexpr size_t SX = MX < MinX ? MinX : MX;
		constexpr size_t SY = MY < MinY ? MinY : MY;

		matrix<SX, SY> result;

		for (size_t i = 0; i < MinX; i++)
			for (size_t o = 0; o < MinY; o++)
				result[i][o] = 0;

		for (size_t i = 0; i < MinX; i++)
			for (size_t o = 0; o < MinY; o++) {
				for (size_t u = 0; u < CompS; u++)
					result[i][o] += vec[i][u] * ov[u][o];
			}
			//vec.template Dot<vecb<float, CompS>, CompS, CompS>(ov.vec.template ColumnVec<CompS>(i), result[i]);

		//Copy over the remainding data that was not be multiplied
		for (size_t i = CompS; i < SX; i++)
			for (size_t o = 0; o < SY; o++)
				result[i][o] = vec[i][o];

		for (size_t i = 0; i < SX; i++)
			for (size_t o = CompS; o < SY; o++)
				result[i][o] = vec[i][o];

		return result;
	}

	template <size_t X2, size_t Y2>
	inline auto& operator*=(const matrix<X2, Y2>& ov)
	{
		*this = *this * ov;
		return *this;
	}

	template<typename T>
	static constexpr auto GetMatrix(const T& angles, bool fromDegrees = false)
	{
		matrix<X, Y> vec = {vecSoa<float, X, Y>()};

		const int VP = 0;
		const int VY = 1;
		const int VR = 2;

		float s[3] = {0}, c[3] = {0};

		auto it = angles;
		if (fromDegrees)
			it *= DEG2RAD;

		for (size_t i = 0; i < 3; i++)
			s[i] = ConstSin(it[i]);

		for (size_t i = 0; i < 3; i++)
			c[i] = ConstCos(it[i]);

		vec[0][0] = c[VP] * c[VY];
		vec[1][0] = c[VP] * s[VY];
		vec[2][0] = -s[VP];

		vec[0][1] = s[VR] * s[VP] * c[VY] + c[VR] * s[VY];
		vec[1][1] = s[VR] * s[VP] * s[VY] - c[VR] * c[VY];
		vec[2][1] = s[VR] * c[VP];

		vec[0][2] = c[VR] * s[VP] * c[VY] + s[VR] * s[VY];
		vec[1][2] = c[VR] * s[VP] * s[VY] - s[VR] * c[VY];
		vec[2][2] = c[VR] * c[VP];

		return vec;
	}

	inline vec3_t GetAngles(bool toDegrees = false)
	{
		vec3_t fwd = (vec3_t)vec.acc[0];
		vec3_t left = (vec3_t)vec.acc[1];
		vec3_t up = (vec3_t)vec.acc[2];
		vec3_t ret(0);

		float xyLen = fwd.Length<2>();

		if (xyLen > 0.001f) {
			ret[0] = atan2f(-fwd[2], xyLen);
			ret[1] = atan2f(fwd[1], fwd[0]);
			ret[2] = atan2f(left[2], up[2]);
		} else {
			ret[0] = atan2f(-fwd[2], xyLen);
			ret[1] = atan2f(-left[0], left[1]);
			ret[2] = 0;
		}

		return toDegrees ? ret * RAD2DEG : ret;
	}

	inline auto Inverse() const
	{
		auto ret = *this;

		float det = vec[0][0] * (vec[1][1] * vec[2][2] - vec[2][1] * vec[1][2]) -
								vec[0][1] * (vec[1][0] * vec[2][2] - vec[1][2] * vec[2][0]) +
								vec[0][2] * (vec[1][0] * vec[2][1] - vec[1][1] * vec[2][0]);

		float invDet = 1.f / det;

		ret[0][0] = (vec[1][1] * vec[2][2] - vec[2][1] * vec[1][2]) * invDet;
		ret[0][1] = (vec[0][2] * vec[2][1] - vec[0][1] * vec[2][2]) * invDet;
		ret[0][2] = (vec[0][1] * vec[1][2] - vec[0][2] * vec[1][1]) * invDet;
		ret[1][0] = (vec[1][2] * vec[2][0] - vec[1][0] * vec[2][2]) * invDet;
		ret[1][1] = (vec[0][0] * vec[2][2] - vec[0][2] * vec[2][0]) * invDet;
		ret[1][2] = (vec[1][0] * vec[0][2] - vec[0][0] * vec[1][2]) * invDet;
		ret[2][0] = (vec[1][0] * vec[2][1] - vec[2][0] * vec[1][1]) * invDet;
		ret[2][1] = (vec[2][0] * vec[0][1] - vec[0][0] * vec[2][1]) * invDet;
		ret[2][2] = (vec[0][0] * vec[1][1] - vec[1][0] * vec[0][1]) * invDet;

		return ret;
	}

	inline auto InverseTranspose() const
	{
		auto ret = *this;

		float det = vec[0][0] * (vec[1][1] * vec[2][2] - vec[2][1] * vec[1][2]) -
								vec[0][1] * (vec[1][0] * vec[2][2] - vec[1][2] * vec[2][0]) +
								vec[0][2] * (vec[1][0] * vec[2][1] - vec[1][1] * vec[2][0]);

		float invDet = 1.f / det;

		ret[0][0] = (vec[1][1] * vec[2][2] - vec[2][1] * vec[1][2]) * invDet;
		ret[0][1] = (vec[1][2] * vec[2][0] - vec[1][0] * vec[2][2]) * invDet;
		ret[0][2] = (vec[1][0] * vec[2][1] - vec[2][0] * vec[1][1]) * invDet;
		ret[1][0] = (vec[0][2] * vec[2][1] - vec[0][1] * vec[2][2]) * invDet;
		ret[1][1] = (vec[0][0] * vec[2][2] - vec[0][2] * vec[2][0]) * invDet;
		ret[1][2] = (vec[2][0] * vec[0][1] - vec[0][0] * vec[2][1]) * invDet;
		ret[2][0] = (vec[0][1] * vec[1][2] - vec[0][2] * vec[1][1]) * invDet;
		ret[2][1] = (vec[1][0] * vec[0][2] - vec[0][0] * vec[1][2]) * invDet;
		ret[2][2] = (vec[0][0] * vec[1][1] - vec[1][0] * vec[0][1]) * invDet;

		return ret;
	}

	template<typename T, size_t Xt = X>
	constexpr typename std::enable_if<!comp_if<Xt, 4>::value, T>::type Vector3Transform(const T& inp) const
	{
		T out(0);

		for (size_t i = 0; i < 3; i++)
			out[i] = inp.Dot(vec[i]) + vec[i][3];

		return out;
	}

	template<typename T, size_t Xt = X>
	constexpr typename std::enable_if<comp_if<Xt, 4>::value, T>::type Vector3Transform(const T& inp) const
	{
		T out(0);

		for (size_t i = 0; i < 3; i++)
			out[i] = inp.Dot(vec[i]) + vec[i][3];

		float w = inp.Dot(vec[3]) + vec[3][3];
		w = (w <= 0 ? std::numeric_limits<float>::infinity() : 1.f / w);
		return out * w;
	}

	template<typename T>
	constexpr auto Vector3ITransform(T inp) const
	{
		T out(0);

		auto vecRot = vec.Rotate();
		inp -= vecRot[3];

		for (size_t i = 0; i < 3; i++)
			out[i] = inp.Dot(vecRot[i]);

		return out;
	}

	template<typename T>
	constexpr T Vector3Rotate(const T& inp) const
	{
		T out(0);

		for (size_t i = 0; i < 3; i++)
			out[i] = inp.Dot(vec[i]);

		return out;
	}

	template<typename T>
	constexpr T Vector3IRotate(const T& inp) const
	{
		T out(0);

		auto vecRot = vec.Rotate();

		for (size_t i = 0; i < 3; i++)
			out[i] = inp.Dot(vecRot[i]);

		return out;
	}

	template<typename T, size_t Xt = X>
	constexpr typename std::enable_if<!comp_if<Xt, 4>::value, T>::type VecSoaTransform(const T& inp) const
	{
		T out(0);

		for (size_t i = 0; i < 3; i++)
			for (size_t o = 0; o < inp.Yt; o++)
				out[i][o] = ((vecp<float, 3>)inp.acc[o]).Dot(vec[i]) + vec[i][3];

		return out;
	}

	template<typename T, size_t Xt = X>
	constexpr typename std::enable_if<comp_if<Xt, 4>::value, T>::type VecSoaTransform(const T& inp) const
	{
		T out(0);
		float w[inp.Yt];

		for (size_t i = 0; i < 3; i++)
			for (size_t o = 0; o < inp.Yt; o++)
				out[i][o] = ((vecp<float, 3>)inp.acc[o]).Dot(vec[i]) + vec[i][3];

		for (size_t i = 0; i < inp.Yt; i++) {
			w[i] = ((vecp<float, 3>)inp.acc[i]).Dot(vec[3]) + vec[3][3];
			w[i] = (w[i] <= 0 ? std::numeric_limits<float>::infinity() : 1.f / w[i]);
		}

		for (size_t i = 0; i < inp.Xt; i++)
			for (size_t o = 0; o < inp.Yt; o++)
				out[i][o] *= w[o];

		return out;
	}

	template<typename T>
	constexpr auto VectorSoaITransform(const T& inp) const
	{
		T out(0);
		T temp = inp - (vecp<float, 3>)vec.acc[3];

		for (size_t i = 0; i < 3; i++)
			for (size_t o = 0; o < inp.Yt; o++)
				out[i][o] = ((vecp<float, 3>)temp.acc[o]).Dot(vec[i]) + vec[i][3];

		return out;
	}

	template<typename T, size_t N = T::Yt>
	constexpr auto WorldToScreen(const T& vec, const vecb<float, 2>& screen, bool* flags) const
	{
		auto out = VecSoaTransform(vec);

		constexpr size_t MX = 2 < T::Xt ? 2 : T::Xt;

		for (size_t o = 0; o < T::Yt; o++)
			flags[o] = true;

		for (size_t i = 0; i < MX; i++)
			for (size_t o = 0; o < T::Yt; o++)
				if (out[i][o] > screen[i])
					flags[o] = false;

		for (size_t o = 0; o < T::Yt; o++)
			if (flags[o]) {
				out[0][o] = screen[0] * 0.5f + out[0][o] * (screen[0] * 0.5f);
				out[1][o] = screen[1] * 0.5f - out[1][o] * (screen[1] * 0.5f);
			}

		return out;
	}

	template<typename T>
	constexpr auto WorldToScreen(const T& vec, const vecb<float, 2>& screen, bool& status) const
	{
		auto out = Vector3Transform(vec);
		if (out[0] <= screen[0] && out[1] <= screen[1]) {
			out[0] = screen[0] * 0.5f + out[0] * screen[0] * 0.5f;
			out[1] = screen[1] * 0.5f - out[1] * screen[1] * 0.5f;

			status = true;

			return out;
		}

		status = false;

		return out;
	}

	constexpr float* operator[](int idx)
	{
		return vec.v[idx];
	}

	constexpr const float* operator[](int idx) const
	{
		return vec.v[idx];
	}
};

typedef matrix<4,4> matrix4x4;
#endif
