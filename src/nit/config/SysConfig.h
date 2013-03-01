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

#if defined(__APPLE_CPP__) || defined(__APPLE_CC__)
#   include <TargetConditionals.h>
#endif

////////////////////////////////////////////////////////////////////////////////

// Determine platform os

#define NIT_OS_UNKNOWN		0x0000
#define NIT_OS_WIN32		0x0001
#define NIT_OS_IOS			0x0004
#define NIT_OS_MAC32		0x0005 // Intel Mac i386
#define NIT_OS_ANDROID		0x0006

#undef NIT_OS
#undef NIT_WIN32
#undef NIT_IOS
#undef NIT_MAC32
#undef NIT_ANDROID

#undef NIT_FAMILY_WIN32
#undef NIT_FAMILY_UNIX
#undef NIT_FAMILY_MACH
#undef NIT_FAMILY_CUSTOM

#undef NIT_CALL_LTR
#undef NIT_CALL_RTL

#if defined(_WIN32)
#	define NIT_OS		NIT_OS_WIN32
#	define NIT_WIN32
#	define NIT_FAMILY_WIN32
#	define NIT_CALL_RTL

#elif TARGET_OS_IPHONE
#	define NIT_OS		NIT_OS_IOS
#	define NIT_IOS
#	define NIT_FAMILY_UNIX
#	define NIT_FAMILY_MACH
#	define NIT_CALL_LTR

#elif TARGET_OS_MAC						// should check TARGET_OS_IPHONE first - ios declares both IPHONE & MAC
#	define NIT_OS		NIT_OS_MAC32
#	define NIT_MAC32
#	define NIT_FAMILY_UNIX
#	define NIT_FAMILY_MACH
#	define NIT_CALL_LTR

#elif defined(ANDROID)
#	define NIT_OS		NIT_OS_ANDROID
#	define NIT_ANDROID
#	define NIT_FAMILY_UNIX
#	define NIT_CALL_LTR

#else
#   warning nit: cant determine os platform
#	define NIT_OS		NIT_OS_UNKNOWN
#	define NIT_FAMILY_CUSTOM
#endif

////////////////////////////////////////////////////////////////////////////////

// Determine processor power

#define NIT_32BIT
#undef NIT_64BIT // planned, but not yet

////////////////////////////////////////////////////////////////////////////////

// Determine endian-ness

#define NIT_ENDIAN_LITTLE	0
#define NIT_ENDIAN_BIG		1
#define NIT_ENDIAN_SERVER	NIT_ENDIAN_LITTLE // assumes x86 family server

#undef NIT_ENDIAN

#if defined(NIT_WIN32)
#	define NIT_ENDIAN		NIT_ENDIAN_LITTLE

#elif defined(NIT_IOS)
#	define NIT_ENDIAN		NIT_ENDIAN_LITTLE

#elif defined(NIT_ANDROID)
#	define NIT_ENDIAN		NIT_ENDIAN_LITTLE

#elif defined(NIT_MAC32)
#	define NIT_ENDIAN		NIT_ENDIAN_LITTLE

#endif

////////////////////////////////////////////////////////////////////////////////

// Determine API version

// TODO: impl this

////////////////////////////////////////////////////////////////////////////////

// Determine Thread API

#undef NIT_THREAD_WIN32
#undef NIT_THREAD_POSIX
#undef NIT_THREAD_CUSTOM
#undef NIT_THREAD_NONE

#if defined(NIT_FAMILY_WIN32)
#	define NIT_THREAD_WIN32

#elif defined(NIT_FAMILY_UNIX)
#	define NIT_THREAD_POSIX

#elif defined(NIT_FAMILY_CUSTOM)
#	define NIT_THREAD_CUSTOM

#else
#	define NIT_THREAD_NONE
#endif

////////////////////////////////////////////////////////////////////////////////

// Determine AlignedMalloc API

#undef AlignedMalloc
#undef AlignedRealloc
#undef AlignedFree

#if defined(NIT_FAMILY_WIN32)
#	define AlignedMalloc(size, alignment)			_aligned_malloc(size, alignment)
#	define AlignedRealloc(p, newSize, alignment)	_aligned_realloc(p, newSize, alignment)
#	define AlignedFree(p)							_aligned_free(p)

#else
// TODO: implement aligned-malloc functions
#	define AlignedMalloc(size, alignment)			malloc(size)
#	define AlignedRealloc(p, newSize, alignment)	realloc(p, newSize)
#	define AlignedFree(p)							free(p)

#endif

////////////////////////////////////////////////////////////////////////////////

// Determine stl container support level

#undef NIT_HASHMAP_STDEXT
#undef NIT_HASHMAP_TR1
#undef NIT_HASHMAP_CPP0X
#undef NIT_HASHMAP_NONE

#if defined(NIT_WIN32)
#	define NIT_HASHMAP_STDEXT			// for VC 9.0

#else
#	define NIT_HASHMAP_NONE

#endif

////////////////////////////////////////////////////////////////////////////////

// workaround for codes sticky to win32

#if !defined(NIT_FAMILY_WIN32)
#	define FORCEINLINE	
#	define MAX_PATH						PATH_MAX
#	define _snprintf					snprintf
#	define _strcmpi						strcasecmp
#	define _isnan						isnan
typedef unsigned long					DWORD;
#endif

////////////////////////////////////////////////////////////////////////////////

// Include platform specific header

#ifdef NIT_WIN32
#	include "SysConfig_win32.h"
#endif

#ifdef NIT_IOS
#	include "SysConfig_ios.h"
#endif

#ifdef NIT_MAC32
#	include "SysConfig_mac32.h"
#endif

#ifdef NIT_ANDROID
#	include "SysConfig_android.h"
#endif

////////////////////////////////////////////////////////////////////////////////
