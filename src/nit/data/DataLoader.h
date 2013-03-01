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

class NIT_API BinDataLoader : public RefCounted
{
public:
	BinDataLoader(BinDataContext* context = NULL);

public:
	void								load(DataValue& outValue, Ref<StreamReader>);

public:									// Partial reading
	void								read(DataValue& outValue, StreamReader* r) { _reader = r; readValue(outValue); }

private:
	template <typename TValue>
	TValue								read()									{ TValue value; if (_reader->readRaw(&value, sizeof(value)) != sizeof(value)) NIT_THROW(EX_CORRUPTED); return value; }

	template <typename TValue>
	void								readData(DataValue::Type type, DataValue& outValue)	{ outValue.loadData(type, _reader, sizeof(TValue)); }

	void								readString(DataValue& outValue)			{ outValue.loadString(_reader, read<uint32>()); }
	void								readBlob(DataValue& outValue)			{ outValue.loadBlob(_reader, read<uint32>()); }

	DataValue::Type						readType()								{ return (DataValue::Type)read<uint8>(); }
	Ref<DataKey>						readKey();

	void								readValue(DataValue& outValue);
	void								readArray(DataValue& outValue);
	void								readRecord(DataValue& outValue);
	void								readObject(DataValue& outValue);
	void								readBuffer(DataValue& outValue);

	Ref<BinDataContext>					_context;
	StreamReader*						_reader;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API PlistDataLoader : public RefCounted
{
public:
	PlistDataLoader(DataContext* context = NULL);

public:
	void								load(DataValue& value, StreamReader* reader);

private:
	enum ElemState
	{
		ES_NONE,
		ES_ROOT,
		ES_DICT,
		ES_ARRAY,
	};

	Ref<DataContext>					_context;

	void*								_parser;
	DataValue*							_rootValue;
	Ref<MemoryBuffer>					_cdata;

	vector<ElemState>::type				_stateStack;
	vector<Ref<DataRecord> >::type		_dictStack;
	vector<Ref<DataArray> >::type		_arrayStack;
	StringVector						_keyStack;

	String								_error;

private:
	DataValue*							newValue();
	void								cleanup();
	void								error(const char* fmt, ...);

private:
	void								onStartElement(const char* name, const char** attrs);
	void								onEndElement(const char* name);
	void								onCharacterData(const char* s, int len);

	static void							startElementHandler(void* ctx, const char* name, const char** attrs);
	static void							endElementHandler(void* ctx, const char* name);
	static void							characterDataHandler(void* ctx, const char* s, int len);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API JsonDataLoader : public RefCounted, private Json::IHandler
{
public:
	JsonDataLoader(DataContext* context = NULL);

public:
	virtual void						load(DataValue& outValue, StreamReader* reader);

private:
	virtual void 						documentBegin()							{ /* nop */ }
	virtual void 						documentEnd();

	virtual void 						pairObjectBegin(const String& key)		{ pushObject(); }
	virtual void 						pair(const String& key, const char* value)	{ topObject()->set(key, value); }
	virtual void 						pair(const String& key, int value)		{ topObject()->set(key, value); }
	virtual void 						pair(const String& key, float value)	{ topObject()->set(key, value); }
	virtual void 						pair(const String& key, bool value)		{ topObject()->set(key, value); }
	virtual void 						pairNull(const String& key)				{ topObject()->set(key, DataValue()); }
	virtual void 						pairArrayBegin(const String& key)		{ pushArray(); }
	virtual void 						pairArrayEnd(const String& key)			{ Ref<DataArray> array = popArray(); topObject()->set(key, array.get()); }
	virtual void 						pairObjectEnd(const String& key);

	virtual void 						elementArrayBegin()						{ pushArray(); }
	virtual void 						element(const char* value)				{ element(DataValue(value)); }
	virtual void 						element(int value)						{ element(DataValue(value)); }
	virtual void 						element(float value)					{ element(DataValue(value)); }
	virtual void 						element(bool value)						{ element(DataValue(value)); }
	virtual void 						elementNull()							{ element(DataValue::Null()); }
	virtual void 						elementObjectBegin()					{ pushObject(); }
	virtual void 						elementObjectEnd();
	virtual void 						elementArrayEnd()						{ element(DataValue(popArray())); }

private:
	void								cleanup();

	DataRecord*							pushObject();
	Ref<DataRecord>						popObject();
	DataRecord*							topObject()								{ return _objStack.back(); }

	DataArray*							pushArray();
	Ref<DataArray>						popArray();
	DataArray*							topArray()								{ return _arrayStack.back(); }

	void								element(const DataValue& value);

	Ref<DataObject>						toDataObject(DataRecord* rec);

private:
	Ref<DataContext>					_context;

	Ref<DataKey>						k_ObjectID;
	Ref<DataKey>						k_Schema;
	Ref<DataKey>						k_Internal;

	vector<Ref<DataRecord> >::type		_objStack;
	vector<Ref<DataArray> >::type		_arrayStack;

	DataValue							_rootObject;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
