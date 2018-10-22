#ifndef SOA_ACCESSOR_H
#define SOA_ACCESSOR_H

#define REF() &&
#define X(...)

#define SOA_VECTOR_CAST(type)					\
	template<size_t B>							\
	explicit inline operator type<T, B>() {		\
		type<T, B> ret;							\
		constexpr size_t mv = B < X ? B : X;	\
		auto& it = *this;						\
		for (size_t i = 0; i < mv; i++)			\
			ret[i] = it[i];						\
		return ret;								\
	}

#define SOA_SCALAR_ASIGNMENT			\
	inline auto& operator=(T val)		\
	{									\
		auto& it = *this;				\
		for (size_t i = 0; i < X; i++)	\
			it[i] = val;				\
		return it;						\
	}

#define SOA_ASIGNMENT(type)						\
	template<size_t B>							\
	inline auto& operator=(type<T, B> vec)		\
	{											\
		constexpr size_t mv = B < X ? B : X;	\
		auto& it = *this;						\
		for (size_t i = 0; i < mv; i++)			\
			it[i] = vec[i];						\
		return it;								\
	}

//XYZ might be invalid on vecSoa, depending on how many columns are thare
#define DEFINE_SOA_ACCESSOR								\
	struct {											\
		struct SoaAccessor {							\
			T x;										\
			T px[Y - 1];								\
			T y;										\
			T py[Y - 1];								\
			T z;										\
														\
			inline T& operator[](int idx)				\
			{											\
				return (&x)[(int)idx * (int)Y];			\
			}											\
														\
			inline auto& Set(SoaAccessor& acc)			\
			{											\
				for(size_t i = 0; i < X; i++)			\
					(*this)[i] = acc[i];				\
				return *this;							\
			}											\
														\
			SOA_ASIGNMENT(vecb);						\
			SOA_ASIGNMENT(vecp);						\
			SOA_SCALAR_ASIGNMENT;						\
			SOA_VECTOR_CAST(vecb);						\
			SOA_VECTOR_CAST(vecp);						\
		} acc2;											\
														\
		inline auto& operator[](int idx) const 			\
		{												\
			return *(SoaAccessor*)(((T*)&acc2)+idx);	\
		}												\
	} acc;

#endif
