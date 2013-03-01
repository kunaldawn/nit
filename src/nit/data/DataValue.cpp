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

#include "nit/data/DataValue.h"
#include "nit/data/DataSchema.h"
#include "nit/data/DataLoader.h"
#include "nit/data/DataSaver.h"
#include "nit/data/DataChannel.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

bool DataValue::parse(const char* str, int& outValue)
{
	outValue = atoi(str);
	return true;
}

bool DataValue::parse(const char* str, int64& outValue)
{
	return sscanf(str, "%lld", &outValue) == 1;
}

bool DataValue::parse(const char* str, float& outValue)
{
	outValue = (float)atof(str);
	return true;
}

bool DataValue::parse(const char* str, double& outValue)
{
	outValue = atof(str);
	return true;
}

bool DataValue::parse(const char* str, Float2& outValue)
{
	return sscanf(str, "[%f,%f]", &outValue.x, &outValue.y) == 2;
}

bool DataValue::parse(const char* str, Float3& outValue)
{
	return sscanf(str, "[%f,%f,%f]", &outValue.x, &outValue.y, &outValue.z) == 3;
}

bool DataValue::parse(const char* str, Float4& outValue)
{
	return sscanf(str, "[%f,%f,%f,%f]", &outValue.x, &outValue.y, &outValue.z, &outValue.w) == 4;
}

bool DataValue::parse(const char* str, Float3x3& outValue)
{
	return sscanf(str, "[%f,%f,%f,%f,%f,%f,%f,%f,%f]",
		&outValue.a.x, &outValue.a.y, &outValue.a.z,
		&outValue.b.x, &outValue.b.y, &outValue.b.z,
		&outValue.c.x, &outValue.c.y, &outValue.c.z) == 9;
}

bool DataValue::parse(const char* str, Float4x4& outValue)
{
	return sscanf(str, "[%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f]",
		&outValue.a.x, &outValue.a.y, &outValue.a.z, &outValue.a.w,
		&outValue.b.x, &outValue.b.y, &outValue.b.z, &outValue.b.w,
		&outValue.c.x, &outValue.c.y, &outValue.c.z, &outValue.c.w,
		&outValue.d.x, &outValue.d.y, &outValue.d.z, &outValue.d.w) == 16;
}

bool DataValue::parse(const char* str, bool& outValue)
{
	if (_strcmpi(str, "true") == 0)
		outValue = true;
	else if (_strcmpi(str, "yes") == 0)
		outValue = true;
	else if (_strcmpi(str, "1") == 0)
		outValue = true;
	else
		outValue = false;

	return true;
}

static DataValue LoadJSONString(const char* str, size_t len)
{
	DataValue loaded;
	Ref<TempMemoryReader> tr = new TempMemoryReader(str, len);

	Ref<JsonDataLoader> loader = new JsonDataLoader();
	loader->load(loaded, tr);

	return loaded;
}

static DataValue LoadBlobData(const void* blob, size_t size)
{
	DataValue value;
	Ref<BinDataLoader> loader = new BinDataLoader();
	loader->load(value, new TempMemoryReader(blob, size));
	return value;
}

static DataValue LoadBlobData(StreamReader* r)
{
	DataValue value;
	Ref<BinDataLoader> loader = new BinDataLoader();
	loader->load(value, r);
	return value;
}

template <typename TValue>
static DataValue DataToBlob(uint8 type, const TValue& data)
{
	uint8 buf[sizeof(uint32) + sizeof(uint8) + sizeof(TValue) + sizeof(uint32)];
	size_t size = sizeof(buf);

	*(uint32*)&buf[0] = NIT_DATA_SIGNATURE;
	buf[4] = type;
	memcpy(&buf[5], &data, sizeof(TValue));

	uint32 crc = StreamUtil::calcCrc32(buf, size - sizeof(crc));
	memcpy(&buf[size - sizeof(crc)], &crc, sizeof(crc));

	return DataValue(buf, size);
}

template <typename TValue>
static Ref<MemoryBuffer> DataToBuffer(uint8 type, const TValue& data)
{
	size_t size = sizeof(uint32) + sizeof(uint8) + sizeof(TValue) + sizeof(uint32);
	Ref<MemoryBuffer> buffer = new MemoryBuffer(size);
	buffer->resize(size);

	uint8* buf = NULL;
	if (buffer->getBlock(0, buf, size))
	{
		*(uint32*)&buf[0] = NIT_DATA_SIGNATURE;
		buf[4] = type;
		memcpy(&buf[5], &data, sizeof(TValue));

		uint32 crc = StreamUtil::calcCrc32(buf, size - sizeof(crc));
		memcpy(&buf[size - sizeof(crc)], &crc, sizeof(crc));

		return buffer;
	}

	assert(false);
	return NULL;
}

template <typename TValue>
static bool BlobToData(const void* blob, size_t size, uint8 targetType, TValue& outValue)
{
	if (size == sizeof(uint32) + sizeof(uint8) + sizeof(TValue) + sizeof(uint32))
	{
		uint8* buf = (uint8*)blob;
		uint32 signature = *(uint32*)buf;
		uint8 type = buf[4];
		if (signature == NIT_DATA_SIGNATURE && type == targetType)
		{
			uint32 crc = StreamUtil::calcCrc32(buf, size - sizeof(crc));
			uint32 loadedCRC;
			memcpy(&loadedCRC, &buf[size - sizeof(crc)], sizeof(crc));

			if (loadedCRC == crc)
			{
				memcpy(&outValue, &buf[5], sizeof(TValue));
				return true;
			}
		}
	}

	DataValue v = LoadBlobData(blob, size);

	if (v.getType() != targetType)
		v.convertTo((DataValue::Type)targetType);

	outValue = v.getData<TValue>();
	return true;
}

template <typename TValue>
static bool BufferToData(Ref<MemoryBuffer> buffer, uint8 targetType, TValue& outValue)
{
	MemoryBuffer::Access access(buffer);

	return BlobToData(access.getMemory(), access.getSize(), targetType, outValue);
}

bool DataValue::convertToBool()
{
	bool v = false;

	switch (_type)
	{
	case TYPE_BOOL:						return getData<uint8>() != 0;

	case TYPE_INT:						v = getData<int>() != 0; break;
	case TYPE_INT64:					v = getData<int>() != 0; break;
	case TYPE_DOUBLE:					v = getData<double>() != 0.0; break;
	case TYPE_FLOAT:					v = getData<float>() != 0.0f; break;

	case TYPE_TIMESTAMP:				v = getData<Timestamp>().isValid(); break;

	case TYPE_VECTOR2:
	case TYPE_SIZE2:
	case TYPE_FLOAT2:					v = true; break;

	case TYPE_VECTOR3:
	case TYPE_SIZE3:
	case TYPE_FLOAT3:					v = true; break;

	case TYPE_VECTOR4:
	case TYPE_QUAT:
	case TYPE_FLOAT4:					v = true; break;

	case TYPE_MATRIX3:
	case TYPE_FLOAT3X3:					v = true; break;

	case TYPE_MATRIX4:
	case TYPE_FLOAT4X4:					v = true; break;

	case TYPE_STRING:					parse(getStringPtr(), v); break;

	case TYPE_BLOB:						{ uint8 value = 0; BlobToData(getBlobPtr(), getBlobSize(), DataValue::TYPE_BOOL, value); v = value != 0; } break;
	case TYPE_BUFFER:					{ uint8 value = 0; BufferToData(getRef<MemoryBuffer>(), DataValue::TYPE_BOOL, value); v = value != 0; } break;

	case TYPE_ARRAY:					v = getRef<DataArray>()->get(0).toBool(); break;
	case TYPE_RECORD:					break;

	case TYPE_VOID:
	case TYPE_NULL:						break;

	default:							assert(0);
	}

	setData(TYPE_BOOL, v);

	return v;
}

int DataValue::convertToInt()
{
	int v = 0;

	switch (_type)
	{
	case TYPE_INT:						return getData<int>();

	case TYPE_BOOL:						v = (int)getData<uint8>(); break;
	case TYPE_INT64:					v = (int)getData<int64>(); break;
	case TYPE_DOUBLE:					v = (int)getData<double>(); break;
	case TYPE_FLOAT:					v = (int)getData<float>(); break;

	case TYPE_TIMESTAMP:				v = (int)getData<Timestamp>().getUnixTime64(); break;

	case TYPE_VECTOR2:
	case TYPE_SIZE2:
	case TYPE_FLOAT2:					v = (int)getData<Float2>().x; break;

	case TYPE_VECTOR3:
	case TYPE_SIZE3:
	case TYPE_FLOAT3:					v = (int)getData<Float3>().x; break;

	case TYPE_VECTOR4:
	case TYPE_QUAT:
	case TYPE_FLOAT4:					v = (int)getData<Float4>().x; break;

	case TYPE_MATRIX3:
	case TYPE_FLOAT3X3:					v = (int)getData<Float3x3>().a.x; break;

	case TYPE_MATRIX4:
	case TYPE_FLOAT4X4:					v = (int)getData<Float4x4>().a.x; break;

	case TYPE_STRING:					parse(getStringPtr(), v); break;
	case TYPE_KEY:						parse(getRef<DataKey>()->getName().c_str(), v); break;

	case TYPE_BLOB:						BlobToData(getBlobPtr(), getBlobSize(), DataValue::TYPE_INT, v); break;
	case TYPE_BUFFER:					BufferToData(getRef<MemoryBuffer>(), DataValue::TYPE_INT, v); break;

	case TYPE_VOID:
	case TYPE_NULL:						break;

	case TYPE_ARRAY:					v = getRef<DataArray>()->get(0).toInt(); break;
	case TYPE_RECORD:					break;

	default:							assert(0);
	}

	setData(TYPE_INT, v);

	return v;
}

