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

#include "nit/data/DataLoader.h"
#include "nit/data/DataChannel.h"
#include "nit/io/MemoryBuffer.h"
#include "nit/io/ZStream.h"

#define XML_BUILDING_EXPAT 1
#include "expat/expat.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

BinDataLoader::BinDataLoader(BinDataContext* context /*= NULL*/)
{
	if (context == NULL)
		context = new BinDataContext();

	_context = context;
	_reader = NULL;
}

void BinDataLoader::load(DataValue& outValue, Ref<StreamReader> r)
{
	uint32 loadedCRC = 0;

	{
		Ref<CalcCRC32Writer> crcw = new CalcCRC32Writer();
		Ref<StreamReader> reader = new CopyReader(r, crcw);
		_reader = reader;

		// signature should be included in crc calculation
		uint32 signature = 0;
		if (reader->readRaw(&signature, sizeof(signature)) != sizeof(signature))
			NIT_THROW(EX_CORRUPTED);

		if (signature == NIT_DATA_SIGNATURE)
		{
			readValue(outValue);

			_reader = NULL;
		}
		else if (signature == NIT_ZDATA_SIGNATURE)
		{
			reader = new CopyReader(new ZStreamReader(r), crcw);
			_reader = reader;

			readValue(outValue);

			_reader = NULL;
		}
		else
		{
			_reader = NULL;
			NIT_THROW(EX_NOT_SUPPORTED);
		}

		loadedCRC = crcw->getValue();
	}

	// crc value itself should not be included in crc calculation :)
	uint32 crc = 0;
	if (r->readRaw(&crc, sizeof(crc)) != sizeof(crc))
	{
		outValue.toVoid();
		NIT_THROW(EX_CORRUPTED);
	}

	if (crc != loadedCRC)
	{
		// crc check failed
		outValue.toVoid();
		NIT_THROW_FMT(EX_CORRUPTED, "crc mismatch");
	}
}

void BinDataLoader::readValue(DataValue& outValue)
{
	DataValue::Type type = readType();

	switch (type)
	{
	case DataValue::TYPE_NULL:			outValue.toNull(); break;
	case DataValue::TYPE_VOID:			outValue.toVoid(); break;

	case DataValue::TYPE_BOOL:			readData<uint8>(type, outValue); break;
	case DataValue::TYPE_INT:			readData<int>(type, outValue); break;
	case DataValue::TYPE_INT64:			readData<int64>(type, outValue); break;
	case DataValue::TYPE_FLOAT:			readData<float>(type, outValue); break;
	case DataValue::TYPE_DOUBLE:		readData<double>(type, outValue); break;
	case DataValue::TYPE_STRING:		readString(outValue); break;
	case DataValue::TYPE_BLOB:			readBlob(outValue); break;

	case DataValue::TYPE_TIMESTAMP:		readData<Timestamp>(type, outValue); break;

	case DataValue::TYPE_VECTOR2:
	case DataValue::TYPE_SIZE2:
	case DataValue::TYPE_FLOAT2:		readData<Float2>(type, outValue); break;

	case DataValue::TYPE_VECTOR3:
	case DataValue::TYPE_SIZE3:
	case DataValue::TYPE_FLOAT3:		readData<Float3>(type, outValue); break;

	case DataValue::TYPE_VECTOR4:
	case DataValue::TYPE_QUAT:
	case DataValue::TYPE_FLOAT4:		readData<Float4>(type, outValue); break;

	case DataValue::TYPE_MATRIX3:
	case DataValue::TYPE_FLOAT3X3:		readData<Float3x3>(type, outValue); break;

	case DataValue::TYPE_MATRIX4:
	case DataValue::TYPE_FLOAT4X4:		readData<Float3x3>(type, outValue); break;

	case DataValue::TYPE_ARRAY:			readArray(outValue); break;
	case DataValue::TYPE_RECORD:		readRecord(outValue); break;
	case DataValue::TYPE_OBJECT:		readObject(outValue); break;
	case DataValue::TYPE_BUFFER:		readBuffer(outValue); break;
	case DataValue::TYPE_KEY:			outValue = readKey(); break;

	default:							NIT_THROW(EX_NOT_SUPPORTED);
	}
}

