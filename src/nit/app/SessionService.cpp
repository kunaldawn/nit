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

#include "nit/app/SessionService.h"

#include "nit/app/Session.h"
#include "nit/app/AppBase.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

SessionService::SessionService()
: Service("SessionService", NULL, SVC_SESSION)
{

}

void SessionService::onRegister()
{
	if (!require(g_Package)) NIT_THROW(EX_NOT_FOUND);
}

void SessionService::onUnregister()
{
}

void SessionService::onInit()
{
	g_App->channel()->bind(EVT::APP_NATIVE_FINISH, this, &SessionService::onAppFinish);
}

void SessionService::onFinish()
{
	endCurrent();

	_current = NULL;
	_next = NULL;
}

void SessionService::restart(bool quick)
{
	// TODO: SetDebugPaused(false);

	if (_current == NULL)
		return;

	const char* name	= _current->getName();
	const char* arg		= _current->getArgument();

	Session* same = create(name, arg);
	if (same)
		start(same);
}

void SessionService::start(Session* session)
{
// TODO: if (IsDebugPaused())
// 	{
// 		LOG(0, "*** Cannot SessionService::SwitchTo() while debug paused\n");
// 		return;
// 	}
	_next = session;

	if (_next)
	{
		if (_current)
			_next->_beforeArgument = _current->getArgument();

		LOG(0, "++ Next session designated: '%s(%s)'\n", _next->getName(), _next->getArgument());
	}
}

void SessionService::enter(Session* session)
{
	ASSERT_THROW(_current, EX_INVALID_STATE);

// TODO: 	if (IsDebugPaused())
// 	{
// 		LOG(0, "*** Cannot SessionService::GoForward() in debug mode\n");
// 		return;
// 	}

	if (_current == NULL)
		return;

	_history.push_back(_current);

	_next = session;

	if (_next)
	{
		_next->_beforeArgument = _current->getArgument();
	}
}

void SessionService::leave()
{
	ASSERT_THROW(!_history.empty(), EX_INVALID_STATE);

// 	TODO: if (IsDebugPaused())
// 	{
// 		LOG(0, "*** Cannot GameApp::GoBack() in debug mode\n");
// 		return;
// 	}

	if (_history.empty())
		return;

	_next = _history.back();
	_history.pop_back();
}

void SessionService::clearHistory()
{
	_history.clear();
}

void SessionService::changeIfNeeded()
{
	if (_next)
	{
		_next->linkPackage();
		if (_next->getPackage())
		{
			_next->getPackage()->setStayForNext(true);
		}

		endCurrent();

		_current = _next;
		_next = NULL;

		g_App->channel()->send(EVT::SESSION_CHANGE, new SessionEvent(_current));

		startCurrent();
	}
}

void SessionService::startCurrent()
{
	if (_current == NULL)
		return;

	LOG_TIMESCOPE(0, ".. SessionService::StartCurrent()");

	_current->start();
}

void SessionService::endCurrent()
{
	if (_current == NULL)
		return;

	LOG_TIMESCOPE(0, ".. SessionService::EndCurrent()");

	_current->stop();
	_current = NULL;
}

void SessionService::onAppFinish(const Event* evt)
{
	endCurrent();
	
	_current = NULL;
	_next = NULL;
}

void SessionService::Register(const String& classname, Constructor cons)
{
	ASSERT_THROW(_classes.find(classname) == _classes.end(), EX_DUPLICATED);

	_classes.insert(std::make_pair(classname, cons));

	LOG(0, ".. session class '%s' registered\n", classname.c_str());
}

void SessionService::Unregister(const String& classname)
{
	ClassNameMap::iterator itr = _classes.find(classname);

	ASSERT_THROW(itr != _classes.end(), EX_NOT_FOUND);

	_classes.erase(classname);

	LOG(0, ".. session class '%s' unregistered\n", classname.c_str());
}

Session* SessionService::create(const String& classname, const String& arg)
{
	ClassNameMap::iterator itr = _classes.find(classname);

	Session* session = NULL;

	if (itr == _classes.end())
	{
		LOG(0, "?? Session '%s(%s)': session class '%s' not registered - using default\n", classname.c_str(), arg.c_str(), classname.c_str());
		
		session = new Session();
	}
	else
	{
		Constructor cons = itr->second;
		session = cons(classname, arg);

		if (session == NULL)
		{
			LOG(0, "*** Session '%s(%s)': failed to create\n", classname.c_str(), arg.c_str());
			return NULL;
		}
	}

	session->_name = classname;
	session->_argument = arg;

	return session;
}

void SessionService::findClass(const String& pattern, StringVector& varResults)
{
	for (ClassNameMap::iterator itr = _classes.begin(), end = _classes.end(); itr != end; ++itr)
	{
		if (Wildcard::match(itr->first, pattern))
			varResults.push_back(itr->first);
	}
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
