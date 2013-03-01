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
#include "nit/script/ScriptDebugger.h"
#include "nit/script/NitBind.h"

#include "nit/event/Event.h"
#include "nit/runtime/MemManager.h"
#include "nit/io/MemoryBuffer.h"

#include "squirrel/sqstate.h"
#include "squirrel/sqtable.h"
#include "squirrel/sqvm.h"
#include "squirrel/sqclass.h"

#include "squirrel/sqstdblob.h"
#include "squirrel/sqstdstream.h"
#include "squirrel/sqstdstring.h"
#include "squirrel/sqstdmath.h"
#include "squirrel/sqstdio.h"
#include "squirrel/sqstdsystem.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class ScriptRefManager
{
public:
	typedef list<IWeak<IScriptRef> >::type RefList;
	RefList								_retained;
	int									_stepUpdateLimit;

	int									_deltaRetained;
	int									_deltaStrong;
	int									_deltaWeak;

	ScriptRefManager()
	{
		_stepUpdateLimit = 50;	// TODO: control outside!
		_deltaRetained = 0;
	}

	~ScriptRefManager()
	{
		CheckClear();
	}

	void Retain(HSQUIRRELVM v, IScriptRef* o)
	{
		o->_lastNativeRefCount = 1;
		ScriptPeer* peer = o->getScriptPeer();
		if (peer) peer->makeStrong(v);

		_retained.push_front(o);
		++_deltaRetained;
	}

	void UpdateList(HSQUIRRELVM v)
	{
		RefList updated;

		int released = 0;
		_deltaStrong = 0;
		_deltaWeak = 0;

		// Update main list in given number by circular queue manner
		// every new introduced objects should be checked now
		int stepLimit = _deltaRetained + _stepUpdateLimit;
		for (int i=0; i<stepLimit && !_retained.empty(); ++i)
		{
			IWeak<IScriptRef>& w = *_retained.begin();

			bool erased = UpdateObj(v, w);

			if (!erased)
			{
				// append this object to tail of updated list 
				updated.splice(updated.end(), _retained, _retained.begin());
			}
			else
			{
				// just erase this
				_retained.pop_front();
				++released;
			}
		}

		// append updated objects into tail of main list and empty updated objects so that checked later
		_retained.splice(_retained.end(), updated);

		if (_deltaRetained > 0 || _deltaStrong > 0 || _deltaWeak > 0 || released)
		{
//			LOG(0, ".. ScriptRef: %d retained, %d released (+%d, -%d) -> %d alive\n", _deltaRetained, released, _deltaStrong, _deltaWeak, _retained.size());
		}

		_deltaRetained = 0;
	}

	bool UpdateObj(HSQUIRRELVM v, IWeak<IScriptRef>& w)
	{
		IScriptRef* o = w.get();

		if (o == NULL) return true;

		int lastNativeRefCount = o->_lastNativeRefCount;
		int nativeRefCount = o->getNativeRefCount();

		ScriptPeer* peer = o->getScriptPeer();

		if (lastNativeRefCount == 0 && nativeRefCount > 0)
		{
			if (peer) peer->makeStrong(v);
			++_deltaStrong;
		}
		else if (lastNativeRefCount > 0 && nativeRefCount == 0)
		{
			if (peer) peer->makeWeak(v);
			++_deltaWeak;
		}

		// could have been deleted during above 'MakeWeak' statement
		if (w == NULL) return true;

		o->_lastNativeRefCount = nativeRefCount;
		return false;
	}

	void ReleaseAll(HSQUIRRELVM v)
	{
		if (_retained.empty()) return;

		LOG_TIMESCOPE(0, "-- ScriptRefManager: ReleaseAll");

		_deltaWeak = 0;
		int released = 0;

		for (RefList::iterator itr = _retained.begin(); itr != _retained.end(); ++itr)
		{
			IWeak<IScriptRef>& w = *itr;

			if (w == NULL)
			{
				++released;
				continue;
			}

			IScriptRef* o = w.get();

			if (o->_lastNativeRefCount > 0)
			{
				ScriptPeer* peer = o->getScriptPeer();
				if (peer) peer->makeWeak(v);
				++_deltaWeak;
			}

			if (!w.alive())
				++released;
		}

		if (_deltaWeak > 0 || released)
		{
			LOG(0, ".. ScriptRef: total %d -> %d released (-%d)\n", _retained.size(), released, _deltaWeak);
		}
	}

	void CheckClear()
	{
		if (_retained.empty()) return;

#if !defined(NIT_SHIPPING)
		int native = 0;
		int error = 0;
		int released = 0;

		for (RefList::iterator itr = _retained.begin(); itr != _retained.end(); ++itr)
		{
			if (itr->alive())
			{
				IScriptRef* o = *itr;
				if (o->getScriptRefCount())
				{
					const char* tname = typeid(*o).name();
					const String& name = o->getDebugString();

					LOG(0, "*** Invalid Count: %s '%s' (%08x) VMRef: %d, NativeRef: %d\n", tname, name.c_str(), o, o->getScriptRefCount(), o->getNativeRefCount());
					++error;
				}
				else
				{
					const char* tname = typeid(*o).name();
					const String& name = o->getDebugString();

					LOG(0, ".. Native alive: %s '%s' (%08x) VMRef: %d, NativeRef: %d\n", tname, name.c_str(), o, o->getScriptRefCount(), o->getNativeRefCount());
					++native;
				}
			}
			else
				++released;
		}

		LOG(0, "%s ScriptRef clears %d: %d released, alive: %d native, %d invalid\n", error ? "***" : "..", _retained.size(), released, native, error);
#endif

		_retained.clear();
	}
};

////////////////////////////////////////////////////////////////////////////////

static void CallErrorHandler(HSQUIRRELVM v, HSQOBJECT& peer, ScriptResult r, const char* methodname)
{
	if (r == SCRIPT_CALL_OK || r == SCRIPT_CALL_NO_METHOD) return;

	const char* errtype = "???";

	switch (r)
	{
	case SCRIPT_CALL_INVALID_ARGS:		errtype = "invalid args"; break;
	case SCRIPT_CALL_ERR:				errtype = "call error"; break;
	case SCRIPT_CALL_NO_METHOD:			errtype = "no method"; break;
	case SCRIPT_CALL_OK:				break;
	}

	const char* errobj = "???";
	const char* lasterr = "???";

	SQInteger top = sq_gettop(v);

	sq_pushobject(v, peer);
	if (SQ_ISWEAKREF(sq_gettype(v, -1)))
	{
		sq_getweakrefval(v, -1);
		sq_replace(v, -2);
	}
	sq_tostring(v, -1);
	sq_getstring(v, -1, &errobj);
	sq_getlasterror(v);
	sq_tostring(v, -1);
	sq_getstring(v, -1, &lasterr);
	LOG(0, "*** %s.%s(): %s (%s)\n", errobj, methodname, errtype, lasterr);

	sq_settop(v, top);
}

////////////////////////////////////////////////////////////////////////////////

ScriptPeer::ScriptPeer(HSQUIRRELVM v, int stackidx)
{
	_runtime = NULL;
	_peerType = SCRIPT_PEER_NONE;
	sq_resetobject(&_peer);

	if (v == NULL || sq_gettype(v, stackidx) == OT_NULL)
		return;

	replace(v, stackidx);
}

ScriptPeer::~ScriptPeer()
{
	releasePeer();
}

void ScriptPeer::replace(HSQUIRRELVM v, int stackidx)
{
	if (_runtime) releasePeer();

	_peerType = SCRIPT_PEER_NONE;
	_runtime = ScriptRuntime::getRuntime(v);

	// obtain strong ref
	if (SQ_ISWEAKREF(sq_gettype(v, stackidx)))
		sq_getweakrefval(v, stackidx);
	else
		sq_push(v, stackidx);

	sq_getstackobj(v, -1, &_peer);
	sq_addref(v, &_peer);
	_peerType = SCRIPT_PEER_STRONG;

	sq_poptop(v);
}

void ScriptPeer::releasePeer()
{
	if (_runtime == NULL || !_runtime->isStarted()) return;
	if (_peerType == SCRIPT_PEER_NONE) return;

	sq_release(_runtime->getWorker(), &_peer);
	_peerType = SCRIPT_PEER_NONE;
	_runtime = NULL;
}

SQObjectRef ScriptPeer::getObjectRef(HSQUIRRELVM v)
{
	if (_peerType == SCRIPT_PEER_NONE)
		return SQObjectRef();

	if (_peerType == SCRIPT_PEER_STRONG)
		return _peer;

	SQObjectPtr val;
	_getrealval(_peer, v, val);
	return val;
}

int ScriptPeer::pushObject(HSQUIRRELVM v)
{
	if (_peerType == SCRIPT_PEER_NONE) 
	{
		sq_pushnull(v);
		return 1;
	}
	
	sq_pushobject(v, _peer);

	if (_peerType == SCRIPT_PEER_STRONG) return 1;

	// pushes always strong ref
	sq_getweakrefval(v, -1);
	sq_replace(v, -2);

	return 1; 
}

int ScriptPeer::pushWeakRef(HSQUIRRELVM v)
{
	if (_peerType == SCRIPT_PEER_NONE) 
	{
		sq_pushnull(v);
		return 1;
	}

	sq_pushobject(v, _peer);

	if (_peerType == SCRIPT_PEER_WEAK) return 1;

	sq_weakref(v, -1, false);
	sq_replace(v, -2);

	return 1;
}

void ScriptPeer::makeStrong(HSQUIRRELVM v)
{
	if (_peerType == SCRIPT_PEER_NONE) return;

	if (_peerType == SCRIPT_PEER_STRONG) return;

	// convert weak to strong ref
	sq_pushobject(v, _peer);			// [weak]
	sq_getweakrefval(v, -1);			// [weak] [strong]
	sq_release(v, &_peer);
	sq_getstackobj(v, -1, &_peer);
	sq_addref(v, &_peer);
	sq_pop(v, 2);						// <empty>

	_peerType = SCRIPT_PEER_STRONG;
}

void ScriptPeer::makeWeak(HSQUIRRELVM v)
{
	if (_peerType == SCRIPT_PEER_NONE) return;

	if (_peerType == SCRIPT_PEER_WEAK) return;

	RefCounted::incRefCount();  // prevent to be deleted by side effect below 'sq_weakref' during operation

	// convert strong to weak ref
	sq_pushobject(v, _peer);			// [strong]
	sq_weakref(v, -1, false);			// [strong] [weak]
	sq_release(v, &_peer);
	sq_getstackobj(v, -1, &_peer);
	sq_addref(v, &_peer);
	sq_pop(v, 2);						// <empty>

	_peerType = SCRIPT_PEER_WEAK;

	RefCounted::decRefCount();
}