void BinDataLoader::readArray(DataValue& outValue)
{
	Ref<DataArray> array = new DataArray();

	uint32 count = read<uint32>();

	for (uint i=0; i < count; ++i)
	{
		DataValue value;
		readValue(value);
		array->append(value);
	}

	outValue = array;
}

void BinDataLoader::readRecord(DataValue& outValue)
{
	Ref<DataRecord> record = new DataRecord();

	while (true)
	{
		Ref<DataKey> key = readKey();

		if (key == NULL)
			break;

		DataValue value;
		readValue(value);

		record->set(key, value);
	}

	outValue = record;
}

Ref<DataKey> BinDataLoader::readKey()
{
	return _context->readKey(_reader);
}

void BinDataLoader::readObject(DataValue& outValue)
{
	uint32 objectId = read<uint32>();

	Ref<DataObject> object = _context->getObject(objectId);

	if (object)
	{
		// TODO: Implement the case when channel already has such an object and update needed (determine by written value)
		outValue = object;
		return;
	}

	Ref<DataKey> schemaKey = readKey();
	object = _context->createObject(schemaKey);

	DataValue internal;
	readValue(internal);

	DataObjectContext* objContext = _context->beginLoadObject(object, objectId, internal);
	object = objContext->getObject();
	Ref<DataSchema> schema = object->getDataSchema();
	internal.toVoid();

	while (true)
	{
		Ref<DataKey> key = readKey();

		if (key == NULL)
			break;

		DataValue value;
		readValue(value);
		DataProperty* prop = schema->getProperty(key);
		if (prop == NULL)
			NIT_THROW(EX_INVALID_STATE);

		if (!prop->setValue(object, value))
			NIT_THROW(EX_INVALID_STATE);
	}

	outValue = _context->endLoadObject();
}

void BinDataLoader::readBuffer(DataValue& outValue)
{
	uint32 size = read<uint32>();
	uint32 blockSize = read<uint32>();

	Ref<MemoryBuffer> buffer = new MemoryBuffer(blockSize);
	if (buffer->load(_reader, 0, size) != size)
		NIT_THROW(EX_CORRUPTED);

	outValue = buffer;
}

////////////////////////////////////////////////////////////////////////////////

PlistDataLoader::PlistDataLoader(DataContext* context)
{
	if (context == NULL)
		context = new DataContext();

	_context	= context;
	_parser	= NULL;
	_cdata		= new MemoryBuffer();

	cleanup();
}

void PlistDataLoader::cleanup()
{
	if (_parser)
		XML_ParserFree(XML_Parser(_parser));

	_parser	= NULL;
	_rootValue	= NULL;
	_cdata->clear();
	_stateStack.clear();
	_dictStack.clear();
	_arrayStack.clear();
	_keyStack.clear();
}

static void* expat_malloc(size_t size)
{
	return NIT_ALLOC(size);
}

static void* expat_realloc(void* ptr, size_t size)
{
	return NIT_REALLOC(ptr, 0, size);
}

static void expat_free(void* ptr)
{
	NIT_DEALLOC(ptr, 0);
}

void PlistDataLoader::load(DataValue& value, StreamReader* reader)
{
	if (_parser || _rootValue)
		NIT_THROW(EX_INVALID_STATE);

	cleanup();

	_error.clear();

	XML_Memory_Handling_Suite mm;
	mm.malloc_fcn	= expat_malloc;
	mm.realloc_fcn	= expat_realloc;
	mm.free_fcn		= expat_free;

	XML_Parser parser = XML_ParserCreate_MM(NULL, &mm, NULL);
	_parser = parser;
	_rootValue = &value;
	_stateStack.push_back(ES_ROOT);

	XML_SetUserData(parser, this);
	XML_SetElementHandler(parser, startElementHandler, endElementHandler);
	XML_SetCharacterDataHandler(parser, characterDataHandler);

	Ref<StreamReader> r = reader;

	char buf[4096];

	while (true)
	{
		int len = r->readRaw(buf, sizeof(buf));
		int done = len < sizeof(buf);

		if (XML_Parse(parser, buf, len, done) == XML_STATUS_ERROR)
		{
			_error = StringUtil::format("can't parse '%s' line %u: %s",
				reader->getUrl().c_str(),
				XML_GetCurrentLineNumber(parser),
				_error.empty() ? XML_ErrorString(XML_GetErrorCode(parser)) : _error.c_str()
				);

			break;
		}

		if (done) break;
	}

	cleanup();

	if (!_error.empty())
		NIT_THROW_FMT(EX_SYNTAX, "%s", _error.c_str());
}

