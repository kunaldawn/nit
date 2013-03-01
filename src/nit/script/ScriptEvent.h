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

class NIT_API IScriptEventSink
{
public:
	virtual IEventSink* getEventSink() = 0;
	virtual WeakRef* _weak() = 0;
	virtual int push(HSQUIRRELVM v) = 0;
};

////////////////////////////////////////////////////////////////////////////////

template <typename TClass, typename TBaseClass>
class TScriptEventSink : public TBaseClass, public IScriptEventSink
{
public:
	virtual IEventSink* getEventSink()
	{
		return static_cast<IEventSink*>(static_cast<TClass*>(this));
	}

	virtual WeakRef* _weak()
	{
		return TBaseClass::_weak();
	}

	virtual int push(HSQUIRRELVM v)
	{
		return NitBind::push<TClass>(v, static_cast<TClass*>(this));
	}
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API ScriptEventHandler : public EventHandler
{
public:
	static EventHandler* get(HSQUIRRELVM v, int sinkIdx, int closureIdx)
	{
		if (NitBind::is<EventHandler>(v, sinkIdx))
			return NitBind::get<EventHandler>(v, sinkIdx);
		return create(v, sinkIdx, closureIdx);
	}

	static ScriptEventHandler* create(HSQUIRRELVM v, int sinkIdx, int closureIdx);

public: // utility methods
	virtual bool						isEventActive(HSQUIRRELVM v) = 0;
	virtual SQInteger					pushEventSink(HSQUIRRELVM v) = 0;
	virtual SQInteger					pushClosure(HSQUIRRELVM v) = 0;

	virtual bool						isSinkDisposed(HSQUIRRELVM v) = 0;
	virtual void						disposeSink(HSQUIRRELVM v) = 0;

public:
	class InvokeWithEvent
	{
	public:
		SQRESULT invoke(HSQUIRRELVM v, Event* evt)
		{
			NitBind::push(v, evt);
			return sq_call(v, 2, SQFalse, SQTrue);
		}
	};

	class InvokeWithoutEvent
	{
	public:
		SQRESULT invoke(HSQUIRRELVM v, Event* evt)
		{
			return sq_call(v, 1, SQFalse, SQTrue);
		}
	};
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API ScriptUnbindingSink : public IEventSink
{
public:
	ScriptUnbindingSink(HSQUIRRELVM v, int sinkIdx);

	~ScriptUnbindingSink();

public:
	SQRESULT							pushObject(HSQUIRRELVM v);

public:									// dummy IEventSink impl
	virtual bool						isEventActive()							{ return true; }
	virtual bool						isDisposed()							{ return false; }

private:
	HSQUIRRELVM							_vm;
	HSQOBJECT							_sink;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
