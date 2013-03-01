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

#include "nit/data/DataSchema.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class TestDataObject : public DataObject
{
public:
	const String&						getName()								{ return _name; }
	int									getAge()								{ return getYear() - _birthYear; }
	int									getBirthYear()							{ return _birthYear; }
	int									getYear();
	DataRecord*							getRecord()								{ return _record; }

	void								setName(const String& name)				{ _name = name; }
	void								setAge(int age)							{ _birthYear = getYear() - age; }

public:									// DataObject impl
	DataSchema*							getDataSchema()							{ return (DataSchema*)_schema; }
	bool								isDisposed()							{ return false; }

	static void test();

protected:
	class Schema;

	TestDataObject(Schema* schema);

	Schema*								_schema;
	String								_name;
	int									_birthYear;
	Ref<DataRecord>						_record;
};

////////////////////////////////////////////////////////////////////////////////

class TestDataObject::Schema : public DataSchema
{
public:
	Schema() : DataSchema("nit.test.TestDataObject.1.0")
	{
		newProperty("Name")
			.type(DataValue::TYPE_STRING)
			.bind(PropGetName, PropSetName);

		newProperty("Age")
			.type(DataValue::TYPE_INT)
			.bind(PropGetAge, PropSetAge);

		newProperty("BirthYear")
			.type(DataValue::TYPE_INT)
			.bind(PropGetBirthYear, NULL);
	}

	int GetYear() { return 2011; }

	virtual Ref<DataObject> create()
	{
		return new TestDataObject(this);
	}

private:
	static bool PropGetName(DataObject* object, DataProperty* prop, DataValue& outValue)
	{
		outValue = ((TestDataObject*)object)->_name;
		return true;
	}

	static bool PropSetName(DataObject* object, DataProperty* prop, DataValue& value)
	{
		((TestDataObject*)object)->setName(value.toString());
		return true;
	}

	static bool PropGetAge(DataObject* object, DataProperty* prop, DataValue& outValue)
	{
		outValue = ((TestDataObject*)object)->getAge();
		return true;
	}

	static bool PropSetAge(DataObject* object, DataProperty* prop, DataValue& value)
	{
		((TestDataObject*)object)->setAge(value.toInt());
		return true;
	}

	static bool PropGetBirthYear(DataObject* object, DataProperty* prop, DataValue& outValue)
	{
		outValue = ((TestDataObject*)object)->getBirthYear();
		return true;
	}
};

int TestDataObject::getYear()
{
	return _schema->GetYear();
}

TestDataObject::TestDataObject(Schema* schema)
{
	_schema = schema;
	_record = new DataRecord();
}

void TestDataObject::test()
{
	Schema schema;
	Ref<DataObject> obj = schema.create();
}

////////////////////////////////////////////////////////////////////////////////

DataProperty::DataProperty(DataSchema* schema, DataKey* key) 
: _key(key)
, _schema(schema)
, _getter(NULL)
, _setter(NULL)
, _type(DataValue::TYPE_ANY)
, _order(1000.0f)
, _save(true)
{

}

bool DataProperty::getValue(DataObject* obj, DataValue& outValue) const
{
	if (_getter && _getter(obj, const_cast<DataProperty*>(this), outValue))
	{
		outValue.convertTo(_type);
		return true;
	}

	return false;
}

bool DataProperty::setValue(DataObject* obj, const DataValue& value) const
{
	return _setter && _setter(obj, const_cast<DataProperty*>(this), DataValue(value).convertTo(_type));
}

////////////////////////////////////////////////////////////////////////////////

DataSchema::DataSchema()
{
	if (_namespace == NULL)
		_namespace = DataNamespace::getGlobal();
}

DataSchema::DataSchema(const String& keyName, DataSchema* base)
: _base(base)
{
	if (_namespace == NULL)
		_namespace = DataNamespace::getGlobal();

	if (!setKeyName(keyName))
		NIT_THROW_FMT(EX_SYNTAX, "bad schema keyname: '%s'", keyName.c_str());
}

DataProperty& DataSchema::newProperty(const String& name)
{
	Ref<DataKey> key = _namespace->add(name);

	return newProperty(key);
}

