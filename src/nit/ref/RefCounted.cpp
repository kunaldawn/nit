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

#include "RefCounted.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class _NullWeakRef : public WeakRef
{
protected:
	virtual void _delete() { }
	static _NullWeakRef Singleton;
	_NullWeakRef() : WeakRef(NULL) { }
	friend class WeakRef;
};

_NullWeakRef _NullWeakRef::Singleton;

WeakRef* WeakRef::_NULL_ = &_NullWeakRef::Singleton;

////////////////////////////////////////////////////////////////////////////////

WeakSupported::~WeakSupported()
{
	if (_weakRef)
		_weakRef->_notifyDestroy();
}

WeakRef* WeakSupported::_weak()
{
	if (_weakRef == NULL)
	{
		_weakRef = new WeakRef(this);
	}

	return _weakRef;
}

////////////////////////////////////////////////////////////////////////////////

#if defined(NIT_REFCOUNTED_DEBUGLIST)

class RefCountedList
{
public:
	RefCounted mHead;
	RefCounted mTail;

	RefCountedList()
		: mHead(NULL, NULL)
		, mTail(NULL, NULL)
	{
		mHead._next = &mTail;
		mTail._prev = &mHead;
	}

	bool IsEmpty()
	{
		return mHead._next == &mTail;
	}

	void Push(RefCounted* obj)
	{
		// detach
		if (obj->_prev) obj->_prev->_next = obj->_next;
		if (obj->_next) obj->_next->_prev = obj->_prev;

		// attach
		obj->_prev = mTail._prev;
		obj->_next = &mTail;

		mTail._prev->_next = obj;
		mTail._prev = obj;
	}

	void Swap(RefCountedList& other)
	{
		RefCounted* temp;

		temp = mHead._next;
		mHead._next = other.mHead._next;
		other.mHead._next = temp;
		mHead._next->_prev = &mHead;
		other.mHead._next->_prev = &other.mHead;

		temp = mTail._prev;
		mTail._prev = other.mTail._prev;
		other.mTail._prev = temp;
		mTail._prev->_next = &mTail;
		other.mTail._prev->_next = &other.mTail;
	}

	static RefCountedList& GetActiveList()
	{
		static RefCountedList current;
		return current;
	}
};

uint32 RefCounted::_nextEnterID = 0;

void RefCounted::_enterlist()
{
	_prev = NULL;
	_next = NULL;
	_enterID = _nextEnterID++;

	RefCountedList::GetActiveList().Push(this);
}

void RefCounted::_leavelist()
{
	if (_refCount != 0)
	{
		LOG(0, "*** %s (%08x) destroyed with refcount %d\n", typeid(*this).name(), this, _refCount);
	}

	if (_prev) _prev->_next = _next;
	if (_next) _next->_prev = _prev;
	_prev = NULL;
	_next = NULL;
}

void RefCounted::_debuglistDropSafe(RefCounted* obj)
{
	obj->_leavelist();
}

bool RefCounted::_debuglistVisitAll(DebugListVisitor visitor, void* up)
{
	RefCountedList& list = RefCountedList::GetActiveList();

	bool cont = true;
	RefCounted* obj = list.mHead._next;

	while (obj != &list.mTail && cont)
	{
		RefCounted* next = obj->_next;
		cont = visitor(obj, up);
		obj = next;
	}

	return cont;
}

static bool debugVisit(RefCounted* obj, void* up)
{
	std::string name = "???";

	try
	{
		name = typeid(*obj).name();
	}
	catch (...)
	{
	}

	LOG(0, "***   refcounted leak: 0x%08x #%d %s (%d)\n", obj, obj->_enterID, name.c_str(), obj->getRefCount());
	return true;
}

void RefCounted::_debuglistPrintAll()
{
	LOG(0, ".. RefCounted::_debuglistPrintAll\n");
	_debuglistVisitAll(debugVisit, NULL);
}

////////////////////////////////////////////////////////////////////////////////

RefCounted::_DebuglistSnapshot::_DebuglistSnapshot()
{
}

RefCounted::_DebuglistSnapshot::_DebuglistSnapshot(const char* name)
{
	update(name);
}

RefCounted::_DebuglistSnapshot::~_DebuglistSnapshot()
{
}

