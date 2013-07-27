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

#include "nit/script/ScriptDebugger.h"
#include "nit/script/ScriptRuntime.h"

#include "nit/net/RemoteDebugger.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

ScriptDebugger::ScriptDebugger()
{
	_active = false;
	_errorHandling = false;
	_targetThread = NULL;

	NitRuntime* rt = NitRuntime::getSingleton();
	_debugServer = rt->getDebugServer();

	ASSERT_THROW(_debugServer, EX_INVALID_STATE);

	_debugServer->setDebugger(this);
}

ScriptDebugger::~ScriptDebugger()
{
	_debugServer->setDebugger(NULL);
}

void ScriptDebugger::reset()
{
	_state = DBG_RUNNING;
	_nestedCalls = 0;
	_errorHandling = false;
	_targetThread = NULL;
}

void ScriptDebugger::setActive(bool flag)
{
	if (_active == flag) return;

	_active = flag;

	if (flag)
	{
		LOG(0, "++ [SQDBG] Debugger Active\n");
	}
	else
	{
		LOG(0, "++ [SQDBG] Debugger Inactive\n");
	}
}

void ScriptDebugger::attach(HSQUIRRELVM v)
{
	SQInteger top = sq_gettop(v);

	sq_enabledebuginfo(v, true);

	sq_setnativedebughook(v, this, native_debug_hook);

	sq_settop(v, top);
}

void ScriptDebugger::detach(HSQUIRRELVM v)
{
	sq_setnativedebughook(v, this, NULL);
	ScriptRuntime::setDefaultErrorHandlers(v);
}

void ScriptDebugger::native_debug_hook(HSQUIRRELVM v, SQInteger type, const SQChar* src, SQInteger line, const SQChar* func, SQUserPointer up)
{
	ScriptDebugger* dbg = (ScriptDebugger*)up;

	dbg->hook(v, type, line, src, func);
}

SQInteger ScriptDebugger::error_handler(HSQUIRRELVM v)
{
	if (!_active || _errorHandling || _state == DBG_DISABLED || _state == DBG_SUSPENDED)
	{
		_errorHandling = false;
		return 0;
	}

	_errorHandling = true;
	_state = DBG_DISABLED;

	SQStackInfos si;
	const SQChar* func = "<unknown>";
	const SQChar* src = "<unknown>";
	String err = "<unknown error>";
	int line = -1;

	if (SQ_SUCCEEDED(sq_stackinfos(v, 0, &si)))
	{
		if (si.funcname) func = si.funcname;
		if (si.source)  src = si.source;
		line = si.line;
	}

	if (sq_gettop(v) >= 1)
	{
		sq_getlasterror(v);
		sq_tostring(v, -1);
		sq_replace(v, -2);
		const char* str;
		sq_getstring(v, -1, &str);
		err = str;
		sq_poptop(v);
	}

	onBreak(v, 1, line, src, func, "error", NULL, err.c_str());
	
	return 0;
}

void ScriptDebugger::updateBreakpoints()
{
	_breakpoints.clear();

	const DebugServer::Breakpoints& rbps = _debugServer->getBreakpoints();

	for (DebugServer::Breakpoints::const_iterator itr = rbps.begin(), end = rbps.end(); itr != end; ++itr)
	{
		const DebugServer::Breakpoint& rbp = itr->second;

		_breakpoints.insert(Breakpoint(rbp.id, rbp.line, rbp.url.c_str()));
	}
}

void ScriptDebugger::hook(HSQUIRRELVM v, int type, int line, const SQChar* src, const SQChar* func)
{
	SQInteger top = sq_gettop(v);
	if (top > 1024)
	{
		onBreak(v, 0, line, src, func, "stackerror?", NULL, NULL);
	}

	if (!_active) return;

	if (type == 'e')
	{
		error_handler(v);
		return;
	}

	switch (_state)
	{
	case DBG_RUNNING:
		if (type == 'l' && !_breakpoints.empty())
		{
			BreakpointSet::iterator itr = _breakpoints.find(Breakpoint(0, line, src));
			if (itr != _breakpoints.end() && checkBreakpoint(v, *itr))
			{
				onBreak(v, 1, line, src, func, "breakpoint", itr->condition.c_str(), NULL);
			}
		}
		break;

	case DBG_SUSPENDED:
		_nestedCalls = 0;
		break;

	case DBG_STEP_OVER:
		switch (type)
		{
		case 'l' : 
			if (_nestedCalls == 0) 
			{ 
				onBreak(v, 1, line, src, func, "step", NULL, NULL); 
			}
			break;
		case 'c':
			++_nestedCalls;
			break;
		case 'r':
			if (_nestedCalls == 0)
				_state = DBG_STEP_INTO;
			else
				--_nestedCalls;
			break;
		case 'q':
			_state = DBG_RUNNING;
			break;
		}
		break;

	case DBG_STEP_INTO:
		if (type == 'l')
		{
			_nestedCalls = 0;
			onBreak(v, 1, line, src, func, "step", NULL, NULL);
		}
		else if (type == 'q')
		{
			_state = DBG_RUNNING;
		}
		break;

	case DBG_STEP_OUT:
		switch (type)
		{
		case 'l': break;
		case 'c': ++_nestedCalls; break;
		case 'r':
			if (_nestedCalls == 0)
				_state = DBG_STEP_INTO;
			else
				--_nestedCalls;
			break;
		case 'q':
			_state = DBG_RUNNING;
			break;
		}
		break;

	case DBG_DISABLED:
		break;
	}
}

