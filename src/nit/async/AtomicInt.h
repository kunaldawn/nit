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

#include "nit/nit.h"

////////////////////////////////////////////////////////////////////////////////

#if defined(NIT_WIN32)

NS_NIT_BEGIN;

class NIT_API AtomicInt
{
public:
	AtomicInt(int initValue = 0) : _value(initValue) { }

public:
	inline void							inc()									{ InterlockedIncrement(&_value); }
	inline void							dec()									{ InterlockedDecrement(&_value); }
	inline int							incGet()								{ return InterlockedIncrement(&_value); }
	inline int							decGet()								{ return InterlockedDecrement(&_value); }

	inline int							_unsafeGet()							{ return _value; }
	inline void							_unsafeSet(int value)					{ _value = value; }

private:
	volatile LONG						_value;
};

NS_NIT_END;

#endif

////////////////////////////////////////////////////////////////////////////////

#if defined(NIT_IOS) || defined(NIT_MAC32)

#include "libkern/OSAtomic.h"

NS_NIT_BEGIN;

class NIT_API AtomicInt
{
public:
	AtomicInt(int initValue = 0) : _value(initValue) { }

public:
	inline void							inc()									{ OSAtomicIncrement32(&_value); }
	inline void							dec()									{ OSAtomicDecrement32(&_value); }
	inline int							incGet()								{ return OSAtomicIncrement32(&_value); }
	inline int							decGet()								{ return OSAtomicDecrement32(&_value); }

	inline int							_unsafeGet()							{ return _value; }
	inline void							_unsafeSet(int value)					{ _value = value; }

private:
	volatile int32_t					_value;
};

NS_NIT_END;

#endif

////////////////////////////////////////////////////////////////////////////////

#if defined(NIT_ANDROID)

#include "sys/atomics.h"

NS_NIT_BEGIN;

class NIT_API AtomicInt
{
public:
	AtomicInt(int initValue = 0) : _value(initValue) { }

public:
	inline void							inc()									{ __atomic_inc(&_value); }
	inline void							dec()									{ __atomic_dec(&_value); }
	inline int							incGet()								{ return __atomic_inc(&_value); }
	inline int							decGet()								{ return __atomic_dec(&_value); }

	inline int							_unsafeGet()							{ return _value; }
	inline void							_unsafeSet(int value)					{ _value = value; }

private:
	volatile int						_value;
};

NS_NIT_END;

#endif

////////////////////////////////////////////////////////////////////////////////
