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

#include "nit/io/Stream.h"
#include "nit/event/Event.h"
#include "nit/event/Timer.h"

#include "nit/script/NitBind.h"
#include "nit/script/ScriptEvent.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class ScriptRuntime;
class ScriptPeer;

////////////////////////////////////////////////////////////////////////////////

// TODO: Refactor to ScriptEvent.h (tried but had problem with compilation of Ref<>)

class NIT_API ScriptEvent : public Event
{
public:
	ScriptEvent()						{ }
	ScriptEvent(HSQUIRRELVM v, int stackIdx);

public:
	ScriptPeer*							getPeer()								{ return _peer; }

public:
	Ref<ScriptPeer>						_peer;
};

////////////////////////////////////////////////////////////////////////////////

class ScriptRefManager;

class NIT_API IScriptRef
{
public:
	IScriptRef() : _scriptRefCount(0), _lastNativeRefCount(0)						{ }
	virtual ~IScriptRef()														{ }

public:
	virtual RefCounted*					_ref() = 0;
	virtual WeakRef*					_weak() = 0;

	virtual ScriptPeer*					getScriptPeer() = 0;
	virtual String						getDebugString() = 0;

public:
	int									getNativeRefCount()						{ return _ref()->getRefCount() - _scriptRefCount; }
	int									getScriptRefCount()						{ return _scriptRefCount; }

	static void							scriptIncRef(IScriptRef* o)				{ ++o->_scriptRefCount; o->_ref()->incRefCount(); }
	static void							scriptDecRef(IScriptRef* o)				{ --o->_scriptRefCount; o->_ref()->decRefCount(); }

private:
	int									_scriptRefCount;
	int									_lastNativeRefCount;
	friend class						ScriptRefManager;
};

////////////////////////////////////////////////////////////////////////////////

enum ScriptResult	{ SCRIPT_CALL_OK, SCRIPT_CALL_NO_METHOD, SCRIPT_CALL_ERR, SCRIPT_CALL_INVALID_ARGS };
enum ScriptPeerType { SCRIPT_PEER_NONE, SCRIPT_PEER_WEAK, SCRIPT_PEER_STRONG };

class NIT_API ScriptPeer : public RefCounted, public PooledAlloc
{
public:
	ScriptPeer(HSQUIRRELVM v, int stackidx);
	virtual ~ScriptPeer();

public:
	ScriptRuntime*						getRuntime()							{ return _runtime; }
	HSQUIRRELVM							getWorker();

	void								replace(HSQUIRRELVM v, int stackidx);

public:
	template <typename TClass>
	void								pushParam(const TClass& value);

	ScriptResult						callMethod(const char* method, int nargs, bool needRet = false);
	ScriptResult						callMethod(const HSQOBJECT& nameObj, int nargs, bool needRet = false);

	ScriptResult						callWith(const HSQOBJECT& closureObj, int nargs, bool needRet = false);

public:
	SQObjectRef							getObjectRef(HSQUIRRELVM v);

	int									pushObject(HSQUIRRELVM v);
	int									pushWeakRef(HSQUIRRELVM v);
	bool								isStrong()								{ return _peerType == SCRIPT_PEER_STRONG; }

public:
	void								makeStrong(HSQUIRRELVM v);
	void								makeWeak(HSQUIRRELVM v);

	String								getTypeName();

protected:
	ScriptPeer() { }

private:
	Weak<ScriptRuntime>					_runtime;
	ScriptPeerType						_peerType;
	HSQOBJECT							_peer;

