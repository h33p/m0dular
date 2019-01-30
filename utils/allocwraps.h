#ifndef ALLOCWRAPS_H
#define ALLOCWRAPS_H

template<auto& BASE, bool REALLOCATABLE, bool FIXED_SIZE_ALLOCATIONS>
class FreeListAllocator;

#include "freelistallocator.h"

//Safely define void reference type
template<typename T>
struct GetReference
{
	using type = T&;
};

template<>
struct GetReference<void>
{
	using type = void;
};

template<typename T, auto& BASE>
struct offset_pointer_t
{
	T* ptr;

    using difference_type = ptrdiff_t;
    using value_type = T;
    using pointer = T*;
    using reference = typename GetReference<T>::type;
    using iterator_category = std::random_access_iterator_tag;

	constexpr offset_pointer_t()
		: ptr((T*)~0ul)
	{
	}

	constexpr void CheckPtr()
	{
#ifdef OFFSET_POINTER_DEBUG
		if (this->operator bool() && (((uintptr_t)ptr + (uintptr_t)BASE) < (uintptr_t)BASE || ((uintptr_t)ptr + (uintptr_t)BASE) > (uintptr_t)BASE + 0x2063a0))
			throw;
#endif
	}

	constexpr offset_pointer_t& operator=(const offset_pointer_t& o)
	{
		ptr = o.ptr;
		CheckPtr();
		return *this;
	}

	constexpr offset_pointer_t& operator=(T* o)
	{
		ptr = o ? (T*)((uintptr_t)o - (uintptr_t)BASE) : (T*)~0ul;
		CheckPtr();
		return *this;
	}


	constexpr operator size_t()
	{
		if ((uintptr_t)ptr ^ ~0ul)
			return (size_t)((uintptr_t)ptr + (uintptr_t)BASE);
		return 0;
	}

	constexpr offset_pointer_t(T* p)
		: ptr(p ? (T*)((uintptr_t)p - (uintptr_t)BASE) : (T*)~0ul)
	{
		CheckPtr();
	}

	template<typename F>
	constexpr offset_pointer_t(offset_pointer_t<F, BASE> o)
	: ptr((T*)o.ptr)
	{
		CheckPtr();
	}

	template<typename F, typename = typename std::enable_if<AllArithmetic<F>::value>::type>
	constexpr offset_pointer_t(F p)
		: ptr(p ? (T*)((uintptr_t)p - (uintptr_t)BASE) : (T*)~0ul)
	{
		CheckPtr();
	}

	template<typename F, typename = typename std::enable_if<AllArithmetic<F>::value>::type>
	constexpr offset_pointer_t(F* p)
		: ptr(p ? (T*)((uintptr_t)p - (uintptr_t)BASE) : (T*)~0ul)
	{
		CheckPtr();
	}

	constexpr T* GetRawPtr() const
	{
		return ((uintptr_t)ptr ^ ~0ul) ? (T*)((uintptr_t)BASE + (uintptr_t)ptr) : nullptr;
	}


	template<typename H = T>
	constexpr typename std::enable_if<!std::is_same<H, void>::value, H&>::type operator*() const
	{
		return *GetRawPtr();
	}

	constexpr T* operator->() const
	{
		return GetRawPtr();
	}

	template<typename H = T>
	constexpr typename std::enable_if<!std::is_same<H, void>::value, H&>::type operator[](int idx) const
	{
		return GetRawPtr()[idx];
	}

	constexpr bool operator==(const offset_pointer_t& o) const
	{
		return ptr == o.ptr;
	}

	constexpr bool operator!=(const offset_pointer_t& o) const
	{
		return ptr != o.ptr;
	}

	constexpr difference_type operator-(offset_pointer_t i) const
	{
		return ptr - i.ptr;
	}

	template<typename F>
	constexpr offset_pointer_t operator+(F i) const
	{
		return offset_pointer_t((uintptr_t)(ptr + i) + (uintptr_t)BASE);
	}