int64 DataValue::convertToInt64()
{
	int64 v = 0;

	switch (_type)
	{
	case TYPE_INT64:					return getData<int64>();

	case TYPE_INT:						v = (int64)getData<int>(); break;
	case TYPE_DOUBLE:					v = (int64)getData<double>(); break;
	case TYPE_FLOAT:					v = (int64)getData<float>(); break;

	case TYPE_TIMESTAMP:				v = (int64)getData<Timestamp>().getUnixTime64(); break;

	case TYPE_VECTOR2:
	case TYPE_SIZE2:
	case TYPE_FLOAT2:					v = (int64)getData<Float2>().x; break;

	case TYPE_VECTOR3:
	case TYPE_SIZE3:
	case TYPE_FLOAT3:					v = (int64)getData<Float3>().x; break;

	case TYPE_VECTOR4:
	case TYPE_QUAT:
	case TYPE_FLOAT4:					v = (int64)getData<Float4>().x; break;

	case TYPE_MATRIX3:
	case TYPE_FLOAT3X3:					v = (int64)getData<Float3x3>().a.x; break;

	case TYPE_MATRIX4:
	case TYPE_FLOAT4X4:					v = (int64)getData<Float4x4>().a.x; break;

	case TYPE_STRING:					parse(getStringPtr(), v); break;
	case TYPE_KEY:						parse(getRef<DataKey>()->getName().c_str(), v); break;

	case TYPE_BLOB:						BlobToData(getBlobPtr(), getBlobSize(), DataValue::TYPE_INT64, v); break;
	case TYPE_BUFFER:					BufferToData(getRef<MemoryBuffer>(), DataValue::TYPE_INT64, v); break;
	case TYPE_VOID:
	case TYPE_NULL:						break;

	case TYPE_ARRAY:					v = getRef<DataArray>()->get(0).toInt64(); break;
	case TYPE_RECORD:					break;

	default:							assert(0);
	}

	setData(TYPE_INT64, v);

	return v;
}

float DataValue::convertToFloat()
{
	float v = 0.0f;

	switch (_type)
	{
	case TYPE_FLOAT:					return getData<float>();

	case TYPE_BOOL:						v = (float)getData<uint8>(); break;
	case TYPE_INT:						v = (float)getData<int>(); break;
	case TYPE_INT64:					v = (float)getData<int64>(); break;
	case TYPE_DOUBLE:					v = (float)getData<double>(); break;

	case TYPE_TIMESTAMP:				v = (float)getData<Timestamp>().getUnixTime64(); break;

	case TYPE_VECTOR2:
	case TYPE_SIZE2:
	case TYPE_FLOAT2:					v = getData<Float2>().x; break;

	case TYPE_VECTOR3:
	case TYPE_SIZE3:
	case TYPE_FLOAT3:					v = getData<Float3>().x; break;

	case TYPE_VECTOR4:
	case TYPE_QUAT:
	case TYPE_FLOAT4:					v = getData<Float4>().x; break;

	case TYPE_MATRIX3:
	case TYPE_FLOAT3X3:					v = getData<Float3x3>().a.x; break;

	case TYPE_MATRIX4:
	case TYPE_FLOAT4X4:					v = getData<Float4x4>().a.x; break;

	case TYPE_STRING:					parse(getStringPtr(), v); break;
	case TYPE_KEY:						parse(getRef<DataKey>()->getName().c_str(), v); break;

	case TYPE_BLOB:						BlobToData(getBlobPtr(), getBlobSize(), DataValue::TYPE_FLOAT, v); break;
	case TYPE_BUFFER:					BufferToData(getRef<MemoryBuffer>(), DataValue::TYPE_FLOAT, v); break;

	case TYPE_VOID:
	case TYPE_NULL:						break;

	case TYPE_ARRAY:					v = getRef<DataArray>()->get(0).toFloat(); break;
	case TYPE_RECORD:					break;

	default:							assert(0);
	}

	setData(TYPE_FLOAT, v);

	return v;
}

double DataValue::convertToDouble()
{
	double v = 0.0;

	switch (_type)
	{
	case TYPE_DOUBLE:					return getData<double>();

	case TYPE_BOOL:						v = (double)getData<uint8>(); break;
	case TYPE_INT:						v = (double)getData<int>(); break;
	case TYPE_INT64:					v = (double)getData<int64>(); break;
	case TYPE_FLOAT:					v = (double)getData<float>(); break;

	case TYPE_TIMESTAMP:				v = (double)getData<Timestamp>().getUnixTime64(); break;

	case TYPE_VECTOR2:
	case TYPE_SIZE2:
	case TYPE_FLOAT2:					v = (double)getData<Float2>().x; break;

	case TYPE_VECTOR3:
	case TYPE_SIZE3:
	case TYPE_FLOAT3:					v = (double)getData<Float3>().x; break;

	case TYPE_VECTOR4:
	case TYPE_QUAT:
	case TYPE_FLOAT4:					v = (double)getData<Float4>().x; break;

	case TYPE_MATRIX3:
	case TYPE_FLOAT3X3:					v = (double)getData<Float3x3>().a.x; break;

	case TYPE_MATRIX4:
	case TYPE_FLOAT4X4:					v = (double)getData<Float4x4>().a.x; break;

	case TYPE_STRING:					parse(getStringPtr(), v); break;
	case TYPE_KEY:						parse(getRef<DataKey>()->getName().c_str(), v); break;

	case TYPE_BLOB:						BlobToData(getBlobPtr(), getBlobSize(), DataValue::TYPE_DOUBLE, v); break;
	case TYPE_BUFFER:					BufferToData(getRef<MemoryBuffer>(), DataValue::TYPE_DOUBLE, v); break;

	case TYPE_VOID:
	case TYPE_NULL:						break;

	case TYPE_ARRAY:					v = getRef<DataArray>()->get(0).toDouble(); break;
	case TYPE_RECORD:					break;

	default:							assert(0);
	}

	setData(TYPE_DOUBLE, v);

	return v;
}

static void ArrayToFloats(DataArray* array, float* floats, int numFloats)
{
	for (int i=0; i<numFloats; ++i)
		floats[i] = array->get(i);
}

template <typename TValue, typename TSrcValue>
static inline TValue Cast(const TSrcValue& value)
{
	return *(TValue*)&value;
}

const Float2& DataValue::convertToFloat2()
{
	Float2 v = { 0 };

	switch (_type)
	{
	case TYPE_VECTOR2:
	case TYPE_SIZE2:
	case TYPE_FLOAT2:					return getData<Float2>();
		
	case TYPE_INT:						v.x = (float)getData<int>(); break;
	case TYPE_INT64:					v.x = (float)getData<int64>(); break;
	case TYPE_FLOAT:					v.x = getData<float>(); break;
	case TYPE_DOUBLE:					v.x = (float)getData<double>(); break;

	case TYPE_TIMESTAMP:				break;

	case TYPE_VECTOR3:
	case TYPE_SIZE3:
	case TYPE_FLOAT3:					v = Cast<Float2>(getData<Float3>()); break;

	case TYPE_VECTOR4:
	case TYPE_QUAT:
	case TYPE_FLOAT4:					v = Cast<Float2>(getData<Float4>()); break;

	case TYPE_MATRIX3:
	case TYPE_FLOAT3X3:					v = Cast<Float2>(getData<Float3x3>()); break;

	case TYPE_MATRIX4:
	case TYPE_FLOAT4X4:					v = Cast<Float2>(getData<Float4x4>());; break;

	case TYPE_STRING:					parse(getStringPtr(), v); break;
	case TYPE_KEY:						parse(getRef<DataKey>()->getName().c_str(), v); break;

	case TYPE_BLOB:						BlobToData(getBlobPtr(), getBlobSize(), DataValue::TYPE_FLOAT2, v); break;
	case TYPE_BUFFER:					BufferToData(getRef<MemoryBuffer>(), DataValue::TYPE_FLOAT2, v); break;

	case TYPE_VOID:
	case TYPE_NULL:						break;

	case TYPE_ARRAY:					ArrayToFloats(getRef<DataArray>(), (float*)&v, 2); break; 
	case TYPE_RECORD:					{ DataRecord* r = getRef<DataRecord>(); v.x = r->get("x"); v.y = r->get("y"); } break;

	default:							assert(0);
	}

	setData(TYPE_FLOAT2, v);

	return getData<Float2>();
}

const Float3& DataValue::convertToFloat3()
{
	Float3 v = { 0 };

	switch (_type)
	{
	case TYPE_VECTOR3:
	case TYPE_SIZE3:
	case TYPE_FLOAT3:					return getData<Float3>();

	case TYPE_INT:						v.x = (float)getData<int>(); break;
	case TYPE_INT64:					v.x = (float)getData<int64>(); break;
	case TYPE_FLOAT:					v.x = getData<float>(); break;
	case TYPE_DOUBLE:					v.x = (float)getData<double>(); break;

	case TYPE_TIMESTAMP:				break;

	case TYPE_VECTOR2:
	case TYPE_SIZE2:
	case TYPE_FLOAT2:					*(Float2*)&v = getData<Float2>(); break;

	case TYPE_VECTOR4:
	case TYPE_QUAT:
	case TYPE_FLOAT4:					v = Cast<Float3>(getData<Float4>()); break;
										
	case TYPE_MATRIX3:
	case TYPE_FLOAT3X3:					v = Cast<Float3>(getData<Float3x3>()); break;

	case TYPE_MATRIX4:
	case TYPE_FLOAT4X4:					v = Cast<Float3>(getData<Float4x4>()); break;

	case TYPE_STRING:					parse(getStringPtr(), v); break;
	case TYPE_KEY:						parse(getRef<DataKey>()->getName().c_str(), v); break;

	case TYPE_BLOB:						BlobToData(getBlobPtr(), getBlobSize(), DataValue::TYPE_FLOAT3, v); break;
	case TYPE_BUFFER:					BufferToData(getRef<MemoryBuffer>(), DataValue::TYPE_FLOAT3, v); break;

	case TYPE_VOID:
	case TYPE_NULL:						break;

	case TYPE_ARRAY:					ArrayToFloats(getRef<DataArray>(), (float*)&v, 3); break; 
	case TYPE_RECORD:					{ DataRecord* r = getRef<DataRecord>(); v.x = r->get("x"); v.y = r->get("y"); v.z = r->get("z"); } break;

	default:							assert(0);
	}

	setData(TYPE_FLOAT3, v);

	return getData<Float3>();
}

