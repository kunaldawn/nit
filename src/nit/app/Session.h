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

#include "nit/app/SessionService.h"

#include "nit/event/Timer.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class Package;
class World;
class Feature;
class ScriptRuntime;

class ConsoleInputEvent;

class Object;

////////////////////////////////////////////////////////////////////////////////

// TODO: refactor to other place

class NIT_API ConsoleInputEvent : public Event
{
public:
	ConsoleInputEvent() { }
	ConsoleInputEvent(const String& line) : line(line) { }
	String								line;
};

NIT_EVENT_DECLARE(NIT_API, CONSOLE_INPUT, ConsoleInputEvent);

////////////////////////////////////////////////////////////////////////////////

class NIT_API Session : public RefCounted
{
public:
	Session();
	Session(const String& name, const String& arg);
	virtual ~Session();

protected:
	virtual void						onStart()								{ }
	virtual void						onReady()								{ }
	virtual void						onStop()								{ }

	virtual ScriptRuntime*				createRuntime();

public:
	bool								isReady()								{ return _ready; }

	const char*							getName()								{ return _name.c_str(); }
	const char*							getArgument()							{ return _argument.c_str(); }

public:
	EventChannel*						channel()								{ return _channel; }

	TickTimer*							getTimer()								{ return _timer; }
	TimeScheduler*						getScheduler()							{ return _scheduler; }

	float								getTickTime()							{ return _timer->getTime(); }
	float								getBaseTime()							{ return _timer->getBaseTime(); }

public:
	Package*							getPackage()							{ return _package; }
	Package*							require(const String& name);

public:
	ScriptRuntime*						getScript()								{ return _script; }
	World*								getWorld()								{ return _world; }

public:
	void								Register(Feature* feature);
	void								Unregister(Feature* feature);

	Feature*							getFeature(const String& name);
	ModuleRegistry*						getLocalFeatures()						{ return &_localFeatures; }

protected:
	friend class						SessionService;
	void								start();
	void								stop();

protected:
	String								_name;
	String								_argument;
	String								_beforeArgument;

protected:
	Ref<ScriptRuntime>					_script;
	Ref<Package>						_package;
	Ref<World>							_world;

protected:
	ModuleRegistry						_localFeatures;

protected:
	friend class Object;
	friend class World;
	typedef set<Ref<Object> >::type IsolatedObjects;
	IsolatedObjects						_isolatedObjects;

	void								addIsolated(Object* obj);
	void								removeIsolated(Object* obj);

private:
	bool								_ready;

	void								linkPackage();
	void								openPackage();

private:
	Ref<EventChannel>					_channel;
	Ref<TickTimer>						_timer;
	Ref<TimeScheduler>					_scheduler;

	void								onClock(const TimeEvent* evt);
	void								onConsoleInput(const ConsoleInputEvent* evt);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API SessionEvent : public Event
{
public:
	SessionEvent()																{ }
	SessionEvent(nit::Session* session) : session(session)						{ }

	Ref<Session>						session;
};

NIT_EVENT_DECLARE(NIT_API, SESSION_START,	SessionEvent);
NIT_EVENT_DECLARE(NIT_API, SESSION_READY,	SessionEvent);
NIT_EVENT_DECLARE(NIT_API, SESSION_STOP,	SessionEvent);
NIT_EVENT_DECLARE(NIT_API, SESSION_CHANGE,	SessionEvent);

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
