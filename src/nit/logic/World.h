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

#include "nit/event/Timer.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class World;
class Object;
class Component;
class Module;

////////////////////////////////////////////////////////////////////////////////

enum WorldStatusFlag
{
	GWS_SOMETHING_ATTACHED				= 0x0040,
	GWS_DISPOSING						= 0x4000,
	GWS_DISPOSED						= 0x8000,
};

////////////////////////////////////////////////////////////////////////////////

class World : public RefCounted, public IEventSink
{
public:
	typedef set<Ref<Object> >::type ObjectSet;
	typedef map<String, Ref<Module> >::type Modules;
	typedef std::pair<Modules::iterator, Modules::iterator> ModuleResults;

public:
	World();
	virtual ~World();

public:
	bool								dispose();

public:
	const String&						getName()								{ return _name; }
	void								setName(const String& name);

public:
	virtual bool						isEventActive()							{ return !isDisposed(); }
	virtual bool						isDisposed()							{ return _status.any(GWS_DISPOSING | GWS_DISPOSED); }

public:
	typedef vector<Object*>::type ObjectResultSet;
	typedef vector<Component*>::type ComponentResultSet;

	Object*								attach(Object* obj);
	Object*								attach(const String& name, Object* obj);

	void								all(ObjectResultSet& outResults);
	void								find(const char* wildcard, ObjectResultSet& outResults, bool activeOnly = false, bool ignoreCase = true);
	void								findComponents(const char* wildcard, ComponentResultSet& outResults, bool activeOnly = true, bool ignoreCase = true);

private:
	BitSet								_status;
	String								_name;

	ObjectSet							_attached;

private:
	friend class						Object;

	bool								_attachObject(Object* object);
	bool								_detachObject(Object* object);
	bool								_renameObject(Object* object, const String& name);

private:
	void								onTick(const TimeEvent* evt);
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
