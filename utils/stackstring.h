#ifndef STACK_STRING
#define STACK_STRING

#include <stddef.h>

#define ST(x) StackString(x)

typedef unsigned char stt;
typedef stt rstt;

#ifdef _WIN32
#define __alwaysinline __forceinline
#else
#define __alwaysinline __inline __attribute__((always_inline))
#endif

template <int N>
__alwaysinline
constexpr int unroll_read(rstt* a, rstt* b, int FN)
{
	a[N - 1] = b[FN - N];
	int ret = unroll_read<N - 1>(a, b, FN);
	return ret;
};

template<>
__alwaysinline
constexpr int unroll_read<0>(rstt* a, rstt* b, int FN)
{
	return 1;
};


template<size_t slen>
struct StackString
{

	static constexpr int len = slen / sizeof(stt) + 1;
	static constexpr int len2 = slen / sizeof(stt);

	char stack[len * sizeof(stt)];
	volatile stt stack2[len];
	volatile stt stack2s[len/2];
	volatile stt stack2e[len2 - len/2];

	//We have to always inline the functions for the trick to work
	__alwaysinline
	constexpr StackString(const char (&Array)[slen])
	{

		unroll_read<len2 - len / 2>((rstt*)stack2e, (rstt*)Array + len / 2, len2 - len / 2);
		unroll_read<len / 2>((rstt*)stack2s, (rstt*)Array, len / 2);

		for (int i = (len - 1) * sizeof(rstt); i < (int)slen; i++)
			((char*)stack2)[i] = Array[i];

		unroll_read<len2 - len / 2>((rstt*)stack + len / 2, (rstt*)stack2e, len2 - len / 2);
		unroll_read<len / 2>((rstt*)stack, (rstt*)stack2s, len / 2);

		for (int i = (len - 1) * sizeof(rstt); i < (int)slen; i++)
			stack[i] = ((char*)stack2)[i];

	}

	__alwaysinline
	constexpr const char* val() const
	{
		return stack;
	}

	inline operator char*()
	{
		return (char*)val();
	}

	constexpr operator const char*() const
	{
		return val();
	}

	constexpr operator const char*()
	{
		return val();
	}
};

#endif
