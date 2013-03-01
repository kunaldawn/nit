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

#include "nit/async/AtomicInt.h"

// TODO: move this flag to config/Build.h
#define NIT_REFCOUNTED_DEBUGLIST
#undef NIT_REFCOUNTED_DEBUGLIST

#ifndef NIT_SHIPPING
#	if defined(NIT_REFCOUNTED_DEBUGLIST)
#		include <list>
#	endif
#endif

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class WeakRef;

class NIT_API WeakSupported
{
public:
	WeakSupported() : _weakRef(NULL) { }
	virtual ~WeakSupported();

	WeakRef*							_weak();

protected:
	friend class WeakRef;
	WeakRef*							_weakRef;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API RefCounted : public WeakSupported
{
public:
	RefCounted() : _refCount(0)													{ _enterlist(); }
	virtual ~RefCounted()														{ _leavelist(); }

public:
	inline RefCounted*					_ref()									{ return this; }

	inline int							getRefCount()							{ return _refCount; }
	inline void 						incRefCount()							{ ++_refCount; }
	inline void 						decRefCount()							{ if (--_refCount == 0) onZeroRef(); }

protected:
	virtual void						onZeroRef()								{ ++_refCount; onDelete(); --_refCount; customDelete(); }
	virtual void 						onDelete()								{ }
	virtual void 						customDelete()							{ assert(_refCount == 0); delete this; }

private:
	int									_refCount;
public:
	// Debugging feature
	typedef bool (*DebugListVisitor) (RefCounted* obj, void* up);

#if defined(NIT_REFCOUNTED_DEBUGLIST)
public:
	static void _debuglistDropSafe(RefCounted* obj);
	static bool _debuglistVisitAll(DebugListVisitor visitor, void* up);
	static void _debuglistPrintAll();
	class NIT_API _DebuglistSnapshot
	{
	public:
		_DebuglistSnapshot();
		_DebuglistSnapshot(const char* name);
		~_DebuglistSnapshot();
		void update(const char* name);
		void dump();
		void dumpDiffs(_DebuglistSnapshot& prev);
		void dumpSince();
		void swap(_DebuglistSnapshot& other);
	private:
		typedef std::map<RefCounted*, std::string> ObjectSet;
		std::string _name;
		ObjectSet _snapshot;
		static bool snapshotVisit(RefCounted* obj, void* up);
		static bool snapshotVisitSince(RefCounted* obj, void* up);
	};
public:
	RefCounted* _prev;
	RefCounted* _next;
	uint32 _enterID;
	static uint32 _nextEnterID;
	RefCounted(RefCounted* prev, RefCounted* next) : _refCount(0), _prev(prev), _next(next) { }
	void _enterlist();
	void _leavelist();
	friend class RefCountedList;
#else
public:
	inline static void _debuglistDropSafe(RefCounted* obj) { }
	inline static bool _debuglistVisitAll(DebugListVisitor visitor, void* up) { return false; }
	inline static void _debuglistPrintAll() { }
	inline void _enterlist() { }
	inline void _leavelist() { }
	class NIT_API _DebuglistSnapshot
	{
	public:
		_DebuglistSnapshot() { }
		_DebuglistSnapshot(const char* name) { }
		void update(const char* name) { }
		void dump() { }
		void dumpDiffs(_DebuglistSnapshot& prev) { }
		void dumpSince() { }
		void swap(_DebuglistSnapshot& other) { }
	};
#endif
};

////////////////////////////////////////////////////////////////////////////////

typedef WeakSupported MTWeakSupported;  // TODO: Implement a real MTWeakSupported

// TODO: To share MTRef among threads, there should be a backing-ref.
// Otherwise, another thread may try to use an object during one thread's OnDelete().
// Due to this, we should find a working OnZeroRef() implementation for MT-safe caching.
// As an alternative, we could consider implementing the collector that collects disposed objects 
// at around of OnZeroRef(), and at the time that quiescent state is obtained, the collector may perform blocking deletion.
// For this, we may need app-level epoch begin / end mechanism.

// TODO: Compare Inc/Dec performance with AtomicInt/AtomicDec and then
// if the difference is negligible, adopt Atomic as a default for the RefCounted class.

class NIT_API xMTRefCounted : public MTWeakSupported
{
public:
	xMTRefCounted() : _refCount(0) { }

public:									// custom MT-Safe RefCount (compatible to Ref<>)
	xMTRefCounted*						_ref()									{ return this; }
	inline int							getRefCount()							{ return _refCount._unsafeGet(); }
	inline void							incRefCount()							{ _refCount.inc(); }
	inline void							decRefCount()							{ if (_refCount.decGet() == 0) onZeroRef(); }

protected:
	virtual void						onZeroRef()								{ _refCount.inc(); onDelete(); _refCount.dec(); customDelete(); }
	virtual void 						onDelete()								{ }
	virtual void 						customDelete()							{ assert(_refCount._unsafeGet() == 0); delete this; }

private:
	AtomicInt							_refCount;
};

typedef RefCounted MTRefCounted;

////////////////////////////////////////////////////////////////////////////////

template <typename T>
class Ref
{
public:
	inline Ref(T* obj = NULL) : _obj(obj)										{ inc(obj); }
	inline Ref(const Ref& other) : _obj(other._obj)								{ inc(_obj); }
	inline ~Ref()																{ dec(_obj); }