bool ScriptDebugger::checkBreakpoint(HSQUIRRELVM v, const Breakpoint& bp)
{
	if (!bp.enabled) return false;

	if (bp.condition.empty()) return true;

	SQBool ret = 0;
	SQRESULT r;

	// evaluate condition
	SQInteger top = sq_gettop(v);

	// make expr
	String line;
	line += "return ";
	line += bp.condition;

	r = sq_compilebuffer(v, line.c_str(), line.size(), "", false);
	if (SQ_FAILED(r)) 
	{
		sq_settop(v, top);
		return false;
	}

	sq_push(v, 1); // this
	r = sq_call(v, 1, true, false);
	if (SQ_SUCCEEDED(r))
	{
		sq_tobool(v, -1, &ret);
	}

	sq_settop(v, top);

	return ret != 0;
}

void ScriptDebugger::onBreak(HSQUIRRELVM v, int level, int line, const SQChar* src, const SQChar* func, const SQChar* type, const SQChar* cond, const SQChar* error)
{
	const SQChar* threadprefix = "thread ";
	const SQChar* threadname = "unknown";

	SQInteger top = sq_gettop(v);
	sq_pushthreadlocal(v, v);
	sq_pushstring(v, "name", -1);
	if (SQ_SUCCEEDED(sq_get(v, -2)))
	{
		sq_getstring(v, -1, &threadname);
	}
	else
	{
		sq_pushthread(v, v);
		sq_tostring(v, -1);
		sq_getstring(v, -1, &threadname);
		threadprefix = "";
	}

	String indicator;

	static char buf[256];
	if (!error)
	{
		sprintf(buf, "++ [SQDBG] %s: %s%s\n", type, threadprefix, threadname);
		indicator += buf;
		sprintf(buf, "  - >>>> %s() at %s line %d\n", func, src, line);
		indicator += buf;
	}
	else
	{
		sprintf(buf, "*** [SQDBG] error '%s': %s%s\n", error, threadprefix, threadname);
		indicator += buf;
		sprintf(buf, "  - >>>> %s() at %s line %d\n", func, src, line);
		indicator += buf;
	}

	sq_settop(v, top);

	SQStackInfos si;

	while(SQ_SUCCEEDED(sq_stackinfos(v,level,&si)))
	{
		const SQChar *fn=_SC("unknown");
		const SQChar *src=_SC("unknown");
		if(si.funcname)fn=si.funcname;
		if(si.source)src=si.source;
		sprintf(buf, _SC("  - [%02d] %s() at %s line %d\n"), level, fn, src, si.line);
		indicator += buf;
		level++;
	}

	LOG(0, indicator.c_str());

	Ref<DataRecord> breakInfo = new DataRecord();
	breakInfo->set("reason", type);
	breakInfo->set("error", error);
	breakInfo->set("cond", cond);

	{
		ScriptRuntime* srt = ScriptRuntime::getRuntime(v);

		// TODO: temporary
		breakInfo->set("func", func);
		breakInfo->set("src", src);
		breakInfo->set("line", line);
		if (src)
		{
			Ref<ScriptUnit> unit = srt->getLoaded(src);
			if (unit)
			{
				Ref<DataNamespace> ns = DataNamespace::getGlobal();
				breakInfo->set("pack", ns->add(unit->getLocator()->getName()));
				breakInfo->set("file", ns->add(unit->getSource()->getName()));
				breakInfo->set("url", ns->add(unit->getSource()->getUrl()));
			}
		}
	}

	breakExecution(v, breakInfo, line);
}

static const char* TypeToStr(int sqtype)
{
	switch (sqtype)
	{
	case OT_NULL:						return "null";
	case OT_INTEGER:					return "integer";
	case OT_FLOAT:						return "float";
	case OT_BOOL:						return "bool";
	case OT_STRING:						return "string";
	case OT_TABLE:						return "table";
	case OT_ARRAY:						return "array";
	case OT_USERDATA:					return "userdata";
	case OT_CLOSURE:					return "closure";
	case OT_NATIVECLOSURE:				return "n-closure";
	case OT_GENERATOR:					return "generator";
	case OT_USERPOINTER:				return "userpointer";
	case OT_THREAD:						return "thread";
	case OT_FUNCPROTO:					return "funcproto";
	case OT_CLASS:						return "class";
	case OT_INSTANCE:					return "instance";
	case OT_WEAKREF:					return "weakref";
	case OT_NATIVEWEAKREF:				return "n-weakref";
	case OT_OUTER:						return "outer";
	case OT_NATIVE_ITR:					return "n-itr";
	default:							return "unknown";
	}
}

