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

// NOTE: Do not include in your file (Event.h will include this)

////////////////////////////////////////////////////////////////////////////////

class IEventSink;

class NIT_API EventHandler : public EventRefCounted, public PooledAlloc
{
public:
	EventHandler() { }
	virtual ~EventHandler() { }

public:
	bool canHandle(EventId id) 
	{ 
		return canHandle(EventInfo::getByEventId(id)->getTypeChecker()->getTypical()); 
	}

	virtual bool canHandle(const Event* tc) = 0;

	/// return true if handler is disposed and should be removed
	virtual bool isDisposed() = 0;

	virtual void call(const Event* evt) = 0;

	virtual bool hasEventSink(IEventSink* sink) = 0;
};

////////////////////////////////////////////////////////////////////////////////

template <typename T> struct EventClassOf;

template <typename T>
struct EventClassOf< EventType<T> >
{
	typedef typename EventType<T>::EventClass type;
};

template <>
struct EventClassOf<EventId>
{
	typedef Event type;
};

////////////////////////////////////////////////////////////////////////////////

template <typename TEventType, typename TEvent, typename TContext>
class EventContextFunction : public EventHandler
{
public:
	typedef bool (*Function)(const TEvent*, TContext&);
	typedef typename EventClassOf<TEventType>::type EventClass;

public:
	EventContextFunction(Function function, const TContext& context)
		: _functionPtr((void*)function), _context(context)
	{
		compileTime_CheckEventClass(static_cast<EventClass*>(NULL));
	}

public:
	virtual bool canHandle(const Event* tc) 
	{ 
		return dynamic_cast<const EventClass*>(tc) != NULL; 
	}

	virtual void call(const Event* evt)
	{
		Function function = reinterpret_cast<Function>(_functionPtr);
		bool alive = function(static_cast<const TEvent*>(evt), _context);
		if (!alive) _functionPtr = NULL;
	}

	virtual bool isDisposed() 
	{ 
		return _functionPtr == NULL; 
	}

	virtual bool hasEventSink(IEventSink* sink)
	{
		return false;
	}

protected:
	void* _functionPtr;
	TContext _context;

	static void compileTime_CheckEventClass(TEvent *) { }
};

////////////////////////////////////////////////////////////////////////////////

template <typename TEventType, typename TEvent, typename TEventSink>
class EventMethod : public EventHandler
{
public:
	typedef typename EventClassOf<TEventType>::type EventClass;
	typedef void (TEventSink::*Method)(const TEvent*);

public:
	EventMethod(TEventSink *sink, Method method)
		: _sink(sink), _method(method)
	{
		compileTime_CheckEventClass(static_cast<EventClass*>(NULL));
	}

	virtual bool canHandle(const Event* tc) 
	{ 
		return dynamic_cast<const EventClass*>(tc) != NULL; 
	}

	virtual bool isDisposed()
	{ 
		return _sink == NULL;
	}

	virtual void call(const Event* evt)
	{
		TEventSink* sink = static_cast<TEventSink*>(_sink.get());

		if (sink) 
			(sink->*_method)(static_cast<const TEvent*>(evt));
	}

	virtual bool hasEventSink(IEventSink* sink)
	{
		return false;
	}

protected:
	Weak<TEventSink> _sink;
	Method _method;

	static void compileTime_CheckEventClass(TEvent *) { }
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API IEventSink
{
public:
	virtual ~IEventSink() { }

	virtual bool isEventActive() = 0;
	virtual bool isDisposed() = 0;
};

////////////////////////////////////////////////////////////////////////////////

template <typename TEventType, typename TEvent, typename TEventSink>
class EventSinkMethod : public EventHandler
{
public:
	typedef typename EventClassOf<TEventType>::type EventClass;
	typedef void (TEventSink::*Method)(const TEvent*);

public:
	EventSinkMethod(TEventSink *sink, Method method)
		: _sink(sink), _method(method)
	{
		compileTime_CheckEventClass(static_cast<EventClass*>(NULL));
	}

	virtual bool canHandle(const Event* tc) 
	{ 
		return dynamic_cast<const EventClass*>(tc) != NULL; 
	}

	virtual bool isDisposed() 
	{ 
		TEventSink* sink = _sink;

		return sink == NULL || sink->isDisposed();
	}

	virtual void call(const Event* evt)
	{
		TEventSink* sink = static_cast<TEventSink*>(_sink.get());

		if (sink && sink->isEventActive())
			(sink->*_method)(static_cast<const TEvent*>(evt));
	}

	virtual bool hasEventSink(IEventSink* sink) 
	{ 
		return static_cast<IEventSink*>(_sink.get()) == sink;
	}

protected:
	Weak<TEventSink> _sink;
	Method _method;

	static void compileTime_CheckEventClass(TEvent *) { }
};

////////////////////////////////////////////////////////////////////////////////

class IEventSink;

template <typename TEvent, typename TEventSink>
static EventHandler* createEventHandler(void* sink, void (TEventSink::*method)(const TEvent*))
{
	return new EventMethod<EventType<TEvent>, TEvent, TEventSink>(static_cast<TEventSink*>(sink), method);
}

template <typename TEvent, typename TEventSink>
static EventHandler* createEventHandler(IEventSink* sink, void (TEventSink::*method)(const TEvent*))
{
	return new EventSinkMethod<EventType<TEvent>, TEvent, TEventSink>(static_cast<TEventSink*>(sink), method);
}

template <typename TEvent, typename TContext>
static EventHandler* createEventHandler(bool (*function)(const TEvent*, TContext&), const TContext& context)
{
	return new EventContextFunction<EventType<TEvent>, TEvent, TContext>(function, context);
}

////////////////////////////////////////////////////////////////////////////////

