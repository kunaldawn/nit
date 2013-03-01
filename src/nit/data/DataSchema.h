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

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class StreamSource;

////////////////////////////////////////////////////////////////////////////////

class NIT_API DataProperty : public WeakSupported
{
public:
	DataProperty(DataSchema* schema, DataKey* key);

public:
	DataKey*							getKey()								{ return _key; }
	const String&						getName()								{ return _key->getName(); }
	DataSchema*							getSchema()								{ return _schema; }

	DataRecord*							getAttributes()							{ return _attributes; }
	void								setAttributes(DataRecord* attrs)		{ _attributes = attrs; }

public:
	bool								getValue(DataObject* obj, DataValue& outValue) const;
	bool								setValue(DataObject* obj, const DataValue& value) const;

public:									// definition methods
	typedef bool						(*GetFunc) (DataObject* object, DataProperty* prop, DataValue& outValue);
	typedef bool						(*SetFunc) (DataObject* object, DataProperty* prop, DataValue& value);

	DataProperty&						bind(GetFunc getter, SetFunc setter)	{ _getter = getter; _setter = setter; return *this; }
	DataProperty&						type(DataValue::Type type)				{ _type = type; return *this; }
	DataProperty&						order(float order)						{ _order = order; return *this; }
	DataProperty&						save(bool flag)							{ _save = flag; return *this; }

public:
	bool								isReadOnly()							{ return _setter == NULL; }
	bool								isSave()								{ return _save; }

public:
	DataValue::Type						getType()								{ return _type; }
	float								getOrder()								{ return _order; }

protected:
	DataSchema*							_schema;
	GetFunc								_getter;
	SetFunc								_setter;
	DataValue::Type						_type;
	float								_order;
	Ref<DataKey>						_key;
	Ref<DataRecord>						_attributes;
	bool								_save;
};

////////////////////////////////////////////////////////////////////////////////

class DataSchemaLookup;

////////////////////////////////////////////////////////////////////////////////

class NIT_API DataSchema : public RefCounted
{
public:
	struct NIT_API Info
	{
		String							prefix;
		String							name;
		String							version;

		Ref<DataKey>					key;

		uint							verMajor;
		uint							verMinor;

		Info() : verMajor(0), verMinor(0) { }

		bool							set(const String& keyName, DataNamespace* ns);
	};

public:
	DataSchema(const String& keyName, DataSchema* base = NULL); // TODO: change to DataKey, auto-detect version code

public:
	DataSchema*							getBase()								{ return _base; }

	const Info&							getInfo()								{ return _info; }

	DataKey*							getKey()								{ return _info.key; }
	const String&						getKeyName()							{ return _info.key->getName(); }

	const String&						getPrefix()								{ return _info.prefix; }
	const String&						getName()								{ return _info.name; }
	const String&						getVersion()							{ return _info.version; }

	uint								getVerMajor()							{ return _info.verMajor; }
	uint								getVerMinor()							{ return _info.verMinor; }

	DataRecord*							getAttributes()							{ return _attributes; }
	void								getAttributes(DataRecord* attrs)		{ _attributes = attrs; }

	bool								isActive()								{ return _active; }

public:
	DataProperty&						newProperty(const String& name);
	DataProperty&						newProperty(DataKey* key);

	DataProperty*						getProperty(const String& name);
	DataProperty*						getProperty(DataKey* key);

	typedef vector<DataProperty*>::type OrderedProperties;
	OrderedProperties&					getOrderedProperties();

public:
	virtual Ref<DataObject>				create() = 0;

public:
	void								dump();
	void								dump(DataObject* obj);

public:									// definition methods
	virtual bool						onRegister()							{ return true; }
	virtual void						onUnregister()							{ }

protected:
	DataSchema();
	bool								setKeyName(const String& keyName);

	virtual void						onDelete();

	void								collectProperties(OrderedProperties& props);
	void								sortProperties();

protected:
	friend class DataSchemaLookup;

	Weak<DataSchemaLookup>				_lookup;

	Info								_info;

	Ref<DataSchema>						_base;
	Ref<DataNamespace>					_namespace;

	Ref<DataRecord>						_attributes;

	typedef unordered_map<Ref<DataKey>, DataProperty*>::type Properties;

	Properties							_properties;
	OrderedProperties					_orderedProperties;

	bool								_active;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API DataSchemaLoader : public WeakSupported
{
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API DataSchemaLookup : public RefCounted
{
public:
	DataSchemaLookup();

	static DataSchemaLookup*			getCurrent();
	static void							setCurrent(DataSchemaLookup* current);

public:
	DataNamespace*						getNamespace()							{ return _namespace; }

	DataSchemaLookup*					getUplink()								{ return _uplink; }
	void								setUplink(DataSchemaLookup* uplink)		{ _uplink = uplink; }

public:
	DataSchema*							get(const String& key)					{ return get(_namespace->get(key)); }
	DataSchema*							get(DataKey* key);
	void								find(const String& pattern, vector<DataSchema*>::type& varResults);

public:
	DataSchema*							load(const String& key);
	DataSchema*							load(DataKey* key);

public:
	void								Register(DataSchema* schema);
	void								Unregister(DataSchema* schema);

protected:
	virtual DataSchema*					onLoad(DataKey* key)					{ return NULL; }

protected:
	Ref<DataNamespace>					_namespace;

	typedef unordered_map<Ref<DataKey>, Ref<DataSchema> >::type KeyLookup;
	KeyLookup							_keyLookup;

	typedef multimap<String, DataSchema*>::type NameLookup;
	NameLookup							_prefixedNameLookup; // TODO: implement

private:
	static Ref<DataSchemaLookup>		s_Current;
	Ref<DataSchemaLookup>				_uplink;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