const Float4& DataValue::convertToFloat4()
{
	Float4 v = { 0 };

	switch (_type)
	{
	case TYPE_VECTOR4:
	case TYPE_QUAT:
	case TYPE_FLOAT4:					return getData<Float4>();

	case TYPE_INT:						v.x = (float)getData<int>(); break;
	case TYPE_INT64:					v.x = (float)getData<int64>(); break;
	case TYPE_FLOAT:					v.x = getData<float>(); break;
	case TYPE_DOUBLE:					v.x = (float)getData<double>(); break;

	case TYPE_TIMESTAMP:				break;

	case TYPE_VECTOR2:
	case TYPE_SIZE2:
	case TYPE_FLOAT2:					*(Float2*)&v = getData<Float2>(); break;

	case TYPE_VECTOR3:
	case TYPE_SIZE3:
	case TYPE_FLOAT3:					*(Float3*)&v = getData<Float3>(); break;

	case TYPE_MATRIX3:
	case TYPE_FLOAT3X3:					*(Float3*)&v = getData<Float3x3>().a; break;

	case TYPE_MATRIX4:
	case TYPE_FLOAT4X4:					v = getData<Float4x4>().a; break;

	case TYPE_STRING:					parse(getStringPtr(), v); break;
	case TYPE_KEY:						parse(getRef<DataKey>()->getName().c_str(), v); break;

	case TYPE_BLOB:						BlobToData(getBlobPtr(), getBlobSize(), DataValue::TYPE_FLOAT4, v); break;
	case TYPE_BUFFER:					BufferToData(getRef<MemoryBuffer>(), DataValue::TYPE_FLOAT4, v); break;

	case TYPE_VOID:
	case TYPE_NULL:						break;

	case TYPE_ARRAY:					ArrayToFloats(getRef<DataArray>(), (float*)&v, 4); break; 
	case TYPE_RECORD:					{ DataRecord* r = getRef<DataRecord>(); v.x = r->get("x"); v.y = r->get("y"); v.z = r->get("z"); v.w = r->get("w"); } break;

	default:							assert(0);
	}

	setData(TYPE_FLOAT4, v);

	return getData<Float4>();
}

static void RecordToMatrix(DataRecord* rec, float* floats, int numRows, int numCols)
{
	for (int r = 0; r < numRows; ++r)
	{
		for (int c = 0; c < numCols; ++c)
		{
			String key = StringUtil::format("m%d%d", r, c);
			int i = numCols * r + c;
			floats[i] = rec->get(key).toFloat();
		}
	}
}

const Float3x3& DataValue::convertToFloat3x3()
{
	Float3x3 v = { 0 };

	switch (_type)
	{
	case TYPE_MATRIX3:
	case TYPE_FLOAT3X3:					return getData<Float3x3>();

	case TYPE_INT:						v.a.x = (float)getData<int>(); break;
	case TYPE_INT64:					v.a.x = (float)getData<int64>(); break;
	case TYPE_FLOAT:					v.a.x = getData<float>(); break;
	case TYPE_DOUBLE:					v.a.x = (float)getData<double>(); break;

	case TYPE_TIMESTAMP:				break;

	case TYPE_VECTOR2:
	case TYPE_SIZE2:
	case TYPE_FLOAT2:					*(Float2*)&v.a = getData<Float2>(); break;

	case TYPE_VECTOR3:
	case TYPE_SIZE3:
	case TYPE_FLOAT3:					v.a = getData<Float3>(); break;

	case TYPE_VECTOR4:
	case TYPE_QUAT:
	case TYPE_FLOAT4:					v.a = Cast<Float3>(getData<Float4>()); break;

	case TYPE_MATRIX4:
	case TYPE_FLOAT4X4:					
		{
			const Float4x4& f44 = getData<Float4x4>();
			v.a = *(Float3*)&f44.a; v.b = *(Float3*)&f44.b; v.c = *(Float3*)&f44.c; 
		}
		break;

	case TYPE_STRING:					parse(getStringPtr(), v); break;
	case TYPE_KEY:						parse(getRef<DataKey>()->getName().c_str(), v); break;

	case TYPE_BLOB:						BlobToData(getBlobPtr(), getBlobSize(), DataValue::TYPE_FLOAT3X3, v); break;
	case TYPE_BUFFER:					BufferToData(getRef<MemoryBuffer>(), DataValue::TYPE_FLOAT3X3, v); break;

	case TYPE_VOID:
	case TYPE_NULL:						break;

	case TYPE_ARRAY:					ArrayToFloats(getRef<DataArray>(), (float*)&v, 3*3); break; 

	case TYPE_RECORD:
		{
			DataRecord* rec = getRef<DataRecord>();
			DataValue a = rec->get("a");
			if (!a.isVoid())
			{
				v.a = a;
				v.b = rec->get("b");
				v.c = rec->get("c");
			}
			else
			{
				RecordToMatrix(rec, (float*)&v, 3, 3);
			}
		}
		break;

	default:							assert(0);
	}

	setData(TYPE_FLOAT3X3, v);

	return getData<Float3x3>();
}

const Float4x4& DataValue::convertToFloat4x4()
{
	Float4x4 v = { 0 };

	switch (_type)
	{
	case TYPE_MATRIX4:
	case TYPE_FLOAT4X4:					return getData<Float4x4>();
		
	case TYPE_INT:						v.a.x = (float)getData<int>(); break;
	case TYPE_INT64:					v.a.x = (float)getData<int64>(); break;
	case TYPE_FLOAT:					v.a.x = getData<float>(); break;
	case TYPE_DOUBLE:					v.a.x = (float)getData<double>(); break;

	case TYPE_TIMESTAMP:				break;

	case TYPE_VECTOR2:
	case TYPE_SIZE2:
	case TYPE_FLOAT2:					*(Float2*)&v.a = getData<Float2>(); break;

	case TYPE_VECTOR3:
	case TYPE_SIZE3:
	case TYPE_FLOAT3:					*(Float3*)&v.a = getData<Float3>(); break;

	case TYPE_VECTOR4:
	case TYPE_QUAT:
	case TYPE_FLOAT4:					v.a = getData<Float4>(); break;

	case TYPE_MATRIX3:
	case TYPE_FLOAT3X3:					
		{
			const Float3x3& f33 = getData<Float3x3>();
			*(Float3*)&v.a = f33.a; *(Float3*)&v.b = f33.b; *(Float3*)&v.c = f33.c; 
		}
		break;

	case TYPE_STRING:					parse(getStringPtr(), v); break;
	case TYPE_KEY:						parse(getRef<DataKey>()->getName().c_str(), v); break;

	case TYPE_BLOB:						BlobToData(getBlobPtr(), getBlobSize(), DataValue::TYPE_FLOAT4X4, v); break;
	case TYPE_BUFFER:					BufferToData(getRef<MemoryBuffer>(), DataValue::TYPE_FLOAT4X4, v); break;

	case TYPE_VOID:
	case TYPE_NULL:						break;

	case TYPE_ARRAY:					ArrayToFloats(getRef<DataArray>(), (float*)&v, 4*4); break; 

	case TYPE_RECORD:
		{
			DataRecord* rec = getRef<DataRecord>();
			DataValue a = rec->get("a");
			if (!a.isVoid())
			{
				v.a = a;
				v.b = rec->get("b");
				v.c = rec->get("c");
				v.d = rec->get("d");
			}
			else
			{
				RecordToMatrix(rec, (float*)&v, 4, 4);
			}
		}
		break;

	default:							assert(0);
	}

	setData(TYPE_FLOAT4X4, v);

	return getData<Float4x4>();
}

const Timestamp& DataValue::convertToTimestamp()
{
	Timestamp v;

	switch (_type)
	{
	case TYPE_INT64:					v = Timestamp(getData<int64>()); break;
	case TYPE_INT:						v = Timestamp((int64)getData<int>()); break;
	case TYPE_FLOAT:					v = Timestamp((int64)getData<float>()); break;
	case TYPE_DOUBLE:					v = Timestamp((int64)getData<double>()); break;

	case TYPE_STRING:					v.setIso8601(getStringPtr()); break;
	case TYPE_KEY:						v.setIso8601(getRef<DataKey>()->getName()); break;

	case TYPE_VECTOR2:
	case TYPE_SIZE2:
	case TYPE_FLOAT2:

	case TYPE_VECTOR3:
	case TYPE_SIZE3:
	case TYPE_FLOAT3:

	case TYPE_VECTOR4:
	case TYPE_QUAT:
	case TYPE_FLOAT4:

	case TYPE_MATRIX3:
	case TYPE_FLOAT3X3:

	case TYPE_BLOB:						BlobToData(getBlobPtr(), getBlobSize(), DataValue::TYPE_TIMESTAMP, v); break;
	case TYPE_BUFFER:					BufferToData(getRef<MemoryBuffer>(), DataValue::TYPE_TIMESTAMP, v); break;

	case TYPE_ARRAY:					v = getRef<DataArray>()->get(0).toTimestamp(); break;
	case TYPE_RECORD:					break;

	case TYPE_VOID:
	case TYPE_NULL:
	case TYPE_BOOL:						break;

	default:							assert(0);
	}

	setData(TYPE_TIMESTAMP, v);

	return getData<Timestamp>();
}

