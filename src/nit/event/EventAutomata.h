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

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class EventState;

class NIT_API EventAutomata : public EventChain
{
public:
	EventAutomata();
	virtual ~EventAutomata();

public:
	EventState*							getState() const						{ return _state; }
	void								setState(EventState* state);

	bool								isEventActive()							{ return _eventActive; }
	void								setEventActive(bool flag)				{ _eventActive = flag; }

protected:
	Ref<EventState>						_state;
	bool								_entering;
	bool								_eventActive;

	virtual bool						sendLocal(const Event* evt);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API EventState : public RefCounted, public IEventSink, public PooledAlloc
{
public:
	EventState();
	virtual ~EventState();

public:
	EventAutomata*						getAutomata()							{ return _automata; }

	virtual bool						isEventActive()							{ return _automata && _automata->isEventActive(); }
	virtual bool						isDisposed()							{ return false; }

public:
	const String&						getName();
	void								setName(const String& name)				{ _name = name; }

protected:
	virtual void						onEnter()								{ }
	virtual void						onEvent(const Event* e)					{ }
	virtual void						onExit()								{ }

protected:
	virtual String						getClassName()							{ return typeid(*this).name(); }

private:
	friend class EventAutomata;
	Weak<EventAutomata>					_automata;
	String								_name;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;