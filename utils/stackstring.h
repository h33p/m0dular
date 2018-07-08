#ifndef STACK_STRING
#define STACK_STRING

typedef unsigned long stt;
typedef stt rstt;

#ifdef _WIN32
#define __alwaysinline __forceinline
#else
#define __alwaysinline __inline __attribute__((always_inline))
#endif

template <int N>
__alwaysinline
constexpr int unroll_read(rstt* a, rstt* b)
{
	a[N - 1] = b[N - 1];
	int ret = unroll_read<N - 1>(a, b);
	return ret;
};

template<>
__alwaysinline
constexpr int unroll_read<0>(rstt* a, rstt* b)
{
	return 1;
};


template<size_t slen>
struct StackString
{

	static constexpr int len = slen / sizeof(stt) + 1;

	volatile stt stack[len];

	//We have to always inline the functions for the trick to work
	__alwaysinline
	constexpr StackString(const char (&Array)[slen])
	{
		unroll_read<len-1>((rstt*)stack, (rstt*)Array);
		for (int i = (len - 1) * sizeof(rstt); i < slen; i++)
			((char*)stack)[i] = Array[i];
	}

	__alwaysinline
	const char* val() const
	{
		return (char*)stack;
	}

	inline operator char*()
	{
		return (char*)val();
	}

	inline operator const char*() const
	{
		return val();
	}

	inline operator const char*()
	{
		return val();
	}
};

#endif
