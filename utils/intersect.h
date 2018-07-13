#ifndef INTERSECT_H
#define INTERSECT_H

#include "../math/mmath.h"

struct CapsuleCollider
{
	vec3_t start;
	vec3_t end;
	float radius;

	bool Intersect(vec3_t a, vec3_t b);

	template<size_t Y>
	unsigned int IntersectSOA(vec3soa<float, Y>& __restrict a, vec3soa<float, Y>& __restrict);
};

template <size_t N>
struct CapsuleColliderSOA
{
	svec3<N> start, end;
	float radius[N];

	unsigned int Intersect(vec3_t a, vec3_t b);
};

#endif
