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

#include "nit/script/NitLibData.h"

#include "nit/script/NitBind.h"
#include "nit/script/NitBindMacro.h"

#include "nit/script/ScriptRuntime.h"

#include "nit/io/MemoryBuffer.h"
#include "nit/io/Stream.h"
#include "nit/io/FileLocator.h"

#include "squirrel/sqstate.h"
#include "squirrel/sqobject.h"
#include "squirrel/sqtable.h"
#include "squirrel/sqvm.h"
#include "squirrel/sqclass.h"
#include "squirrel/sqstdblob.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

ScriptDataSchema::ScriptDataSchema(AdaptFunc adapter, PushObjectFunc push, DataProperty::GetFunc getter, DataProperty::SetFunc setter)
{
	_adapter	= adapter;
	_push		= push;

	_getter	= getter;
	_setter	= setter;
}

void ScriptDataSchema::onDelete()
{
	DataSchema::onDelete();
}

Ref<DataObject> ScriptDataSchema::create()
{
	if (_classPeer == NULL) return NULL;

	HSQUIRRELVM v = _classPeer->getWorker();

	SQInteger top = sq_gettop(v);

	_classPeer->pushObject(v);
	sq_push(v, -1);
	SQRESULT sr = sq_call(v, 1, true, true);

	if (SQ_FAILED(sr))
	{
		sq_settop(v, top);
		LOG(0, "*** can't call constructor of schema '%s'\n", getKeyName().c_str());
		return NULL;
	}

	DataValue value;
	if (SQ_FAILED(ScriptDataValue::toValue(v, -1, value)))
	{
		sq_settop(v, top);
		LOG(0, "*** can't convert object from schema '%s'\n", getKeyName().c_str());
		return NULL;
	}

	sq_settop(v, top);

	return value.toObject();
}

bool ScriptDataSchema::onRegister()
{
	// nothing to do
	return true;
}

void ScriptDataSchema::onUnregister()
{
	if (_classPeer == NULL) return;

	HSQUIRRELVM v = _classPeer->getWorker();
	if (v == NULL) return;

	SQInteger top = sq_gettop(v);

	// Remove '_schema' if ScriptRuntime alive
	_classPeer->pushObject(v);
	sq_pushstring(v, "_schema", -1);
	sq_deleteslot(v, -2, false);

	sq_settop(v, top);
}

SQRESULT ScriptDataSchema::setup(HSQUIRRELVM v, int clsIdx, const String& keyName)
{
	if (_info.key != NULL) return SQ_OK;

	SQRESULT sr = SQ_OK;

	clsIdx = NitBind::toAbsIdx(v, clsIdx);

	if (!setKeyName(keyName))
		return sq_throwfmt(v, "bad schema keyname: '%s'", keyName.c_str());

	// get class attr
	int top = sq_gettop(v);
	sq_pushnull(v);

	if (SQ_SUCCEEDED(sq_getattributes(v, clsIdx)))
	{
		// Convert schema attr to record
		if (sq_gettype(v, -1) != OT_NULL)
		{
			Ref<DataRecord> schemaAttrs = new DataRecord(_namespace);

			sr = ScriptDataValue::toRecord(v, -1, schemaAttrs);
			if (SQ_FAILED(sr))
			{
				LOG(0, "*** schema '%s': can't convert attrs of class to record\n", getKeyName().c_str());
				sq_settop(v, top);
				return sr;
			}

			DataValue schema = schemaAttrs->get("schema");
			if (schema.getType() == DataValue::TYPE_BOOL && schema.toBool() == false)
			{
				// This is not for a schema
				sq_settop(v, top);
				return sq_throwfmt(v, "schema '%s': class is not for a schema", keyName.c_str());
			}

			_attributes = schemaAttrs;
		}
	}
	sq_settop(v, top);

	SQClass* cls = sqi_class(stack_get(v, clsIdx));

	for (NitIterator itr(v, clsIdx); itr.hasNext(); itr.next())
	{
		if (sq_gettype(v, itr.keyIndex()) != OT_STRING) continue;
		String propname = sqx_getstring(v, itr.keyIndex());

		// Inspect getter and setter (only property returns something)
		sq_push(v, itr.keyIndex());
		sq_getproperty(v, clsIdx);

		bool hasGetter = sq_gettype(v, -2) != OT_NULL;
		bool hasSetter = sq_gettype(v, -1) != OT_NULL;
		sq_pop(v, 2);

		// At least getter should be present for a schema
		if (!hasGetter) continue;

		// Store property attribute as a DataRecord
		sq_push(v, itr.keyIndex());
		if (SQ_FAILED(sq_getattributes(v, clsIdx))) continue;
		if (sq_gettype(v, -1) != OT_TABLE) { sq_poptop(v); continue; }

		Ref<DataRecord> propAttrs = new DataRecord(_namespace);
		if (SQ_FAILED(ScriptDataValue::toRecord(v, -1, propAttrs)))
		{
			LOG(0, "*** schema '%s': can't convert attrs of '%s' to record\n",
				getKeyName().c_str(), propname.c_str());
			sr = SQ_ERROR;
			continue;
		}
		sq_poptop(v); // attributes

		const char* propType = propAttrs->get("type").c_str();
	
		DataValue::Type dt = DataValue::strToType(propType);
		if (dt == DataValue::TYPE_VOID)
		{
			LOG(0, "*** unknown property type '%s.%s': %s\n", getKeyName().c_str(), propname.c_str(), propType);
			sr = SQ_ERROR;
			continue;
		}

		bool isSave = propAttrs->get("save").Default(false).toBool();
		float order = propAttrs->get("order").Default(0.0f).toFloat();

		if (isSave && !hasSetter)
		{
			LOG(0, "*** property '%s.%s' saves but has no setter\n",
				getKeyName().c_str(), propname.c_str());
			sr = SQ_ERROR;
			continue;
		}

		// The property has been inspected, generate key
		Ref<DataKey> key = _namespace->add(propname);

		SQInteger sqPropIndex = cls->GetIndex(stack_get(v, itr.keyIndex()));

		ScriptDataProperty* prop = new ScriptDataProperty(this, key, sqPropIndex);
		_properties.insert(std::make_pair(key, prop));

		prop->type(dt);
		prop->order(order);
		prop->save(isSave);
		prop->setAttributes(propAttrs);

		prop->bind(_getter, hasSetter ? _setter : NULL);
	}

	// TODO: Link constructor

	if (SQ_FAILED(sr))
		sq_throwfmt(v, "can't setup ScriptDataSchema '%s'", keyName.c_str());

	_classPeer = new ScriptPeer(v, clsIdx);
	_classPeer->makeWeak(v);

	return sr;
}

SQInteger ScriptDataSchema::Register(HSQUIRRELVM v, int clsIdx, const char* keyName)
{
	clsIdx = NitBind::toAbsIdx(v, clsIdx);

	if (sq_gettype(v, clsIdx) != OT_CLASS)
		return sq_throwfmt(v, "ScriptDataSchema '%s': invalid class", keyName);

	Ref<DataSchema> schema;

	sq_pushstring(v, "_schema", -1);
	SQRESULT r = sq_get(v, clsIdx);

	if (SQ_SUCCEEDED(r))
	{
		sq_pushstring(v, "_classname", -1);
		sq_get(v, clsIdx);
		String clsName = NitBind::getString(v, -1);
		// Fail if the class has been already registered
		sq_pop(v, 2);
		return sq_throwfmt(v, "ScriptDataSchema '%s': class '%s' already registered", keyName, clsName.c_str());
	}

	// Check if already registered to lookup
	DataSchemaLookup* lookup = DataSchemaLookup::getCurrent();
	if (lookup->get(keyName) != NULL)
		return sq_throwfmt(v, "schema '%s' already registered", keyName);
	
	// 1. DataObject descendant : Link to the DataSchmea specified
	// 2. IScriptRef		    : Link to ScriptRefDataAdapter for IScriptRef
	// 3. other				    : Link to ScriptDataAdapter
	// 
	// Determine class (Register)
	// 1. DataObject descendant : Follow up base links and see if there's registered class using nit.DataObject typetag
	// 2. IScriptRef		    : Check if the class has IScriptRef interface
	// 
	// Determine instance (Adapt)
	// 1. DataObject descendant	: ToValue() will check Is<DataObject> -> Check "_schema" at GetSchema()
	// 2. other					: ScriptDataSchema.Adapt(): calls Ref<DataObject> 

	// Check if a descendant of DataObject
	bool isDataObject = false;
	{
		NitBind::pushClass<DataObject>(v);
		SQClass* dataObjectCls = sqi_class(stack_get(v, -1));

		SQClass* cl = sqi_class(stack_get(v, clsIdx));
		while (cl && cl != dataObjectCls)
		{
			cl = cl->_base;
		}
		isDataObject = cl == dataObjectCls;
		sq_poptop(v);
	}

	bool isScriptRef = false;
	isScriptRef = NitBind::hasInterface<IScriptRef>(v, clsIdx);

	if (schema == NULL && isDataObject)
	{
		// Invoke _CreateDataSchema() method of the class
		sq_pushstring(v, "_CreateDataSchema", -1);
		SQRESULT sr = sq_get(v, 2);
		if (SQ_SUCCEEDED(sr))
		{
			sq_push(v, 2);
			sr = sq_call(v, 1, true, true);
			if (SQ_SUCCEEDED(sr))
			{
				schema = NitBind::get<DataSchema>(v, -1);
				sq_poptop(v);
			}
			sq_poptop(v);
		}

		// If failed treat as no schema
	}

	if (schema == NULL && isScriptRef)
	{
		schema = new ScriptDataSchema(
			ScriptRefDataAdapter::adaptFunc, 
			ScriptRefDataAdapter::pushFunc, 
			ScriptRefDataAdapter::getFunc, 
			ScriptRefDataAdapter::setFunc);
	}

	if (schema == NULL)
	{
		schema = new ScriptDataSchema(
			ScriptDataAdapter::adaptFunc, 
			ScriptDataAdapter::pushFunc,
			ScriptDataAdapter::getFunc, 
			ScriptDataAdapter::setFunc);
	}

	if (schema == NULL)
		return sq_throwfmt(v, "schema '%s': not supported class", keyName);

	ScriptDataSchema* sSchema = dynamic_cast<ScriptDataSchema*>(schema.get());
	if (sSchema)
	{
		SQRESULT sr = sSchema->setup(v, clsIdx, keyName);
		if (SQ_FAILED(sr)) return sr;
	}

	// Register to lookup
	lookup->Register(schema);

	// Setup completed, register to the class
	NitBind::newSlot(v, clsIdx, "_schema", schema.get(), true);

	return NitBind::push(v, schema.get());
}

