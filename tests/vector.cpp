#include <stdio.h>
#include "../math/vector.h"
#include "../math/matrix.h"
#include <stdlib.h>
#include <time.h>

constexpr vec3_t rotation = vec3_t(0, M_PI / 2, 0);
constexpr matrix<3, 4> rotationMatrix = matrix<3, 4>::GetMatrix(rotation);
constexpr vec3_t rotatedVec = rotationMatrix.Vector3Rotate(vec3_t(1, 0, 0));
constexpr vec3_t expectedVec = vec3_t(0, 1, 0);
static_assert(rotatedVec.DistToSqr(expectedVec) < 0.1);

int exit_error(int i)
{
	fprintf(stderr, "%d\n", i);
	return i;
}

int main()
{
	srand(time(nullptr));

	vec3_t a(1), b(1, 2);
	[[maybe_unused]] vec3soa<float, 8> stype = b;
	[[maybe_unused]] vec3 v3 = b;
	[[maybe_unused]] vec3_t d = {3.f, 3.f, 3.f};

	printf("%f %f %f\n%f %f %f\n", a[0], a[1], a[2], b[0], b[1], b[2]);

	{
		vec3_t arr[100];
		for (int i = 0; i < 100; i++)
			arr[i] = vec3_t(rand() % 100 + 1, rand() % 100 + 1, rand() % 100 + 1, rand() % 100 + 1);

		{
			vec3_t combined[50];

			for (int i = 0; i < 50; i++)
				combined[i] = arr[i] + arr[i + 50];

			vec3_t back[100];

			for (int i = 0; i < 50; i++)
				back[i] = combined[i] - arr[i + 50];

			for (int i = 0; i < 50; i++)
				back[i + 50] = combined[i] - arr[i];

			for (int i = 0; i < 100; i++)
				if (arr[i] != back[i])
					return exit_error(1000 + i);
		}

		printf("Vec test 1 passed!\n");

		{
			vec3_t combined[50];

			for (int i = 0; i < 50; i++)
				combined[i] = arr[i] * arr[i + 50];

			vec3_t back[100];

			for (int i = 0; i < 50; i++)
				back[i] = combined[i] / arr[i + 50];

			for (int i = 0; i < 50; i++)
				back[i + 50] = combined[i] / arr[i];

			for (int i = 0; i < 100; i++)
				if ((arr[i] - back[i]).LengthSqr() > 0.01f) {
					printf("Mismatch! %f %f %f | %f %f %f\n", arr[i][0], arr[i][1], arr[i][2], back[i][0], back[i][1], back[i][2]);
					return exit_error(2000 + i);
				}
		}

		printf("Vec test 2 passed!\n");

		{
			vec3_t combined[100];

			for (int i = 0; i < 100; i++)
				combined[i] = arr[i] * arr[i];

			vec3_t back[100];

			for (int i = 0; i < 100; i++)
				back[i] = combined[i].Sqrt();

			for (int i = 0; i < 100; i++)
				if ((arr[i] - back[i]).LengthSqr() > 0.01f)
					return exit_error(3000 + i);
		}

		printf("Vec test 3 passed!\n");
	}

	{
		vec3soa<float, 8> arr[100];
		for (int i = 0; i < 100; i++)
			for (int u = 0; u < 3; u++)
				for (int o = 0; o < 8; o++)
					arr[i][u][o] = rand() % 100 + 1;

		{
			vec3soa<float, 8> combined[50];

			for (int i = 0; i < 50; i++)
				combined[i] = arr[i] + arr[i + 50];

			vec3soa<float, 8> back[100];

			for (int i = 0; i < 50; i++)
				back[i] = combined[i] - arr[i + 50];

			for (int i = 0; i < 50; i++)
				back[i + 50] = combined[i] - arr[i];

			for (int i = 0; i < 100; i++)
				if ((arr[i] - back[i]).Abs().AddedUpTotal() > 0.01f)
					return exit_error(10000 + i);
		}

		printf("VecSoa test 1 passed!\n");

		{
			vec3soa<float, 8> combined[50];

			for (int i = 0; i < 50; i++)
				combined[i] = arr[i] * arr[i + 50];

			vec3soa<float, 8> back[100];

			for (int i = 0; i < 50; i++)
				back[i] = combined[i] / arr[i + 50];

			for (int i = 0; i < 50; i++)
				back[i + 50] = combined[i] / arr[i];

			for (int i = 0; i < 100; i++)
				if ((arr[i] - back[i]).Abs().AddedUpTotal() > 0.01f)
					return exit_error(20000 + i);
		}

		printf("VecSoa test 2 passed!\n");

		{
			vec3soa<float, 8> combined[100];

			for (int i = 0; i < 100; i++)
				combined[i] = arr[i] * arr[i];

			printf("Part 1\n");

			vec3soa<float, 8> back[100];

			for (int i = 0; i < 100; i++)
				back[i] = combined[i].Sqrt();

			printf("Part 2\n");

			for (int i = 0; i < 100; i++)
				if ((arr[i] - back[i]).Abs().AddedUpTotal() > 0.01f)
					return exit_error(30000 + i);
		}

		printf("VecSoa test 3 passed!\n");
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

static_assert(!AllArithmetic<vecp<float, 3>, vecp<float, 3>>::value);
static_assert(AllArithmetic<int, float, double, long>::value);
static_assert(!AllArithmetic<int, float, char*>::value);
