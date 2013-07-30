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

#include "squirrel/squirrel.h"
#include "squirrel/sqxapi.h"

#include "nit/script/ScriptRuntime.h"
#include "nit/net/RemoteDebugger.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NIT_API ScriptDebugger : public IDebugger
{
public:
	enum DebugState
	{
		DBG_RUNNING,
		DBG_STEP_OVER,
		DBG_STEP_INTO,
		DBG_STEP_OUT,
		DBG_SUSPENDED,
		DBG_DISABLED,
	};

	struct Breakpoint
	{
		Breakpoint(int id, int line = 0, const SQChar* src = NULL) : id(id), line(line), source(src), enabled(true) { }

		inline bool operator < (const Breakpoint& other) const
		{
			if (line < other.line) return true;
			if (line > other.line) return false;
			return source < other.source;
		}

		int								id;
		int								line;
		String							source;
		bool							enabled;
		String							condition;
	};

	typedef set<Breakpoint>::type		BreakpointSet;

public:
	ScriptDebugger();
	virtual ~ScriptDebugger();

public:
	void								reset();
	void								disable()								{ _state = DBG_DISABLED; }

	bool								isActive()								{ return _active; }
	void								setActive(bool flag);

	void								attach(HSQUIRRELVM v);
	void								detach(HSQUIRRELVM v);

	DebugState							getState()								{ return _state; }
	BreakpointSet&						getBreakpoints()						{ return _breakpoints; }

	bool								command(String line);

	static void							dumpStack(HSQUIRRELVM v)				{ sqx_dump(v); }

public:									// IDebugger Impl
	virtual bool						Break();
	virtual bool						go();
	virtual bool						stepInto();
	virtual bool						stepOver();
	virtual bool						stepOut();

	virtual bool						inspect(int inspectId, DataValue& outValue);

	virtual void						updateBreakpoints();

protected:
	static void							native_debug_hook(HSQUIRRELVM v, SQInteger type, const SQChar* src, SQInteger line, const SQChar* func, SQUserPointer up);
	SQInteger							error_handler(HSQUIRRELVM v);

	void								hook(HSQUIRRELVM v, int type, int line, const SQChar* src, const SQChar* func);
	bool								checkBreakpoint(HSQUIRRELVM v, const Breakpoint& bp);
	void								breakExecution(HSQUIRRELVM v, Ref<DataRecord> breakInfo, int line);

protected:
	virtual void						onBreak(HSQUIRRELVM v, int level, int line, const SQChar* src, const SQChar* func, const SQChar* type, const SQChar* cond, const SQChar* error);

private:
	DebugServer*						_debugServer;

	BreakpointSet						_breakpoints;
	DebugState							_state;
	HSQUIRRELVM							_targetThread;
	int									_nestedCalls;

	bool								_active;
	bool								_errorHandling;

	int									_nextThreadID;
	int									_nextStackID;
	int									_nextInspectID;

	typedef unordered_map<int, Ref<ScriptPeer> >::type ObjectMap;
	ObjectMap							_inspectTrap;

	void								clearTrap(HSQUIRRELVM v, ObjectMap& trap);

	void								resetTraps(HSQUIRRELVM v);
	Ref<DataRecord>						createThreadInfo(HSQUIRRELVM th, ScriptRuntime* srt, int startLevel, int lineFix = -1);
	Ref<DataRecord>						createObjInfo(HSQUIRRELVM v, int stackIdx, const char* kind);
	void								populateMemberInfo(HSQUIRRELVM v, int stackIdx, Ref<DataRecord> members);
	friend class						NB_ScriptDebugger;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
