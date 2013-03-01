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

#include "nit/app/Module.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NIT_API Service : public Module
{
public:
	enum ServiceID
	{
		SVC_NULL,

		SVC_ID_01,
		SVC_SESSION,
		SVC_PACKAGE,
		SVC_CONTENTS,
		SVC_INPUT,
		SVC_RENDER,
		SVC_PROFILER,
		SVC_ID_08,

		SVC_KITS,
		SVC_MEDIA,
		SVC_NET,
		SVC_ID_12,
		SVC_ID_13,
		SVC_ID_14,
		SVC_ID_15,
		SVC_ID_16,

		SVC_OGRE,
		SVC_COCOS,
		SVC_ID_19,
		SVC_ID_20,
		SVC_ID_21,
		SVC_ID_22,
		SVC_ID_23,
		SVC_BUNDLER,

		SVC_ID_APP_01,
		SVC_ID_APP_02,
		SVC_ID_APP_03,
		SVC_ID_APP_04,
		SVC_ID_APP_05,
		SVC_ID_APP_06,
		SVC_ID_APP_07,
		SVC_ID_APP_08,

		SERVICE_ID_COUNT
	};

protected:
	Service(const String& name, Package* package, ServiceID id);
	virtual ~Service();		

public:
	ServiceID							getServiceID()							{ return _serviceID; }

protected:
	virtual void						onContextEnter()						{ }
	virtual void						onContextLeave()						{ }

protected:
	ServiceID							_serviceID;

	friend class						ServiceContext;
};

typedef vector<Weak<Service> >::type ServiceList;

////////////////////////////////////////////////////////////////////////////////

class NIT_API ServiceContext
{
public:
	ServiceContext();
	virtual ~ServiceContext();

public:
	inline Service*						getService(Service::ServiceID id)		{ return _serviceID[id]; }

public:
	void								Register(Service* service);
	void								Unregister(Service* service);

public:
	static ServiceContext*				getCurrent()							{ ASSERT_THROW(Thread::current() == NULL, EX_NOT_SUPPORTED); return s_Current; }
	bool								isCurrent()								{ return s_Current == this; }

	void								enter();
	void								leave();

protected:
	static ServiceContext*				s_Current;
	static std::vector<ServiceContext*>	s_ContextStack;

	Service*							_serviceID[Service::SERVICE_ID_COUNT];

	void								notifyContextEnter();
	void								notifyContextLeave();

	friend class AppBase;
	void								unregisterAll();
};

template <typename TClass>
class ScopedEnter
{
public:
	ScopedEnter(TClass* obj) : _obj(obj)										{ if (_obj) _obj->enter(); }
	~ScopedEnter()																{ if (_obj) _obj->leave(); }

	TClass* _obj;
};

////////////////////////////////////////////////////////////////////////////////

#define g_Service						(::nit::ServiceContext::getCurrent())

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