static String toString(HSQUIRRELVM v, int stackIdx, const char* name, const char* func)
{
	int type = sq_gettype(v, stackIdx);

	const char* str = "???";

	if (type == OT_STRING)
	{
		sq_getstring(v, stackIdx, &str);
		return str;
	}

	// HACK: It may be dangerous to call tostring() or to inspect unless a native-instance has instance-up assigned.
	// This is common for trapping on initializer or constructor call
	bool hazardCase = strcmp(name, "this") == 0 &&
		(strcmp(func, "_initializer") == 0 || strcmp(func, "constructor") == 0);

	// TODO: Investigate side-effect for failure of metamethod call to '_tostring()' at sqvm.ToString()

	if (!hazardCase)
	{
		sq_tostring(v, stackIdx);
		sq_getstring(v, -1, &str);
		String value = str;
		sq_poptop(v);
		return value;
	}

	str = "[constructing]";

	if (type != OT_INSTANCE)
		return str;

	SQUserPointer tag = NULL;

	// Check if is a native instance descendant
	sq_getclass(v, stackIdx);
	while (tag == NULL)
	{
		if (SQ_FAILED(sq_getbase(v, -1)))
		{
			sq_poptop(v);
			break;
		}
		sq_gettypetag(v, -1, &tag);
		sq_replace(v, -2);
	}
	sq_poptop(v);

	SQUserPointer up = NULL;
	sq_getinstanceup(v, stackIdx, &up, NULL);

	// Perform tostring() when pure-instance (tag==NULL) or instance-up assigned.
	// (Native instances are mostly ok when instance-up assigned.)
	if (tag == NULL || up)
	{
		sq_tostring(v, stackIdx);
		sq_getstring(v, -1, &str);
		String value = str;
		sq_poptop(v);
		return value;
	}
	return str;
}

static String getName(HSQUIRRELVM th, int stackIdx)
{
	String name;

	int type = sq_gettype(th, stackIdx);

	if (type == OT_CLASS)
	{
		const char* str = "";
		sq_pushstring(th, "_classname", stackIdx);
		sq_rawget(th, -2);
		sq_getstring(th, -1, &str);
		name = str;
		sq_poptop(th); // _classname
	}
	else if (type == OT_INSTANCE)
	{
		if (SQ_SUCCEEDED(sq_getclass(th, -1)))
		{
			const char* str = "";
			sq_pushstring(th, "_classname", stackIdx);
			sq_rawget(th, -2);
			sq_getstring(th, -1, &str);
			name = str;
			sq_poptop(th); // _classname
		}
		else name = "<purged>";
	}
	else
	{
		// TODO: if (roottable) name = "root"

		name = TypeToStr(type);
	}

	return name;
}

Ref<DataRecord> ScriptDebugger::createThreadInfo(HSQUIRRELVM th, ScriptRuntime* srt, int startLevel, int lineFix)
{
	Ref<DataRecord> threadInfo = new DataRecord();

	int th_state = sq_getvmstate(th);
	const char* th_name = "thread";
	sq_getthreadname(th, &th_name);

	threadInfo->set("thread_id", _nextThreadID++);
	threadInfo->set("state", th_state);
	threadInfo->set("name", th_name);

	Ref<DataArray> callstack = new DataArray();
	threadInfo->set("callstack", callstack);

	SQStackInfos si;

	int level = startLevel;

	while (true)
	{
		if (SQ_FAILED(sq_stackinfos(th, level, &si)))
			break;

		const char* func = si.funcname ? si.funcname : "";

		Ref<DataRecord> stackInfo = new DataRecord();
		callstack->append(stackInfo);

		stackInfo->set("stack_id", _nextStackID++);
		stackInfo->set("func", func);
		stackInfo->set("src", si.source);
		if (level == startLevel && lineFix != -1)
			stackInfo->set("line", lineFix);
		else
			stackInfo->set("line", si.line);

		Ref<ScriptUnit> unit = si.source ? srt->getLoaded(si.source) : NULL;

		if (unit)
		{
			Ref<DataNamespace> ns = DataNamespace::getGlobal();
			stackInfo->set("pack", ns->add(unit->getLocator()->getName()));
			stackInfo->set("file", ns->add(unit->getSource()->getName()));
			stackInfo->set("url", ns->add(unit->getSource()->getUrl()));
		}

		Ref<DataRecord> locals = new DataRecord();
		stackInfo->set("locals", locals);

		const char* varName = NULL;
		int seq = 0;
		while ((varName = sq_getlocal(th, level, seq)))
		{
			int type = sq_gettype(th, -1);

			locals->set(varName, createObjInfo(th, -1, "local", varName, func));

			if (strcmp(varName, "this") == 0)
			{
				String thisName = getName(th, -1);
				stackInfo->set("this_name", thisName);
			}

			sq_poptop(th);
			++seq;
		}
		++level;
	}

	return threadInfo;
}

// TODO: remove varName & func
Ref<DataRecord> ScriptDebugger::createObjInfo(HSQUIRRELVM v, int stackIdx, const char* kind, const char* varName, const char* func)
{
	int objIdx = NitBind::toAbsIdx(v, stackIdx);

	// Obtain the type
	int type = sq_gettype(v, objIdx);

	// If we can inspect, assign an id and do trap
	bool inspectable = false;

	switch (type)
	{
	case OT_TABLE:
	case OT_ARRAY:
	case OT_USERDATA:
	case OT_THREAD:
	case OT_CLASS:
	case OT_INSTANCE:
	case OT_WEAKREF:
	case OT_NATIVEWEAKREF:
		inspectable = true; 
		break;

	case OT_CLOSURE:
	case OT_NATIVECLOSURE:
		inspectable = true;
		// TODO: for instance: if the closure is same with one of instance's class (class inherited methods)
		// do not show. (class will show them)
		// otherwise show them
		return NULL;
	}

	Ref<DataRecord> object = new DataRecord();

	if (inspectable)
	{
		int inspectID = _nextInspectID++;
		Ref<ScriptPeer> obj = new ScriptPeer(v, objIdx);
		_inspectTrap.insert(std::make_pair(inspectID, obj));
		object->set("inspect_id", inspectID);
	}

	// Represent the value by tostring()

	String value = toString(v, stackIdx, varName, func);

	if (value.size() > 100)
	{
		object->set("truncated", (int)value.size());
		value = value.substr(0, 100);
	}

	object->set("type", TypeToStr(type));
	object->set("kind", kind);
	object->set("value", value);

	return object;
}

