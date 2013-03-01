﻿//
// This source file is from part of poco library
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//

#pragma once

// from poco 1.3

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NIT_API ThreadImpl
{
public:	
    typedef DWORD TIDImpl;
	typedef void (*Callable)(void*);

#if defined(_DLL)
	typedef DWORD (WINAPI *Entry)(LPVOID);
#else
	typedef unsigned (__stdcall *Entry)(void*);
#endif

	struct CallbackData
	{
		CallbackData(): callback(0), pData(0)
		{
		}

		Callable  callback;
		void*     pData; 
	};

	enum Priority
	{
		PRIO_LOWEST_IMPL  = THREAD_PRIORITY_LOWEST,
		PRIO_LOW_IMPL     = THREAD_PRIORITY_BELOW_NORMAL,
		PRIO_NORMAL_IMPL  = THREAD_PRIORITY_NORMAL,
		PRIO_HIGH_IMPL    = THREAD_PRIORITY_ABOVE_NORMAL,
		PRIO_HIGHEST_IMPL = THREAD_PRIORITY_HIGHEST
	};

	ThreadImpl();				
	~ThreadImpl();

	TIDImpl tidImpl() const;
	void setPriorityImpl(int prio);
	int getPriorityImpl() const;
	void setOSPriorityImpl(int prio);
	int getOSPriorityImpl() const;
	static int getMaxConcurrencyImpl();
	static int getMinOSPriorityImpl();
	static int getMaxOSPriorityImpl();
	void setStackSizeImpl(int size);
	int getStackSizeImpl() const;
	void startImpl(Runnable& target);
	void startImpl(Callable target, void* pData = 0);

	void joinImpl();
	bool joinImpl(long milliseconds);
	bool isRunningImpl() const;
	static void sleepImpl(long milliseconds);
	static void yieldImpl();
	static ThreadImpl* currentImpl();
	static TIDImpl currentTidImpl();
    
protected:
#if defined(_DLL)
	static DWORD WINAPI runnableEntry(LPVOID pThread);
#else
	static unsigned __stdcall runnableEntry(void* pThread);
#endif

#if defined(_DLL)
	static DWORD WINAPI callableEntry(LPVOID pThread);
#else
	static unsigned __stdcall callableEntry(void* pThread);
#endif

	void createImpl(Entry ent, void* pData);
	void threadCleanup();

private:
	class CurrentThreadHolder
	{
	public:
		CurrentThreadHolder(): _slot(TlsAlloc())
		{
			if (_slot == TLS_OUT_OF_INDEXES)
				NIT_THROW_FMT(EX_SYSTEM, "cannot allocate thread context key");
		}
		~CurrentThreadHolder()
		{
			TlsFree(_slot);
		}
		ThreadImpl* get() const
		{
			return reinterpret_cast<ThreadImpl*>(TlsGetValue(_slot));
		}
		void set(ThreadImpl* pThread)
		{
			TlsSetValue(_slot, pThread);
		}
	
	private:
		DWORD _slot;
	};

	Runnable*    _pRunnableTarget;
	CallbackData _callbackTarget;
	HANDLE       _thread;
	DWORD        _threadId;
	int          _prio;
	int          _stackSize;

	static CurrentThreadHolder _currentThreadHolder;
};


//
// inlines
//
inline int ThreadImpl::getPriorityImpl() const
{
	return _prio;
}


inline int ThreadImpl::getOSPriorityImpl() const
{
	return _prio;
}

inline int ThreadImpl::getMinOSPriorityImpl()
{
	return PRIO_LOWEST_IMPL;
}


inline int ThreadImpl::getMaxOSPriorityImpl()
{
	return PRIO_HIGHEST_IMPL;
}


inline void ThreadImpl::sleepImpl(long milliseconds)
{
	Sleep(DWORD(milliseconds));
}


inline void ThreadImpl::yieldImpl()
{
	Sleep(0);
}


inline void ThreadImpl::setStackSizeImpl(int size)
{
	_stackSize = size;
}


inline int ThreadImpl::getStackSizeImpl() const
{
	return _stackSize;
}


inline ThreadImpl::TIDImpl ThreadImpl::tidImpl() const
{
	return _threadId;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
