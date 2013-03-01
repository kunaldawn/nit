/*
-----------------------------------------------------------------------------
This source file is from part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#pragma once

#include "nit/nit.h"
#include "nit/legacy/Legacy_Ogre.h"

#include <memory>

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

// from OgreMemoryUserAlloc.h

// User memory functions - Replace these with your memory functions

typedef void*	(*user_alloc_func) (size_t size);
typedef void	(*user_free_func) (void* ptr);
typedef size_t	(*user_max_alloc_size_func) ();

typedef void*	(*user_aligned_alloc_func) (size_t size, size_t alignment);
typedef void	(*user_aligned_free_func) (void* ptr, size_t alignment);
typedef size_t	(*user_aligned_max_alloc_size_func) (size_t alignment);

// TODO: remove user_xxx stuff and hard link to MemManager (make it work before the others!)

extern NIT_API user_alloc_func							user_alloc;
extern NIT_API user_free_func							user_free;
extern NIT_API user_max_alloc_size_func					user_max_alloc_size;

extern NIT_API user_aligned_alloc_func					user_aligned_alloc;
extern NIT_API user_aligned_free_func					user_aligned_free;
extern NIT_API user_aligned_max_alloc_size_func			user_aligned_max_alloc_size;

////////////////////////////////////////////////////////////////////////////////

// Below comes internal interface with AllocPolicy system

class NIT_API UserAllocPolicy
{
public:
	static inline void* allocateBytes(size_t count, const char*  = 0, int  = 0, const char* = 0)
	{
		assert(user_alloc != NULL && "MemManager not initialized yet");

		void* ptr = user_alloc(count);
		return ptr;
	}

	static inline void deallocateBytes(void* ptr)
	{
		user_free(ptr);
	}

	/// Get the maximum size of a single allocation
	static inline size_t getMaxAllocationSize()
	{
		return user_max_alloc_size();
	}
private:
	// no instantiation
	UserAllocPolicy()
	{ }
};

////////////////////////////////////////////////////////////////////////////////

template <size_t Alignment = 0>
class UserAlignedAllocPolicy
{
public:
	// compile-time check alignment is available.
	typedef int IsValidAlignment
		[Alignment <= 128 && ((Alignment & (Alignment-1)) == 0) ? +1 : -1];

	static inline void* allocateBytes(size_t count, const char*  = 0, int  = 0, const char* = 0)
	{
		void* ptr = Alignment ? user_aligned_alloc(count, Alignment) : user_alloc(count);
		return ptr;
	}

	static inline void deallocateBytes(void* ptr)
	{
		if (Alignment)
			user_aligned_free(ptr, Alignment);
		else
			user_free(ptr);
	}

	/// Get the maximum size of a single allocation
	static inline size_t getMaxAllocationSize()
	{
		return Alignment ? user_aligned_max_alloc_size(Alignment) : user_max_alloc_size();
	}
private:
	// No instantiation
	UserAlignedAllocPolicy()
	{ }
};

////////////////////////////////////////////////////////////////////////////////

// from OgreMemoryAllocatorConfig.h

template <MemoryCategory Cat> class CategorisedAllocPolicy : public UserAllocPolicy {};
template <MemoryCategory Cat, size_t align = 0> class CategorisedAlignAllocPolicy : public UserAlignedAllocPolicy<align>{};

////////////////////////////////////////////////////////////////////////////////

// from OgreMemorySTLAllocator.h

// Base STL allocator class.
template<typename T>
struct STLAllocatorBase
{	// base class for generic allocators
	typedef T value_type;
};

// Base STL allocator class. (const T version).
template<typename T>
struct STLAllocatorBase<const T>
{	// base class for generic allocators for const T
	typedef T value_type;
};

template
<
typename T,
typename AllocPolicy
>
class STLAllocator : public STLAllocatorBase<T>
{
public :
	/// define our types, as per ISO C++
	typedef STLAllocatorBase<T>			Base;
	typedef typename Base::value_type	value_type;
	typedef value_type*					pointer;
	typedef const value_type*			const_pointer;
	typedef value_type&					reference;
	typedef const value_type&			const_reference;
	typedef std::size_t					size_type;
	typedef std::ptrdiff_t				difference_type;


	/// the standard rebind mechanism
	template<typename U>
	struct rebind
	{
		typedef STLAllocator<U, AllocPolicy> other;
	};

	/// ctor
	inline explicit STLAllocator()
	{ }

	/// dtor
	virtual ~STLAllocator()
	{ }

	/// copy ctor - done component wise
	inline STLAllocator( STLAllocator const& )
	{ }

	/// cast
	template <typename U>
	inline STLAllocator( STLAllocator<U, AllocPolicy> const& )
	{ }

	/// cast
	template <typename U, typename P>
	inline STLAllocator( STLAllocator<U, P> const& )
	{ }

	/// memory allocation (elements, used by STL)
	inline pointer allocate( size_type count,
		typename std::allocator<void>::const_pointer ptr = 0 )
	{
		(void)ptr;
		// convert request to bytes
		register size_type sz = count*sizeof( T );
		pointer p  = static_cast<pointer>(AllocPolicy::allocateBytes(sz));
		return p;
	}

	/// memory deallocation (elements, used by STL)
	inline void deallocate( pointer ptr, size_type )
	{
		// convert request to bytes, but we can't use this?
		// register size_type sz = count*sizeof( T );
		AllocPolicy::deallocateBytes(ptr);
	}

	pointer address(reference x) const
	{
		return &x;
	}

	const_pointer address(const_reference x) const
	{
		return &x;
	}

	size_type max_size() const throw()
	{
		// maximum size this can handle, delegate
		return AllocPolicy::getMaxAllocationSize();
	}

	void construct(pointer p, const T& val)
	{
		// call placement new
		new(static_cast<void*>(p)) T(val);
	}

	void destroy(pointer p)
	{
		// do we have to protect against non-classes here?
		// some articles suggest yes, some no
		p->~T();
	}

};

/// determine equality, can memory from another allocator
/// be released by this allocator, (ISO C++)
template<typename T, typename T2, typename P>
inline bool operator==(STLAllocator<T,P> const&,
					   STLAllocator<T2,P> const&)
{
	// same alloc policy (P), memory can be freed
	return true;
}

/// determine equality, can memory from another allocator
/// be released by this allocator, (ISO C++)
template<typename T, typename P, typename OtherAllocator>
inline bool operator==(STLAllocator<T,P> const&,
					   OtherAllocator const&)
{
	return false;
}
/// determine equality, can memory from another allocator
/// be released by this allocator, (ISO C++)
template<typename T, typename T2, typename P>
inline bool operator!=(STLAllocator<T,P> const&,
					   STLAllocator<T2,P> const&)
{
	// same alloc policy (P), memory can be freed
	return false;
}

/// determine equality, can memory from another allocator
/// be released by this allocator, (ISO C++)
template<typename T, typename P, typename OtherAllocator>
inline bool operator!=(STLAllocator<T,P> const&,
					   OtherAllocator const&)
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////

// from OgreMemoryAllocatedObject.h

template <class Alloc>
class NIT_API AllocatedObject
{
public:
	explicit AllocatedObject()
	{ }

	~AllocatedObject()
	{ }

	/// operator new, with debug line info
	void* operator new(size_t sz, const char* file, int line, const char* func)
	{
		return Alloc::allocateBytes(sz, file, line, func);
	}

	void* operator new(size_t sz)
	{
		return Alloc::allocateBytes(sz);
	}

	/// placement operator new
	void* operator new(size_t sz, void* ptr)
	{
		(void) sz;
		return ptr;
	}

	/// array operator new, with debug line info
	void* operator new[] ( size_t sz, const char* file, int line, const char* func )
	{
		return Alloc::allocateBytes(sz, file, line, func);
	}

	void* operator new[] ( size_t sz )
	{
		return Alloc::allocateBytes(sz);
	}

	void operator delete( void* ptr )
	{
		Alloc::deallocateBytes(ptr);
	}

	// Corresponding operator for placement delete (second param same as the first)
	void operator delete( void* ptr, void* )
	{
		(void) ptr;
	}

	// only called if there is an exception in corresponding 'new'
	void operator delete( void* ptr, const char* , int , const char*  )
	{
		Alloc::deallocateBytes(ptr);
	}

	void operator delete[] ( void* ptr )
	{
		Alloc::deallocateBytes(ptr);
	}


	void operator delete[] ( void* ptr, const char* , int , const char*  )
	{
		Alloc::deallocateBytes(ptr);
	}
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