bool ScriptDebugger::inspect(int inspectId, DataValue& outValue)
{
	ObjectMap::iterator itr = _inspectTrap.find(inspectId);
	if (itr == _inspectTrap.end())
		return false;

	Ref<ScriptPeer> obj = itr->second;
	HSQUIRRELVM v = obj->getWorker();
	obj->pushObject(v);

	Ref<DataRecord> members = new DataRecord();
	populateMemberInfo(v, -1, members);
	sq_poptop(v);

	outValue = members;
	return true;
}

void ScriptDebugger::populateMemberInfo(HSQUIRRELVM v, int stackidx, Ref<DataRecord> members)
{
	const char* varName = "";
	const char* func = "";

	int objIdx = NitBind::toAbsIdx(v, stackidx);

	int type = sq_gettype(v, objIdx);

	int clean = sq_gettop(v);

	int metaIdx = 0;

	switch (type)
	{
	case OT_TABLE:
	case OT_USERDATA:
		sq_getdelegate(v, objIdx);
		if (sq_gettype(v, -1) != OT_NULL)
			members->set("$delegate", createObjInfo(v, -1, "internal", varName, func));
		sq_poptop(v);
		metaIdx = objIdx;
		break;

	case OT_INSTANCE:
		sq_getclass(v, objIdx);
		if (sq_gettype(v, -1) != OT_NULL)
			members->set("$class", createObjInfo(v, -1, "internal", varName, func));
		metaIdx = NitBind::toAbsIdx(v, -1);
		break;

	case OT_CLASS:
		sq_getbase(v, objIdx);
		if (sq_gettype(v, -1) != OT_NULL)
			members->set("$base", createObjInfo(v, -1, "internal", varName, func));
		sq_poptop(v);
		metaIdx = objIdx;
		break;

	case OT_WEAKREF:
	case OT_NATIVEWEAKREF:
		sq_getweakrefval(v, objIdx);
		members->set("$ref", createObjInfo(v, -1, "internal", varName, func));
		sq_poptop(v);
		break;

	case OT_CLOSURE:
		break;

	case OT_NATIVECLOSURE:
		break;
	}

	if (metaIdx == 0)
	{
		sq_settop(v, clean);
		return;
	}

	Ref<DataNamespace> ns = DataNamespace::getGlobal();

	for (NitIterator itr(v, metaIdx); itr.hasNext(); itr.next())
	{
		Ref<DataKey> key;
		const char* str = NULL;
		sq_tostring(v, itr.keyIndex());
		sq_getstring(v, -1, &str);
		if (str) key = ns->add(str);
		sq_poptop(v);

		if (key == NULL) continue;

		Ref<DataRecord> objInfo;

		if (type != OT_INSTANCE)
		{
			objInfo = createObjInfo(v, itr.valueIndex(), "item", varName, func);
			if (objInfo)
			{
				members->set(key, objInfo);
			}
			continue;
		}

		sq_push(v, itr.keyIndex());
		if (SQ_SUCCEEDED(sq_get(v, objIdx)))
		{
			objInfo = createObjInfo(v, -1, "member", varName, func);
			if (objInfo)
				members->set(key, objInfo);
			sq_poptop(v);
		}
		else
		{
			objInfo = new DataRecord();
			objInfo->set("kind", "member");
			objInfo->set("type", "");
			objInfo->set("value", "<error>");

			members->set(key, objInfo);
		}
	}

	sq_settop(v, clean);
}

void ScriptDebugger::clearTrap(HSQUIRRELVM v, ObjectMap& trap)
{
	trap.clear();
}

void ScriptDebugger::resetTraps(HSQUIRRELVM v)
{
	_nextThreadID = 1;
	_nextStackID = 1;
	_nextInspectID = 1;

	clearTrap(v, _inspectTrap);
}

void ScriptDebugger::breakExecution(HSQUIRRELVM v, Ref<DataRecord> breakInfo, int line)
{
	if (_debugServer->isDebugging())
	{
		LOG(0, "*** BreakExecution again???\n");
	}

 	_state = DBG_SUSPENDED;

	int level = _errorHandling ? 0 : 0;

	resetTraps(v);

	Ref<DataArray> threads = new DataArray();
	breakInfo->set("threads", threads);

	ScriptRuntime* srt = ScriptRuntime::getRuntime(v);

	Ref<DataRecord> threadInfo;

	threadInfo = createThreadInfo(v, srt, level, line);
	threads->append(threadInfo);

	breakInfo->set("thread", threadInfo->get("thread_id"));

	srt->getThreads(v);
	for (NitIterator itr(v, -1); itr.hasNext(); itr.next())
	{
		HSQUIRRELVM th = NULL;
		sq_getthread(v, itr.valueIndex(), &th);
		if (th == NULL) continue;

		threadInfo = createThreadInfo(th, srt, 0);
		threads->append(threadInfo);
	}
	sq_poptop(v);

	bool enableTempFields = false;

	// Assign '_locals', '_this', '_thread' temporary field for debugging
	if (enableTempFields)
	{
		sq_pushroottable(v);

		SQStackInfos si;
		if (SQ_SUCCEEDED(sq_stackinfos(v, level, &si)))
		{
			sq_pushstring(v, "_locals", -1);
			sq_newtable(v);
			const char* name = NULL;
			int seq = 0;
			int thisSeq = -1;
			while ((name = sq_getlocal(v, level, seq)))
			{
				sq_pushstring(v, name, -1);
				sq_push(v, -2);
				sq_newslot(v, -4, false);
				sq_poptop(v);
				if (strcmp(name, "this") == 0)
					thisSeq = seq;
				++seq;
			}
			sq_newslot(v, -3, false);

			if (thisSeq != -1)
			{
				sq_pushstring(v, "_this", -1);
				sq_getlocal(v, level, thisSeq);
				sq_newslot(v, -3, false);
			}
		}

		sq_pushstring(v, "_thread", -1);
		sq_pushthread(v, v);
		sq_newslot(v, -3, false);

		sq_poptop(v);
	}

	_targetThread = v;

	_debugServer->breakTrap(breakInfo);

	_targetThread = NULL;

	// Remove temporary fields for debugging
	if (enableTempFields)
	{
		sq_pushroottable(v);
		sq_pushstring(v, "_locals", -1);
		sq_deleteslot(v, -2, false);
		sq_pushstring(v, "_this", -1);
		sq_deleteslot(v, -2, false);
		sq_pushstring(v, "_thread", -1);
		sq_deleteslot(v, -2, false);
		sq_poptop(v);
	}

	resetTraps(v);

	if (_state == DBG_SUSPENDED)
		_state = DBG_RUNNING;
}

