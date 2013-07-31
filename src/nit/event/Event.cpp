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

#include "nit/event/Event.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////

class EventInfo::Registry
{
public:
	Registry()
	{
		_head = NULL;
		_nextOrder = 1;
		_numEvents = 0;
		_array = NULL;
		_callback = NULL;
		_callbackContext = NULL;
	}

	~Registry()
	{
	}

	EventInfo* Register(EventInfo* e, int serialID)
	{
		if (_array)
		{
			delete[] _array;
			_array = NULL;
		}

		e->_serialId = serialID;
		_numEvents++;
		e->_order = _nextOrder++;

		EventInfo* next = _head;
		_head = e;

		if (_callback) _callback(e, _callbackContext);

		return next;
	}

	void invalidateArray()
	{
		if (_array)
		{
			delete[] _array;
			_array = NULL;
		}
	}

	void unregister(EventInfo* e)
	{
		// TODO: Check if Registry get destroyed before an EventInfo.

		EventInfo*& head = _head;

		if (head == e)
		{
			head = e->_next;
			--_numEvents;
			return invalidateArray();
		}

		EventInfo* p = head;
		EventInfo* r = head->_next;

		while (r)
		{
			if (e == r)
			{
				p->_next = r->_next;
				--_numEvents;
				return invalidateArray();
			}

			p = r;
			r = r->_next;
		}
	}

	struct OrderCompare
	{
		bool operator() (EventInfo* a, EventInfo* b)
		{
			return a->_order < b->_order;
		}
	};

	const EventInfo** getArray()
	{
		if (_array == NULL)
		{
			_array = new EventInfo* [_numEvents];

			uint count = 0;
			for (EventInfo* e = _head; e; e = e->_next)
			{
				_array[count++] = e;
			}

			ASSERT(count == _numEvents);

			std::sort(_array, _array + count, OrderCompare());
		}

		return const_cast<const EventInfo**>(_array);
	}

	int _nextOrder;
	uint _numEvents;
	EventInfo* _head;
	EventInfo** _array;

	EventInfo::_RegisterCallback _callback;
	void* _callbackContext;

