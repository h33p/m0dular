#include "intersect_box.h"

bool AABBCollider::Intersect(vec3_t a, vec3_t b, vec3_t* out)
{
	vec3_t d = b - a;
	vec3_t dInv = 1.f / d;

	vec3_t t1 = (start - a) * dInv;
	vec3_t t2 = (end - a) * dInv;

	vec3_t minv = t1.Min(t2);
	vec3_t maxv = t1.Max(t2);

	float tmin = minv.MaxUp();
	float tmax = maxv.MinUp();

	bool ret = tmax >= fmaxf(0.f, tmin) && tmin < 1;

	if (out)
		*out = a + d * fminf(tmin, 1);

	return ret;
}