	template<typename F>
	constexpr offset_pointer_t operator-(F i) const
	{
		return offset_pointer_t((uintptr_t)(ptr - i) + (uintptr_t)BASE);
	}

	constexpr offset_pointer_t& operator++()
	{
		ptr++;
		return *this;
	}

	constexpr offset_pointer_t& operator--()
	{
		ptr--;
		return *this;
	}

	constexpr operator bool() const
	{
		return ((uintptr_t)ptr ^ ~0ul);
	}

	constexpr bool operator!() const
	{
		return !this->operator bool();
	}

	constexpr operator pointer() const
	{
		return GetRawPtr();
	}

	template<typename F>
	static constexpr offset_pointer_t pointer_to(F& ref)
	{
		return offset_pointer_t((T*)&ref);
	}
};

template<auto& BASE, bool REALLOCATABLE = false, bool FIXED_SIZE_ALLOCATIONS = false>
class generic_free_list_allocator : FreeListAllocator<BASE, REALLOCATABLE, FIXED_SIZE_ALLOCATIONS>
{
  public:
	static constexpr auto& base = BASE;

	template<typename F>
	struct pointer_t : offset_pointer_t<F, BASE>
	{
	    typedef offset_pointer_t<F, BASE> base_class;
		typedef typename base_class::difference_type difference_type;
		typedef typename base_class::value_type value_type;
		typedef typename base_class::pointer pointer;
		typedef typename base_class::reference reference;
		typedef typename base_class::iterator_category iterator_category;

		constexpr pointer_t()
			: base_class()
		{
		}

		constexpr pointer_t& operator=(const pointer_t& o)
		{
			base_class::operator=(o);
			return *this;
		}

		constexpr pointer_t(F* p)
			: base_class(p) {}

		constexpr pointer_t(const base_class& o)
			: base_class(o) {}

		template<typename H = F>
		constexpr typename std::enable_if<!std::is_same<H, void>::value, H&>::type operator*() const
		{
			return base_class::operator*();
		}

		constexpr F* operator->() const
		{
			return base_class::operator->();
		}

		constexpr bool operator==(const pointer_t& o) const
		{
			return base_class::operator==(o);
		}

		constexpr bool operator!=(const pointer_t& o) const
		{
			return base_class::operator!=(o);
		}

		constexpr difference_type operator-(pointer_t i) const
		{
			return this->ptr - i.ptr;
		}

		template<typename G>
		constexpr pointer_t operator+(G i) const
		{
		    return base_class::template operator+(i);
		}

		template<typename G>
		constexpr pointer_t operator-(G i) const
		{
			return base_class::template operator-(i);
		}

		constexpr pointer_t& operator++()
		{
		    base_class::operator++();
			return *this;
		}

		constexpr pointer_t& operator--()
		{
		    base_class::operator--();
			return *this;
		}

		constexpr operator bool() const
		{
			return base_class::operator bool();
		}


		template<typename H>
		static constexpr pointer_t pointer_to(H& ref)
		{
			return pointer_t((F*)&ref);
		}
	};

	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef std::false_type is_always_equal;
	typedef std::true_type propagate_on_container_move_assignment;

	template<typename... Args>
	generic_free_list_allocator(Args... args)
	: FreeListAllocator<BASE, REALLOCATABLE, FIXED_SIZE_ALLOCATIONS>(args...)
	{
	}

	~generic_free_list_allocator()
	{
	}

	template<typename T>
	inline auto allocate(size_type size)
	{
	    auto ptr = this->Allocate(sizeof(T) * size, std::alignment_of<T>::value);
		return pointer_t<T>((T*)&*ptr);
	}

	template<typename T>
	inline void deallocate(pointer_t<T> ptr, size_type size)
	{
		this->Free(offset_pointer_t<void*, BASE>((void**)&*ptr));
	}

