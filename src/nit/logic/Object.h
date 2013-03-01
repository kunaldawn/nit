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
#include "nit/data/DataValue.h"
#include "nit/event/event.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class World;
class Object;
class Component;
class Feature;
class PropValue;

class Transform;

////////////////////////////////////////////////////////////////////////////////

enum ObjectStatusFlag
{
	GOS_ACTIVE							= 0x0001,
	GOS_REACTIVATING					= 0x0002,
	GOS_ACTIVATED						= 0x0004,
	GOS_HASERROR						= 0x0010,
	GOS_DISPOSING						= 0x4000,
	GOS_DISPOSED						= 0x8000,
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API Object : public DataObject, public IEventSink
{
public:
	typedef multimap<String, Ref<Component> >::type Components;
	typedef map<String, Ref<Feature> >::type Features;

public:
	Object();
	virtual ~Object();

public:
	World*								getWorld()								{ return _world; }

	bool								dispose();

public:
	String&								getName()								{ return _name; }
	void								setName(const String& name);

	bool								isActive()								{ return _status.all(GOS_ACTIVE); }
	void								setActive(bool flag);

public:
	BitSet								getStatusFlag()							{ return _status; }
	bool								isAttached()							{ return _world != NULL; }
	bool								isActivated()							{ return _status.any(GOS_ACTIVATED); }
	bool								isEditing()								{ return _editCount > 0; }
	bool								isReactivating()						{ return _status.any(GOS_REACTIVATING); }
	bool								hasError()								{ return _status.all(GOS_HASERROR); }

public:
	uint								getRuntimeId()							{ return _runtimeID; }

public:									// DataObject impl
	virtual DataSchema*					getDataSchema()							{ return _dataSchema; }

public:
	Component*							attach(Component* comp);
	Component*							attach(const String& name, Component* comp);

	Component*							get(const String& name);
	void								find(const String& pattern, vector<Component*>::type& varResults);

public:
	Feature*							getFeature(const String& name);
	bool								hasFeature(Feature* mod);
	void								findFeature(const String& pattern, vector<Feature*>::type& varResults);

	bool								install(Feature* mod);
	bool								uninstall(Feature* mod);

public:
	bool								beginEdit()								{ ++_editCount; return _editCount == 1; }
	bool								endEdit()								{ if (--_editCount == 0) _reactivateAll(); return _editCount == 0; }

public:
	bool								isEventActive()							{ return isActivated(); }
	virtual bool						isDisposed()							{ return _status.any(GOS_DISPOSING | GOS_DISPOSED); }

	EventChannel*						channel()								{ return _channel ? _channel : _channel = createChannel(); }

public:
	Transform*							getTransform()							{ return _transform; }
	void								setTransform(Transform* transformNode);

	void								beginTransform();
	void								endTransform();

	const Vector3						getPosition();
	const Quat							getRotation();

	void								setPosition(const Vector3& pos);
	void								setRotation(const Quat& rot);

	void								move(const Vector3& delta);
	void								rotate(const Quat& delta);

public:
	void								info(const String& msg, const String& prefix = ""); // TODO: remove this
	void								error(const String& msg, const String& prefix = ""); // TODO: remove this

protected:
	uint								_runtimeID;
	BitSet								_status;

	Ref<DataSchema>						_dataSchema;

	Components							_components;	// TODO: optimize memory footage (or lazy init pointer)
	Features							_features;		// TODO: optimize memory footage (or lazy init pointer)
	int									_editCount;

	Transform*							_transform;

	Ref<EventChannel>					_channel;
	Weak<World>							_world;

	String								_name;

private:
	friend class						SpawnClass;
	friend class						World;
	friend class						Component;
	friend class						Feature;

	void								_reactivateAll();
	void								_deactivateAll();
	void								_disposeByWorld();
	void								_dispose();

	bool								_attachComponent(Component* comp);
	bool								_detachComponent(Component* comp);
	bool								_renameComponent(Component* comp, const String& name);

	void								cancelEdit()							{ --_editCount; }
	void								_quitEdit();

	static uint							s_LastRuntimeID;

protected:
	virtual void						onAttach()								{ }
	virtual void						onDispose()								{ }

	virtual EventChannel*				createChannel()							{ return new EventChannel(); }
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
