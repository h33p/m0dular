#ifndef MATRIX_H
#define MATRIX_H

#include "vector.h"

template<size_t X, size_t Y>
struct matrix
{
	vecSoa<float, X, Y> vec;

	template <size_t X2, size_t Y2>
	inline auto& operator =(matrix<X2, Y2>& ov)
	{
		constexpr size_t MX = X2 < X ? X2 : X;
		constexpr size_t MY = Y2 < Y ? Y2 : Y;
		for (size_t i = 0; i < MX; i++)
			for (size_t o = 0; o < MY; o++)
				vec[i][o] = ov.vec[i][o];
		return *this;
	}

	inline auto Inverse()
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

	inline auto InverseTranspose()
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
	inline typename std::enable_if<!comp_if<Xt, 4>::value, T>::type Vector3Transform(T& inp)
	{
		T out;

		for (size_t i = 0; i < 3; i++)
			out[i] = inp.Dot(vec[i]) + vec[i][3];

		return out;
	}

	template<typename T, size_t Xt = X>
	inline typename std::enable_if<comp_if<Xt, 4>::value, T>::type Vector3Transform(T& inp)
	{
		T out;

		for (size_t i = 0; i < 3; i++)
			out[i] = inp.Dot(vec[i]) + vec[i][3];

		float w = inp.Dot(vec[3]) + vec[3][3];
		w = (w <= 0 ? std::numeric_limits<float>::infinity() : 1.f / w);
		return out * w;
	}

	template<typename T>
	inline auto Vector3ITransform(T inp)
	{
		T out;

		auto vecRot = vec.Rotate();
		inp -= vecRot[3];

		for (size_t i = 0; i < 3; i++)
			out[i] = inp.Dot(vecRot[i]);

		return out;
	}

	template<typename T, size_t Xt = X>
	inline typename std::enable_if<!comp_if<Xt, 4>::value, T>::type VecSoaTransform(T& inp)
	{
		T out;

		for (size_t i = 0; i < 3; i++)
			for (size_t o = 0; o < inp.Yt; o++)
				out[i][o] = ((vecp<float, 3>)inp.acc[o]).Dot(vec[i]) + vec[i][3];

		return out;
	}

	template<typename T, size_t Xt = X>
	inline typename std::enable_if<comp_if<Xt, 4>::value, T>::type VecSoaTransform(T& inp)
	{
		T out;
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
	inline auto VectorSoaITransform(T& inp)
	{
		T out;
		T temp = inp - (vecp<float, 3>)vec.acc[3];

		for (size_t i = 0; i < 3; i++)
			for (size_t o = 0; o < inp.Yt; o++)
				out[i][o] = ((vecp<float, 3>)temp.acc[o]).Dot(vec[i]) + vec[i][3];

		return out;
	}

	template<typename T, size_t N = T::Yt>
	inline auto WorldToScreen(T& vec, vecb<float, 2>& screen, bool flags[N])
	{
		auto out = VecSoaTransform(vec);

		constexpr int MX = 2 < T::Xt ? 2 : T::Xt;

		for (int o = 0; o < T::Yt; o++)
			flags[o] = true;

		for (int i = 0; i < MX; i++)
			for (int o = 0; o < T::Yt; o++)
				if (out[i][o] > screen[i])
					flags[o] = false;

		for (int o = 0; o < T::Yt; o++)
			if (flags[o]) {
				out[0][o] = screen[0] * 0.5f + out[0][o] * (screen[0] * 0.5f);
				out[1][o] = screen[1] * 0.5f - out[1][o] * (screen[1] * 0.5f);
			}

		return out;
	}

	template<typename T>
	inline auto WorldToScreen(T& vec, vecb<float, 2> screen, bool& status)
	{
		auto out = Vector3Transform(vec);
		if(out[0] <= screen[0] && out[1] >= screen[1])
		{
			out[0] = screen[0] * 0.5f + out[0] * screen[0] * 0.5f;
			out[1] = screen[1] * 0.5f - out[1] * screen[1] * 0.5f;

			status = true;

			return out;
	  }

		status = false;

		return out;
	}

	inline float* operator[](int idx)
	{
		return vec[idx];
	}
};

typedef matrix<4,4> matrix4x4;
#endif
