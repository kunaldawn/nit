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

#include "nit/script/ScriptRuntime.h"
#include "nit/script/NitBind.h"
#include "nit/script/NitBindMacro.h"
#include "nit/script/ScriptEvent.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

ScriptEvent::ScriptEvent(HSQUIRRELVM v, int peerIdx)
{
	_peer = new ScriptPeer(v, peerIdx);
}

////////////////////////////////////////////////////////////////////////////////

template <typename InvokeImpl>
class TScriptEventClosure : public ScriptEventHandler
{
public:
	TScriptEventClosure(HSQUIRRELVM v, int sinkIdx, int closureIdx)
	{
		_runtime = ScriptRuntime::getRuntime(v);

		SQObjectType sinkType = sq_gettype(v, sinkIdx);
		if (SQ_ISWEAKREF(sinkType))
		{
			sq_resetobject(&_weakSink);
			sq_getstackobj(v, sinkIdx, &_weakSink);
			sq_addref(v, &_weakSink);
		}
		else
		{
			sq_weakref(v, sinkIdx, true);
			sq_resetobject(&_weakSink);
			sq_getstackobj(v, -1, &_weakSink);
			sq_addref(v, &_weakSink);
			sq_poptop(v);
		}

		sq_resetobject(&_closure);
		sq_getstackobj(v, closureIdx, &_closure);
		sq_addref(v, &_closure);
	}

	virtual ~TScriptEventClosure()
	{
		Dispose();
	}

public:
	virtual void Dispose()
	{
		if (_runtime && _runtime->isStarted())
		{
			HSQUIRRELVM v = _runtime->getWorker();
			sq_release(v, &_weakSink);
			sq_release(v, &_closure);
			_runtime = NULL;
		}
	}

	virtual bool canHandle(const Event* tc) 
	{ 
		return true; 
	}

	virtual bool isDisposed() 
	{ 
		return _runtime == NULL || !_runtime->isStarted(); 
	}

	virtual bool hasEventSink(IEventSink* sink) 
	{ 
		ScriptUnbindingSink* ubsink = dynamic_cast<ScriptUnbindingSink*>(sink);
		if (ubsink == NULL) return false;

		HSQUIRRELVM v = _runtime->getWorker();
		if (v == NULL) return false;

		SQInteger clear = sq_gettop(v);

		sq_pushobject(v, _weakSink);
		if (SQ_ISWEAKREF(_weakSink._type))
		{
			sq_getweakrefval(v, -1);
			sq_replace(v, -2);
		}

		ubsink->pushObject(v);
		bool same = sq_cmp(v) == 0;

		sq_settop(v, clear);

		return same;
	}

	virtual void call(const Event* original)
	{
		if (isDisposed()) return;

		Event* evt = const_cast<Event*>(original);

		HSQUIRRELVM v = _runtime->getWorker();
		SQInteger clear = sq_gettop(v);
		SQRESULT r;

		sq_pushobject(v, _closure);

		sq_pushobject(v, _weakSink);
		if (SQ_ISWEAKREF(_weakSink._type))
		{
			sq_getweakrefval(v, -1);
			sq_replace(v, -2);

			if (sq_gettype(v, -1) == OT_NULL)
			{
				// If the weak vaporized: treat as sink disposed.
				sq_settop(v, clear);
				Dispose();
				return;
			}
		}

		Ref<EventHandler> safe = this;

		r = InvokeImpl().invoke(v, evt);

		if (SQ_FAILED(r))
			error(v, evt);

		sq_settop(v, clear);
	}

	virtual SQRESULT invokeClosure(HSQUIRRELVM v, Event* evt)
	{
		NitBind::push(v, evt);
		return sq_call(v, 2, SQFalse, SQTrue);
	}

	void error(HSQUIRRELVM v, const Event* evt)
	{
		SQInteger top = sq_gettop(v);

		const char* errobj = "???";
		const char* lasterr = "???";

		sq_pushobject(v, _weakSink);
		if (SQ_ISWEAKREF(_weakSink._type))
		{
			sq_getweakrefval(v, -1);
			sq_replace(v, -2);
		}
		sq_tostring(v, -1);
		sq_getstring(v, -1, &errobj);

		sq_getlasterror(v);
		sq_tostring(v, -1);
		sq_getstring(v, -1, &lasterr);

		if (evt)
		{
			const EventInfo* info = EventInfo::getByEventId(evt->getId());
			LOG(0, "*** event handler %s.%s(%s): %s\n", errobj, info->getEventName(), info->getTypeName(), lasterr);
		}
		else
		{
			LOG(0, "*** event handler %s(null): %s\n", errobj, lasterr);
		}

		sq_settop(v, top);
	}