Ref<DataObject> ScriptDataSchema::adapt(HSQUIRRELVM v, int instIdx)
{
	Ref<DataObject> adapter;

	instIdx = NitBind::toAbsIdx(v, instIdx);

	sq_getclass(v, instIdx);
	sq_pushstring(v, "_schema", -1);

	if (SQ_SUCCEEDED(sq_get(v, -2)))
	{
		Ref<ScriptDataSchema> schema = dynamic_cast<ScriptDataSchema*>(NitBind::get<DataSchema>(v, -1));
		sq_poptop(v);

		if (schema)
			adapter = schema->_adapter(schema, v, instIdx);
	}

	sq_poptop(v);

	return adapter;
}

SQInteger ScriptDataSchema::pushObject(HSQUIRRELVM v, DataObject* adapter)
{
	Ref<DataObject> safe = adapter;

	return _push(this, v, adapter);
}

////////////////////////////////////////////////////////////////////////////////

ScriptDataProperty::ScriptDataProperty(DataSchema* schema, DataKey* key, int sqPropIndex) 
: DataProperty(schema, key)
{
	_propIndex = sqPropIndex;
}

inline static bool InstanceGet(HSQUIRRELVM v, SQInstance* inst, SQInteger sqPropIndex, DataValue& outValue)
{
	SQObjectPtr val;
	if (inst->GetByIndex(v, sqPropIndex, val) == SQInstance::IA_OK)
	{
		v->Push(val);
		bool ok = SQ_SUCCEEDED(ScriptDataValue::toValue(v, -1, outValue));
		v->Pop();
		return ok; 
	}

	return false;
}

inline static bool InstanceSet(HSQUIRRELVM v, SQInstance* inst, SQInteger sqPropIndex, DataValue& value)
{
	if (SQ_FAILED(ScriptDataValue::push(v, value))) return false;
	SQObjectPtr val = v->Top();

	bool ok = inst->SetByIndex(v, sqPropIndex, val) == SQInstance::IA_OK;
	sq_poptop(v); // val

	return ok;
}

bool ScriptDataProperty::peerGet(ScriptPeer* peer, ScriptDataProperty* sprop, DataValue& outValue)
{
	if (peer == NULL) return false;

	HSQUIRRELVM v = peer->getWorker();
	if (v == NULL) return false;

	SQObjectRef ref = peer->getObjectRef(v);
	SQInstance* inst = sqi_instance(ref);

	return inst && InstanceGet(v, inst, sprop->_propIndex, outValue);
}

bool ScriptDataProperty::peerSet(ScriptPeer* peer, ScriptDataProperty* sprop, DataValue& value)
{
	if (peer == NULL) return false;

	HSQUIRRELVM v = peer->getWorker();
	if (v == NULL) return false;

	SQObjectRef ref = peer->getObjectRef(v);
	SQInstance* inst = sqi_instance(ref);

	return inst && InstanceSet(v, inst, sprop->_propIndex, value);
}

////////////////////////////////////////////////////////////////////////////////

SQRESULT ScriptDataValue::toValue(HSQUIRRELVM v, int idx, DataValue& outValue)
{
	switch (sq_gettype(v, idx))
	{
	case OT_NULL:						outValue.toNull(); return SQ_OK;
	case OT_BOOL:						{ SQBool value = 0; sq_getbool(v, idx, &value); outValue = value != 0; return SQ_OK; }
	case OT_INTEGER:					{ int value = 0; sq_getinteger(v, idx, &value); outValue = value; return SQ_OK; }
	case OT_FLOAT:						{ float value = 0.0f; sq_getfloat(v, idx, &value); outValue = value; return SQ_OK; }
	case OT_STRING:						{ const char* value = ""; sq_getstring(v, idx, &value); outValue = value; return SQ_OK; }
	case OT_ARRAY:						{ Ref<DataArray> array = new DataArray(); SQRESULT sr = toArray(v, idx, array); outValue = array; return sr; }
	case OT_TABLE:						{ Ref<DataRecord> record = new DataRecord(); SQRESULT sr = toRecord(v, idx, record); outValue = record; return sr; }
	case OT_INSTANCE:
		// TODO: Optimize to table lookup instead of expensive Is<XXX>() call
		if (NitBind::is<DataValue>(v, idx))			outValue = *NitBind::get<DataValue>(v, idx);
		else if (NitBind::is<DataObject>(v, idx))	outValue = NitBind::get<DataObject>(v, idx);
		else if (NitBind::is<DataRecord>(v, idx))	outValue = NitBind::get<DataRecord>(v, idx);
		else if (NitBind::is<DataArray>(v, idx))	outValue = NitBind::get<DataArray>(v, idx);
		else if (NitBind::is<DataKey>(v, idx))		outValue = NitBind::get<DataKey>(v, idx);
		else if (NitBind::is<MemoryBuffer>(v, idx))	outValue = NitBind::get<MemoryBuffer>(v, idx);
		else if (NitBind::is<Timestamp>(v, idx))	outValue = *NitBind::get<Timestamp>(v, idx);
		else if (NitBind::is<Vector2>(v, idx))		outValue = *NitBind::get<Vector2>(v, idx);
		else if (NitBind::is<Vector3>(v, idx))		outValue = *NitBind::get<Vector3>(v, idx);
		else if (NitBind::is<Vector4>(v, idx))		outValue = *NitBind::get<Vector4>(v, idx);
		else if (NitBind::is<Quat>(v, idx))			outValue = *NitBind::get<Quat>(v, idx);
		else if (NitBind::is<Matrix3>(v, idx))		outValue = *NitBind::get<Matrix3>(v, idx);
		else if (NitBind::is<Matrix4>(v, idx))		outValue = *NitBind::get<Matrix4>(v, idx);
		else
		{
			Ref<DataObject> adapter = ScriptDataSchema::adapt(v, idx);
			if (adapter == NULL)
				return sq_throwerror(v, "not supported instance");

			outValue = adapter;
		}

		return SQ_OK;

	default:
		return sq_throwerror(v, "not supported type");
	}
}

SQRESULT ScriptDataValue::toArray(HSQUIRRELVM v, int idx, DataArray* array)
{
	if (sq_gettype(v, idx) != OT_ARRAY)
		return sq_throwerror(v, "array expected");

	for (NitIterator itr(v, idx); itr.hasNext(); itr.next())
	{
		DataValue value;
		SQRESULT sr = toValue(v, itr.valueIndex(), value);
		if (SQ_FAILED(sr))
			return sr;

		array->append(value);
	}

	return SQ_OK;
}

SQRESULT ScriptDataValue::toRecord(HSQUIRRELVM v, int idx, DataRecord* record)
{
	if (sq_gettype(v, idx) != OT_TABLE)
		return sq_throwerror(v, "table expected");

	for (NitIterator itr(v, idx); itr.hasNext(); itr.next())
	{
		String name = NitBind::getString(v, itr.keyIndex());

		DataValue value;
		SQRESULT sr = toValue(v, itr.valueIndex(), value);

		if (SQ_FAILED(sr))
			return sr;

		record->set(name, value);
	}

	return SQ_OK;
}

