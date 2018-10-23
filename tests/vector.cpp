#include <stdio.h>
#include "../math/vector.h"
#include <stdlib.h>

int main()
{
	srand(time(nullptr));

	vec3_t b(1, 2);
	vec3_t arr[100], a;

	printf("%f %f %f\n%f %f %f\n", a[0], a[1], a[2], b[0], b[1], b[2]);

	for (int i = 0; i < 100; i++) {
		arr[i] = vec3_t(rand() % 100, rand() % 100, rand() % 100, rand() % 100);
		printf("%f %f %f\n", arr[i][0], arr[i][1], arr[i][2]);
	}

	return 0;
}

constexpr vec3_t v(1);
constexpr vec3_t v2(2);

static_assert(v[0] == 1);
static_assert((v * v2)[0] == 2);

constexpr vec3soa<float, 8> vs(1, 2);
static_assert(vs[0][0] == 1 && vs[1][0] == 1);
static_assert(vs[0][1] == 2 && vs[1][1] == 2);
static_assert(vs[0][2] == 1 && vs[1][2] == 1);