	// utility methods
	virtual bool isEventActive(HSQUIRRELVM v)
	{
		return !isSinkDisposed(v);
	}

	virtual SQInteger pushEventSink(HSQUIRRELVM v)
	{
		if (isDisposed()) return 0;

		sq_pushobject(v, _weakSink);
		if (SQ_ISWEAKREF(_weakSink._type))
		{
			sq_getweakrefval(v, -1);
			sq_replace(v, -2);
		}

		return 1;
	}

	SQInteger pushClosure(HSQUIRRELVM v)
	{
		if (isDisposed()) return 0;

		sq_pushobject(v, _closure);
		return 1;
	}

	virtual bool isSinkDisposed(HSQUIRRELVM v)
	{
		if (isDisposed()) return true;

		SQInteger top = sq_gettop(v);

		bool disposed = false;

		sq_pushobject(v, _weakSink);
		if (SQ_ISWEAKREF(_weakSink._type))
		{
			sq_getweakrefval(v, -1);
			sq_replace(v, -2);

			// If the weak vaporized: treat as sink disposed.
			if (sq_gettype(v, -1) == OT_NULL)
				disposed = true;
		}
		
		sq_settop(v, top);

		return disposed;
	}

	virtual void disposeSink(HSQUIRRELVM v)
	{
		Dispose();
	}

	Weak<ScriptRuntime> _runtime;
	HSQOBJECT _weakSink;
	HSQOBJECT _closure;
};

////////////////////////////////////////////////////////////////////////////////

template <typename InvokeImpl>
class TScriptEventSinkHandler : public ScriptEventHandler
{
public:
	TScriptEventSinkHandler(HSQUIRRELVM v, IScriptEventSink* sink, int closureIdx)
	{
		_runtime = ScriptRuntime::getRuntime(v);
		_sink = sink;

		if (_sink == NULL)
			sqx_throw(v, "invalid native sink");

		sq_resetobject(&_closure);
		sq_getstackobj(v, closureIdx, &_closure);
		sq_addref(v, &_closure);
	}

	virtual ~TScriptEventSinkHandler()
	{
		Dispose();
	}

public:
	virtual void Dispose()
	{
		if (_runtime && _runtime->isStarted())
		{
			HSQUIRRELVM v = _runtime->getWorker();
			sq_release(v, &_closure);
			_sink = NULL;
			_runtime = NULL;
		}
	}

	virtual bool canHandle(const Event* tc) 
	{ 
		return true; 
	}

	virtual bool isDisposed() 
	{ 
		return _runtime == NULL || !_runtime->isStarted() || _sink == NULL || _sink->getEventSink()->isDisposed();
	}

	virtual bool hasEventSink(IEventSink* sink)
	{
		return _sink && _sink->getEventSink() == sink;
	}

	virtual void call(const Event* original)
	{
		if (isDisposed()) return;

		if (!_sink->getEventSink()->isEventActive()) return;

		Event* evt = const_cast<Event*>(original);

		HSQUIRRELVM v = _runtime->getWorker();
		SQInteger clear = sq_gettop(v);
		SQRESULT r;

		sq_pushobject(v, _closure);

		_sink->push(v);

		Ref<EventHandler> safe = this;

		r = InvokeImpl().invoke(v, evt);

		if (SQ_FAILED(r))
			error(v, evt);

		sq_settop(v, clear);
	}

	void error(HSQUIRRELVM v, const Event* evt)
	{
		SQInteger top = sq_gettop(v);

		const char* errobj = "???";
		const char* lasterr = "???";

		_sink->push(v);
		sq_tostring(v, -1);
		sq_getstring(v, -1, &errobj);

		sq_getlasterror(v);
		sq_tostring(v, -1);
		sq_getstring(v, -1, &lasterr);
		
		if (evt)
		{
			const EventInfo* info = EventInfo::getByEventId(evt->getId());
			LOG(0, "*** event handler %s.%s(%s): %s\n", errobj, info->getEventName(), info->getTypeName(), lasterr);
		}
		else
		{
			LOG(0, "*** event handler %s(null): %s\n", errobj, lasterr);
		}

		sq_settop(v, top);
	}