SQInteger ScriptDataValue::push(HSQUIRRELVM v, const DataValue& value, bool fullTransform)
{
	SQRESULT sr = SQ_ERROR;

	switch (value._type)
	{
	case DataValue::TYPE_VOID:
	case DataValue::TYPE_NULL:			sq_pushnull(v); sr = 1; break;

	case DataValue::TYPE_INT:			sq_pushinteger(v, value.getData<int>()); sr = 1; break;
	case DataValue::TYPE_INT64:			sq_pushinteger(v, (int)value.getData<int64>()); sr = 1; break;
	case DataValue::TYPE_DOUBLE:		sq_pushfloat(v, (float)value.getData<double>()); sr = 1; break;
	case DataValue::TYPE_FLOAT:			sq_pushfloat(v, value.getData<float>()); sr = 1; break;

	case DataValue::TYPE_TIMESTAMP:		sr = NitBind::push(v, value.getData<Timestamp>()); break;

	case DataValue::TYPE_VECTOR2:
	case DataValue::TYPE_FLOAT2:		sr = NitBind::push(v, value.getData<Vector2>()); break;
	case DataValue::TYPE_SIZE2:			NIT_THROW(EX_NOT_IMPLEMENTED); break;

	case DataValue::TYPE_VECTOR3:
	case DataValue::TYPE_FLOAT3:		sr = NitBind::push(v, value.getData<Vector3>()); break;
	case DataValue::TYPE_SIZE3:			NIT_THROW(EX_NOT_IMPLEMENTED); break;

	case DataValue::TYPE_VECTOR4:
	case DataValue::TYPE_FLOAT4:		sr = NitBind::push(v, value.getData<Vector4>()); break;
	case DataValue::TYPE_QUAT:			sr = NitBind::push(v, value.getData<Quat>()); break;

	case DataValue::TYPE_MATRIX3:
	case DataValue::TYPE_FLOAT3X3:		sr = NitBind::push(v, value.getData<Matrix3>()); break;

	case DataValue::TYPE_MATRIX4:
	case DataValue::TYPE_FLOAT4X4:		sr = NitBind::push(v, value.getData<Matrix4>()); break;

	case DataValue::TYPE_STRING:		sq_pushstring(v, value.getStringPtr(), value.getStringSize()); sr = 1; break;

	case DataValue::TYPE_BLOB:			sr = NitBind::push(v, new MemoryBuffer(value.getBlobPtr(), value.getBlobSize())); break;

	case DataValue::TYPE_BUFFER:		sr = NitBind::push(v, value.getRef<MemoryBuffer>()); break;

	case DataValue::TYPE_KEY:			sr = NitBind::push(v, value.getRef<DataKey>()); break;

	case DataValue::TYPE_ARRAY:			
		if (fullTransform)
			sr = push(v, value.getRef<DataArray>(), fullTransform); 
		else
			sr = NitBind::push(v, value.getRef<DataArray>());
		break;

	case DataValue::TYPE_RECORD:		
		if (fullTransform)
			sr = push(v, value.getRef<DataRecord>(), fullTransform);
		else
			sr = NitBind::push(v, value.getRef<DataRecord>());
		break;

	case DataValue::TYPE_OBJECT:
		{
			DataObject* o = value.getRef<DataObject>();
			ScriptDataSchema* sSchema = dynamic_cast<ScriptDataSchema*>(o->getDataSchema());
			if (sSchema)
				sr = sSchema->pushObject(v, o);
			else
				sr = NitBind::push(v, o);
		}
		break;

	default:							sr = sq_throwerror(v, "not supported DataValue script conversion");
	}

	return sr;
}

SQInteger ScriptDataValue::push(HSQUIRRELVM v, DataArray* array, bool fullTransform)
{
	sq_newarray(v, 0);
	for (DataArray::Iterator itr = array->begin(), end = array->end(); itr != end; ++itr)
	{
		SQInteger sr = push(v, *itr, fullTransform);
		if (SQ_FAILED(sr))
			return sr;

		sq_arrayappend(v, -2);
	}

	return 1;
}

SQInteger ScriptDataValue::push(HSQUIRRELVM v, DataRecord* record, bool fullTransform)
{
	sq_newtable(v);
	for (DataRecord::Iterator itr = record->begin(), end = record->end(); itr != end; ++itr)
	{
		const String& name = itr->first->getName();
		sq_pushstring(v, name.c_str(), name.length());
		SQInteger sr = push(v, itr->second, fullTransform);
		if (SQ_FAILED(sr))
			return sr;

		sq_newslot(v, -3, false);
	}

	return 1;
}

////////////////////////////////////////////////////////////////////////////////

struct NB_DataArrayItr
{
	typedef NB_DataArrayItr ThisClass;

	Ref<DataArray> _array;
	uint _index;

	NB_DataArrayItr(DataArray* array) : _array(array), _index(0) { }

	static SQInteger _nexti(HSQUIRRELVM v, DataArray* array)
	{
		ThisClass* itr = (ThisClass*) sq_newnativeitr(v, itrfunc, sizeof(ThisClass));
		new (itr) ThisClass(array);
		return 1;
	}

	static SQInteger itrfunc(HSQUIRRELVM v, SQUserPointer pItr)
	{
		ThisClass* itr = (ThisClass*)pItr;

		if (v == NULL)
		{
			itr->~ThisClass();
			return 0;
		}

		if (itr->_index < itr->_array->getCount())
		{
			ScriptDataValue::push(v, itr->_array->get(itr->_index++));
			return 1;
		}

		return 0;
	};
};

////////////////////////////////////////////////////////////////////////////////

struct NB_DataRecordItr
{
	typedef NB_DataRecordItr ThisClass;

	Ref<DataRecord> _record;
	int _index;

	NB_DataRecordItr(DataRecord* rec) : _record(rec), _index(0) { }

	static SQInteger _nexti(HSQUIRRELVM v, DataRecord* record)
	{
		ThisClass* itr = (ThisClass*) sq_newnativeitr(v, itrfunc, sizeof(ThisClass));
		new (itr) ThisClass(record);
		return 1;
	}

	static SQInteger itrfunc(HSQUIRRELVM v, SQUserPointer pItr)
	{
		ThisClass* itr = (ThisClass*)pItr;

		if (v == NULL)
		{
			itr->~ThisClass();
			return 0;
		}

		HashTable::Pair* pair = itr->_record->getTable().next(itr->_index);

		if (pair)
		{
			NitBind::push(v, pair->first->getName());
			ScriptDataValue::push(v, pair->second);
			return 2;
		}

		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_VALUE(NIT_API, nit::DataValue, NULL);

class NB_DataValue : TNitClass<DataValue>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(count),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()\n"
										"(value)"),

			FUNC_ENTRY_H(void,			"[class] (): DataValue"),
			FUNC_ENTRY_H(null,			"[class] (): DataValue"),

			FUNC_ENTRY_H(toVoid,		"()"),
			FUNC_ENTRY_H(toNull,		"(): null"),
			FUNC_ENTRY_H(toBool,		"(): bool"),
			FUNC_ENTRY_H(toInt,			"(): int"),
			FUNC_ENTRY_H(toInt64,		"(): int"),
			FUNC_ENTRY_H(toFloat,		"(): float"),
			FUNC_ENTRY_H(toDouble,		"(): double"),
			FUNC_ENTRY_H(toString,		"(): string"),
			FUNC_ENTRY_H(toVector2,		"(): Vector2"),
			FUNC_ENTRY_H(toVector3,		"(): Vector3"),
			FUNC_ENTRY_H(toVector4,		"(): Vector4"),
			FUNC_ENTRY_H(toQuat,		"(): Quat"),
			FUNC_ENTRY_H(toMatrix3,		"(): Matrix3"),
			FUNC_ENTRY_H(toMatrix4,		"(): Matrix4"),
			FUNC_ENTRY_H(toTimestamp,	"(): Timestamp"),

			FUNC_ENTRY_H(toArray,		"(): DataArray"),
			FUNC_ENTRY_H(toRecord,		"(): DataRecord"),
			FUNC_ENTRY_H(toObject,		"(): DataObject"),
			FUNC_ENTRY_H(toBuffer,		"(): MemoryBuffer"),
			FUNC_ENTRY_H(toKey,			"(): DataKey"),
			FUNC_ENTRY_H(toScript,		"(): object"),

			FUNC_ENTRY_H(fromJson,		"[class] (json: string): DataValue"),
			FUNC_ENTRY_H(toJson,		"(compact=true): string"),

			FUNC_ENTRY_H(load,			"[class] (source: StreamSource, treatAs=ContentType.UNKNOWN)"),
			FUNC_ENTRY_H(save,			"(w: StreamWriter)"),
			FUNC_ENTRY_H(saveCompressed,"(w: StreamWriter)"),
			FUNC_ENTRY_H(saveJson,		"(w: StreamWriter)"),
			FUNC_ENTRY_H(savePlist,		"(w: StreamWriter)"),

			FUNC_ENTRY_H(convertTo,		"(TYPE): this"),
			FUNC_ENTRY_H(default,		"(value): this // returns value if void or null"),

			FUNC_ENTRY_H(tointeger,		"(): int"),
			FUNC_ENTRY_H(tofloat,		"(): float"),

			FUNC_ENTRY_H(get,			"(index): DataValue"),
			FUNC_ENTRY_H(set,			"(index, value)"),

