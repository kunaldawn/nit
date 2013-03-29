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

#include "nit/app/Session.h"

#include "nit/app/SessionService.h"
#include "nit/app/Module.h"
#include "nit/app/AppBase.h"
#include "nit/app/Package.h"

#include "nit/logic/World.h"
#include "nit/logic/Object.h"

#include "nit/runtime/MemManager.h"

#include "nit/script/ScriptRuntime.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NIT_EVENT_DEFINE(SESSION_START,		SessionEvent);
NIT_EVENT_DEFINE(SESSION_READY,		SessionEvent);
NIT_EVENT_DEFINE(SESSION_STOP,			SessionEvent);
NIT_EVENT_DEFINE(SESSION_CHANGE,		SessionEvent);

NIT_EVENT_DEFINE(CONSOLE_INPUT,		ConsoleInputEvent);

////////////////////////////////////////////////////////////////////////////////

Session::Session()
{
	_ready	= false;
}

Session::Session( const String& name, const String& arg )
: _name(name), _argument(arg), _ready(false)
{

}

Session::~Session()
{
	LOG(0, ".. Session '%s(%s)' destroyed\n", _name.c_str(), _argument.c_str());
}

void Session::onConsoleInput(const ConsoleInputEvent* evt)
{
	// TODO: handle this event not here! refactor to other place
	if (g_Session != this) return;

	_script->command(evt->line);
}

void Session::onClock(const TimeEvent* evt)
{
	if (g_Session != this) return;

	if (!_ready && !g_Package->isAsyncLoading())
	{
		_ready = true;

		_localFeatures.init();

		onReady();
		_channel->send(EVT::SESSION_READY, new SessionEvent(this));
	}

	if (!_ready)
		return;

	_timer->sourceTimeHandler()->call(evt);
}

void Session::start()
{
	LOG_TIMESCOPE(0, "++ Starting Session '%s(%s)'", _name.c_str(), _argument.c_str());

	_ready = false;

	if (_channel == NULL)
	{
		_channel = new EventChannel();
		_channel->uplink(g_App->channel());
	}

	if (_timer == NULL)
	{
		_timer = new TickTimer();

		TickTimer* appTimer = g_App->getTimer();

		// copy configuration from app timer
		_timer->setSpeed(appTimer->getSpeed());
		_timer->setTickInterval(appTimer->getTickInterval());
		_timer->setTickLimit(appTimer->getTickLimit());
	}

	if (_scheduler == NULL)
	{
		_scheduler = new TimeScheduler();

		// event path : kernel.clock -> session.OnClock -> ticktimer.OnSourceTime -> scheduler.OnSourceTime
		g_App->getClock()->channel()->bind(EVT::CLOCK, this, &Session::onClock);
		_timer->channel()->bind(EVT::TICK, _scheduler->sourceTimeHandler());
	}

	g_MemManager->dump();

	_script = createRuntime();
	_script->setDefaultLocator(_package);

	_timer->channel()->bind(EVT::TICK, _script->tickHandler());
	g_App->getClock()->channel()->bind(EVT::CLOCK, _script->clockHandler());
	g_App->getScheduler()->repeat(_script->gcLoopHandler(), 0.1f);
	_script->startup();

	g_App->channel()->bind(EVT::CONSOLE_INPUT, this, &Session::onConsoleInput);
	
	HSQUIRRELVM v = _script->getWorker();
	sq_pushroottable(v);
	NitBind::newSlot(v, -1, "session", this);
	sq_poptop(v);

	_world = new World();
	
	openPackage();

	onStart();
	_channel->send(EVT::SESSION_START, new SessionEvent(this));

	// NOTE: if our script needs SessionStat, Ready, Stop event then Bind() its handler to the channel
}

void Session::linkPackage()
{
	String packname = _name;

	_package = g_Package->link(packname.c_str());

	if (_package)
		_package->incUseCount();
}

void Session::openPackage()
{
	if (_package)
	{
		_package->loadAsync();
	}
	else
	{
		LOG(0, "*** cannot find package for session '%s(%s)'\n", _name.c_str(), _argument.c_str());
	}
}

void Session::stop()
{
	g_MemManager->dump();

	LOG_TIMESCOPE(0, "++ Stopping Session '%s(%s)'", _name.c_str(), _argument.c_str());

	// Broadcast session stop
	onStop();
	_channel->send(EVT::SESSION_STOP, new SessionEvent(this));

	// Shutup Debugger
// 	_script->GetDebugger()->Disable();

	// Kill script cotasks
	_script->killAllThreads(false);

	// Dispose all isolated objects
	for (IsolatedObjects::iterator itr = _isolatedObjects.begin(), end = _isolatedObjects.end(); itr != end; ++itr)
	{
		Object* obj = *itr;
		obj->dispose();
	}
	_isolatedObjects.clear();

	// Finish Local Features
	_localFeatures.finish();

	// Dispose world
	_world->dispose();
	_world = NULL;

	// Disuse session package
	if (_package)
		_package->decUseCount();
	_package = NULL;

	// Shutdown script session
	_script->shutdown();
	_script = NULL;

	// Release channel
	_channel = NULL;
}

void Session::addIsolated(Object* obj)
{
	_isolatedObjects.insert(obj);
}

void Session::removeIsolated(Object* obj)
{
	_isolatedObjects.erase(obj);
}

ScriptRuntime* Session::createRuntime()
{
	return new ScriptRuntime();
}

/// Local Feature registration : a Session can be registered with Feature not its superclass Module.
/// Its rule follows:
/// - Session.Start : register
/// - Session.Ready : init by require order
///   register after init : init at once
/// - Session.Stop  : finish by reversed require order -> unregister
/// So we can utilize Feature or ScriptFeature in case a module-like code limited by a session scope

void Session::Register(Feature* feature)
{
	_localFeatures.Register(feature);
}

void Session::Unregister(Feature* feature)
{
	_localFeatures.Unregister(feature);
}

Feature* Session::getFeature(const String& name)
{
	Feature* ret = static_cast<Feature*>(_localFeatures.get(name));

	if (ret == NULL)
		ret = dynamic_cast<Feature*>(g_App->getModule(name));

	return ret;
}

Package* Session::require(const String& name)
{
	Package* pack = g_Package->load(name.c_str());

	if (pack == NULL)
		return NULL;
	
	if (_package)
		_package->require(pack);
	else
		_package = pack;

	return pack;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
