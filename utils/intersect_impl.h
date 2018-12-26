#ifndef INTERSECT_IMPL_H
#define INTERSECT_IMPL_H

/*
  Template implementations of SOA intersection functions.
  These can not be inside a source file since templates then will not be created.
*/

#include "intersect.h"

static inline float clampf(float a, float min, float max)
{
	return fminf(max, fmaxf(min, a));
}

template<size_t Y>
static vec3soa<float, Y> DirBetweenLines(const vec3soa<float, Y>& a, const vec3soa<float, Y>& b, const vec3soa<float, Y>& c, const vec3soa<float, Y>& d)
{
	auto d1 = (b - a);
	auto d2 = (d - c);

	auto cross = d1.Cross(d2);

	auto cross1 = d1.Cross(cross);
	auto cross2 = d2.Cross(cross);

	float dirDotC2[Y], d1DotC2[Y], dirDotC1[Y], d2DotC1[Y];
	(c - a).Dot(cross2, dirDotC2);
	d1.Dot(cross2, d1DotC2);
	(a - c).Dot(cross1, dirDotC1);
	d2.Dot(cross1, d2DotC1);

	float c2Div[Y], c1Div[Y];

	for (size_t i = 0; i < Y; i++) {
		c2Div[i] = clampf(dirDotC2[i] / d1DotC2[i], 0, 1);
		c1Div[i] = clampf(dirDotC1[i] / d2DotC1[i], 0, 1);
	}

	auto sp = c + d2 * c1Div;
	auto ep = a + d1 * c2Div;

	auto diff = ep - sp;

	return diff;
}

template<size_t Y>
unsigned int CapsuleCollider::IntersectSOA(vec3soa<float, Y>& __restrict a, vec3soa<float, Y>& __restrict b, vec3soa<float, Y>* __restrict out)
{
	unsigned int flags = 0;
    svec3<Y> soaStart, soaEnd;
	soaStart = start;
	soaEnd = end;
	float radiusSqr = radius * radius;

	vec3soa<float, Y> dirs = DirBetweenLines(a, b, soaStart, soaEnd);

	if (out)
		*out = dirs;

	float lens[Y];
	dirs.LengthSqr(lens);

	for (size_t i = 0; i < Y; i++)
		if (lens[i] <= radiusSqr)
			flags |= (1 << i);

	return flags;
}


template unsigned int CapsuleCollider::IntersectSOA(nvec3& __restrict a, nvec3& __restrict b, nvec3* __restrict out);

template <size_t N>
unsigned int CapsuleColliderSOA<N>::Intersect(vec3_t a, vec3_t b, svec3<N>* out)
{
	unsigned int flags = 0;
	svec3<N> va = a, vb = b;

	svec3<N> dirs = DirBetweenLines(va, vb, start, end);

	if (out)
		*out = dirs;

	float lens[N];
	dirs.LengthSqr(lens);

	for (size_t i = 0; i < N; i++)
		if (lens[i] <= radius[i] * radius[i])
			flags |= (1 << i);

	return flags;
}

#endif