	// auto casting
	inline operator T* () const													{ return _obj; }
	inline T& operator* () const												{ return *_obj; }
	inline T* operator-> () const												{ return _obj; }

	// assignment
	inline Ref& operator= (const Ref& other)									{ set(other._obj); return *this; }
	inline Ref& operator= (T* obj)												{ set(obj); return *this; }

	// comparisons
	inline bool operator== (T* obj) const										{ return _obj == obj; }
	inline bool operator!= (T* obj) const										{ return _obj != obj; }
	inline bool operator== (const Ref& other) const								{ return _obj == other._obj; }
	inline bool operator!= (const Ref& other) const								{ return _obj != other._obj; }

	// utility
	inline void swap(Ref& other)												{ T* t = _obj; _obj = other._obj; other._obj = t; }
	inline T* get() const														{ return _obj; }
	inline void set(T* obj)														{ T* old = _obj; _obj = obj; inc(obj); dec(old); }

	// ogre-compatible only do not use!
	// TODO: remove
	inline T* getPointer() const												{ return _obj; }
	inline bool isNull() const													{ return _obj == NULL; }
	inline void setNull()														{ set(NULL); }
	inline void bind(T* obj)													{ assert(_obj == NULL); set(obj); }
	inline unsigned int useCount() const										{ return _obj ? _obj->getRefCount() : 0; }

protected:
	inline static void inc(T* obj)												{ if (obj) obj->_ref()->incRefCount(); }
	inline static void dec(T* obj)												{ if (obj) obj->_ref()->decRefCount(); }

	T* _obj;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API WeakRef : public PooledAlloc
{
public:
	WeakRef(WeakSupported* obj) : _object(obj), _refCount(0), _listeners(NULL) { }
	virtual ~WeakRef()															{ }

	inline int							getRefCount()							{ return _refCount; }
	inline void 						incRefCount()							{ ++_refCount; }
	inline void 						decRefCount()							{ if (--_refCount == 0) _delete(); }

	inline bool							alive()									{ return _object != NULL; }
	inline WeakSupported*				getObject()								{ return _object; }

public:
	class IListener;

	void								addListener(IListener* listener);
	void								removeListener(IListener* listener);

public:
	static WeakRef*						_NULL_;

protected:
	virtual void						_delete();								// internal use only.

private:
	WeakRef() { }
	void								_notifyDestroy();

	typedef list<IListener*>::type ListenerList;

	WeakSupported*						_object;
	int									_refCount;
	ListenerList*						_listeners;

	friend class WeakSupported;
};

class NIT_API WeakRef::IListener 
{
public:
	virtual ~IListener() { }

	virtual void						onDestroy(WeakSupported* object) = 0;
};

////////////////////////////////////////////////////////////////////////////////

// Weak<> template for single concrete inheritance (descendants of WeakSupport)

template <typename T>
class Weak
{
public:
	inline Weak() : _weak(WeakRef::_NULL_)										{ }
	inline Weak(T* obj) : _weak(WeakRef::_NULL_)								{ set(obj); }
	inline Weak(const Weak& other) : _weak(other._weak) 						{ inc(_weak); }
	inline ~Weak()																{ dec(_weak); }

	// auto casting
	inline operator T* () const													{ return get(); }
	inline T& operator* () const												{ return *get(); }
	inline T* operator-> () const												{ return get(); }