DataProperty& DataSchema::newProperty(DataKey* key)
{
	ASSERT_THROW(key && key->getNamespace() == _namespace, EX_INVALID_PARAMS);

	NIT_THROW_FMT(EX_DUPLICATED, "property '%s' already exists", key->getName().c_str());

	DataProperty& prop = *_properties.insert(std::make_pair(key, new DataProperty(this, key))).first->second;

	_orderedProperties.clear(); // Will be generated and sorted again at next GetOrdererdProperties() lazy call

	return prop;
}

DataProperty* DataSchema::getProperty(DataKey* key)
{
	if (key == NULL) return NULL;

	Properties::iterator itr = _properties.find(key);
	if (itr != _properties.end())
		return itr->second;

	if (_base)
		return _base->getProperty(key);

	return NULL;
}

DataProperty* DataSchema::getProperty(const String& name)
{
	return getProperty(_namespace->get(name));
}

void DataSchema::dump()
{
	LOG(0, "-- schema '%s'\n", getKeyName().c_str());

	if (_base)
		LOG(0, "-- - base: '%s'\n", _base->getKeyName().c_str());

	OrderedProperties& props = getOrderedProperties();

	for (uint i=0; i < props.size(); ++i)
	{
		DataProperty* prop = props[i];

		LOG(0, "-- - #%3.1f %-13s : %s%s%s\n", 
			prop->getOrder(),
			prop->getName().c_str(), 
			DataValue::typeToStr(prop->getType()), 
			prop->isReadOnly() ? " [ro]" : "",
			prop->isSave() ? " [save]" : ""
			);
	}
}

void DataSchema::dump(DataObject* obj)
{
	LOG(0, "-- object '%s' %08x\n", getKeyName().c_str(), obj);

	if (_base)
		LOG(0, "-- - base: '%s'\n", _base->getKeyName().c_str());

	OrderedProperties& props = getOrderedProperties();

	for (uint i=0; i < props.size(); ++i)
	{
		DataProperty* prop = props[i];
		
		DataValue value;
		String valueStr = "<error>";

		if (prop->getValue(obj, value))
			valueStr = value.dumpString();

		LOG(0, "-- - #%3.1f %-13s = %s: %s%s%s\n", 
			prop->getOrder(),
			prop->getName().c_str(), 
			valueStr.c_str(), 
			DataValue::typeToStr(prop->getType()), 
			prop->isReadOnly() ? " [ro]" : "",
			prop->isSave() ? " [save]" : ""
			);
	}
}

void DataSchema::onDelete()
{
	for (Properties::iterator itr = _properties.begin(), end = _properties.end(); itr != end; ++itr)
	{
		delete itr->second;
	}
	_properties.clear();

	if (_lookup)
		_lookup->Unregister(this);
}

DataSchema::OrderedProperties& DataSchema::getOrderedProperties()
{
	if (!_properties.empty() && _orderedProperties.empty())
	{
		sortProperties();
	}

	return _orderedProperties;
}

bool LessPropOrder(DataProperty* a, DataProperty* b)
{
	return a->getOrder() < b->getOrder();
}

void DataSchema::collectProperties(OrderedProperties& props)
{
	for (Properties::iterator itr = _properties.begin(), end = _properties.end(); itr != end; ++itr)
	{
		props.push_back(itr->second);
	}

	if (_base)
		_base->collectProperties(props);
}

void DataSchema::sortProperties()
{
	_orderedProperties.clear();

	collectProperties(_orderedProperties);

	std::sort(_orderedProperties.begin(), _orderedProperties.end(), LessPropOrder);
}

bool DataSchema::setKeyName(const String& keyName)
{
	return _info.set(keyName, _namespace);
}

