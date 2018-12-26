#ifndef MM_FUNCS_H
#define MM_FUNCS_H

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

#define PASTETWO(A, B) A##B

#define GEN_FUNC_PROTO(FUNC, SIZE)				\
	template<size_t sz, typename F>				\
    constexpr auto _mm_##FUNC = throwfunc;	\

#define GEN_FUNC_SI2(RFUNC, FUNC, SIZE)							\
	template<> constexpr auto RFUNC <SIZE, float> = PASTETWO(FUNC,_ps); \
	template<> constexpr auto RFUNC <SIZE, double> = PASTETWO(FUNC,_pd); \
	template<typename F> constexpr auto RFUNC <SIZE, std::enable_if<std::is_integral<F>::value, F>> = PASTETWO(FUNC, _si##SIZE);

#define GEN_FUNC_EPI2(RFUNC, FUNC, SIZE)								\
	template<> constexpr auto RFUNC <SIZE, float> = PASTETWO(FUNC, _ps); \
	template<> constexpr auto RFUNC <SIZE, double> = PASTETWO(FUNC, _pd); \
	template<typename F> constexpr auto RFUNC <SIZE, std::enable_if<std::is_integral<F>::value && sizeof(F) == 1, F>> = PASTETWO(FUNC, _epi8); \
	template<typename F> constexpr auto RFUNC <SIZE, std::enable_if<std::is_integral<F>::value && sizeof(F) == 2, F>> = PASTETWO(FUNC, _epi16); \
	template<typename F> constexpr auto RFUNC <SIZE, std::enable_if<std::is_integral<F>::value && sizeof(F) == 4, F>> = PASTETWO(FUNC, _epi32); \
	template<typename F> constexpr auto RFUNC <SIZE, std::enable_if<std::is_integral<F>::value && sizeof(F) == 8, F>> = PASTETWO(FUNC, _epi64);

#define GEN_FNAME_128(NAME) PASTETWO(_mm_,NAME)
#define GEN_FNAME_256(NAME) PASTETWO(_mm256_,NAME)
#define GEN_FNAME_512(NAME) PASTETWO(_mm512_,NAME)

#define CALL(F, ...)F(__VA_ARGS__)

#define MM_NAME(NAME, SIZE) CALL(GEN_FNAME_##SIZE, NAME)

#define GEN_FUNC_SI(FUNC, SIZE) GEN_FUNC_SI2(_mm_##FUNC, MM_NAME(FUNC, SIZE), SIZE)
#define GEN_FUNC_EPI(FUNC, SIZE) GEN_FUNC_EPI2(_mm_##FUNC, MM_NAME(FUNC, SIZE), SIZE)

GEN_FUNC_PROTO(loadu, SIZE);
GEN_FUNC_PROTO(storeu, SIZE);
GEN_FUNC_PROTO(add, SIZE);
GEN_FUNC_PROTO(sub, SIZE);

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