			FUNC_ENTRY	(_dump),
			FUNC_ENTRY	(_clone),
			FUNC_ENTRY  (_call),
			FUNC_ENTRY	(_get),
			FUNC_ENTRY	(_set),
			FUNC_ENTRY	(_nexti),
			FUNC_ENTRY	(_tostring),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "TYPE");
		vector<std::pair<std::string, type::Type> >::type types;
		type::allTypes(types);
		for (uint i=0; i < types.size(); ++i)
		{
			newSlot(v, -1, types[i].first.c_str(), (int)types[i].second);
		}
		sq_poptop(v);
	}

	NB_PROP_GET(count)					{ return push(v, self(v)->getCount()); }

	NB_CONS()
	{
		if (isNone(v, 2))
			new (self(v)) DataValue();
		else
		{
			new (self(v)) DataValue();
			return ScriptDataValue::toValue(v, 2, *self(v));
		}
		return SQ_OK;
	}

	NB_FUNC(void)						{ return push(v, DataValue::Void()); }
	NB_FUNC(null)						{ return push(v, DataValue::Null()); }

	NB_FUNC(toArray)					{ return push(v, self(v)->toArray()); }
	NB_FUNC(toRecord)					{ return push(v, self(v)->toRecord()); }
	NB_FUNC(toObject)					{ return push(v, self(v)->toObject()); }
	NB_FUNC(toBuffer)					{ return push(v, self(v)->toBuffer()); }
	NB_FUNC(toKey)						{ return push(v, self(v)->toKey()); }
	NB_FUNC(toScript)					{ return ScriptDataValue::push(v, *self(v), true); }

	NB_FUNC(toVoid)						{ self(v)->toVoid(); return 0; }
	NB_FUNC(toNull)						{ self(v)->toNull(); return 0; }
	NB_FUNC(toBool)						{ return push(v, self(v)->toBool()); }
	NB_FUNC(toInt)						{ return push(v, self(v)->toInt()); }
	NB_FUNC(toInt64)					{ return push(v, (int)self(v)->toInt64()); }
	NB_FUNC(toFloat)					{ return push(v, self(v)->toFloat()); }
	NB_FUNC(toDouble)					{ return push(v, (float)self(v)->toDouble()); }
	NB_FUNC(toString)					{ size_t len; const char* str = self(v)->c_str(&len); return push(v, str, len); }
	NB_FUNC(toVector2)					{ return push(v, self(v)->toVector2()); }
	NB_FUNC(toVector3)					{ return push(v, self(v)->toVector3()); }
	NB_FUNC(toVector4)					{ return push(v, self(v)->toVector4()); }
	NB_FUNC(toQuat)						{ return push(v, self(v)->toQuat()); }
	NB_FUNC(toMatrix3)					{ return push(v, self(v)->toMatrix3()); }
	NB_FUNC(toMatrix4)					{ return push(v, self(v)->toMatrix4()); }
	NB_FUNC(toTimestamp)				{ return push(v, self(v)->toTimestamp()); }

	NB_FUNC(convertTo)					{ return push(v, self(v)->convertTo((DataValue::Type)getInt(v, 2))); }
	NB_FUNC(default)					{ if (self(v)->isVoidOrNull()) { sq_push(v, 2); return 1; } sq_push(v, 1); return 1; }

	NB_FUNC(tointeger)					{ return push(v, self(v)->toInt()); }
	NB_FUNC(tofloat)					{ return push(v, self(v)->toFloat()); }

	NB_FUNC(fromJson)
	{
		const char* str = getString(v, 2);
		size_t len = sq_getsize(v, 2);
		return push(v, DataValue::fromJson(str, len));
	}

	NB_FUNC(toJson)						{ return push(v, self(v)->toJson(optBool(v, 2, true))); }

	NB_FUNC(load)						{ return push(v, DataValue::load(get<StreamSource>(v, 2), *opt<ContentType>(v, 3, ContentType::UNKNOWN))); }
	NB_FUNC(save)						{ self(v)->save(get<StreamWriter>(v, 2)); return 0; }
	NB_FUNC(saveCompressed)				{ self(v)->saveCompressed(get<StreamWriter>(v, 2)); return 0; }
	NB_FUNC(saveJson)					{ self(v)->saveJson(get<StreamWriter>(v, 2)); return 0; }
	NB_FUNC(savePlist)					{ self(v)->savePlist(get<StreamWriter>(v, 2)); return 0; }

	NB_FUNC(_call)						{ return ScriptDataValue::push(v, *self(v)); }

	NB_FUNC(_clone)						{ return push(v, self(v)->clone()); }

	NB_FUNC(get)
	{
		type* o = self(v);

		switch (o->_type)
		{
		case type::TYPE_ARRAY:			
			{
				DataValue value = isInt(v, 2) ? ((DataArray*)o->_ref)->get(getInt(v, 2)) : DataValue::Void();
				return push(v, value);
			}
			break;

		case type::TYPE_RECORD:
			{
				DataValue value = ((DataRecord*)o->_ref)->get(getString(v, 2));
				return push(v, value);
			}
			break;

		case type::TYPE_OBJECT:
			{
				DataObject* obj = ((DataObject*)o->_ref);
				DataSchema* schema = obj->getDataSchema();
				// TODO: Optimize ScriptDataObject
				DataProperty* prop = schema ? schema->getProperty(getString(v, 2)) : NULL;
				if (prop)
				{
					DataValue value;
					if (prop->getValue(obj, value))
						return push(v, value);
				}
			}
			break;

		case type::TYPE_VOID:
		case type::TYPE_NULL:
			break;

		default:
			if (isInt(v, 2) && getInt(v, 2) == 0)
				return push(v, *o);
		}

		return push(v, DataValue::Void());
	}

	NB_FUNC(_get)
	{
		type* o = self_noThrow(v);

		SQInteger sr = SQ_ERROR;

		if (o)
		{
			switch (o->_type)
			{
			case type::TYPE_ARRAY:			
				{
					DataValue value = isInt(v, 2) ? ((DataArray*)o->_ref)->get(getInt(v, 2)) : DataValue::Void();
					if (value.isVoid())
						sq_reseterror(v);
					else
						sr = ScriptDataValue::push(v, value);
				}
				break;

			case type::TYPE_RECORD:
				{
					DataValue value = ((DataRecord*)o->_ref)->get(getString(v, 2));
					if (value.isVoid())
						sq_reseterror(v);
					else
						sr = ScriptDataValue::push(v, value);
				}
				break;

			case type::TYPE_OBJECT:
				{
					DataObject* obj = ((DataObject*)o->_ref);
					DataSchema* schema = obj->getDataSchema();
					// TODO: Optimize ScriptDataObject
					DataProperty* prop = schema ? schema->getProperty(getString(v, 2)) : NULL;
					if (prop)
					{
						DataValue value;
						if (prop->getValue(obj, value))
							sr = ScriptDataValue::push(v, value);
					}
					else
						sq_reseterror(v);
				}
				break;

			case type::TYPE_VOID:
			case type::TYPE_NULL:
				sq_reseterror(v); // clean failure
				break;

			default:
				if (isInt(v, 2) && getInt(v, 2) == 0)
					sr = ScriptDataValue::push(v, *o);
				else
					sq_reseterror(v); // clean failure
			}
		}

		return sr;
	}

	NB_FUNC(set)
	{
		type* o = self(v);

		SQInteger sr = SQ_ERROR;

		switch (o->_type)
		{
		case type::TYPE_ARRAY:
			{
				int index = getInt(v, 2);
				DataValue value;
				sr = ScriptDataValue::toValue(v, 3, value);
				if (SQ_SUCCEEDED(sr))
					((DataArray*)o->_ref)->set(index, value);
			}

		case type::TYPE_RECORD:
			{
				const char* name = getString(v, 2);
				DataValue value;
				sr = ScriptDataValue::toValue(v, 3, value);
				if (SQ_SUCCEEDED(sr))
					((DataRecord*)o->_ref)->set(name, value);
			}
			break;

		case type::TYPE_OBJECT:
			{
				DataObject* obj = ((DataObject*)o->_ref);
				DataSchema* schema = obj->getDataSchema();
				// TODO: Optimize ScriptDataObject
				DataProperty* prop = schema ? schema->getProperty(getString(v, 2)) : NULL;
				if (prop)
				{
					DataValue value;
					sr = ScriptDataValue::toValue(v, 3, value);
					if (SQ_SUCCEEDED(sr))
						if (prop->setValue(obj, value))
							return SQ_OK;
				}
			}
			break;

		case type::TYPE_VOID:
		case type::TYPE_NULL:
			break;

		default:
			if (getInt(v, 2) == 0)
				sr = ScriptDataValue::toValue(v, 2, *o);
			break;
		}

		return sr;
	}

	NB_FUNC(_set)
	{
		type* o = self_noThrow(v);

		SQInteger sr = SQ_ERROR;

		if (o)
		{
			switch (o->_type)
			{
			case type::TYPE_ARRAY:
				{
					int index = getInt(v, 2);
					DataValue value;
					sr = ScriptDataValue::toValue(v, 3, value);
					if (SQ_SUCCEEDED(sr))
						((DataArray*)o->_ref)->set(index, value);
				}

			case type::TYPE_RECORD:
				{
					const char* name = getString(v, 2);
					DataValue value;
					sr = ScriptDataValue::toValue(v, 3, value);
					if (SQ_SUCCEEDED(sr))
						((DataRecord*)o->_ref)->set(name, value);
				}
				break;

			case type::TYPE_OBJECT:
				{
					DataObject* obj = ((DataObject*)o->_ref);
					DataSchema* schema = obj->getDataSchema();
					// TODO: Optimize ScriptDataObject
					DataProperty* prop = schema ? schema->getProperty(getString(v, 2)) : NULL;
					if (prop)
					{
						DataValue value;
						sr = ScriptDataValue::toValue(v, 3, value);
						if (SQ_SUCCEEDED(sr))
						{
							if (prop->setValue(obj, value))
								sr = SQ_OK;
							else
							{
								// clean failure
								sq_reseterror(v);
								sr = SQ_ERROR;
							}
						}
					}
				}
				break;

			case type::TYPE_VOID:
			case type::TYPE_NULL:
				sq_reseterror(v); // clean failure
				break;

			default:
				if (getInt(v, 2) == 0)
				{
					sr = ScriptDataValue::toValue(v, 2, *o);
				}
				break;
			}
		}
		
		return sr;
	}

	NB_FUNC(_nexti)
	{
		type* o = self(v);

		switch (o->_type)
		{
		case type::TYPE_ARRAY:			return NB_DataArrayItr::_nexti(v, (DataArray*)o->_ref);
		case type::TYPE_RECORD:			return NB_DataRecordItr::_nexti(v, (DataRecord*)o->_ref);
		default:
			if (sq_gettype(v, 2) == OT_NULL)
			{
				sq_pushinteger(v, 0);
				return 1;
			}
			else
			{
				return NULL;
			}
		}
	}

	NB_FUNC(_dump)
	{
		type* o = self(v);

		switch (o->_type)
		{
		case type::TYPE_ARRAY:
			((DataArray*)o->_ref)->dump();
			break;

		case type::TYPE_RECORD:
			((DataRecord*)o->_ref)->dump();
			break;

		case type::TYPE_OBJECT:
			{
				DataObject* obj = (DataObject*)o->_ref;
				DataSchema* schema = obj->getDataSchema();
				if (schema)
				{
					schema->dump(obj);
					break;
				}
				else
				{
					// fall through
				}
			}

		default:
			LOG(0, "-- value: %s\n", o->dumpString().c_str());
		}

		return push(v, true);
	}

	NB_FUNC(_tostring)
	{
		type* o = self(v);
		return pushFmt(v, "(DataValue: %s)", o->dumpString().c_str());
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::DataArray, RefCounted, incRefCount, decRefCount);