const char* DataValue::convertToString()
{
	char buf[256];
	int len = -1;

	switch (_type)
	{
	case TYPE_STRING:					return getStringPtr();

	case TYPE_KEY:
		{ 
			// The other ConvertTo() functions assume that here the data value is converted to TYPE_STRING.
			// So we have to perform type conversion explicitly rather than just returning the name string of the DataKey.
			// And there's chance of deleting c_str during conversion, it's safe to make a copy.

			String name = getRef<DataKey>()->getName();
			setString(name); 
			return getStringPtr();
		}
		break;

	case TYPE_BOOL:						sprintf(buf, "%s", getData<uint8>() ? "true" : "false"); break;
	case TYPE_INT:						sprintf(buf, "%d", getData<int>()); break;
	case TYPE_INT64:					sprintf(buf, "%lld", getData<int64>()); break;
	case TYPE_FLOAT:					sprintf(buf, "%f", getData<float>()); break;
	case TYPE_DOUBLE:					sprintf(buf, "%lf", getData<double>()); break;

	case TYPE_TIMESTAMP:				setString(getData<Timestamp>().getIso8601()); return getStringPtr();

	case TYPE_VECTOR2:
	case TYPE_SIZE2:
	case TYPE_FLOAT2:					
		{
			const Float2& f2 = getData<Float2>();
			len = sprintf(buf, "[%f,%f]", f2.x, f2.y);
		}
		break;

	case TYPE_VECTOR3:
	case TYPE_SIZE3:
	case TYPE_FLOAT3:				
		{
			const Float3& f3 = getData<Float3>();
			len = sprintf(buf, "[%f,%f,%f]", f3.x, f3.y, f3.z); 
		}
		break;

	case TYPE_VECTOR4:
	case TYPE_QUAT:
	case TYPE_FLOAT4:					
		{
			const Float4& f4 = getData<Float4>();
			len = sprintf(buf, "[%f,%f,%f,%f]", f4.x, f4.y, f4.z, f4.w); 
		}
		break;

	case TYPE_MATRIX3:
	case TYPE_FLOAT3X3:					
		{
			const Float3x3& f33 = getData<Float3x3>();
			len = sprintf(buf, "[%f,%f,%f,%f,%f,%f,%f,%f,%f]",
				f33.a.x, f33.a.y, f33.a.z,
				f33.b.x, f33.b.y, f33.b.z,
				f33.c.x, f33.c.y, f33.c.z); 
		}
		break;
	
	case TYPE_MATRIX4:
	case TYPE_FLOAT4X4:				
		{
			const Float4x4& f44 = getData<Float4x4>();
			len = sprintf(buf, "[%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f]",
				f44.a.x, f44.a.y, f44.a.z, f44.a.w,
				f44.b.x, f44.b.y, f44.b.z, f44.b.w,
				f44.c.x, f44.c.y, f44.c.z, f44.c.w,
				f44.d.x, f44.d.y, f44.d.z, f44.d.w); 
		}
		break;

	case TYPE_BLOB:	
		{
			// blob -> encode
			Ref<MemoryBuffer::Writer> w = new MemoryBuffer::Writer();
			Ref<Base64Encoder> encoder = new Base64Encoder(w);
			encoder->writeRaw(getBlobPtr(), getBlobSize());
			encoder = NULL;

			// encoded -> string
			loadString(new MemoryBuffer::Reader(w->getBuffer(), NULL), w->getSize());
			return getStringPtr();
		}
		break;

	case TYPE_BUFFER:
		{
			// buffer -> encode
			Ref<MemoryBuffer> buffer = getRef<MemoryBuffer>();
			Ref<MemoryBuffer::Writer> w = new MemoryBuffer::Writer();
			Ref<Base64Encoder> encoder = new Base64Encoder(w);
			encoder->copy(new MemoryBuffer::Reader(buffer, NULL));
			encoder = NULL;

			// encoded -> string
			loadString(new MemoryBuffer::Reader(w->getBuffer(), NULL), w->getSize());
			return getStringPtr();
		}
		break;

	case TYPE_VOID:
	case TYPE_NULL:						buf[0] = 0; break;

	case TYPE_ARRAY:
	case TYPE_RECORD:
		{
			Ref<MemoryBuffer::Writer> w = new MemoryBuffer::Writer();
			Ref<JsonDataSaver> saver = new JsonDataSaver(w);
			saver->printValue(*this);
			setString(w->getBuffer()->toString());
			return getStringPtr();
		}


	default:							assert(0); strcpy(buf, "<unknown>");
	}

	setString(buf, len);

	return getStringPtr();
}

const void* DataValue::convertToBlob()
{
	switch (_type)
	{
	case TYPE_BLOB:						return getBlobPtr();

	case TYPE_BOOL:						*this = DataToBlob(_type, getData<uint8>()); break;
	case TYPE_INT:						*this = DataToBlob(_type, getData<int>()); break;
	case TYPE_INT64:					*this = DataToBlob(_type, getData<int64>()); break;
	case TYPE_FLOAT:					*this = DataToBlob(_type, getData<float>()); break;
	case TYPE_DOUBLE:					*this = DataToBlob(_type, getData<double>()); break;

	case TYPE_VECTOR2:
	case TYPE_SIZE2:
	case TYPE_FLOAT2:					*this = DataToBlob(_type, getData<Float2>()); break;

	case TYPE_VECTOR3:
	case TYPE_SIZE3:
	case TYPE_FLOAT3:					*this = DataToBlob(_type, getData<Float3>()); break;

	case TYPE_VECTOR4:
	case TYPE_QUAT:
	case TYPE_FLOAT4:					*this = DataToBlob(_type, getData<Float4>()); break;

	case TYPE_MATRIX3:
	case TYPE_FLOAT3X3:					*this = DataToBlob(_type, getData<Float3x3>()); break;

	case TYPE_MATRIX4:
	case TYPE_FLOAT4X4:					*this = DataToBlob(_type, getData<Float4x4>()); break;

	case TYPE_STRING:
		{
			// string -> base64 decode
			Ref<Base64Decoder> decoder = new Base64Decoder(new TempMemoryReader(getStringPtr(), getStringSize()));
			Ref<MemoryBuffer::Writer> w = new MemoryBuffer::Writer();
			w->copy(decoder);
			decoder = NULL;

			loadBlob(new MemoryBuffer::Reader(w->getBuffer(), NULL), w->getSize());
			return getBlobPtr();
		}

	case TYPE_KEY:						return convertTo(TYPE_STRING).toBlob(NULL);

	case TYPE_ARRAY:
	case TYPE_RECORD:
	case TYPE_OBJECT:
		{
			Ref<MemoryBuffer::Writer> w = new MemoryBuffer::Writer();
			save(w);

			Ref<MemoryBuffer::Reader> r = new MemoryBuffer::Reader(w->getBuffer(), NULL);
			loadBlob(r, r->getSize());
			return getBlobPtr();
		}

	case TYPE_BUFFER:					
		{ 
			Ref<MemoryBuffer> buf = getRef<MemoryBuffer>(); 
			loadBlob(new MemoryBuffer::Reader(buf, NULL), buf->getSize());
			return getBlobPtr();
		}

	case TYPE_VOID:
	case TYPE_NULL:						return NULL;

	default:							assert(0); return NULL;
	}

	return getBlobPtr();
}

MemoryBuffer* DataValue::ConvertToBuffer()
{
	size_t sz = 0;

	Ref<MemoryBuffer> buffer;

	switch (_type)
	{
	case TYPE_BUFFER:					return getRef<MemoryBuffer>();

	case TYPE_BOOL:						buffer = DataToBuffer(_type, getData<uint8>()); break;
	case TYPE_INT:						buffer = DataToBuffer(_type, getData<int>()); break;
	case TYPE_INT64:					buffer = DataToBuffer(_type, getData<int64>()); break;
	case TYPE_FLOAT:					buffer = DataToBuffer(_type, getData<float>()); break;
	case TYPE_DOUBLE:					buffer = DataToBuffer(_type, getData<double>()); break;

	case TYPE_VECTOR2:
	case TYPE_SIZE2:
	case TYPE_FLOAT2:					buffer = DataToBuffer(_type, getData<Float2>()); break;

	case TYPE_VECTOR3:
	case TYPE_SIZE3:
	case TYPE_FLOAT3:					buffer = DataToBuffer(_type, getData<Float3>()); break;

	case TYPE_VECTOR4:
	case TYPE_QUAT:
	case TYPE_FLOAT4:					buffer = DataToBuffer(_type, getData<Float4>()); break;

	case TYPE_MATRIX3:
	case TYPE_FLOAT3X3:					buffer = DataToBuffer(_type, getData<Float3x3>()); break;

	case TYPE_MATRIX4:
	case TYPE_FLOAT4X4:					buffer = DataToBuffer(_type, getData<Float4x4>()); break;

	case TYPE_BLOB:						sz = getBlobSize(); break;

	case TYPE_STRING:
		{
			// string -> base64 decode
			Ref<Base64Decoder> decoder = new Base64Decoder(new TempMemoryReader(getStringPtr(), getStringSize()));
			Ref<MemoryBuffer::Writer> w = new MemoryBuffer::Writer();
			w->copy(decoder);
			decoder = NULL;

			buffer = w->getBuffer();
		}
		break;

	case TYPE_KEY:						return convertTo(TYPE_STRING).toBuffer();

	case TYPE_ARRAY:
	case TYPE_RECORD:
	case TYPE_OBJECT:
		{
			Ref<MemoryBuffer::Writer> w = new MemoryBuffer::Writer();
			save(w);
			buffer = w->getBuffer();
		}
		break;

	case TYPE_VOID:
	case TYPE_NULL:						return NULL;

	default:							assert(0); return NULL;
	}

	if (buffer == NULL)
		buffer = new MemoryBuffer(getBlobPtr(), sz);
	setRef(TYPE_BUFFER, buffer);

	return getRef<MemoryBuffer>();
}

