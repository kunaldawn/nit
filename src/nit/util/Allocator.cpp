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

#include "nit_pch.h"

#include "Allocator.h"

#include "nit/runtime/MemManager.h"
#include "nit/math/Bitwise.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

// default memory interfaces

static void* DefaultUserAlloc(size_t size)
{
	return g_MemManager->Allocate(size, MEM_DEFAULT_ALIGNMENT, MEM_HINT_NONE);
}

static void DefaultUserFree(void* ptr)
{
	g_MemManager->deallocate(ptr, MEM_DEALLOC_SIZE_UNKNOWN);
}

static size_t DefaultMaxAllocSize()
{
	return std::numeric_limits<size_t>::max();
}

static void* DefaultAlignedAlloc(size_t size, size_t alignment)
{
	if (alignment <= MEM_DEFAULT_ALIGNMENT)
		return DefaultUserAlloc(size);
	else
	{
		// from OgreAlignedAllocator.cpp
		assert(0 < alignment && alignment <= 128 && Bitwise::isPO2(alignment));

		unsigned char* p = new unsigned char[size + alignment];
		size_t offset = alignment - (size_t(p) & (alignment-1));

		unsigned char* result = p + offset;
		result[-1] = (unsigned char)offset;

		return result;
	}
}

static void DefaultAlignedFree(void* ptr, size_t alignment)
{
	if (alignment <= MEM_DEFAULT_ALIGNMENT)
		return DefaultUserFree(ptr);
	else			
	{
		// from OgreAlignedAllocator.cpp
		if (ptr)
		{
			unsigned char* mem = (unsigned char*)ptr;
			mem = mem - mem[-1];
			delete [] mem;
		}
	}
}

static size_t DefaultAlignedMaxAllocSize(size_t alignment)
{
	return std::numeric_limits<size_t>::max();
}

////////////////////////////////////////////////////////////////////////////////

user_alloc_func						user_alloc						= NULL;
user_free_func						user_free						= NULL;
user_max_alloc_size_func			user_max_alloc_size				= NULL;

user_aligned_alloc_func				user_aligned_alloc				= NULL;
user_aligned_free_func				user_aligned_free				= NULL;
user_aligned_max_alloc_size_func	user_aligned_max_alloc_size		= NULL;

////////////////////////////////////////////////////////////////////////////////

class __DefaultAllocatorInit
{
public:
	__DefaultAllocatorInit()
	{
		if (user_alloc == NULL)
		{
			user_alloc				= DefaultUserAlloc;
			user_free				= DefaultUserFree;
			user_max_alloc_size		= DefaultMaxAllocSize;
		}

		if (user_aligned_alloc == NULL)
		{
			user_aligned_alloc					= DefaultAlignedAlloc;
			user_aligned_free					= DefaultAlignedFree;
			user_aligned_max_alloc_size			= DefaultAlignedMaxAllocSize;
		}
	}
};

__DefaultAllocatorInit _init;

NS_NIT_END;