ScriptResult ScriptPeer::callMethod(const char* method, int nargs, bool needRet)
{
	if (_peerType == SCRIPT_PEER_NONE || _runtime == NULL) return SCRIPT_CALL_NO_METHOD;

	HSQUIRRELVM v = _runtime->getWorker();
	if (v == NULL) return SCRIPT_CALL_OK;

	SQInteger clear = sq_gettop(v) - nargs;
	if (clear < 0)
	{
		LOG(0, "*** ScriptObject::InvokeMethod: need nargs on stack\n");
		sq_settop(v, 0);
		return SCRIPT_CALL_INVALID_ARGS;
	}

	SQRESULT r;

	sq_insert(v, - nargs, 2);										// stack: no no a1 a2 a3
	pushObject(v);													// stack: no no a1 a2 a3 o 
	sq_pushstring(v, method, -1);									// stack: no no a1 a2 a3 o m
	r = sq_get(v, -2);												// stack: no no a1 a2 a3 o fn
	if (SQ_FAILED(r)) { sq_settop(v, clear); return SCRIPT_CALL_NO_METHOD; }
	sq_replace(v, - nargs - 4);										// stack: fn no a1 a2 a3 o
	sq_replace(v, - nargs - 2);										// stack: fn o  a1 a2 a3
	r = sq_call(v, nargs + 1, SQTrue, SQTrue);						// stack: fn ret
	if (SQ_FAILED(r)) { CallErrorHandler(v, _peer, SCRIPT_CALL_ERR, method); sq_settop(v, clear); return SCRIPT_CALL_ERR; }
	sq_replace(v, -2);												// stack: ret
	if (!needRet) 
		sq_poptop(v);

	return SCRIPT_CALL_OK;
}

ScriptResult ScriptPeer::callMethod(const HSQOBJECT& nameObj, int nargs, bool needRet /*= false*/)
{
	if (_peerType == SCRIPT_PEER_NONE || _runtime == NULL) return SCRIPT_CALL_NO_METHOD;

	HSQUIRRELVM v = _runtime->getWorker();
	if (v == NULL) return SCRIPT_CALL_OK;

	SQInteger clear = sq_gettop(v) - nargs;
	if (clear < 0)
	{
		LOG(0, "*** ScriptObject::InvokeMethod: need nargs on stack\n");
		sq_settop(v, 0);
		return SCRIPT_CALL_INVALID_ARGS;
	}

	SQRESULT r;

	sq_insert(v, - nargs, 2);										// stack: no no a1 a2 a3
	pushObject(v);													// stack: no no a1 a2 a3 o 
	sq_pushobject(v, nameObj);										// stack: no no a1 a2 a3 o m
	r = sq_get(v, -2);												// stack: no no a1 a2 a3 o fn
	if (SQ_FAILED(r)) { sq_settop(v, clear); return SCRIPT_CALL_NO_METHOD; }
	sq_replace(v, - nargs - 4);										// stack: fn no a1 a2 a3 o
	sq_replace(v, - nargs - 2);										// stack: fn o  a1 a2 a3
	r = sq_call(v, nargs + 1, SQTrue, SQTrue);						// stack: fn ret
	if (SQ_FAILED(r)) {
		const char* method = "???";
		sq_pushobject(v, nameObj);
		sq_tostring(v, -1);
		sq_getstring(v, -1, &method);
		CallErrorHandler(v, _peer, SCRIPT_CALL_ERR, method); 
		sq_settop(v, clear); 
		return SCRIPT_CALL_ERR; 
	}
	sq_replace(v, -2);												// stack: ret
	if (!needRet) 
		sq_poptop(v);

	return SCRIPT_CALL_OK;
}

ScriptResult ScriptPeer::callWith(const HSQOBJECT& closureObj, int nargs, bool needRet /*= false*/)
{
	if (_peerType == SCRIPT_PEER_NONE || _runtime == NULL) return SCRIPT_CALL_NO_METHOD;

	HSQUIRRELVM v = _runtime->getWorker();
	if (v == NULL) return SCRIPT_CALL_OK;

	SQInteger clear = sq_gettop(v) - nargs;
	if (clear < 0)
	{
		LOG(0, "*** ScriptObject::InvokeMethod: need nargs on stack\n");
		sq_settop(v, 0);
		return SCRIPT_CALL_INVALID_ARGS;
	}

	if (closureObj._type == OT_NULL)
	{
		sq_settop(v, clear);
		return SCRIPT_CALL_NO_METHOD;
	}

	SQRESULT r;

	sq_insert(v, - nargs, 2);										// stack: no no a1 a2 a3
	pushObject(v);													// stack: no no a1 a2 a3 o 
	sq_pushobject(v, closureObj);									// stack: no no a1 a2 a3 o fn
	sq_replace(v, - nargs - 4);										// stack: fn no a1 a2 a3 o
	sq_replace(v, - nargs - 2);										// stack: fn o  a1 a2 a3
	r = sq_call(v, nargs + 1, SQTrue, SQTrue);						// stack: fn ret
	if (SQ_FAILED(r)) {
		const char* method = "???";
		sq_pushobject(v, closureObj);
		sq_tostring(v, -1);
		sq_getstring(v, -1, &method);
		CallErrorHandler(v, _peer, SCRIPT_CALL_ERR, method); 
		sq_settop(v, clear); 
		return SCRIPT_CALL_ERR; 
	}
	sq_replace(v, -2);												// stack: ret
	if (!needRet) 
		sq_poptop(v);

	return SCRIPT_CALL_OK;
}

ScriptResult ScriptPeer::callEvent(const Event* evt)
{
	if (_peerType == SCRIPT_PEER_NONE || evt == NULL) return SCRIPT_CALL_NO_METHOD;

	HSQUIRRELVM v = _runtime->getWorker();

	SQInteger clear = sq_gettop(v);
	SQRESULT r;

	Ref<ScriptPeer> safe = this;

	sq_pushobject(v, _runtime->_eventsTable);								// stack: <et>
	sq_pushuserpointer(v, (void*)EventInfo::getByEventId(evt->getId()));	// stack: <et> <ei>
	r = sq_get(v, -2);														// stack: <et> mn
	if (SQ_FAILED(r)) { sq_settop(v, clear); return SCRIPT_CALL_NO_METHOD; }
	sq_replace(v, -2);														// stack: mn

	pushObject(v);															// stack: mn o
	sq_push(v, -2);															// stack: mn o mn
	r = sq_get(v, -2);														// stack: mn o fn
	if (SQ_FAILED(r)) { sq_settop(v, clear); return SCRIPT_CALL_NO_METHOD; }
	sq_push(v, -2);															// stack: mn o fn o
	NitBind::push(v, const_cast<Event*>(evt));								// stack: mn o fn evt
	r = sq_call(v, 2, false, true);											// stack: mn o fn
	if (SQ_FAILED(r)) { CallErrorHandler(v, _peer, SCRIPT_CALL_ERR, sqx_getstring(v, -3)); sq_settop(v, clear); return SCRIPT_CALL_ERR; }
	sq_settop(v, clear);													// stack: empty

	return SCRIPT_CALL_OK;
}

String ScriptPeer::getTypeName()
{
	if (this == NULL)
		return "(null ScriptPeer)";

	HSQUIRRELVM v = _runtime->getWorker();

	SQRESULT r;
	SQInteger top = sq_gettop(v);

	pushObject(v);						// [obj]
	sq_pushstring(v, "_classname", -1);	// [obj] "_classname"
	r = sq_get(v, -2);					// [obj] [_classname]
	if (SQ_FAILED(r)) { sq_settop(v, top); return "(ScriptPeer)"; }

	const char* name = "???";
	sq_getstring(v, -1, &name);
	String ret = name;
	sq_settop(v, top);					// <empty>
	return ret;
}

////////////////////////////////////////////////////////////////////////////////

ScriptClosure::ScriptClosure()
{
	_runtime = NULL;
}

ScriptClosure::ScriptClosure(HSQUIRRELVM v, int stackidx)
{
	_runtime = NULL;
	replace(v, stackidx);
}

ScriptClosure::~ScriptClosure()
{
	releasePeer();
}

void ScriptClosure::releasePeer()
{
	if (_runtime == NULL || !_runtime->isStarted()) return;

	HSQUIRRELVM v = _runtime->getWorker();

	sq_release(v, &_peer);
	_runtime = NULL;
}

void ScriptClosure::replace(HSQUIRRELVM v, int stackidx)
{
	if (v == NULL || sq_gettype(v, stackidx) == OT_NULL)
	{
		releasePeer();
		return;
	}

	SQObjectType t = sq_gettype(v, stackidx);
	if (SQ_ISWEAKREF(t))
	{
		sq_getweakrefval(v, stackidx);
		t = sq_gettype(v, -1);
		sq_poptop(v);
	}

	if (t != OT_CLOSURE && t != OT_NATIVECLOSURE)
		sqx_throw(v, "only closure can be used"); 

	releasePeer();

	_runtime = ScriptRuntime::getRuntime(v);
	sq_resetobject(&_peer);
	sq_getstackobj(v, stackidx, &_peer);
	sq_addref(v, &_peer);
}

ScriptResult ScriptClosure::call(int nargs, bool needRet)
{
	if (_runtime == NULL) return SCRIPT_CALL_NO_METHOD;

	HSQUIRRELVM v = _runtime->getWorker();

	SQInteger clear = sq_gettop(v) - nargs;
	if (clear < 0)
	{
		LOG(0, "*** ScriptMethod::Invoke: need nargs on stack\n");
		sq_settop(v, 0);
		return SCRIPT_CALL_INVALID_ARGS;
	}

	SQRESULT r;

	Ref<ScriptClosure> safe = this;

	// call function without 'this' - closure should have binded function env or should not use 'this'.

	sq_insert(v, - nargs, 2);							// [null] [null] [a1] [a2] [a3]
	sq_pushobject(v, _peer);							// [null] [null] [a1] [a2] [a3] [fn]
	sq_replace(v, - nargs - 3);							// [fn]   [null] [a1] [a2] [a3]
	r = sq_call(v, nargs + 1, SQTrue, SQTrue);			// [fn]   [ret]
	if (SQ_FAILED(r)) { CallErrorHandler(v, _peer, SCRIPT_CALL_ERR, ""); sq_settop(v, clear); return SCRIPT_CALL_ERR; }
	sq_replace(v, -2);									// [ret]
	if (!needRet) sq_poptop(v);

	return SCRIPT_CALL_OK;
}

int ScriptClosure::pushClosure(HSQUIRRELVM v)
{
	if (_runtime == NULL) return 0;

	sq_pushobject(v, _peer);

	if (_peer._type != OT_WEAKREF) return 1;

	sq_getweakrefval(v, -1);
	sq_replace(v, -2);

	return 1; 
}