	// assignment
	inline Weak& operator= (const Weak& other)									{ set(other.get()); return *this; }
	inline Weak& operator= (T* obj)												{ set(obj); return *this; }

	// comparisons
	inline bool operator== (T* obj) const										{ return get() == obj; }
	inline bool operator!= (T* obj) const										{ return get() != obj; }
	inline bool operator== (const Weak& other) const							{ return _weak == other._weak; }
	inline bool operator!= (const Weak& other) const							{ return _weak != other._weak; }
	inline bool operator< (const Weak& other) const								{ return _weak < other._weak; }

	// utility
	inline bool alive()															{ return _weak->alive(); }
	inline void swap(Weak& other)												{ WeakRef* t = _weak; _weak = other._weak; _weak = t; }
	inline T* get() const														{ return static_cast<T*>(_weak->getObject()); }

	inline WeakRef* getWeak(T* obj)												{ return obj ? obj->_weak() : WeakRef::_NULL_; }
	inline void set(T* obj)														{ WeakRef* w = getWeak(obj); WeakRef* old = _weak; _weak = w; inc(w); dec(old); }

protected:
	inline static void inc(WeakRef* w)											{ if (w) w->incRefCount(); }
	inline static void dec(WeakRef* w)											{ if (w) w->decRefCount(); }

	WeakRef* _weak;
};

////////////////////////////////////////////////////////////////////////////////

// Weak<> templates for interface inherited classes
// ex) with 'class B : A, I', use this when you can't compile Weak<I> due to static_cast related problem.

template <typename T>
class IWeak
{
public:
	inline IWeak() : _weak(WeakRef::_NULL_)										{ }
	inline IWeak(T* obj) : _weak(WeakRef::_NULL_)								{ set(obj); }
	inline IWeak(const IWeak& other) : _weak(other._weak) 						{ inc(_weak); }
	inline ~IWeak()																{ dec(_weak); }

	// auto casting
	inline operator T* () const													{ return get(); }
	inline T& operator* () const												{ return *get(); }
	inline T* operator-> () const												{ return get(); }

	// assignment
	inline IWeak& operator= (const IWeak& other)								{ set(other.get()); return *this; }
	inline IWeak& operator= (T* obj)											{ set(obj); return *this; }

	// comparisons
	inline bool operator== (T* obj) const										{ return get() == obj; }
	inline bool operator!= (T* obj) const										{ return get() != obj; }
	inline bool operator== (const IWeak& other) const							{ return _weak == other._weak; }
	inline bool operator!= (const IWeak& other) const							{ return _weak != other._weak; }
	inline bool operator< (const IWeak& other) const							{ return _weak < other._weak; }

	// utility
	inline bool alive()															{ return _weak->alive(); }
	inline void swap(IWeak& other)												{ WeakRef* t = _weak; _weak = other._weak; _weak = t; }
	inline T* get() const														{ return dynamic_cast<T*>(_weak->getObject()); }

	inline WeakRef* getWeak(T* obj)												{ return obj ? obj->_weak() : WeakRef::_NULL_; }
	inline void set(T* obj)														{ WeakRef* w = getWeak(obj); inc(w); dec(_weak); _weak = w; }

protected:
	inline static void inc(WeakRef* w)											{ if (w) w->incRefCount(); }
	inline static void dec(WeakRef* w)											{ if (w) w->decRefCount(); }

	WeakRef* _weak;
};

////////////////////////////////////////////////////////////////////////////////

// TODO: merge Ogre into nitspace and remove these

template <typename TClass, typename TRootClass>
class StaticCastRef : public Ref<TClass>
{
public:
	typedef StaticCastRef<TClass, TRootClass> ThisClass;
	StaticCastRef(TRootClass* obj = NULL) : Ref<TClass>(static_cast<TClass*>(obj))					{ }
	StaticCastRef(const Ref<TRootClass>& other) : Ref<TClass>(static_cast<TClass*>(other.get()))	{ }
	StaticCastRef<TClass, TRootClass>& operator= (const Ref<TRootClass>& other)						{ ThisClass::set(static_cast<TClass*>(other.get())); return *this; }
	StaticCastRef<TClass, TRootClass>& operator= (TRootClass* obj)									{ ThisClass::set(static_cast<TClass*>(obj)); return *this; }
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