class NB_DataArray : TNitClass<DataArray>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(count),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(array=null)"),
			FUNC_ENTRY_H(toScript,		"(): array"),
			FUNC_ENTRY_H(toJson,		"(compact=true): string"),
			FUNC_ENTRY_H(append,		"(value)"),
			FUNC_ENTRY_H(insert,		"(index: int, value)"),
			FUNC_ENTRY_H(erase,			"(index: int, count=1)"),
			FUNC_ENTRY_H(clear,			"()"),
			FUNC_ENTRY_H(get,			"(index: int): DataValue"),
			FUNC_ENTRY_H(set,			"(index: int, value: object)"),
			FUNC_ENTRY	(_clone),
			FUNC_ENTRY	(_get),
			FUNC_ENTRY	(_set),
			FUNC_ENTRY	(_nexti),
			FUNC_ENTRY	(_dump),
			FUNC_ENTRY	(_call),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(count)					{ return push(v, self(v)->getCount()); }

	NB_CONS()
	{
		DataArray* self = new DataArray();
		if (!(isNone(v, 2) || isNull(v, 2)) && SQ_FAILED(ScriptDataValue::toArray(v, 2, self)))
		{
			Ref<DataArray> safe = self;
			return sq_throwerror(v, "can't create with given param");
		}
		setSelf(v, self);
		return SQ_OK;
	}

	NB_FUNC(toScript)					{ return ScriptDataValue::push(v, self(v), true); }
	NB_FUNC(toJson)						{ return push(v, DataValue(self(v)).toJson(optBool(v, 2, true))); }

	NB_FUNC(_clone)						{ return push(v, self(v)->clone()); }

	NB_FUNC(_call)						{ return ScriptDataValue::push(v, self(v)); }

	NB_FUNC(append)
	{
		DataValue value;
		SQRESULT sr = ScriptDataValue::toValue(v, 2, value);
		if (SQ_SUCCEEDED(sr))
			self(v)->append(value);
		return sr;
	}

	NB_FUNC(insert)
	{
		DataValue value;
		SQRESULT sr = ScriptDataValue::toValue(v, 3, value);
		if (SQ_SUCCEEDED(sr))
			self(v)->insert(getInt(v, 2), value);
		return sr;
	}

	NB_FUNC(erase)						{ self(v)->erase(getInt(v, 2), optInt(v, 3, 1)); return 0; }
	NB_FUNC(clear)						{ self(v)->clear(); return 0; }

	NB_FUNC(get)						
	{ 
		DataValue value = self(v)->get(getInt(v, 2));
		return push(v, value);
	}

	NB_FUNC(set)
	{
		int index = getInt(v, 2);
		DataValue value;
		SQRESULT sr = ScriptDataValue::toValue(v, 3, value);
		if (SQ_SUCCEEDED(sr))
			self(v)->set(index, value);
		return sr;
	}

	NB_FUNC(_get)
	{
		type* self = self_noThrow(v);

		SQObjectType t = sq_gettype(v, 2);
		if (t == OT_INTEGER || t == OT_FLOAT)
		{
			DataValue value = self ? self->get(getInt(v, 2)) : DataValue::Void();
			if (!value.isVoid())
				return ScriptDataValue::push(v, value);
		}

		// not found - clean failure
		sq_reseterror(v);
		return SQ_ERROR;
	}

	NB_FUNC(_set)
	{
		type* self = self_noThrow(v);
		int index = getInt(v, 2);

		if (self)
		{
			DataValue value;
			SQRESULT sr = ScriptDataValue::toValue(v, 3, value);
			if (SQ_SUCCEEDED(sr))
				self->set(index, value);
			return sr;
		}

		// not found - clean failure
		sq_reseterror(v);
		return SQ_ERROR;
	}

	NB_FUNC(_nexti)						{ return NB_DataArrayItr::_nexti(v, self(v)); }

	NB_FUNC(_dump)						{ self(v)->dump(); return push(v, true); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::DataNamespace, RefCounted, incRefCount, decRefCount);

class NB_DataNamespace : TNitClass<DataNamespace>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(name),
			PROP_ENTRY_R(fullName),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(find,			"(wildcard: string): DataKey[]"),
			FUNC_ENTRY_H(add,			"(name): DataKey"),
			FUNC_ENTRY_H(get,			"(name): DataKey"),
			FUNC_ENTRY	(_call),
			NULL
		};

		bind(v, props, funcs);

		addStatic(v, "global",			DataNamespace::getGlobal());
	}

	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }
	NB_PROP_GET(fullName)				{ return push(v, self(v)->getFullName()); }

	NB_FUNC(_call)						{ return push(v, get<DataNamespace>(v, 1)->add(getString(v, 3)).get()); }

	NB_FUNC(add)						{ return push(v, self(v)->add(getString(v, 2)).get()); }
	NB_FUNC(get)						{ return push(v, self(v)->get(getString(v, 2))); }

	NB_FUNC(find)
	{
		sq_newarray(v, 0);
		type* o = self(v);
		const char* pattern = getString(v, 2);

		for (type::Iterator itr = o->begin(), end = o->end(); itr != end; ++itr)
		{
			if (Wildcard::match(pattern, itr->first.c_str()))
				arrayAppend(v, -1, itr->second);
		}

		return 1;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::DataKey, RefCounted, incRefCount, decRefCount);

