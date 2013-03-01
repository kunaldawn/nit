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

#include "nit/data/DataSaver.h"
#include "nit/data/DataChannel.h"
#include "nit/io/ZStream.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

BinDataSaver::BinDataSaver(BinDataContext* context /*= NULL*/)
{
	if (context == NULL)
		context = new BinDataContext();

	_context = context;
	_writer = NULL;
}

void BinDataSaver::save(const DataValue& value, StreamWriter* w)
{
	Ref<StreamWriter> safe = w;

	uint32 crc = 0;

	{
		Ref<CalcCRC32Writer> crcw = new CalcCRC32Writer();
		Ref<StreamWriter> writer = new ShadowWriter(w, crcw);

		_writer = writer;

		// Signature will be included crc calculation .
		write((uint32)NIT_DATA_SIGNATURE);

		writeValue(value);

		_writer = NULL;
		crc = crcw->getValue();
	}

	// Do not include crc itself in crc calculation
	if (w->writeRaw(&crc, sizeof(crc)) != sizeof(crc))
		NIT_THROW_FMT(EX_WRITE, "can't write checksum");
}

void BinDataSaver::saveCompressed(const DataValue& value, StreamWriter* w)
{
	Ref<StreamWriter> safe = w;

	uint32 crc = 0;

	{
		Ref<CalcCRC32Writer> crcw = new CalcCRC32Writer();
		Ref<StreamWriter> writer = new ShadowWriter(w, crcw);

		_writer = writer;

		// Signature will be included crc calculation,
		// but not be included in compression
		// Loading is transparent to compression so this is safe.
		write((uint32)NIT_ZDATA_SIGNATURE);

		writer = new ShadowWriter(new ZStreamWriter(w), crcw);
		_writer = writer;

		writeValue(value);

		_writer = NULL;
		crc = crcw->getValue();
	}

	// Do not include crc itself in crc calculation
	if (w->writeRaw(&crc, sizeof(crc)) != sizeof(crc))
		NIT_THROW_FMT(EX_WRITE, "can't write checksum");
}

void BinDataSaver::writeString(const char* str, uint32 len)
{
	write(len);
	_writer->writeRaw(str, len);
}

void BinDataSaver::writeBlob(const void* blob, uint32 size)
{
	write(size);
	_writer->writeRaw(blob, size);
}

void BinDataSaver::writeKey(DataKey* key)
{
	_context->writeKey(_writer, key);
}

void BinDataSaver::writeValue(const DataValue& value)
{
    DataValue::Type type = value.getType();
    
	switch (type)
	{
	case DataValue::TYPE_NULL:			writeType(type); break;
	case DataValue::TYPE_VOID:			writeType(type); break;

	case DataValue::TYPE_BOOL:			writeType(type); write<uint8>(value.getData<bool>()); break;
	case DataValue::TYPE_INT:			writeType(type); write(value.getData<int>()); break;
	case DataValue::TYPE_INT64:			writeType(type); write(value.getData<int64>()); break;
	case DataValue::TYPE_FLOAT:			writeType(type); write(value.getData<float>()); break;
	case DataValue::TYPE_DOUBLE:		writeType(type); write(value.getData<double>()); break;
	case DataValue::TYPE_STRING:		writeType(type); writeString(value.getStringPtr(), value.getStringSize()); break;
	case DataValue::TYPE_BLOB:			writeType(type); writeBlob(value.getBlobPtr(), value.getBlobSize()); break;

	case DataValue::TYPE_TIMESTAMP:		writeType(type); write(value.getData<Timestamp>()); break;

	case DataValue::TYPE_VECTOR2:		
	case DataValue::TYPE_SIZE2:	
	case DataValue::TYPE_FLOAT2:		writeType(type); write(value.getData<Float2>()); break;

	case DataValue::TYPE_VECTOR3:		
	case DataValue::TYPE_SIZE3:	
	case DataValue::TYPE_FLOAT3:		writeType(type); write(value.getData<Float3>()); break;

	case DataValue::TYPE_VECTOR4:		
	case DataValue::TYPE_QUAT:	
	case DataValue::TYPE_FLOAT4:		writeType(type); write(value.getData<Float4>()); break;

	case DataValue::TYPE_MATRIX3:
	case DataValue::TYPE_FLOAT3X3:		writeType(type); write(value.getData<Float3x3>()); break;

	case DataValue::TYPE_MATRIX4:
	case DataValue::TYPE_FLOAT4X4:		writeType(type); write(value.getData<Float4x4>()); break;

	case DataValue::TYPE_ARRAY:			writeType(type); writeArray(value.getRef<DataArray>()); break;
	case DataValue::TYPE_RECORD:		writeType(type); writeRecord(value.getRef<DataRecord>()); break;
	case DataValue::TYPE_OBJECT:		writeType(type); writeObject(value.getRef<DataObject>()); break;
	case DataValue::TYPE_BUFFER:		writeType(type); writeBuffer(value.getRef<MemoryBuffer>()); break;
	case DataValue::TYPE_KEY:			writeType(type); writeKey(value.getRef<DataKey>()); break;

	default:							NIT_THROW(EX_NOT_SUPPORTED);
	}
}