DataKey* DataValue::ConvertToKey()
{
	Ref<DataKey> key;

	switch (_type)
	{
	case TYPE_KEY:						return getRef<DataKey>();

	case TYPE_VOID:
	case TYPE_NULL:						return NULL;

	default:
		key = DataNamespace::getGlobal()->add(toString());
	}

	setRef(TYPE_KEY, key);

	return getRef<DataKey>();
}

static void AppendFloatsIntoArray(const float* floats, size_t numElem, DataArray* array)
{
	for (uint i=0; i<numElem; ++i)
		array->append(floats[i]);
}

DataArray* DataValue::convertToArray()
{
	Ref<DataArray> array;

	switch (_type)
	{
	case TYPE_ARRAY:					return getRef<DataArray>();

	case TYPE_FLOAT2:					AppendFloatsIntoArray((float*)getBlobPtr(), 2, array = new DataArray()); break;
	case TYPE_FLOAT3:					AppendFloatsIntoArray((float*)getBlobPtr(), 3, array = new DataArray()); break;
	case TYPE_FLOAT4:					AppendFloatsIntoArray((float*)getBlobPtr(), 4, array = new DataArray()); break;
	case TYPE_FLOAT3X3:					AppendFloatsIntoArray((float*)getBlobPtr(), 3*3, array = new DataArray()); break;
	case TYPE_FLOAT4X4:					AppendFloatsIntoArray((float*)getBlobPtr(), 4*4, array = new DataArray()); break;

	case TYPE_STRING:					
		{
			DataValue loaded = LoadJSONString(getStringPtr(), getStringSize());
	
			if (loaded._type == TYPE_ARRAY)
				array = (DataArray*)loaded._ref;
			else
				share(loaded);
		}
		break;

	case TYPE_KEY:						return convertTo(TYPE_STRING).toArray();

	case TYPE_BLOB:
		{
			DataValue loaded = LoadBlobData(getBlobPtr(), getBlobSize());

			if (loaded._type == TYPE_ARRAY)
				array = (DataArray*)loaded._ref;
			else
				share(loaded);
		}
		break;

	case TYPE_BUFFER:
		{
			DataValue loaded = LoadBlobData(new MemoryBuffer::Reader(getRef<MemoryBuffer>(), NULL));

			if (loaded._type == TYPE_RECORD)
				array = loaded.getRef<DataArray>();
			else
				share(loaded);
		}
		break;

	case TYPE_RECORD:
		{
			DataRecord* rec = getRef<DataRecord>();
			DataValue v = rec->get("value");
			if (v._type == TYPE_ARRAY)
				array = (DataArray*)v._ref;
			else if (!v.isVoid())
				share(v);
			else
				return NULL;
		}

	case TYPE_VOID:
	case TYPE_NULL:						return NULL;
	}

	if (array == NULL)
	{
		array = new DataArray();
		array->append(*this);
	}

	setRef(TYPE_ARRAY, array);
	return array;
}

DataRecord* DataValue::convertToRecord()
{
	Ref<DataRecord> record;

	switch (_type)
	{
	case TYPE_RECORD:					return getRef<DataRecord>();

	case TYPE_STRING:
		{
			DataValue loaded = LoadJSONString(getStringPtr(), getStringSize());

			if (loaded._type == TYPE_RECORD)
				record = (DataRecord*)loaded._ref;
		}
		break;

	case TYPE_KEY:						return convertTo(TYPE_STRING).toRecord();

	case TYPE_FLOAT2:
	case TYPE_VECTOR2:
	case TYPE_SIZE2:
		{
			const Float2& v = getData<Float2>();
			record = new DataRecord();
			record->set("x", v.x);
			record->set("y", v.y);
		}
		break;

	case TYPE_FLOAT3:
	case TYPE_VECTOR3:
	case TYPE_SIZE3:
		{
			const Float3& v = getData<Float3>();
			record = new DataRecord();
			record->set("x", v.x);
			record->set("y", v.y);
			record->set("z", v.z);
		}
		break;

	case TYPE_FLOAT4:
	case TYPE_VECTOR4:
	case TYPE_QUAT:
		{
			const Float4& v = getData<Float4>();
			record = new DataRecord();
			record->set("x", v.x);
			record->set("y", v.y);
			record->set("z", v.z);
			record->set("w", v.w);
		}
		break;

	case TYPE_FLOAT3X3:
	case TYPE_MATRIX3:
		{
			const Float3x3& m = getData<Float3x3>();
			record = new DataRecord();
			record->set("a", m.a);
			record->set("b", m.b);
			record->set("c", m.c);
		}
		break;

	case TYPE_FLOAT4X4:
	case TYPE_MATRIX4:
		{
			const Float4x4& m = getData<Float4x4>();
			record = new DataRecord();
			record->set("a", m.a);
			record->set("b", m.b);
			record->set("c", m.c);
			record->set("d", m.d);
		}
		break;

	case TYPE_BLOB:
		{
			DataValue loaded = LoadBlobData(getStringPtr(), getStringSize());

			if (loaded._type == TYPE_RECORD)
				record = loaded.getRef<DataRecord>();
			else
			{
				record = new DataRecord();
				record->set("value", loaded);
			}
		}
		break;

	case TYPE_BUFFER:
		{
			DataValue loaded = LoadBlobData(new MemoryBuffer::Reader(getRef<MemoryBuffer>(), NULL));

			if (loaded._type == TYPE_RECORD)
				record = loaded.getRef<DataRecord>();
			else
			{
				record = new DataRecord();
				record->set("value", loaded);
			}
		}
		break;

	default:
		record = new DataRecord();
		record->set("value", *this);
		break;
	}

	if (record)
		setRef(TYPE_RECORD, record);

	return record;
}

DataObject* DataValue::convertToObject()
{
	Ref<DataObject> object;

	switch (_type)
	{
	case TYPE_OBJECT:					return (DataObject*)_ref; // TODO: change to GetRef

	case TYPE_VOID:
	case TYPE_NULL:						return NULL;

	case TYPE_STRING:
		{
			DataValue loaded = LoadJSONString(getStringPtr(), getStringSize());

			if (loaded._type == TYPE_OBJECT)
				object = loaded.getRef<DataObject>();
		}
		break;

	case TYPE_KEY:						return convertTo(TYPE_STRING).toObject();

	case TYPE_BLOB:
		{
			DataValue loaded = LoadBlobData(getStringPtr(), getStringSize());

			if (loaded._type == TYPE_OBJECT)
				object = loaded.getRef<DataObject>();
		}
		break;

	case TYPE_BUFFER:
		{
			DataValue loaded = LoadBlobData(new MemoryBuffer::Reader(getRef<MemoryBuffer>(), NULL));

			if (loaded._type == TYPE_OBJECT)
				object = loaded.getRef<DataObject>();
		}
		break;

	default:
		return NULL;
	}

	if (object)
		setRef(TYPE_OBJECT, object);

	return object;
}

DataValue& DataValue::doConvert(Type type)
{
	switch (type)
	{
	case TYPE_ANY:						return *this;

	case TYPE_BOOL:						convertToBool(); return *this;
	case TYPE_INT:						convertToInt(); return *this;
	case TYPE_INT64:					convertToInt64(); return *this;
	case TYPE_DOUBLE:					convertToDouble(); return *this;
	case TYPE_FLOAT:					convertToFloat(); return *this;
	case TYPE_STRING:					convertToString(); return *this;
	case TYPE_BLOB:						convertToBlob(); return *this;

	case TYPE_TIMESTAMP:				convertToTimestamp(); return *this;

	case TYPE_VECTOR2:
	case TYPE_SIZE2:
	case TYPE_FLOAT2:					convertToFloat2(); return *this;

	case TYPE_VECTOR3:
	case TYPE_SIZE3:
	case TYPE_FLOAT3:					convertToFloat3(); return *this;

	case TYPE_VECTOR4:
	case TYPE_QUAT:
	case TYPE_FLOAT4:					convertToFloat4(); return *this;

	case TYPE_MATRIX3:
	case TYPE_FLOAT3X3:					convertToFloat3x3(); return *this;

	case TYPE_MATRIX4:
	case TYPE_FLOAT4X4:					convertToFloat4x4(); return *this;

	case TYPE_ARRAY:					convertToArray(); return *this;
	case TYPE_RECORD:					convertToRecord(); return *this;
	case TYPE_OBJECT:					convertToObject(); return *this;
	case TYPE_BUFFER:					ConvertToBuffer(); return *this;
	case TYPE_KEY:						ConvertToKey(); return *this;

	case TYPE_VOID:						toVoid(); return *this;
	case TYPE_NULL:						toNull(); return *this;

	default:							NIT_THROW(EX_NOT_SUPPORTED);
	}

	return *this;
}

typedef std::map<std::string, DataValue::Type> StrToTypeMap;

static StrToTypeMap& GetTypeStrs()
{
	static StrToTypeMap types;

	if (types.empty())
	{
		types["VOID"]		= DataValue::TYPE_VOID;
		types["NULL"]		= DataValue::TYPE_NULL;

		types["ANY"]		= DataValue::TYPE_ANY;
		types["INT"]		= DataValue::TYPE_INT;
		types["BOOL"]		= DataValue::TYPE_BOOL;
		types["INT64"]		= DataValue::TYPE_INT64;
		types["DOUBLE"]		= DataValue::TYPE_DOUBLE;
		types["FLOAT"]		= DataValue::TYPE_FLOAT;
		types["STRING"]		= DataValue::TYPE_STRING;
		types["BLOB"]		= DataValue::TYPE_BLOB;

		types["FLOAT2"]		= DataValue::TYPE_FLOAT2;
		types["FLOAT3"]		= DataValue::TYPE_FLOAT3;
		types["FLOAT4"]		= DataValue::TYPE_FLOAT4;
		types["FLOAT3X3"]	= DataValue::TYPE_FLOAT3X3;
		types["FLOAT4X4"]	= DataValue::TYPE_FLOAT4X4;

		types["TIMESTAMP"]	= DataValue::TYPE_TIMESTAMP;

		types["VECTOR2"]	= DataValue::TYPE_VECTOR2;
		types["VECTOR3"]	= DataValue::TYPE_VECTOR3;
		types["VECTOR4"]	= DataValue::TYPE_VECTOR4;
		types["QUAT"]		= DataValue::TYPE_QUAT;
		types["MATRIX3"]	= DataValue::TYPE_MATRIX3;
		types["MATRIX4"]	= DataValue::TYPE_MATRIX4;

		types["ARRAY"]		= DataValue::TYPE_ARRAY;
		types["RECORD"]		= DataValue::TYPE_RECORD;
		types["OBJECT"]		= DataValue::TYPE_OBJECT;
		types["BUFFER"]		= DataValue::TYPE_BUFFER;
		types["KEY"]		= DataValue::TYPE_KEY;
	}

	return types;
}