bool ScriptDebugger::command(String line)
{
	if (!_active) return false;

	if (!_debugServer->isDebugging()) return false;

	StringUtil::trim(line);
	if (!line.empty() && *line.rbegin() == 10)
		line.resize(line.size()-1);

	if (line == "go")
		go();
	else if (line == "into" || line == "si")
		stepInto();
	else if (line == "step" || line == "so")
		stepOver();
	else if (line == "ret" || line == "sr")
		stepOut();
	else if (line == "?")
	{
		LOG(0, "Debug Commands:\n");
		LOG(0, "  go         : continue    (F5)\n");
		LOG(0, "  into  (si) : step into   (F11)\n");
		LOG(0, "  step  (so) : step over   (F10)\n");
		LOG(0, "  ret   (sr) : step out    (Shift+F11)\n");
		LOG(0, "  ?debug     : dump debugger\n");
		LOG(0, "  ?_locals   : dump locals\n");
		LOG(0, "  ?_this     : dump locals[\"this\"] : alternative for 'this' keyword\n");
		LOG(0, "  ?_thread   : dump current thread\n");
	}
	else
		return false;

	return true;
}

bool ScriptDebugger::Break()
{
	if (!_active) return false;

	_state = DBG_STEP_INTO;
	return true;
}

bool ScriptDebugger::go()
{
	if (!_active) return false;

	_state = DBG_RUNNING;
	_nestedCalls = 0;
	_debugServer->Continue();
	return true;
}

bool ScriptDebugger::stepInto()
{
	if (!_active) return false;

	_state = DBG_STEP_INTO;
	_debugServer->Continue();
	return true;
}

bool ScriptDebugger::stepOver()
{
	if (!_active) return false;

	_state = DBG_STEP_OVER;
	_debugServer->Continue();
	return true;
}

bool ScriptDebugger::stepOut()
{
	if (!_active) return false;

	_state = DBG_STEP_OUT;
	_debugServer->Continue();
	return true;
}

////////////////////////////////////////////////////////////////////////////////

#include "nit/script/NitBind.h"
#include "nit/script/NitBindMacro.h"

NB_TYPE_RAW_PTR(NIT_API, ScriptDebugger, NULL);

