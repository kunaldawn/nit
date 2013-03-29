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

#include "nit/script/NitLibCore.h"

#include "nit/script/NitBind.h"
#include "nit/script/NitBindMacro.h"

#include "nit/script/ScriptRuntime.h"

#include "nit/runtime/MemManager.h"
#include "nit/runtime/LogManager.h"
#include "nit/io/Stream.h"
#include "nit/io/MemoryBuffer.h"
#include "nit/io/ZStream.h"
#include "nit/io/Archive.h"
#include "nit/io/FileLocator.h"
#include "nit/data/Settings.h"
#include "nit/data/ParserUtil.h"
#include "nit/event/EventAutomata.h"
#include "nit/ref/CacheHandle.h"
#include "nit/data/DateTime.h"
#include "nit/data/RegExp.h"
#include "nit/async/AsyncJob.h"

#include "squirrel/sqstdstring.h"
#include "squirrel/sqstdblob.h"

#include "squirrel/sqobject.h"
#include "squirrel/sqstate.h"
#include "squirrel/sqtable.h"
#include "squirrel/sqvm.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::RefCounted, NULL, incRefCount, decRefCount);

class NB_RefCounted : TNitClass<RefCounted>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(_refCount),
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY(weak),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(_refCount)				{ return push(v, self(v)->getRefCount()); }

	static SQRESULT WeakGetRef(HSQUIRRELVM v, SQUserPointer up, SQObjectPtr& outValue)
	{
		WeakRef* weak = (WeakRef*)up;

		RefCounted* obj = static_cast<RefCounted*>(weak->getObject());
		SQObjectRef inst;
		SQRESULT sr;

		if (obj)
		{
			sr = bindInstance(v, obj, inst);
			if (SQ_SUCCEEDED(sr))
				outValue = inst;
		}
		else
		{
			outValue.Null();
			sr = SQ_OK;
		}

		return sr;
	}

	static SQInteger WeakRelease(HSQUIRRELVM v, SQUserPointer up, SQInteger size)
	{
		WeakRef* weak = (WeakRef*)up;
		weak->decRefCount();
		return 0;
	}

	NB_FUNC(weak)
	{
		WeakRef* weak = self(v)->_weak();
		weak->incRefCount();

		SQNativeWeakRef* nw = SQNativeWeakRef::Create(weak, WeakGetRef, _ss(v));
		nw->_hook = WeakRelease;
		
		v->Push(nw);
		return 1;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_RAW_PTR(NIT_API, nit::MemManager, NULL);

class NB_MemManager : TNitClass<MemManager>
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
			FUNC_ENTRY_H(dump,			"()"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_FUNC(dump)						{ self(v)->dump(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_RAW_PTR(NIT_API, nit::LogManager, NULL);

class NB_LogManager : TNitClass<LogManager>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(defaultLogLevel),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(beginScope,	"(channel: LogChannel)"),
			FUNC_ENTRY_H(endScope,		"(channel: LogChannel)"),
			FUNC_ENTRY_H(setLogLevel,	"(tag: string, level: LEVEL_*)"),
			FUNC_ENTRY_H(getLogLevel,	"(tag: string): LEVEL_*"),
			FUNC_ENTRY_H(ignore,		"(tag: string, ...) // same as SetLogLevel(tag, LOG_LEVEL_IGNORED)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "LOG_LEVEL");
		newSlot(v, -1, "DEFAULT",		(int)LOG_LEVEL_DEFAULT);
		newSlot(v, -1, "IGNORED",		(int)LOG_LEVEL_IGNORED);
		newSlot(v, -1, "VERBOSE",		(int)LOG_LEVEL_VERBOSE);
		newSlot(v, -1, "DEBUG",			(int)LOG_LEVEL_DEBUG);
		newSlot(v, -1, "INFO",			(int)LOG_LEVEL_INFO);
		newSlot(v, -1, "WARNING",		(int)LOG_LEVEL_WARNING);
		newSlot(v, -1, "ERROR",			(int)LOG_LEVEL_ERROR);
		newSlot(v, -1, "FATAL",			(int)LOG_LEVEL_FATAL);
		newSlot(v, -1, "QUIET",			(int)LOG_LEVEL_QUIET);
		sq_poptop(v);
	};

	NB_PROP_GET(defaultLogLevel)		{ return push(v, (int)self(v)->getDefaultLogLevel()); }

	NB_PROP_SET(defaultLogLevel)		{ self(v)->setDefaultLogLevel((LogLevel)getInt(v, 2)); return 0; }

	NB_FUNC(beginScope)					{ self(v)->beginScope(get<LogChannel>(v, 2)); return 0; }
	NB_FUNC(endScope)					{ self(v)->endScope(get<LogChannel>(v, 2)); return 0; }

	NB_FUNC(setLogLevel)				{ self(v)->setLogLevel(getString(v, 2), (LogLevel)getInt(v, 3)); return 0; }
	NB_FUNC(getLogLevel)				{ return push(v, (int)self(v)->getLogLevel(LogManager::tagId(getString(v, 2)))); }

	NB_FUNC(ignore)
	{
		int nargs = sq_gettop(v);
		for (int i=2; i<=nargs; ++i)
		{
			self(v)->setLogLevel(getString(v, i), LOG_LEVEL_IGNORED);
		}
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::LogChannel, RefCounted, incRefCount, decRefCount);

class NB_LogChannel : TNitClass<LogChannel>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(name),
			PROP_ENTRY_R(parent),
			PROP_ENTRY_R(indent),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(name: string, parent: LogChannel = null)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(name)					{ return push(v, self(v)->getName().c_str()); }
	NB_PROP_GET(parent)					{ return push(v, self(v)->getParent()); }
	NB_PROP_GET(indent)					{ return push(v, self(v)->getIndent()); }

	NB_CONS()							{ setSelf(v, new LogChannel(getString(v, 2), opt<LogChannel>(v, 3, NULL))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WEAK(NIT_API, nit::ScriptUnit, NULL);

class NB_ScriptUnit : TNitClass<ScriptUnit>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(id),
			PROP_ENTRY_R(source),
			PROP_ENTRY_R(locator),
			PROP_ENTRY_R(compiled),
			PROP_ENTRY_R(required),
			NULL,
		};

		FuncEntry funcs[] =
		{
			NULL,
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(id)						{ return push(v, self(v)->getId()); }
	NB_PROP_GET(source)					{ return push(v, self(v)->getSource()); }
	NB_PROP_GET(locator)				{ return push(v, self(v)->getLocator()); }
	NB_PROP_GET(compiled)				{ return push(v, self(v)->isCompiled()); }
	NB_PROP_GET(required)				{ return push(v, self(v)->isRequired()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::ScriptRuntime, NULL, incRefCount, decRefCount);

class NB_ScriptRuntime : TNitClass<ScriptRuntime>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(unit),
			PROP_ENTRY_R(locator),
			PROP_ENTRY_R(allLoaded),
			PROP_ENTRY_R(allRequired),

			PROP_ENTRY	(defaultLocator),
			NULL,
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(getLoaded,		"(id: string): ScriptUnit"),
			FUNC_ENTRY_H(getClasses,	"(): class[]"),
			FUNC_ENTRY_H(command,		"(cmdline: string)"),
			NULL,
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(unit)					{ return push(v, self(v)->getUnit()); }
	NB_PROP_GET(locator)				{ return push(v, self(v)->getLocator()); }
	NB_PROP_GET(defaultLocator)			{ return push(v, self(v)->getDefaultLocator()); }

	NB_PROP_SET(defaultLocator)			{ self(v)->setDefaultLocator(opt<StreamLocator>(v, 2, NULL)); return 0; }

	NB_PROP_GET(allLoaded)
	{
		vector<String>::type r;
		for (ScriptRuntime::UnitMap::const_iterator itr = self(v)->allLoaded().begin(), end = self(v)->allLoaded().end(); itr != end; ++itr)
		{
			r.push_back(itr->first);
		}
		std::sort(r.begin(), r.end());

		sq_newarray(v, 0);
		for (uint i=0; i<r.size(); ++i)
		{
			arrayAppend(v, -1, r[i]);
		}
		return 1;
	}

	NB_PROP_GET(allRequired)
	{
		vector<String>::type r;
		for (ScriptRuntime::UnitMap::const_iterator itr = self(v)->allLoaded().begin(), end = self(v)->allLoaded().end(); itr != end; ++itr)
		{
			if (!itr->second->isRequired())
				continue;
			r.push_back(itr->first);
		}
		std::sort(r.begin(), r.end());

		sq_newarray(v, 0);
		for (uint i=0; i<r.size(); ++i)
		{
			arrayAppend(v, -1, r[i]);
		}
		return 1;
	}

	NB_FUNC(getLoaded)					{ return push(v, self(v)->getLoaded(getString(v, 2))); }

	NB_FUNC(getClasses)
	{
		sq_newarray(v, 0);
		for (uint i=0; i<v->_sharedstate->_classes->size(); ++i)
		{
			v->Push((*v->_sharedstate->_classes)[i]);
			if (sq_gettype(v, -1) == OT_NULL)
			{
				sq_poptop(v);
				continue;
			}
			sq_arrayappend(v, -2);
		}
		return 1;
	}

	NB_FUNC(command)					{ self(v)->command(getString(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

typedef ScriptWaitBlock WaitBlock;

NB_TYPE_REF(NIT_API, nit::WaitBlock, RefCounted, incRefCount, decRefCount);

class NB_WaitBlock : TNitClass<WaitBlock>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(numSuspended),
			PROP_ENTRY	(allowCount),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"),
			FUNC_ENTRY_H(wait,			"(signalMask: uint = -1): uint	// returns received signal, zero when timeout"),
			FUNC_ENTRY_H(signal,		"(signalFlags: uint): int	// returns wakeup count"),
			FUNC_ENTRY_H(killAll,		"(msg: string=null)"),
			FUNC_ENTRY_H(all,			"(): thread[]"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(numSuspended)			{ return push(v, self(v)->getNumSuspended()); }
	NB_PROP_GET(allowCount)				{ return push(v, self(v)->getAllowCount()); }

	NB_PROP_SET(allowCount)				{ self(v)->setAllowCount(getInt(v, 2)); return 0; }

	NB_CONS()							{ sq_setinstanceup(v, 1, new ScriptWaitBlock(ScriptRuntime::getRuntime(v))); return 0; }
	NB_FUNC(wait)						{ return self(v)->wait(v, optInt(v, 2, -1)); }
	NB_FUNC(signal)						{ return push(v, self(v)->signal(getInt(v, 2))); }
	NB_FUNC(killAll)					{ self(v)->killAll(optString(v, 2, NULL)); return 0; }
	NB_FUNC(all)						{ return self(v)->pushAll(v); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WEAK(NIT_API, nit::CacheManager, NULL);

class NB_CacheManager : TNitClass<CacheManager>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(name),
			PROP_ENTRY_R(validCount),
			PROP_ENTRY_R(validFootprint),
			PROP_ENTRY_R(wiredCount),
			PROP_ENTRY_R(wiredFootprint),
			PROP_ENTRY_R(activeCount),
			PROP_ENTRY_R(activeFootprint),
			PROP_ENTRY_R(inactiveCount),
			PROP_ENTRY_R(inactiveFootprint),
			PROP_ENTRY	(inactiveAgeThreshold),
			PROP_ENTRY	(footprintThreshold),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(invalidateAll,	"()"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }
	NB_PROP_GET(validCount)				{ return push(v, self(v)->getValidCount()); }
	NB_PROP_GET(validFootprint)			{ return push(v, self(v)->getValidFootprint()); }
	NB_PROP_GET(wiredCount)				{ return push(v, self(v)->getWiredCount()); }
	NB_PROP_GET(wiredFootprint)			{ return push(v, self(v)->getWiredFootprint()); }
	NB_PROP_GET(activeCount)			{ return push(v, self(v)->getActiveCount()); }
	NB_PROP_GET(activeFootprint)		{ return push(v, self(v)->getActiveFootprint()); }
	NB_PROP_GET(inactiveCount)			{ return push(v, self(v)->getInactiveCount()); }
	NB_PROP_GET(inactiveFootprint)		{ return push(v, self(v)->getInactiveFootprint()); }
	NB_PROP_GET(inactiveAgeThreshold)	{ return push(v, self(v)->getInactiveAgeThreshold()); }
	NB_PROP_GET(footprintThreshold)		{ return push(v, self(v)->getFootprintThreshold()); }

	NB_PROP_SET(inactiveAgeThreshold)	{ self(v)->setInactiveAgeThreshold(getInt(v, 2)); return 0; }
	NB_PROP_SET(footprintThreshold)		{ self(v)->setFootprintThreshold(getInt(v, 2)); return 0; }

	NB_FUNC(invalidateAll)				{ self(v)->invalidateAll(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::CacheHandle, RefCounted, incRefCount, decRefCount);

class NB_CacheHandle : TNitClass<CacheHandle>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(cacheManager),
			PROP_ENTRY	(cacheFootprint),
			PROP_ENTRY_R(cacheValid),
			PROP_ENTRY_R(cacheWired),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(wire,			"(): bool"),
			FUNC_ENTRY_H(unwire,		"()"),
			FUNC_ENTRY_H(touch,			"(): bool"),
			FUNC_ENTRY_H(invalidate,	"(): bool"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(cacheManager)			{ return push(v, self(v)->getCacheManager()); }
	NB_PROP_GET(cacheFootprint)			{ return push(v, self(v)->getCacheFootprint()); }
	NB_PROP_GET(cacheValid)				{ return push(v, self(v)->isCacheValid()); }
	NB_PROP_GET(cacheWired)				{ return push(v, self(v)->isCacheWired()); }

	NB_PROP_SET(cacheFootprint)			{ self(v)->setCacheFootprint(getInt(v, 2)); return 0; }

	NB_FUNC(wire)						{ return push(v, self(v)->wire()); }
	NB_FUNC(unwire)						{ self(v)->unwire(); return 0; }
	NB_FUNC(touch)						{ return push(v, self(v)->touch()); }
	NB_FUNC(invalidate)					{ return push(v, self(v)->invalidate()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_VALUE(NIT_API, nit::Timestamp, NULL);

class NB_Timestamp : TNitClass<Timestamp>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] = 
		{
			PROP_ENTRY_R(valid),
			PROP_ENTRY_R(unixTime64Str),
			PROP_ENTRY	(iso8601),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"(d: DateTime)"
			"\n"						"(iso8601: string)"),
			FUNC_ENTRY_H(now,			"[class] (): Timestamp"),
			FUNC_ENTRY_H(format,		"(fmt = \"%Y-%m-%d %H:%M:%S\"): string // see strftime"),
			FUNC_ENTRY_H(formatz,		"(fmt = \"%Y-%m-%d %H:%M:%S\"): string // see strftime"),
			FUNC_ENTRY	(_tostring),
			FUNC_ENTRY	(_add),
			FUNC_ENTRY	(_sub),
			NULL
		};

		bind(v, props, funcs);

		addStatic(v, "SECOND",			(int)Timestamp::SECOND);
		addStatic(v, "MINUTE",			(int)Timestamp::MINUTE);
		addStatic(v, "HOUR",			(int)Timestamp::HOUR);
		addStatic(v, "DAY",				(int)Timestamp::DAY);
		addStatic(v, "WEEK",			(int)Timestamp::WEEK);
	}

	NB_CONS()							
	{ 
		if (isString(v, 2))
			new (self(v)) Timestamp(getString(v, 2));
		else
			new (self(v)) Timestamp(*get<DateTime>(v, 2)); 
		return 0; 
	}

	NB_PROP_GET(valid)					{ return push(v, self(v)->isValid()); }
	NB_PROP_GET(unixTime64Str)			{ int64 t = self(v)->getUnixTime64(); return pushFmt(v, "%lld", t); }
	NB_PROP_GET(iso8601)				{ return push(v, self(v)->getIso8601()); }

	NB_PROP_SET(iso8601)				{ bool ok = self(v)->setIso8601(getString(v, 2)); return ok ? 0 : sq_throwfmt(v, "invalid iso8601 format: '%s'", getString(v, 2)); }

	NB_FUNC(now)						{ return push(v, Timestamp::now()); }
	NB_FUNC(format)						{ return push(v, self(v)->format(optString(v, 2, "%Y-%m-%d %H:%M:%S"))); }
	NB_FUNC(formatz)					{ return push(v, self(v)->formatz(optString(v, 2, "%Y-%m-%d %H:%M:%S"))); }

	NB_FUNC(_tostring)					{ String s = self(v)->getIso8601(); return pushFmt(v, "(Timestamp: %s)", s.c_str()); }

	NB_FUNC(_add)						{ return push(v, *self(v) + getFloat(v, 2)); }

	NB_FUNC(_sub)						
	{ 
		if (isNumber(v, 2))
			return push(v, *self(v) - getFloat(v, 2));
		else
			return push(v, *self(v) - *get<Timestamp>(v, 2));
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_VALUE(NIT_API, nit::DateTime, NULL);

class NB_DateTime : TNitClass<DateTime>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] = 
		{
			PROP_ENTRY_R(valid),
			PROP_ENTRY_R(second),
			PROP_ENTRY_R(minute),
			PROP_ENTRY_R(hour),
			PROP_ENTRY_R(day),
			PROP_ENTRY_R(month),
			PROP_ENTRY_R(year),
			PROP_ENTRY_R(weekDay),
			PROP_ENTRY_R(yearDay),
			PROP_ENTRY_R(summerTime),
			PROP_ENTRY_R(timeZoneDiff),
			PROP_ENTRY	(iso8601),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"(t: Timestamp)"
			"\n"						"(iso8601: string)"),
			
			FUNC_ENTRY_H(now,			"[class] (): DateTime"),
			FUNC_ENTRY_H(nowUtc,		"[class] (): DateTime"),
			FUNC_ENTRY_H(today,			"[class] (hour=0, minute=0, second=0): DateTime"),
			FUNC_ENTRY_H(todayUtc,		"[class] (hour=0, minute=0, second=0): DateTime"),
			FUNC_ENTRY_H(wantDay,		"[class] (year, month, day, hour, minute, second, wday, yday, summer): DateTime"),
			FUNC_ENTRY_H(wantDayUtc,	"[class] (year, month, day, hour, minute, second, wday, yday, summer): DateTime"),
			FUNC_ENTRY_H(format,		"(fmt = \"%Y-%m-%d %H:%M:%S\"): string // see strftime"),
			FUNC_ENTRY_H(formatz,		"(fmt = \"%Y-%m-%d %H:%M:%S\"): string // see strftime"),
			FUNC_ENTRY_H(toUtc,			"(): DateTime"),
			FUNC_ENTRY_H(toLocal,		"(): DateTime"
			"\n"						"(timeZoneDiffInSecond: int): DateTime"),
			FUNC_ENTRY	(_tostring),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(valid)					{ return push(v, self(v)->isValid()); }
	NB_PROP_GET(second)					{ return push(v, self(v)->getSecond()); }
	NB_PROP_GET(minute)					{ return push(v, self(v)->getMinute()); }
	NB_PROP_GET(hour)					{ return push(v, self(v)->getHour()); }
	NB_PROP_GET(day)					{ return push(v, self(v)->getDay()); }
	NB_PROP_GET(month)					{ return push(v, self(v)->getMonth()); }
	NB_PROP_GET(year)					{ return push(v, self(v)->getYear()); }
	NB_PROP_GET(weekDay)				{ return push(v, self(v)->getWeekDay()); }
	NB_PROP_GET(yearDay)				{ return push(v, self(v)->getYearDay()); }
	NB_PROP_GET(summerTime)				{ return push(v, self(v)->isSummerTime()); }
	NB_PROP_GET(timeZoneDiff)			{ return push(v, self(v)->getTimeZoneDiff() / Timestamp::SECOND); }
	NB_PROP_GET(iso8601)				{ return push(v, self(v)->getIso8601()); }

	NB_PROP_SET(iso8601)				{ bool ok = self(v)->setIso8601(getString(v, 2)); return ok ? 0 : sq_throwfmt(v, "invalid iso8601 format: '%s'", getString(v, 2)); }

	NB_CONS()							
	{ 
		if (isString(v, 2))
			new (self(v)) DateTime(getString(v, 2));
		else
			new (self(v)) DateTime(*get<Timestamp>(v, 2)); 
		return 0; 
	}

	NB_FUNC(now)						{ return push(v, DateTime::now()); }
	NB_FUNC(nowUtc)						{ return push(v, DateTime::nowUtc()); }
	NB_FUNC(today)						{ return push(v, DateTime::today(optInt(v, 2, 0), optInt(v, 3, 0), optInt(v, 4, 0))); }
	NB_FUNC(todayUtc)					{ return push(v, DateTime::todayUtc(optInt(v, 2, 0), optInt(v, 3, 0), optInt(v, 4, 0))); }
	NB_FUNC(wantDay)					{ return push(v, DateTime::wantDay(getInt(v, 2), getInt(v, 3), getInt(v, 4), getInt(v, 5), getInt(v, 6), getInt(v, 7), getInt(v, 8), getInt(v, 9), getBool(v, 10))); }
	NB_FUNC(wantDayUtc)					{ return push(v, DateTime::wantDay(getInt(v, 2), getInt(v, 3), getInt(v, 4), getInt(v, 5), getInt(v, 6), getInt(v, 7), getInt(v, 8), getInt(v, 9), getBool(v, 10))); }
	NB_FUNC(format)						{ return push(v, self(v)->format(optString(v, 2, "%Y-%m-%d %H:%M:%S"))); }
	NB_FUNC(formatz)					{ return push(v, self(v)->format(optString(v, 2, "%Y-%m-%d %H:%M:%S"))); }
	NB_FUNC(toUtc)						{ return push(v, self(v)->toUtc()); }
	NB_FUNC(toLocal)					{ if (isNone(v, 2)) return push(v, self(v)->toLocal()); else return push(v, self(v)->toLocal(getInt(v, 2))); }
	NB_FUNC(_tostring)					{ String s = self(v)->formatz(); return pushFmt(v, "(DateTime: %s)", s.c_str()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_VALUE(NIT_API, nit::RegExp, NULL);

// It may be redundancy but we'll replace regexp code later so bind another Regexp here.

class NB_RegExp : TNitClass<RegExp>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(subExpCount),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(pattern)"),
			FUNC_ENTRY_H(match,			"(string): bool"),
			FUNC_ENTRY_H(search,		"(string, start=0): [int, int] // returns null when not found"),
			FUNC_ENTRY_H(capture,		"(string, subexp=0, start=0): string // returns null when not found"),
			FUNC_ENTRY_H(captureAll,	"(string, start=0): string[] // returns null when not found"),
			FUNC_ENTRY_H(substitute,	"(string, replacement: string, subexp=0): string"),
			FUNC_ENTRY_H(format,		"(string, fmt: string, start=0): string // %*, %<, %>, %0~%9"),
			FUNC_ENTRY_H(help,			"[class] () // shows help text to log"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(subExpCount)			{ return push(v, self(v)->getSubExpCount()); }

	NB_CONS()							
	{ 
		try
		{
			new (self(v)) RegExp(getString(v, 2)); 
			return 0; 
		}
		catch (Exception& ex)
		{
			return sq_throwerror(v, ex.getFullDescription().c_str());
		}
	}

	NB_FUNC(match)						{ return push(v, self(v)->match(getString(v, 2))); }

	NB_FUNC(search)
	{
		const char* str = getString(v, 2);
		const char *begin, *end;
		bool found = self(v)->search(getString(v, 2), &begin, &end, optInt(v, 3, 0));
		if (!found) return 0;
		sq_newarray(v, 0);
		arrayAppend(v, -1, int(begin - str));
		arrayAppend(v, -1, int(end - str));
		return 1;
	}

	NB_FUNC(capture)
	{
		const char* str = getString(v, 2);
		RegExp::SubExp subexp;
		bool found = self(v)->capture(str, subexp, optInt(v, 3, 0), optInt(v, 4, 0));
		if (!found) return 0;
		sq_pushstring(v, subexp.begin, subexp.length());
		return 1;
	}

	NB_FUNC(captureAll)
	{
		const char* str = getString(v, 2);
		vector<RegExp::SubExp>::type results;
		bool found = self(v)->captureAll(str, results, optInt(v, 3, 0));
		if (!found) return 0;

		sq_newarray(v, 0);
		for (uint i=0; i<results.size(); ++i)
			sq_pushstring(v, results[i].begin, results[i].length());
		return 1;
	}

	NB_FUNC(substitute)					{ return push(v, self(v)->substitute(getString(v, 2), getString(v, 3), optInt(v, 4, 0), optInt(v, 5, 0))); }
	NB_FUNC(format)						{ return push(v, self(v)->format(getString(v, 2), getString(v, 3), optInt(v, 4, 0))); }
	NB_FUNC(help)						{ RegExp::Help(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_VALUE(NIT_API, nit::ContentType, NULL);

class NB_ContentType : TNitClass<ContentType>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(name),
			PROP_ENTRY_R(extension),
			PROP_ENTRY_R(mimeType),
			PROP_ENTRY_R(value),

			PROP_ENTRY_R(isUnknown),
			PROP_ENTRY_R(isText),
			PROP_ENTRY_R(isScript),
			PROP_ENTRY_R(isXML),
			PROP_ENTRY_R(isBinary),
			PROP_ENTRY_R(isOgre),
			PROP_ENTRY_R(isNit),
			PROP_ENTRY_R(isArchive),
			PROP_ENTRY_R(isImage),
			PROP_ENTRY_R(isAudio),
			PROP_ENTRY_R(isVideo),
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY_H(fromValue,		"[class] (value: int): ContentType"),
			FUNC_ENTRY_H(fromName,		"[class] (name: string): ContentType"),
			FUNC_ENTRY_H(fromExtension,	"[class] (ext: string): ContentType"),
			FUNC_ENTRY_H(fromStreamName,"[class] (streamname: string): ContentType"),
			FUNC_ENTRY_H(fromMimeType,	"[class] (mimetype: string): ContentTYpe"),
			FUNC_ENTRY	(_eq), // TODO: This meta method not yet exists
			FUNC_ENTRY	(_tostring),
			NULL
		};

		bind(v, props, funcs);

		vector<ContentType>::type cts;
		ContentType::allContentTypes(cts);

		for (uint i=0; i<cts.size(); ++i)
		{
			const String& name = cts[i].getName();
			size_t ppos = name.find('.');
			if (ppos != name.npos)
			{
				addStaticTable(v, name.substr(0, ppos).c_str(), false, false);
				newSlot(v, -1, name.substr(ppos+1), cts[i]);
				sq_poptop(v);
			}
			else
			{
				addStatic(v, name.c_str(), cts[i]);
			}
		}
	}

	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }
	NB_PROP_GET(extension)				{ return push(v, self(v)->getExtension()); }
	NB_PROP_GET(mimeType)				{ return push(v, self(v)->getMimeType()); }
	NB_PROP_GET(value)					{ return push(v, self(v)->getValue()); }

	NB_PROP_GET(isUnknown)				{ return push(v, self(v)->isUnknown()); }
	NB_PROP_GET(isText)					{ return push(v, self(v)->isText()); }
	NB_PROP_GET(isScript)				{ return push(v, self(v)->isScript()); }
	NB_PROP_GET(isXML)					{ return push(v, self(v)->isXML()); }
	NB_PROP_GET(isBinary)				{ return push(v, self(v)->isBinary()); }
	NB_PROP_GET(isOgre)					{ return push(v, self(v)->isOgre()); }
	NB_PROP_GET(isNit)					{ return push(v, self(v)->isNit()); }
	NB_PROP_GET(isArchive)				{ return push(v, self(v)->isArchive()); }
	NB_PROP_GET(isImage)				{ return push(v, self(v)->isImage()); }
	NB_PROP_GET(isAudio)				{ return push(v, self(v)->isAudio()); }
	NB_PROP_GET(isVideo)				{ return push(v, self(v)->isVideo()); }

	NB_FUNC(fromValue)					{ return push(v, ContentType(getInt(v, 2))); }
	NB_FUNC(fromName)					{ return push(v, ContentType::fromName(getString(v, 2))); }
	NB_FUNC(fromExtension)				{ return push(v, ContentType::fromExtension(getString(v, 2))); }
	NB_FUNC(fromStreamName)				{ return push(v, ContentType::fromStreamName(getString(v, 2))); }
	NB_FUNC(fromMimeType)				{ return push(v, ContentType::fromMimeType(getString(v, 2))); }

	NB_FUNC(_eq)						{ return push(v, self(v)->getValue() == get<ContentType>(v, 2)->getValue()); }
	NB_FUNC(_tostring)					{ sqx_pushstringf(v, "(ContentType.%s: 0x%04X)", self(v)->getName().c_str(), (int)self(v)->getValue()); return 1; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::ContentBase, RefCounted, incRefCount, decRefCount);

class NB_ContentBase : TNitClass<ContentBase>
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
			NULL
		};

		bind(v, props, funcs);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::StreamLocator, RefCounted, incRefCount, decRefCount);

class NB_StreamLocator : TNitClass<StreamLocator>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(name),
			PROP_ENTRY_R(hasProxy),
			PROP_ENTRY	(proxy),
			PROP_ENTRY_R(useCount),
			PROP_ENTRY_R(caseSensitive),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(locate,		"(streamName, throwEx=true): StreamSource\n"
										"(streamName, locatorPattern, throwEx=true): StreamSource"),
			FUNC_ENTRY_H(find,			"(pattern): StreamSource[]\n"
										"(streamPattern, locatorPattern): StreamSource[]"),
			FUNC_ENTRY_H(locateLocal,	"(streamName): StreamSource"),
			FUNC_ENTRY_H(findLocal,		"(pattern): StreamSource[]"),
			FUNC_ENTRY_H(getLocator,	"(locatorName, throwEx=true): StreamLocator"),
			FUNC_ENTRY_H(findLocator,	"(locatorPattern): StreamLocator[]"),

			FUNC_ENTRY_H(isRequired,	"(req: StreamLocator): bool"),
			FUNC_ENTRY_H(require,		"(req: StreamLocator, first=false)"),
			FUNC_ENTRY_H(unrequire,		"(req: StreamLocator)"),
			FUNC_ENTRY_H(unrequireAll,	"()"),
			FUNC_ENTRY_H(getRequiredList, "(): StreamLocator[]"),

			FUNC_ENTRY_H(makeUrl,		"(path): string"),

			FUNC_ENTRY_H(incUseCount,	"()"),
			FUNC_ENTRY_H(decUseCount,	"()"),

			FUNC_ENTRY	(_dump),
			FUNC_ENTRY	(_tostring),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }
	NB_PROP_GET(hasProxy)				{ return push(v, self(v)->hasProxy()); }
	NB_PROP_GET(proxy)					{ return push(v, self(v)->getProxy()); }
	NB_PROP_GET(useCount)				{ return push(v, self(v)->getUseCount()); }
	NB_PROP_GET(caseSensitive)			{ return push(v, self(v)->isCaseSensitive()); }
	
	NB_PROP_SET(proxy)					{ self(v)->setProxy(opt<StreamLocator>(v, 2, NULL)); return 0; }

	NB_FUNC(locateLocal)				{ return push(v, self(v)->locateLocal(getString(v, 2))); }
	NB_FUNC(getLocator)					{ return push(v, self(v)->getLocator(getString(v, 2), optBool(v, 3, true))); }

	NB_FUNC(isRequired)					{ return push(v, self(v)->isRequired(get<StreamLocator>(v, 2))); }
	NB_FUNC(require)					{ self(v)->require(get<StreamLocator>(v, 2), optBool(v, 3, false)); return 0; }
	NB_FUNC(unrequire)					{ self(v)->unrequire(get<StreamLocator>(v, 2)); return 0; }
	NB_FUNC(unrequireAll)				{ self(v)->unrequireAll(); return 0; }

	NB_FUNC(incUseCount)				{ self(v)->incUseCount(); return 0; }
	NB_FUNC(decUseCount)				{ self(v)->decUseCount(); return 0; }

	NB_FUNC(getRequiredList)
	{
		const StreamLocatorList& sl = self(v)->getRequiredList();

		sq_newarray(v, 0);
		for (uint i=0; i<sl.size(); ++i)
			arrayAppend(v, -1, sl[i].get());
		return 1;
	}

	NB_FUNC(makeUrl)					{ return push(v, self(v)->makeUrl(getString(v, 2))); }

	NB_FUNC(locate)					
	{ 
		if (isNone(v, 3) || isBool(v, 3))
			return push(v, self(v)->locate(getString(v, 2), optBool(v, 3, true))); 
		else
			return push(v, self(v)->locate(getString(v, 2), getString(v, 3), optBool(v, 4, true)));
	}

	NB_FUNC(find)
	{
		StreamSourceMap sl;

		if (isNone(v, 3))
			self(v)->find(getString(v, 2), sl);
		else
			self(v)->find(getString(v, 2), getString(v, 3), sl);

		sq_newarray(v, 0);
		for (StreamSourceMap::iterator itr = sl.begin(), end = sl.end(); itr != end; ++itr)
			arrayAppend(v, -1, itr->second.get());
		return 1;
	}

	NB_FUNC(findLocal)
	{
		StreamSourceMap sl;
		self(v)->findLocal(getString(v, 2), sl);

		sq_newarray(v, 0);
		for (StreamSourceMap::iterator itr = sl.begin(), end = sl.end(); itr != end; ++itr)
			arrayAppend(v, -1, itr->second.get());
		return 1;
	}

	NB_FUNC(findLocator)
	{
		StreamLocatorList sl;
		self(v)->findLocator(getString(v, 2), sl);

		sq_newarray(v, 0);
		for (uint i=0; i<sl.size(); ++i)
			arrayAppend(v, -1, sl[i].get());
		return 1;
	}

	NB_FUNC(_dump)
	{
		LOG(0, "  requires:\n");
		self(v)->printDependancy(2);
		return push(v, true);
	}

	NB_FUNC(_tostring)
	{
		type* o = self(v);
		const char* clsname = typeid(*o).name();
		if (clsname) clsname = &clsname[6]; // skip "class "
		return pushFmt(v, "(%s: '%s' -> %08X)", clsname, o->getName().c_str(), o);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::StreamSource, RefCounted, incRefCount, decRefCount);

class NB_StreamSource : TNitClass<StreamSource>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(locator),
			PROP_ENTRY_R(name),
			PROP_ENTRY_R(locatorName),
			PROP_ENTRY_R(url),
			PROP_ENTRY_R(contentType),
			PROP_ENTRY_R(content),
			PROP_ENTRY_R(extension),
			PROP_ENTRY_R(streamSize),
			PROP_ENTRY_R(memorySize),
			PROP_ENTRY_R(timestamp),
			PROP_ENTRY_R(hasCacheEntry),
			PROP_ENTRY_R(cached),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(open,			"(): StreamReader"),
			FUNC_ENTRY_H(link,			"(throwEx=true): Content"),
			FUNC_ENTRY_H(calcCrc32,		"(): int"),
			FUNC_ENTRY_H(calcAdler32,	"(): int"),
			FUNC_ENTRY	(_tostring),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(locator)				{ return push(v, self(v)->getLocator()); }
	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }
	NB_PROP_GET(locatorName)			{ return push(v, self(v)->getLocatorName()); }
	NB_PROP_GET(url)					{ return push(v, self(v)->getUrl()); }
	NB_PROP_GET(contentType)			{ return push(v, self(v)->getContentType()); }
	NB_PROP_GET(content)				{ return push(v, self(v)->getContent()); }
	NB_PROP_GET(extension)				{ return push(v, StreamSource::getExtension(self(v)->getName())); }
	NB_PROP_GET(streamSize)				{ return push(v, self(v)->getStreamSize()); }
	NB_PROP_GET(memorySize)				{ return push(v, self(v)->getMemorySize()); }
	NB_PROP_GET(timestamp)				{ return push(v, self(v)->getTimestamp()); }
	NB_PROP_GET(hasCacheEntry)			{ return push(v, self(v)->hasCacheEntry()); }
	NB_PROP_GET(cached)					{ return push(v, self(v)->IsCached()); }

	NB_FUNC(open)						{ return push(v, self(v)->open()); }
	NB_FUNC(link)						{ return push(v, self(v)->link<ContentBase>(optBool(v, 2, true)).get()); }
	NB_FUNC(calcCrc32)					{ return push(v, self(v)->calcCrc32()); }
	NB_FUNC(calcAdler32)				{ return push(v, self(v)->calcAdler32()); }

	NB_FUNC(_tostring)
	{ 
		type* o = self(v);
		const char* clsname = typeid(*o).name();
		if (clsname) clsname = &clsname[6]; // skip "class "
		return pushFmt(v, "('%s: %s': %s -> %08X)", o->getLocatorName().c_str(), o->getName().c_str(), clsname, o);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::StreamReader, RefCounted, incRefCount, decRefCount);

class NB_StreamReader : TNitClass<StreamReader>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(source),
			PROP_ENTRY_R(name),
			PROP_ENTRY_R(locatorName),

			PROP_ENTRY_R(buffered),
			PROP_ENTRY_R(sized),
			PROP_ENTRY_R(seekable),

			PROP_ENTRY_R(eof),
			PROP_ENTRY_R(size),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(buffer,		"(): MemoryBuffer"),
			FUNC_ENTRY_H(skip,			"(count: int)"),
			FUNC_ENTRY_H(seek,			"(pos: int)"),
			FUNC_ENTRY_H(tell,			"(): int"),
			FUNC_ENTRY_H(readI32,		"(): int"),
			FUNC_ENTRY_H(readU32,		"(): int"),
			FUNC_ENTRY_H(readI24,		"(): int"),
			FUNC_ENTRY_H(readU24,		"(): int"),
			FUNC_ENTRY_H(readI16,		"(): int"),
			FUNC_ENTRY_H(readU16,		"(): int"),
			FUNC_ENTRY_H(readI8,		"(): int"),
			FUNC_ENTRY_H(readU8,		"(): int"),
			FUNC_ENTRY_H(readF32,		"(): float"),
			FUNC_ENTRY_H(readChars,		"(numChars: int): string"),
			FUNC_ENTRY_H(readWChars,	"(numChars: int): string"),
			FUNC_ENTRY_H(readCStr,		"(): string"),
			FUNC_ENTRY_H(readWCStr,		"(): string"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(source)					{ return push(v, self(v)->getSource()); }
	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }
	NB_PROP_GET(locatorName)			{ return push(v, self(v)->getLocatorName()); }
	NB_PROP_GET(buffered)				{ return push(v, self(v)->isBuffered()); }
	NB_PROP_GET(sized)					{ return push(v, self(v)->isSized()); }
	NB_PROP_GET(seekable)				{ return push(v, self(v)->isSeekable()); }

	NB_PROP_GET(eof)					{ return push(v, self(v)->isEof()); }
	NB_PROP_GET(size)					{ return push(v, self(v)->getSize()); }

	NB_FUNC(buffer)						{ return push(v, self(v)->buffer()); }
	NB_FUNC(skip)						{ self(v)->skip(getInt(v, 2)); return 0; }
	NB_FUNC(seek)						{ self(v)->seek(getInt(v, 2)); return 0; }
	NB_FUNC(tell)						{ return push(v, self(v)->tell()); }

	NB_FUNC(readI32)					{ int32 value; self(v)->read(&value, sizeof(value)); return push(v, value); }
	NB_FUNC(readU32)					{ uint32 value; self(v)->read(&value, sizeof(value)); return push(v, (int)value); }
	NB_FUNC(readI24)					{ int32 value = 0; self(v)->read(&value, 3); return push(v, value); } // TODO: big endian platform
	NB_FUNC(readU24)					{ uint32 value = 0; self(v)->read(&value, 3); return push(v, (int)value); } // TODO: big endian platform
	NB_FUNC(readI16)					{ int16 value; self(v)->read(&value, sizeof(value)); return push(v, (int)value); }
	NB_FUNC(readU16)					{ uint16 value; self(v)->read(&value, sizeof(value)); return push(v, (int)value); }
	NB_FUNC(readI8)						{ int8 value; self(v)->read(&value, sizeof(value)); return push(v, (int)value); }
	NB_FUNC(readU8)						{ uint8 value; self(v)->read(&value, sizeof(value)); return push(v, (int)value); }
	NB_FUNC(readF32)					{ float value; self(v)->read(&value, sizeof(value)); return push(v, value); }

	NB_FUNC(readChars)					
	{ 
		size_t len = getInt(v, 2);
		if (len == 0) return push(v, "");
		vector<char>::type buf; 
		buf.resize(len); 
		self(v)->read(&buf[0], len); 
		sq_pushstring(v, &buf[0], len); 
		return 1; 
	}

	NB_FUNC(readWChars)				
	{ 
		size_t len = getInt(v, 2);
		if (len == 0) return push(v, "");
		vector<UniChar>::type buf; 
		buf.resize(len); 
		self(v)->read(&buf[0], len * sizeof(UniChar)); 
		return push(v, Unicode::toUtf8(&buf[0], len)); 
	}

	NB_FUNC(readCStr)
	{
		type* o = self(v);

		vector<char>::type buf;
		while (true)
		{
			char ch;
			if (o->readRaw(&ch, 1) != 1) break;
			if (ch == 0) break;
			buf.push_back(ch);
		}

		if (buf.empty())
			return push(v, "");

		sq_pushstring(v, &buf[0], buf.size());
		return 1;
	}

	NB_FUNC(readWCStr)
	{
		type* o = self(v);

		vector<UniChar>::type buf;
		while (true)
		{
			UniChar ch;
			if (o->readRaw(&ch, sizeof(ch)) != sizeof(ch)) break;
			if (ch == 0) break;
			buf.push_back(ch);
		}

		if (buf.empty()) 
			return push(v, "");

		return push(v, Unicode::toUtf8(&buf[0], buf.size()));
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::StreamWriter, RefCounted, incRefCount, decRefCount);

class NB_StreamWriter : TNitClass<StreamWriter>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(source),
			PROP_ENTRY_R(name),
			PROP_ENTRY_R(locatorName),

			PROP_ENTRY_R(buffered),
			PROP_ENTRY_R(sized),
			PROP_ENTRY_R(seekable),

			PROP_ENTRY_R(size),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(skip,			"(count: int)"),
			FUNC_ENTRY_H(seek,			"(pos: int)"),
			FUNC_ENTRY_H(tell,			"(): int"),
			FUNC_ENTRY_H(flush,			"(): bool"),
			FUNC_ENTRY_H(copy,			"(reader: StreamReader, offset=0, len=0, bufSize=4096): int"),

			// print support
			FUNC_ENTRY_H(writef,		"(fmt, ...): int // no auto linefeed"),
			FUNC_ENTRY_H(print,			"(str): int // auto linefeed, returns written length"),
			FUNC_ENTRY_H(printf,		"(fmt, ...): int // auto linefeed, returns written length"),
			FUNC_ENTRY_H(setPrintEncoding, "(encoding: string)"),
			FUNC_ENTRY_H(setAutoLineFeed, "(lf: string)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(source)					{ return push(v, self(v)->getSource()); }
	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }
	NB_PROP_GET(locatorName)			{ return push(v, self(v)->getLocatorName()); }
	NB_PROP_GET(buffered)				{ return push(v, self(v)->isBuffered()); }
	NB_PROP_GET(sized)					{ return push(v, self(v)->isSized()); }
	NB_PROP_GET(seekable)				{ return push(v, self(v)->isSeekable()); }

	NB_PROP_GET(size)					{ return push(v, self(v)->getSize()); }

	NB_FUNC(skip)						{ self(v)->skip(getInt(v, 2)); return 0; }
	NB_FUNC(seek)						{ self(v)->seek(getInt(v, 2)); return 0; }
	NB_FUNC(tell)						{ return push(v, self(v)->tell()); }
	NB_FUNC(flush)						{ return push(v, self(v)->flush()); }
	NB_FUNC(copy)						{ return push(v, self(v)->copy(get<StreamReader>(v, 2), optInt(v, 3, 0), optInt(v, 4, 0), optInt(v, 5, 4096))); }

	NB_FUNC(writef)
	{
		sq_tostring(v, 2);
		sq_replace(v, 2);
		SQChar* out;
		int len;
		if (SQ_FAILED(sqstd_format(v, 2, &len, &out)))
			return SQ_ERROR;

		len = self(v)->print(out, len);

		return push(v, len);
	}

	NB_FUNC(print)
	{
		const char* str;
		sq_tostring(v, 2);
		sq_replace(v, 2);
		sq_getstring(v, 2, &str);
		int len = sq_getsize(v, 2);
		len = self(v)->print(str, len);
		if (AutoLineFeed[0] != 0)
		{
			len += self(v)->print(AutoLineFeed, strlen(AutoLineFeed));
		}
		return push(v, len);
	}

	NB_FUNC(printf)
	{
		sq_tostring(v, 2);
		sq_replace(v, 2);
		SQChar* out;
		int len;
		if (SQ_FAILED(sqstd_format(v, 2, &len, &out)))
			return SQ_ERROR;

		len = self(v)->print(out, len);
		if (AutoLineFeed[0] != 0)
		{
			len += self(v)->print(AutoLineFeed, strlen(AutoLineFeed));
		}
		return push(v, len);
	}

	NB_FUNC(setPrintEncoding)
	{
		return sq_throwerror(v, "not implemented");
	}

	static char AutoLineFeed[3];

	NB_FUNC(setAutoLineFeed)
	{
		const char* str = getString(v, 2);

		for (int i=0; i<COUNT_OF(AutoLineFeed); ++i)
		{
			AutoLineFeed[i] = *str;
			if (*str++ == 0)
				break;
		}
		return 0;
	}
};

char NB_StreamWriter::AutoLineFeed[] = { '\n', 0 };

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::Base64Encoder, StreamWriter, incRefCount, decRefCount);

class NB_Base64Encoder : TNitClass<Base64Encoder>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(to),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(to: StreamWriter, bufSize=0)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(to)						{ return push(v, self(v)->getTo()); }

	NB_CONS()							{ setSelf(v, new Base64Encoder(get<StreamWriter>(v, 2), optInt(v, 3, 0))); return SQ_OK; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::Base64Decoder, StreamReader, incRefCount, decRefCount);

class NB_Base64Decoder : TNitClass<Base64Decoder>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(from),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(from: StreamReader, bufSize=0)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(from)					{ return push(v, self(v)->getFrom()); }

	NB_CONS()							{ setSelf(v, new Base64Decoder(get<StreamReader>(v, 2), optInt(v, 3, 0))); return SQ_OK; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::CalcCRC32Writer, StreamWriter, incRefCount, decRefCount);

class NB_CalcCRC32Writer : TNitClass<CalcCRC32Writer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(value),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(value)					{ return push(v, self(v)->getValue()); }

	NB_CONS()							{ setSelf(v, new CalcCRC32Writer()); return SQ_OK; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::CopyReader, StreamReader, incRefCount, decRefCount);

class NB_CopyReader : TNitClass<CopyReader>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(from),
			PROP_ENTRY_R(to),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(from: StreamReader, to: StreamWriter)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(from)					{ return push(v, self(v)->getFrom()); }
	NB_PROP_GET(to)						{ return push(v, self(v)->getTo()); }

	NB_CONS()							{ setSelf(v, new CopyReader(get<StreamReader>(v, 2), get<StreamWriter>(v, 3))); return SQ_OK; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::ShadowWriter, StreamWriter, incRefCount, decRefCount);

class NB_ShadowWriter : TNitClass<ShadowWriter>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(target),
			PROP_ENTRY_R(shadow),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(target: StreamWriter, shadow: StreamWriter)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(target)					{ return push(v, self(v)->getTarget()); }
	NB_PROP_GET(shadow)					{ return push(v, self(v)->getShadow()); }

	NB_CONS()							{ setSelf(v, new ShadowWriter(get<StreamWriter>(v, 2), get<StreamWriter>(v, 3))); return SQ_OK; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::LogWriter, StreamWriter, incRefCount, decRefCount);

class NB_LogWriter : TNitClass<LogWriter>
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
			CONS_ENTRY_H(				"(tag=null, channel: LogChannel=null)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, new LogWriter(optString(v, 2, NULL), opt<LogChannel>(v, 3, NULL))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::HexDumpWriter, StreamWriter, incRefCount, decRefCount);

class NB_HexDumpWriter : TNitClass<HexDumpWriter>
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
			CONS_ENTRY_H(				"(column=16, target:StreamWriter=null)"),
			FUNC_ENTRY_H(hack,			"(target: object, size: int)\n"
										"(addr: int, size: int)\n"
										"(addr: userpointer, size: int)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, new HexDumpWriter(optInt(v, 2, 16), opt<StreamWriter>(v, 3, NULL))); return 0; }

	NB_FUNC(hack)
	{
#ifndef NIT_SHIPPING
		type* o = self(v);

		if (sq_gettype(v, 2) == OT_INSTANCE)
		{
			SQUserPointer up = NULL;
			sq_getinstanceup(v, 2, &up, NULL);

			o->seek((size_t)up);
			o->writeRaw(up, getInt(v, 3));
		}
		else if (sq_gettype(v, 2) == OT_USERPOINTER)
		{
			SQUserPointer up = NULL;
			sq_getuserpointer(v, 2, &up);

			o->seek((size_t)up);
			o->writeRaw(up, getInt(v, 3));
		}
		else 
		{
			SQUserPointer up = (SQUserPointer)getInt(v, 2);

			o->seek((size_t)up);
			o->writeRaw(up, getInt(v, 3));
		}

		o->flush();
#endif
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::ZStreamReader, StreamReader, incRefCount, decRefCount);

class NB_ZStreamReader : TNitClass<ZStreamReader>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(from),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(from: StreamReader, bufSize=0)"),
			FUNC_ENTRY_H(finish,		"(): int // returns adler32, purges instance"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(from)					{ return push(v, self(v)->getFrom()); }

	NB_CONS()							{ setSelf(v, new ZStreamReader(get<StreamReader>(v, 2), optInt(v, 3, 0))); return 0; }

	NB_FUNC(finish)						{ uint32 adler = self(v)->finish(); sq_purgeinstance(v, 1); return push(v, adler); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::ZStreamWriter, StreamWriter, incRefCount, decRefCount);

class NB_ZStreamWriter : TNitClass<ZStreamWriter>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(to),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(to: StreamWriter, moreSpeed=false, flushOnlyOnClose=true, bufSize=0)"),
			FUNC_ENTRY_H(finish,		"(): int // returns adler32, purges instance"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(to)						{ return push(v, self(v)->getTo()); }

	NB_CONS()							{ setSelf(v, new ZStreamWriter(get<StreamWriter>(v, 2), optBool(v, 3, false), optBool(v, 4, true), optInt(v, 5, 0))); return 0; }

	NB_FUNC(finish)						{ uint32 adler = self(v)->finish(); sq_purgeinstance(v, 1); return push(v, adler); }
};

////////////////////////////////////////////////////////////////////////////////

class NB_StreamPrinterArgItr : public StreamPrinter::IArgIterator
{
public:
	NB_StreamPrinterArgItr(HSQUIRRELVM v, int idx, int top)
		: v(v), index(idx), end(top)
	{
	}

	virtual bool						hasNext()								{ return index <= end; }
	virtual int							nextInt()								{ return NitBind::getInt(v, index++); }
	virtual int64						nextInt64()								{ return NitBind::getInt(v, index++); }
	virtual float						nextFloat()								{ return NitBind::getFloat(v, index++); }
	virtual double						nextDouble()							{ return NitBind::getFloat(v, index++); }

	virtual const char*					nextCStr()
	{
		if (NitBind::isString(v, index))
			return NitBind::getString(v, index++);

		sq_tostring(v, index++);
		const char* str;
		sq_getstring(v, -1, &str);
		temp.assign(str, sq_getsize(v, -1));
		sq_poptop(v);

		return temp.c_str();
	}

	HSQUIRRELVM v;
	int index;
	int end;
	String temp;
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::StreamPrinter, RefCounted, incRefCount, decRefCount);

class NB_StreamPrinter : TNitClass<StreamPrinter>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(writer),
			PROP_ENTRY	(compact),
			PROP_ENTRY	(indentStr),
			PROP_ENTRY	(newlineStr),
			PROP_ENTRY_R(indent),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(w: StreamWriter=null) // default to MemoryBuffer.Writer"),

			FUNC_ENTRY_H(incIndent,		"()"),
			FUNC_ENTRY_H(decIndent,		"()"),
			FUNC_ENTRY_H(newline,		"(): bool"),
			FUNC_ENTRY_H(doIndent,		"(): bool"),

			FUNC_ENTRY_H(putch,			"(unichar: int)"),
			FUNC_ENTRY_H(print,			"(str: string)"),
			FUNC_ENTRY_H(printf,		"(fmt: string, ...) // %z: escaped, %q: quoted"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(writer)					{ return push(v, self(v)->getWriter()); }
	NB_PROP_GET(compact)				{ return push(v, self(v)->isCompact()); }
	NB_PROP_GET(indentStr)				{ return push(v, self(v)->getIndentStr()); }
	NB_PROP_GET(newlineStr)				{ return push(v, self(v)->getNewlineStr()); }
	NB_PROP_GET(indent)					{ return push(v, self(v)->getIndent()); }

	NB_PROP_SET(compact)				{ self(v)->setCompact(getBool(v, 2)); return 0; }
	NB_PROP_SET(indentStr)				{ self(v)->setIndentStr(getString(v, 2)); return 0; }
	NB_PROP_SET(newlineStr)				{ self(v)->setNewlineStr(getString(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, new StreamPrinter(opt<StreamWriter>(v, 2, NULL))); return SQ_OK; }

	NB_FUNC(incIndent)					{ self(v)->incIndent(); return 0; }
	NB_FUNC(decIndent)					{ self(v)->decIndent(); return 0; }
	NB_FUNC(newline)					{ return push(v, self(v)->newline()); }
	NB_FUNC(doIndent)					{ return push(v, self(v)->doIndent()); }

	NB_FUNC(putch)						{ self(v)->putch(getInt(v, 2)); return 0; }

	NB_FUNC(print)
	{
		const char* str;

		sq_tostring(v, 2);
		sq_replace(v, 2);
		sq_getstring(v, 2, &str);
		int len = sq_getsize(v, 2);

		self(v)->print(str, len);
		sq_poptop(v);
		return 0;
	}

	NB_FUNC(printf)
	{
		NB_StreamPrinterArgItr itr(v, 3, sq_gettop(v));
		self(v)->aprintf(getString(v, 2), &itr); 
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::JsonPrinter, StreamPrinter, incRefCount, decRefCount);

class NB_JsonPrinter : TNitClass<JsonPrinter>
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
			CONS_ENTRY_H(				"(w: StreamWriter=null) // default to MemoryBuffer.Writer"),
			FUNC_ENTRY_H(beginObject,	"()"),
			FUNC_ENTRY_H(endObject,		"()"),
			FUNC_ENTRY_H(beginArray,	"()"),
			FUNC_ENTRY_H(endArray,		"()"),
			FUNC_ENTRY_H(key,			"(key: string)"),
			FUNC_ENTRY_H(value,			"(value: object)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, new JsonPrinter(opt<StreamWriter>(v, 2, NULL))); return SQ_OK; }

	NB_FUNC(beginObject)				{ self(v)->beginObject(); return 0; }
	NB_FUNC(endObject)					{ self(v)->endObject(); return 0; }
	NB_FUNC(beginArray)					{ self(v)->beginArray(); return 0; }
	NB_FUNC(endArray)					{ self(v)->endArray(); return 0; }

	static SQRESULT EmitKey(HSQUIRRELVM v, int idx, type* self)
	{
		const char* str;

		if (!isString(v, 2))
			return sq_throwerror(v, "key must be a string");
		sq_getstring(v, 2, &str);
		self->key(str, sq_getsize(v, 2)); 

		return SQ_OK;
	}

	static SQRESULT EmitValue(HSQUIRRELVM v, int idx, type* self)
	{
		const char* str;

		switch (sq_gettype(v, idx))
		{
		case OT_STRING:					sq_getstring(v, idx, &str); self->value(str, sq_getsize(v, idx)); break;
		case OT_INTEGER:				self->value(getInt(v, idx)); break;
		case OT_FLOAT:					self->value(getFloat(v, idx)); break;
		case OT_BOOL:					self->value(getBool(v, idx)); break;
		case OT_NULL:					self->valueNull(); break;

		case OT_TABLE:					
			self->beginObject();
			for (NitIterator itr(v, idx); itr.hasNext(); itr.next())
			{
				SQRESULT sr;
				
				sr = EmitKey(v, itr.keyIndex(), self);
				if (SQ_FAILED(sr)) return sr;
				sr = EmitValue(v, itr.valueIndex(), self);
				if (SQ_FAILED(sr)) return sr;
			}
			self->endObject();
			break;

		case OT_ARRAY:
			self->beginArray();
			for (NitIterator itr(v, idx); itr.hasNext(); itr.next())
			{
				SQRESULT sr;
				sr = EmitValue(v, itr.valueIndex(), self);
				if (SQ_FAILED(sr)) return sr;
			}
			self->endArray();
			break;

		default:
			return sq_throwerror(v, "invalid value type");
		}

		return SQ_OK;
	}

	NB_FUNC(key)						{ return EmitKey(v, 2, self(v)); }
	NB_FUNC(value)						{ return EmitValue(v, 2, self(v)); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::XmlPrinter, StreamPrinter, incRefCount, decRefCount);

class NB_XMLPrinter : TNitClass<XmlPrinter>
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
			CONS_ENTRY_H(				"(w: StreamWriter=null) // default to MemoryBuffer.Writer"),
			FUNC_ENTRY_H(beginElem,		"(elemName: string, attrs: table=null)"),
			FUNC_ENTRY_H(endElem,		"()"),
			FUNC_ENTRY_H(elem,			"(name: string, attrs: table=null, text: string=null) // == BeginElem(name, attrs), Text(text), EndElem()"),
			FUNC_ENTRY_H(attr,			"(name: string, value: string)\n"
										"(attrs: table)"),
			FUNC_ENTRY_H(text,			"(text: string)"),
			FUNC_ENTRY_H(comment,		"(fmt, ...)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, new XmlPrinter(opt<StreamWriter>(v, 2, NULL))); return SQ_OK; }

	static SQRESULT CallAttrWithTable(HSQUIRRELVM v, int tableIdx, type* self)
	{
		for (NitIterator itr(v, tableIdx); itr.hasNext(); itr.next())
		{
			const char* key;
			const char* value;
			sq_tostring(v, itr.keyIndex());
			sq_getstring(v, -1, &key);
			sq_tostring(v, itr.valueIndex());
			sq_getstring(v, -1, &value);

			self->attr(key, value);
			sq_pop(v, 2);
		}
		return SQ_OK;
	}

	static SQRESULT CallText(HSQUIRRELVM v, int textIdx, type* self)
	{
		const char* str;
		sq_tostring(v, textIdx);
		sq_getstring(v, -1, &str);
		self->text(str, sq_getsize(v, -1));
		sq_poptop(v);
		return SQ_OK;
	}

	NB_FUNC(beginElem)
	{ 
		type* o = self(v);
		o->beginElem(getString(v, 2)); 

		if (!isNone(v, 3) && sq_gettype(v, 3) == OT_TABLE)
		{
			SQRESULT sr = CallAttrWithTable(v, 3, o);
			if (SQ_FAILED(sr)) return sr;
		}

		return 0; 
	}

	NB_FUNC(endElem)					{ self(v)->endElem(); return 0; }

	NB_FUNC(attr)						
	{ 
		type* o = self(v);
		if (sq_gettype(v, 2) == OT_TABLE)
		{
			SQRESULT sr = CallAttrWithTable(v, 2, o);
		}
		else
		{
			o->attr(getString(v, 2), getString(v, 3));
		}
		return 0; 
	}

	NB_FUNC(elem)
	{
		type* o = self(v);
		o->beginElem(getString(v, 2));

		bool hasAttrs = !isNone(v, 3) && sq_gettype(v, 3) == OT_TABLE;
		bool hasText = !isNone(v, 4);

		if (hasAttrs)
		{
			SQRESULT sr = CallAttrWithTable(v, 3, o);
			if (SQ_FAILED(sr)) return sr;
		}

		if (hasText)
		{
			SQRESULT sr = CallText(v, 4, o);
			if (SQ_FAILED(sr)) return sr;
		}
		
		o->endElem();

		return 0;
	}

	NB_FUNC(text)
	{ 
		return CallText(v, 2, self(v));
	}

	NB_FUNC(comment)					
	{ 
		NB_StreamPrinterArgItr itr(v, 3, sq_gettop(v)); 
		self(v)->commentA(getString(v, 2), &itr); 
		return 0; 
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::MemoryBuffer, RefCounted, incRefCount, decRefCount);

class NB_MemoryBuffer : TNitClass<MemoryBuffer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(blockSize),
			PROP_ENTRY_R(size),
			PROP_ENTRY_R(numBlocks),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(blockSize=0)"
			"\n"						"(reader: StreamReader, blockSize=0)"
			"\n"						"(string, blockSize=0)"),
			FUNC_ENTRY_H(reserve,		"(size: int)"),
			FUNC_ENTRY_H(resize,		"(size: int)"),
			FUNC_ENTRY_H(clear,			"(): this"),
			FUNC_ENTRY_H(load,			"(reader: StreamReader, pos=0, size=0): this"),
			FUNC_ENTRY_H(save,			"(writer: StreamWriter, pos=0, size=0): this"),
			FUNC_ENTRY_H(clone,			"(blockSize=0): MemoryBuffer"),
			FUNC_ENTRY_H(calcCrc32,		"(): int"),
			FUNC_ENTRY_H(calcAdler32,	"(): int"),
			FUNC_ENTRY_H(compress,		"(moreSpeed=false): this"),
			FUNC_ENTRY_H(uncompress,	"(): this"),
			FUNC_ENTRY_H(toString,		"(utf16=false): string"
			"\n"						"(pos: int, size=0, utf16=false): string"),
			FUNC_ENTRY_H(pushBack,		"(string): this"),
			FUNC_ENTRY_H(popFront,		"(size: int): this"),
			FUNC_ENTRY_H(hexDump,		"()"),
			FUNC_ENTRY	(_clone),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(blockSize)				{ return push(v, self(v)->getBlockSize()); }
	NB_PROP_GET(size)					{ return push(v, self(v)->getSize()); }
	NB_PROP_GET(numBlocks)				{ return push(v, self(v)->getNumBlocks()); }

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new MemoryBuffer());
		else if (isInt(v, 2))
			setSelf(v, new MemoryBuffer(getInt(v, 2)));
		else if (isString(v, 2))
		{
 			const char* str;
 			sq_getstring(v, 2, &str);
 			setSelf(v, new MemoryBuffer(str, sq_getsize(v, 2), optInt(v, 3, 0)));
		}
		else
			setSelf(v, new MemoryBuffer(get<StreamReader>(v, 2), optInt(v, 3, 0)));
		return 0;
	}

	NB_FUNC(reserve)					{ self(v)->reserve(getInt(v, 2)); return 0; }
	NB_FUNC(resize)						{ self(v)->resize(getInt(v, 2)); return 0; }
	NB_FUNC(clear)						{ self(v)->clear(); sq_push(v, 1); return 1; }

	NB_FUNC(clone)						{ return push(v, self(v)->clone(optInt(v, 2, 0))); }

	NB_FUNC(load)						{ self(v)->load(get<StreamReader>(v, 2), optInt(v, 3, 0), optInt(v, 4, 0)); sq_push(v, 1); return 1; }
	NB_FUNC(save)						{ self(v)->save(get<StreamWriter>(v, 2), optInt(v, 3, 0), optInt(v, 4, 0)); sq_push(v, 1); return 1; }

	NB_FUNC(toString)
	{
		if (isNone(v, 2) || isBool(v, 2))
			return push(v, self(v)->toString(optBool(v, 2, false)));
		else
			return push(v, self(v)->toString(getInt(v, 2), optInt(v, 3, 0), optBool(v, 4, false)));
	}

	NB_FUNC(pushBack)					{ self(v)->pushBack(getString(v, 2)); sq_push(v, 1); return 1;  }
	NB_FUNC(popFront)					{ self(v)->popFront(getInt(v, 2)); sq_push(v, 1); return 1;  }
	NB_FUNC(hexDump)					{ self(v)->hexDump(); return 0; }

	NB_FUNC(compress)					{ self(v)->compress(optBool(v, 2, false)); sq_push(v, 1); return 1; }
	NB_FUNC(uncompress)					{ self(v)->uncompress(); sq_push(v, 1); return 1;  }
	NB_FUNC(calcCrc32)					{ return push(v, self(v)->calcCrc32()); }
	NB_FUNC(calcAdler32)				{ return push(v, self(v)->calcAdler32()); }

	NB_FUNC(_clone)						{ return push(v, self(v)->clone()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::MemorySource, StreamSource, incRefCount, decRefCount);

class NB_MemorySource : TNitClass<MemorySource>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(locator),
			PROP_ENTRY	(name),
			PROP_ENTRY	(contentType),
			PROP_ENTRY	(timestamp),
			PROP_ENTRY_R(buffer),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(name: string, buffer: MemoryBuffer=null) // creates new buffer when null\n"
										"(real: StreamSource, buffer: MemoryBuffer=null) // creates new buffer when null"),
			FUNC_ENTRY_H(newReader,		"(): StreamReader // equals to Open()"),
			FUNC_ENTRY_H(newWriter,		"(): StreamWriter"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(locator)				{ return push(v, self(v)->getLocator()); }
	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }
	NB_PROP_GET(contentType)			{ return push(v, self(v)->getContentType()); }
	NB_PROP_GET(timestamp)				{ return push(v, self(v)->getTimestamp()); }
	NB_PROP_GET(buffer)					{ return push(v, self(v)->getBuffer()); }

	NB_PROP_SET(locator)				{ self(v)->setLocator(get<StreamLocator>(v, 2)); return 0; }
	NB_PROP_SET(name)					{ self(v)->setName(getString(v, 2)); return 0; }
	NB_PROP_SET(contentType)			{ self(v)->setContentType(*get<ContentType>(v, 2)); return 0; }
	NB_PROP_SET(timestamp)				{ self(v)->setTimestamp(*get<Timestamp>(v, 2)); return 0; }

	NB_CONS()
	{ 
		if (isString(v, 2))
			setSelf(v, new MemorySource(getString(v, 2), opt<MemoryBuffer>(v, 3, NULL)));
		else
			setSelf(v, new MemorySource(get<StreamSource>(v, 2), opt<MemoryBuffer>(v, 3, NULL)));
		return 0;
	}

	NB_FUNC(newReader)					{ return push(v, self(v)->newReader()); }
	NB_FUNC(newWriter)					{ return push(v, self(v)->newWriter()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::MemoryBuffer::Reader, StreamReader, incRefCount, decRefCount);

class NB_MemoryBufferReader : TNitClass<MemoryBuffer::Reader>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(target),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(buffer: MemoryBuffer, source: StreamSource)\n"
										"(target: StreamReader, blockSize=0)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(target)					{ return push(v, self(v)->getTarget()); }

	NB_CONS()
	{
		if (is<MemoryBuffer>(v, 2))
			setSelf(v, new MemoryBuffer::Reader(get<MemoryBuffer>(v, 2), get<StreamSource>(v, 3)));
		else
			setSelf(v, new MemoryBuffer::Reader(get<StreamReader>(v, 2), optInt(v, 3, 0)));
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::MemoryBuffer::Writer, StreamWriter, incRefCount, decRefCount);

class NB_MemoryBufferWriter : TNitClass<MemoryBuffer::Writer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(target),
			PROP_ENTRY_R(buffer),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(blockSize=0)\n"
										"(buffer: MemoryBuffer, source: StreamSource)\n"
										"(target: StreamWriter, blockSize=0)"),
			FUNC_ENTRY_H(toString,		"(): string"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(target)					{ return push(v, self(v)->getTarget()); }
	NB_PROP_GET(buffer)					{ return push(v, self(v)->getBuffer()); }

	NB_CONS()
	{
		if (isNone(v, 2) || isInt(v, 2))
			setSelf(v, new MemoryBuffer::Writer(optInt(v, 2, 0)));
		else if (is<MemoryBuffer>(v, 2))
			setSelf(v, new MemoryBuffer::Writer(get<MemoryBuffer>(v, 2), get<StreamSource>(v, 3)));
		else
			setSelf(v, new MemoryBuffer::Writer(get<StreamWriter>(v, 2), optInt(v, 3, 0)));
		return 0;
	}

	NB_FUNC(toString)					{ return push(v, self(v)->getBuffer()->toString()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::Archive, StreamLocator, incRefCount, decRefCount);

class NB_Archive : TNitClass<Archive>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(readOnly),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(load,			"()"),
			FUNC_ENTRY_H(unload,		"() // purges instance"),
			FUNC_ENTRY_H(open,			"(streamName): StreamReader"),
			FUNC_ENTRY_H(create,		"(streamName): StreamWriter"),
			FUNC_ENTRY_H(modify,		"(streamName): StreamWriter"),
			FUNC_ENTRY_H(remove,		"(pattern)"),
			FUNC_ENTRY_H(rename,		"(streamName, newName)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(readOnly)				{ return push(v, self(v)->isReadOnly()); }
	
	NB_FUNC(load)						{ self(v)->load(); return 0; }
	NB_FUNC(unload)						{ self(v)->unload(); sq_purgeinstance(v, 1); return 0; }

	NB_FUNC(open)						{ return push(v, self(v)->open(getString(v, 2))); }
	NB_FUNC(create)						{ return push(v, self(v)->create(getString(v, 2))); }
	NB_FUNC(modify)						{ return push(v, self(v)->modify(getString(v, 2))); }
	NB_FUNC(remove)						{ self(v)->remove(getString(v, 2)); return 0; }
	NB_FUNC(rename)						{ self(v)->rename(getString(v, 2), getString(v, 3)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::CustomArchive, Archive, incRefCount, decRefCount);

class NB_CustomArchive : TNitClass<CustomArchive>
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
			CONS_ENTRY_H(				"(name: string)"),
			FUNC_ENTRY_H(addLocal,		"(name: string, source: StreamSource)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ sq_setinstanceup(v, 1, new CustomArchive(getString(v, 2))); return 0; }
	NB_FUNC(addLocal)					{ self(v)->addLocal(getString(v, 2), get<StreamSource>(v, 3)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_RAW_PTR(NIT_API, nit::FileUtil, NULL);

class NB_FileUtil : TNitClass<FileUtil>
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
			FUNC_ENTRY_H(hasFileSystem,	"(): bool"),
			FUNC_ENTRY_H(isReadOnly,	"(): bool"),
			FUNC_ENTRY_H(isCaseSensitive, "(): bool"),

			FUNC_ENTRY_H(exists,		"(path: string): bool"),
			FUNC_ENTRY_H(isDirectory,	"(path: string): bool"),
			FUNC_ENTRY_H(isAbsolutePath,"(path: string): bool"),

			FUNC_ENTRY_H(createDir,		"(path: string, throwIfExists=false)"),
			FUNC_ENTRY_H(writeFile,		"(path: string, buf: MemoryBuffer)\n"
										"(path: string, text: string)\n"
										"(path: string, from: StreamReader, bufSize=4096)"),
			FUNC_ENTRY_H(readFile,		"(path: string, into: MemoryBuffer)\n"
										"(path: string): string\n"
										"(path: string, into: StreamWriter, bufSize=4096)"),
			FUNC_ENTRY_H(remove,		"(patterm: string)"),
			FUNC_ENTRY_H(move,			"(from, to: string)"),
			FUNC_ENTRY_H(copy,			"(from, to: string)"),
			NULL
		};

		bind(v, props, funcs);
	};

	NB_FUNC(hasFileSystem)				{ return push(v, FileUtil::hasFileSystem()); }
	NB_FUNC(isReadOnly)					{ return push(v, FileUtil::isReadOnly()); }
	NB_FUNC(isCaseSensitive)			{ return push(v, FileUtil::isCaseSensitive()); }

	NB_FUNC(exists)						{ return push(v, FileUtil::exists(getString(v, 2))); }
	NB_FUNC(isDirectory)				{ return push(v, FileUtil::exists(getString(v, 2))); }
	NB_FUNC(isAbsolutePath)				{ return push(v, FileUtil::isAbsolutePath(getString(v, 2))); }

	NB_FUNC(createDir)					{ FileUtil::createDir(getString(v, 2), optBool(v, 3, false)); return 0; }

	NB_FUNC(writeFile)
	{ 
		if (is<MemoryBuffer>(v, 3))
			FileUtil::writeFile(getString(v, 2), get<MemoryBuffer>(v, 3)); 
		else if (is<StreamReader>(v, 3))
			FileUtil::writeFile(getString(v, 2), get<StreamReader>(v, 3), optInt(v, 4, 4096));
		else
		{
			const char* str = getString(v, 3);
			size_t len = sq_getsize(v, 3);
			FileUtil::writeFile(getString(v, 2), str, len);
		}

		return 0; 
	}

	NB_FUNC(readFile)
	{
		if (is<MemoryBuffer>(v, 3))
			FileUtil::readFile(getString(v, 2), get<MemoryBuffer>(v, 3));
		else if (is<StreamWriter>(v, 3))
			FileUtil::readFile(getString(v, 2), get<StreamWriter>(v, 3), optInt(v, 4, 4096));
		else
		{
			String buf;
			FileUtil::readFile(getString(v, 2), buf);
			return push(v, buf);
		}
		return 0;
	}

	NB_FUNC(remove)						{ FileUtil::remove(getString(v, 2)); return 0; }
	NB_FUNC(move)						{ FileUtil::move(getString(v, 2), getString(v, 3)); return 0; }
	NB_FUNC(copy)						{ FileUtil::copy(getString(v, 2), getString(v, 3)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::FileLocator, Archive, incRefCount, decRefCount);

class NB_FileLocator : TNitClass<FileLocator>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(baseUrl),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(name, path: string, readOnly=true, findRecursive=false)"
			"\n"						"(path: string, readOnly=true, findRecursive=false)"),
			FUNC_ENTRY_H(findFiles,		"(pattern, recursive=false): StreamSource[]"),
			FUNC_ENTRY_H(findDirs,		"(pattern): string[]"),
			FUNC_ENTRY_H(normalizePath,	"(path): string"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(baseUrl)				{ return push(v, self(v)->getBaseUrl()); }

	NB_CONS()							
	{ 
		if (!isNone(v, 3) && isString(v, 3))
			setSelf(v, new FileLocator(getString(v, 2), getString(v, 3), optBool(v, 4, true), optBool(v, 5, false)));
		else
			setSelf(v, new FileLocator(getString(v, 2), optBool(v, 3, true), optBool(v, 4, false)));
		return SQ_OK;
	}

	NB_FUNC(normalizePath)				{ return push(v, self(v)->normalizePath(getString(v, 2))); }

	NB_FUNC(findFiles)
	{
		StreamSourceMap sl;
		self(v)->findFiles(getString(v, 2), sl, optBool(v, 3, false));

		sq_newarray(v, 0);
		for (StreamSourceMap::iterator itr = sl.begin(), end = sl.end(); itr != end; ++itr)
			arrayAppend(v, -1, itr->second.get());
		return 1;
	}

	NB_FUNC(findDirs)
	{
		StringVector dirs;
		self(v)->findDirs(getString(v, 2), dirs);

		sq_newarray(v, 0);
		for (uint i=0; i<dirs.size(); ++i)
			arrayAppend(v, -1, dirs[i]);
		return 1;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::File, StreamSource, incRefCount, decRefCount);

class NB_File : TNitClass<File>
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
			FUNC_ENTRY_H(openRange,		"(offset: int, size: int, source: StreamSource = null): StreamReader"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_FUNC(openRange)					{ return push(v, self(v)->openRange(getInt(v, 2), getInt(v, 3), opt<StreamSource>(v, 4, NULL))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::Settings, RefCounted, incRefCount, decRefCount);

class NB_Settings : TNitClass<Settings>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(name),
			PROP_ENTRY_R(source),
			PROP_ENTRY_R(parent),
			PROP_ENTRY_R(root),
			PROP_ENTRY_R(path),
			PROP_ENTRY	(base),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(source: StreamSource=null)"),
			FUNC_ENTRY_H(load,			"[class] (source: StreamSource, treatAs=ContentType.UNKNOWN): Settings"),

			FUNC_ENTRY_H(clear,			"(recursive=true)"),
			FUNC_ENTRY_H(has,			"(keyPath: string): bool"),
			FUNC_ENTRY_H(get,			"(keyPath: string, defaultVal=\"\", warnIfNotFound=true): string"),
			FUNC_ENTRY_H(set,			"(keyPath: string, value: string) // same as Remove then Add"),
			FUNC_ENTRY_H(add,			"(keyPath: string, value: string)"),
			FUNC_ENTRY_H(remove,		"(keyPath: string)"),
			FUNC_ENTRY_H(find,			"(pattern: string): string[]"),
			FUNC_ENTRY_H(findKeys,		"(pattern: string): string[]"),

			FUNC_ENTRY_H(getSection,	"(path: string): Settings"),
			FUNC_ENTRY_H(addSection,	"(path: string, section: Settings)"),
			FUNC_ENTRY_H(removeSection,	"(name: string) // can't use path"),
			FUNC_ENTRY_H(findSections,	"(pattern: string, recursive=false): Settings[]"),

			FUNC_ENTRY_H(expand,		"(aliased: string, throwEx=true): string // replace \"$(alias)\" to its value"),

			FUNC_ENTRY_H(dump,			"()"),

			FUNC_ENTRY_H(saveCfg,		"(writer: StreamWriter)"),
			FUNC_ENTRY_H(saveJson,		"(printer: JSONPrinter)"),

			FUNC_ENTRY	(_tostring),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }
	NB_PROP_GET(source)					{ return push(v, self(v)->getSource()); }
	NB_PROP_GET(parent)					{ return push(v, self(v)->getParent()); }
	NB_PROP_GET(root)					{ return push(v, self(v)->getRoot()); }
	NB_PROP_GET(path)					{ return push(v, self(v)->getPath()); }
	NB_PROP_GET(base)					{ return push(v, self(v)->getBase()); }

	NB_PROP_SET(base)					{ self(v)->setBase(opt<Settings>(v, 2, NULL)); return 0; }

	NB_CONS()							{ setSelf(v, new Settings(opt<StreamSource>(v, 2, NULL))); return 0; }

	NB_FUNC(load)						{ return push(v, Settings::load(get<StreamSource>(v, 2), *opt<ContentType>(v, 3, ContentType::UNKNOWN))); }

	NB_FUNC(clear)						{ self(v)->clear(); return 0; }
	NB_FUNC(has)						{ return push(v, self(v)->has(getString(v, 2))); }
	NB_FUNC(get)						{ return push(v, self(v)->get(getString(v, 2), optString(v, 3, ""), optBool(v, 4, true))); }
	NB_FUNC(set)						{ self(v)->set(getString(v, 2), getString(v, 3)); return 0; }
	NB_FUNC(add)						{ self(v)->add(getString(v, 2), getString(v, 3)); return 0; }
	NB_FUNC(remove)						{ self(v)->remove(getString(v, 2)); return 0; }

	NB_FUNC(find)
	{
		StringVector values;
		self(v)->find(getString(v, 2), values);
		sq_newarray(v, 0);
		for (uint i=0; i<values.size(); ++i)
			arrayAppend(v, -1, values[i]);
		return 1;
	}

	NB_FUNC(findKeys)
	{
		StringVector values;
		self(v)->findKeys(getString(v, 2), values);
		sq_newarray(v, 0);
		for (uint i=0; i<values.size(); ++i)
			arrayAppend(v, -1, values[i]);
		return 1;
	}

	NB_FUNC(getSection)					{ return push(v, self(v)->getSection(getString(v, 2))); }
	NB_FUNC(addSection)					{ self(v)->addSection(getString(v, 2), get<Settings>(v, 3)); return 0; }
	NB_FUNC(removeSection)				{ self(v)->removeSection(getString(v, 2)); return 0; }
	
	NB_FUNC(findSections)
	{
		SettingsList sections;
		self(v)->findSections(getString(v, 2), sections, optBool(v, 3, false));
		sq_newarray(v, 0);
		for (uint i=0; i<sections.size(); ++i)
			arrayAppend(v, -1, sections[i].get());
		return 1;
	}

	NB_FUNC(saveCfg)					{ self(v)->saveCfg(get<StreamWriter>(v, 2)); return 0; }
	NB_FUNC(saveJson)					{ self(v)->saveJson(get<JsonPrinter>(v, 2)); return 0; }

	NB_FUNC(expand)						{ return push(v, self(v)->expand(getString(v, 2), optBool(v, 3, true))); }

	NB_FUNC(_tostring)
	{ 
		type* o = self(v);
		return pushFmt(v, "(Settings: '%s' -> %08X)", o->getPath().c_str(), o);
	}

	NB_FUNC(dump)						{ self(v)->dump(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::CmdLineParser, RefCounted, incRefCount, decRefCount);

class NB_CmdLineParser : TNitClass<CmdLineParser>
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
			CONS_ENTRY_H(						"()"),
			FUNC_ENTRY_H(allowUnknownOptions,	"(flag: bool)"),
			FUNC_ENTRY_H(allowUnknownParams,	"(flag: bool)"),
			FUNC_ENTRY_H(allowOmitMandatory,	"(flag: bool)"),
			FUNC_ENTRY_H(addOption,				"(name, abbr, desc: string, optional=true, multiple=false)"),
			FUNC_ENTRY_H(addSwitch,				"(name, abbr, desc: string, optional=true)"),
			FUNC_ENTRY_H(addParam,				"(name, desc: string, variadic=false)"),
			FUNC_ENTRY_H(addSection,			"(shellSection: Settings=null)"),
			FUNC_ENTRY_H(makeUsage,				"(appname, logo, text: string): string"),
			FUNC_ENTRY_H(showUsage,				"(appname=\"\", logo=\"\", text=\"\"): string"),
			FUNC_ENTRY_H(split,					"[class] (cmdline: string): string[]"),
			FUNC_ENTRY_H(parse,					"(settings: Settings, cmdline: string, throwEx=false, begin=0): bool\n"
												"(settings: Settings, args: [], throwEx=false, begin=0): bool\n"
												"(settings: Settings, pf: Platform, throwEx=false, begin=1): bool"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, new CmdLineParser()); return SQ_OK; }

	NB_FUNC(allowUnknownOptions)		{ self(v)->allowUnknownOptions(getBool(v, 2)); return 0; }
	NB_FUNC(allowUnknownParams)			{ self(v)->allowUnknownParams(getBool(v, 2)); return 0; }
	NB_FUNC(allowOmitMandatory)			{ self(v)->allowOmitMandatory(getBool(v, 2)); return 0; }

	NB_FUNC(addOption)					{ self(v)->addOption(getString(v, 2), getString(v, 3), getString(v, 4), optBool(v, 5, true), optBool(v, 6, false)); return 0; }
	NB_FUNC(addSwitch)					{ self(v)->addSwitch(getString(v, 2), getString(v, 3), getString(v, 4), optBool(v, 5, true)); return 0; }
	NB_FUNC(addParam)					{ self(v)->addParam(getString(v, 2), getString(v, 3), optBool(v, 4, false)); return 0; }
	NB_FUNC(addSection)					{ self(v)->addSection(opt<Settings>(v, 2, NULL)); return 0; }
	NB_FUNC(showUsage)					{ self(v)->showUsage(optString(v, 2, ""), optString(v, 3, ""), optString(v, 4, "")); return 0; }
	NB_FUNC(makeUsage)					{ return push(v, self(v)->makeUsage(getString(v, 2), getString(v, 3), getString(v, 4))); }

	NB_FUNC(split)
	{
		StringVector args;
		CmdLineParser::split(getString(v, 2), args);
		sq_newarray(v, 0);
		for (uint i=0; i<args.size(); ++i)
			arrayAppend(v, -1, args[i]);
		return 1;
	}

	NB_FUNC(parse)
	{
		if (!isNone(v, 3) && sq_gettype(v, 3) == OT_ARRAY)
		{
			StringVector args;
			for (NitIterator itr(v, 3); itr.hasNext(); itr.next())
			{
				if (sq_gettype(v, itr.valueIndex()) == OT_STRING)
					args.push_back(getString(v, itr.valueIndex()));
				else
				{
					sq_tostring(v, itr.valueIndex());
					args.push_back(getString(v, -1));
					sq_poptop(v);
				}
			}
			return push(v, self(v)->parse(get<Settings>(v, 2), args, optBool(v, 4, false), optInt(v, 5, 0)));
		}
		else if (is<NitRuntime>(v, 3))
			return push(v, self(v)->parse(get<Settings>(v, 2), get<NitRuntime>(v, 3), optBool(v, 4, false), optInt(v, 5, 1)));
		else
			return push(v, self(v)->parse(get<Settings>(v, 2), getString(v, 3), optBool(v, 4, false), optInt(v, 5, 0)));
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::EventAutomata, RefCounted, incRefCount, decRefCount);

class NB_EventAutomata : TNitClass<EventAutomata>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(state),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(state)					{ return push(v, self(v)->getState()); }
	NB_PROP_SET(state)					{ self(v)->setState(get<EventState>(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, new EventAutomata()); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::EventState, RefCounted, incRefCount, decRefCount);

class NB_EventState : TNitClass<EventState>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(name),
			PROP_ENTRY_R(automata),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }
	NB_PROP_GET(automata)				{ return push(v, self(v)->getAutomata()); }

	NB_PROP_SET(name)					{ self(v)->setName(getString(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

typedef TScriptState<EventState> ScriptEventState;

NB_TYPE_REF(NIT_API, nit::ScriptEventState, nit::EventState, IScriptRef::scriptIncRef, IScriptRef::scriptDecRef);

class NB_ScriptEventState : TNitClass<ScriptEventState>
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
 			CONS_ENTRY	(),
 			FUNC_ENTRY	(_inherited),
			FUNC_ENTRY	(_tostring),
			NULL
		};

		bind(v, props, funcs);
		addInterface<IScriptEventSink>(v);
		addInterface<IScriptRef>(v);
	}

 	NB_FUNC(_inherited)					{ return SQ_OK; }

	NB_FUNC(_tostring)			
	{
		type* o = self(v);
		SQInteger top = sq_gettop(v);
		const char* clsname = "???";
		push(v, "_classname");
		sq_get(v, 1);
		sq_getstring(v, -1, &clsname);
		sq_settop(v, top);
		return pushFmt(v, "(%s: %s -> %08X)", o->getName().c_str(), clsname, o);
	}

	NB_CONS()	
	{
		ScriptEventState* obj = new ScriptEventState(new ScriptPeer(v, 1));

		ScriptRuntime::getRuntime(v)->retain(obj);

		sq_setinstanceup(v, 1, obj);
		
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_RAW_PTR(NIT_API, nit::NitRuntime, NULL);

class NB_NitRuntime : TNitClass<NitRuntime>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(infoEnabled),
			PROP_ENTRY	(alertEnabled),
			PROP_ENTRY_R(arguments),
			PROP_ENTRY_R(debugServer),

			PROP_ENTRY_R(platform),
			PROP_ENTRY_R(build),
			PROP_ENTRY_R(deviceName),
			PROP_ENTRY_R(deviceModel),
			PROP_ENTRY_R(osVersion),

			PROP_ENTRY_R(language),
			PROP_ENTRY_R(country),

			PROP_ENTRY_R(appPath),
			PROP_ENTRY_R(appDataPath),
			PROP_ENTRY_R(appCachePath),
			PROP_ENTRY_R(userDataPath),
			PROP_ENTRY_R(userCachePath),
			PROP_ENTRY_R(sysTempPath),

			PROP_ENTRY_R(netAvailable),
			PROP_ENTRY_R(hostName),
			PROP_ENTRY_R(mainIp),
			PROP_ENTRY_R(ipAddrs),

			PROP_ENTRY_R(config),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(info,			"(title: string, message: string)"),
			FUNC_ENTRY_H(alert,			"(title: string, message: string)"),
			FUNC_ENTRY_H(openArchive,	"(name: string, url: string): Archive"),
			FUNC_ENTRY_H(channel,		"(): EventChannel"),
			FUNC_ENTRY_H(execute,		"(cmdline: string, in: StreamWriter=null, out: StreamWriter=null, err: StreamWriter=null): int"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(infoEnabled)			{ return push(v, self(v)->isInfoEnabled()); }
	NB_PROP_GET(alertEnabled)			{ return push(v, self(v)->isAlertEnabled()); }
	NB_PROP_GET(debugServer)			{ return push(v, self(v)->getDebugServer()); }

	NB_PROP_GET(arguments)
	{
		NitRuntime* rt = self(v);
		const std::vector<std::string>& args = rt->getArguments();
		sq_newarray(v, 0);
		for (uint i=0; i < args.size(); ++i)
		{
			arrayAppend(v, -1, args[i].c_str());
		}
		return 1;
	}

	NB_PROP_GET(platform)				{ return push(v, self(v)->getPlatform()); }
	NB_PROP_GET(build)					{ return push(v, self(v)->getBuild()); }
	NB_PROP_GET(deviceName)				{ return push(v, self(v)->getDeviceName()); }
	NB_PROP_GET(deviceModel)			{ return push(v, self(v)->getDeviceModel()); }
	NB_PROP_GET(osVersion)				{ return push(v, self(v)->getOsVersion()); }

	NB_PROP_GET(language)				{ return push(v, self(v)->getLanguage()); }
	NB_PROP_GET(country)				{ return push(v, self(v)->getCountry()); }

	NB_PROP_GET(appPath)				{ return push(v, self(v)->getAppPath()); }
	NB_PROP_GET(appDataPath)			{ return push(v, self(v)->getAppDataPath()); }
	NB_PROP_GET(appCachePath)			{ return push(v, self(v)->getAppCachePath()); }
	NB_PROP_GET(userDataPath)			{ return push(v, self(v)->getUserDataPath()); }
	NB_PROP_GET(userCachePath)			{ return push(v, self(v)->getUserCachePath()); }
	NB_PROP_GET(sysTempPath)			{ return push(v, self(v)->getSysTempPath()); }

	NB_PROP_GET(netAvailable)			{ return push(v, self(v)->isNetAvailable()); }
	NB_PROP_GET(hostName)				{ return push(v, self(v)->getHostName()); }
	NB_PROP_GET(mainIp)					{ return push(v, self(v)->getMainIp().c_str()); }

	NB_PROP_GET(ipAddrs)
	{
		NitRuntime* rt = self(v);
		sq_newtable(v);
		const NitRuntime::IPAddressMap& addrs = rt->getIpAddrs();
		for (NitRuntime::IPAddressMap::const_iterator itr = addrs.begin(), end = addrs.end(); itr != end; ++itr)
		{
			NitBind::newSlot(v, -1, itr->first.c_str(), itr->second.c_str());
		}
		return 1;
	}

	NB_PROP_GET(config)					{ return push(v, self(v)->getConfig()); }

	NB_PROP_SET(infoEnabled)			{ self(v)->setInfoEnabled(getBool(v, 2)); return 0; }
	NB_PROP_SET(alertEnabled)			{ self(v)->setAlertEnabled(getBool(v, 2)); return 0; }

	NB_FUNC(info)						{ self(v)->info(getString(v, 2), getString(v, 3), true); return 0; }
	NB_FUNC(alert)						{ self(v)->alert(getString(v, 2), getString(v, 3)); return 0; }
	NB_FUNC(openArchive)				{ return push(v, self(v)->openArchive(getString(v, 2), getString(v, 3))); }
	NB_FUNC(channel)					{ return push(v, self(v)->channel()); }

	NB_FUNC(execute)					{ return push(v, self(v)->execute(getString(v, 2), opt<StreamReader>(v, 3, NULL), opt<StreamWriter>(v, 4, NULL), opt<StreamWriter>(v, 5, NULL))); }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::AsyncJob, RefCounted, incRefCount, decRefCount);

class NB_AsyncJob: TNitClass<AsyncJob>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(status),
			PROP_ENTRY_R(doing),
			PROP_ENTRY_R(done),
			PROP_ENTRY_R(canceled),
			PROP_ENTRY_R(subJobCount),
			PROP_ENTRY_R(parentJob),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(cancel,		"(join: bool)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "JOB");
		newSlot(v, -1, "IDLE",			(int)AsyncJob::JOB_IDLE);
		newSlot(v, -1, "PREPARING",		(int)AsyncJob::JOB_PREPARING);
		newSlot(v, -1, "PENDING",		(int)AsyncJob::JOB_PENDING);
		newSlot(v, -1, "DOING",			(int)AsyncJob::JOB_DOING);
		newSlot(v, -1, "SUCCESS",		(int)AsyncJob::JOB_SUCCESS);
		newSlot(v, -1, "FAILED",		(int)AsyncJob::JOB_FAILED);
		newSlot(v, -1, "CANCELED",		(int)AsyncJob::JOB_CANCELED);
		newSlot(v, -1, "ERROR",			(int)AsyncJob::JOB_ERROR);
		sq_poptop(v);
	}

	NB_PROP_GET(status)					{ return push(v, (int)self(v)->getStatus()); }
	NB_PROP_GET(doing)					{ return push(v, self(v)->isDoing()); }
	NB_PROP_GET(done)					{ return push(v, self(v)->isDone()); }
	NB_PROP_GET(canceled)				{ return push(v, self(v)->isCanceled()); }
	NB_PROP_GET(subJobCount)			{ return push(v, self(v)->getSubJobCount()); }
	NB_PROP_GET(parentJob)				{ return push(v, self(v)->getParentJob()); }

	NB_FUNC(cancel)						{ self(v)->cancel(getBool(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::AsyncJobManager, RefCounted, incRefCount, decRefCount);

class NB_AsyncJobManager : TNitClass<AsyncJobManager>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(name),
			PROP_ENTRY_R(workerCount),
			PROP_ENTRY_R(jobCount),
			PROP_ENTRY_R(prepCount),
			PROP_ENTRY_R(pendingCount),
			PROP_ENTRY_R(doingCount),
			PROP_ENTRY_R(doneCount),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(name: string, numWorkers: int)"),
			FUNC_ENTRY_H(addWorkers,	"(count: int)"),
			FUNC_ENTRY_H(resume,		"()"),
			FUNC_ENTRY_H(suspend,		"(join=true)"),
			FUNC_ENTRY_H(stop,			"()"),
			FUNC_ENTRY_H(update,		"()"),
			FUNC_ENTRY_H(enqueue,		"(job: AsyncJob)"),
			FUNC_ENTRY_H(cancel,		"(job: AsyncJob, join: bool)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }
	NB_PROP_GET(workerCount)			{ return push(v, self(v)->getWorkerCount()); }
	NB_PROP_GET(jobCount)				{ return push(v, self(v)->getJobCount()); }
	NB_PROP_GET(prepCount)				{ return push(v, self(v)->getPrepCount()); }
	NB_PROP_GET(pendingCount)			{ return push(v, self(v)->getPendingCount()); }
	NB_PROP_GET(doingCount)				{ return push(v, self(v)->getDoingCount()); }
	NB_PROP_GET(doneCount)				{ return push(v, self(v)->getDoneCount()); }

	NB_CONS()							{ setSelf(v, new AsyncJobManager(getString(v, 2), getInt(v, 3))); return SQ_OK; }

	NB_FUNC(addWorkers)					{ self(v)->addWorkers(getInt(v, 2)); return 0; }
	NB_FUNC(resume)						{ self(v)->resume(); return 0; }
	NB_FUNC(suspend)					{ self(v)->suspend(optBool(v, 2, true)); return 0; }
	NB_FUNC(stop)						{ self(v)->stop(); return 0; }
	NB_FUNC(update)						{ self(v)->update(); return 0; }
	NB_FUNC(enqueue)					{ self(v)->enqueue(get<AsyncJob>(v, 2)); return 0; }
	NB_FUNC(cancel)						{ self(v)->cancel(get<AsyncJob>(v, 2), getBool(v, 3)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

SQRESULT NitLibCore(HSQUIRRELVM v)
{
	NB_RefCounted::Register(v);

	NB_MemManager::Register(v);
	NB_LogManager::Register(v);
	NB_LogChannel::Register(v);
	NB_ScriptUnit::Register(v);
	NB_ScriptRuntime::Register(v);
	NB_WaitBlock::Register(v);

	NB_CacheManager::Register(v);
	NB_CacheHandle::Register(v);

	NB_Timestamp::Register(v);
	NB_DateTime::Register(v);
	NB_RegExp::Register(v);

	NB_ContentType::Register(v);
	NB_ContentBase::Register(v);

	NB_StreamLocator::Register(v);
	NB_StreamSource::Register(v);
	NB_StreamReader::Register(v);
	NB_StreamWriter::Register(v);

	NB_Base64Encoder::Register(v);
	NB_Base64Decoder::Register(v);
	NB_CalcCRC32Writer::Register(v);

	NB_CopyReader::Register(v);
	NB_ShadowWriter::Register(v);
	NB_LogWriter::Register(v);
	NB_HexDumpWriter::Register(v);

	NB_ZStreamReader::Register(v);
	NB_ZStreamWriter::Register(v);

	NB_StreamPrinter::Register(v);
	NB_JsonPrinter::Register(v);
	NB_XMLPrinter::Register(v);

	NB_MemoryBuffer::Register(v);
	NB_MemoryBufferReader::Register(v);
	NB_MemoryBufferWriter::Register(v);
	NB_MemorySource::Register(v);

	NB_Archive::Register(v);
	NB_CustomArchive::Register(v);

	NB_FileUtil::Register(v);
	NB_FileLocator::Register(v);
	NB_File::Register(v);

	NB_Settings::Register(v);
	NB_CmdLineParser::Register(v);

	NB_EventAutomata::Register(v);
	NB_EventState::Register(v);
	NB_ScriptEventState::Register(v);

	NB_NitRuntime::Register(v);

	NB_AsyncJob::Register(v);
	NB_AsyncJobManager::Register(v);

	sq_pushroottable(v);
	NitBind::newSlot(v, -1, "stdout",	(StreamWriter*)StdIOWriter::createStdOut());
	NitBind::newSlot(v, -1, "stderr",	(StreamWriter*)StdIOWriter::createStdErr());
	sq_poptop(v);

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

extern SQRESULT NitLibCore(HSQUIRRELVM v);
extern SQRESULT NitLibMath(HSQUIRRELVM v);
extern SQRESULT NitLibEvent(HSQUIRRELVM v);
extern SQRESULT NitLibTimer(HSQUIRRELVM v);
extern SQRESULT NitLibData(HSQUIRRELVM v);
extern SQRESULT NitLibCoreExt(HSQUIRRELVM v);
extern SQRESULT NitLibDebug(HSQUIRRELVM v);

#if defined(NIT_WIN32)
extern SQRESULT NitLibCom(HSQUIRRELVM v);
#endif

#if defined(NIT_ANDROID)
extern SQRESULT NitLibJava(HSQUIRRELVM v);
extern SQRESULT NitLibAndroid(HSQUIRRELVM v);
#endif

class NitLibCoreModule
{
public:
	NitLibCoreModule()
	{
		o.push_back(new NIT_LIB_ENTRY(NitLibCore,		""));
		o.push_back(new NIT_LIB_ENTRY(NitLibMath,		"NitLibCore"));
		o.push_back(new NIT_LIB_ENTRY(NitLibEvent,		"NitLibCore"));
		o.push_back(new NIT_LIB_ENTRY(NitLibTimer,		"NitLibCore NitLibEvent"));
		o.push_back(new NIT_LIB_ENTRY(NitLibData,		"NitLibCore NitLibEvent"));
		o.push_back(new NIT_LIB_ENTRY(NitLibCoreExt,		"NitLibCore NitLibEvent"));
		o.push_back(new NIT_LIB_ENTRY(NitLibDebug,		"NitLibCore"));
#if defined(NIT_WIN32)
		o.push_back(new NIT_LIB_ENTRY(NitLibCom,			"NitLibCore"));
#endif

#if defined(NIT_ANDROID)
		o.push_back(new NIT_LIB_ENTRY(NitLibJava,		"NitLibCore"));
		o.push_back(new NIT_LIB_ENTRY(NitLibAndroid,		"NitLibCore NitLibEvent NitLibJava"));
#endif
	}

	~NitLibCoreModule()
	{
		std::for_each(o.begin(), o.end(), DeleteFunc());
	}

private:
	std::vector<NitBindLibRegistry*> o;
};

NitLibCoreModule s_NitLibCoreModule;

void* RegisterNitLibCoreModule()
{
	return &s_NitLibCoreModule;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;