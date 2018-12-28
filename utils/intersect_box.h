#ifndef INTERSECT_BOX_H
#define INTERSECT_BOX_H

#include "../math/mmath.h"

struct AABBCollider
{
	vec3_t start, end;

	AABBCollider(vec3_t s, vec3_t e)
		: start(s), end(e) {}

	bool Intersect(vec3_t a, vec3_t b, vec3_t* out = nullptr);
	template<size_t Y>
	uint64_t IntersectSOA(const vec3soa<float, Y>& __restrict a, const vec3soa<float, Y>& __restrict b, vec3soa<float, Y>* __restrict out = nullptr);
};

struct OBBCollider
	: AABBCollider
{
	matrix<3, 4> w2l;

	OBBCollider(vec3_t s, vec3_t e, matrix<3, 4> m)
		: AABBCollider(s, e), w2l(m) {}

	inline bool Intersect(vec3_t a, vec3_t b, vec3_t* out = nullptr)
	{
		return AABBCollider::Intersect(w2l.Vector3Transform(a), w2l.Vector3Transform(b), out);
	}
};

template<size_t Y>
struct AABBColliderSOA
{
	vec3soa<float, Y> start, end;

	AABBColliderSOA(vec3soa<float, Y> s, vec3soa<float, Y> e)
		: start(s), end(e) {}

	uint64_t Intersect(vec3_t a, vec3_t b, vec3soa<float, Y>* out = nullptr);
	uint64_t IntersectSSOA(const vec3soa<float, Y>& __restrict a, const vec3soa<float, Y>& __restrict b, vec3soa<float, Y>* out = nullptr);
};

#endif