	static Registry* getSingleton()
	{
		static Registry s_Instance;
		return &s_Instance;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////

EventInfo::EventInfo()
: _serialId(0), _eventName(NULL), _typeName(NULL)
{
	static EventTypeChecker s_DummyChecker;
	_typeChecker = &s_DummyChecker;

	_next = NULL;
}

EventInfo::EventInfo(int serialID, const char* eventName, const char* typeName, EventTypeChecker* typeChecker)
: _eventName(eventName), _typeName(typeName), _typeChecker(typeChecker)
{
	Registry* reg = Registry::getSingleton();

	_next = reg->Register(this, serialID);
}

EventInfo::~EventInfo()
{
	Registry* reg = Registry::getSingleton();

	if (reg) reg->unregister(this);
}

uint EventInfo::getCount()
{
	Registry* reg = Registry::getSingleton();

	return reg->_numEvents;
}

const EventInfo** EventInfo::getArray()
{
	Registry* reg = Registry::getSingleton();

	return reg->getArray();
}

void EventInfo::throwInvalidEvent(EventId id)
{
	const char* evtname = id ? getByEventId(id)->getEventName() : "(null)";
	NIT_THROW_FMT(EX_INVALID_PARAMS, "invalid event for '%s'", evtname);
}

void EventInfo::throwInvalidHandler(EventId id)
{
	const char* evtname = id ? getByEventId(id)->getEventName() : "(null)";
	NIT_THROW_FMT(EX_INVALID_PARAMS, "invalid handler for '%s'", evtname);
}

void EventInfo::_DllLoaderSetRegisterCallback( _RegisterCallback callback, void* context )
{
	Registry* reg = Registry::getSingleton();

	ASSERT_THROW(callback == NULL || reg->_callback == NULL, EX_INVALID_STATE);

	reg->_callback = callback;
	reg->_callbackContext = context;
}

void EventInfo::_DLLForceUnregister( EventInfo* info )
{
	Registry* reg = Registry::getSingleton();

	reg->unregister(info);
}

////////////////////////////////////////////////////////////////////////////////

Event* EventTypeChecker::getTypical()
{
	static Event evt; 
	return &evt;
}

Event* EventTypeChecker::clone(Event* evt)
{
	return new Event();
}

////////////////////////////////////////////////////////////////////////////////

class EventChain::SingleImpl : public EventChain::Impl
{
public:
	virtual bool isEmpty()
	{
		return _chain == NULL;
	}

	virtual bool isFull()
	{
		return _chain != NULL;
	}

	virtual void send(const Event* evt)
	{
		if (_chain)
			_chain->send(evt);
	}

	virtual void connect(EventChain* uplink)
	{
		assert(_chain == NULL);

		_chain = uplink;
	}

	virtual void disconnect(EventChain* uplink)
	{
		if (_chain == uplink)
			_chain = NULL;
	}

	virtual Impl* expand();

	Weak<EventChain> _chain;
};

////////////////////////////////////////////////////////////////////////////////

class EventChain::ManyImpl : public EventChain::Impl
{
public:
	ManyImpl(SingleImpl* single)
	{
		connect(single->_chain);
	}

	virtual bool isEmpty() 
	{ 
		return _chains.empty(); 
	}

	virtual bool isFull()
	{
		return false;
	}

	virtual void send(const Event* evt)
	{
		Ref<Impl> safe = this;

		bool invalidDetected = false;

		_safeItr.begin(_chains);

		while (_safeItr.hasNext())
		{
			EventChain* chain = _safeItr.next();

			if (chain == NULL)
				invalidDetected = true;
			else
				chain->send(evt);

			if (evt->isConsumed()) break;
		}

		_safeItr.end();

		if (!_safeItr.hasBegun() && invalidDetected)
			removeInvalids();
	}

	void removeInvalids()
	{
		for (Chains::iterator curr, itr = _chains.begin(); itr != _chains.end();)
		{
			curr = itr++;
			EventChain* chain = *curr;
			if (chain == NULL)
			{
				_safeItr.onErase(curr);
				_chains.erase(curr);
			}
		}
	}

	virtual void connect(EventChain* uplink)
	{
		_safeItr.onInsert(_chains.insert(uplink));
	}

	virtual void disconnect(EventChain* uplink)
	{
		Chains::iterator itr = _chains.find(uplink);
		if (itr != _chains.end())
		{
			_safeItr.onErase(itr);
			_chains.erase(itr);
		}
	}

	virtual Impl* expand();

	typedef set<Weak<EventChain> >::type Chains;
	Chains								_chains;
	SafeItr<Chains>						_safeItr;
};

///////////////////////////////////////////////////////////////////////////////////////////

EventChain::Impl* EventChain::SingleImpl::expand()
{
	return new ManyImpl(this);
}

EventChain::Impl* EventChain::ManyImpl::expand()
{
	assert(false);
	return this;
}

///////////////////////////////////////////////////////////////////////////////////////////

EventChain::EventChain()
{
}

void EventChain::send(EventId id, Event* evt)
{
	Ref<Event> safe = evt;

	if (!evt->setId(id))
		EventInfo::throwInvalidEvent(id);

	send(evt);
}

void EventChain::send(const Event* evt)
{
	if (sendLocal(evt) && _impl && !evt->isConsumed() && evt->isUplinking()) _impl->send(evt);
}

void EventChain::sendLocal(EventId id, Event* evt)
{
	Ref<Event> safe = evt;

	if (!evt->setId(id))
		EventInfo::throwInvalidEvent(id);

	sendLocal(evt);
}

void EventChain::uplink(EventChain* uplink)
{
	if (_impl == NULL)
		_impl = new SingleImpl();

	if (_impl->isFull())
		_impl = _impl->expand();

	_impl->connect(uplink);
}

void EventChain::disconnect(EventChain* uplink)
{
	if (_impl == NULL)
		return;

	_impl->disconnect(uplink);

	if (_impl->isEmpty())
		_impl = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////

class EventBinder::SingleImpl : public EventBinder::Impl
{
public:
	SingleImpl()
	{
		_id = 0;
		_handler = NULL;
	}

	virtual bool isEmpty()
	{
		return _handler == NULL;
	}

	virtual bool isFull()
	{
		return _handler != NULL;
	}

	virtual void bind(EventHandler* handler, EventId eventID)
	{
		assert(_handler == NULL);

		_id = eventID;
		_handler = handler;
	}

	virtual void unbind(EventHandler* handler, EventId id)
	{
		if (_handler == NULL) return;

		if ( (id == 0 || _id == id) && handler == _handler)
			_handler = NULL;
	}

	virtual void unbind(IEventSink* sink, EventId id)
	{
		if (_handler == NULL) return;

		if ( (id == 0 || _id == id) && _handler->hasEventSink(sink))
			_handler = NULL;
	}

	virtual void sendLocal(const Event* evt)
	{
		if (evt->getId() != _id) return;
		if (_handler == NULL) return;

		Ref<Impl> safe = this;
		Ref<EventHandler> h = _handler;

		h->call(evt);

		if (h->isDisposed())
			_handler = NULL;
	}

	virtual Impl* expand();

	EventId _id;
	Ref<EventHandler> _handler;
};

////////////////////////////////////////////////////////////////////////////////////////////

class EventBinder::ManyImpl : public EventBinder::Impl
{
public:
	ManyImpl()
	{
	}

	ManyImpl(SingleImpl* single)
	{
		bind(single->_handler, single->_id);
	}

	virtual bool isEmpty()
	{
		return _handlers.empty();
	}

	virtual bool isFull()
	{
		return false;
	}

	virtual void bind(EventHandler* handler, EventId id)
	{
		HandlerMap::iterator idBegin = _handlers.find(id);

		// Ignore the second binding of an already bound ID
		for (HandlerMap::iterator itr = idBegin, end = _handlers.end(); itr != end && itr->first == id; ++itr)
		{
			if (itr->second == handler) return;
		}

		// Attach new handler to the *frontmost* of exisiting eventID list.
		// If we do like this, it will not interfere the iteration currently in progress.
		// Moreover, we can avoid current OnInsert() of SaferItr implementation which is not very efficient.
		// A side-effect is that the new handler will get called prior to existing handlers.
		_handlers.insert(idBegin, std::make_pair(id, handler));
	}

	virtual void unbind(EventHandler* handler, EventId id)
	{
		if (id != 0)
		{
			for (HandlerMap::iterator curr, itr = _handlers.find(id); itr != _handlers.end() && itr->first == id; )
			{
				curr = itr++;
				EventHandler* h = curr->second;
				if (handler == h)
				{
					_safeItr.onErase(curr);
					_handlers.erase(curr);
					return; // No more search needed (one ID for one handler)
				}
			}
		}
		else
		{
			for (HandlerMap::iterator curr, itr = _handlers.begin(); itr != _handlers.end(); )
			{
				curr = itr++;
				EventHandler* h = curr->second;
				if (handler == h)
				{
					_safeItr.onErase(curr);
					_handlers.erase(curr);
				}
			}
		}
	}

	virtual void unbind(IEventSink* sink, EventId id)
	{
		for (HandlerMap::iterator curr, itr = id ? _handlers.find(id) : _handlers.begin(); itr != _handlers.end() && (id == 0 || itr->first == id); )
		{
			curr = itr++;
			EventHandler* h = curr->second;
			if (h->hasEventSink(sink))
			{
				_safeItr.onErase(curr);
				_handlers.erase(curr);
			}
		}
	}

	void sendLocal(const Event* evt)
	{
		if (_handlers.empty()) return;

		Ref<Impl> safe = this;

		bool invalidDetected = false;
		EventId id = evt->getId();

		_safeItr.begin(_handlers, _handlers.find(id));

		while (_safeItr.hasNext())
		{
			HandlerMap::value_type& pair = _safeItr.next();

			if (pair.first != id)
				break;

			Ref<EventHandler> h = pair.second;

			h->call(evt);

			if (h->isDisposed())
				invalidDetected = true;

			if (evt->isConsumed()) break;
		}

		_safeItr.end();

		if (!_safeItr.hasBegun() && invalidDetected)
			removeInvalids();
	}

	void removeInvalids()
	{
		for (HandlerMap::iterator curr, hitr = _handlers.begin(); hitr != _handlers.end(); )
		{
			curr = hitr++;
			EventHandler* h = curr->second;
			if (h->isDisposed())
			{
				_safeItr.onErase(curr);
				_handlers.erase(curr);
			}
		}
	}

	virtual Impl* expand();

	typedef multimap<EventId, Ref<EventHandler> >::type HandlerMap;
	HandlerMap							_handlers;

	SafeItr<HandlerMap>					_safeItr;
};

////////////////////////////////////////////////////////////////////////////////////////////

EventBinder::Impl* EventBinder::SingleImpl::expand()
{
	return new ManyImpl(this);
}

EventBinder::Impl* EventBinder::ManyImpl::expand()
{
	assert(false);

	return this;
}

////////////////////////////////////////////////////////////////////////////////////////////

EventBinder::EventBinder()
{
	_impl = NULL;
}

EventBinder::~EventBinder()
{
}

void EventBinder::send(EventId id, Event* evt)
{
	Ref<Event> safe = evt;

	if (!evt->setId(id))
		EventInfo::throwInvalidEvent(id);

	sendLocal(evt);
}

bool EventBinder::sendLocal(const Event* evt)
{
	Ref<EventBinder> safe = this;

	if (_impl && !evt->isConsumed())
		_impl->sendLocal(evt);

	if (_impl && _impl->isEmpty())
		_impl = NULL;

	return true;
}

EventHandler* EventBinder::doBind(EventId eventID, EventHandler* handler)
{
	Ref<EventHandler> autorel = handler;

	if (handler == NULL || !handler->canHandle(eventID))
		EventInfo::throwInvalidHandler(eventID);

	needMore()->bind(handler, eventID);

	return handler;
}

void EventBinder::doUnbind(EventId id, EventHandler* handler)
{
	Ref<EventHandler> autorel = handler;

	if (_impl == NULL) return;
	if (handler == NULL) return;

	_impl->unbind(handler, id);

	if (_impl->isEmpty())
		_impl = NULL;
}

void EventBinder::doUnbind(EventId id, IEventSink* sink)
{
	if (_impl == NULL) return;

	_impl->unbind(sink, id);

	if (_impl->isEmpty())
		_impl = NULL;
}

EventBinder::Impl* EventBinder::needMore()
{
	if (_impl == NULL)
		_impl = new SingleImpl();

	if (_impl->isFull())
		_impl = _impl->expand();

	return _impl;
}

////////////////////////////////////////////////////////////////////////////////////////////

class EventChannel::SingleImpl : public EventChannel::Impl
{
public:
	SingleImpl()
	{
		_binder = new EventBinder();
	}

	virtual EventBinder* find(float priority)
	{
		if (priority == 0.0f)
			return _binder;

		return NULL;
	}

	virtual bool sendLocal(const Event* evt)
	{
		_binder->sendLocal(evt);

		return true;
	}

	virtual void unbind(EventId id, EventHandler* handler)
	{
		_binder->unbind(id, handler);
	}

	virtual void unbind(EventId id, IEventSink* sink)
	{
		_binder->unbind(id, sink);
	}

	virtual Impl* expand();

	Ref<EventBinder> _binder;
};

////////////////////////////////////////////////////////////////////////////////////////////

class EventChannel::ManyImpl : public EventChannel::Impl
{
public:
	ManyImpl(SingleImpl* impl)
	{
		_binders.insert(std::make_pair(0.0f, impl->_binder));
	}

	virtual EventBinder* find(float priority)
	{
		Channels::iterator itr = _binders.find(priority);

		if (itr != _binders.end())
			return itr->second;

		EventBinder* newBinder = new EventBinder();
		_safeItr.onInsert(_binders.insert(std::make_pair(priority, newBinder)));

		return newBinder;
	}

	virtual bool sendLocal(const Event* evt)
	{
		_safeItr.begin(_binders);

		bool cont = false;

		while (_safeItr.hasNext())
		{
			EventBinder* binder = _safeItr.next().second;

			cont = binder->sendLocal(evt) && !evt->isConsumed();

			if (!cont) break;
		}

		_safeItr.end();

		return cont;
	}

	virtual void unbind(EventId id, EventHandler* handler)
	{
		for (Channels::iterator itr = _binders.begin(); itr != _binders.end(); ++itr)
		{
			itr->second.get()->unbind(id, handler);
		}
	}

	virtual void unbind(EventId id, IEventSink* sink)
	{
		for (Channels::iterator itr = _binders.begin(); itr != _binders.end(); ++itr)
		{
			itr->second.get()->unbind(id, sink);
		}
	}

	virtual Impl* expand()
	{
		assert(false);
		return this;
	}

	typedef map<float, Ref<EventBinder> >::type Channels;

	Channels _binders;
	SafeItr<Channels> _safeItr;
};

////////////////////////////////////////////////////////////////////////////////////////////

EventChannel::Impl* EventChannel::SingleImpl::expand()
{
	return new ManyImpl(this);
}

////////////////////////////////////////////////////////////////////////////////////////////

EventBinder* EventChannel::priority(float priority)
{
	if (_impl == NULL)
		_impl = new SingleImpl();

	EventBinder* channel = _impl->find(priority);
	if (channel == NULL)
	{
		_impl = _impl->expand();
		channel = _impl->find(priority);
		assert(channel);
	}

	return channel;
}

bool EventChannel::sendLocal(const Event* evt)
{
	Ref<EventChannel> safe = this;

	if (preFilter(evt) && _impl && !evt->isConsumed())
		_impl->sendLocal(evt);

	return postFilter(evt);
}

void EventChannel::doUnbind(EventId id, EventHandler* handler)
{
	if (_impl)
		_impl->unbind(id, handler);
}

void EventChannel::doUnbind(EventId id, IEventSink* sink)
{
	if (_impl)
		_impl->unbind(id, sink);
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