	void								releasePeer();
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API ScriptClosure : public RefCounted, public PooledAlloc
{
public:
	ScriptClosure();
	ScriptClosure(HSQUIRRELVM v, int stackidx);
	virtual ~ScriptClosure();

public:
	ScriptRuntime*						getRuntime()							{ return _runtime; }

public:
	ScriptResult						call(int nargs, bool needRet = false);

public:
	void								replace(HSQUIRRELVM v, int stackidx);
	int									pushClosure(HSQUIRRELVM v);
	int									pushWeakRef(HSQUIRRELVM v);

private:
	Weak<ScriptRuntime>					_runtime;
	HSQOBJECT							_peer;

	void								releasePeer();
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API ScriptWaitBlock : public RefCounted, public PooledAlloc
{
public:
	ScriptWaitBlock(ScriptRuntime* runtime);
	virtual ~ScriptWaitBlock();

public:
	ScriptRuntime*						getRuntime()							{ return _runtime; }

public:
	uint								getNumSuspended()						{ return _suspended.size(); }
	SQInteger							pushAll(HSQUIRRELVM v);

	uint								getAllowCount()							{ return _allowCount; }
	void								setAllowCount(int count)				{ _allowCount = count; }

public:
	SQInteger							wait(HSQUIRRELVM v, uint signalMask);
	int									signal(uint signalFlags);
	void								killAll(const SQChar* msg = NULL);

private:
	struct Entry
	{
		HSQOBJECT						weakThread;
		uint							signalMask;
	};

	typedef deque<Entry>::type ThreadEntries;

	Weak<ScriptRuntime>					_runtime;
	ThreadEntries						_suspended;
	uint								_allowCount;
	bool								_killing;

	friend class						ScriptRuntime;
	static SQInteger					clearWaitBlockFlag(HSQUIRRELVM th);
};

////////////////////////////////////////////////////////////////////////////////

template <typename TState>
class NIT_API TScriptState : public TState, public IScriptEventSink, public IScriptRef
{
public:
	TScriptState(ScriptPeer* peer) : _peer(peer)								{ }
	virtual ~TScriptState()														{ }

public:
	virtual IEventSink*					getEventSink()							{ return this; }

	virtual void 						onEnter()								{ if (_peer) _peer->callMethod("onEnter", 0); }
	virtual void 						onExit()								{ if (_peer) _peer->callMethod("onExit", 0); }
	virtual void 						onEvent(const Event* e)					{ callOnEvent(e); }

public:									// IScriptRef impl
	virtual RefCounted*					_ref()									{ return RefCounted::_ref(); }
	virtual WeakRef*					_weak()									{ return RefCounted::_weak(); }
	virtual ScriptPeer*					getScriptPeer()							{ return _peer; }
	virtual String						getDebugString()						{ return _peer ? _peer->getTypeName() : "(ScriptState: null)"; }

public:									// IScriptEventSink impl
	virtual int							push(HSQUIRRELVM v)						{ return NitBind::push(v, static_cast<TState*>(this)); }

protected:
	virtual String						getClassName()							{ return _peer->getTypeName(); }

protected:
	Ref<ScriptPeer>						_peer;

	inline void							callOnEvent(const Event* e);
};

template <typename TState>
inline void TScriptState<TState>::callOnEvent(const Event* e)
{
	HSQUIRRELVM v = _peer ? _peer->getWorker() : NULL;
	if (v == NULL) return;

	NitBind::push(v, const_cast<Event*>(e));
	_peer->callMethod("onEvent", 1);
}

////////////////////////////////////////////////////////////////////////////////

class NIT_API ScriptUnit : public RefCounted
{
public:
	ScriptUnit(ScriptRuntime* runtime, const String& id, StreamSource* source);

public:
	const String&						getId()									{ return _id; }
	StreamSource*						getSource()								{ return _source; }
	StreamLocator*						getLocator()							{ return this && _source ? _source->getLocator() : NULL; }

public:
	bool								isCompiled()							{ return _compiled; }
	bool								isRequired()							{ return _required; }

	void								setRequired(bool flag)					{ _required = flag; }

public:
	SQRESULT							compile(HSQUIRRELVM v);
	SQRESULT							execute(HSQUIRRELVM v);
	SQRESULT							unload(HSQUIRRELVM v);

	SQInteger							pushBody(HSQUIRRELVM v);

private:
	Weak<ScriptRuntime>					_runtime;
	Ref<StreamSource>					_source;
	String								_id;

	HSQOBJECT							_body;

	bool								_compiled : 1;
	bool								_required : 1;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API ScriptIO
{
public:
	static SQInteger					lexfeed_ASCII(SQUserPointer p);
	static SQInteger					lexfeed_UTF8(SQUserPointer p);
	static SQInteger					lexfeed_UCS2_LE(SQUserPointer p);
	static SQInteger					lexfeed_UCS2_BE(SQUserPointer p);
	static SQInteger					bytecode_read(SQUserPointer p, SQUserPointer buf, SQInteger size);
	static SQInteger					bytecode_write(SQUserPointer p, SQUserPointer buf, SQInteger size);
	static SQRESULT						loadstream(HSQUIRRELVM v, StreamReader* srcReader, const String& source_id, SQBool printerror);
};

////////////////////////////////////////////////////////////////////////////////

class ScriptDebugger;

class NIT_API ScriptRuntime : public RefCounted
{
public:
	ScriptRuntime();
	virtual								~ScriptRuntime();

public:
	void								command(const String& line);

	EventHandler*						gcLoopHandler()							{ return _gcLoopHandler; }
	EventHandler*						clockHandler()							{ return _clockHandler; }
	EventHandler*						tickHandler()							{ return _tickHandler; }

public:
	static ScriptRuntime*				getRuntime(HSQUIRRELVM v)				{ return static_cast<ScriptRuntime*>(NitBindImpl::getUserEnv(v)); }

public:
	void								startup();
	void								shutdown();

	bool								isStarted()								{ return _started; }
	bool								isPaused()								{ return _paused; }
	bool								isDebugPaused()							{ return _debugPaused; }

	size_t								getMemoryAllocated_DebugPurpose();

public:
	HSQUIRRELVM							getWorker()								{ HSQUIRRELVM v = sq_getcurrentthread(_root); return v ? v : _root; }
	ScriptDebugger*						getDebugger()							{ return _debugger; }

public:
	HSQUIRRELVM							getRoot()								{ return _root; }

	void								killAllThreads(bool cleanup);

	SQInteger							clearThreadTimeout(HSQUIRRELVM th);

public:
	void								registerFn(const char* fnname, SQFUNCTION fn);
	bool								doString(const char* string, bool spawncotask = false);
	SQRESULT							loadStream(HSQUIRRELVM v, StreamReader* srcStream, SQBool printerror);

public:
	StreamLocator*						getDefaultLocator()						{ return _defaultLocator; }
	void								setDefaultLocator(StreamLocator* locator);

	StreamLocator*						getLocatorOverride()					{ return _locatorOverride; }
	void								setLocatorOverride(StreamLocator* locator);

	SQRESULT							require(const String& unitName, Ref<ScriptUnit>& outUnit, StreamLocator* locator = NULL);
	bool								doFile(const String& unitName, StreamLocator* locator = NULL);

public:									// returns current loading unit & its locator
	ScriptUnit*							getUnit()								{ return _unitStack.empty() ? NULL : _unitStack.back(); }
	StreamLocator*						getLocator()							{ return getUnit()->getLocator(); }
	

public:
	typedef map<String, Ref<ScriptUnit> >::type UnitMap;
	ScriptUnit*							getLoaded(const String& id);

	String								unitSourceID(StreamSource* source);
	StreamSource*						locateUnit(const String& unitName, StreamLocator* locator);
	ScriptUnit*							createUnit(const String& unitName, StreamLocator* locator);
	const UnitMap&						allLoaded()								{ return _units; }
	void								unloadUnitsFrom(const String& locatorPattern);

public:
	void								weakAdd(WeakSupported* object);
	void								weakRelease(WeakSupported* object);

public:
	void								retain(IScriptRef* obj);

public:
	int									getOpLimit()							{ return _oplimit; }
	void								setOpLimit(int oplimit)					{ _oplimit = oplimit; }
	void								stepGC();

public:
	static SQInteger					requireHandler(HSQUIRRELVM v);
	static SQInteger					errorHandler(HSQUIRRELVM v);
	static void							compileErrorHandler(HSQUIRRELVM v,const SQChar *sErr,const SQChar *sSource,SQInteger line,SQInteger column);
	static void							setDefaultErrorHandlers(HSQUIRRELVM v);

	static void							printCallStack(HSQUIRRELVM v, SQInteger level, bool dumplocals);

private:
	HSQUIRRELVM							_root;
	bool								_started;

	ScriptDebugger*						_debugger;

public:
	UnitMap								_units;
	vector<Ref<ScriptUnit> >::type		_unitStack;
	Ref<StreamLocator>					_locatorOverride;
	Ref<StreamLocator>					_defaultLocator;

public:
	void								updateEventBindings();
	void								registerScriptEventInfo(EventInfo* info);

private:
	friend class ScriptPeer;			
	HSQOBJECT							_eventsTable;
	void								initEventBindings();
	void								finishEventBindings();

	vector<EventInfo*>::type			_scriptEventInfos;

private:
	Ref<EventHandler>					_gcLoopHandler;
	Ref<EventHandler>					_clockHandler;
	Ref<EventHandler>					_tickHandler;

	void								onGcLoop(const Event* evt);
	void								onClock(const TimeEvent* evt);
	void								onTick(const TimeEvent* evt);

private:
	bool								_paused;
	bool								_debugPaused;
	bool								_stepGcPaused;

	int									_oplimit;

	typedef list<HSQOBJECT>::type SQObjList;
	SQObjList							_threadList;

	Ref<ScriptWaitBlock>				_timeWait;

	WeakRef::IListener*					_weakListener;

	ScriptRefManager*					_scriptRefManager;

	void								updateThreadList();

	static SQInteger					newThread(HSQUIRRELVM v);
	static SQInteger					costart(HSQUIRRELVM v);
	static SQInteger					cotimeout(HSQUIRRELVM v);
	static SQInteger					sleep(HSQUIRRELVM v);

	static SQInteger					getThreads(HSQUIRRELVM v);
	static SQInteger					thread_Call(HSQUIRRELVM v);
	static SQInteger					thread_Wakeup(HSQUIRRELVM v);
	static SQInteger					thread_Kill(HSQUIRRELVM v);
	static SQInteger					thread_Dump(HSQUIRRELVM v);
	static SQInteger					thread_cotimeout(HSQUIRRELVM v);
	static SQInteger					thread_tostring(HSQUIRRELVM v);

	static SQInteger					pauseGc(HSQUIRRELVM v);
	static SQInteger					dumpGc(HSQUIRRELVM v);

	struct TimeoutEntry
	{
		float							wakeTime;
		float							startTime;
		uint							timeoutId;
		HSQOBJECT						weakThread;

		bool operator <					(const TimeoutEntry& other)				{ return wakeTime > other.wakeTime; } // to use with std::make_heap
	};

	typedef vector<TimeoutEntry>::type TimeoutHeap;

	float								_tickTime;
	float								_clockTime;

	TimeoutHeap							_tickTimeoutHeap;
	TimeoutHeap							_clockTimeoutHeap;
	uint								_nextTimeoutID;

	SQInteger							setTimeout(TimeoutHeap& heap, HSQUIRRELVM th, float timeout, float currtime);
	void								updateTimeout(TimeoutHeap& heap, float time);
	void								removeAllTimeout(TimeoutHeap& heap);

public:
	// HACK : Temporary implementation for wxTrackable.
	// TODO: If you need another extension, implement more nit one.
	class NIT_API Extension : public RefCounted
	{
	};

	Extension*							getWxWeakTracker()						{ return _wxWeakTracker; }
	void								setWxWeakTracker(Extension* trk)		{ _wxWeakTracker = trk; }

	Ref<Extension>						_wxWeakTracker;

private:
	friend class						ScriptIO;
	friend class						ScriptRuntimeLib;
	friend class						ScriptDebugger;
	friend class						NB_ScriptRuntime;
	friend class						NB_ScriptDebugger;
};

////////////////////////////////////////////////////////////////////////////////

inline HSQUIRRELVM ScriptPeer::getWorker()
{
	return _runtime ? _runtime->getWorker() : NULL;
}

template <typename TClass>
void ScriptPeer::pushParam(const TClass& value)
{
	NitBind::push(_runtime->getWorker(), value);
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