class NB_ScriptDebugger : TNitClass<ScriptDebugger>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(active),
			PROP_ENTRY_R(root),
			PROP_ENTRY_R(worker),
			PROP_ENTRY_R(threads),

			PROP_ENTRY_R(memoryAllocated),
			PROP_ENTRY_R(numThreads),
			PROP_ENTRY_R(numTickTimeouts),
			PROP_ENTRY_R(numClockTimeouts),
			PROP_ENTRY_R(timeWait),
			PROP_ENTRY_R(stepGcPaused),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(go,			"()"),
			FUNC_ENTRY_H(stepInto,		"()"),
			FUNC_ENTRY_H(stepOver,		"()"),
			FUNC_ENTRY_H(stepOut,		"()"),

			FUNC_ENTRY_H(attach,		"(thread)"),
			FUNC_ENTRY_H(detach,		"(thread)"),

			FUNC_ENTRY_H(all,			"(): bp[]"),
			FUNC_ENTRY_H(clear,			"()"),
			FUNC_ENTRY_H(bp,			"(src, line, enable=true, cond=\"\")"),

			FUNC_ENTRY_H(enable,		"(index, [index, ...])"),
			FUNC_ENTRY_H(disable,		"(index, [index, ...])"),
			FUNC_ENTRY_H(condition,		"(index, cond)"),

			FUNC_ENTRY_H(stackSize,		"(thread=null): int"),
			FUNC_ENTRY_H(stackTrace,	"(thread=null, dumplocals=true)"),
			FUNC_ENTRY_H(stackDump,		"(thread=null)"),

			FUNC_ENTRY_H(gcDump,		"(itemfilter="", aftersweep=false) // itemfilter : wildcard"),

			FUNC_ENTRY	(_dump),
			FUNC_ENTRY	(_call),
			NULL
		};

		bind(v, props, funcs);

		sq_pushroottable(v);
		newSlot(v, -1, "debug", ScriptRuntime::getRuntime(v)->getDebugger());
		sq_poptop(v);

		sq_dostring(v, "::bp := debug.bp.bind(debug)");
	}

	NB_PROP_GET(active)					{ return push(v, self(v)->isActive()); }
	NB_PROP_SET(active)					{ self(v)->setActive(getBool(v, 2)); return 0; }

	NB_PROP_GET(root)					{ sq_pushthread(v, ScriptRuntime::getRuntime(v)->getRoot()); return 1; }
	NB_PROP_GET(worker)					{ sq_pushthread(v, ScriptRuntime::getRuntime(v)->getWorker()); return 1; }
	NB_PROP_GET(threads)				{ return ScriptRuntime::getRuntime(v)->getThreads(v); }

	NB_PROP_GET(memoryAllocated)		{ return push(v, ScriptRuntime::getRuntime(v)->getMemoryAllocated_DebugPurpose()); }
	NB_PROP_GET(numThreads)				{ return push(v, ScriptRuntime::getRuntime(v)->_threadList.size()); }
	NB_PROP_GET(numTickTimeouts)		{ return push(v, ScriptRuntime::getRuntime(v)->_tickTimeoutHeap.size()); }
	NB_PROP_GET(numClockTimeouts)		{ return push(v, ScriptRuntime::getRuntime(v)->_clockTimeoutHeap.size()); }
	NB_PROP_GET(timeWait)				{ return push(v, ScriptRuntime::getRuntime(v)->_timeWait.get()); }
	NB_PROP_GET(stepGcPaused)			{ return push(v, ScriptRuntime::getRuntime(v)->_stepGcPaused); }

	NB_FUNC(go)							{ self(v)->go(); return 0; }
	NB_FUNC(stepInto)					{ self(v)->stepInto(); return 0; }
	NB_FUNC(stepOver)					{ self(v)->stepOver(); return 0; }
	NB_FUNC(stepOut)					{ self(v)->stepOut(); return 0; }

	NB_FUNC(stackSize)
	{ 
		HSQUIRRELVM th = v;
		sq_getthread(v, 2, &th);

		return push(v, sq_gettop(th));
	}

	NB_FUNC(stackTrace)						
	{ 
		HSQUIRRELVM th = v;
		sq_getthread(v, 2, &th);
		ScriptRuntime::printCallStack(th, v == th ? 1 : 0, optBool(v, 3, true)); 
		return 0; 
	}

	NB_FUNC(stackDump)
	{
		HSQUIRRELVM th = v;
		sq_getthread(v, 2, &th);

		sqx_dump(th);
		return 0;
	}

	NB_FUNC(attach)
	{
		HSQUIRRELVM th = NULL;
		sq_getthread(v, 2, &th);
		if (th == NULL) return sq_throwerror(v, "thread required");

		self(v)->attach(th);
		return 0;
	}

	NB_FUNC(detach)
	{
		HSQUIRRELVM th = NULL;
		sq_getthread(v, 2, &th);
		if (th == NULL) return sq_throwerror(v, "thread required");

		self(v)->detach(th);
		return 0;
	}

	NB_FUNC(all)
	{
		ScriptDebugger::BreakpointSet& bps = self(v)->getBreakpoints();

		sq_newarray(v, 0);
		for (ScriptDebugger::BreakpointSet::iterator itr = bps.begin(), end = bps.end(); itr != end; ++itr)
		{
			sq_newtable(v);
			newSlot(v, -1, "source", itr->source);
			newSlot(v, -1, "line", itr->line);
			newSlot(v, -1, "enabled", itr->enabled);
			newSlot(v, -1, "condition", itr->condition);
			sq_arrayappend(v, -2);
		}

		return 1;
	}

	NB_FUNC(gcDump);

	NB_FUNC(_dump)
	{
		ScriptDebugger::BreakpointSet& bps = self(v)->getBreakpoints();
		if (!bps.empty())
		{
			LOG(0, "Breakpoints:\n");

			int i=0;
			for (ScriptDebugger::BreakpointSet::iterator itr = bps.begin(), end = bps.end(); itr != end; ++itr)
			{
				LOG(0, "  #%02d %s %s line %d %s%s\n", i++, (itr->enabled ? "on " : "off"), itr->source.c_str(), itr->line, (itr->condition.empty() ? "" : "when "), itr->condition.c_str());
			}

			LOG(0, "Methods:\n");
		}

		return push(v, true);
	}

	NB_FUNC(_call)
	{
		self(v)->setActive(optBool(v, 3, true));
		return 0;
	}

	NB_FUNC(enable)
	{
		set<int>::type idx;
		for (int i=2; i<=sq_gettop(v); ++i)
			idx.insert(getInt(v, i));

		ScriptDebugger::BreakpointSet& bps = self(v)->getBreakpoints();
		int i=0;
		for (ScriptDebugger::BreakpointSet::iterator itr = bps.begin(), end = bps.end(); itr != end; ++itr)
		{
			if (idx.find(i++) != idx.end())
				const_cast<bool&>(itr->enabled) = true;
		}
		return 0;
	}

	NB_FUNC(disable)
	{
		set<int>::type idx;
		for (int i=2; i<=sq_gettop(v); ++i)
			idx.insert(getInt(v, i));

		ScriptDebugger::BreakpointSet& bps = self(v)->getBreakpoints();
		int i=0;
		for (ScriptDebugger::BreakpointSet::iterator itr = bps.begin(), end = bps.end(); itr != end; ++itr)
		{
			if (idx.find(i++) != idx.end())
				const_cast<bool&>(itr->enabled) = false;
		}
		return 0;
	}

	NB_FUNC(condition)
	{
		int idx = getInt(v, 2);
		const char* cond = getString(v, 3);

		ScriptDebugger::BreakpointSet& bps = self(v)->getBreakpoints();
		int i=0;
		for (ScriptDebugger::BreakpointSet::iterator itr = bps.begin(), end = bps.end(); itr != end; ++itr)
		{
			if (i++ == idx)
				const_cast<String&>(itr->condition) = cond;
		}
		return 0;
	}

	NB_FUNC(clear)
	{
		self(v)->_breakpoints.clear();
		return 0;
	}

	NB_FUNC(bp)
	{
		ScriptDebugger* dbg = self(v);

		if (dbg->_state == ScriptDebugger::DBG_DISABLED) 
			return sq_throwerror(v, "debugger disabled");

		const char* src		= NULL;
		int line			= -1;
		SQBool enabled		= 1;
		const char* cond	= "";
		bool inplace		= false;

		if (sq_gettop(v) == 1 || sq_gettop(v) == 2)
		{
			HSQUIRRELVM th = v;

			SQStackInfos si;
			if (SQ_SUCCEEDED(sq_stackinfos(th, 1, &si)))
			{
				if (si.source) src = si.source;
				line = si.line;
			}

			if (sq_gettop(v) == 2)
				sq_getstring(v, 2, &cond);

			inplace = true;
		}
		else
		{
			sq_getstring(v, 2, &src);
			sq_getinteger(v, 3, &line);
			sq_getbool(v, 4, &enabled);
			sq_getstring(v, 5, &cond);

			if (src == NULL || line == -1)
			{
				return sq_throwerror(v, "source and line required");
			}
		}

		bool changed = false;

		ScriptDebugger::BreakpointSet::iterator itr;
		
		itr = dbg->_breakpoints.find(ScriptDebugger::Breakpoint(0, line, src));

		if (itr == dbg->_breakpoints.end())
		{
			itr = dbg->_breakpoints.insert(ScriptDebugger::Breakpoint(0, line, src)).first;
			changed = true;
		}

		if (!inplace)
		{
			bool en = enabled != 0;
			if (itr->enabled != en)
			{
				const_cast<bool&>(itr->enabled) = en;
				changed = true;
			}
		}

		if (itr->condition != cond)
		{
			const_cast<String&>(itr->condition) = cond;
			changed = true;
		}

		if (changed)
		{
			LOG(0, "++ [SQDBG] breakpoint %s at %s line %d %s%s\n", (enabled ? "enabled" : "disabled"), src, line, (*cond ? "when " : ""), cond);

			if (ScriptRuntime::getRuntime(v)->getLoaded(src) == NULL)
			{
				LOG(0, "*** [SQDBG] '%s' not loaded yet\n", src);
			}
		}

		return 1;
	}
};