int ScriptClosure::pushWeakRef(HSQUIRRELVM v)
{
	if (_runtime == NULL) return 0;

	sq_pushobject(v, _peer);

	if (_peer._type == OT_WEAKREF) return 1;

	sq_weakref(v, -1, false);
	sq_replace(v, -2);

	return 1;
}

////////////////////////////////////////////////////////////////////////////////

ScriptWaitBlock::ScriptWaitBlock(ScriptRuntime* runtime)
{
	_runtime = runtime;
	_killing = false;
	_allowCount = 0;
}

ScriptWaitBlock::~ScriptWaitBlock()
{
	killAll();
}

SQInteger ScriptWaitBlock::wait(HSQUIRRELVM th, uint signalMask)
{
	if (_killing)
		return sq_throwerror(th, "can't suspend while killing all");

	if (_allowCount > 0 && _suspended.size() >= _allowCount)
		return sq_throwerror(th, "can't suspend more");

	HSQUIRRELVM m = ScriptRuntime::getRuntime(th)->getRoot();

	if (th == m)
		return sq_throwerror(th, "can't suspend system threads");

	SQRESULT ret = sq_suspendvm(th);
	if (ret == SQ_ERROR) return ret;

	SQInteger mTop = sq_gettop(m);
	SQInteger thTop = sq_gettop(th);

	Entry e;
	e.signalMask = signalMask;

	sq_pushthread(m, th);

	sq_weakref(m, -1, false);
	sq_resetobject(&e.weakThread);
	sq_getstackobj(m, -1, &e.weakThread);
	sq_addref(m, &e.weakThread);

	sq_pushthreadlocal(m, th);
	sq_pushstring(m, "wb", -1);
	sq_pushuserpointer(m, this);
	sq_newslot(m, -3, false);

	sq_settop(th, thTop);
	sq_settop(m, mTop);

	_suspended.push_back(e);

	return ret;
}

SQInteger ScriptWaitBlock::pushAll(HSQUIRRELVM v)
{
	sq_newarray(v, 0);
	for (ThreadEntries::iterator itr = _suspended.begin(), end = _suspended.end(); itr != end; ++itr)
	{
		sq_pushobject(v, itr->weakThread);
		sq_getweakrefval(v, -1);
		sq_replace(v, -2);

		if (sq_gettype(v, -1) == OT_THREAD)
			sq_arrayappend(v, -2);
		else
			sq_poptop(v);
	}
	return 1;
}

int ScriptWaitBlock::signal(uint signalFlags)
{
	if (_runtime == NULL || !_runtime->isStarted())
		return 0;

	int recvCount = 0;

	HSQUIRRELVM m = _runtime->getRoot();

	uint count = _suspended.size();

	while (count && !_suspended.empty())
	{
		Entry e = _suspended.front();
		_suspended.pop_front();
		--count;

		uint waiting = e.signalMask;
		uint recv = waiting & signalFlags;

		sq_pushobject(m, e.weakThread);
		sq_getweakrefval(m, -1);
		HSQUIRRELVM th = NULL;
		sq_getthread(m, -1, &th);
		sq_pop(m, 2);

		bool ok = th && sq_getvmstate(th) == SQ_VMSTATE_SUSPENDED;

		if (ok && signalFlags != 0 && recv == 0)
		{
			// this thread doesn't need the signal.. send back to queue
			_suspended.push_back(e);
			continue;
		}

		if (ok)
		{
			SQInteger top = sq_gettop(th);

			SQUserPointer up = NULL;

			// check wb
			sq_pushthreadlocal(th, th);
			sq_pushstring(th, "wb", -1);
			ok = ok && SQ_SUCCEEDED(sq_deleteslot(th, -2, true));
			ok = ok && SQ_SUCCEEDED(sq_getuserpointer(th, -1, &up));
			ok = ok && (up == this);
			sq_settop(th, top);
		}

		if (!ok)
		{
			sq_release(m, &e.weakThread);
			continue;
		}

		++recvCount;

		_runtime->clearThreadTimeout(th);

		sq_pushinteger(th, recv);
		sq_wakeupvm(th, SQTrue, SQFalse, SQTrue, SQFalse);

		if (sq_getvmstate(th) == SQ_VMSTATE_IDLE)
			sq_closethread(th);

		sq_release(m, &e.weakThread);
	}

	return recvCount;
}

void ScriptWaitBlock::killAll(const SQChar* msg /*= NULL*/)
{
	if (_runtime == NULL || !_runtime->isStarted())
		return;

	if (_killing) return;

	_killing = true;

	if (msg == NULL)
		msg = "waitblock kills";

	HSQUIRRELVM m = _runtime->getRoot();

	for (ThreadEntries::iterator itr = _suspended.begin(), end = _suspended.end(); itr != end; ++itr)
	{
		HSQOBJECT& wt = itr->weakThread;

		sq_pushobject(m, wt);
		sq_getweakrefval(m, -1);
		HSQUIRRELVM th = NULL;
		sq_getthread(m, -1, &th);
		sq_pop(m, 2);

		bool ok = th && sq_getvmstate(th) == SQ_VMSTATE_SUSPENDED;

		if (ok)
		{
			SQInteger top = sq_gettop(th);

			SQUserPointer up = NULL;

			// check wb
			sq_pushthreadlocal(th, th);
			sq_pushstring(th, "wb", -1);
			ok = ok && SQ_SUCCEEDED(sq_deleteslot(th, -2, true));
			ok = ok && SQ_SUCCEEDED(sq_getuserpointer(th, -1, &up));
			ok = ok && (up == this);
			sq_settop(th, top);
		}

		if (!ok)
		{
			sq_release(m, &wt);
			continue;
		}

		_runtime->clearThreadTimeout(th);

		sq_throwerror(th, msg);
		sq_wakeupvm(th, SQFalse, SQFalse, SQFalse, SQTrue);

		if (sq_getvmstate(th) == SQ_VMSTATE_IDLE)
			sq_closethread(th);

		sq_release(m, &wt);
	}
	_suspended.clear();

	_killing = false;
}

