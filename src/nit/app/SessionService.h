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

#include "nit/app/Service.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

#define g_SessionService				static_cast<nit::SessionService*>(g_Service->getService(::nit::Service::SVC_SESSION))
#define g_Session						(g_SessionService->getCurrent())

////////////////////////////////////////////////////////////////////////////////

class Session;

////////////////////////////////////////////////////////////////////////////////

class NIT_API SessionService : public Service
{
public:
	SessionService();

public:
	typedef Session*					(*Constructor) (const String& name, const String& arg);

	void								Register(const String& classname, Constructor cons);
	void								Unregister(const String& classname);
	Session*							create(const String& classname, const String& arg);
	void								findClass(const String& pattern, StringVector& varResults);

public:
	Session*							getCurrent()							{ return _current; }
	Session*							getNext()								{ return _next; }

public:
	void								restart(bool quick = true);
	void								start(Session* session);
	void								enter(Session* session);
	void								leave();
	void								clearHistory();
	bool								hasHistory()							{ return !_history.empty(); }

protected:
	virtual void						onRegister();
	virtual void						onUnregister();

	virtual void						onInit();
	virtual void						onFinish();

protected:
	void								changeIfNeeded();
	void								startCurrent();
	void								endCurrent();

	Ref<Session>						_current;
	Ref<Session>						_next;

	vector<Ref<Session> >::type			_history;

	typedef map<String, Constructor>::type ClassNameMap;
	ClassNameMap						_classes;

	// TODO: temporary
	friend class AppBase;

	void								onAppFinish(const Event* evt);
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
