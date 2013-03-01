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

#include "nit/script/NitLibCore.h"

#include "nit/data/DataValue.h"
#include "nit/data/DataSchema.h"
#include "nit/data/DataSaver.h"
#include "nit/data/DataLoader.h"
#include "nit/data/DataChannel.h"

#include "nit/data/Database.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NIT_API ScriptDataSchema : public DataSchema
{
public:
	typedef Ref<DataObject> (*AdaptFunc) (ScriptDataSchema* schema, HSQUIRRELVM v, int instIdx);
	typedef SQInteger (*PushObjectFunc) (ScriptDataSchema* schema, HSQUIRRELVM v, DataObject* object);

public:
	ScriptDataSchema(AdaptFunc adapter, PushObjectFunc push, DataProperty::GetFunc getter, DataProperty::SetFunc setter);

public:
	static SQInteger					Register(HSQUIRRELVM v, int clsIdx, const char* key);
	static Ref<DataObject>				adapt(HSQUIRRELVM v, int instIdx);

public:
	void								dispose();

	SQRESULT							setup(HSQUIRRELVM v, int clsIdx, const String& key);

	SQInteger							pushObject(HSQUIRRELVM v, DataObject* object);

protected:								// DataSchema overrides
	virtual Ref<DataObject>				create();

	virtual bool						onRegister();
	virtual void						onUnregister();

	virtual void						onDelete();

private:
	Ref<ScriptPeer>						_classPeer;

	AdaptFunc							_adapter;
	PushObjectFunc						_push;

	DataProperty::GetFunc				_getter;
	DataProperty::SetFunc				_setter;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API ScriptDataProperty : public DataProperty
{
public:
	ScriptDataProperty(DataSchema* schema, DataKey* key, int sqPropIndex);

	static bool peerGet(ScriptPeer* peer, ScriptDataProperty* sprop, DataValue& outValue);

	static bool peerSet(ScriptPeer* peer, ScriptDataProperty* sprop, DataValue& value);

	SQInteger _propIndex;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API ScriptDataAdapter : public DataObject
{
public:
	ScriptDataAdapter(ScriptDataSchema* schema, HSQUIRRELVM v, int instIdx);

	ScriptDataSchema*					getScriptDataSchema()					{ return _schema; }
	ScriptPeer*							getScriptPeer()							{ return _peer; }

public:
	static Ref<DataObject>				adaptFunc(ScriptDataSchema* schema, HSQUIRRELVM v, int instIdx);
	static SQInteger					pushFunc(ScriptDataSchema* schema, HSQUIRRELVM v, DataObject* object);
	static bool							getFunc(DataObject* object, DataProperty* prop, DataValue& outValue);
	static bool							setFunc(DataObject* object, DataProperty* prop, DataValue& value);

protected:								// DataObject impl
	virtual DataSchema*					getDataSchema()							{ return _schema; }
	virtual void*						getRealPtr();

	virtual void						onLoadBegin(DataObjectContext* context);
	virtual void						onLoadEnd(DataObjectContext* context);

	virtual void						onSaveBegin(DataObjectContext* context);
	virtual void						onSaveEnd(DataObjectContext* context);

private:
	Ref<ScriptDataSchema>				_schema;
	Ref<ScriptPeer>						_peer;
};

//////////////////////////////////////////////////////////////////////////

class NIT_API ScriptRefDataAdapter : public DataObject
{
public:
	ScriptRefDataAdapter(ScriptDataSchema* schema, IScriptRef* ref);

public:
	static Ref<DataObject>				adaptFunc(ScriptDataSchema* schema, HSQUIRRELVM v, int instIdx);
	static SQInteger					pushFunc(ScriptDataSchema* schema, HSQUIRRELVM v, DataObject* object);
	static bool							getFunc(DataObject* object, DataProperty* prop, DataValue& outValue);
	static bool							setFunc(DataObject* object, DataProperty* prop, DataValue& value);

protected:								// DataObject impl
	virtual DataSchema*					getDataSchema()							{ return _schema; }
	virtual void*						getRealPtr()							{ return _ref->_ref(); }

	virtual void						onLoadBegin(DataObjectContext* context);
	virtual void						onLoadEnd(DataObjectContext* context);

	virtual void						onSaveBegin(DataObjectContext* context);
	virtual void						onSaveEnd(DataObjectContext* context);

private:
	Ref<ScriptDataSchema>				_schema;
	Ref<IScriptRef>						_ref;
};

//////////////////////////////////////////////////////////////////////////

NS_NIT_END;
