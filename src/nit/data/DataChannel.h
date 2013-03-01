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

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NIT_API DataObjectContext : public WeakSupported
{
public:
	DataObjectContext(uint32 id, DataObject* obj)
		: _objectID(id), _object(obj)											{ }

public:
	uint32								getObjectID()							{ return _objectID; }

	DataObject*							getObject()								{ return _object; }
	const DataValue&					getInternal()							{ return _internal; }

	void								setObject(DataObject* object)			{ _object = object; }
	void								setInternal(const DataValue& internal)	{ _internal = internal; }

private:
	uint32								_objectID;
	Ref<DataObject>						_object;
	DataValue							_internal;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API DataContext : public RefCounted
{
public:
	DataContext(bool syncRWIndex = false, DataNamespace* ns = NULL);

public:
	DataNamespace*						getNamespace()							{ return _namespace; }

	uint32								getObjectId(DataObject* obj);
	DataObject*							getObject(uint32 objectId);

public:
	DataObjectContext*					beginSaveObject(Ref<DataObject> object);
	void								endSaveObject();

	Ref<DataObject>						createObject(Ref<DataKey> schemaKey);
	DataObjectContext* 					beginLoadObject(Ref<DataObject> object, uint32 objectId, const DataValue& internal);
	Ref<DataObject>						endLoadObject();

protected:
	// TODO: Stack cleanup needed when exception thrown - scoped enter?

	vector<DataObjectContext>::type		_contextStack;
	void								replaceObject(uint32 id, DataObject* from, DataObject* to);

protected:
	typedef unordered_map<uint32, Ref<DataKey> >::type	IndexToKey;
	typedef unordered_map<Ref<DataKey>, uint32>::type	KeyToIndex;
	IndexToKey							_indexToKey;
	KeyToIndex							_keyToIndex;
	uint32								_nextKeyIndex;

	typedef unordered_map<uint32, Ref<DataObject> >::type IndexToObject;
	typedef unordered_map<void*, uint32>::type ObjectToIndex;
	IndexToObject						_indexToObject;
	ObjectToIndex						_objectToIndex;
	uint32								_nextObjectIndex;

	Ref<DataNamespace>					_namespace;

	bool								_syncRWIndex; // TODO: Separate DataLoadContext & DataSaveContext
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API BinDataContext : public DataContext
{
public:
	BinDataContext(bool syncRWIndex = false, DataNamespace* ns = NULL);

public:
	uint32								writeKey(StreamWriter* w, DataKey* key);
	Ref<DataKey>						readKey(StreamReader* r);

private:
	void								writeString(StreamWriter* w, const String& str) { writeString(w, str.c_str(), str.length()); }
	void								writeString(StreamWriter* w, const char* str, uint32 len = 0);
	void								readString(StreamReader* r, String& outString);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API DataChannel : public WeakSupported
{
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