void PlistDataLoader::onStartElement(const char* name, const char** attrs)
{
	_cdata->clear();

	if (strcmp(name, "dict") == 0 || strcmp(name, "object") == 0)
	{
		_stateStack.push_back(ES_DICT);
		_dictStack.push_back(new DataRecord(_context->getNamespace()));
	}
	else if (strcmp(name, "array") == 0)
	{
		_stateStack.push_back(ES_ARRAY);
		_arrayStack.push_back(new DataArray());
	}
	else if (strcmp(name, "object") == 0)
	{
		NIT_THROW(EX_NOT_SUPPORTED);
	}
}

DataValue* PlistDataLoader::newValue()
{
	ASSERT_THROW(!_stateStack.empty(), EX_INVALID_STATE);

	switch (_stateStack.back())
	{
	case ES_ROOT:
		_stateStack.pop_back();
		return _rootValue;

	case ES_DICT:
		{
			ASSERT_THROW(!_keyStack.empty(), EX_INVALID_STATE);
			ASSERT_THROW(!_dictStack.empty(), EX_INVALID_STATE);

			const String& key = _keyStack.back();
			DataValue* ret = &_dictStack.back()->set(key, DataValue());
			_keyStack.pop_back();

			return ret;
		}

	case ES_ARRAY:
		ASSERT_THROW(!_arrayStack.empty(), EX_INVALID_STATE);

		return &_arrayStack.back()->append(DataValue());

	default:
		ASSERT_THROW(false, EX_INVALID_STATE);
	}
}

void PlistDataLoader::onEndElement(const char* name)
{
	if (strcmp(name, "dict") == 0)
	{
		Ref<DataRecord> dict = _dictStack.back();
		_dictStack.pop_back();
		_stateStack.pop_back();
		*newValue() = dict;
		// TODO: for object, implement utilizing '@schema' key of a dict
	}
	else if (strcmp(name, "array") == 0)
	{
		Ref<DataArray> array = _arrayStack.back();
		_arrayStack.pop_back();
		_stateStack.pop_back();
		*newValue() = array;
	}
	else if (strcmp(name, "key") == 0)
	{
		_keyStack.push_back(_cdata->toString());
	}
	else if (strcmp(name, "string") == 0)
	{
		newValue()->loadString(new MemoryBuffer::Reader(_cdata, NULL), _cdata->getSize());
	}
	else if (strcmp(name, "integer") == 0)
	{
		*newValue() = DataValue::parse<int64>(_cdata->toString().c_str());
	}
	else if (strcmp(name, "real") == 0)
	{
		*newValue() = DataValue::parse<double>(_cdata->toString().c_str());
	}
	else if (strcmp(name, "true") == 0)
	{
		*newValue() = true;
	}
	else if (strcmp(name, "false") == 0)
	{
		*newValue() = false;
	}
	else if (strcmp(name, "date") == 0)
	{
		// TODO: we're ready to implement
		LOG(0, "?? date not supported yet - treated as string\n");
		*newValue() = _cdata;
	}
	else if (strcmp(name, "data") == 0)
	{
		Ref<MemoryBuffer::Writer> w = new MemoryBuffer::Writer();
		w->copy(new Base64Decoder(new MemoryBuffer::Reader(_cdata, NULL)));
		*newValue() = w->getBuffer();
	}
}

void PlistDataLoader::onCharacterData(const char* s, int len)
{
	_cdata->pushBack(s, len);
}

void PlistDataLoader::error(const char* fmt, ...)
{
	if (_parser)
	{
		XML_StopParser((XML_Parser)_parser, false);
	}

	va_list args;
	va_start(args, fmt);
	_error = StringUtil::vformat(fmt, args);
	va_end(args);
}

void PlistDataLoader::startElementHandler(void* ctx, const char* name, const char** attrs)
{
	PlistDataLoader* loader = (PlistDataLoader*)ctx;
	try
	{
		loader->onStartElement(name, attrs);
	}
	catch (Exception& ex)
	{
		loader->error("%s", ex.getFullDescription().c_str());
	}
}

