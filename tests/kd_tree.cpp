#include <stdio.h>
#include <math.h>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include "../utils/kd_tree.h"
#include "../utils/freelistallocator.h"
#include "../utils/allocwraps.h"
#include <algorithm>

constexpr int DIMS = 150;
constexpr int ALLOC_COUNT = 700;

template<typename T>
struct KDPoint
{
	T pt[2];
	T depth;

	constexpr KDPoint() : pt(), depth(0)
	{

	}

	constexpr KDPoint(T x, T y) : pt{x, y}, depth(0)
	{

	}

	constexpr KDPoint(T x, T y, T d) : pt{x, y}, depth(d)
	{

	}

	inline const T& operator[](int idx) const
	{
		return pt[idx];
	}

	inline bool operator==(const KDPoint& o) const
	{
		return pt[0] == o[0] && pt[1] == o[1];
	}

	inline bool operator!=(const KDPoint& o) const
	{
		return !(*this == o);
	}
};

uintptr_t allocBase = 0;
generic_free_list_allocator<allocBase, true> alloc(3200, PlacementPolicy::FIND_FIRST);
KDTree<KDPoint<int>, 2, stateful_allocator<TreeNode_t<KDPoint<int>>, alloc>> tree;
std::vector<KDPoint<int>> testData;

int main()
{
	srand(time(nullptr));

	int status = 0;

	for (size_t i = 0; i < 3; i++) {
		printf("Clearing...\n");
		tree.Clear();
		testData.clear();

		for (int i = 0; i < ALLOC_COUNT; i++) {
			auto ref = tree.Insert(KDPoint<int>(rand() % DIMS, rand() % DIMS));
			testData.push_back(**ref);
		}

		for (int i = 0; i < DIMS; i++) {

			bool dirty = false;

			for (int o = 0; o < DIMS; o++) {
				KDPoint<int> pt(i, o);
				auto ref = tree.Find(pt);

				if (ref) {
					auto iter = std::find(testData.begin(), testData.end(), pt);
					if (iter != testData.end())
						;//putchar('#');
					else {
						putchar('X');
						dirty = true;
						status++;
					}
				} else {
					auto iter = std::find(testData.begin(), testData.end(), pt);
					if (iter == testData.end())
						;//putchar('.');
					else {
						putchar('x');
						dirty = true;
						status++;
					}
				}
			}
			if (dirty)
				putchar('\n');
		}
	}

	return status;
}