	// utility methods
	virtual bool isEventActive(HSQUIRRELVM v)
	{
		return !isDisposed() && _sink->getEventSink()->isEventActive();
	}

	SQInteger pushEventSink(HSQUIRRELVM v)
	{
		if (isDisposed()) return 0;

		return _sink->push(v);
	}

	SQInteger pushClosure(HSQUIRRELVM v)
	{
		if (isDisposed()) return 0;

		sq_pushobject(v, _closure);
		return 1;
	}

	virtual bool isSinkDisposed(HSQUIRRELVM v)
	{
		return isDisposed();
	}

	virtual void disposeSink(HSQUIRRELVM v)
	{
		Dispose();
	}

private:
	Weak<ScriptRuntime> _runtime;
	IWeak<IScriptEventSink> _sink;
	HSQOBJECT _closure;
};

////////////////////////////////////////////////////////////////////////////////

ScriptEventHandler* ScriptEventHandler::create(HSQUIRRELVM v, int sinkIdx, int closureIdx)
{
	// Check the number of parameter of the closure
	SQUnsignedInteger nparams = 0, nouters = 0;
	sq_getclosureinfo(v, closureIdx, &nparams, &nouters);
	if (nparams != 1 && nparams != 2)
		sqx_throw(v, "invalid closure");

	// Check if sink is an instance or has a IEventSinkQuery interface
	IScriptEventSink* sink = NULL;

	sq_push(v, sinkIdx);

	if (sq_gettype(v, -1) == OT_WEAKREF)
	{
		sq_getweakrefval(v, -1);
		sq_replace(v, -2);
	}

	if (sq_gettype(v, -1) == OT_INSTANCE)
	{
		void* up = NULL;
		sq_getinstanceup(v, -1, &up, NULL);
		if (up)
		{
			sink = NitBind::getInterface<IScriptEventSink>(v, -1, true);
			if (sink == NULL)
				sqx_throw(v, "invalid native sink");
		}
	}
	sq_poptop(v);

	if (sink)
	{
		if (nparams == 1)
			return new TScriptEventSinkHandler<InvokeWithoutEvent>(v, sink, closureIdx);
		else
			return new TScriptEventSinkHandler<InvokeWithEvent>(v, sink, closureIdx);
	}
	else
	{
		if (nparams == 1)
			return new TScriptEventClosure<InvokeWithoutEvent>(v, sinkIdx, closureIdx);
		else
			return new TScriptEventClosure<InvokeWithEvent>(v, sinkIdx, closureIdx);
	}
}

////////////////////////////////////////////////////////////////////////////////

ScriptUnbindingSink::ScriptUnbindingSink(HSQUIRRELVM v, int sinkIdx)
{
	_vm = v;

	SQObjectType sinkType = sq_gettype(v, sinkIdx);
	if (SQ_ISWEAKREF(sinkType))
	{
		sq_getweakrefval(v, sinkIdx);
		sq_resetobject(&_sink);
		sq_getstackobj(v, -1, &_sink);
		sq_addref(v, &_sink);
		sq_poptop(v);
	}
	else
	{
		sq_resetobject(&_sink);
		sq_getstackobj(v, sinkIdx, &_sink);
		sq_addref(v, &_sink);
	}
}

ScriptUnbindingSink::~ScriptUnbindingSink()
{
	sq_release(_vm, &_sink);
}

SQRESULT ScriptUnbindingSink::pushObject(HSQUIRRELVM v)
{
	sq_pushobject(v, _sink);
	return 1;
}

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::Event, RefCounted, incRefCount, decRefCount);

class NB_Event : TNitClass<Event>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(id),
			PROP_ENTRY_R(consumed),
			PROP_ENTRY_R(uplinking),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"),
			FUNC_ENTRY_H(consume,		"(consume=true)"),
			FUNC_ENTRY_H(uplink,		"(uplinking=true)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(id)						{ return push(v, const_cast<EventInfo*>(EventInfo::getByEventId(self(v)->getId()))); }
	NB_PROP_GET(consumed)				{ return push(v, self(v)->isConsumed()); }
	NB_PROP_GET(uplinking)				{ return push(v, self(v)->isUplinking()); }

	NB_CONS()							{ sq_setinstanceup(v, 1, new Event()); return 0; }

	NB_FUNC(consume)					{ self(v)->consume(optBool(v, 2, true)); return 0; }
	NB_FUNC(uplink)						{ self(v)->uplink(optBool(v, 2, true)); return 0; }
};

//////////////////////////////////////////////////////////////////////////////

