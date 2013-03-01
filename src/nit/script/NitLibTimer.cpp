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

#include "nit/script/NitBind.h"
#include "nit/script/NitBindMacro.h"
#include "nit/script/ScriptRuntime.h"

#include "nit/event/Timer.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::TimeEvent, Event, incRefCount, decRefCount);

class NB_TimeEvent : TNitClass<TimeEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(timer),
			PROP_ENTRY_R(time),
			PROP_ENTRY_R(delta),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(timer)					{ return push(v, self(v)->getTimer()); }
	NB_PROP_GET(time)					{ return push(v, self(v)->getTime()); }
	NB_PROP_GET(delta)					{ return push(v, self(v)->getDelta()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::Timer, RefCounted, incRefCount, decRefCount);

class NB_Timer : TNitClass<Timer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(time),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(time)					{ return push(v, self(v)->getTime()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::Clock, Timer, incRefCount, decRefCount);

class NB_Clock : TNitClass<Clock>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"),

			FUNC_ENTRY_H(channel,		"(): EventChannel"),
			FUNC_ENTRY_H(loopHandler,	"(): EventHandler"),
			NULL
		};

		bind(v, props, funcs);
	}
	
	NB_CONS()							{ sq_setinstanceup(v, 1, new Clock()); return 0; }

	NB_FUNC(channel)					{ return push(v, self(v)->channel()); }
	NB_FUNC(loopHandler)				{ return push(v, self(v)->loopHandler()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::TickTimer, Timer, incRefCount, decRefCount);

class NB_TickTimer : TNitClass<TickTimer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(tick),
			PROP_ENTRY	(tickLimit),
			PROP_ENTRY	(speed),
			PROP_ENTRY	(tickInterval),
			PROP_ENTRY_R(sourceTime),
			PROP_ENTRY_R(fadeSpeed),
			PROP_ENTRY_R(sourceStep),
			PROP_ENTRY_R(timeStep),
			PROP_ENTRY_R(paused),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"),

			FUNC_ENTRY_H(channel,		"(): EventChannel"),
			FUNC_ENTRY_H(sourceTimeHandler, "(): EventHandler"),

			FUNC_ENTRY_H(fade,			"(duration, speed: float, inOut=0.0)"),
			FUNC_ENTRY_H(pause,			"(): bool // returns if paused"),
			FUNC_ENTRY_H(resume,		"(): bool // returns if paused yet"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(tick)					{ return push(v, self(v)->getTick()); }
	NB_PROP_GET(tickLimit)				{ return push(v, self(v)->getTickLimit()); }
	NB_PROP_GET(speed)					{ return push(v, self(v)->getSpeed()); }
	NB_PROP_GET(tickInterval)			{ return push(v, self(v)->getTickInterval()); }
	NB_PROP_GET(fadeSpeed)				{ return push(v, self(v)->getFadeSpeed()); }
	NB_PROP_GET(sourceTime)				{ return push(v, self(v)->getBaseTime()); }
	NB_PROP_GET(sourceStep)				{ return push(v, self(v)->getBaseStep()); }
	NB_PROP_GET(timeStep)				{ return push(v, self(v)->getTimeStep()); }
	NB_PROP_GET(paused)					{ return push(v, self(v)->isPaused()); }

	NB_PROP_SET(tickLimit)				{ self(v)->setTickLimit(getInt(v, 2)); return 0; }
	NB_PROP_SET(speed)					{ self(v)->setSpeed(getFloat(v, 2)); return 0; }
	NB_PROP_SET(tickInterval)			{ self(v)->setTickInterval(getFloat(v, 2)); return 0; }

	NB_CONS()							{ sq_setinstanceup(v, 1, new TickTimer()); return 0; }

	NB_FUNC(channel)					{ return push(v, self(v)->channel()); }
	NB_FUNC(sourceTimeHandler)			{ return push(v, self(v)->sourceTimeHandler()); }

	NB_FUNC(fade)						{ self(v)->fade(getFloat(v, 2), getFloat(v, 3), optFloat(v, 4, 0.0f)); return 0; }
	NB_FUNC(pause)						{ return push(v, self(v)->pause()); }
	NB_FUNC(resume)						{ return push(v, self(v)->resume()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::TimeScheduler, Timer, incRefCount, decRefCount);

class NB_TimeScheduler : TNitClass<TimeScheduler>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(updateQuota),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"),

			FUNC_ENTRY_H(sourceTimeHandler, "(): EventHandler"),
			FUNC_ENTRY_H(once,			"(handler: EventHandler, after: float): EventHandler"
			"\n"						"(sink, closure, after: float): EventHandler"),
			FUNC_ENTRY_H(repeat,		"(handler: EventHandler, interval: float, after=0.0): EventHandler"
			"\n"						"(sink, closure, interval: float, after=0.0): EventHandler"),
			FUNC_ENTRY_H(unbind,		"(handler: EventHandler)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(updateQuota)			{ return push(v, self(v)->getUpdateQuota()); }

	NB_PROP_SET(updateQuota)			{ self(v)->setUpdateQuota(getInt(v, 2)); return 0; }

	NB_CONS()							{ sq_setinstanceup(v, 1, new TimeScheduler()); return 0; }

	NB_FUNC(sourceTimeHandler)			{ return push(v, self(v)->sourceTimeHandler()); }

	NB_FUNC(once)
	{
		if (is<EventHandler>(v, 2))
			return push(v, self(v)->once(get<EventHandler>(v, 2), getFloat(v, 3)));
		else
			return push(v, self(v)->once(ScriptEventHandler::create(v, 2, 3), getFloat(v, 4)));
	}

	NB_FUNC(repeat)
	{
		if (is<EventHandler>(v, 2))
			return push(v, self(v)->repeat(get<EventHandler>(v, 2), getFloat(v, 3), optFloat(v, 4, 0.0f)));
		else
			return push(v, self(v)->repeat(ScriptEventHandler::create(v, 2, 3), getFloat(v, 4), optFloat(v, 5, 0.0f)));
	}

	NB_FUNC(unbind)					
	{ 
		if (is<EventHandler>(v, 2))
		{
			self(v)->unbind(get<EventHandler>(v, 2)); 
			return 0;
		}
		else if (sq_gettype(v, 2) == OT_INSTANCE)
		{
			IEventSink* sink = getInterface<IEventSink>(v, 2, true);
			if (sink) 
			{
				self(v)->unbind(sink);
				return 0;
			}
		}

		ScriptUnbindingSink ubsink(v, 2);
		self(v)->unbind(&ubsink);
		return 0; 
	}
};

////////////////////////////////////////////////////////////////////////////////

SQRESULT NitLibTimer(HSQUIRRELVM v)
{
	NB_TimeEvent::Register(v);
	NB_Timer::Register(v);
	NB_Clock::Register(v);
	NB_TickTimer::Register(v);
	NB_TimeScheduler::Register(v);

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
