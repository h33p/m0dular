#ifndef INTERSECT_BOX_IMPL_H
#define INTERSECT_BOX_IMPL_H

#include "intersect_box.h"

template<size_t Y, typename ClassSource, typename Source, typename Dest>
[[gnu::flatten]] static inline uint64_t PerformIntersect(const ClassSource& __restrict start, const ClassSource& __restrict end, const Source& __restrict a, const Source& __restrict b, Dest* __restrict out)
{
	Dest d = b - a;
	auto dInv = 1.f / d;

	auto t1 = (start - a) * dInv;
	auto t2 = (end - a) * dInv;

	auto minv = t1.Min(t2);
	auto maxv = t1.Max(t2);

	auto tmin = minv.MaxUp();
	auto tmax = maxv.MinUp();

	uint64_t ret = 0;

	for (size_t i = 0; i < sizeof(tmin) / sizeof(tmin[0]); i++)
		if (tmax[i] >= fmaxf(0.f, tmin[i]) && tmin[i] < 1)
			ret |= (1ull << i);

	if (out) {
		for (size_t i = 0; i < sizeof(tmin) / sizeof(tmin[0]); i++)
			tmin[i] = fminf(tmin[i], 0);
		*out = d * tmin + a;
	}

	return ret;
}

template<size_t Y>
uint64_t AABBCollider::IntersectSOA(const vec3soa<float, Y>& __restrict a, const vec3soa<float, Y>& __restrict b, vec3soa<float, Y>* __restrict out)
{
	return PerformIntersect<Y>(start, end, a, b, out);
}

template<size_t Y>
uint64_t AABBColliderSOA<Y>::Intersect(vec3_t a, vec3_t b, vec3soa<float, Y>* out)
{
	return PerformIntersect<Y>(start, end, a, b, out);
}

template<size_t Y>
uint64_t AABBColliderSOA<Y>::IntersectSSOA(const vec3soa<float, Y>& __restrict a, const vec3soa<float, Y>& __restrict b, vec3soa<float, Y>* out)
{
	return PerformIntersect<Y>(start, end, a, b, out);
}

#endif
