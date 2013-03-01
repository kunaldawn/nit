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

#include "nit/event/Timer.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NIT_EVENT_DEFINE(OnClock,				TimeEvent);
NIT_EVENT_DEFINE(OnTick,				TimeEvent);
NIT_EVENT_DEFINE(OnTimeSchedule,		TimeEvent);
NIT_EVENT_DEFINE(OnTickPause,			TimeEvent);
NIT_EVENT_DEFINE(OnTickResume,			TimeEvent);

////////////////////////////////////////////////////////////////////////////////

Timer::Timer()
{
	_time = 0.0f;
}

////////////////////////////////////////////////////////////////////////////////

Clock::Clock()
{
	_sysClock = 0;
	_clock = 0;
	_bigTime = 0.0;

	_loopHandler = createEventHandler(this, &Clock::onMainLoop);

	_resolution = SystemTimer::secondsPerTick();
}

void Clock::onMainLoop(const Event* evt)
{
	update();
}

void Clock::update()
{
	// clock_t will be reset about every 24 days (a long type)
	// TODO: check platform-wise and use 64-bit counter when possible (Win32: PerformanceCounter)

	SystemTimer::Tick c = SystemTimer::currentTick();

	if (_sysClock == 0)
		_sysClock = c;

	SystemTimer::Tick dc = c - _sysClock;

	if (SystemTimer::needBigTime() && c < _sysClock)
	{
		// overflow
		_bigTime += (unsigned long)(-1) * 0.001;
		_sysClock = 0;
		_clock = 0;
		dc = c;
	}

	if (dc == 0) return;

	_clock += dc;
	_sysClock = c;

	_time = float(_clock * _resolution);
	if (SystemTimer::needBigTime())
		_time = float(_time + _bigTime);

	float dt = float(dc * _resolution);

	if (_channel) _channel->send(Events::OnClock, new TimeEvent(this, dt));
}

////////////////////////////////////////////////////////////////////////////////

TickTimer::TickTimer()
{
	_tick = 0;
	_tickLimit = 0;

	_speed = 1.0f;
	_fadeSpeed = 1.0f;
	_tickInterval = 0.0f;

	_baseTime = 0.0f;
	_lastBaseTime = 0.0f;

	_baseStep = 0.0f;
	_timeStep = 0.0f;

	_pauseCount = 0;

	_sourceTimeHandler = createEventHandler(this, &TickTimer::onSourceTime);
}

uint TickTimer::advance(float dt)
{
	if (_pauseCount > 0) return 0;

	Ref<TickTimer> safe = this;

	_baseStep = dt;
	_baseTime += dt;
	updateFadeTime();

	if (_tickInterval <= 0.0f)
	{
		_timeStep = dt * _speed * _fadeSpeed;
		_time += _timeStep;
		++_tick;
		if (_channel) _channel->send(Events::OnTick, new TimeEvent(this, _timeStep));
		return 1;
	}

	uint numTicks = 0;

	float delta = _baseTime - _lastBaseTime;

	while (delta >= _tickInterval)
	{
		_lastBaseTime += _tickInterval;
		delta -= _tickInterval;

		if (_tickLimit && numTicks >= _tickLimit)
			continue; // TODO: Refactor if it's possible to break not continue

		_timeStep = _tickInterval * _speed * _fadeSpeed;
		_time += _timeStep;
		++numTicks;
		++_tick;

		if (_channel) _channel->send(Events::OnTick, new TimeEvent(this, _timeStep));
	}

	return numTicks;
}

void TickTimer::onSourceTime(const TimeEvent* evt)
{
	advance(evt->getDelta());
}

void TickTimer::setTickLimit(uint limit)
{
	_tickLimit = limit;
}

void TickTimer::setSpeed(float speed)
{
	_speed = speed;
	_timeStep = _tickInterval * _speed * _fadeSpeed;
}

void TickTimer::setTickInterval(float interval)
{
	_tickInterval = interval;
	_timeStep = _tickInterval * _speed * _fadeSpeed;
}

void TickTimer::fade(float duration, float speed, float inOut /*= 0.0f*/)
{
	_timeFadeStack.push_back(TimeFade());
	TimeFade& fade = _timeFadeStack.back();

	fade.startTime = _baseTime;
	fade.duration = duration;
	fade.speed = speed;
	fade.inOut = inOut;
}

void TickTimer::updateFadeTime()
{
	_fadeSpeed = 1.0f;

	while (!_timeFadeStack.empty())
	{
		TimeFade& fade = _timeFadeStack.back();

		float t = _baseTime - fade.startTime;

		if (t > fade.duration)
		{
			_timeFadeStack.pop_back();
			continue;
		}

		float targetSpeed;

		if (fade.inOut <= 0.0f)
			targetSpeed = fade.speed;
		else if (t < fade.inOut)
			targetSpeed = Math::lerp(t / fade.inOut, 1.0f, fade.speed);
		else if (t > fade.duration - fade.inOut)
			targetSpeed = Math::lerp((t - fade.duration + fade.inOut) / fade.inOut, fade.speed, 1.0f);
		else
			targetSpeed = fade.speed;

		_fadeSpeed = targetSpeed;
		break;
	}
}

