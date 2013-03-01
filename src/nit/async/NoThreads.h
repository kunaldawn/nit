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

// NOTE: Use this file for only initial porting step to lessen the burden of porting async system.

#include "nit/util/ScopedLock.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class Mutex
{
public:
	void lock() { }
	void unlock() { }

	typedef nit::ScopedLock<Mutex> ScopedLock;
};

////////////////////////////////////////////////////////////////////////////////

typedef Mutex FastMutex;

////////////////////////////////////////////////////////////////////////////////

class Runnable
{
public:
	virtual ~Runnable() { }
};

////////////////////////////////////////////////////////////////////////////////

class EventSemaphore
{
public:
	void								set()									{ }
	void								reset()									{ }
	void								wait()									{ }
};

////////////////////////////////////////////////////////////////////////////////

class Thread
{
public:
	enum Priority
	{
		PRIO_LOWEST,
		PRIO_LOW,
		PRIO_NORMAL,
		PRIO_HIGH,
		PRIO_HIGHEST,
	};

	Thread(const char* name)													{ }

	void								start(Runnable& runner)					{ }
	void								join()									{ }
	void								setPriority(Priority prio)				{ }
	std::string							name()									{ return ""; }

	static Thread*						current()								{ return NULL; }

	static void yield() 
	{
#if defined(NIT_FAMILY_WIN32)
		Sleep(0);
#elif defined(NIT_FAMILY_UNIX)
		sched_yield();
#endif
	}

	static void sleep(long milliseconds)
	{
#if defined(NIT_FAMILY_WIN32)
		Sleep(DWORD(milliseconds));
#elif defined(NIT_FAMILY_UNIX)
		;;; // TODO: need to impl this?
#endif
	}
};

////////////////////////////////////////////////////////////////////////////////

template <typename TClass>
class ThreadLocal
{
public:
	ThreadLocal() { }
	
	ThreadLocal& operator= (TClass& val) { _value = val; }
	
	operator TClass* () { return &_value; }
	TClass& operator* () { return _value; }
		
private:
	TClass _value;
};

class ThreadLocalStorage
{
public:
	static ThreadLocalStorage* current() { return NULL; }
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
