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

#include "nit/data/DataSchema.h"
#include "nit/io/Stream.h"
#include "nit/data/ParserUtil.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class DataContext;
class BinDataContext;

////////////////////////////////////////////////////////////////////////////////

class NIT_API BinDataSaver : public RefCounted
{
public:
	BinDataSaver(BinDataContext* context = NULL);

public:
	void								save(const DataValue& value, StreamWriter* w);
	void								saveCompressed(const DataValue& value, StreamWriter* w);

public:									// partial write
	void								write(const DataValue& value, StreamWriter* w) { _writer = w; writeValue(value); }

private:
	template <typename TValue>
	inline void							write(const TValue& value)				{ _writer->writeRaw(&value, sizeof(value)); }

	inline void							writeType(uint8 type)					{ write(type); }
	void								writeString(const char* str, uint32 len);
	void								writeBlob(const void* blob, uint32 size);
	void								writeKey(DataKey* key);

	void								writeValue(const DataValue& value);
	void								writeArray(DataArray* array);
	void								writeRecord(DataRecord* record);
	void								writeObject(Ref<DataObject> object);
	void								writeBuffer(MemoryBuffer* buffer);

	Ref<BinDataContext>					_context;
	StreamWriter*						_writer;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API JsonDataSaver : public JsonPrinter
{
public:
	JsonDataSaver(StreamWriter* w = NULL, DataContext* context = NULL);

public:
	void								printValue(const DataValue& value);

public:
	void								setRecordSorted(bool flag)				{ _sortRecordKeys = flag; }
	bool								isRecordSorted()						{ return _sortRecordKeys; }

private:
	Ref<DataContext>					_context;
	bool								_sortRecordKeys;

	void								printArray(DataArray* array);
	void								printRecord(DataRecord* record);
	void								printObject(Ref<DataObject> object);
	void 								printBlob(const DataValue& value);
	void 								printBlob(const void* memory, size_t size);
	void 								printFloats(const char* type, float* floats, size_t numFloats);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API PlistDataSaver : public XmlPrinter
{
public:
	PlistDataSaver(StreamWriter* w = NULL, DataContext* context = NULL);

public:
	void								printHeader();
	void								printValue(const DataValue& value);

public:
	void								setRecordSorted(bool flag)				{ _sortRecordKeys = flag; }
	bool								isRecordSorted()						{ return _sortRecordKeys; }

private:
	Ref<DataContext>					_context;
	bool								_sortRecordKeys;

	void								printArray(DataArray* array);
	void								printRecord(DataRecord* rec);
	void								printObject(Ref<DataObject> object);
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
