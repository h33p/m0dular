#ifndef VECTOR_OPERATORS_H

#define VEC_OP(type, OP)									\
	friend inline auto operator OP(type v, const type& ov)	\
	{														\
		for (size_t o = 0; o < N; o++)						\
			v.v[o] = v.v[o] OP ov.v[o];						\
		return v;											\
	}														\
															\
	friend inline auto operator OP(type v, const T& ov)		\
	{														\
		for (size_t o = 0; o < N; o++)						\
			v.v[o] = v.v[o] OP ov;							\
		return v;											\
	}														\
															\
	friend inline auto operator OP(type v, const T* ov)		\
	{														\
		for (size_t o = 0; o < N; o++)						\
			v.v[o] = v.v[o] OP ov[o];						\
		return v;											\
	}														\
															\
    auto& operator OP##=(const type& ov)					\
	{														\
		for (size_t o = 0; o < N; o++)						\
			v[o] OP##= ov.v[o];								\
		return *this;										\
	}														\
															\
	auto& operator OP##=(const T& ov)						\
	{														\
		for (size_t o = 0; o < N; o++)						\
			v[o] OP##= ov;									\
		return *this;										\
	}														\
															\
	auto& operator OP##=(const T* ov)						\
	{														\
		for (size_t o = 0; o < N; o++)						\
			v[o] OP##= ov[o];								\
		return *this;										\
	}

#define SOA_OP(type, OP)							\
	friend inline auto operator OP(type v, const type& ov)	\
	{													\
		for (size_t i = 0; i < X; i++)					\
			for (size_t o = 0; o < Y; o++)				\
				v.v[i][o] = v.v[i][o] OP ov.v[i][o];	\
		return v;										\
	}													\
														\
	friend inline auto operator OP(type v, const T& ov)	\
	{													\
		for (size_t i = 0; i < X; i++)					\
			for (size_t o = 0; o < Y; o++)				\
				v.v[i][o] = v.v[i][o] OP ov;			\
		return v;										\
	}													\
														\
	friend inline auto operator OP(type v, const T* ov)	\
	{													\
		for (size_t i = 0; i < X; i++)					\
			for (size_t o = 0; o < Y; o++)				\
				v.v[i][o] = v.v[i][o] OP ov[o];			\
		return v;										\
	}													\
														\
    auto& operator OP##=(const type& ov)				\
	{													\
		for (size_t i = 0; i < X; i++)					\
			for (size_t o = 0; o < Y; o++)				\
				v[i][o] OP##= ov.v[i][o];				\
		return *this;									\
	}													\
														\
	auto& operator OP##=(const T& ov)					\
	{													\
		for (size_t i = 0; i < X; i++)					\
			for (size_t o = 0; o < Y; o++)				\
				v[i][o] OP##= ov;						\
		return *this;									\
	}													\
														\
	auto& operator OP##=(const T* ov)					\
	{													\
		for (size_t i = 0; i < X; i++)					\
			for (size_t o = 0; o < Y; o++)				\
				v[i][o] OP##= ov[o];					\
		return *this;									\
	}


#define WIDE_OP(type, OP)							\
	friend inline auto operator OP(type v, const type& ov)	\
	{													\
		for (size_t i = 0; i < Y; i++)					\
			for (size_t o = 0; o < X; o++)				\
				v[i][o] = v[i][o] OP ov[i][o];			\
		return v;										\
	}													\
														\
	friend inline auto operator OP(type v, const T& ov)	\
	{													\
		for (size_t i = 0; i < Y; i++)					\
			for (size_t o = 0; o < X; o++)				\
				v[i][o] = v[i][o] OP ov;				\
		return v;										\
	}													\
														\
	friend inline auto operator OP(type v, const T* ov)	\
	{													\
		for (size_t i = 0; i < Y; i++)					\
			for (size_t o = 0; o < X; o++)				\
				v[i][o] = v[i][o] OP ov[o];				\
		return v;										\
	}													\
														\
    auto& operator OP##=(const type& ov)				\
	{													\
		for (size_t i = 0; i < Y; i++)					\
			for (size_t o = 0; o < X; o++)				\
				w[i][o] OP##= ov[i][o];					\
		return *this;									\
	}													\
														\
	auto& operator OP##=(const T& ov)					\
	{													\
		for (size_t i = 0; i < Y; i++)					\
			for (size_t o = 0; o < X; o++)				\
				w[i][o] OP##= ov;						\
		return *this;									\
	}													\
														\
	auto& operator OP##=(const T* ov)					\
	{													\
		for (size_t i = 0; i < Y; i++)					\
			for (size_t o = 0; o < X; o++)				\
				w[i][o] OP##= ov[o];					\
		return *this;									\
	}


#define DEFINE_WIDE_OPS(type)					\
	WIDE_OP(type, +);							\
	WIDE_OP(type, -);							\
	WIDE_OP(type, *);							\
	WIDE_OP(type, /);							\
												\
	inline auto& operator =(const T& ov)		\
	{											\
		for (size_t i = 0; i < Y; i++)			\
			for (size_t o = 0; o < X; o++)		\
				w[i][o] = ov;					\
		return *this;							\
	}											\
												\
	inline auto& operator =(const T* ov)		\
	{											\
		for (size_t i = 0; i < Y; i++)			\
			for (size_t o = 0; o < X; o++)		\
				w[i][o] = ov[o];				\
		return *this;							\
	}


#define DEFINE_SOA_OPS(type)					\
	SOA_OP(type, +);							\
	SOA_OP(type, -);							\
	SOA_OP(type, *);							\
	SOA_OP(type, /);							\
												\
	inline auto& operator =(const T& ov)		\
	{											\
		for (size_t i = 0; i < X; i++)			\
			for (size_t o = 0; o < Y; o++)		\
				v[i][o] = ov;					\
		return *this;							\
	}											\
												\
	inline auto& operator =(const T* ov)		\
	{											\
		for (size_t i = 0; i < X; i++)			\
			for (size_t o = 0; o < Y; o++)		\
				v[i][o] = ov[o];				\
		return *this;							\
	}


#define DEFINE_VEC_OPS(type)					\
	VEC_OP(type, +);							\
	VEC_OP(type, -);							\
	VEC_OP(type, *);							\
	VEC_OP(type, /);							\
												\
	auto& operator =(const T& ov)				\
	{											\
		for (size_t o = 0; o < N; o++)			\
			v[o] = ov;							\
		return *this;							\
	}											\
												\
	auto& operator =(const T* ov)				\
	{											\
		for (size_t o = 0; o < N; o++)			\
			v[o] = ov[o];						\
		return *this;							\
	}



#endif
