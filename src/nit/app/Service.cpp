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

#include "nit_pch.h"

#include "nit/app/Service.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

Service::Service(const String& name, Package* package, Service::ServiceID id)
: Module(name, package), _serviceID(id)
{
}

Service::~Service()
{
}

////////////////////////////////////////////////////////////////////////////////

ServiceContext* ServiceContext::s_Current = NULL;
std::vector<ServiceContext*> ServiceContext::s_ContextStack;

ServiceContext::ServiceContext()
{
	memset(_serviceID, 0, sizeof(_serviceID));
}

ServiceContext::~ServiceContext()
{
}

void ServiceContext::enter()
{
	bool switching = s_Current != this;

	if (switching && s_Current)
		s_Current->notifyContextLeave();

	s_Current = this;

	s_ContextStack.push_back(this);

	if (switching && s_Current)
		s_Current->notifyContextEnter();
}

void ServiceContext::leave()
{
	ASSERT_THROW(s_Current == this, EX_INVALID_STATE);

	s_ContextStack.pop_back();

	ServiceContext* top = s_ContextStack.empty() ? NULL : s_ContextStack.back();

	bool switching = s_Current != top;

	if (switching && s_Current)
		s_Current->notifyContextLeave();

	s_Current = top;

	if (switching && s_Current)
		s_Current->notifyContextEnter();
}

void ServiceContext::Register(Service* service)
{
	if (_serviceID[service->getServiceID()] != NULL)
	{
		Service* svc = _serviceID[service->getServiceID()];
		NIT_THROW_FMT(EX_DUPLICATED, "Service '%s' already with same id for '%s'", svc->getName().c_str(), service->getName().c_str());
	}

	_serviceID[service->getServiceID()] = service;
}

void ServiceContext::Unregister(Service* service)
{
	if (_serviceID[service->getServiceID()] != service)
		return;

	_serviceID[service->getServiceID()] = NULL;
}

void ServiceContext::unregisterAll()
{
	for (uint i=0; i<Service::SERVICE_ID_COUNT; ++i)
	{
		_serviceID[i] = NULL;
	}
}

void ServiceContext::notifyContextEnter()
{
	for (uint i=0; i<Service::SERVICE_ID_COUNT; ++i)
	{
		Service* svc = _serviceID[i];
		if (svc)
			svc->onContextEnter();
	}
}

void ServiceContext::notifyContextLeave()
{
	for (uint i=0; i<Service::SERVICE_ID_COUNT; ++i)
	{
		Service* svc = _serviceID[i];
		if (svc)
			svc->onContextLeave();
	}
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