void BinDataSaver::writeArray(DataArray* array)
{
	uint32 count = array->getCount();
	write(count);

	for (uint i=0; i < count; ++i)
	{
		writeValue(array->get(i));
	}
}

void BinDataSaver::writeRecord(DataRecord* record)
{
	int index = 0;
	HashTable::Pair* pair;

	while ((pair = record->getTable().next(index)))
	{
		if (pair->second.isVoid()) continue;

		writeKey(pair->first);
		writeValue(pair->second);
	}

	writeKey(NULL);
}

void BinDataSaver::writeObject(Ref<DataObject> object)
{
	DataSchema* schema = object->getDataSchema();

	if (schema == NULL)
		NIT_THROW(EX_NOT_SUPPORTED);

	uint32 objectId = _context->getObjectId(object);

	if (objectId != 0)
	{
		// TODO: Implement the case when channel already has such an object and update needed
		// (channel -> mark something on context -> check on write)
		write<int32>(objectId);
		return;
	}

	DataObjectContext* objContext = _context->beginSaveObject(object);
	object = objContext->getObject();
	schema = object->getDataSchema();
	objectId = objContext->getObjectID();

	write<int32>(objectId);

	writeKey(schema->getKey());

	writeValue(objContext->getInternal());

	DataSchema::OrderedProperties& props = schema->getOrderedProperties();
	for (uint i = 0; i < props.size(); ++i)
	{
		DataProperty* prop = props[i];
		if (prop->isReadOnly() || !prop->isSave())
			continue;

		DataValue value;
		if (!prop->getValue(object, value))
			NIT_THROW(EX_INVALID_STATE);

		writeKey(prop->getKey());
		writeValue(value);
	}

	writeKey(NULL);

	_context->endSaveObject();
}

void BinDataSaver::writeBuffer(MemoryBuffer* buffer)
{
	write((uint32)buffer->getSize());
	write((uint32)buffer->getBlockSize());
	buffer->save(_writer);
}

////////////////////////////////////////////////////////////////////////////////

static bool LessKeyName(DataKey* a, DataKey* b)
{
	return a->getName() < b->getName();
}

////////////////////////////////////////////////////////////////////////////////

JsonDataSaver::JsonDataSaver(StreamWriter* w, DataContext* context)
: JsonPrinter(w)
{
	if (context == NULL)
		context = new DataContext();

	_context = context;
	_sortRecordKeys = false;
}