bool TickTimer::pause()
{
	if (++_pauseCount == 1)
	{
		_channel->send(Events::OnTickPause, new TimeEvent(this, 0.0f));
	}

	return _pauseCount > 0;
}

bool TickTimer::resume()
{
	if (--_pauseCount == 0)
	{
		_channel->send(Events::OnTickResume, new TimeEvent(this, 0.0f));
	}

	return _pauseCount > 0;
}

////////////////////////////////////////////////////////////////////////////////

// for std::make_heap()
struct TimeScheduler::UpdateEntry::HeapCompare
{
	bool operator () (const UpdateEntry* a, const UpdateEntry* b)
	{ 
		return a->wakeTime > b->wakeTime; 
	}
};

TimeScheduler::TimeScheduler()
{
	_sourceTimeHandler = createEventHandler(this, &TimeScheduler::onSourceTime);
	_updateQuota = 0;
}

TimeScheduler::~TimeScheduler()
{
	unbindAll();
}

void TimeScheduler::unbindAll()
{
	for (uint i=0; i<_entries.size(); ++i)
	{
		delete _entries[i];
	}

	_entries.clear();
}

EventHandler* TimeScheduler::once(EventHandler* handler, float after)
{
	Ref<EventHandler> autoRel = handler;

	if (handler == NULL || !handler->canHandle(Events::OnTimeSchedule))
		EventInfo::throwInvalidHandler(Events::OnTimeSchedule);

	UpdateEntry* e = new UpdateEntry();

	e->time		= _time;
	e->wakeTime	= _time + after;
	e->interval	= 0.0f;
	e->handler	= handler;

	_entries.push_back(e);
	std::push_heap(_entries.begin(), _entries.end(), UpdateEntry::HeapCompare());

	return handler;
}

EventHandler* TimeScheduler::repeat(EventHandler* handler, float interval, float after)
{
	Ref<EventHandler> autoRel = handler;

	if (handler == NULL || !handler->canHandle(Events::OnTimeSchedule))
		EventInfo::throwInvalidHandler(Events::OnTimeSchedule);

	UpdateEntry* e = new UpdateEntry();

	e->time		= _time;
	e->wakeTime	= after > 0.0f ? _time + after : _time + interval;
	e->interval	= interval;
	e->handler	= handler;

	_entries.push_back(e);
	std::push_heap(_entries.begin(), _entries.end(), UpdateEntry::HeapCompare());

	return handler;
}

void TimeScheduler::unbind(EventHandler* handler)
{
	for (uint i=0; i<_entries.size(); ++i)
	{
		if (handler == _entries[i]->handler)
			_entries[i]->handler = NULL;
	}
}

void TimeScheduler::unbind(IEventSink* sink)
{
	for (uint i=0; i<_entries.size(); ++i)
	{
		if (_entries[i]->handler->hasEventSink(sink))
			_entries[i]->handler = NULL;
	}
}

void TimeScheduler::onSourceTime(const TimeEvent* evt)
{
	advance(evt->getDelta());
}

void TimeScheduler::advance(float dt)
{
	_time += dt;

	update();
}

void TimeScheduler::update()
{
	Ref<TimeScheduler> safe = this;

	int count = 0;

	while (!_entries.empty())
	{
		// Terminal condition
		if (_time < _entries[0]->wakeTime) break;

		std::pop_heap(_entries.begin(), _entries.end(), UpdateEntry::HeapCompare()); // popped entry will go tail of the vector

		// Prepare entry for update
		UpdateEntry* e = _entries.back();
		Ref<EventHandler> handler = e->handler;

		if (handler == NULL || handler->isDisposed())
		{
			// Remove from queue if handler disposed
			delete e;
			_entries.pop_back();
			continue;
		}

		float dt = _time - e->time;

		if (e->interval <= 0.0f)
		{
			// Remove from queue if one-timed 
			delete e;
			_entries.pop_back();
		}
		else
		{
			// Prepare next step
			e->time = _time;
			while (e->wakeTime <= _time)
				e->wakeTime += e->interval;

			std::push_heap(_entries.begin(), _entries.end(), UpdateEntry::HeapCompare());
		}

		// Send event - Should after heap rearrangement
		Ref<TimeEvent> evt = new TimeEvent(this, dt);
		evt->setId(Events::OnTimeSchedule);
		handler->call(evt);

		// Check update quota
		if (_updateQuota && ++count >= _updateQuota) return;
	}
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