DataValue::Type DataValue::strToType(const char* str)
{
	StrToTypeMap& types = GetTypeStrs();
	StrToTypeMap::iterator itr = types.find(str);

	return itr != types.end() ? itr->second : TYPE_VOID;
}

const char* DataValue::typeToStr(Type t)
{
	typedef std::map<Type, std::string> TypeToStrMap;
	static TypeToStrMap types;

	if (types.empty())
	{
		StrToTypeMap& strMap = GetTypeStrs();
		for (StrToTypeMap::iterator itr = strMap.begin(), end = strMap.end(); itr != end; ++itr)
		{
			types.insert(std::make_pair(itr->second, itr->first));
		}
	}

	TypeToStrMap::iterator itr = types.find(t);

	return itr != types.end() ? itr->second.c_str() : NULL;
}

void DataValue::allTypes(vector<std::pair<std::string, Type> >::type& outResults)
{
	StrToTypeMap& strMap = GetTypeStrs();
	for (StrToTypeMap::iterator itr = strMap.begin(), end = strMap.end(); itr != end; ++itr)
	{
		outResults.push_back(*itr);
	}
}

size_t DataValue::copyBlobInto(void* buf, size_t bufLen) const
{
	size_t blobSize = getBlobSize();
	if (blobSize < bufLen)
		bufLen = blobSize;

	if (bufLen)
		memcpy(buf, getBlobPtr(), bufLen);

	return bufLen;
}

void DataValue::doRelease()
{
	if (_isChunked)
	{
		DataChunk* chunk = _chunk;
		_chunk = NULL;
		_isChunked = false;
		chunk->decRefCount();
	}
	else if (_isRef)
	{
		RefCounted* ref = _ref;
		_ref = NULL;
		_isRef = false;
		ref->decRefCount();
	}
}

void* DataValue::allocChunk(size_t sz)
{
	_chunk = DataChunk::alloc(sz);
	if (_chunk)
	{
		_isChunked = true;
		_chunk->incRefCount();
		_chunkSize = sz;
		_chunkAddr = (uint8*)_chunk->getMemory(0);
		return _chunkAddr;
	}
	return NULL;
}

static String DumpFloats(const char* type, const float* floats, int numFloats)
{
	char v[64];
	String temp;
	temp.append(type);
	temp.push_back('(');
	for (int i=0; i<numFloats; ++i)
	{
		if (i > 0) temp.append(", ");
		sprintf(v, "%f", floats[i]);
		temp.append(v);
	}
	temp.push_back(')');

	return temp;
}

String DataValue::dumpString()
{
	switch (_type)
	{
	case TYPE_NULL:						return "null";
	case TYPE_VOID:						return "void";
	case TYPE_ANY:						return "??any";

	case TYPE_BOOL:						return getData<uint8>() ? "true" : "false";
	case TYPE_INT:						return StringUtil::format("int(%d)", getData<int>());
	case TYPE_INT64:					return StringUtil::format("int64(%lld)", getData<int64>());
	case TYPE_FLOAT:					return StringUtil::format("float(%f)", getData<float>());
	case TYPE_DOUBLE:					return StringUtil::format("double(%lf)", getData<double>());

	case TYPE_TIMESTAMP:				return StringUtil::format("timestamp(%s)", getData<Timestamp>().getIso8601().c_str());

	case TYPE_FLOAT2:					return DumpFloats("Float2", (float*)getBlobPtr(), 2);
	case TYPE_VECTOR2:					return DumpFloats("Vector2", (float*)getBlobPtr(), 2);
	case TYPE_SIZE2:					return DumpFloats("Size2", (float*)getBlobPtr(), 2);

	case TYPE_FLOAT3:					return DumpFloats("Float3", (float*)getBlobPtr(), 3);
	case TYPE_VECTOR3:					return DumpFloats("Vector3", (float*)getBlobPtr(), 3);
	case TYPE_SIZE3:					return DumpFloats("Size3", (float*)getBlobPtr(), 3);

	case TYPE_FLOAT4:					return DumpFloats("Float4", (float*)getBlobPtr(), 4);
	case TYPE_VECTOR4:					return DumpFloats("Vector4", (float*)getBlobPtr(), 4);
	case TYPE_QUAT:						return DumpFloats("Quat", (float*)getBlobPtr(), 4);

	case TYPE_MATRIX3:					return DumpFloats("Matrix3", (float*)getBlobPtr(), 3 * 3);

	case TYPE_MATRIX4:					return DumpFloats("Matrix4", (float*)getBlobPtr(), 4 * 4);

	case TYPE_ARRAY:					return StringUtil::format("array(%d)", getRef<DataArray>()->getCount());
	case TYPE_RECORD:					return StringUtil::format("record(%d)", getRef<DataRecord>()->getCount());

	case TYPE_STRING:					return StringUtil::format("string(\"%s\")", getStringPtr());
	case TYPE_BLOB:						return StringUtil::format("blob(%08x, %d)", getBlobPtr(), getBlobSize());
	case TYPE_BUFFER:					return StringUtil::format("buffer(%08x, %d)", getRef<MemoryBuffer>(), getRef<MemoryBuffer>()->getSize());
	case TYPE_KEY:						return StringUtil::format("key(\"%s\")", getRef<DataKey>()->getName().c_str());

	case TYPE_OBJECT:					
		{
			DataObject* obj = getRef<DataObject>();
			DataSchema* schema = obj->getDataSchema();
			if (schema)
				return StringUtil::format("%s(%08x)", schema->getKeyName().c_str(), obj);
			else
				return StringUtil::format("object(%08x)", obj);
		}

	default:							return StringUtil::format("??unknown(%d)", _type);
	}
}

DataValue DataValue::fromJson(const char* str, int len)
{
	if (len == -1) len = strlen(str);

	DataValue value;

	Ref<JsonDataLoader> loader = new JsonDataLoader();
	loader->load(value, new TempMemoryReader(str, len));

	return value;
}

String DataValue::toJson(bool compact)
{
	Ref<MemoryBuffer::Writer> w = new MemoryBuffer::Writer();
	Ref<JsonDataSaver> s = new JsonDataSaver(w);

	s->setCompact(compact);
	s->setRecordSorted(!compact);
	s->printValue(*this);

	s->flush();

	return w->getBuffer()->toString();
}

static void LoadGDStream(DataValue& value, Ref<StreamReader> r)
{
	Ref<CalcCRC32Writer> crcw = new CalcCRC32Writer();

	Ref<BinDataLoader> loader = new BinDataLoader();
	loader->load(value, new CopyReader(r, crcw));

	uint32 myCRC = crcw->getValue();

	uint32 crc = 0;

	if (r->readRaw(&crc, sizeof(crc)) != sizeof(crc))
		crc = 0;

	if (myCRC != crc)
	{
		value.toVoid();
		NIT_THROW(EX_CORRUPTED);
	}
}

DataValue DataValue::load(StreamSource* source, ContentType treatAs /*= ContentType::UNKNOWN*/)
{
	DataValue value;

	Ref<StreamSource> safe = source;

	if (treatAs.isUnknown())
		treatAs = source->getContentType();

	switch (treatAs)
	{
	case ContentType::TEXT_PLIST:
		{
			Ref<StreamReader> reader = source->open();
			Ref<PlistDataLoader> loader = new PlistDataLoader();
			loader->load(value, reader);
		}
		break;

	case ContentType::TEXT_JSON:
		{
			Ref<StreamReader> reader = source->open();
			Ref<JsonDataLoader> loader = new JsonDataLoader();
			loader->load(value, reader);
		}
		break;

	case ContentType::BINARY_DATA:
	case ContentType::BINARY_ZDATA:
		{
			Ref<StreamReader> reader = source->open();
			Ref<BinDataLoader> loader = new BinDataLoader();
			loader->load(value, reader);
		}
		break;

	default:
		NIT_THROW_FMT(EX_NOT_SUPPORTED, "Unknown DataValue format: '%s'", source->getUrl().c_str());
	}

	return value;
}

void DataValue::save(StreamWriter* w)
{
	Ref<BinDataSaver> saver = new BinDataSaver();
	saver->save(*this, w);
}

void DataValue::saveCompressed(StreamWriter* w)
{
	Ref<BinDataSaver> saver = new BinDataSaver();
	saver->saveCompressed(*this, w);
}

void DataValue::saveJson(StreamWriter* w)
{
	Ref<JsonDataSaver> saver = new JsonDataSaver(w);
	saver->printValue(*this);
}

void DataValue::savePlist(StreamWriter* w)
{
	Ref<PlistDataSaver> saver = new PlistDataSaver(w);
	saver->printHeader();
	saver->printValue(*this);
}