void JsonDataSaver::printValue(const DataValue& v)
{
	switch (v.getType())
	{
	case DataValue::TYPE_RECORD:		printRecord(v.getRef<DataRecord>()); break;
	case DataValue::TYPE_ARRAY:			printArray(v.getRef<DataArray>()); break;
	case DataValue::TYPE_OBJECT:		printObject(v.getRef<DataObject>()); break;

	case DataValue::TYPE_VOID:			valueNull(); break;
	case DataValue::TYPE_NULL:			valueNull(); break;
	case DataValue::TYPE_BOOL:			value(v.getData<uint8>() != 0); break;
	case DataValue::TYPE_INT:			value(v.getData<int>()); break;
	case DataValue::TYPE_INT64:			value(v.getData<int64>()); break;
	case DataValue::TYPE_DOUBLE:		value(v.getData<double>()); break;
	case DataValue::TYPE_FLOAT:			value(v.getData<float>()); break;
	case DataValue::TYPE_STRING:		value(v.getStringPtr(), v.getStringSize()); break; // treat as a plain string

	case DataValue::TYPE_TIMESTAMP:		value(v.getData<Timestamp>().getIso8601()); break;

	case DataValue::TYPE_BUFFER:
	case DataValue::TYPE_BLOB:			printBlob(v); break;

	case DataValue::TYPE_KEY:			value(v.getRef<DataKey>()->getName()); break;

	case DataValue::TYPE_VECTOR2:
	case DataValue::TYPE_FLOAT2:		printFloats(NULL, (float*)&v.getData<Float2>(), 2); break;

	case DataValue::TYPE_VECTOR3:
	case DataValue::TYPE_FLOAT3:		printFloats(NULL, (float*)&v.getData<Float3>(), 3); break;

	case DataValue::TYPE_VECTOR4:
	case DataValue::TYPE_QUAT:
	case DataValue::TYPE_FLOAT4:		printFloats(NULL, (float*)&v.getData<Float4>(), 4); break;

	case DataValue::TYPE_MATRIX3:
	case DataValue::TYPE_FLOAT3X3:		printFloats(NULL, (float*)&v.getData<Float3x3>(), 9); break;

	case DataValue::TYPE_MATRIX4:
	case DataValue::TYPE_FLOAT4X4:		printFloats(NULL, (float*)&v.getData<Float4x4>(), 16); break;

	default:							NIT_THROW_FMT(EX_SYNTAX, "not supported type");
	}
}

void JsonDataSaver::printBlob(const DataValue& value)
{
	comma();

	if (value.getType() == DataValue::TYPE_BLOB)
	{
		printBlob(value.getBlobPtr(), value.getBlobSize());
	}
	else if (value.getType() == DataValue::TYPE_BUFFER)
	{
		MemoryBuffer::Access access(value.getRef<MemoryBuffer>());
		printBlob(access.getMemory(), access.getSize());
	}

	needComma();
}

void JsonDataSaver::printBlob(const void* memory, size_t size)
{
	// Write as BASE64 encoded string
	putch('\"');
	Ref<Base64Encoder> enc = new Base64Encoder(_writer);
	enc->writeRaw(memory, size);
	enc = NULL;
	putch('\"');
}

void JsonDataSaver::printFloats(const char* type, float* floats, size_t numFloats)
{
	beginArray();

	for (uint i=0; i<numFloats; ++i)
	{
		value(floats[i]);
	}

	endArray();
}

void JsonDataSaver::printRecord(DataRecord* rec)
{
	beginObject();

	if (_sortRecordKeys)
	{
		vector<DataKey*>::type keys;
		for (DataRecord::Iterator itr = rec->begin(), end = rec->end(); itr != end; ++itr)
		{
			DataKey* k = itr->first;
			keys.push_back(k);
		}

		std::sort(keys.begin(), keys.end(), LessKeyName);

		for (uint i=0; i<keys.size(); ++i)
		{
			DataKey* k = keys[i];
			DataValue v = rec->get(k);

			if (v.isVoid()) continue;

			key(k->getName());
			printValue(v);
		}
	}
	else
	{
		for (DataRecord::Iterator itr = rec->begin(), end = rec->end(); itr != end; ++itr)
		{
			DataKey* k = itr->first;
			DataValue& v = itr->second;

			if (v.isVoid()) continue;

			key(k->getName());
			printValue(v);
		}
	}

	endObject();
}

void JsonDataSaver::printArray(DataArray* array)
{
	beginArray();

	for (uint i = 0; i < array->getCount(); ++i)
	{
		printValue(array->get(i));
	}

	endArray();
}

