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

#include "nit/script/ScriptRuntime.h"

#include "nit/script/NitLibData.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NIT_API ScriptObject : public TScriptEventSink<ScriptObject, Object>, public IScriptRef
{
public:
	ScriptObject(ScriptPeer* peer);
	virtual ~ScriptObject();

public:									// IScriptRef impl
	virtual RefCounted*					_ref()									{ return RefCounted::_ref(); }
	virtual WeakRef*					_weak()									{ return RefCounted::_weak(); }
	virtual ScriptPeer*					getScriptPeer()							{ return _peer; }
	virtual String						getDebugString()						{ return getName(); }

public:									// DataObject override
	virtual DataSchema*					getDataSchema();

	virtual void						onLoadBegin(DataObjectContext* context);
	virtual void						onLoadEnd(DataObjectContext* context);

	virtual void						onSaveBegin(DataObjectContext* context);
	virtual void						onSaveEnd(DataObjectContext* context);

protected:
	virtual void						onAttach();
	virtual void						onDispose();

public:									// ScriptDataSchema support
	static Ref<DataObject>				adaptFunc(ScriptDataSchema* schema, HSQUIRRELVM v, int instIdx);
	static SQInteger					pushFunc(ScriptDataSchema* schema, HSQUIRRELVM v, DataObject* object);
	static bool							getFunc(DataObject* object, DataProperty* prop, DataValue& outValue);
	static bool							setFunc(DataObject* object, DataProperty* prop, DataValue& value);

private:
	Ref<ScriptPeer>						_peer;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API ScriptComponent : public TScriptEventSink<ScriptComponent, Component>, public IScriptRef
{
public:
	ScriptComponent(ScriptPeer* peer);

public:									// IScriptRef impl
	virtual RefCounted*					_ref()									{ return RefCounted::_ref(); }
	virtual WeakRef*					_weak()									{ return RefCounted::_weak(); }
	virtual ScriptPeer*					getScriptPeer()							{ return _peer; }
	virtual String						getDebugString()						{ return getName(); }

protected:
	virtual bool						onValidate();
	virtual void						onEventStart();
	virtual void						onActivate();
	virtual void						onDeactivate();
	virtual void						onDispose();

private:
	Ref<ScriptPeer>						_peer;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API ScriptFeature : public Feature, public IScriptEventSink, public IScriptRef
{
public:
	ScriptFeature(const String& name, Package* package, HSQUIRRELVM v, SQInteger idx);

public:									// IScriptEventSink implementation
	virtual IEventSink*					getEventSink()							{ return this; }
	virtual int							push(HSQUIRRELVM v)						{ return NitBind::push(v, this); }

protected:								// Module implementation
	virtual void						onRegister();
	virtual void						onUnregister();

	virtual void						onInit();
	virtual void						onFinish();

protected:								// Feature implementation
	virtual bool						onInstall(Object* obj);
	virtual bool						onUninstall(Object* obj);

public:									// IScriptRef implementation
	virtual RefCounted*					_ref()									{ return RefCounted::_ref(); }
	virtual WeakRef*					_weak()									{ return RefCounted::_weak(); }
	virtual ScriptPeer*					getScriptPeer()							{ return _peer; }
	virtual String						getDebugString()						{ return getName(); }

private:
	Ref<ScriptPeer>						_peer;

	template <typename TClass>
	bool								callMethod(const char* name, TClass* target);

	void								onSessionStop(const Event* evt);
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