NB_TYPE_RAW_PTR(NIT_API, nit::EventInfo, NULL);

class NB_EventInfo : TNitClass<EventInfo>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(runtimeId),
			PROP_ENTRY_R(serialId),
			PROP_ENTRY_R(eventName),
			PROP_ENTRY_R(typeName),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(name, type: string)"),
			FUNC_ENTRY	(_tostring),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(runtimeId)				{ return push(v, self(v)->getRuntimeId()); }
	NB_PROP_GET(serialId)				{ return push(v, self(v)->getSerialId()); }
	NB_PROP_GET(eventName)				{ return push(v, self(v)->getEventName()); }
	NB_PROP_GET(typeName)				{ return push(v, self(v)->getTypeName()); }

	class ScriptEventInfo : public EventInfo
	{
	public:
		ScriptEventInfo(const char* eventName, const char* typeHint)
		{
			_eventNameStr = eventName;
			_typeHintStr = typeHint;

			_eventName = _eventNameStr.c_str();
			_typeName = _typeHintStr.c_str();
		}

		String _eventNameStr;
		String _typeHintStr;
	};

	NB_CONS()
	{
		ScriptEventInfo* info = new ScriptEventInfo(getString(v, 2), getString(v, 3));
		ScriptRuntime* runtime = ScriptRuntime::getRuntime(v);

		runtime->registerScriptEventInfo(info);
		setSelf(v, info);

		return 0;
	}

	NB_FUNC(_tostring)
	{
		type* o = self(v);
		return pushFmt(v, "(EventInfo: %s(%s))", o->getEventName(), o->getTypeName());
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::EventHandler, RefCounted, incRefCount, decRefCount);