void JsonDataSaver::printObject(Ref<DataObject> object)
{
	DataSchema* schema = object->getDataSchema();

	if (schema == NULL)
		NIT_THROW(EX_NOT_SUPPORTED);

	uint32 objectId = _context->getObjectId(object);

	if (objectId != 0)
	{
		// We have no plan to use JSON on DataChannel so just write a simple id.
		beginObject();
		key("@objectid");	value(objectId);
		endObject();
		return;
	}

	DataObjectContext* objContext = _context->beginSaveObject(object);
	object = objContext->getObject();
	schema = object->getDataSchema();
	objectId = objContext->getObjectID();

	beginObject();

	key("@objectid");	value(objectId);
	key("@schema");		value(schema->getKey()->getName());

	if (!objContext->getInternal().isVoidOrNull())
	{
		key("@internal");	printValue(objContext->getInternal());
	}

	DataSchema::OrderedProperties& props = schema->getOrderedProperties();
	for (uint i=0; i < props.size(); ++i)
	{
		DataProperty* prop = props[i];
		if (prop->isReadOnly() || !prop->isSave())
			continue;

		DataValue value;
		if (!prop->getValue(object, value))
			NIT_THROW(EX_INVALID_STATE);

		key(prop->getKey()->getName());
		printValue(value);
	}

	endObject();

	_context->endSaveObject();
}

////////////////////////////////////////////////////////////////////////////////

// TODO: Implement a nit way and remove this code

class Base64_1
{
public:
	// refer to: https://svn.apache.org/repos/asf/jackrabbit/branches/2.0/jackrabbit-jcr-commons/src/main/java/org/apache/jackrabbit/util/Base64.java

	Base64_1()
	{
		_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

		_padChar = '=';

		for (int i=0; i < COUNT_OF(_decodeTable); ++i)
			_decodeTable[i] = 0x7F;

		for (int i=0; i < COUNT_OF(_chars); ++i)
			_decodeTable[_chars[i]] = (uint8)i;
	}

    size_t CalcEncodedLength(size_t dataLength) {
		size_t encLen = dataLength * 4 / 3;
        encLen += (encLen + 4) % 4;
        return encLen;
    }

    size_t guessDecodedLength(size_t encLength) {
        long decLen = encLength * 3 / 4;
        return decLen + 3;
    }

	void encode(uint8* data, size_t size, size_t off, size_t len, Ref<StreamWriter> writer)
	{
		if (len == 0) return;

		if (off >= size || len + off > size)
			NIT_THROW(EX_INVALID_PARAMS);

		char enc[4];

		while (len >= 3) 
		{
			int i = ((data[off] & 0xff) << 16)
				+ ((data[off + 1] & 0xff) << 8)
				+ (data[off + 2] & 0xff);
			enc[0] = _chars[i >> 18];
			enc[1] = _chars[(i >> 12) & 0x3f];
			enc[2] = _chars[(i >> 6) & 0x3f];
			enc[3] = _chars[i & 0x3f];
			writer->writeRaw(enc, 4);
			off += 3;
			len -= 3;
		}

		// add padding if necessary
		if (len == 1) 
		{
			int i = data[off] & 0xff;
			enc[0] = _chars[i >> 2];
			enc[1] = _chars[(i << 4) & 0x3f];
			enc[2] = _padChar;
			enc[3] = _padChar;
			writer->writeRaw(enc, 4);
		} 
		else if (len == 2) 
		{
			int i = ((data[off] & 0xff) << 8) + (data[off + 1] & 0xff);
			enc[0] = _chars[i >> 10];
			enc[1] = _chars[(i >> 4) & 0x3f];
			enc[2] = _chars[(i << 2) & 0x3f];
			enc[3] = _padChar;
			writer->writeRaw(enc, 4);
		}
	}

