#include "intersect_impl.h"

static vec3_t DirBetweenLines(vec3_t a, vec3_t b, vec3_t c, vec3_t d)
{
	vec3_t d1 = (b - a);
	vec3_t d2 = (d - c);

	vec3_t cross = d1.Cross(d2);

	vec3_t cross1 = d1.Cross(cross);
	vec3_t cross2 = d2.Cross(cross);

	vec3_t sp = c + std::clamp((a - c).Dot(cross1) / (d2.Dot(cross1)), 0.f, 1.f) * d2;
	vec3_t ep = a + std::clamp((c - a).Dot(cross2) / (d1.Dot(cross2)), 0.f, 1.f) * d1;

	return ep - sp;
}

bool CapsuleCollider::Intersect(vec3_t a, vec3_t b)
{
	vec3_t dir = DirBetweenLines(a, b, start, end);

	return dir.LengthSqr() <= radius * radius;
}