void DataValue::loadString(StreamReader* r, size_t len)
{
	Ref<StreamReader> safe = r;

	release();

	// Setup in advance in order to handle memory safely even exception throws

	_type = TYPE_STRING;

	if (len < MAX_SMALLDATA_SIZE)
	{
		_smallData[len] = 0;
		_smallDataSize = len+1;
		if (r->readRaw(_smallData, len) != len)
			NIT_THROW(EX_CORRUPTED);
	}
	else
	{
		char* data = (char*)allocChunk(len+1);
		if (data == NULL)
			NIT_THROW(EX_MEMORY);

		data[len] = 0;
		if (r->readRaw(data, len) != len)
			NIT_THROW(EX_CORRUPTED);
	}
}

void DataValue::loadBlob(StreamReader* r, size_t size)
{
	Ref<StreamReader> safe = r;

	release();

	// Setup in advance in order to handle memory safely even exception throws

	_type = TYPE_BLOB;

	if (size <= MAX_SMALLDATA_SIZE)
	{
		_smallDataSize = size;
		if (r->readRaw(_smallData, size) != size)
			NIT_THROW(EX_CORRUPTED);
	}
	else
	{
		void* data = allocChunk(size);
		if (data == NULL)
			NIT_THROW(EX_MEMORY);

		if (r->readRaw(data, size) != size)
			NIT_THROW(EX_CORRUPTED);
	}
}

void DataValue::loadData(Type type, StreamReader* r, size_t size)
{
	release();

	_type = type;

	if (size <= MAX_SMALLDATA_SIZE)
	{
		if (r->readRaw(_smallData, size) != size)
			NIT_THROW(EX_CORRUPTED);
	}
	else
	{
		if (r->readRaw(allocChunk(size), size) != size)
			NIT_THROW(EX_CORRUPTED);
	}
}

DataValue DataValue::get(const String& key)
{
	if (_type == TYPE_RECORD)
		return getRef<DataRecord>()->get(key);
	else if (_type == TYPE_OBJECT)
		return getRef<DataObject>()->getProperty(key);

	return Void();
}

DataValue DataValue::get(DataKey* key)
{
	if (_type == TYPE_RECORD)
		return getRef<DataRecord>()->get(key);
	else if (_type == TYPE_OBJECT)
		return getRef<DataObject>()->getProperty(key);

	return Void();
}

DataValue DataValue::get(uint index)
{
	if (_type == TYPE_ARRAY)
		return getRef<DataArray>()->get(index);

	if (index == 0)
		return *this;

	return Void();
}

uint DataValue::getCount()
{
	switch (_type)
	{
	case TYPE_ARRAY:					return getRef<DataArray>()->getCount();
	case TYPE_RECORD:					return getRef<DataRecord>()->getCount();

	case TYPE_NULL:
	case TYPE_VOID:						return 0;

	default:							return 1;
	}
}

DataValue DataValue::clone() const
{
	bool useDataSave = true; 

	if (useDataSave)
	{
		switch (_type)
		{
		case TYPE_ARRAY:					break;
		case TYPE_RECORD:					break;
		case TYPE_OBJECT:					break;
		case TYPE_BUFFER:					return getRef<MemoryBuffer>()->clone();

		default:							return DataValue(*this);
		}
		
		// save to a memory buffer and load from it (ineffective)
		// TODO: optimize

		Ref<MemoryBuffer::Writer> writer = new MemoryBuffer::Writer();
		Ref<BinDataSaver> saver = new BinDataSaver();
		saver->save(*this, writer);

		Ref<StreamReader> reader = new MemoryBuffer::Reader(writer->getBuffer(), NULL);
		Ref<BinDataLoader> loader = new BinDataLoader();
		DataValue value;
		loader->load(value, reader);

		if (value._type != _type)
			NIT_THROW_FMT(EX_CORRUPTED, "can't clone value with data save");

		return value;
	}

	// TODO: Implement clone context (to handle same object with different name)
	switch (_type)
	{
	case TYPE_ARRAY:					return getRef<DataArray>()->clone();
	case TYPE_RECORD:					return getRef<DataRecord>()->clone();
	case TYPE_OBJECT:					return getRef<DataObject>()->clone();
	case TYPE_BUFFER:					return getRef<MemoryBuffer>()->clone();

	default:							return DataValue(*this);
	}
}

////////////////////////////////////////////////////////////////////////////////

DataChunk* DataChunk::alloc(size_t size)
{
	DataChunk* chunk = (DataChunk*)NIT_ALLOC(sizeof(DataChunk) + size);
	if (chunk)
		new (chunk) DataChunk(size);

	return chunk;
}

DataChunk* DataChunk::realloc(DataChunk* chunk, size_t size)
{
	DataChunk* newChunk = (DataChunk*)NIT_REALLOC(chunk, chunk->_allocSize, size);
	newChunk->_allocSize = size;
	return chunk;
}

void DataChunk::free(DataChunk* chunk)
{
	size_t allocSize = chunk->_allocSize;
	chunk->~DataChunk();
	NIT_DEALLOC(chunk, allocSize);
}

////////////////////////////////////////////////////////////////////////////////

DataArray::Iterator DataArray::s_NullItr;

////////////////////////////////////////////////////////////////////////////////

HashTable::HashTable(uint capacity)
{
	if (capacity)
	{
		uint pow2Size = 4;
		while (capacity > pow2Size) pow2Size <<= 1;
		allocNodes(pow2Size);
	}
	else
	{
		_numNodes = NULL;
		_firstFree = NULL;
		_nodes = NULL;
	}

	_usedNodes = 0;
}

HashTable::~HashTable()
{
	if (_nodes)
	{
		for (uint i=0; i<_numNodes; ++i)
			_nodes[i].~HashNode();
		NIT_DEALLOC(_nodes, _numNodes * sizeof(HashNode));
	}
}

HashTable* HashTable::clone()
{
	HashTable* newTable = new HashTable(_numNodes);

	int index = 0;
	Pair* pair;

	while ((pair = next(index)))
		newTable->newSlot(pair->first, pair->second);

	return newTable;
}

void HashTable::allocNodes(uint newSize)
{
	HashNode* nodes = (HashNode*) NIT_ALLOC(sizeof(HashNode) * newSize);
	for (uint i=0; i<newSize; ++i)
		new (&nodes[i]) HashNode();

	_numNodes = newSize;
	_nodes = nodes;
	_firstFree = &_nodes[_numNodes-1];
}

DataValue& HashTable::newSlot(DataKey* key, const DataValue& value)
{
	if (_nodes == NULL)
		allocNodes(4);

	DataKey::HashValue h = key->getHashValue() & (_numNodes - 1);

	HashNode* mp = &_nodes[h];

	if (mp->_pair.first)
	{
		HashNode* n = _firstFree; // get a free place

		DataKey::HashValue mph = mp->_pair.first->getHashValue() & (_numNodes - 1);

		HashNode* other; // main position of colliding node

		if (mp > n && (other = &_nodes[mph]) != mp)
		{
			// move colliding node into free position

			// find previous
			while (other->_next != mp)
				other = other->_next; 

			// redo the chain with 'n' in place of 'mp'
			other->_next = n; 

			// copy colliding node into free pos (mp->_next also goes)
			*n = *mp;

			// now 'mp' is free
			mp->_pair.first = NULL;
			mp->_pair.second.toVoid();
			mp->_next = NULL; 
		}
		else
		{
			// new node will go into free position
			n->_next = mp->_next; // chain new position
			mp->_next = n;
			mp = n;
		}
	}

	mp->_pair.first = key;

	// correct 'first free'
	while (true)
	{
		if (_firstFree->_pair.first == NULL && _firstFree->_next == NULL)
		{
			// ok: table still has a free place
			++_usedNodes;
			return (mp->_pair.second = value);
		}
		else if (_firstFree == _nodes) break; // cannot decrement from here
		else --_firstFree;
	}

	rehash(true);
	return set(key, value);
}

HashTable::Pair* HashTable::next(int& varIndex)
{
	while ((uint)varIndex < _numNodes)
	{
		HashNode& n = _nodes[varIndex++];
		if (n._pair.first)
		{
			// first found
			return &n._pair;
		}
	}

	// iteration end
	return NULL;
}

void HashTable::clear()
{
	if (_nodes)
	{
		for (uint i=0; i<_numNodes; ++i)
			_nodes[i].~HashNode();
		NIT_DEALLOC(_nodes, _numNodes * sizeof(HashNode));

		_numNodes = 0;
		_usedNodes = 0;
		_nodes = NULL;
		_firstFree = NULL;
	}
}

void HashTable::rehash(bool forced)
{
	uint oldSize = _numNodes;
	HashNode* old = _nodes;

	// prevent problems with the integer division
	if (oldSize < 4) oldSize = 4;
	uint numElems = _usedNodes;
	if (numElems >= oldSize - oldSize / 4) // using more than 3/4?
		allocNodes(oldSize * 2);
	else if (numElems <= oldSize / 4 && oldSize > 4)
		allocNodes(oldSize / 2);
	else if (forced)
		allocNodes(oldSize);
	else
		return;

	_usedNodes = 0;

	if (old)
	{
		for (uint i=0; i < oldSize; ++i)
		{
			HashNode& n = old[i];
			if (n._pair.first)
				newSlot(n._pair.first, n._pair.second);
		}

		for (uint i=0; i < oldSize; ++i)
			old[i].~HashNode();

		NIT_DEALLOC(old, oldSize * sizeof(HashNode));
	}
}

void HashTable::Delete(DataKey* key)
{
	HashNode* n = getNode(key, key->getHashValue() & (_numNodes - 1));

	if (n)
	{
		n->_pair.first = NULL;
		n->_pair.second.toVoid();
		--_usedNodes;
		rehash(false);
	}
}

////////////////////////////////////////////////////////////////////////////////

DataRecord::Iterator DataRecord::s_NullItr;

DataRecord::DataRecord(DataNamespace* keys) : _namespace(keys)
{
	if (keys == NULL)
		_namespace = DataNamespace::getGlobal();
}