void PlistDataLoader::endElementHandler(void* ctx, const char* name)
{
	PlistDataLoader* loader = (PlistDataLoader*)ctx;
	try
	{
		loader->onEndElement(name);
	}
	catch (Exception& ex)
	{
		loader->error("%s", ex.getFullDescription().c_str());
	}
}

void PlistDataLoader::characterDataHandler(void* ctx, const char* s, int len)
{
	PlistDataLoader* loader = (PlistDataLoader*)ctx;
	try
	{
		loader->onCharacterData(s, len);
	}
	catch (Exception& ex)
	{
		loader->error("%s", ex.getFullDescription().c_str());
	}
}

////////////////////////////////////////////////////////////////////////////////

JsonDataLoader::JsonDataLoader(DataContext* context) 
{
	if (context == NULL)
		context = new DataContext();

	_context = context;
}

void JsonDataLoader::load(DataValue& outValue, StreamReader* reader)
{
	cleanup();

	Json json(this);
	json.parse(reader);

	outValue = _rootObject;

	cleanup();
}

void JsonDataLoader::cleanup()
{
	_objStack.clear();
	_arrayStack.clear();

	_rootObject.toVoid();
}

DataRecord* JsonDataLoader::pushObject()
{
	DataRecord* obj = new DataRecord(_context->getNamespace());
	_objStack.push_back(obj);
	return obj;
}

Ref<DataRecord> JsonDataLoader::popObject()
{
	Ref<DataRecord> rec = _objStack.back();
	_objStack.pop_back();
	return rec;
}

DataArray* JsonDataLoader::pushArray()
{
	DataArray* array = new DataArray();
	_arrayStack.push_back(array);
	return array;
}

Ref<DataArray> JsonDataLoader::popArray()
{
	Ref<DataArray> array = _arrayStack.back();
	_arrayStack.pop_back();
	return array;
}

void JsonDataLoader::element(const DataValue& value)
{
	if (!_arrayStack.empty())
	{
		topArray()->append(value);
		return;
	}

	// no root object yet - make it as root object
	if (_rootObject.isVoid())
	{
		_rootObject = value;
		return;
	}

	// multiple root objects -> contain all of them in a root array and make the array as root object
	pushArray();
	topArray()->append(_rootObject);

	_rootObject.toVoid();
}

void JsonDataLoader::documentEnd()
{
	if (!_arrayStack.empty())
	{
		_rootObject = topArray();
		popArray();
	}
}

void JsonDataLoader::pairObjectEnd(const String& key)
{
	Ref<DataRecord> rec = popObject();
	Ref<DataObject> obj = toDataObject(rec);

	if (obj)
		topObject()->set(key, obj.get());
	else
		topObject()->set(key, rec.get());
}

void JsonDataLoader::elementObjectEnd()
{
	Ref<DataRecord> rec = popObject();
	Ref<DataObject> obj = toDataObject(rec);

	if (obj)
		element(DataValue(obj));
	else
		element(DataValue(rec));
}

Ref<DataObject> JsonDataLoader::toDataObject(DataRecord* rec)
{
	if (k_ObjectID == NULL)
		k_ObjectID = _context->getNamespace()->add("@objectid");

	uint objectId = (uint)rec->get(k_ObjectID).Default(0).toInt64();

	if (objectId == 0) return NULL;

	Ref<DataObject> object = _context->getObject(objectId);

	if (object) 
		return object;

	if (k_Schema == NULL)
		k_Schema = _context->getNamespace()->add("@schema");

	Ref<DataKey> schemaKey = _context->getNamespace()->add(rec->get(k_Schema).toString());
	object = _context->createObject(schemaKey);

	if (k_Internal == NULL)
		k_Internal = _context->getNamespace()->add("@internal");

	DataObjectContext* objContext = _context->beginLoadObject(object, objectId, rec->get(k_Internal));
	object = objContext->getObject();
	Ref<DataSchema> schema = object->getDataSchema();

	// json does not preserve property order, so process by the order of OrderedProperties 
	DataSchema::OrderedProperties& props = schema->getOrderedProperties();
	for (uint i = 0; i < props.size(); ++i)
	{
		DataProperty* prop = props[i];
		if (prop->isReadOnly() || !prop->isSave())
			continue;

		DataValue value = rec->get(prop->getKey());
		if (value.isVoid()) continue;

		if (!prop->setValue(object, value))
			NIT_THROW(EX_INVALID_STATE);
	}

	return _context->endLoadObject();
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