	inline bool operator==(const generic_free_list_allocator& o) const
	{
		return this->m_start_ptr == o.m_start_ptr;
	}
};

template<typename T, auto& BASE, bool REALLOCATABLE = true, size_t SIZE = 10000, PlacementPolicy PLACEMENT_POLICY = PlacementPolicy::FIND_FIRST, bool FIXED_SIZE_ALLOCATIONS = false>
class free_list_allocator : public generic_free_list_allocator<BASE, REALLOCATABLE, FIXED_SIZE_ALLOCATIONS>
{

  public:
	static constexpr auto& base = BASE;

	template<typename F>
	using pointer_t = typename generic_free_list_allocator<BASE, REALLOCATABLE, FIXED_SIZE_ALLOCATIONS>::template pointer_t<F>;
	using value_type = T;
	using pointer = pointer_t<T>;
	using size_type = size_t;
	using difference_type = ptrdiff_t;
	using is_always_equal = std::false_type;
	using propagate_on_container_move_assignment = std::false_type;

	free_list_allocator(size_t size = SIZE, PlacementPolicy pPolicy = PLACEMENT_POLICY)
	: generic_free_list_allocator<BASE, REALLOCATABLE, FIXED_SIZE_ALLOCATIONS>(size * sizeof(T), pPolicy)
	{
	}

	template<typename H, size_t SZ2, PlacementPolicy PP2>
	free_list_allocator(const free_list_allocator<H, BASE, REALLOCATABLE, SZ2, PP2, FIXED_SIZE_ALLOCATIONS>& o)
	: generic_free_list_allocator<BASE, REALLOCATABLE, FIXED_SIZE_ALLOCATIONS>(SZ2 * sizeof(T), PP2)
	{
	}


	~free_list_allocator()
	{
	}

	inline pointer allocate(size_type size)
	{
		return generic_free_list_allocator<BASE, REALLOCATABLE, FIXED_SIZE_ALLOCATIONS>::template allocate<T>(size);
	}

	inline void deallocate(pointer ptr, size_type size)
	{
		generic_free_list_allocator<BASE, REALLOCATABLE, FIXED_SIZE_ALLOCATIONS>::template deallocate<T>(ptr, size);
	}

	inline bool operator==(const free_list_allocator& o)
	{
		return generic_free_list_allocator<BASE, REALLOCATABLE, FIXED_SIZE_ALLOCATIONS>::template operator==(o);
	}

	template<typename Other>
	struct rebind
	{
	    using other = free_list_allocator<Other, BASE, REALLOCATABLE, SIZE, PLACEMENT_POLICY, FIXED_SIZE_ALLOCATIONS>;
	};
};

template<typename T, typename F>
class stateful_pointer_allocator
{
  public:
	using value_type = T;
	using parent_type = typename std::decay<decltype(*F())>::type;
	using pointer = typename parent_type::template pointer_t<T>;
	using size_type = size_t;
	using difference_type = ptrdiff_t;
	using is_always_equal = std::false_type;
	using propagate_on_container_move_assignment = std::true_type;

	template<typename H>
	stateful_pointer_allocator(const stateful_pointer_allocator<H, F>& o)
	{
	}

	stateful_pointer_allocator()
	{
	}

	~stateful_pointer_allocator()
	{
	}

	inline pointer allocate(size_type size)
	{
		return (*F()).template allocate<T>(size);
	}

	inline void deallocate(pointer ptr, size_type size)
	{
		(*F()).template deallocate<T>(ptr, size);
	}

	inline bool operator==(const stateful_pointer_allocator& o)
	{
		return F()->template operator==(o);
	}

	template<typename Other>
	struct rebind
	{
		using other = stateful_pointer_allocator<Other, F>;
	};
};

template<typename T, auto& G>
using stateful_allocator = stateful_pointer_allocator<T, pointer_proxy<G>>;

#endif
