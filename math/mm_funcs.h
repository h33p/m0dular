#ifndef MM_FUNCS_H
#define MM_FUNCS_H

//In order to template various SIMD operations we have to have templateable types and functions which is not an easy feat to be done manually, thus we use this macro cave to be able to easily expand this later on

template<size_t sz, typename F>
struct m_
{
	struct type {
	};
};

static inline void throwfunc()
{
	throw;
}

template<size_t sz, typename F>
using _m = typename m_<sz, F>::type;

template<auto val>
using EnableIf = typename std::enable_if<val, std::true_type>::type;

#define GEN_FUNCC(RFUNC, FUNC, SIZE, ...)								\
	template<size_t sz, typename F, EnableIf<sz == SIZE && __VA_ARGS__>* = nullptr, typename... Args> constexpr auto RFUNC(Args... args) \
	{																	\
		return FUNC(args...);											\
	}

#define GEN_FUNC(RFUNC, type, FUNC, SIZE) GEN_FUNCC(RFUNC, FUNC, SIZE, std::is_same<type, F>::value)

#define PASTETWO(A, B) A##B

#define GEN_FUNC_SI2(RFUNC, FUNC, SIZE)							\
	GEN_FUNC(RFUNC, float, PASTETWO(FUNC, _ps), SIZE);			\
	GEN_FUNC(RFUNC, double, PASTETWO(FUNC, _pd), SIZE);			\
	GEN_FUNCC(RFUNC, PASTETWO(FUNC, _pd), SIZE, std::is_integral<F>::value); \

#define GEN_FUNC_EPI2(RFUNC, FUNC, SIZE)								\
	GEN_FUNC(RFUNC, float, PASTETWO(FUNC, _ps), SIZE);					\
	GEN_FUNC(RFUNC, double, PASTETWO(FUNC, _pd), SIZE);					\
	GEN_FUNCC(RFUNC, PASTETWO(FUNC, _epi8), SIZE, std::is_integral<F>::value && sizeof(F) == 1); \
	GEN_FUNCC(RFUNC, PASTETWO(FUNC, _epi16), SIZE, std::is_integral<F>::value && sizeof(F) == 2); \
	GEN_FUNCC(RFUNC, PASTETWO(FUNC, _epi32), SIZE, std::is_integral<F>::value && sizeof(F) == 4); \
	GEN_FUNCC(RFUNC, PASTETWO(FUNC, _epi64), SIZE, std::is_integral<F>::value && sizeof(F) == 8); \

#define GEN_FNAME_128(NAME) PASTETWO(_mm_,NAME)
#define GEN_FNAME_256(NAME) PASTETWO(_mm256_,NAME)
#define GEN_FNAME_512(NAME) PASTETWO(_mm512_,NAME)

#define CALL(F, ...)F(__VA_ARGS__)

#define MM_NAME(NAME, SIZE) CALL(GEN_FNAME_##SIZE, NAME)

#define GEN_FUNC_SI(FUNC, SIZE) GEN_FUNC_SI2(_mm_##FUNC, MM_NAME(FUNC, SIZE), SIZE)
#define GEN_FUNC_EPI(FUNC, SIZE) GEN_FUNC_EPI2(_mm_##FUNC, MM_NAME(FUNC, SIZE), SIZE)

#define GEN_FUNCS(SIZE)			\
	GEN_FUNC_SI(loadu, SIZE);	\
	GEN_FUNC_SI(storeu, SIZE);	\
	GEN_FUNC_EPI(add, SIZE);	\
	GEN_FUNC_EPI(sub, SIZE);	\

#define DEFINE_MM(SIZE)													\
	template<> struct m_<SIZE, float> { using type = __m##SIZE; };		\
	template<> struct m_<SIZE, double> { using type = __m##SIZE##d; };	\
	template<typename F> struct m_<SIZE, std::enable_if<std::is_integral<F>::value, F>> { using type = __m##SIZE##i; }; \
	GEN_FUNCS(SIZE);													\

#endif