SQRESULT NitLibDebug(HSQUIRRELVM v)
{
	NB_ScriptDebugger::Register(v);

	return SQ_OK;
}

NS_NIT_END;

////////////////////////////////////////////////////////////////////////////////

// HACKING SQUIRREL SECTION:

// Following includes must be here neither above nor below.
#undef Yield
#include "squirrel/sqstate.h"
#include "squirrel/sqgc.h"
#include "squirrel/sqvm.h"
#include "squirrel/sqobject.h"
#include "squirrel/sqfuncproto.h"
#include "squirrel/sqclosure.h"
#include "squirrel/sqtable.h"
#include "squirrel/sqclass.h"
#include "squirrel/sqarray.h"

NS_NIT_BEGIN;

struct DumpGCVisitor
{
	int TotalCount;
	typedef map<String, int>::type TypeCounter;

	TypeCounter Counters;
	String ItemFilter;

	DumpGCVisitor()
	{
		TotalCount = 0;
	}

	void Report()
	{
		LOG(0, "++ DumpGCVisitor: total %d objects\n", TotalCount);

		for (TypeCounter::iterator itr = Counters.begin(), end = Counters.end(); itr != end; ++itr)
		{
			LOG(0, "++ %s: %d\n", itr->first.c_str(), itr->second);
		}
	}

	void PrintObject(SQObjectPtr& obj, const char* prefix)
	{
		SQCollectable* c = NULL;

		switch (sqi_type(obj))
		{
		case OT_TABLE:			c = sqi_table(obj); break;
		case OT_CLASS:			c = sqi_class(obj); break;
		case OT_ARRAY:			c = sqi_array(obj); break;
		case OT_INSTANCE:		c = sqi_instance(obj); break;
		case OT_CLOSURE:		c = sqi_closure(obj); break;
		case OT_NATIVECLOSURE:	c = sqi_nativeclosure(obj); break;
		case OT_THREAD:			c = sqi_thread(obj); break;
		default: break;
		}

		if (c)
		{
			const char* objtype = typeid(*c).name();
			if (strstr(objtype, "struct ") == objtype)
				objtype += strlen("struct ");
			PrintObject(c, objtype, prefix, true);
		}
		else
		{
			switch (sqi_type(obj))
			{
			case OT_STRING:		LOG(0, "%sstring: '%s'\n", prefix, sqi_string(obj)->_val); break;
			default: break;
			}
		}
	}

