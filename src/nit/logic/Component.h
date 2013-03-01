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

class World;
class Object;
class Component;
class Feature;

////////////////////////////////////////////////////////////////////////////////

enum ComponentStatusFlag
{
	GCS_ACTIVE							= 0x0001,
	GCS_ACTIVATED						= 0x0002,
	GCS_HASERROR						= 0x0010,
	GCS_REACTIVATING					= 0x1000,
	GCS_DISPOSING						= 0x4000,
	GCS_DISPOSED						= 0x8000,
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API Component : public RefCounted, public IEventSink, public PooledAlloc
{
protected:
	Component();
	virtual ~Component();

public:
	World*								getWorld();
	Object*								getObject()								{ return _object; }
	Feature*							getFeature()							{ return _feature; }

	bool								dispose();

public:
	const String&						getName()								{ return _name; }
	void								setName(const String& name);

	bool								isActive()								{ return _status.any(GCS_ACTIVE); }
	void								setActive(bool flag);

public:
	BitSet								getStatusFlag()							{ return _status; }
	bool								isActivated()							{ return _status.all(GCS_ACTIVATED); }
	bool								isAttached()							{ return _object != NULL; }
	bool								isDisposed()							{ return _status.any(GCS_DISPOSING | GCS_DISPOSED); }
	bool								hasError()								{ return _status.all(GCS_HASERROR); }

	bool								checkValid();

	void								info(const String& msg, const String& prefix = ""); // TODO: remove this
	void								error(const String& msg, const String& prefix = ""); // TODO: remove this

public:
	virtual bool						isEventActive()							{ return isActivated(); }
	EventChannel*						channel()								{ return _channel ? _channel : _channel = createChannel(); }

protected:
	BitSet								_status;
	Weak<Object>						_object;
	Weak<Feature>						_feature;
	Ref<EventChannel>					_channel;
	String								_name;

	virtual EventChannel*				createChannel()							{ return new EventChannel(); }

private:
	friend class						Object;
	friend class						Feature;

	void								_reactivate();
	void								_deactivate();
	void								_disposeByObject();
	void								_dispose();

protected:
										// DO NOT CALL DIRECTLY FROM SUBCLASS!
	virtual bool						onValidate()							{ return true; }
	virtual void						onEventStart()							{ }
	virtual void						onActivate()							{ }
	virtual void						onDeactivate()							{ }
	virtual void						onDispose()								{ }
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