class NB_EventHandler : TNitClass<EventHandler>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(disposed),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(call,			"(evt: Event)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(disposed)				{ return push(v, self(v)->isDisposed()); }

	NB_FUNC(call)						{ self(v)->call(get<Event>(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::ScriptEvent, Event, incRefCount, decRefCount);

class NB_ScriptEvent : TNitClass<ScriptEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] = 
		{
			PROP_ENTRY_R(peer),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(peer)"),
			FUNC_ENTRY	(_get),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(peer)
	{
		ScriptPeer* peer = self(v)->getPeer();
		return peer ? peer->pushObject(v) : 0;
	}

	NB_CONS()
	{
		if (isNone(v, 2))
			return sq_throwerror(v, "peer expected");

		setSelf(v, new ScriptEvent(v, 2));
		return 0;
	}

	NB_FUNC(_get)
	{
		type* o = self_noThrow(v);
		ScriptPeer* peer = o ? o->getPeer() : NULL;
		if (peer)
		{
			peer->pushObject(v);
			sq_push(v, 2);
			if (SQ_SUCCEEDED(sq_get(v, -2)))
			{
				sq_replace(v, -2);
				return 1;
			}
			sq_poptop(v);
		}

		// not found - clean failure
		sq_reseterror(v);
		return SQ_ERROR;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::ScriptEventHandler, EventHandler, incRefCount, decRefCount);

class NB_ScriptEventHandler : TNitClass<ScriptEventHandler>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(active),
			PROP_ENTRY_R(sink),
			PROP_ENTRY_R(disposed),
			PROP_ENTRY_R(closure),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(sink, closure)"),
			FUNC_ENTRY_H(dispose,		"() // purges instance"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(active)					{ return push(v, self(v)->isEventActive(v)); }
	NB_PROP_GET(sink)					{ return self(v)->pushEventSink(v); }
	NB_PROP_GET(disposed)				{ return push(v, self(v)->isDisposed()); }
	NB_PROP_GET(closure)				{ return self(v)->pushClosure(v); }

	NB_CONS()							{ setSelf(v, ScriptEventHandler::create(v, 2, 3)); return 0; }

	NB_FUNC(dispose)					{ self(v)->disposeSink(v); sq_purgeinstance(v, 1); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::EventChain, RefCounted, incRefCount, decRefCount);

class NB_EventChain : TNitClass<EventChain>
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
			FUNC_ENTRY_H(send,			"(id: EventInfo, evt: Event)"),
			FUNC_ENTRY_H(sendLocal,		"(id: EventInfo, evt: Event)"),
			FUNC_ENTRY_H(uplink,		"(chain: EventChain)"),
			FUNC_ENTRY_H(disconnect,	"(chain: EventChain)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_FUNC(send)						{ self(v)->send(get<EventInfo>(v, 2)->getRuntimeId(), get<Event>(v, 3)); return 0; }
	NB_FUNC(sendLocal)					{ self(v)->sendLocal(get<EventInfo>(v, 2)->getRuntimeId(), get<Event>(v, 3)); return 0; }
	NB_FUNC(uplink)						{ self(v)->uplink(get<EventChain>(v, 2)); return 0; }
	NB_FUNC(disconnect)					{ self(v)->disconnect(get<EventChain>(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

class NB_IEventBinder : TNitInterface<IEventBinder>
{
public:
	template <typename TClass>
	static void Register(HSQUIRRELVM v)
	{
		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(bind,			"(info: EventInfo, handler: EventHandler): EventHandler"
			"\n"						"(info: EventInfo, sink, closure): EventHandler"
			"\n"						"(info: EventInfo, sink): EventHandler // closure default to sink[info.EventName]"),
			FUNC_ENTRY_H(unbind,		"(id: EventInfo, handler: EventHandler) // when id=null, unbind all that handler regardless id"),
			NULL
		};

		bind<TClass>(v, NULL, funcs);
	}

	NB_FUNC(bind)
	{
		EventInfo* info = get<EventInfo>(v, 2);

		if (is<EventHandler>(v, 3))
			return push(v, self(v)->bind(info->getRuntimeId(), get<EventHandler>(v, 3)));

		if (isNone(v, 4))
		{
			push(v, info->getEventName());
			if (SQ_FAILED(sq_get(v, 3)))
			{
				sq_tostring(v, 3);
				const char* sinkstr = "???";
				sq_getstring(v, -1, &sinkstr);
				return sq_throwfmt(v, "can't find '%s' from event sink '%s'", info->getEventName(), sinkstr);
			}
		}

		return push(v, self(v)->bind(info->getRuntimeId(), ScriptEventHandler::create(v, 3, 4)));
	}

	NB_FUNC(unbind)					
	{ 
		EventInfo* info = opt<EventInfo>(v, 2, NULL);
		EventId id = info ? info->getRuntimeId() : 0;

		if (is<EventHandler>(v, 3))
		{
			self(v)->unbind(id, get<EventHandler>(v, 3)); 
			return 0;
		}
		else if (sq_gettype(v, 3) == OT_INSTANCE)
		{
			IEventSink* sink = getInterface<IEventSink>(v, 3, true);
			if (sink) 
			{
				self(v)->unbind(id, sink);
				return 0;
			}
		}

		ScriptUnbindingSink ubsink(v, 3);
		self(v)->unbind(id, &ubsink);

		return 0; 
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::EventBinder, RefCounted, incRefCount, decRefCount);

class NB_EventBinder : TNitClass<EventBinder>
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
			FUNC_ENTRY_H(send,			"(id: EventInfo, evt: Event)"),
			NULL
		};

		bind(v, props, funcs);

		NB_IEventBinder::Register<type>(v);
	}

	NB_CONS()							{ sq_setinstanceup(v, 1, new EventBinder()); return 0; }

	NB_FUNC(send)						{ self(v)->send(get<EventInfo>(v, 2)->getRuntimeId(), get<Event>(v, 3)); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::EventChannel, EventChain, incRefCount, decRefCount);

class NB_EventChannel : TNitClass<EventChannel>
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

			FUNC_ENTRY_H(priority,		"(priority: float): EventBinder"),
			NULL
		};

		bind(v, props, funcs);

		NB_IEventBinder::Register<type>(v);
	}

	NB_CONS()							{ sq_setinstanceup(v, 1, new EventChannel()); return 0; }

	NB_FUNC(priority)					{ return push(v, self(v)->priority(optFloat(v, 2, 0.0f))); }
};

////////////////////////////////////////////////////////////////////////////////

SQRESULT NitLibEvent(HSQUIRRELVM v)
{
	NB_Event::Register(v);
	NB_EventInfo::Register(v);
	NB_EventHandler::Register(v);
	NB_EventChain::Register(v);
	NB_EventBinder::Register(v);
	NB_EventChannel::Register(v);

	NB_ScriptEvent::Register(v);
	NB_ScriptEventHandler::Register(v);

	// Rename EventHandler to NativeEventHandler, ScriptEventHandler to EventHandler
	sq_dostring(v, "nit.NativeEventHandler := nit.EventHandler");
	sq_dostring(v, "nit.EventHandler := nit.ScriptEventHandler");

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;