void DataRecord::Delete(const String& name)
{
	Delete(this ? _namespace->get(name) : NULL);
}

void DataRecord::Delete(DataKey* key)
{
	if (this == NULL || key == NULL) return;

	_table.Delete(key);
}

void DataRecord::clear()
{
	if (this == NULL) return;

	_table.clear();
}

void DataRecord::onDelete()
{
	clear();
}

void DataRecord::find(const String& pattern, vector<DataValue>::type& outValues)
{
	if (this == NULL) return;

	if (pattern == "*")
	{
		int index = 0;
		HashTable::Pair* pair = NULL;
		while ((pair = _table.next(index)))
			outValues.push_back(pair->second);
	}
	else
	{
		int index = 0;
		HashTable::Pair* pair = NULL;
		while ((pair = _table.next(index)))
		{
			if (Wildcard::match(pattern, pair->first->getName()))
				outValues.push_back(pair->second);
		}
	}
}

void DataRecord::dump()
{
	int index = 0;
	HashTable::Pair* pair;

	LOG(0, "-- record:\n");
	while ((pair = _table.next(index)))
	{
		LOG(0, "-- - %-18s = %s\n", pair->first->getName().c_str(), pair->second.dumpString().c_str());
	}
}

DataRecord* DataRecord::clone() const
{
	DataRecord* rec = const_cast<DataRecord*>(this);

	bool useDataSave = true; 

	if (useDataSave)
	{
		// save to a memory buffer and load from it (ineffective)
		// TODO: optimize

		Ref<MemoryBuffer::Writer> writer = new MemoryBuffer::Writer();
		Ref<BinDataSaver> saver = new BinDataSaver();
		saver->save(DataValue(rec), writer);

		Ref<StreamReader> reader = new MemoryBuffer::Reader(writer->getBuffer(), NULL);
		Ref<BinDataLoader> loader = new BinDataLoader();
		DataValue value;
		loader->load(value, reader);

		DataRecord* clone = value.toRecord();

		if (clone == NULL)
			NIT_THROW_FMT(EX_CORRUPTED, "can't clone record with data save");

		return clone;
	}

	// TODO: Implement clone context (to handle same object with different name)
	DataRecord* clone = new DataRecord();
	for (Iterator itr = rec->begin(), end = rec->end(); itr != end; ++itr)
	{
		clone->set(itr->first, itr->second.clone());
	}

	return clone;
}

////////////////////////////////////////////////////////////////////////////////

void DataArray::dump()
{
	LOG(0, "-- array:\n");
	for (uint i=0; i<_array.size(); ++i)
	{
		LOG(0, "-- - %-18d = %s\n", i, _array[i].dumpString().c_str());
	}
}

DataArray* DataArray::clone() const
{
	DataArray* array = const_cast<DataArray*>(this);

	bool useDataSave = true; 

	if (useDataSave)
	{
		// save to a memory buffer and load from it (ineffective)
		// TODO: optimize

		Ref<MemoryBuffer::Writer> writer = new MemoryBuffer::Writer();
		Ref<BinDataSaver> saver = new BinDataSaver();
		saver->save(DataValue(array), writer);

		Ref<StreamReader> reader = new MemoryBuffer::Reader(writer->getBuffer(), NULL);
		Ref<BinDataLoader> loader = new BinDataLoader();
		DataValue value;
		loader->load(value, reader);

		DataArray* clone = value.toArray();

		if (clone == NULL)
			NIT_THROW_FMT(EX_CORRUPTED, "can't clone array with data save");

		return clone;
	}

	// TODO: Implement clone context (to handle same object with different name)
	DataArray* clone = new DataArray();
	for (uint i=0; i<_array.size(); ++i)
		clone->_array[i] = _array[i].clone();

	return clone;
}

////////////////////////////////////////////////////////////////////////////////

DataNamespace::Iterator DataNamespace::s_NullItr;

void DataNamespace::onDelete()
{
	_names.clear();
}

Ref<DataKey> DataNamespace::add(const String& name)
{
	ASSERT_THROW(this, EX_NULL);

	NameMap::iterator itr = _names.find(name);
	if (itr != _names.end())
		return itr->second;

	Ref<DataKey> key = new DataKey();
	itr = _names.insert(std::make_pair(name, key)).first;
	key->_namespace = this;
	key->_iterator = itr;
	key->_hashValue = DataKey::calcHashValue(name.c_str(), name.size());

	return key;
}

DataKey* DataNamespace::get(const String& name)
{
	if (!this) return NULL;

	NameMap::iterator itr = _names.find(name);
	return itr != _names.end() ? itr->second : NULL;
}

void DataNamespace::onDeleteKey(DataKey* key)
{
	if (key->_namespace == this)
	{
		_names.erase(key->_iterator);
	}
}

class GlobalNamespaceLib : public TRuntimeSingleton<GlobalNamespaceLib>
{
public:
	virtual void onInit()
	{
		_keys = new DataNamespace("global");
	}

	virtual void onFinish()
	{
		// TODO: Keys are referenced by Ref<> so must be clear
		_keys = NULL;
	}

	DataNamespace* GetSharedDataKeys()
	{
		return _keys;
	}

	Ref<DataNamespace>					_keys;
};

DataNamespace* DataNamespace::getGlobal()
{
	GlobalNamespaceLib& lib = GlobalNamespaceLib::getSingleton();

	return lib.GetSharedDataKeys();
}

////////////////////////////////////////////////////////////////////////////////

DataKey::HashValue DataKey::calcHashValue(const char* str, int len /*= -1*/)
{
	if (len == -1) len = strlen(str);

	HashValue h = 1; // seed
	int step = (len >> 5) | 1; // if string is too long, don't hash all its chars
	for (; len >= step; len -= step)
		h = h ^ ((h << 5) + (h >> 2) + (unsigned short) * (str++));

	return h;
}

////////////////////////////////////////////////////////////////////////////////

void DataObject::setProperty(const String& key, const DataValue& value)
{
	DataSchema* schema = getDataSchema();
	DataProperty* prop = schema ? schema->getProperty(key) : NULL;

	if (prop == NULL)
		NIT_THROW_FMT(EX_NOT_FOUND, "no property '%s.%s'", schema->getName().c_str(), key.c_str());

	if (!prop->setValue(this, value))
		NIT_THROW_FMT(EX_ACCESS, "can't set property '%s.%s'", schema->getName().c_str(), key.c_str());
}

void DataObject::setProperty(DataKey* key, const DataValue& value)
{
	DataSchema* schema = getDataSchema();
	DataProperty* prop = schema ? schema->getProperty(key) : NULL;

	if (prop == NULL)
		NIT_THROW_FMT(EX_NOT_FOUND, "no property '%s.%s'", schema->getName().c_str(), key->getName().c_str());

	if (!prop->setValue(this, value))
		NIT_THROW_FMT(EX_ACCESS, "can't set property '%s.%s'", schema->getName().c_str(), key->getName().c_str());
}

DataValue DataObject::getProperty(const String& key)
{
	DataSchema* schema = getDataSchema();
	DataProperty* prop = schema ? schema->getProperty(key) : NULL;
	if (prop)
	{
		DataValue value;
		if (prop->getValue(this, value))
			return value;
	}

	return DataValue::Void();
}

DataValue DataObject::getProperty(DataKey* key)
{
	DataSchema* schema = getDataSchema();
	DataProperty* prop = schema ? schema->getProperty(key) : NULL;
	if (prop)
	{
		DataValue value;
		if (prop->getValue(this, value))
			return value;
	}

	return DataValue::Void();
}

Ref<DataObject> DataObject::clone() const
{
	// TODO: Implement clone context (to handle same object with different name)
	DataObject* obj = const_cast<DataObject*>(this);
	DataSchema* schema = obj->getDataSchema();
	if (schema == NULL)
		NIT_THROW_FMT(EX_INVALID_STATE, "can't clone object without schema");

	bool useDataSave = true; 

	if (useDataSave)
	{
		// save to a memory buffer and load from it (ineffective)
		// TODO: optimize

		Ref<MemoryBuffer::Writer> writer = new MemoryBuffer::Writer();
		Ref<BinDataSaver> saver = new BinDataSaver();
		saver->save(DataValue(obj), writer);

		Ref<StreamReader> reader = new MemoryBuffer::Reader(writer->getBuffer(), NULL);
		Ref<BinDataLoader> loader = new BinDataLoader();
		DataValue value;
		loader->load(value, reader);

		Ref<DataObject> clone = value.toObject();

		if (clone == NULL || clone->getDataSchema() != schema)
			NIT_THROW_FMT(EX_CORRUPTED, "can't clone object with data save");

		return clone;
	}

	// NOTE: This routine is dangerous for the case same object with different name
	// So using useDataSave=true for now

	Ref<DataObject> clone = schema->create();

	DataObjectContext ctx(0, obj);
	obj->onSaveBegin(&ctx);

	ctx.setObject(clone);
	clone->onLoadBegin(&ctx);

	DataSchema::OrderedProperties& props = schema->getOrderedProperties();
	for (uint i = 0; i < props.size(); ++i)
	{
		DataProperty* prop = props[i];
		if (prop->isReadOnly() || !prop->isSave())
			continue;

		DataValue value;
		bool ok = prop->getValue(obj, value);
		if (!ok) NIT_THROW_FMT(EX_CORRUPTED, "can't clone get property '%s.%s'", schema->getName().c_str(), prop->getName().c_str());

		// TODO: Handle the case for same object with different name (blind copy now)

		ok = prop->setValue(clone, value.clone());
		if (!ok) NIT_THROW_FMT(EX_CORRUPTED, "can't clone set property '%s.%s'", schema->getName().c_str(), prop->getName().c_str());
	}

	clone->onLoadEnd(&ctx);

	ctx.setObject(obj);
	obj->onSaveEnd(&ctx);

	return clone;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
