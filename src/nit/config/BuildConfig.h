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

#include "nit/config/SysConfig.h"

////////////////////////////////////////////////////////////////////////////////

// Preprocessor Definitions:

// NIT_STATIC		: when DLL is not available/suitable
// NIT_BUILD_DLL	: when building DLL
//
// NIT_DEBUG    or _DEBUG             : non-optimized with full debug symbols
// NIT_DEVELOP  or _DEVELOP           : optimized build with debug symbols (incorrect but fast)
// NIT_RELEASE  or _RELEASE or NDEBUG : optimized build without debug symbols, with log/profile support
// NIT_SHIPPING or _SHIPPING for your final app build (launching onto market)

////////////////////////////////////////////////////////////////////////////////

#ifndef NIT_THREAD_STACK_SIZE
#	define NIT_THREAD_STACK_SIZE 0 // Zero means OS default
#endif

////////////////////////////////////////////////////////////////////////////////

// Determine Debug Level

#if !defined(NIT_DEBUG) && defined(_DEBUG)
#	define NIT_DEBUG
#endif

#if !defined(NIT_DEBUG) && NIT_OS==NIT_OS_IOS && DEBUG
#	define NIT_DEBUG
#endif

#if !defined(NIT_DEVELOP) && defined(_DEVELOP)
#	define NIT_DEVELOP
#endif

#if !defined(NIT_RELEASE) && (defined(_RELEASE) || defined(NDEBUG))
#	define NIT_RELEASE
#endif

#if !defined(NIT_SHIPPING) && defined(_SHIPPING)
#	define NIT_SHIPPING
#endif

////////////////////////////////////////////////////////////////////////////////

// Do not use Log facility on shipping build
#if defined(NIT_SHIPPING)
#	define NIT_NO_LOG
#endif

////////////////////////////////////////////////////////////////////////////////

// Turn on REFCOUNTED_DEBUGLIST except on shipping build (RefCounted.h)

#if !defined(NIT_SHIPPING)
#	define NIT_REFCOUNTED_DEBUGLIST
#endif

////////////////////////////////////////////////////////////////////////////////
