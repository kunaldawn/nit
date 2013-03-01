/// nit - Noriter Framework
/// A Cross-platform Open Source Integration for Game-oriented Apps
///
/// http://www.github.com/ellongrey/nit
///
/// Copyright (c) 2013 by Jun-hyeok Jang
/// 
/// (see each file to see the different copyright owners)
/// 
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///
/// Author: ellongrey

#pragma once

////////////////////////////////////////////////////////////////////////////////

// define alias of primitive types

#ifdef NIT_32BIT

NS_NIT_BEGIN;

typedef unsigned char		byte;

typedef unsigned char		uchar;
typedef unsigned short		ushort;
typedef unsigned int		uint;
typedef unsigned long		ulong;

typedef signed char			int8;
typedef unsigned char		uint8;
typedef signed short		int16;
typedef unsigned short		uint16;
typedef signed int			int32;
typedef unsigned int		uint32;
typedef signed long long	int64;
typedef unsigned long long	uint64;

typedef float				Real;

typedef int8				i8;
typedef uint8				u8;
typedef int16				i16;
typedef uint16				u16;
typedef int32				i32;
typedef uint32				u32;
typedef int64				i64;
typedef uint64				u64;

NS_NIT_END;

#endif // #ifdef NIT_32BIT

////////////////////////////////////////////////////////////////////////////////

// includes Allocator.h for container type definition

#include "nit/util/Allocator.h"

// define DefaultAllocPolicy

NS_NIT_BEGIN;

typedef CategorisedAllocPolicy<MEMCATEGORY_DEFAULT> DefaultAllocPolicy;
typedef CategorisedAllocPolicy<MEMCATEGORY_GLOBAL_POOL> PooledAllocPolicy;

typedef AllocatedObject<DefaultAllocPolicy> DefaultAlloc;
typedef AllocatedObject<PooledAllocPolicy> PooledAlloc;

NS_NIT_END;

////////////////////////////////////////////////////////////////////////////////

// import stl containers with custom allocation

#include <deque>
#include <vector>
#include <list>
#include <set>
#include <map>

NS_NIT_BEGIN;

template <typename T, typename A = STLAllocator<T, DefaultAllocPolicy> > 
struct deque 
{ 
	typedef typename std::deque<T, A> type;    
}; 

template <typename T, typename A = STLAllocator<T, DefaultAllocPolicy> > 
struct vector 
{ 
	typedef typename std::vector<T, A> type;    
}; 

template <typename T, typename A = STLAllocator<T, DefaultAllocPolicy> > 
struct list 
{ 
	typedef typename std::list<T, A> type;    
}; 

template <typename T, typename P = std::less<T>, typename A = STLAllocator<T, DefaultAllocPolicy> > 
struct set 
{ 
	typedef typename std::set<T, P, A> type;    
}; 

template <typename K, typename V, typename P = std::less<K>, typename A = STLAllocator<std::pair<const K, V>, DefaultAllocPolicy> > 
struct map 
{ 
	typedef typename std::map<K, V, P, A> type; 
}; 

template <typename K, typename V, typename P = std::less<K>, typename A = STLAllocator<std::pair<const K, V>, DefaultAllocPolicy> > 
struct multimap 
{ 
	typedef typename std::multimap<K, V, P, A> type; 
}; 

NS_NIT_END;

////////////////////////////////////////////////////////////////////////////////

// Import unordered_set and unordered_map

// In many platform, this is not a standard, so determine by platform

#if defined(NIT_HASHMAP_CPP0X)

#include <unordered_set>
#include <unordered_map>

NS_NIT_BEGIN;

template <typename K, typename V, typename P = std::hash_compare<K, std::less<K> >, typename A = STLAllocator<std::pair<const K, V>, DefaultAllocPolicy> >
struct unordered_map
{
	typedef typename std::unordered_map<K, V, P, A> type;
};

template <typename K, typename V, typename P = std::hash_compare<K, std::less<K> >, typename A = STLAllocator<std::pair<const K, V>, DefaultAllocPolicy> >
struct unordered_multimap
{
	typedef typename std::unordered_multimap<K, V, P, A> type;
};

template <typename T, typename H = std::hash<T>, typename E = std::equal_to<T>, typename A = STLAllocator<T, DefaultAllocPolicy> >
struct unordered_set
{
	typedef typename std::unordered_set<T, H, E, A> type;
};

template <typename T, typename H = std::hash<T>, typename E = std::equal_to<T>, typename A = STLAllocator<T, DefaultAllocPolicy> >
struct unordered_multiset
{
	typedef typename std::unordered_multiset<T, H, E, A> type;
};

NS_NIT_END;

#endif // #if defined(NIT_HASHMAP_CPP0X)

// Typically in Visual Studio 2008, stdext supported

#ifdef NIT_HASHMAP_STDEXT

#include <hash_set>
#include <hash_map>

NS_NIT_BEGIN;

template <typename K, typename V, typename P = stdext::hash_compare<K, std::less<K> >, typename A = STLAllocator<std::pair<const K, V>, DefaultAllocPolicy> >
struct unordered_map
{
	typedef typename stdext::hash_map<K, V, P, A> type;
};

template <typename K, typename V, typename P = stdext::hash_compare<K, std::less<K> >, typename A = STLAllocator<std::pair<const K, V>, DefaultAllocPolicy> >
struct unordered_multimap
{
	typedef typename stdext::hash_multimap<K, V, P, A> type;
};

template <typename T, typename P = stdext::hash_compare<T, std::less<T> >, typename A = STLAllocator<T, DefaultAllocPolicy> >
struct unordered_set
{
	typedef typename stdext::hash_set<T, P, A> type;
};

template <typename T, typename P = stdext::hash_compare<T, std::less<T> >, typename A = STLAllocator<T, DefaultAllocPolicy> >
struct unordered_multiset
{
	typedef typename stdext::hash_multiset<T, P, A> type;
};

NS_NIT_END;

#endif // #ifdef NIT_HASHMAP_STDEXT

#ifdef NIT_HASHMAP_NONE

NS_NIT_BEGIN;

template <typename T, typename P = std::less<T>, typename A = STLAllocator<T, DefaultAllocPolicy> > 
struct unordered_set 
{ 
	typedef typename std::set<T, P, A> type;    
}; 

template <typename K, typename V, typename P = std::less<K>, typename A = STLAllocator<std::pair<const K, V>, DefaultAllocPolicy> > 
struct unordered_map
{ 
	typedef typename std::map<K, V, P, A> type; 
}; 

template <typename K, typename V, typename P = std::less<K>, typename A = STLAllocator<std::pair<const K, V>, DefaultAllocPolicy> > 
struct unordered_multimap
{ 
	typedef typename std::multimap<K, V, P, A> type; 
}; 

NS_NIT_END;

#endif

////////////////////////////////////////////////////////////////////////////////

// define String and StringUtil

#include "nit/data/NitString.h"
#include "nit/data/StringUtil.h"

////////////////////////////////////////////////////////////////////////////////