	void decode(uint8* chars, size_t size, size_t off, size_t len, Ref<StreamWriter> out)
	{
		if (len == 0) return;

		if (off >= size || len + off > size)
			NIT_THROW(EX_INVALID_PARAMS);

		char chunk[4];
		uint8 dec[3];

		size_t posChunk = 0;

		// decode in chunks of 4 characters
		for (size_t i = off; i < (off + len); i++) 
		{
			uint8 c = chars[i];
			if ((c < COUNT_OF(_decodeTable) && _decodeTable[c] != 0x7f) || c == _padChar)
			{
				chunk[posChunk++] = c;
				if (posChunk == size) 
				{
					int b0 = _decodeTable[chunk[0]];
					int b1 = _decodeTable[chunk[1]];
					int b2 = _decodeTable[chunk[2]];
					int b3 = _decodeTable[chunk[3]];

					if (chunk[3] == _padChar && chunk[2] == _padChar) 
					{
						dec[0] = (byte) ((b0 << 2 & 0xfc) | (b1 >> 4 & 0x3));
						out->writeRaw(dec, 1);
					} 
					else if (chunk[3] == _padChar) 
					{
						dec[0] = (byte) ((b0 << 2 & 0xfc) | (b1 >> 4 & 0x3));
						dec[1] = (byte) ((b1 << 4 & 0xf0) | (b2 >> 2 & 0xf));
						out->writeRaw(dec, 2);
					} 
					else 
					{
						dec[0] = (byte) ((b0 << 2 & 0xfc) | (b1 >> 4 & 0x3));
						dec[1] = (byte) ((b1 << 4 & 0xf0) | (b2 >> 2 & 0xf));
						dec[2] = (byte) ((b2 << 6 & 0xc0) | (b3 & 0x3f));
						out->writeRaw(dec, 3);
					}

					posChunk = 0;
				}
			} 
			else NIT_THROW_FMT(EX_INVALID_PARAMS, "specified data is not base64 encoded");
		}
	}

private:
	const char* _chars;
	char _padChar;
	uint8 _decodeTable[128];
};

////////////////////////////////////////////////////////////////////////////////

PlistDataSaver::PlistDataSaver(StreamWriter* w, DataContext* context)
: XmlPrinter(w)
{
	if (context == NULL)
		context = new DataContext();

	_context = context;
	_compact = false;
	_sortRecordKeys = true;
}