bool DataSchema::Info::set(const String& keyName, DataNamespace* ns)
{
	size_t minorStart = keyName.rfind('.');
	size_t majorStart = minorStart != keyName.npos ? keyName.rfind('.', minorStart-1) : keyName.npos;
	size_t nameStart = majorStart != keyName.npos ? keyName.rfind('.', majorStart-1) : keyName.npos;

	uint verMinor = 0;
	uint verMajor = 0;

	char ch;

	if (minorStart == keyName.npos) return false;
	if (majorStart == keyName.npos) return false;

	if (minorStart + 1 >= keyName.length()) return false;
	if (majorStart + 1 >= minorStart) return false;

	for (uint i=minorStart+1; i < keyName.length(); ++i)
	{
		ch = keyName[i];
		if (ch < '0' || ch > '9') return false;

		verMinor = 10 * verMinor + (ch - '0');
	}

	for (uint i=majorStart+1; i < minorStart; ++i)
	{
		ch = keyName[i];
		if (ch < '0' || ch > '9') return false;

		verMajor = 10 * verMajor + (ch - '0');
	}

	if (nameStart != keyName.npos)
	{
		name = keyName.substr(nameStart + 1, majorStart - nameStart - 1);
		prefix = keyName.substr(0, nameStart);
	}
	else
	{
		name = keyName.substr(0, majorStart);
		prefix.clear();
	}

	verMajor = verMajor;
	verMinor = verMinor;
	version = keyName.substr(majorStart + 1);

	key = ns->add(keyName);

	return true;
}

////////////////////////////////////////////////////////////////////////////////

// TODO: Rearrange relationship between SchemaLookup and Namespace

Ref<DataSchemaLookup> DataSchemaLookup::s_Current = NULL;

// TODO: Refactor s_Current stuffs!

DataSchemaLookup::DataSchemaLookup()
{
	if (_namespace == NULL)
		_namespace = DataNamespace::getGlobal();
}

void DataSchemaLookup::setCurrent(DataSchemaLookup* current)
{
	if (s_Current == current)
		return;

	s_Current = current;
}

DataSchemaLookup* DataSchemaLookup::getCurrent()
{
	if (s_Current == NULL)
		s_Current = new DataSchemaLookup();

	return s_Current;
}

void DataSchemaLookup::Register(DataSchema* schema)
{
	Ref<DataKey> key = schema->getKey();

	if (key == NULL)
		NIT_THROW_FMT(EX_INVALID_PARAMS, "invalid schema without key");

	KeyLookup::iterator itr = _keyLookup.find(key);

	if (itr != _keyLookup.end())
		NIT_THROW_FMT(EX_DUPLICATED, "schema '%s' already registered", schema->getKeyName().c_str());

	ASSERT_THROW(schema->_lookup == NULL, EX_INVALID_STATE);

	schema->_lookup = this;

	_keyLookup.insert(std::make_pair(key, schema));

	bool ok = schema->onRegister();
	if (ok)
	{
		LOG(0, ".. DataSchema '%s' registered\n", key->getName().c_str());
	}

	// TODO: Register to PrefixedNameLookup also.
}

void DataSchemaLookup::Unregister(DataSchema* schema)
{
	if (schema->_lookup == NULL) return;

	ASSERT_THROW(schema->_lookup == this, EX_INVALID_STATE);

	Ref<DataKey> key = schema->getKey();

	KeyLookup::iterator itr = _keyLookup.find(key);

	// Call OnUnregister() only when actually registered to lookup
	if (itr != _keyLookup.end())
	{
		schema->onUnregister();
		schema->_lookup = NULL;
		_keyLookup.erase(itr);
		LOG(0, ".. DataSchema '%s' unregistered\n", key->getName().c_str());
	}
}

DataSchema* DataSchemaLookup::get(DataKey* key)
{
	if (key == NULL) return NULL;

	KeyLookup::iterator itr = _keyLookup.find(key);

	if (itr != _keyLookup.end())
		return itr->second;

	return _uplink ? _uplink->get(key) : NULL;
}

void DataSchemaLookup::find(const String& pattern, vector<DataSchema*>::type& varResults)
{
	for (KeyLookup::iterator itr = _keyLookup.begin(); itr != _keyLookup.end(); ++itr)
	{
		if (Wildcard::match(pattern, itr->first->getName()))
			varResults.push_back(itr->second);
	}

	if (_uplink) _uplink->find(pattern, varResults);
}

DataSchema* DataSchemaLookup::load(const String& name)
{
	// Not-yet-exist key can appear while loading, so Add + Ref.
	Ref<DataKey> key = _namespace->add(name);
	return load(key);
}

DataSchema* DataSchemaLookup::load(DataKey* key)
{
	if (key == NULL) return NULL;

	DataSchema* schema = get(key);

	if (schema == NULL)
		schema = onLoad(key);

	return schema;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