class NB_DataKey : TNitClass<DataKey>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(name),
			PROP_ENTRY_R(fullName),
			PROP_ENTRY_R(namespace),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY	(_tostring),
			FUNC_ENTRY	(_call),
			NULL
		};

		bind(v, props, funcs);

		addStatic(v, "global",			DataNamespace::getGlobal());
	}

	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }
	NB_PROP_GET(fullName)				{ return push(v, self(v)->getFullName()); }
	NB_PROP_GET(namespace)				{ return push(v, self(v)->getNamespace()); }

	NB_FUNC(_tostring)
	{
		type* o = self(v);
		return pushFmt(v, "(DataKey: '%s' -> %08X)", 
			o->getFullName().c_str(), o);
	}

	NB_FUNC(_call)						{ return push(v, self(v)->getName()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::DataRecord, RefCounted, incRefCount, decRefCount);

class NB_DataRecord : TNitClass<DataRecord>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(count),
			PROP_ENTRY_R(namespace),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(table=null)"),
			FUNC_ENTRY_H(toScript,		"(): table"),
			FUNC_ENTRY_H(toJson,		"(compact=true): string"),
			FUNC_ENTRY_H(get,			"(key: string): DataValue"),
			FUNC_ENTRY_H(set,			"(key: string, value: object)"),
			FUNC_ENTRY_H(delete,		"(key: string)"),
			FUNC_ENTRY_H(find,			"(pattern: string): DataValue[]"),
			FUNC_ENTRY_H(clear,			"()"),
			FUNC_ENTRY	(_clone),
			FUNC_ENTRY	(_get),
			FUNC_ENTRY	(_set),
			FUNC_ENTRY	(_nexti),
			FUNC_ENTRY	(_dump),
			FUNC_ENTRY	(_call),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(count)					{ return push(v, self(v)->getCount()); }
	NB_PROP_GET(namespace)				{ return push(v, self(v)->getNamespace()); }

	NB_CONS()
	{
		DataRecord* self = new DataRecord();
		if (!(isNone(v, 2) || isNull(v, 2)) && SQ_FAILED(ScriptDataValue::toRecord(v, 2, self)))
		{
			Ref<DataRecord> safe = self;
			return sq_throwerror(v, "can't create with given param");
		}
		setSelf(v, self);
		return SQ_OK;
	}

	NB_FUNC(toScript)					{ return ScriptDataValue::push(v, self(v), true); }
	NB_FUNC(toJson)						{ return push(v, DataValue(self(v)).toJson(optBool(v, 2, true))); }

	NB_FUNC(get)						
	{ 
		DataValue value = self(v)->get(getString(v, 2));
		return push(v, value);
	}

	NB_FUNC(set)
	{
		DataValue& value = self(v)->set(getString(v, 2), DataValue());
		return ScriptDataValue::toValue(v, 3, value);
	}

	NB_FUNC(find)
	{
		vector<DataValue>::type values;
		self(v)->find(getString(v, 2), values);

		sq_newarray(v, 0);
		for (uint i=0; i<values.size(); ++i)
			arrayAppend(v, -1, values[i]);

		return 1;
	}

	NB_FUNC(delete)						{ self(v)->Delete(getString(v, 2)); return 0; }
	NB_FUNC(clear)						{ self(v)->clear(); return 0; }

	NB_FUNC(_call)						{ return ScriptDataValue::push(v, self(v)); }

	NB_FUNC(_clone)						{ return push(v, self(v)->clone()); }

	NB_FUNC(_get)
	{
		type* self = self_noThrow(v);
		DataValue value;
		if (self) value = self->get(getString(v, 2));
		if (!value.isVoid())
			return ScriptDataValue::push(v, value);

		// not found - clean failure
		sq_reseterror(v);
		return SQ_ERROR;
	}

	NB_FUNC(_set)
	{
		type* o = self_noThrow(v);

		if (o)
		{
			DataValue& value = o->set(getString(v, 2), DataValue());
			return ScriptDataValue::toValue(v, 3, value);
		}

		return SQ_OK;
	}

	NB_FUNC(_nexti)						{ return NB_DataRecordItr::_nexti(v, self(v)); }

	NB_FUNC(_dump)						{ self(v)->dump(); return push(v, true); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::DataObject, RefCounted, incRefCount, decRefCount);

class NB_DataObject : TNitClass<DataObject>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(dataSchema),
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY	(_clone),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(dataSchema)				{ return push(v, self(v)->getDataSchema()); }

	NB_FUNC(_clone)						{ return push(v, self(v)->clone().get()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::DataSchema, RefCounted, incRefCount, decRefCount);

class NB_DataSchema : TNitClass<DataSchema>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(base),
			PROP_ENTRY_R(keyName),
			PROP_ENTRY_R(prefix),
			PROP_ENTRY_R(name),
			PROP_ENTRY_R(version),
			PROP_ENTRY_R(verMajor),
			PROP_ENTRY_R(verMinor),
			PROP_ENTRY_R(version),
			PROP_ENTRY_R(attributes),
			PROP_ENTRY_R(properties),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(create,		"(): DataObject"),
			FUNC_ENTRY_H(dump,			"(obj: DataObject)"),

			FUNC_ENTRY_H(register,		"[class] (class, keyName: string): DataSchema"),
			FUNC_ENTRY	(_dump),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(base)					{ return push(v, self(v)->getBase()); }
	NB_PROP_GET(keyName)				{ return push(v, self(v)->getKeyName()); }
	NB_PROP_GET(prefix)					{ return push(v, self(v)->getPrefix()); }
	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }
	NB_PROP_GET(version)				{ return push(v, self(v)->getVersion()); }
	NB_PROP_GET(verMajor)				{ return push(v, self(v)->getVerMajor()); }
	NB_PROP_GET(verMinor)				{ return push(v, self(v)->getVerMinor()); }
	NB_PROP_GET(attributes)				{ return push(v, self(v)->getAttributes()); }

	NB_PROP_GET(properties)
	{
		DataSchema::OrderedProperties& props = self(v)->getOrderedProperties();

		sq_newarray(v, 0);
		for (uint i=0; i<props.size(); ++i)
		{
			arrayAppend(v, -1, props[i]);
		}

		return 1;
	}

	NB_FUNC(create)						{ return push(v, self(v)->create().get()); }

	NB_FUNC(dump)						{ self(v)->dump(get<DataObject>(v, 2)); return 0; }
	NB_FUNC(_dump)						{ self(v)->dump(); return push(v, true); }

	NB_FUNC(register)
	{
		return ScriptDataSchema::Register(v, 2, getString(v, 3));
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WEAK(NIT_API, nit::DataProperty, NULL);

class NB_DataProperty : TNitClass<DataProperty>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(type),
			PROP_ENTRY_R(order),
			PROP_ENTRY_R(readOnly),
			PROP_ENTRY_R(save),
			PROP_ENTRY_R(attributes),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(type)					{ return push(v, DataValue::typeToStr(self(v)->getType())); }
	NB_PROP_GET(order)					{ return push(v, self(v)->getOrder()); }
	NB_PROP_GET(readOnly)				{ return push(v, self(v)->isReadOnly()); }
	NB_PROP_GET(save)					{ return push(v, self(v)->isSave()); }
	NB_PROP_GET(attributes)				{ return push(v, self(v)->getAttributes()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::DataSchemaLookup, RefCounted, incRefCount, decRefCount);

class NB_DataSchemaLookup : TNitClass<DataSchemaLookup>
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
			FUNC_ENTRY_H(current,		"[class] (): DataSchemaLookup"),
			FUNC_ENTRY_H(get,			"(name: string): DataSchema"),
			FUNC_ENTRY_H(find,			"(pattern: string): DataSchema[]"),
			FUNC_ENTRY_H(load,			"(name: string): DataSchema"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_FUNC(current)					{ return push(v, DataSchemaLookup::getCurrent()); }

	NB_FUNC(get)						{ return push(v, self(v)->get(getString(v, 2))); }
	NB_FUNC(load)						{ return push(v, self(v)->load(getString(v, 2))); }
	
	NB_FUNC(find)
	{
		vector<DataSchema*>::type results;
		self(v)->find(getString(v, 2), results);
		sq_newarray(v, 0);
		for (uint i=0; i<results.size(); ++i)
			arrayAppend(v, -1, results[i]);
		return 1;
	}
};

////////////////////////////////////////////////////////////////////////////////

ScriptDataAdapter::ScriptDataAdapter(ScriptDataSchema* schema, HSQUIRRELVM v, int instIdx)
: _schema(schema)
{
	ASSERT_THROW(sq_gettype(v, instIdx) == OT_INSTANCE, EX_INVALID_PARAMS);

	_peer = new ScriptPeer(v, instIdx);
}

void* ScriptDataAdapter::getRealPtr()
{
	HSQUIRRELVM v = _peer->getWorker();
	if (v == NULL) return NULL;

	SQObjectRef ref = _peer->getObjectRef(v);

	SQInstance* inst = sqi_instance(ref);

	void* realPtr = inst ? inst->_userpointer : NULL;
	return realPtr ? realPtr : inst;
}

Ref<DataObject> ScriptDataAdapter::adaptFunc(ScriptDataSchema* schema, HSQUIRRELVM v, int instIdx)
{
	return new ScriptDataAdapter(schema, v, instIdx);
}

SQInteger ScriptDataAdapter::pushFunc(ScriptDataSchema* schema, HSQUIRRELVM v, DataObject* object)
{
	ScriptDataAdapter* adapter = (ScriptDataAdapter*)object;
	return adapter->getScriptPeer()->pushObject(v);
}

bool ScriptDataAdapter::getFunc(DataObject* object, DataProperty* prop, DataValue& outValue)
{
	ScriptDataAdapter* adapter = (ScriptDataAdapter*)object;
	ScriptDataProperty* sprop = (ScriptDataProperty*)prop;
	return ScriptDataProperty::peerGet(adapter->getScriptPeer(), sprop, outValue);
}

bool ScriptDataAdapter::setFunc(DataObject* object, DataProperty* prop, DataValue& value)
{
	ScriptDataAdapter* adapter = (ScriptDataAdapter*)object;
	ScriptDataProperty* sprop = (ScriptDataProperty*)prop;
	return ScriptDataProperty::peerSet(adapter->getScriptPeer(), sprop, value);
}

void ScriptDataAdapter::onLoadBegin(DataObjectContext* context)
{
	HSQUIRRELVM v = _peer->getWorker();

	NitBind::push(v, context);
	ScriptResult sr = _peer->callMethod("onLoadBegin", 1, false);

	// TODO: Check sr and throw if error
}

void ScriptDataAdapter::onLoadEnd(DataObjectContext* context)
{
	HSQUIRRELVM v = _peer->getWorker();

	NitBind::push(v, context);
	ScriptResult sr = _peer->callMethod("onLoadEnd", 1, false);

	// TODO: Check sr and throw if error
}

void ScriptDataAdapter::onSaveBegin(DataObjectContext* context)
{
	// TODO: Check if the instance purged!

	HSQUIRRELVM v = _peer->getWorker();

	NitBind::push(v, context);
	ScriptResult sr = _peer->callMethod("onSaveBegin", 1, false);

	// TODO: Check sr and throw if error
}

void ScriptDataAdapter::onSaveEnd(DataObjectContext* context)
{
	HSQUIRRELVM v = _peer->getWorker();

	NitBind::push(v, context);
	ScriptResult sr = _peer->callMethod("onSaveEnd", 1, false);

	// TODO: Check sr and throw if error
}

////////////////////////////////////////////////////////////////////////////////

ScriptRefDataAdapter::ScriptRefDataAdapter(ScriptDataSchema* schema, IScriptRef* ref)
: _schema(schema), _ref(ref)
{

}

Ref<DataObject> ScriptRefDataAdapter::adaptFunc(ScriptDataSchema* schema, HSQUIRRELVM v, int instIdx)
{
	IScriptRef* ref = NitBind::getInterface<IScriptRef>(v, instIdx);

	return new ScriptRefDataAdapter(schema, ref);
}

SQInteger ScriptRefDataAdapter::pushFunc(ScriptDataSchema* schema, HSQUIRRELVM v, DataObject* object)
{
	ScriptRefDataAdapter* adapter = (ScriptRefDataAdapter*)object;
	return adapter->_ref->getScriptPeer()->pushObject(v);
}

bool ScriptRefDataAdapter::getFunc(DataObject* object, DataProperty* prop, DataValue& outValue)
{
	ScriptRefDataAdapter* adapter = (ScriptRefDataAdapter*)object;
	ScriptDataProperty* sprop = (ScriptDataProperty*)prop;
	return ScriptDataProperty::peerGet(adapter->_ref->getScriptPeer(), sprop, outValue);
}

bool ScriptRefDataAdapter::setFunc(DataObject* object, DataProperty* prop, DataValue& value)
{
	ScriptRefDataAdapter* adapter = (ScriptRefDataAdapter*)object;
	ScriptDataProperty* sprop = (ScriptDataProperty*)prop;
	return ScriptDataProperty::peerSet(adapter->_ref->getScriptPeer(), sprop, value);
}

void ScriptRefDataAdapter::onLoadBegin(DataObjectContext* context)
{
	ScriptPeer* peer = _ref->getScriptPeer();
	HSQUIRRELVM v = peer->getWorker();

	NitBind::push(v, context);
	ScriptResult sr = peer->callMethod("onLoadBegin", 1, false);

	// TODO: Check sr and throw if error
}

void ScriptRefDataAdapter::onLoadEnd(DataObjectContext* context)
{
	ScriptPeer* peer = _ref->getScriptPeer();
	HSQUIRRELVM v = peer->getWorker();

	NitBind::push(v, context);
	ScriptResult sr = peer->callMethod("onLoadEnd", 1, false);

	// TODO: Check sr and throw if error
}

void ScriptRefDataAdapter::onSaveBegin(DataObjectContext* context)
{
	ScriptPeer* peer = _ref->getScriptPeer();
	HSQUIRRELVM v = peer->getWorker();

	NitBind::push(v, context);
	ScriptResult sr = peer->callMethod("onSaveBegin", 1, false);

	// TODO: Check sr and throw if error
}

void ScriptRefDataAdapter::onSaveEnd(DataObjectContext* context)
{
	ScriptPeer* peer = _ref->getScriptPeer();
	HSQUIRRELVM v = peer->getWorker();

	NitBind::push(v, context);
	ScriptResult sr = peer->callMethod("onSaveEnd", 1, false);

	// TODO: Check sr and throw if error
}

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WEAK(NIT_API, nit::DataObjectContext, NULL);

class NB_DataObjectContext : TNitClass<DataObjectContext>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(objectID),
			PROP_ENTRY	(object),
			PROP_ENTRY	(internal),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(objectID)				{ return push(v, self(v)->getObjectID()); }
	NB_PROP_GET(object)					{ return push(v, self(v)->getObject()); }
	NB_PROP_GET(internal)				{ return push(v, self(v)->getInternal()); }

	NB_PROP_SET(object)					{ self(v)->setObject(get<DataObject>(v, 2)); return 0; }

	NB_PROP_SET(internal)
	{
		DataValue internal;
		SQRESULT sr = ScriptDataValue::toValue(v, 2, internal);
		if (SQ_FAILED(sr)) return sr;

		self(v)->setInternal(internal);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::DataContext, RefCounted, incRefCount, decRefCount);

class NB_DataContext : TNitClass<DataContext>
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

NB_TYPE_REF(NIT_API, nit::BinDataContext, RefCounted, incRefCount, decRefCount);

class NB_BinDataContext : TNitClass<BinDataContext>
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

NB_TYPE_REF(NIT_API, nit::BinDataSaver, RefCounted, incRefCount, decRefCount);

class NB_BinDataSaver : TNitClass<BinDataSaver>
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
			CONS_ENTRY_H(				"(context: BinDataContext = null)"),
			FUNC_ENTRY_H(save,			"(value: object, w: StreamWriter)"),
			FUNC_ENTRY_H(saveCompressed,"(value: object, w: StreamWriter)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, new BinDataSaver(opt<BinDataContext>(v, 2, NULL))); return SQ_OK; }

	NB_FUNC(save)
	{
		DataValue value;
		StreamWriter* w = get<StreamWriter>(v, 3);
		SQRESULT sr = ScriptDataValue::toValue(v, 2, value);
		if (SQ_SUCCEEDED(sr))
			self(v)->save(value, w);

		return sr;
	}

	NB_FUNC(saveCompressed)
	{
		DataValue value;
		StreamWriter* w = get<StreamWriter>(v, 3);
		SQRESULT sr = ScriptDataValue::toValue(v, 2, value);
		if (SQ_SUCCEEDED(sr))
			self(v)->saveCompressed(value, w);

		return sr;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::JsonDataSaver, JsonPrinter, incRefCount, decRefCount);

class NB_JsonDataSaver : TNitClass<JsonDataSaver>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(recordSorted),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(w: StreamWriter=null, context: DataContext=null)"),
			FUNC_ENTRY_H(printValue,	"(value: object)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(recordSorted)			{ return push(v, self(v)->isRecordSorted()); }

	NB_PROP_SET(recordSorted)			{ self(v)->setRecordSorted(getBool(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, new JsonDataSaver(opt<StreamWriter>(v, 2, NULL))); return SQ_OK; }

	NB_FUNC(printValue)
	{
		DataValue value;
		SQRESULT sr = ScriptDataValue::toValue(v, 2, value);
		if (SQ_SUCCEEDED(sr))
			self(v)->printValue(value);

		return sr;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::PlistDataSaver, XmlPrinter, incRefCount, decRefCount);

class NB_PlistDataSaver : TNitClass<PlistDataSaver>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(recordSorted),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(w: StreamWriter=null, context: DataContext=null)"),
			FUNC_ENTRY_H(printHeader,	"()"),
			FUNC_ENTRY_H(printValue,	"(value: object)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(recordSorted)			{ return push(v, self(v)->isRecordSorted()); }

	NB_PROP_SET(recordSorted)			{ self(v)->setRecordSorted(getBool(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, new PlistDataSaver(opt<StreamWriter>(v, 2, NULL), opt<DataContext>(v, 3, NULL))); return SQ_OK; }

	NB_FUNC(printHeader)				{ self(v)->printHeader(); return 0; }

	NB_FUNC(printValue)
	{
		DataValue value;
		SQRESULT sr = ScriptDataValue::toValue(v, 2, value);
		if (SQ_SUCCEEDED(sr))
			self(v)->printValue(value);

		return sr;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::Database, RefCounted, incRefCount, decRefCount);

class NB_Database : TNitClass<Database>
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
			CONS_ENTRY_H(				"(locator: FileLocator, filename: string) // creates db file if not found\n"
										"(uriPath: string)"),

			FUNC_ENTRY_H(prepare,		"(sql: string): DBStatement"),
			FUNC_ENTRY_H(exec,			"(sql: string) // results are ignored"),
			FUNC_ENTRY_H(fetch,			"(sql: string): array<table>"),

			FUNC_ENTRY_H(newLocator,	"(name: string, path: string): DBBlobLocator // path: '[db.]table/blob_column[.id_column]'"),

			FUNC_ENTRY_H(print,			"(sql: string, maxRows = 100, maxWidth = 40)"),

			FUNC_ENTRY	(_call),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		if (is<FileLocator>(v, 2))
			setSelf(v, Database::open(get<FileLocator>(v, 2), getString(v, 3)));
		else
			setSelf(v, Database::open(getString(v, 2)));
		return SQ_OK;
	}

	NB_FUNC(prepare)					{ return push(v, self(v)->prepare(getString(v, 2))); }
	NB_FUNC(exec)						{ self(v)->exec(getString(v, 2)); return 0; }

	NB_FUNC(newLocator)					{ return push(v, self(v)->newLocator(getString(v, 2), getString(v, 3))); }

	struct PrintContext
	{
		HSQUIRRELVM v;
		int line;
		int maxRows;
		uint maxWidth;
		vector<uint>::type Widths;
		vector<StringVector>::type Lines;
	};

	static int PrintCallback(void* c, int numColumns, char** texts, char** columnNames)
	{
		if (numColumns == 0)
			return 0;

		PrintContext* ctx = (PrintContext*) c;
		int line = ctx->line++;

		if (line >= ctx->maxRows)
		{
			ctx->maxRows = 0;
			return 0;
		}

		ctx->Widths.resize(numColumns);

		if (line == 0)
		{
			ctx->Lines.push_back(StringVector());
			StringVector& columns = ctx->Lines.back();
			columns.resize(numColumns);
			for (int i=0; i<numColumns; ++i)
			{
				if (columnNames[i])
					columns[i] = columnNames[i];
				else
					columns[i] = "(null)";

				if (columns[i].length() > ctx->maxWidth)
					columns[i].resize(ctx->maxWidth);
				if (ctx->Widths[i] < columns[i].length())
					ctx->Widths[i] = columns[i].length();
			}
		}

		ctx->Lines.push_back(StringVector());
		StringVector& row = ctx->Lines.back();
		row.resize(numColumns);
		for (int i=0; i<numColumns; ++i)
		{
			if (texts[i])
				row[i] = texts[i];
			else
				row[i] = "(null)";

			if (row[i].length() > ctx->maxWidth)
			{
				row[i].resize(ctx->maxWidth-3);
				row[i].append("...");
			}
			if (ctx->Widths[i] < row[i].length())
				ctx->Widths[i] = row[i].length();
		}

		return 0;
	}

	static int DoPrint(HSQUIRRELVM v, type* self, const char* sql, int maxRows, int maxWidth)
	{
		if (maxWidth < 4) maxWidth = 4;

		PrintContext ctx;
		ctx.v = v;
		ctx.line = 0;
		ctx.maxRows = maxRows;
		ctx.maxWidth = maxWidth;

		self->exec(sql, PrintCallback, &ctx);

		String line;

		LOG(0, "-- %d row(s) from '%s'\n", ctx.line, sql);

		for (uint i = 0; i<ctx.Lines.size(); ++i)
		{
			line.resize(0);

			StringVector& row = ctx.Lines[i];
			for (uint c = 0; c < row.size(); ++c)
			{
				line.append(row[c]);
				line.append(ctx.Widths[c] - row[c].length() + 1, ' ');
			}

			if (i == 0)
			{
				LOG(0, "--      %s\n", line.c_str());

				line.resize(0);
				for (uint c = 0; c < ctx.Widths.size(); ++c)
				{
					line.append(ctx.Widths[c], '-');
					line.push_back(' ');
				}
				LOG(0, "--      %s\n", line.c_str());
			}
			else
			{
				LOG(0, "-- %04d %s\n", i, line.c_str());
			}
		}

		if (ctx.maxRows == 0)
			LOG(0, "-- ....\n");

		return ctx.line;
	}

	NB_FUNC(print)
	{
		int numRows = DoPrint(v, self(v), getString(v, 2), optInt(v, 3, 100), optInt(v, 4, 40));
		return push(v, numRows);
	}

	NB_FUNC(_call)
	{
		int maxRows = optInt(v, 4, 0);
#if defined(NIT_SHIPPING)
		self(v)->exec(getString(v, 3)); 
#else
		if (maxRows > 0)
			DoPrint(v, self(v), getString(v, 3), maxRows, optInt(v, 5, 40));
		else
			self(v)->exec(getString(v, 3));
#endif
		return 0;
	}

	struct FetchContext
	{
		HSQUIRRELVM V;
		SQInteger ArrayIdx;
	};

	static int FetchCallback(void* c, int numColumns, char** texts, char** columnNames)
	{
		if (numColumns == 0)
			return 0;

		FetchContext* ctx = (FetchContext*)c;
		HSQUIRRELVM v = ctx->V;
		sq_newtable(v);
		for (int i=0; i<numColumns; ++i)
		{
			if (columnNames[i] == NULL)
				sq_pushinteger(v, i+1);
			else
				sq_pushstring(v, columnNames[i], -1);

			if (texts[i] == NULL)
				sq_pushnull(v);
			else
				sq_pushstring(v, texts[i], -1);

			sq_newslot(v, -3, false);
		}
		sq_arrayappend(v, ctx->ArrayIdx);
		return 0;
	}

	NB_FUNC(fetch)
	{ 
		const char* sql = getString(v, 2);

		sq_newarray(v, 0);

		FetchContext ctx;
		ctx.V = v;
		ctx.ArrayIdx = toAbsIdx(v, -1);

		self(v)->exec(sql, FetchCallback, &ctx);

		return 1;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::Database::Query, RefCounted, incRefCount, decRefCount);

class NB_DatabaseQuery : TNitClass<Database::Query>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(stepResult),
			PROP_ENTRY_R(numParams),
			PROP_ENTRY_R(numColumns),
			PROP_ENTRY_R(readOnly),
			PROP_ENTRY	(sql),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(getParamIndex,	"(paramName: string): int"),
			FUNC_ENTRY_H(getParamName,	"(paramIndex: int, goodname=true): string"),

			FUNC_ENTRY_H(reset,			"()"),
			FUNC_ENTRY_H(clearBindings, "()"),

			FUNC_ENTRY_H(bind,			"(paramIndex: int, value)\n"
										"(paramName: string, value)"),

			FUNC_ENTRY_H(step,			"(): bool // true if you can call Get methods"),
			FUNC_ENTRY_H(exec,			"(): int // returns total changes by this query"),

			FUNC_ENTRY_H(getDatabaseName, "(column: int): string"),
			FUNC_ENTRY_H(getTableName,	"(column: int): string"),
			FUNC_ENTRY_H(getOriginName,	"(column: int): string"),
			FUNC_ENTRY_H(getColumnName,	"(column: int): string"),
			FUNC_ENTRY_H(getValue,		"(column: int): value"),

			NULL
		};

		bind(v, props, funcs);

		pushClass<type>(v);
		sq_pushstring(v, "binding", -1); sq_pushnull(v); sq_newslot(v, -3, false);
		sq_pushstring(v, "mapping", -1); sq_pushnull(v); sq_newslot(v, -3, false);
		sq_poptop(v);
	}

	NB_PROP_GET(stepResult)				{ return push(v, self(v)->getStepResult()); }
	NB_PROP_GET(numParams)				{ return push(v, self(v)->getNumParams()); }
	NB_PROP_GET(numColumns)				{ return push(v, self(v)->getNumColumns()); }
	NB_PROP_GET(readOnly)				{ return push(v, self(v)->isReadOnly()); }
	NB_PROP_GET(sql)					{ return push(v, self(v)->getSql()); }

	NB_PROP_SET(sql)					{ self(v)->setSql(getString(v, 2)); return 0; }

	NB_FUNC(clearBindings)				{ self(v)->clearBindings(); return 0; }
	NB_FUNC(reset)						{ self(v)->reset(); return 0; }
	NB_FUNC(step)						{ return push(v, self(v)->step()); }
	NB_FUNC(exec)						{ return push(v, self(v)->exec()); }

	NB_FUNC(getParamIndex)				{ return push(v, self(v)->getParamIndex(getString(v, 2))); }
	NB_FUNC(getParamName)				{ return push(v, self(v)->getParamName(getInt(v, 2), optBool(v, 3, true))); }
	NB_FUNC(getDatabaseName)			{ return push(v, self(v)->getDatabaseName(getInt(v, 2))); }
	NB_FUNC(getTableName)				{ return push(v, self(v)->getTableName(getInt(v, 2))); }
	NB_FUNC(getOriginName)				{ return push(v, self(v)->getOriginName(getInt(v, 2))); }
	NB_FUNC(getColumnName)				{ return push(v, self(v)->getColumnName(getInt(v, 2))); }
	NB_FUNC(getValue)					{ return PushColumnValue(v, self(v), getInt(v, 2)); }

	static SQRESULT SimpleBind(HSQUIRRELVM v)
	{
		type* o = self(v);
		int paramIndex = 0;

		if (isString(v, 2))
			paramIndex = o->getParamIndex(getString(v, 2));
		else
			paramIndex = getInt(v, 2);

		if (paramIndex == 0)
			return sq_throwerror(v, "invalid param index");

		if (isNone(v, 3))
			return sq_throwerror(v, "value expected");

		return BindValue(v, o, paramIndex, 3);
	}

	static SQRESULT BindValue(HSQUIRRELVM v, type* self, int paramIndex, int valueIdx)
	{
		valueIdx = toAbsIdx(v, valueIdx);

		switch (sq_gettype(v, valueIdx))
		{
		case OT_NULL:		self->bindNull(paramIndex); return SQ_OK;
		case OT_INTEGER:	{ int value; sq_getinteger(v, valueIdx, &value); self->bind(paramIndex, value); } return SQ_OK;
		case OT_FLOAT:		{ float value; sq_getfloat(v, valueIdx, &value); self->bind(paramIndex, value); } return SQ_OK;
		case OT_STRING:		{ const char* value; sq_getstring(v, valueIdx, &value); self->bind(paramIndex, value); } return SQ_OK;

		case OT_TABLE:
		case OT_ARRAY:
			{
				DataValue value;
				SQRESULT sr = ScriptDataValue::toValue(v, valueIdx, value);
				if (SQ_FAILED(sr)) return sr;

				self->bindValue(paramIndex, value);
			}
			return SQ_OK;

		case OT_INSTANCE:
			if (is<MemoryBuffer>(v, valueIdx))
			{
				self->bind(paramIndex, get<MemoryBuffer>(v, valueIdx));
			}
			else if (is<StreamSource>(v, valueIdx))
			{
				Ref<StreamReader> rdr = get<StreamSource>(v, valueIdx)->open();
				Ref<MemoryBuffer> buf = rdr->buffer();
				self->bind(paramIndex, buf);
			}
			else if (is<DataValue>(v, valueIdx))
			{
				self->bindValue(paramIndex, *get<DataValue>(v, valueIdx));
			}
			else
			{
				DataValue value;
				SQRESULT sr = ScriptDataValue::toValue(v, valueIdx, value);
				if (SQ_FAILED(sr)) return sr;

				self->bindValue(paramIndex, value);
			}
			return SQ_OK;

		default:
			return sq_throwerror(v, "not supported type");
		}
	}

	NB_FUNC(bind)
	{
		return SimpleBind(v);
	}

	static int PushColumnValue(HSQUIRRELVM v, type* self, int column)
	{
		type::ColumnType t = self->getType(column);

		switch (t)
		{
		case type::TYPE_INTEGER:	sq_pushinteger(v, self->getInt(column)); break;
		case type::TYPE_FLOAT:		sq_pushfloat(v, (float)self->getDouble(column)); break;
		case type::TYPE_NULL:		sq_pushnull(v); break;

		case type::TYPE_TEXT:
		case type::TYPE_BLOB:
		default:
			int len = 0;
			const char* text = self->getText(column, &len);
			sq_pushstring(v, text, len);
		}

		return 1;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::BlobLocator, StreamLocator, incRefCount, decRefCount);

class NB_BlobLocator : public TNitClass<BlobLocator>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(database),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(name: string, db: Database, tblName, blobColumn: string, idColumn=\"rowid\", dbName=\"\")"),
			FUNC_ENTRY_H(open,			"(streamName: string): StreamReader"),
			FUNC_ENTRY_H(modify,		"(streamName: string): StreamWriter"),
			FUNC_ENTRY_H(update,		"(streamName: string, newSize: int): StreamWriter"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(database)				{ return push(v, self(v)->getDatabase()); }

	NB_CONS()							{ setSelf(v, new BlobLocator(getString(v, 2), get<Database>(v, 3), getString(v, 4), getString(v, 5), optString(v, 6, "rowid"), optString(v, 7, ""))); return SQ_OK; }

	NB_FUNC(open)						{ return push(v, self(v)->open(getString(v, 2))); }
	NB_FUNC(modify)						{ return push(v, self(v)->modify(getString(v, 2))); }
	NB_FUNC(update)						{ return push(v, self(v)->update(getString(v, 2), getInt(v, 3))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::BlobSource, StreamSource, incRefCount, decRefCount);

class NB_BlobSource : public TNitClass<BlobSource>
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
			FUNC_ENTRY_H(modify,		"(): StreamWriter"),
			FUNC_ENTRY_H(update,		"(newSize: int): StreamWriter"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_FUNC(modify)						{ return push(v, self(v)->modify()); }
	NB_FUNC(update)						{ return push(v, self(v)->update(getInt(v, 2))); }
};

////////////////////////////////////////////////////////////////////////////////

SQRESULT NitLibData(HSQUIRRELVM v)
{
	NB_DataValue::Register(v);
	NB_DataArray::Register(v);
	NB_DataNamespace::Register(v);
	NB_DataKey::Register(v);
	NB_DataRecord::Register(v);
	NB_DataObject::Register(v);
	NB_DataSchema::Register(v);
	NB_DataProperty::Register(v);
	NB_DataSchemaLookup::Register(v);

	NB_DataObjectContext::Register(v);
	NB_DataContext::Register(v);
	NB_BinDataContext::Register(v);

	NB_BinDataSaver::Register(v);
	NB_JsonDataSaver::Register(v);
	NB_PlistDataSaver::Register(v);

	NB_Database::Register(v);
	NB_DatabaseQuery::Register(v);
	NB_BlobLocator::Register(v);
	NB_BlobSource::Register(v);

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