	bool PrintObject(SQCollectable* obj, const char* objtype, const char* prefix, bool no_into = false)
	{
		if (ItemFilter.empty() || !Wildcard::match(ItemFilter, objtype))
			return true;

// 		if (obj == g_ScriptService->GetRootThread())
// 		{
// 			LOG(0, "%s%08x (%d) MAIN THREAD\n", prefix, obj, obj->_uiRef);
// 			return true;
// 		}
// 
		SQVM* vm = dynamic_cast<SQVM*>(obj);
		if (vm)
		{
			LOG(0, "%s%08x (%d) %s\n", prefix, obj, obj->_uiRef, objtype, vm->_top);
			return true;
		}

		SQTable* t = dynamic_cast<SQTable*>(obj);
		if (t)
		{
			LOG(0, "%s%08x (%d) %s: %d items\n", prefix, obj, obj->_uiRef, objtype, t->CountUsed());

			if (!no_into)
			{
				for (uint i=0; i < (uint)t->_numofnodes; ++i)
				{
					PrintObject(t->_nodes[i].key, "..  key ");
					PrintObject(t->_nodes[i].val, "..  value ");
				}
			}

			return true;
		}

		SQNativeClosure* nc = dynamic_cast<SQNativeClosure*>(obj);
		if (nc)
		{
			LOG(0, "%s%08x (%d) %s: %s -> %08x()\n", prefix, obj, obj->_uiRef, objtype, nc->_name, nc->_function);
			return true;
		}

		SQInstance* inst = dynamic_cast<SQInstance*>(obj);
		if (inst)
		{
			const char* classname = (inst->_class && sqi_type(inst->_class->_methods._vals->val) == OT_STRING) ? sqi_string(inst->_class->_methods._vals->val)->_val : "<unknown>";
			LOG(0, "%s%08x (%d) %s: %s -> %08x\n", prefix, obj, obj->_uiRef, objtype, classname, inst->_userpointer);
			return true;
		}

		SQClass* c = dynamic_cast<SQClass*>(obj);
		if (c)
		{
			const char* classname = sqi_type(c->_methods._vals->val) == OT_STRING ? sqi_string(c->_methods._vals->val)->_val : "<unknown>";
			LOG(0, "%s%08x (%d) %s: %s\n", prefix, obj, obj->_uiRef, objtype, classname);
			return true;
		}

		SQClosure* cl = dynamic_cast<SQClosure*>(obj);
		if (cl)
		{
			const char* fn = cl->_function->_name._type == OT_STRING ? cl->_function->_name._unVal.pString->_val : "<unknown>";
			const char* src = cl->_function->_sourcename._type == OT_STRING ? cl->_function->_sourcename._unVal.pString->_val : "src: <unknown>";
			LOG(0, "%s%08x (%d) %s: %s() from '%s'\n", prefix, obj, obj->_uiRef, objtype, fn, src);
			return true;
		}

		SQFunctionProto* fp = dynamic_cast<SQFunctionProto*>(obj);
		if (fp)
		{
			return true; // function proto is associated to a SQClosure so don't need to print.
		}

		LOG(0, "%s%08x (%d) %s\n", prefix, obj, obj->_uiRef, objtype);
		return true;
	}

	static SQBool VisitGC(SQCollectable* obj, SQUserPointer up)
	{
		const char* objtype = typeid(*obj).name();
		if (strstr(objtype, "struct ") == objtype)
			objtype += strlen("struct ");

		DumpGCVisitor* self = (DumpGCVisitor*)up;
		self->TotalCount++;
		self->Counters[objtype]++;

		self->PrintObject(obj, objtype, "-- ");

		return true;
	}
};

static SQBool ThreadVisitor(SQCollectable* obj, SQUserPointer up)
{
	SQVM* vm = dynamic_cast<SQVM*>(obj);

// 	if (vm && vm != g_ScriptService->GetRootThread())
// 	{
// 		LOG(0, "-- thread: %08x vmstate: %d\n", vm, sq_getvmstate(vm));
// 
// 		sqx_dump(vm);
// 	}

	return true;
}

void GCDump(HSQUIRRELVM v, const char* filter = "*")
{
	SQGC* gc = &_ss(v)->_gc;
	DumpGCVisitor visitor;
	visitor.ItemFilter = filter;
	gc->debugVisitAll(DumpGCVisitor::VisitGC, &visitor);

	visitor.Report();
}

void SSFinale(SQSharedState* ss)
{
#ifndef _RELEASE
	LOG(0, "++ SSFinale\n");
	ss->_gc.debugVisitAll(ThreadVisitor, NULL);
#endif
}

void GCFinale(SQGC* gc)
{
#ifndef _RELEASE
	LOG(0, "++ GCFinale : final isolated objects:\n");
	DumpGCVisitor visitor;
	visitor.ItemFilter = "*";
	gc->debugVisitAll(DumpGCVisitor::VisitGC, &visitor);

	visitor.Report();
#endif
}

void GCCredit(SQGC* gc)
{
#ifndef _RELEASE
	LOG(0, "++ GCCredit : remaining objects after closed\n");
	DumpGCVisitor visitor;
	visitor.ItemFilter = "*";
	gc->debugVisitAll(DumpGCVisitor::VisitGC, &visitor);

	visitor.Report();
#endif
}

SQInteger NB_ScriptDebugger::_nb_gcDump(HSQUIRRELVM v)
{
	SQGC& gc = v->_sharedstate->_gc;

	if (optBool(v, 3, false))
		gc.fullSweep();

	DumpGCVisitor visitor;
	visitor.ItemFilter = optString(v, 2, "");
	gc.debugVisitAll(DumpGCVisitor::VisitGC, &visitor);

	visitor.Report();
	return 0;
}

NS_NIT_END;

////////////////////////////////////////////////////////////////////////////////
