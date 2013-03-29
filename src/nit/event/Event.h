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

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

typedef uint EventId;
class Event;
class EventInfo;
class EventChain;
class EventBinder;
class EventChannel;
class EventChanneled;
class EventHandler;

////////////////////////////////////////////////////////////////////////////////

#define NIT_EVENT_DECLARE(API, NAME, TYPE) \
	namespace EVT { extern API const EventType<TYPE> NAME; }

#define NIT_EVENT_DEFINE(NAME, TYPE) \
	namespace EVT { \
		const EventInfo __EI_##NAME(0, #NAME, #TYPE, EventType<TYPE>::getTypeChecker()); \
		const EventType<TYPE> NAME(__EI_##NAME.getRuntimeId()); \
	}

#define NIT_NET_EVENT_DEFINE(SERIAL_ID, NAME, TYPE) \
	namespace EVT { \
		const EventInfo __EI_##NAME(SERIAL_ID, #NAME, #TYPE, EventType<TYPE>::getTypeChecker()); \
		const EventType<TYPE> NAME(__EI_##NAME.getRuntimeId()); \
	}

////////////////////////////////////////////////////////////////////////////////

#define NIT_CLASS_EVENT(NAME, TYPE) \
	public: const static EventType<TYPE> NAME; \
	private: const static EventInfo __EI_##NAME;

#define NIT_CLASS_EVENT_DEFINE(CLS, NAME, TYPE) \
	const EventInfo CLS::__EI_##NAME(0, #CLS "::" #NAME, #TYPE, EventType<TYPE>::getTypeChecker()); \
	const EventType<TYPE> CLS::NAME(CLS::__EI_##NAME.getRuntimeId());

#define NIT_NET_CLASS_EVENT_DEFINE(SERIAL_ID, CLS, NAME, TYPE) \
	const EventInfo CLS::__EI_##NAME(SERIAL_ID, #CLS "::" #NAME, #TYPE, EventType<TYPE>::getTypeChecker()); \
	const EventType<TYPE> CLS::NAME(CLS::__EI_##NAME.getRuntimeId());

////////////////////////////////////////////////////////////////////////////////

class NIT_API EventTypeChecker
{
public:
	virtual bool						compatible(Event* evt)					{ return true; }
	virtual Event*						getTypical();

	virtual Event*						clone(Event* evt);
	virtual void						copy(Event* src, Event* dst)			{ }
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API EventInfo
{
public:
	EventInfo(int serialID, const char* eventName, const char* typeName, EventTypeChecker* typeChecker);
	~EventInfo();

	operator EventId () const													{ return reinterpret_cast<EventId>(this); }

public:
	EventId								getRuntimeId() const					{ return reinterpret_cast<EventId>(this); }
	int 								getSerialId() const						{ return _serialId; }
	const char* 						getEventName() const					{ return _eventName; }
	const char* 						getTypeName() const						{ return _typeName; }
	EventTypeChecker*					getTypeChecker() const					{ return _typeChecker; }

public:
	static bool							checkType(EventId id, Event& evt)		{ return id && getByEventId(id)->getTypeChecker()->compatible(&evt); }

public:
	static const EventInfo*				getByEventId(EventId id)				{ return reinterpret_cast<const EventInfo*>(id); }

	static uint							getCount();
	static const EventInfo**			getArray();

	static void							throwInvalidEvent(EventId id);
	static void							throwInvalidHandler(EventId id);

public:
	typedef void (*_RegisterCallback) (EventInfo* info, void* context);
	static void							_DllLoaderSetRegisterCallback(_RegisterCallback callback, void* context);
	static void							_DLLForceUnregister(EventInfo* info);

protected:
	EventInfo();

	EventId								_serialId;
	int									_order;
	const char*							_eventName;
	const char*							_typeName;
	EventTypeChecker*					_typeChecker;

	class Registry;
	friend class Registry;

	EventInfo*							_next;
};

////////////////////////////////////////////////////////////////////////////////

// tiny ref count class compatible with Ref<> template

class NIT_API EventRefCounted // TODO : Make a proprietary memory pool for events
{
public:
	EventRefCounted() : _refCount(0)											{ }
	virtual ~EventRefCounted()													{ }

public:
	EventRefCounted*					_ref()									{ return this; }
	void								incRefCount()							{ ++_refCount; }
	void								decRefCount()							{ if (--_refCount == 0) delete this; }
	int									getRefCount()							{ return _refCount; }

private:
	int									_refCount;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API Event : public EventRefCounted, public PooledAlloc
{
public:
	Event() : _id(0), _consumed(false), _uplinking(true)						{ }
	virtual ~Event()															{ }

	inline bool							setId(EventId id)						{ if (EventInfo::checkType(id, *this)) { _id = id; return true; } return false; }

public:
	EventId								getId()	const							{ return _id; }

	bool 								isConsumed() const						{ return _consumed; }
	bool 								isUplinking() const						{ return _uplinking; }

	void 								consume(bool consume = true) const		{ _consumed = consume; }
	void 								uplink(bool uplinking = true) const		{ _uplinking = uplinking; }

private:
	EventId								_id;
	mutable bool 						_consumed : 1;
	mutable bool 						_uplinking : 1;
};

////////////////////////////////////////////////////////////////////////////////

template <typename T>
class EventType
{
public:
	typedef T EventClass;

	EventType(EventId id)														{ _id = id; }
	EventType(const EventInfo* info)											{ _id = info->getSerialId(); }

	operator const EventId&	() const											{ return _id; }

public:
	EventId								getId() const							{ return _id; }
	const EventInfo*					getInfo() const							{ return reinterpret_cast<const EventInfo*>(_id); }

public:
	class TypeChecker : public EventTypeChecker
	{
		virtual bool					compatible(Event* evt)					{ return dynamic_cast<const EventClass*>(evt) != NULL; }
		virtual Event*					getTypical()							{ return &_typical; }
		EventClass						_typical;

		virtual Event* clone(Event* evt)
		{
			EventClass* n = new EventClass();
			*n = *static_cast<EventClass*>(evt);
			return n;
		}

		virtual void copy(Event* src, Event* dst)
		{
			*static_cast<EventClass*>(dst) = *static_cast<EventClass*>(src);
		}
	};

	static EventTypeChecker*			getTypeChecker()						{ static TypeChecker tc; return &tc; }

private:
	EventId								_id;
};

////////////////////////////////////////////////////////////////////////////////

#include "EventHandler.h"

////////////////////////////////////////////////////////////////////////////////

class NIT_API EventChain : public RefCounted
{
public:
	EventChain();

public:
	void								send(EventId id, Event* evt);
	void								sendLocal(EventId id, Event* evt);

	void								uplink(EventChain* uplink);
	void								disconnect(EventChain* uplink);

protected:
	void								send(const Event* evt);
	virtual bool						sendLocal(const Event* evt) = 0;

public:
	class Impl : public RefCounted, public PooledAlloc
	{
	public:
		virtual bool 					isEmpty() = 0;
		virtual bool 					isFull() = 0;
		virtual void 					send(const Event* evt) = 0;
		virtual void 					connect(EventChain* uplink) = 0;
		virtual void 					disconnect(EventChain* uplink) = 0;
		virtual Impl*					expand() = 0;
	};

	class SingleImpl;
	class ManyImpl;

	Ref<Impl>							_impl;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API IEventBinder
{
public:
	// Bind an object method (not type-specific)
	template <typename TEventType, typename TEvent, typename TEventSink>
	EventHandler*						bind(const TEventType& eventType, void* sink, void (TEventSink::*method)(const TEvent*));

	// Bind an EventSink method
	template <typename TEventType, typename TEvent, typename TEventSink>
	EventHandler*						bind(const TEventType& eventType, IEventSink* sink, void (TEventSink::*method)(const TEvent*));

	// Bind a static function with context object
	template <typename TEventType, typename TEvent, typename TContext>
	EventHandler*						bind(const TEventType& eventType, bool (*function)(const TEvent*, TContext&), const TContext& context);

	// Bind an IEventHandler implementation
	EventHandler*						bind(EventId eventID, EventHandler* handler) { return doBind(eventID, handler); }

	// Unbind : Remove the handler which obtained from Bind() (use zero if you don't care about ID)
	void 								unbind(EventId id, EventHandler* handler) { doUnbind(id, handler); }

	// Unbind : Remove all handlers of an EventSink by ID (zero ID means ALL regardless ID)
	void								unbind(EventId id, IEventSink* sink) { doUnbind(id, sink); }

protected:
	virtual EventHandler*				doBind(EventId id, EventHandler* handler) = 0;
	virtual void						doUnbind(EventId id, EventHandler* handler) = 0;
	virtual void						doUnbind(EventId id, IEventSink* sink) = 0;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API EventBinder : public RefCounted, public IEventBinder, public PooledAlloc
{
public:
	EventBinder();
	virtual ~EventBinder();

public:
	void								send(EventId id, Event* evt);

protected:
	virtual bool						sendLocal(const Event* evt);

protected:
	virtual EventHandler*				doBind(EventId id, EventHandler* handler);
	virtual void						doUnbind(EventId id, EventHandler* handler);
	virtual void						doUnbind(EventId id, IEventSink* sink);

private:
	class Impl : public RefCounted, public PooledAlloc
	{
	public:
		virtual bool 					isEmpty() = 0;
		virtual bool 					isFull() = 0;
		virtual void 					bind(EventHandler* handler, EventId eventID) = 0;
		virtual void 					unbind(EventHandler* handler, EventId id) = 0;
		virtual void 					unbind(IEventSink* sink, EventId id) = 0;
		virtual void 					sendLocal(const Event* evt) = 0;
		virtual Impl*					expand() = 0;
	};

	class SingleImpl;
	class ManyImpl;

	Impl*								needMore();
	Ref<Impl>							_impl;

	friend class EventChannel;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API EventChannel : public EventChain, public IEventBinder, public PooledAlloc
{
public:
	virtual bool						sendLocal(const Event* evt);

protected:
	virtual bool						preFilter(const Event* evt)				{ return true; }
	virtual bool						postFilter(const Event* evt)			{ return true; }

public:
	EventBinder*						priority(float priority);

protected:
	virtual EventHandler*				doBind(EventId id, EventHandler* handler)	{ return priority(0.0f)->doBind(id, handler); }
	virtual void						doUnbind(EventId id, EventHandler* handler);
	virtual void						doUnbind(EventId id, IEventSink* sink);

private:
	class Impl : public RefCounted, public PooledAlloc
	{
	public:
		virtual EventBinder*			find(float priority) = 0;
		virtual bool					sendLocal(const Event* evt) = 0;
		virtual Impl*					expand() = 0;
		virtual void					unbind(EventId id, EventHandler* handler) = 0;
		virtual void					unbind(EventId id, IEventSink* sink) = 0;
	};

	class SingleImpl;
	class ManyImpl;

	Ref<Impl>							_impl;
};

////////////////////////////////////////////////////////////////////////////////

/// IEventBinder template implemenatation

template <typename TEventType, typename TEvent, typename TEventSink>
EventHandler* IEventBinder::bind(const TEventType& eventType, void* sink, void (TEventSink::*method)(const TEvent*)) 
{ 
	return doBind(eventType, new EventMethod<TEventType, TEvent, TEventSink>(static_cast<TEventSink*>(sink), method)); 
}

template <typename TEventType, typename TEvent, typename TEventSink>
EventHandler* IEventBinder::bind(const TEventType& eventType, IEventSink* sink, void (TEventSink::*method)(const TEvent*)) 
{ 
	return doBind(eventType, new EventSinkMethod<TEventType, TEvent, TEventSink>(static_cast<TEventSink*>(sink), method)); 
}

template <typename TEventType, typename TEvent, typename TContext>
EventHandler* IEventBinder::bind(const TEventType& eventType, bool (*function)(const TEvent*, TContext&), const TContext& context)
{ 
	return doBind(eventType, new EventContextFunction<TEventType, TEvent, TContext>(function, context)); 
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;