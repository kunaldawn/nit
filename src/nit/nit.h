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

// WARNING: following order of definitions are carefully decided.

////////////////////////////////////////////////////////////////////////////////

// Namespace

#define NS_BEGIN(NS)	namespace NS {
#define NS_END(NS)		}

#define NS_NIT_BEGIN	NS_BEGIN(nit)
#define NS_NIT_END		NS_END(nit)

////////////////////////////////////////////////////////////////////////////////

// Define ASSERT macro

#define ASSERT assert
#define ASSERT_MSG(COND, FMT, ...)		 ASSERT((COND) && (FMT)) // TODO: impl this
#define ASSERT_THROW(COND, CODE)		 { if (!(COND)) { NIT_THROW_FMT(CODE, "assert(%s) failed", #COND); } }
#include <cassert>

////////////////////////////////////////////////////////////////////////////////

// Typical standard headers

#include <ctime>
#include <string>
#include <algorithm>
#include <utility>
#include <limits>

////////////////////////////////////////////////////////////////////////////////

// Platform & Build Configuration headers

#include "nit/config/SysConfig.h"
#include "nit/config/BuildConfig.h"

////////////////////////////////////////////////////////////////////////////////

// Determine DLL spec

#undef NIT_DLL

#if !defined(NIT_STATIC)
#	define NIT_DLL
#endif

#if defined(NIT_DLL)
#	if defined(NIT_BUILD_DLL)
#		define NIT_API NIT_DLL_EXPORT
#	else
#		define NIT_API NIT_DLL_IMPORT
#	endif
#else
#	define NIT_API
#endif

#ifndef SQUIRREL_API
#	define SQUIRREL_API NIT_API
#endif

////////////////////////////////////////////////////////////////////////////////

// Main nitcore headers

#define NIT_MAKE_CC(a, b, c, d) ( (a) | ((b) << 8) | ((c) << 16) | ((d) << 24) )

#include "nit/platform/Types.h"
#include "nit/platform/SystemTimer.h"
#include "nit/runtime/Exception.h"
#include "nit/ref/RefCounted.h"
#include "nit/util/Algorithms.h"
#include "nit/util/Containers.h"
#include "nit/math/NitMath.h"
#include "nit/event/Event.h"

#ifdef NIT_THREAD_NONE
#	include "nit/async/NoThreads.h"
#else
#	include "nit/async/Thread.h"
#	include "nit/async/ThreadLocal.h"
#endif // #ifdef NIT_THREAD_NONE

#include "nit/runtime/LogManager.h"

////////////////////////////////////////////////////////////////////////////////

#include "nit/runtime/NitRuntime.h"

#include "nit/data/DataValue.h"
#include "nit/data/DataSchema.h"

#include "nit/script/ScriptRuntime.h"

#include "nit/app/AppBase.h"
#include "nit/app/Package.h"
#include "nit/app/Session.h"
#include "nit/app/SessionService.h"

#include "nit/logic/Component.h"
#include "nit/logic/Object.h"
#include "nit/logic/Feature.h"
#include "nit/logic/World.h"
