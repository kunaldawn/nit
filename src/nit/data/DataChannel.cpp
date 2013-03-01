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

#include "nit/data/DataChannel.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

DataContext::DataContext(bool syncRWIndex, DataNamespace* ns)
{
	if (ns == NULL)
		ns = DataNamespace::getGlobal();

	_nextKeyIndex = 1;
	_nextObjectIndex = 1;

	_namespace = ns;
	_syncRWIndex = syncRWIndex;
}

// This method is for stub/proxy or singleton instances
void DataContext::replaceObject(uint32 id, DataObject* from, DataObject* to)
{
	if (from == to) return;

	if (to == NULL || to->getDataSchema() == NULL)
		NIT_THROW(EX_NOT_SUPPORTED);

	_objectToIndex.erase(from->getRealPtr());
	_indexToObject.erase(id);

	_objectToIndex.insert(std::make_pair(to->getRealPtr(), id));
	_indexToObject.insert(std::make_pair(id, to));
}

DataObjectContext* DataContext::beginSaveObject(Ref<DataObject> object)
{
	uint32 objectId = 0;

	void* realPtr = object->getRealPtr();
	ObjectToIndex::iterator itr = _objectToIndex.find(realPtr);

	if (itr != _objectToIndex.end())
	{
		objectId = itr->second;
	}
	else
	{
		objectId = _nextObjectIndex++;
		_objectToIndex.insert(std::make_pair(realPtr, objectId));

		if (_syncRWIndex)
			_indexToObject.insert(std::make_pair(objectId, object));
	}

	_contextStack.push_back(DataObjectContext(objectId, object));
	DataObjectContext& objContext = _contextStack.back();

	// TODO: turn on saving flag
	object->onSaveBegin(&objContext);

	if (objContext.getObject() != object)
		replaceObject(objectId, object, objContext.getObject());

	return &objContext;
}

void DataContext::endSaveObject()
{
	if (_contextStack.empty())
		NIT_THROW(EX_INVALID_STATE);

	DataObjectContext& objContext = _contextStack.back();
	Ref<DataObject> object = objContext.getObject();

	object->onSaveEnd(&objContext);
	// TODO: turn off saving flag

	if (objContext.getObject() != object)
		replaceObject(objContext.getObjectID(), object, objContext.getObject());

	_contextStack.pop_back();
}

uint32 DataContext::getObjectId(DataObject* obj)
{
	ObjectToIndex::iterator itr = _objectToIndex.find(obj->getRealPtr());

	return itr != _objectToIndex.end() ? itr->second : 0;
}

DataObject* DataContext::getObject(uint32 objectId)
{
	IndexToObject::iterator itr = _indexToObject.find(objectId);

	return itr != _indexToObject.end() ? itr->second : NULL;
}

Ref<DataObject> DataContext::createObject(Ref<DataKey> schemaKey)
{
	DataSchemaLookup* lookup = DataSchemaLookup::getCurrent();

	Ref<DataSchema> schema = lookup->load(schemaKey);

	// TODO: Recovery support

	// TODO: search for an alternative
	if (schema == NULL)
		NIT_THROW_FMT(EX_NOT_FOUND, "can't load schema '%s'", schemaKey->getName().c_str());

	Ref<DataObject> object = schema->create();
	// TODO: turn on create-loading flag

	if (object == NULL)
		NIT_THROW(EX_NULL);

	return object;
}

DataObjectContext* DataContext::beginLoadObject(Ref<DataObject> object, uint32 objectId, const DataValue& internal)
{
	_indexToObject.insert(std::make_pair(objectId, object));

	if (_syncRWIndex)
	{
		_objectToIndex.insert(std::make_pair(object->getRealPtr(), objectId));
		_nextObjectIndex = objectId + 1;
	}

	_contextStack.push_back(DataObjectContext(objectId, object));
	DataObjectContext& objContext = _contextStack.back();
	objContext.setInternal(internal);

	// TODO: turn on loading flag
	object->onLoadBegin(&objContext);

	if (objContext.getObject() != object)
		replaceObject(objectId, object, objContext.getObject());

	return &objContext;
}

Ref<DataObject> DataContext::endLoadObject()
{
	if (_contextStack.empty())
		NIT_THROW(EX_INVALID_STATE);

	DataObjectContext& objContext = _contextStack.back();
	Ref<DataObject> object = objContext.getObject();

	// TODO: turn off loading / create-loading flags
	object->onLoadEnd(&objContext);

	if (objContext.getObject() != object)
		replaceObject(objContext.getObjectID(), object, objContext.getObject());

	object = objContext.getObject();

	_contextStack.pop_back();

	return object;
}

////////////////////////////////////////////////////////////////////////////////

BinDataContext::BinDataContext(bool syncRWIndex, DataNamespace* ns)
: DataContext(syncRWIndex, ns)
{
}

nit::uint32 BinDataContext::writeKey(StreamWriter* w, DataKey* key)
{
	uint32 keyIndex = 0;

	if (key == NULL)
	{
		w->writeRaw(&keyIndex, sizeof(keyIndex));
		return keyIndex;
	}

	KeyToIndex::iterator itr = _keyToIndex.find(key);
	if (itr == _keyToIndex.end())
	{
		keyIndex = _nextKeyIndex++;
		_keyToIndex.insert(std::make_pair(key, keyIndex));

		w->writeRaw(&keyIndex, sizeof(keyIndex));
		writeString(w, key->getName());

		if (_syncRWIndex)
			_indexToKey.insert(std::make_pair(keyIndex, key));
	}
	else
	{
		keyIndex = itr->second;
		w->writeRaw(&keyIndex, sizeof(keyIndex));
	}

	return keyIndex;
}

Ref<DataKey> BinDataContext::readKey(StreamReader* r)
{
	uint32 keyIndex;
	if (r->readRaw(&keyIndex, sizeof(keyIndex)) != sizeof(keyIndex))
		NIT_THROW(EX_CORRUPTED);

	if (keyIndex == 0)
		return NULL;

	IndexToKey::iterator itr = _indexToKey.find(keyIndex);
	if (itr == _indexToKey.end())
	{
		String name;
		readString(r, name);
		Ref<DataKey> key = _namespace->add(name);
		_indexToKey.insert(std::make_pair(keyIndex, key));

		if (_syncRWIndex)
		{
			_keyToIndex.insert(std::make_pair(key, keyIndex));
			_nextKeyIndex = keyIndex + 1;
		}

		return key;
	}

	return itr->second;
}

void BinDataContext::writeString(StreamWriter* w, const char* str, uint32 len /*= 0*/)
{
	if (len == 0) len = strlen(str);

	w->writeRaw(&len, sizeof(len));
	w->writeRaw(str, len);
}

void BinDataContext::readString(StreamReader* r, String& outString)
{
	outString.resize(0);

	uint32 len;
	if (r->readRaw(&len, sizeof(len)) != sizeof(len))
		NIT_THROW(EX_CORRUPTED);

	char buf[256];

	while (len > 0)
	{
		size_t step = len < sizeof(buf) ? len : sizeof(buf);

		if (r->readRaw(buf, step) != step)
			NIT_THROW(EX_CORRUPTED);

		outString.append(buf, step);
		len -= step;
	}
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