void PlistDataSaver::printHeader()
{
	print("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	newline();
	print("<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">");
	newline();
	beginElem("plist");
	attr("version", "1.0");
}

void PlistDataSaver::printValue(const DataValue& v)
{
	switch (v.getType())
	{
	case DataValue::TYPE_RECORD:	printRecord(v.getRef<DataRecord>()); break;
	case DataValue::TYPE_ARRAY:		printArray(v.getRef<DataArray>()); break;
	case DataValue::TYPE_OBJECT:	printObject(v.getRef<DataObject>()); break;

	case DataValue::TYPE_VOID:		break;
	case DataValue::TYPE_NULL:		beginElem("string"); text(""); endElem(); break; // PList has no null representation

	case DataValue::TYPE_BOOL:		if (v.getData<uint8>() != 0) beginElem("true"); else beginElem("false"); endElem(); break;

	case DataValue::TYPE_TIMESTAMP:	beginElem("date"); text(v.getData<Timestamp>().getIso8601()); endElem(); break;

	case DataValue::TYPE_INT:		beginElem("integer"); text(StringUtil::format("%d", v.getData<int>())); endElem(); break;
	case DataValue::TYPE_INT64:		beginElem("integer"); text(StringUtil::format("%lld", v.getData<int64>())); endElem(); break;
	case DataValue::TYPE_FLOAT:		beginElem("real"); text(StringUtil::format("%f", v.getData<float>())); endElem(); break;
	case DataValue::TYPE_DOUBLE:	beginElem("real"); text(StringUtil::format("%lf", v.getData<double>())); endElem(); break;
	case DataValue::TYPE_STRING:	beginElem("string"); text(v.getStringPtr(), v.getStringSize()); endElem(); break;
	case DataValue::TYPE_KEY:		beginElem("string"); text(v.getRef<DataKey>()->getName()); endElem(); break; // treat as a plain string

	case DataValue::TYPE_FLOAT2:
	case DataValue::TYPE_FLOAT3:
	case DataValue::TYPE_FLOAT4:
	case DataValue::TYPE_FLOAT3X3:
	case DataValue::TYPE_FLOAT4X4:
	case DataValue::TYPE_VECTOR2:
	case DataValue::TYPE_VECTOR3:
	case DataValue::TYPE_VECTOR4:
	case DataValue::TYPE_QUAT:
	case DataValue::TYPE_MATRIX3:
	case DataValue::TYPE_MATRIX4:
		{
			DataValue copy(v);
			copy.toString();
			beginElem("string");
			attr("type", DataValue::typeToStr(v.getType()));
			text(copy.getStringPtr(), copy.getStringSize());
			endElem();
		}
		break;

	case DataValue::TYPE_BLOB:
		{
			text("");
			Ref<Base64Encoder> encoder = new Base64Encoder(_writer);
			encoder->writeRaw(v.getBlobPtr(), v.getBlobSize());
		}
		break;

	case DataValue::TYPE_BUFFER:
		{
			text("");
			Ref<Base64Encoder> encoder = new Base64Encoder(_writer);
			v.getRef<MemoryBuffer>()->save(encoder);
		}
		break;

	default:						
		NIT_THROW_FMT(EX_SYNTAX, "not supported type");
	}
}

void PlistDataSaver::printArray(DataArray* array)
{
	beginElem("array");

	for (uint i = 0; i < array->getCount(); ++i)
	{
		printValue(array->get(i));
	}

	endElem();
}

void PlistDataSaver::printRecord(DataRecord* rec)
{
	beginElem("dict");

	if (_sortRecordKeys)
	{
		vector<DataKey*>::type keys;
		for (DataRecord::Iterator itr = rec->begin(), end = rec->end(); itr != end; ++itr)
		{
			DataKey* key = itr->first;
			keys.push_back(key);
		}

		std::sort(keys.begin(), keys.end(), LessKeyName);

		for (uint i=0; i<keys.size(); ++i)
		{
			DataKey* key = keys[i];
			DataValue value = rec->get(key);

			if (value.isVoid()) continue;

			beginElem("key"); text(key->getName()); endElem();
			printValue(value);
		}
	}
	else
	{
		for (DataRecord::Iterator itr = rec->begin(), end = rec->end(); itr != end; ++itr)
		{
			DataKey* key = itr->first;
			DataValue& value = itr->second;

			if (value.isVoid()) continue;

			beginElem("key"); text(key->getName()); endElem();
			printValue(value);
		}
	}

	endElem();
}

void PlistDataSaver::printObject(Ref<DataObject> object)
{
	DataSchema* schema = object->getDataSchema();

	if (schema == NULL)
		NIT_THROW(EX_NOT_SUPPORTED);

	uint32 objectId = _context->getObjectId(object);

	if (objectId != 0)
	{
		beginElem("dict");
		beginElem("key"); text("@objectid"); endElem();
		beginElem("integer"); text(StringUtil::format("ud", objectId)); endElem();
		endElem();
		return;
	}

	DataObjectContext* objContext = _context->beginSaveObject(object);
	object = objContext->getObject();
	schema = object->getDataSchema();
	objectId = objContext->getObjectID();

	beginElem("dict");

	beginElem("key"); text("@objectid"); endElem();
	beginElem("integer"); text(StringUtil::format("%u", objectId)); endElem();

	beginElem("key"); text("@schema"); endElem();
	beginElem("string"); text(schema->getKey()->getName()); endElem();

	if (!objContext->getInternal().isVoidOrNull())
	{
		beginElem("key"); text("@internal"); endElem();
		printValue(objContext->getInternal());
	}

	DataSchema::OrderedProperties& props = schema->getOrderedProperties();
	for (uint i=0; i < props.size(); ++i)
	{
		DataProperty* prop = props[i];
		if (prop->isReadOnly() || !prop->isSave())
			continue;

		DataValue value;
		if (!prop->getValue(object, value))
			NIT_THROW(EX_INVALID_STATE);

		beginElem("key"); text(prop->getKey()->getName()); endElem();
		printValue(value);
	}

	endElem();

	_context->endSaveObject();
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