SQInteger ScriptWaitBlock::clearWaitBlockFlag(HSQUIRRELVM th)
{
	SQInteger top = sq_gettop(th);

	sq_pushthreadlocal(th, th);
	sq_pushstring(th, "wb", -1);
	sq_deleteslot(th, -2, false);
	sq_settop(th, top);

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

SQInteger ScriptIO::lexfeed_ASCII(SQUserPointer p)
{
	StreamReader* s = (StreamReader*)p;
	char c;
	if (s->readRaw(&c, 1) == sizeof(c)) return c;
	return 0;
}

SQInteger ScriptIO::lexfeed_UTF8(SQUserPointer p)
{
	StreamReader* s = (StreamReader*)p;
#define READ() if (s->readRaw(&inchar, 1) != sizeof(inchar)) return 0

	static const SQInteger utf8_lengths[16] =
	{
		1,1,1,1,1,1,1,1,        /* 0000 to 0111 : 1 byte (plain ASCII) */
		0,0,0,0,                /* 1000 to 1011 : not valid */
		2,2,                    /* 1100, 1101 : 2 bytes */
		3,                      /* 1110 : 3 bytes */
		4                       /* 1111 :4 bytes */
	};

	static unsigned char byte_masks[5] = {0,0,0x1f,0x0f,0x07};
	unsigned char inchar;
	SQInteger c = 0;
	READ();
	c = inchar;
	//
	if(c >= 0x80) {
		SQInteger tmp;
		SQInteger codelen = utf8_lengths[c>>4];
		if(codelen == 0) 
			return 0;
		//"invalid UTF-8 stream";
		tmp = c&byte_masks[codelen];
		for(SQInteger n = 0; n < codelen-1; n++) {
			tmp<<=6;
			READ();
			tmp |= inchar & 0x3F;
		}
		c = tmp;
	}
	return c;

#undef READ
}

SQInteger ScriptIO::lexfeed_UCS2_LE(SQUserPointer p)
{
	StreamReader* s = (StreamReader*)p;
	UniChar c;
	if (s->readRaw(&c, sizeof(c)) == sizeof(c)) return (SQChar)c;
	return 0;
}

SQInteger ScriptIO::lexfeed_UCS2_BE(SQUserPointer p)
{
	StreamReader* s = (StreamReader*)p;
	UniChar c;
	if (s->readRaw(&c, sizeof(c)) == sizeof(c)) 
	{
		c = ((c>>8)&0x00FF)| ((c<<8)&0xFF00);
		return (SQChar)c;
	}
	return 0;
}

SQInteger ScriptIO::bytecode_read(SQUserPointer p, SQUserPointer buf, SQInteger size)
{
	StreamReader* s = (StreamReader*)p;

	SQInteger ret;
	if ( (ret = s->readRaw(buf, size)) != 0) return ret;
	return -1;
}

SQInteger ScriptIO::bytecode_write(SQUserPointer p, SQUserPointer buf, SQInteger size)
{
	StreamWriter * s= (StreamWriter*)p;
	return s->writeRaw(buf, size);
}

SQRESULT ScriptIO::loadstream(HSQUIRRELVM v, StreamReader* srcReader, const String& source_id, SQBool printerror)
{
//	LOG(0, ".. loading '%s'\n", source_id.c_str());

	SQInteger ret;
	unsigned short us;
	unsigned char uc;
	SQLEXREADFUNC func = lexfeed_ASCII;

	Ref<StreamReader> reader = srcReader;

	if (!reader->isSeekable())
	{
		LOG_TIMESCOPE(0, ".. buffering '%s'", reader->getUrl().c_str());
		reader = new MemoryBuffer::Reader(reader);
	}

	reader->seek(0);
	ret = reader->readRaw(&us, sizeof(us));
	if (ret != 2) 
	{
		//probably an empty file
		us = 0;
	}
	if (us == SQ_BYTECODE_STREAM_TAG) 
	{ 
		//BYTECODE
		reader->seek(0);
		if (SQ_SUCCEEDED(sq_readclosure(v,bytecode_read,reader))) 
		{
			return SQ_OK;
		}
	}
	else 
	{ 
		//SCRIPT
		switch(us)
		{
			// TODO: gotta swap the next 2 lines on BIG endian machines
		case 0xFFFE: func = lexfeed_UCS2_BE; break;//UTF-16 little endian;
		case 0xFEFF: func = lexfeed_UCS2_LE; break;//UTF-16 big endian;
		case 0xBBEF: 
			if (reader->read(&uc, sizeof(uc)) == 0)
			{
				return sq_throwerror(v,_SC("io error")); 
			}
			if (uc != 0xBF) 
			{ 
				return sq_throwerror(v,_SC("Unrecognozed ecoding")); 
			}

			// TODO: At now, we will treat BOM prefixed utf-8 as plain ascii.
			// TODO: Decide this later!
			{
				bool treatBOMPrefiedUTF8AsAscii = true;

				if (treatBOMPrefiedUTF8AsAscii)
					func = lexfeed_ASCII;
				else
					func = lexfeed_UTF8;
			}
			break;
		default: 
			// ascii
			reader->seek(0);
		}

		if(SQ_SUCCEEDED(sq_compile(v,func, reader, source_id.c_str(), printerror)))
		{
			return SQ_OK;
		}
	}
	return SQ_ERROR;
}

////////////////////////////////////////////////////////////////////////////////

class ScriptRuntimeLib : NitBind
{
public:
	static void install(HSQUIRRELVM v)
	{
		sq_pushroottable(v);

		sq_register_h(v, "print", logprint,							"(line)");

		sq_register_h(v, "isfile", isfile,							"(filename, locator=null): bool");
		sq_register_h(v, "dofile", dofile,							"(filename, locator=null): ret // returns executed file's return value");
		sq_register_h(v, "loadfile", loadfile,						"(filename, locator=null): func // returns loaded file as a function");

		sq_register_h(v, "dostream", dostream,						"(reader: StreamReader): ret // returns executed code's return value");
		sq_register_h(v, "loadstream", loadstream,					"(reader: StreamReader): func // returns loaded code as a function");

		sq_dostring(v, "function printf(...) \"(fmt, ...)\" { print(format.acall(this, vargv)); }");

		sq_register_h(v, "getregistrytable", getRegistryTable,		"(): table");
		sq_register_h(v, "getcachetable", getCacheTable,			"(): table");
		sq_register_h(v, "getdefaultdelegate", GetDefaultDelegate,	"(obj): table");

		sq_register_h(v, "debugbreak", debugbreak,					"() // jumps into devenv debugger");
		sq_getdefaultdelegate(v, OT_STRING);

		// Add methods to 'string' delegate
		sq_register_h(v, "wildcard", Str_Wildcard,	"(wildcard: string, ignoreCase=false): bool");
		sq_register_h(v, "replace", Str_Replace, "(substr: string, replace: string, begin=0, end=this.len()): string // can use minus for begin, end");
		sq_register_h(v, "utf_len",		Str_UTF8Len, "(): int");
		sq_register_h(v, "utf_char",	Str_UTF8Char, "(i): int");
		sq_register_h(v, "utf_chars",	Str_UTF8Chars, "([begin[, end]]): array<int>");
		sq_register_h(v, "utf_slice",	Str_UTF8Slice, "(begin[, end]): string");

		sq_poptop(v);

		sq_poptop(v); // root
	}

	static void uninstall(HSQUIRRELVM v)
	{
	}

	static SQInteger Str_Wildcard(HSQUIRRELVM v)
	{
		return push(v, Wildcard::match(getString(v, 2), getString(v, 1), optBool(v, 3, true)));
	}

	static SQInteger Str_Replace(HSQUIRRELVM v)
	{
		const char* str = getString(v, 1);
		const char* substr = getString(v, 2);
		const char* replace = getString(v, 3);

		int len = sq_getsize(v, 1);

		String ret;
		ret.reserve(len);

		int begin = optInt(v, 4, 0);
		if (begin < 0) begin = len + begin;
		if (begin < 0) begin = 0;
		if (begin > len) begin = len;
		int end = optInt(v, 5, len);
		if (end < 0) end = len + end;
		if (end < 0) end = 0;
		if (end > len) end = len;
		const char* eos = str + end;

		if (begin > 0)
			ret.append(str, str + begin);

		str += begin;
		int substrLen = sq_getsize(v, 2);

		while (str < eos)
		{
			const char* found = strstr(str, substr);
			if (found)
			{
				ret.append(str, found);
				ret.append(replace);
				str = found + substrLen;
			}
			else
			{
				ret.append(str);
				break;
			}
		}

		return push(v, ret);
	}

	static SQInteger Str_UTF8Char(HSQUIRRELVM v)
	{
		return push(v, Unicode::uniCharAt(getString(v, 1), getInt(v, 2)));
	}

	static SQInteger Str_UTF8Chars(HSQUIRRELVM v)
	{
		const char* utf8 = getString(v, 1);
		int begin = optInt(v, 2, 0);
		int end = optInt(v, 3, -1);
		
		sq_newarray(v, 0);

		utf8 += Unicode::utf8ByteCount(utf8, begin);

		if (end >= 0)
		{
			for (int i = begin; i < end; ++i)
			{
				int ch = Unicode::utf8Advance(utf8);
				if (ch == 0) break;
				sq_pushinteger(v, ch);
				sq_arrayappend(v, -2);
			}
		}
		else
		{
			int ch;
			while ((ch = Unicode::utf8Advance(utf8)) != 0)
			{
				sq_pushinteger(v, ch);
				sq_arrayappend(v, -2);
			}
		}

		return 1;
	}

	static SQInteger Str_UTF8Len(HSQUIRRELVM v)
	{
		const char* utf8 = getString(v, 1);
		return push(v, Unicode::utf8Length(utf8));
	}

	static SQInteger Str_UTF8Slice(HSQUIRRELVM v)
	{
		const char* utf8 = getString(v, 1);
		if (isNone(v, 3))
		{
			int begin = Unicode::utf8ByteCount(utf8, getInt(v, 2));
			sq_pushstring(v, utf8 + begin, -1);
			return 1;
		}
		else
		{
			int begin = getInt(v, 2);
			int end = getInt(v, 3);
			int count = end - begin;
			
			begin = Unicode::utf8ByteCount(utf8, begin);
			count = Unicode::utf8ByteCount(utf8 + begin, count);
			sq_pushstring(v, utf8 + begin, count);
			return 1;
		}
	}

	static SQInteger debugbreak(HSQUIRRELVM v)
	{
#ifdef NIT_WIN32
		__debugbreak();
#else
		NIT_THROW(EX_NOT_SUPPORTED); 
#endif
		return 0;
	}

	static SQInteger isfile(HSQUIRRELVM v)
	{
		const char* filename = getString(v, 2);
		StreamLocator* locator = opt<StreamLocator>(v, 3, NULL);

		ScriptRuntime* runtime = ScriptRuntime::getRuntime(v);

		sq_pushbool(v, runtime->locateUnit(filename, locator) != NULL);
		return 1;
	}

	static SQInteger dofile(HSQUIRRELVM v)
	{
		const char* filename = getString(v, 2);
		StreamLocator* locator = opt<StreamLocator>(v, 3, NULL);

		ScriptRuntime* runtime = ScriptRuntime::getRuntime(v);
		Ref<ScriptUnit> unit = runtime->createUnit(filename, locator);

		if (unit == NULL)
			return sqx_throwfmt(v, "can't do file: %s", filename);

		runtime->_unitStack.push_back(unit);

		// push 'this'
		sq_push(v, 1);
		SQRESULT r = unit->execute(v);

		runtime->_unitStack.pop_back();

		// removes 'this' from stack
		if (SQ_SUCCEEDED(r))
			sq_replace(v, -2);
		else
			sq_poptop(v);

		return r;
	}

	static SQInteger loadfile(HSQUIRRELVM v)
	{
		const char* filename = getString(v, 2);
		StreamLocator* locator = opt<StreamLocator>(v, 3, NULL);

		ScriptRuntime* runtime = ScriptRuntime::getRuntime(v);
		Ref<ScriptUnit> unit = runtime->createUnit(filename, locator);

		if (unit == NULL)
			return sqx_throwfmt(v, "can't load file: %s", filename);

		unit->compile(v);

		return unit->pushBody(v);
	}

	static SQInteger dostream(HSQUIRRELVM v)
	{
		ScriptRuntime* runtime = ScriptRuntime::getRuntime(v);
		StreamReader* reader = get<StreamReader>(v, 2);
		SQRESULT r;

		SQInteger top = sq_gettop(v);

		{
			String id = runtime->unitSourceID(reader->getSource());
			LOG_TIMESCOPE(0, ".. compiling '%s'", id.c_str());
			r = ScriptIO::loadstream(v, reader, id, true);

			if (SQ_FAILED(r)) { sq_settop(v, top); return SQ_ERROR; }
		}

		{
			LOG_TIMESCOPE(0, ".. executing '%s'", reader->getUrl().c_str());

			sq_push(v, 1);						// [this] [closure] [this]
			r = sq_call(v, 1, true, true);		// [this] [closure] [ret]

			if (SQ_FAILED(r)) { sq_settop(v, top); return r; }
			ASSERT(r == SQ_OK); // TODO: suspend?
			sq_replace(v, -2);					// [this] [ret]
		}

		return 1;
	}

	static SQInteger loadstream(HSQUIRRELVM v)
	{
		ScriptRuntime* runtime = ScriptRuntime::getRuntime(v);
		StreamReader* reader = get<StreamReader>(v, 2);
		SQRESULT r;

		SQInteger top = sq_gettop(v);

		{
			String id = runtime->unitSourceID(reader->getSource());
			LOG_TIMESCOPE(0, ".. compiling '%s'", id.c_str());
			r = ScriptIO::loadstream(v, reader, id, true);

			if (SQ_FAILED(r)) { sq_settop(v, top); return SQ_ERROR; }
		}

		return 1;
	}

#ifndef NIT_NO_LOG
	static Ref<LogChannel> GetLogChannel(HSQUIRRELVM v)
	{
		LogManager& l = LogManager::getSingleton();

		Ref<LogChannel> channel = NULL;

		sq_pushthread(v, v);
		sq_pushuserpointer(v, &l);
		if (SQ_SUCCEEDED(sq_get(v, -2)))
		{
			channel = NitBind::get<LogChannel>(v, -1);
			sq_pop(v, 2);
		}
		else
		{
			const SQChar* thname = "sol_thread";
			sq_getthreadname(v, &thname);
			channel = new LogChannel(thname);
			sq_pushuserpointer(v, &l);
			NitBind::push(v, channel.get());
			sq_newslot(v, -3, false);
			sq_pop(v, 1);
		}

		return channel;
	}

	static SQInteger logprint(HSQUIRRELVM v)
	{	
		const char *msg;
		int msgLen = -1;
		uint32 tagBuf;

		sq_tostring(v, 2);
		sq_getstring(v, -1, &msg); 

		LogManager& l = LogManager::getSingleton();

		const char* tag = l.parseTag(msg, msgLen, tagBuf);
	
		Ref<LogChannel> channel = GetLogChannel(v);

		if (l.getLogLevel(l.tagId(tag)) >= LOG_LEVEL_ERROR)
		{
			SQStackInfos si;

			if (SQ_SUCCEEDED(sq_stackinfos(v, 1, &si)))
			{
				LogManager::getSingleton().doLog(channel, 0, si.source, si.line, si.funcname, tag, msg, msgLen, true);
				sq_poptop(v);
				return 0;
			}
		}

		LogManager::getSingleton().doLog(channel, 0, 0, 0, 0, tag, msg, msgLen, true);
		sq_poptop(v);

		return 0;
	}

	static void printfunc(HSQUIRRELVM v,const SQChar *fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		LogManager::getSingleton().vlog(0, 0, 0, 0, 0, fmt, args);
		va_end(args);
	}

	static void logprintfunc(HSQUIRRELVM v,const SQChar *fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		LogManager::getSingleton().vlog(GetLogChannel(v), 0, 0, 0, 0, fmt, args);
		va_end(args);
	}
#else
	static Ref<LogChannel> GetLogChannel(HSQUIRRELVM v)							{ return NULL; }
	static SQInteger logprint(HSQUIRRELVM v)									{ return 0; }
	static void printfunc(HSQUIRRELVM v,const SQChar *fmt, ...)					{ }
	static void logprintfunc(HSQUIRRELVM v,const SQChar *fmt, ...)				{ }
#endif

	static SQInteger GetDefaultDelegate(HSQUIRRELVM v)
	{
		sq_getdefaultdelegate(v, sq_gettype(v, 2));
		return 1;
	}
};

////////////////////////////////////////////////////////////////////////////////

class ScriptWeakListener : public WeakRef::IListener
{
public:
	ScriptWeakListener(ScriptRuntime* runtime) : _runtime(runtime)
	{
	}

	virtual ~ScriptWeakListener()
	{
		Clear();
	}

	void Add(WeakSupported* object)
	{
		WeakRef* weak = object->_weak();

		// if we don't have it in the set, inc ref and add listener to it
		bool duplicated = !_weakRefs.insert(weak).second;
		if (duplicated)
		{
			LOG(0, "*** invalid add weak reference: 0x%08x for object 0x%08x\n", weak, object);
			assert(false);
			return;
		}
		weak->incRefCount();
		weak->addListener(this);
	}

	void Release(WeakSupported* object)
	{
		WeakRef* weak = object->_weak();
	
		size_t numErased = _weakRefs.erase(weak);
		if (numErased == 0)
		{
			LOG(0, "*** invalid release weak reference: 0x%08x for object 0x%08x\n", weak, object);
			assert(false);
			return;
		}
		weak->removeListener(this);
		weak->decRefCount(); 
	}

	void Clear()
	{
		for (Targets::iterator itr = _weakRefs.begin(), end = _weakRefs.end(); itr != end; ++itr)
		{
			WeakRef* weak = *itr;
			weak->removeListener(this);
			weak->decRefCount();
		}
		_weakRefs.clear();
	}

	virtual void onDestroy(WeakSupported* object)
	{
		if (!_runtime->isStarted()) return;

		HSQUIRRELVM v = _runtime->getRoot();

		SQObjectRef inst;
		if (SQ_SUCCEEDED(NitBindImpl::getCachedInstance(v, object, NULL, inst)))
		{
#if 0 && !defined(NIT_RELEASE) && !defined(NIT_SHIPPING)
			sq_pushstring(v, "_classname", -1);
			sq_get(v, -2);
			LOG(0, ".. weak (%s -> 0x%08x) destroyed outside script.\n", sqx_getstring(v, -1), object);
			sq_poptop(v);
#endif
			sqi_instance(inst)->Purge();
			// triggers ReleaseHook -> ReleaseWeak chain
		}
		else
		{
			// Type information has already gone because onDestroy() called by ~WeakRefSupported() destructor.
			LOG(0, "*** unbound weak 0x%08x destroyed outside script.\n", object);
		}
	}

	typedef unordered_set<WeakRef*>::type Targets;

	ScriptRuntime* _runtime;
	Targets _weakRefs;
};

////////////////////////////////////////////////////////////////////////////////

ScriptUnit::ScriptUnit(ScriptRuntime* runtime, const String& id, StreamSource* source)
: _runtime(runtime), _source(source), _id(id), _compiled(false), _required(false)
{
	sq_resetobject(&_body);
}

SQRESULT ScriptUnit::compile(HSQUIRRELVM v)
{
	if (_compiled) return SQ_OK;

	LOG_TIMESCOPE(0, ".. compiling '%s'", _id.c_str());

	SQInteger top = sq_gettop(v);

	Ref<StreamReader> reader = _source->open();
	SQRESULT r = ScriptIO::loadstream(v, reader, _id, true);

	sq_getstackobj(v, -1, &_body);
	sq_addref(v, &_body);

	sq_settop(v, top);

	_compiled = SQ_SUCCEEDED(r);
	return r;
}

SQRESULT ScriptUnit::execute(HSQUIRRELVM v)
{
	SQRESULT r = compile(v);

	if (SQ_FAILED(r)) 
		return r;

	LOG_TIMESCOPE(0, ".. executing '%s'", _id.c_str());

	// assumes that 'this' pushed at top

	// push body closure
	sq_pushobject(v, _body);

	// push 'this'
	sq_push(v, -2);

	r = sq_call(v, 1, true, true);
	
	// removes the closure
	if (SQ_SUCCEEDED(r))
		sq_remove(v, -2);
	else
		sq_poptop(v);

	return r;
}

SQRESULT ScriptUnit::unload(HSQUIRRELVM v)
{
	if (!_compiled) return SQ_OK;

	sq_release(v, &_body);

	_compiled = false;
	_required = false;

	// TODO: call OnUnload() on unit's table

	return SQ_OK;
}

SQInteger ScriptUnit::pushBody(HSQUIRRELVM v)
{
	if (!_compiled) 
		sq_pushnull(v);
	else
		sq_pushobject(v, _body);
	return 1;
}

////////////////////////////////////////////////////////////////////////////////

ScriptRuntime::ScriptRuntime()
{
	_started = false;

	_paused = false;
	_debugPaused = false;
	_stepGcPaused = false;

	_defaultLocator = NULL;

	_gcLoopHandler = createEventHandler(this, &ScriptRuntime::onGcLoop);
	_clockHandler = createEventHandler(this, &ScriptRuntime::onClock);
	_tickHandler = createEventHandler(this, &ScriptRuntime::onTick);

	_debugger = NULL;

	_wxWeakTracker = NULL;
}

ScriptRuntime::~ScriptRuntime()
{
	if (_started)
	{
		LOG(0, "*** ScriptRuntime %08x: destroyed without shutdown - shutdown now\n", this);
		shutdown();
	}
	else
		LOG(0, "++ ScriptRuntime %08x: destroyed\n", this);
}

#if !defined(NIT_SHIPPING)
// TODO: Not stable on cascaded ScriptRuntime or multi-threading
static size_t g_ScriptTotalLeaked = 0;
static size_t g_ScriptTotalAllocated = 0;
#endif

static void* SqUserMalloc(size_t size)
{
#if !defined(NIT_SHIPPING)
	g_ScriptTotalAllocated += size;
#endif

	return g_MemManager->Allocate(size, MEM_DEFAULT_ALIGNMENT, MEM_HINT_NONE);
}

static void* SqUserRealloc(void* ptr, size_t oldSize, size_t newSize)
{
#if !defined(NIT_SHIPPING)
	g_ScriptTotalAllocated -= oldSize;
	g_ScriptTotalAllocated += newSize;
#endif

	return g_MemManager->reallocate(ptr, newSize, oldSize, MEM_DEFAULT_ALIGNMENT, MEM_HINT_NONE);
}

static void SqUserFree(void* ptr, size_t size)
{
#if !defined(NIT_SHIPPING)
	g_ScriptTotalAllocated -= size;
#endif

	g_MemManager->deallocate(ptr, size);
}

extern void* RegisterNitLibCoreModule();

void ScriptRuntime::startup()
{
	if (_started) return;

	LOG_TIMESCOPE(0, "++ ScriptRuntime %08x: startup", this);

	if (sq_user_malloc == NULL)
	{
		LOG(0, ".. default sq memory fn installed\n");
		sq_user_malloc					= SqUserMalloc;
		sq_user_realloc 				= SqUserRealloc;
		sq_user_free					= SqUserFree;
	}

	_clockTime = 0.0f;
	_tickTime = 0.0f;

#if !defined(NIT_SHIPPING)
	_debugger = new ScriptDebugger();
#endif

	_weakListener = new ScriptWeakListener(this);
	_scriptRefManager = new ScriptRefManager();

	_started = true;
	_stepGcPaused = false;
	_nextTimeoutID = 1;

	_root = sq_open(1024);
	ASSERT(_root);

	sq_setthreadname(_root, "sol_main", -1);

	HSQUIRRELVM v = _root;

	sq_setprintfunc(v, ScriptRuntimeLib::printfunc, ScriptRuntimeLib::printfunc);
	sq_setrequirehandler(v, requireHandler);

	setDefaultErrorHandlers(v);

	if (_debugger)
	{
		_debugger->reset(); // TODO: Refactor to independant debugger?
		_debugger->attach(_root);
	}

	sq_pushroottable(v);

	// initialize baselibs
	sqstd_register_bloblib(v);
	sqstd_register_iolib(v);
	sqstd_register_systemlib(v);
	sqstd_register_mathlib(v);
	sqstd_register_stringlib(v);

	ScriptRuntimeLib::install(v);

	_timeWait = new ScriptWaitBlock(this);

	// Hide some dangling system functions
	sq_dostring(v, "::_newthread := delete(\"newthread\")");
	sq_dostring(v, "::_stream := delete(\"stream\")");
	sq_dostring(v, "::_file := delete(\"file\")");
	sq_dostring(v, "::_writeclosuretofile := delete(\"writeclosuretofile\")");

	// Add new methods related thread
	sq_register_h(v, "getthreads",		getThreads,			"(): array");
	sq_register_h(v, "costart",			costart,			"(func, ...): thread // calls func(...)");
	sq_register_h(v, "cotimeout",		cotimeout,			"(timeout: float, clocktime=false)");
	sq_register_h(v, "sleep",			sleep,				"(duration=0: float, clocktime=false) // sleep() waits just one tick");

	// Add methods to 'thread' delegate
	sq_getdefaultdelegate(v, OT_THREAD);
	sq_register_h(v, "call",			thread_Call,		"(...): ret");
	sq_register_h(v, "kill",			thread_Kill,		"() // throws exception to thread to kill");
 	sq_register_h(v, "wakeup",			thread_Wakeup,		"(wakeupval=null)");
	sq_register_h(v, "cotimeout",		thread_cotimeout,	"(timeout: float, clocktime=false)");
	sq_register  (v, "_dump",			thread_Dump);
	sq_register  (v, "_tostring",		thread_tostring);
	sq_poptop(v); // thread delegate

	// Add system function
	sq_register_h(v, "pausegc",			pauseGc,			"(flag: bool)");

	bool ok;
	
	sq_pushroottable(v);

	if (RegisterNitLibCoreModule())
	{
		LOG_TIMESCOPE(0, ".. NitBind::Install");
		ok = NitBindImpl::install(v, this);
	}
	else
	{
		NIT_THROW_FMT(EX_SCRIPT, "*** NitLibCoreModule is not registered\n");
	}

	initEventBindings();
	sq_dostring(v, "nit.Events := ::Events");

	if (!ok) return;

	// Add nit core objects
	NitBind::newSlot(v, -1, "mem",		g_MemManager);
	NitBind::newSlot(v, -1, "runtime",	this);

	sq_poptop(v); // root table

	// From now on, use log channel
	sq_setprintfunc(v, ScriptRuntimeLib::logprintfunc, ScriptRuntimeLib::logprintfunc);
}

void ScriptRuntime::command(const String& cmdline)
{
	// process console input
	const char* line = cmdline.c_str();

	// ignore heading whitespaces
	line = line + strspn(line, " \t\r\n");

	if (_debugger && _debugger->command(line))
	{
		return;
	}
	else if (strstr(line, "=") == line)
	{
		String modified = "print(";
		modified += (line+1);
		modified += ")";

		doString(modified.c_str());
	}
	else if (strstr(line, "?") == line)
	{
		String modified = "dump(";
		modified += (line+1);
		modified += ")";
		doString(modified.c_str());
	}
	else if (strstr(line, "$\"") == line)
	{
		String modified = "watch(";
		modified += (line+1);
		modified += ")";
		doString(modified.c_str());
	}
	else if (strcmp(line, "$\n") == 0)
	{
		doString("watch(\"$\", false); watch(false)");
	}
	else if (strstr(line, "$") == line)
	{
		String modified = "watch(\"$\", ";
		modified += (line+1);
		modified += ")";
		doString(modified.c_str());
	}
	else
	{
		doString(line, true);
	}
}

SQInteger ScriptRuntime::requireHandler(HSQUIRRELVM v)
{
	const SQChar* fn = NULL;
	sq_tostring(v, -1);
	sq_replace(v, -2);
	sq_getstring(v, -1, &fn);
	if (fn == NULL)
	{
		return sq_throwerror(v, "no filename");
	}

	// TODO: For delayed require call, we may not found the file from the package user expected.
	// Remember the unit on which current code run and utilize.
	StreamLocator* locator = NULL; 

	Ref<ScriptUnit> unit;
	return getRuntime(v)->require(fn, unit, locator);
}

void ScriptRuntime::shutdown()
{
	LOG_TIMESCOPE(0, "++ ScriptRuntime %08x: shutdown");

	if (_root == NULL) return;

	if (_debugger) _debugger->disable();

	killAllThreads(true);

	_timeWait = NULL;

	removeAllTimeout(_tickTimeoutHeap);
	removeAllTimeout(_clockTimeoutHeap);

	safeDelete(_weakListener); // Order is important: WeakListener should be deleted prior to InheritableManager

	_scriptRefManager->ReleaseAll(_root);

 	finishEventBindings();

	ScriptRuntimeLib::uninstall(_root);

	_started = false;

	NitBind::uninstall(_root);

	safeDelete(_scriptRefManager);

	_root = NULL;
	_units.clear();

	safeDelete(_debugger);

#if !defined(NIT_SHIPPING)
	g_ScriptTotalLeaked += g_ScriptTotalAllocated;

	if (g_ScriptTotalAllocated > 0)
	{
		LOG(0, "*** script leaks %d bytes, total %d bytes\n", g_ScriptTotalAllocated, g_ScriptTotalLeaked);
	}
	
	g_ScriptTotalAllocated = 0;
#endif
}

SQInteger ScriptRuntime::errorHandler(HSQUIRRELVM v)
{
	SQPRINTFUNCTION pf = sq_geterrorfunc(v);
	if (pf == NULL) return 0;

	const SQChar* err = "<unknown error>";

	if (sq_gettop(v) >= 1)
	{
		sq_tostring(v, 2);
		sq_replace(v, -2);
		sq_getstring(v, -1, &err);
	}

	pf(v, "*** %s\n", err);
	printCallStack(v, 1, false);

	return 0;
}

void ScriptRuntime::compileErrorHandler(HSQUIRRELVM v,const SQChar *sErr,const SQChar *sSource,SQInteger line,SQInteger column)
{
	SQPRINTFUNCTION pf = sq_geterrorfunc(v);
	if(pf) {
		pf(v,_SC("*** %s at %s line %d, column %d\n"), sErr, sSource, line, column);
	}
}

void ScriptRuntime::setDefaultErrorHandlers(HSQUIRRELVM v)
{
	sq_setcompilererrorhandler(v, compileErrorHandler);
	sq_newclosure(v, errorHandler, 0);
	sq_seterrorhandler(v);
}

void ScriptRuntime::printCallStack(HSQUIRRELVM v, SQInteger level, bool dumplocals)
{
	SQPRINTFUNCTION pf = sq_geterrorfunc(v);
	if (pf == NULL) return;

	SQInteger top = sq_gettop(v);

	SQStackInfos si;
	const SQChar *name=0; 
	SQInteger seq=0;

	if (dumplocals)
	{
		pf(v, "CALLSTACK: \n");
	}

	while(SQ_SUCCEEDED(sq_stackinfos(v,level,&si)))
	{
		const SQChar *fn ="unknown";
		const SQChar *src = "unknown";
		if (si.funcname) fn = si.funcname;
		if (si.source) src = si.source;
		pf(v, "  - [%02d] %s() at %s line %d\n", level, fn, src, si.line);
		level++;
	}

	if (dumplocals)
	{
		pf(v, "LOCALS: \n");
		for(level=0;level<10;level++){
			seq=0;
			while((name = sq_getlocal(v,level,seq)))
			{
				seq++;
				const SQChar* value = "???";
				sq_tostring(v, -1);
				sq_replace(v, -2);
				sq_getstring(v, -1, &value);

				pf(v, "  - [%02d] %-20s = %s\n", level, name, value);
				sq_pop(v, 2);
			}
		}
	}

	sq_settop(v, top);
}

void ScriptRuntime::stepGC()
{
	if (_stepGcPaused) return;

 	_scriptRefManager->UpdateList(_root);

	SQGCInfo info;
	sq_gcstep(_root, &info);

	if (info.sweepcount > 0)
	{
		LOG(0, ".. GC sweeps: %d\n", info.sweepcount);
	}
}

void ScriptRuntime::registerFn(const char* fnname, SQFUNCTION fn)
{
	sq_pushroottable(_root);
	sq_pushstring(_root, fnname, -1);
	sq_newclosure(_root, fn, 0);
	sq_setnativeclosureinfo(_root, -1, fnname, NULL);
	sq_createslot(_root, -3); 
	sq_pop(_root, 1);
}

bool ScriptRuntime::doString(const char* string, bool spawncotask)
{
	char* buffer = (char*)string;
	int i = 0;

	HSQUIRRELVM c = getWorker();

	i = scstrlen(buffer);
	if (i == 0) return false;

	SQInteger top = sq_gettop(c);
	SQRESULT r;

	r = sq_compilebuffer(c, buffer, i, _SC("<console>"), SQTrue);

	if (SQ_FAILED(r)) goto error;

//	sqx_dumpfunc(c, -1);

	if (spawncotask)
	{
		sq_newclosure(c, ScriptRuntime::costart, 0);
		sq_pushroottable(c);
		sq_pushstring(c, "console", -1);
		sq_push(c, -4); // compiled closure
		r = sq_call(c, 3, false, true);

		if (SQ_FAILED(r)) goto error;
	}
	else
	{
		sq_pushroottable(c);
		r = sq_call(c, 1, false, true);

		if (SQ_FAILED(r)) goto error;
	}

	sq_settop(c, top);
	return true;

error:
	sq_settop(c, top);
	return false;
}

SQRESULT ScriptRuntime::loadStream(HSQUIRRELVM v, StreamReader* reader, SQBool printerror)
{
	String id = unitSourceID(reader->getSource());
	LOG_TIMESCOPE(0, ".. compiling '%s'", id.c_str());
	return ScriptIO::loadstream(v, reader, id, printerror);
}

void ScriptRuntime::setDefaultLocator(StreamLocator* locator)
{
	_defaultLocator = locator;
}

String ScriptRuntime::unitSourceID(StreamSource* source)
{
	return source->getUrl();
}

StreamSource* ScriptRuntime::locateUnit(const String& unitName, StreamLocator* locator)
{
	// If no locator specified (NULL) : auto detect locator.

	// First, select the locator with which this loads current script.
	if (locator == NULL)
		locator = getLocator();

	// If no such, select locator specified as default.
	if (locator == NULL)
		locator = _defaultLocator;

	if (locator == NULL)
		NIT_THROW_FMT(EX_INVALID_STATE, "No locator provided for '%s'", unitName.c_str());

	locator = locator->getProxy();

	String streamName = unitName;
	if (!Wildcard::match("*.nit", streamName))
		streamName += ".nit";

	return locator->locate(streamName, false);
}

ScriptUnit* ScriptRuntime::createUnit(const String& unitName, StreamLocator* locator)
{
	Ref<StreamSource> source = locateUnit(unitName, locator);

	if (source == NULL)
		return NULL;

	return new ScriptUnit(this, unitSourceID(source), source);
}

ScriptUnit* ScriptRuntime::getLoaded(const String& id)
{
	// First, search from unit map
	UnitMap::iterator itr = _units.find(id);

	if (itr != _units.end())
		return itr->second;

	// Not found, then search from unit stack
	for (uint i = 0; i < _unitStack.size(); ++i)
	{
		ScriptUnit* unit = _unitStack[i];
		if (unit->getId() == id) return unit;
	}

	return NULL;
}

SQRESULT ScriptRuntime::require(const String& unitName, Ref<ScriptUnit>& unit, StreamLocator* locator)
{
	HSQUIRRELVM v = getWorker();

	unit = createUnit(unitName, locator);

	if (unit == NULL)
	{
		LOG(0, "*** cannot open required '%s'\n", locator ? locator->makeUrl(unitName).c_str() : unitName.c_str());
		return sq_throwerror(v, "cannot open required script");
	}

	UnitMap::iterator itr = _units.find(unit->getId());

	if (itr != _units.end())
		return SQ_OK;

	SQInteger top = sq_gettop(v);
	_unitStack.push_back(unit);

	sq_pushroottable(v);
	unit->setRequired(true);
	SQRESULT r = unit->execute(v);

	_unitStack.pop_back();
	sq_settop(v, top); // ignore return value

	if (SQ_FAILED(r))
		return r;

	// Compiled and executed - register the unit
	_units.insert(std::make_pair(unit->getId(), unit));

	return SQ_OK;
}

void ScriptRuntime::unloadUnitsFrom(const String& locatorPattern)
{
	HSQUIRRELVM v = getWorker();

	vector<Ref<ScriptUnit> >::type ulist;

	for (UnitMap::iterator itr = _units.begin(); itr != _units.end(); ++itr)
	{
		ScriptUnit* unit = itr->second;

		if (Wildcard::match(locatorPattern, unit->getSource()->getLocatorName()))
			ulist.push_back(unit);
	}

	for (uint i=0; i<ulist.size(); ++i)
	{
		ScriptUnit* unit = ulist[i];
		_units.erase(unit->getId());
		unit->unload(v);
	}
}

bool ScriptRuntime::doFile(const String& unitName, StreamLocator* locator)
{
	HSQUIRRELVM v = getWorker();

	Ref<ScriptUnit> unit = createUnit(unitName, locator);

	if (unit == NULL)
	{
		LOG(0, "*** cannot open script '%s'\n", locator->makeUrl(unitName).c_str());
		return false;
	}

	SQInteger top = sq_gettop(v);
	_unitStack.push_back(unit);

	sq_pushroottable(v);
	SQRESULT r = unit->execute(v);

	_unitStack.pop_back();
	sq_settop(v, top); // ignore return value

	return SQ_SUCCEEDED(r);
}

void ScriptRuntime::initEventBindings()
{
	HSQUIRRELVM v = _root;

	SQInteger clear = sq_gettop(v);

	sq_pushroottable(v);

	NitBind::push(v, "Events");
	sq_newtable(v);

	sq_resetobject(&_eventsTable);
	sq_getstackobj(v, -1, &_eventsTable);
	sq_addref(v, &_eventsTable);

	const EventInfo** infos = EventInfo::getArray();

	for (uint i=0; i<EventInfo::getCount(); ++i)
	{
		NitBind::newSlot(v, -1, infos[i]->getEventName(), const_cast<EventInfo*>(infos[i]));

		sq_pushuserpointer(v, (void*) infos[i]);
		NitBind::push(v, infos[i]->getEventName());
		sq_newslot(v, -3, false);
	}

	sq_newslot(v, -3, false);

	sq_settop(v, clear);
}

void ScriptRuntime::finishEventBindings()
{
	sq_release(_root, &_eventsTable);

	for (uint i=0; i<_scriptEventInfos.size(); ++i)
	{
		delete _scriptEventInfos[i];
	}
	_scriptEventInfos.clear();
}

void ScriptRuntime::updateEventBindings()
{
	finishEventBindings();
	initEventBindings();
}

void ScriptRuntime::registerScriptEventInfo(EventInfo* info)
{
	_scriptEventInfos.push_back(info);
}

SQInteger ScriptRuntime::costart(HSQUIRRELVM v)
{
	SQInteger vtop = sq_gettop(v);
	SQInteger nargs = vtop - 2;

	if (vtop < 2) { sqx_throw(v, "fn, ... needed"); }

	SQRESULT r;

	HSQUIRRELVM mt = getRuntime(v)->_root;

	SQInteger mtop = sq_gettop(mt);

	String name;

	if (sq_gettype(v, 2) == OT_STRING)
	{
		name = NitBind::getString(v, 2);
		sq_remove(v, 2);
		--nargs;
	}

	// bind this.weakref() if no env is bound to fn
	sq_getenv(v, 2);								// [this] [fn] [a1] [a2] ... | [env]
	if (sq_gettype(v, -1) == OT_NULL)
	{
		sq_weakref(v, 1, true);						// [this] [fn] [a1] [a2] ... | [env] [this.weak]
		sq_bindenv(v, 2);							// [this] [fn] [a1] [a2] ... | [env] [bfn]
		sq_replace(v, 2);							// [this] [bfn] [a1] [a2] ... | [env]
	}
	sq_poptop(v);									// [this] [bfn] [a1] [a2] ... | 

	// prepare 'newthread' call in main thread
	sq_pushroottable(mt);
	sq_pushstring(mt, "_newthread", -1);			// <mainthread> ... | [root] "_newthread"
	sq_get(mt, -2);									// <mainthread> ... | [root] [newthread]
	sq_replace(mt, -2);								// <mainthread> ... | [newthread]

	// call 'newthread'
	sq_move(mt, v, 1);								// <mainthread> ... | [newthread] [this]
	sq_move(mt, v, 2);								// <mainthread> ... | [newthread] [this] [bfn]
	r = sq_call(mt, 2, true, true);					// <mainthread> ... | [newthread] [thread]
	if (SQ_FAILED(r)) goto error;
	sq_replace(mt, -2);								// <mainthread> ... | [thread]

	// set name of the thread if provided
	if (!name.empty())
	{
		HSQUIRRELVM th;
		sq_getthread(mt, -1, &th);
		sq_setthreadname(th, name.c_str(), name.length());
	}

	// move new thread to caller thread
	if (mt != v)
	{
		sq_move(v, mt, -1);							// [this] [bfn] [a1] [a2] ... | [thread]
		sq_poptop(mt);								// <mainthread> ... | 
	}

	// prepare 'thread.call()'
	sq_newclosure(v, thread_Call, 0);				// [this] [bfn] [a1] [a2] ... | [thread] [call]
	sq_push(v, -2);									// [this] [bfn] [a1] [a2] ... | [thread] [call] [thread]

	// call 'thread.call()'
	for (int i=0; i<nargs; ++i)
		sq_push(v, 3 + i);							// [this] [bfn] [a1] [a2] ... | [thread] [call] [thread] [a1] [a2] ...
	r = sq_call(v, nargs + 1, false, false);		// [this] [bfn] [a1] [a2] ... | [thread] [call]

	if (SQ_FAILED(r))
	{
		// we can just return SQ_ERROR here.. 
		// but then errorhandler will be called twice which is somewhat annoying
		sq_settop(v, vtop);
		return 0;
	}

	sq_poptop(v);									// [this] [fn] [a1] [a2] ... | [thread]

	return 1;

error:
	sq_settop(v, vtop);
	sq_settop(mt, mtop);
	return SQ_ERROR;
}

SQInteger ScriptRuntime::cotimeout(HSQUIRRELVM v)
{
	ScriptRuntime* runtime = getRuntime(v);

	float timeout = NitBind::getFloat(v, 2);
	bool clocktime = NitBind::optBool(v, 3, false);

	float currtime = clocktime ? runtime->_clockTime : runtime->_tickTime;
	TimeoutHeap& heap = clocktime ? runtime->_clockTimeoutHeap : runtime->_tickTimeoutHeap;

	return getRuntime(v)->setTimeout(heap, v, timeout, currtime);
}

SQInteger ScriptRuntime::setTimeout(TimeoutHeap& heap, HSQUIRRELVM th, float timeout, float currtime)
{
	HSQUIRRELVM m = _root;

	SQInteger top = sq_gettop(m);

	sq_pushthread(m, th);

	// thread.local.TimeoutID = e.TimeoutID
	sq_pushthreadlocal(m, th);

	// add timeout
	TimeoutEntry e;

	e.startTime = currtime;
	e.wakeTime = currtime + timeout;
	e.timeoutId = _nextTimeoutID++;

	sq_pushuserpointer(m, &heap);
	sq_pushinteger(m, e.timeoutId);
	sq_newslot(m, -3, false);

	// remember thread weakref
	sq_weakref(m, -2, false);
	sq_resetobject(&e.weakThread);
	sq_getstackobj(m, -1, &e.weakThread);
	sq_addref(m, &e.weakThread);

	heap.push_back(e);
	std::push_heap(heap.begin(), heap.end());

	sq_settop(m, top);

	return SQ_OK;
}

SQInteger ScriptRuntime::clearThreadTimeout(HSQUIRRELVM th)
{
	HSQUIRRELVM m = _root;

	SQInteger top = sq_gettop(m);

	sq_pushthreadlocal(m, th);
	sq_pushuserpointer(m, &_tickTimeoutHeap);
	sq_pushuserpointer(m, &_clockTimeoutHeap);
	sq_deleteslot(m, -2, false);

	sq_settop(m, top);

	return SQ_OK;
}

void ScriptRuntime::onGcLoop(const Event* evt)
{
	if (!_started) return;
	if (_paused) return;

	updateThreadList();
	stepGC();
}

void ScriptRuntime::onClock(const TimeEvent* evt)
{
	if (!_started) return;
	if (_paused) return;

	_clockTime = evt->getTime();
	_timeWait->signal(0x01);
	updateTimeout(_clockTimeoutHeap, evt->getTime());
}

void ScriptRuntime::onTick(const TimeEvent* evt)
{
	if (!_started) return;
	if (_paused) return;

	_tickTime = evt->getTime();
	_timeWait->signal(0x02);
	updateTimeout(_tickTimeoutHeap, evt->getTime());
}

void ScriptRuntime::updateTimeout(TimeoutHeap& heap, float time)
{
	if (heap.empty()) return;

	HSQUIRRELVM m = _root;

	SQInteger top = sq_gettop(m);

	while (!heap.empty())
	{
		float waketime = heap[0].wakeTime;

		if (time < waketime) break;

		std::pop_heap(heap.begin(), heap.end());
		TimeoutEntry e = heap.back();
		heap.pop_back();

		sq_settop(m, top);

		sq_pushobject(m, e.weakThread);
		sq_getweakrefval(m, -1);
		sq_release(m, &e.weakThread);

		// check if thread vaporized
		if (sq_gettype(m, -1) != OT_THREAD) continue; 

		HSQUIRRELVM th;
		sq_getthread(m, -1, &th);

		// check if thread already dead or running
		if (sq_getvmstate(th) != SQ_VMSTATE_SUSPENDED) continue;

		// check if others wake up already
		sq_pushthreadlocal(m, th);
		sq_pushuserpointer(m, &heap);

		if (SQ_FAILED(sq_deleteslot(m, -2, true))) continue;

		// check if another timeout is set
		int timeoutID = -1;
		if (SQ_FAILED(sq_getinteger(m, -1, &timeoutID))) continue;

		if (timeoutID != e.timeoutId) continue;

		ScriptWaitBlock::clearWaitBlockFlag(th);

		sq_settop(m, top);

		sq_pushinteger(th, 0); // signal 0 : timeout
		sq_wakeupvm(th, SQTrue, SQFalse, SQTrue, SQFalse);

		if (sq_getvmstate(th) == SQ_VMSTATE_IDLE)
			sq_closethread(th);
	}

	sq_settop(m, top);
}

void ScriptRuntime::removeAllTimeout(TimeoutHeap& heap)
{
	HSQUIRRELVM m = _root;

	for (uint i=0; i<heap.size(); ++i)
	{
		TimeoutEntry& e = heap[i];
		sq_release(m, &e.weakThread);
	}

	heap.clear();
}

SQInteger ScriptRuntime::sleep(HSQUIRRELVM v)
{
	ScriptRuntime* runtime = getRuntime(v);

	if (v == runtime->_root)
		return sq_throwerror(v, "cannot sleep main thread");

	float timeout = NitBind::optFloat(v, 2, 0.0f);
	bool clocktime = NitBind::optBool(v, 3, false);

	float currtime = clocktime ? runtime->_clockTime : runtime->_tickTime;
	TimeoutHeap& heap = clocktime ? runtime->_clockTimeoutHeap : runtime->_tickTimeoutHeap;

	if (timeout > 0.0f)
	{
		runtime->setTimeout(heap, v, timeout, currtime);
		return sq_suspendvm(v);
	}
	else
	{
		uint signal = clocktime ? 0x01 : 0x02;
		return runtime->_timeWait->wait(v, signal);
	}
}

SQInteger ScriptRuntime::thread_cotimeout(HSQUIRRELVM v)
{
	HSQUIRRELVM th = NULL;
	sq_getthread(v, 1, &th);

	if (th == NULL) return sq_throwerror(v, "object is not a thread");

	ScriptRuntime* runtime = getRuntime(v);

	float timeout = NitBind::getFloat(v, 2);
	bool clocktime = NitBind::optBool(v, 3, false);

	float currtime = clocktime ? runtime->_clockTime : runtime->_tickTime;
	TimeoutHeap& heap = clocktime ? runtime->_clockTimeoutHeap : runtime->_tickTimeoutHeap;

	return getRuntime(v)->setTimeout(heap, th, timeout, currtime);
}

SQInteger ScriptRuntime::thread_Call(HSQUIRRELVM v)
{
	HSQUIRRELVM th = NULL;
	sq_getthread(v, 1, &th);
	
	if (th == NULL) return sq_throwerror(v, "object is not a thread");

	SQInteger nparams = sq_gettop(v);

	// default 'newthread' allocates stack size too small. this might be a problem in metamethod calls
	const int MIN_RESERVE_STACK_SIZE = 128; // TODO: This depends on a luck not on a concrete logic: fix metamethod.
	sq_reservestack(th, MIN_RESERVE_STACK_SIZE);

	sq_pushroottable(th);
	
	for (SQInteger i = 2; i < nparams+1; ++i)
		sq_move(th, v, i);

//	sq_setoplimit(th, 64 * 1024);
	if (SQ_FAILED(sq_call(th, nparams, true, true)))
	{
		// error : v->_lasterror = th->_lasterror;
		sq_getlasterror(th);
		sq_move(v, th, -1);
		sq_setlasterror(v);

		sq_settop(th, 1); // clear stack of the thread

		return SQ_ERROR;
	}

	if (sq_getvmstate(th) == SQ_VMSTATE_IDLE)
	{
		sq_move(v, th, -1); // return value
		sq_closethread(th);
	}
	else
	{
		// add thread to thread list
		HSQOBJECT o;
		sq_resetobject(&o);
		sq_getstackobj(v, 1, &o);
		sq_addref(v, &o);
		getRuntime(v)->_threadList.push_back(o);
		sq_move(v, th, -1); // return value
		sq_poptop(th);
	}

	return 1;
}

SQInteger ScriptRuntime::thread_Wakeup(HSQUIRRELVM v)
{
	HSQUIRRELVM th = NULL;
	sq_getthread(v, 1, &th);

 	if (th == NULL) return sq_throwerror(v, "object is not a thread");

	switch (sq_getvmstate(th))
	{
	case SQ_VMSTATE_SUSPENDED:	break; // OK
	case SQ_VMSTATE_IDLE:		return sq_throwerror(v, "cannot wakeup an idle thread");
	case SQ_VMSTATE_RUNNING:	return sq_throwerror(v, "cannot wakeup a running thread");
	default:					return sq_throwerror(v, "cannot wakeup an invalid thread");
	}

	ScriptRuntime* runtime = getRuntime(v);
	runtime->clearThreadTimeout(th);

	ScriptWaitBlock::clearWaitBlockFlag(th);

	bool wakeupRet = sq_gettop(v) > 1;
	if (wakeupRet)
		sq_move(th, v, 2);

	if (SQ_FAILED(sq_wakeupvm(th, wakeupRet, true, true, false)))
	{
		// error : v->_lasterror = th->_lasterror;
		sq_getlasterror(th);
		sq_move(v, th, -1);
		sq_setlasterror(v);

		sq_settop(th, 1); // clear stack of the thread

		return SQ_ERROR;
	}

	// success - move return value
	sq_move(v, th, -1);
	sq_pop(th, 1); // pop retval

	if (sq_getvmstate(th) == SQ_VMSTATE_IDLE)
		sq_closethread(th);

	return 1;
}

SQInteger ScriptRuntime::thread_Kill(HSQUIRRELVM v)
{
	HSQUIRRELVM th;
	SQRESULT r; 
	r = sq_getthread(v, 1, &th);
	if (SQ_FAILED(r)) return sq_throwerror(v, "object is not a thread");

	if (sq_getvmstate(th) == SQ_VMSTATE_RUNNING)
	{
		if (th == v) 
			return sq_throwerror(v, "thread killed");
		else
			return sq_throwerror(v, "cannot kill a running thread");
	}

	const char* err = "thread killed";
	sq_getstring(v, 2, &err);

	ScriptRuntime* runtime = getRuntime(v);
	runtime->clearThreadTimeout(th);

	ScriptWaitBlock::clearWaitBlockFlag(th);

	sq_throwerror(th, err);
	sq_wakeupvm(th, false, false, false, true);

	if (sq_getvmstate(th) == SQ_VMSTATE_IDLE)
		sq_closethread(th);

	return 0;
}

SQInteger ScriptRuntime::thread_Dump(HSQUIRRELVM v)
{
	HSQUIRRELVM t;
	SQRESULT r; 
	r = sq_getthread(v, 1, &t);
	if (SQ_FAILED(r)) sqx_throw(v, "object is not a thread");

	SQPRINTFUNCTION pf = sq_geterrorfunc(v);
	if (pf == NULL) { return 0; }

	switch (sq_getvmstate(t))
	{
	case SQ_VMSTATE_IDLE:		pf(v, "  - status: idle\n"); break;
	case SQ_VMSTATE_RUNNING:	pf(v, "  - status: running\n"); break;
	case SQ_VMSTATE_SUSPENDED:	pf(v, "  - status: suspended\n"); break;
	default:					pf(v, "  - status: unknown\n"); break;
	}

	pf(v, "  - stacksize: %d\n", sq_gettop(t));

	printCallStack(t, v == t ? 1 : 0, false);

	sq_pushbool(v, true);
	return 1;
}

SQInteger ScriptRuntime::thread_tostring(HSQUIRRELVM v)
{
	// TODO: Currently not supported by VM

	HSQUIRRELVM th = NULL;
	sq_getthread(v, 1, &th);

	if (th == NULL) return sq_throwerror(v, "object is not a thread");

	const char* name = "cotask";

	SQInteger top = sq_gettop(v);

	sq_pushthreadlocal(v, th);
	sq_pushstring(v, "Name", -1);
	if (SQ_SUCCEEDED(sq_get(v, -2)))
		sq_getstring(v, -1, &name);

	char* buf = sq_getscratchpad(v, 8 + strlen(name));
	int len = sprintf(buf, "(%s: %08x)", name, (int)th);

	sq_settop(v, top);
	sq_pushstring(v, buf, len);

	return 1;
}

void ScriptRuntime::killAllThreads(bool cleanup)
{
	HSQUIRRELVM v = _root;

	SQInteger top = sq_gettop(v);

	SQObjList::iterator itr = _threadList.begin();
	while (itr != _threadList.end())
	{
		sq_settop(v, top);
		sq_pushobject(v, *itr);
		HSQUIRRELVM th;
		sq_getthread(v, -1, &th);

		sq_throwerror(th, "session closing");
		sq_wakeupvm(th, false, false, false, true);

		if (sq_getvmstate(th) != SQ_VMSTATE_IDLE)
		{
			// TODO: use _tostring()
			LOG(0, "*** (%s %08x) resists to be killed\n", "thread", th);
			printCallStack(th, 0, true);
		}

		if (sq_getvmstate(th) == SQ_VMSTATE_IDLE || cleanup)
		{
			sq_closethread(th);
			sq_release(v, &*itr);
			_threadList.erase(itr++);
		}
		else
		{
			++itr;
		}
	}

	sq_settop(v, top);
}

void ScriptRuntime::updateThreadList()
{
	if (_threadList.empty()) return;

	HSQUIRRELVM v = _root;

	int count = _threadList.size() / 10; // TODO: inspect platform's stl implementation
	if (count == 0) count = 1;

	for (int i=0; i<count; ++i)
	{
		if (_threadList.empty()) return;

		HSQOBJECT o = *_threadList.begin();
		HSQUIRRELVM th = NULL;
		sq_pushobject(v, o);
		sq_getthread(v, -1, &th);
		sq_poptop(v);

		bool alive = (th != NULL) && sq_getvmstate(th) != SQ_VMSTATE_IDLE;

		if (!alive)
			sq_release(v, &o);
		else
			_threadList.push_back(o);

		_threadList.pop_front();
	}
}

SQInteger ScriptRuntime::getThreads(HSQUIRRELVM v)
{
	ScriptRuntime* runtime = getRuntime(v);

	runtime->updateThreadList();

	sq_newarray(v, 0);
	for (SQObjList::iterator itr = runtime->_threadList.begin(), end = runtime->_threadList.end(); itr != end; ++itr)
	{
		sq_pushobject(v, *itr);
		sq_arrayappend(v, -2);
	}

	return 1;
}

void ScriptRuntime::weakAdd(WeakSupported* object)
{
	ScriptWeakListener* myListener = (ScriptWeakListener*)_weakListener;

	if (myListener)
		myListener->Add(object);
}

void ScriptRuntime::weakRelease(WeakSupported* object)
{
	ScriptWeakListener* myListener = (ScriptWeakListener*)_weakListener;

	if (myListener)
		myListener->Release(object);
}

SQInteger ScriptRuntime::pauseGc(HSQUIRRELVM v)
{
	getRuntime(v)->_stepGcPaused = NitBind::getBool(v, 2);
	return 0;
}

void ScriptRuntime::retain(IScriptRef* o)
{
	_scriptRefManager->Retain(_root, o);
}

size_t ScriptRuntime::getMemoryAllocated_DebugPurpose()
{
#if !defined(NIT_SHIPPING)
	return g_ScriptTotalAllocated;
#else
	return 0;
#endif
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