void RefCounted::_DebuglistSnapshot::update(const char* name)
{
	_name = name;
	_debuglistVisitAll(snapshotVisit, this);
}

bool RefCounted::_DebuglistSnapshot::snapshotVisit(RefCounted* obj, void* up)
{
	_DebuglistSnapshot* s = (_DebuglistSnapshot*)up;

	std::string name = "???";
	try { name = typeid(*obj).name(); } catch (...) { }
	s->_snapshot.insert(std::make_pair(obj, name));
	return true;
}

void RefCounted::_DebuglistSnapshot::dump()
{
	LOG(0, ".. RefCounted snapshot '%s':\n", _name.c_str());

	for (ObjectSet::iterator itr = _snapshot.begin(), end = _snapshot.end(); itr != end; ++itr)
	{
		RefCounted* obj = itr->first;
		std::string name;
		try { name = typeid(*obj).name(); } catch (...) { name = itr->second + "?"; }

		LOG(0, "..   0x%08x #%d %s (%d)\n", obj, obj->_enterID, name.c_str(), obj->getRefCount());
	}
}

void RefCounted::_DebuglistSnapshot::dumpDiffs(_DebuglistSnapshot& prev)
{
	LOG(0, ".. RefCounted snapshot '%s' diff with '%s'\n", _name.c_str(), prev._name.c_str());
	for (ObjectSet::iterator itr = _snapshot.begin(), end = _snapshot.end(); itr != end; ++itr)
	{
		RefCounted* obj = itr->first;
		if (prev._snapshot.find(obj) != prev._snapshot.end())
			continue;
		std::string name;
		try { name = typeid(*obj).name(); } catch (...) { name = itr->second + "?"; }

		LOG(0, "..   0x%08x #%d %s (%d)\n", obj, obj->_enterID, name.c_str(), obj->getRefCount());
	}
}

bool RefCounted::_DebuglistSnapshot::snapshotVisitSince(RefCounted* obj, void* up)
{
	_DebuglistSnapshot* s = (_DebuglistSnapshot*)up;

	std::string name;
	try 
	{ 
		name = typeid(*obj).name(); 
		if (s->_snapshot.find(obj) != s->_snapshot.end())
			return true;
	} 
	catch (...) 
	{ 
		ObjectSet::iterator itr = s->_snapshot.find(obj);
		if (itr != s->_snapshot.end())
			name = itr->second + "?";
		else
			name = "???";
	}

	LOG(0, "..  0x%08x #%d %s (%d)\n", obj, obj->_enterID, name.c_str(), obj->getRefCount());
	return true;
}

void RefCounted::_DebuglistSnapshot::dumpSince()
{
	LOG(0, ".. RefCounted snapshot since '%s':\n", _name.c_str());
	_debuglistVisitAll(snapshotVisitSince, this);
}

void RefCounted::_DebuglistSnapshot::swap(_DebuglistSnapshot& other)
{
	_name.swap(other._name);
	_snapshot.swap(other._snapshot);
}

#endif

////////////////////////////////////////////////////////////////////////////////

void WeakRef::_delete()
{
	if (_refCount != 0) return;

	if (_object)
	{
		_object->_weakRef = NULL;
	}

	if (_listeners)
		delete _listeners;

	delete this;
}

void WeakRef::_notifyDestroy()
{
	incRefCount();

	ListenerList* listeners = _listeners;

	_listeners = NULL; // prevent removeListener mutate this

	if (listeners)
	{
		for (ListenerList::iterator itr = listeners->begin(), end = listeners->end(); itr != end; ++itr)
		{
			(*itr)->onDestroy(_object);
		}

		delete listeners;
	}

	_object = NULL;

	if (_listeners)
	{
		decRefCount();
		NIT_THROW_FMT(EX_INVALID_STATE, "Can't add listener while destroying one");
		return;
	}

	decRefCount();
}

void WeakRef::addListener(IListener* listener)
{
	if (_listeners == NULL)
		_listeners = new ListenerList();

	_listeners->push_back(listener);
}

void WeakRef::removeListener(IListener* listener)
{
	if (_listeners == NULL)
		return;

	_listeners->remove(listener);
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
