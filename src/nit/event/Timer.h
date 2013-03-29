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

#include "nit/event/Event.h"

#include "nit/platform/SystemTimer.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NIT_API Timer : public RefCounted
{
public:
	Timer();

public:
	float								getTime()								{ return _time; }

	virtual bool						isEventActive()							{ return true; }
	virtual bool						isDisposed()							{ return false; }

protected:
	float								_time;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API TimeEvent : public Event
{
public:
	TimeEvent() { } 
	TimeEvent(Timer* timeSource, float delta) 
		: _timer(timeSource), _delta(delta)									{ }

public:
	Timer*								getTimer() const						{ return _timer; }
	float								getTime() const							{ return _timer->getTime(); }
	float								getDelta() const						{ return _delta; }

protected:
	Ref<Timer>							_timer;
	float								_delta;
};

////////////////////////////////////////////////////////////////////////////////

NIT_EVENT_DECLARE(NIT_API, CLOCK,			TimeEvent);
NIT_EVENT_DECLARE(NIT_API, TICK,			TimeEvent);
NIT_EVENT_DECLARE(NIT_API, TIME_SCHEDULE,	TimeEvent);
NIT_EVENT_DECLARE(NIT_API, TICK_PAUSE,		TimeEvent);
NIT_EVENT_DECLARE(NIT_API, TICK_RESUME,		TimeEvent);

////////////////////////////////////////////////////////////////////////////////

class NIT_API Clock : public Timer
{
public:
	Clock();

public:
	void								update();
	EventHandler*						loopHandler()							{ return _loopHandler; }

	EventChannel*						channel()								{ return _channel ? _channel : _channel = new EventChannel(); }

	double								getResolution()							{ return _resolution; }
	
protected:
	void								onMainLoop(const Event* evt);

	Ref<EventChannel>					_channel;
	Ref<EventHandler>					_loopHandler;

	SystemTimer::Tick					_clock;
	SystemTimer::Tick					_sysClock;

	double								_resolution;

	double								_bigTime;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API TickTimer : public Timer
{
public:
	TickTimer();

public:
	EventChannel*						channel()								{ return _channel ? _channel : _channel = new EventChannel(); }

public:
	float								getBaseTime()							{ return _baseTime; }

	uint								getTick()								{ return _tick; }
	uint								getTickLimit()							{ return _tickLimit; }
	float								getTickInterval()						{ return _tickInterval; }

	void								setTickLimit(uint limit);
	void								setTickInterval(float interval);

	float								getSpeed()								{ return _speed; }
	void								setSpeed(float speed);

	void								fade(float duration, float speed, float inOut = 0.0f);
	float								getFadeSpeed()							{ return _fadeSpeed; }

	float								getBaseStep()							{ return _baseStep; }
	float								getTimeStep()							{ return _timeStep; }

	bool								isPaused()								{ return _pauseCount > 0; }
	bool								pause();
	bool								resume();

public:
	uint								advance(float dt);
	EventHandler*						sourceTimeHandler()						{ return _sourceTimeHandler; }

protected:
	void								onSourceTime(const TimeEvent* evt);

	Ref<EventChannel>					_channel;
	Ref<EventHandler>					_sourceTimeHandler;

	uint								_tick;
	uint								_tickLimit;

	float								_speed;
	float								_fadeSpeed;
	float								_tickInterval;

	float								_baseTime;
	float								_lastBaseTime;

	float								_baseStep;
	float								_timeStep;

	int									_pauseCount;

	struct TimeFade
	{
		float							startTime;
		float							duration;
		float							speed;
		float							inOut;
	};

	typedef vector<TimeFade>::type		TimeFadeStack;
	TimeFadeStack						_timeFadeStack;

	void								updateFadeTime();
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API TimeScheduler : public Timer
{
public:
	TimeScheduler();
	virtual ~TimeScheduler();

public:
	void								advance(float dt);
	EventHandler*						sourceTimeHandler()						{ return _sourceTimeHandler; }

public:
	EventHandler*						once(EventHandler* handler, float after);
	EventHandler*						repeat(EventHandler* handler, float interval, float after = 0.0f);
	void								unbind(EventHandler* handler);
	void								unbind(IEventSink* sink);

public:
	int									getUpdateQuota()						{ return _updateQuota; }
	void								setUpdateQuota(int quota)				{ _updateQuota = quota; }

protected:
	void								onSourceTime(const TimeEvent* evt);
	void								update();
	void								unbindAll();

	struct UpdateEntry
	{
		float							wakeTime;
		float							time;
		float							interval;
		Ref<EventHandler>				handler;

		struct HeapCompare;
	};

	typedef vector<UpdateEntry*>::type UpdateHeap;

	UpdateHeap							_entries;
	int									_updateQuota;

	Ref<EventHandler>					_sourceTimeHandler;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
