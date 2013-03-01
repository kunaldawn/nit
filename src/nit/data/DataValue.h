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
#include "nit/runtime/MemManager.h"
#include "nit/io/ContentTypes.h"
#include "nit/io/MemoryBuffer.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

enum
{
	// NIT Data API Version 1.0
	NIT_DATA_SIGNATURE					= NIT_MAKE_CC('N', 'D', 0x01, 0x00),
	NIT_ZDATA_SIGNATURE					= NIT_MAKE_CC('N', 'Z', 0x01, 0x00),
};

////////////////////////////////////////////////////////////////////////////////

class DataValue;

class DataArray;
class DataRecord;
class DataObject;
class DataSchema;
class DataProperty;

class DataContext;
class DataObjectContext;

class DataLoader;
class DataSaver;

class ScriptDataValue;
class NB_DataValue;
class StreamSource;
class StreamReader;
class StreamWriter;

////////////////////////////////////////////////////////////////////////////////

struct NIT_API Float2
{
	float x, y;
};

struct NIT_API Float3
{
	float x, y, z;
};

struct NIT_API Float4
{
	float x, y, z, w;
};

struct NIT_API Float3x3
{
	Float3 a, b, c;
};

struct NIT_API Float4x4
{
	Float4 a, b, c, d;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API DataArray : public RefCounted
{
public:
	typedef vector<DataValue>::type		Container;
	typedef Container::iterator			Iterator;

public:
	DataArray*							clone() const;

public:
	uint								getCount()								{ return this ? _array.size() : 0; }
	DataValue							get(uint index);
	void								set(uint index, const DataValue& value);
	DataValue&							append(const DataValue& value)			{ ASSERT_THROW(this, EX_NULL); _array.push_back(value); return _array.back(); }
	void								insert(uint index, const DataValue& value) { ASSERT_THROW(this, EX_NULL); _array.insert(_array.begin() + index, value); }
	void								erase(uint index, uint count = 1)		{ if (this) { _array.erase(_array.begin() + index, _array.begin() + index + count); } }
	void								clear()									{ if (this) { _array.clear(); } }

	Iterator							begin()									{ return this ? _array.begin() : s_NullItr; }
	Iterator							end()									{ return this ? _array.end() : s_NullItr; }

public:
	void								dump();

private:
	Container							_array;

	static Iterator						s_NullItr;

	friend class						NB_DataValue;
};

////////////////////////////////////////////////////////////////////////////////

class DataKey;

class NIT_API DataNamespace : public RefCounted
{
public:
	DataNamespace(const String& name) : _name(name)							{ }

	static DataNamespace*				getGlobal();

public:
	const String&						getName()								{ return _name; }
	String								getFullName()							{ return _parent ? _parent->getFullName() + "." + _name : _name; }

public:
	Ref<DataKey>						add(const String& name);
	DataKey*							get(const String& name);

public:
	typedef unordered_map<String, DataKey*>::type NameMap;
	typedef NameMap::iterator			Iterator;

	Iterator							begin()									{ return this ? _names.begin() : s_NullItr; }
	Iterator							end()									{ return this ? _names.end() : s_NullItr; }

protected:
	virtual void						onDelete();

private:
	NameMap								_names;
	Ref<DataNamespace>					_parent;
	String								_name;

	static Iterator						s_NullItr;

private:
	friend class DataKey;
	void								onDeleteKey(DataKey* key);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API DataKey : public RefCounted
{
public:
	virtual ~DataKey()															{ if (_namespace) _namespace->onDeleteKey(this); }

public:
	const String&						getName()								{ return _iterator->first; }
	String								getFullName()							{ return _namespace->getFullName() + "." + getName(); }
	DataNamespace*						getNamespace()							{ return _namespace; }

public:
	typedef uint						HashValue;
	HashValue							getHashValue()							{ return _hashValue; }

	static HashValue					calcHashValue(const char* str, int len = -1);

private:
	friend class DataNamespace;
	Ref<DataNamespace>					_namespace;
	DataNamespace::Iterator				_iterator;

	HashValue							_hashValue;
};

inline size_t hash_value(const Ref<DataKey>& name)								{ return (size_t)name->getHashValue(); }

////////////////////////////////////////////////////////////////////////////////

class NIT_API HashTable : public PooledAlloc
{
public:
	HashTable(uint capacity = 0);
	~HashTable();

	HashTable*							clone();

public:
	uint								getCount()								{ return _usedNodes; }
	uint								getCapacity()							{ return _numNodes; }

	inline DataValue					get(DataKey* key);
	inline DataValue&					set(DataKey* key, const DataValue& value);
	void								Delete(DataKey* key);
	void								clear();

public:
	typedef std::pair<Ref<DataKey>, DataValue> Pair;

	class NIT_API Iterator
	{
	public:
		Iterator() : _index(-1), _table(NULL), _pair(NULL)					{ }

		bool							operator == (const Iterator& other)		{ return _pair == other._pair; }
		bool							operator != (const Iterator& other)		{ return _pair != other._pair; }
		Iterator&						operator ++ ()							{ _pair = _table->next(_index); return *this; }
		Iterator						operator ++ (int)						{ Iterator tmp = *this; ++*this; return tmp; }
		Pair&							operator * ()							{ return *_pair; }
		Pair*							operator -> ()							{ return _pair; }

	private:
		friend class HashTable;
		Iterator(HashTable* table) : _index(0), _table(table), _pair(NULL)	{ _pair = _table->next(_index); }

		int								_index;
		HashTable*						_table;
		Pair*							_pair;
	};

	Iterator							begin()									{ return Iterator(this); }
	Iterator							end()									{ return Iterator(); }

	Pair*								next(int& varIndex);

private:
	class HashNode;

	HashNode*							_firstFree;
	HashNode*							_nodes;
	uint								_numNodes;
	uint								_usedNodes;

	void								allocNodes(uint newSize);
	void								rehash(bool forced);
	inline HashNode*					getNode(DataKey* key, uint index);
	DataValue&							newSlot(DataKey* key, const DataValue& value);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API DataRecord : public RefCounted
{
public:
	DataRecord(DataNamespace* ns = NULL);

public:
	DataRecord*							clone() const;

public:
	uint								getCount()								{ return this ? _table.getCount() : 0; }

	inline DataValue&					set(const String& key, const DataValue& value);
	inline DataValue&					set(DataKey* key, const DataValue& value);

	DataValue							get(const String& key);
	DataValue							get(DataKey* key);

	void								find(const String& pattern, vector<DataValue>::type& outValues);

	void								Delete(const String& key);
	void								Delete(DataKey* key);

	void								clear();

public:
	DataNamespace*						getNamespace()							{ return this ? _namespace : NULL; } 

	HashTable&							getTable()								{ return _table; }

	typedef HashTable::Iterator			Iterator;
	Iterator							begin()									{ return this ? _table.begin() : s_NullItr; }
	Iterator							end()									{ return this ? _table.end() : s_NullItr; }

public:
	void								dump();

private:
	HashTable							_table;
	Ref<DataNamespace>					_namespace;

	static Iterator						s_NullItr;

	virtual void						onDelete();
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API DataObject : public RefCounted
{
public:
	virtual DataSchema*					getDataSchema() = 0;
	virtual void*						getRealPtr()							{ return this; }

	void								setProperty(const String& key, const DataValue& value);
	void								setProperty(DataKey* key, const DataValue& value);

	DataValue							getProperty(const String& key);
	DataValue							getProperty(DataKey* key);

	virtual Ref<DataObject>				clone() const;

protected:
	friend class DataSchema;
	friend class DataContext;
	virtual void						onLoadBegin(DataObjectContext* context)	{ }
	virtual void						onLoadEnd(DataObjectContext* context)	{ }

	virtual void						onSaveBegin(DataObjectContext* context)	{ }
	virtual void						onSaveEnd(DataObjectContext* context)	{ }

	friend class						NB_DataValue;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API DataChunk
{
public:
	static DataChunk*					alloc(size_t size);
	static DataChunk*					realloc(DataChunk* chunk, size_t size);
	static void							free(DataChunk* chunk);

public:
	inline void							incRefCount()							{ _refCount.inc(); }
	inline void							decRefCount()							{ if (_refCount.decGet() == 0) free(this); }

public:
	void*								getMemory(uint offset)					{ return ((uint8*)this) + sizeof(DataChunk) + offset; }

private:
	DataChunk(size_t allocSize) : _refCount(0), _allocSize(allocSize) { }
	~DataChunk()						{ }

	AtomicInt							_refCount;
	size_t								_allocSize;
};

////////////////////////////////////////////////////////////////////////////////

#undef TYPE_BOOL // iOS seems to have TYPE_BOOL macro, so ignore it 

// NOTE: When you add a brand new type, check follows:
// - DataValue: type enum
// - DataValue: constructor
// - DataValue: cast operator
// - DataValue: ToNewType()
// - DataValue: ConvertToNewTYpe()
// - DataValue: ParseNewType()
// - DataValue: add a case to every ConvertToXXX() functions
// - DataValue: add a case to DoConvert()
// - DataValue: add a string mapping to Str2Type() functino
// - DataValue: add a case DumpString()
// - ScriptDataValue: add a case to Push()
// - ScriptDataValue: add a case to ToValue()
// - NB_DataValue: add a ToXXX() FUNC_ENTRY
// - Database::Query: add a case to BindValue()
// - BinDataLoader: add a case to ReadValue()
// - JSONDataSaver, PListDataSaver: inspect code carefully if has related part.
// - BinDataSaver: add a case to WriteValue()
// - JSONDataSaver: add a case to PrintValue()
// - PListDataSaver: add a case to PrintValue()
// - wxVariantDataValue: add a case GetAsAny()
// TODO: Overwhelming! Nit so nit!

////////////////////////////////////////////////////////////////////////////////

class NIT_API DataValue : public PooledAlloc
{
public:
	enum Type							// uint8
	{
		MAX_SMALLDATA_SIZE				= 12,

		// Flags
		FLAG_COMPATIBLE					= 0x08,
		FLAG_STRUCTURED					= 0x10,
		FLAG_REFERENCE					= 0x20,
		FLAG_VAR_SIZED					= 0x40,

		// max 8 types per same flag combination

		// Utility Types
		TYPE_NULL						= 0x00,	// default, found but 'null'
		TYPE_VOID						= 0x01,	// 'not found'
		TYPE_ANY						= 0x02, // property wildcard

		// Primitive Types
		TYPE_BOOL						= 0x03,
		TYPE_INT						= 0x04,
		TYPE_INT64						= 0x05,
		TYPE_DOUBLE						= 0x06,
		TYPE_FLOAT						= 0x07,

		// Var-sized Types
		TYPE_STRING						= 0x00 | FLAG_VAR_SIZED,
		TYPE_BLOB						= 0x01 | FLAG_VAR_SIZED,

		// Reference Types
		TYPE_ARRAY						= 0x00 | FLAG_REFERENCE,
		TYPE_RECORD						= 0x01 | FLAG_REFERENCE,
		TYPE_OBJECT						= 0x02 | FLAG_REFERENCE,
		TYPE_BUFFER						= 0x03 | FLAG_REFERENCE,
		TYPE_KEY						= 0x04 | FLAG_REFERENCE,

		// Structured Types
		TYPE_FLOAT2						= 0x00 | FLAG_STRUCTURED,
		TYPE_FLOAT3						= 0x01 | FLAG_STRUCTURED,
		TYPE_FLOAT4						= 0x02 | FLAG_STRUCTURED,
		TYPE_FLOAT3X3					= 0x03 | FLAG_STRUCTURED,
		TYPE_FLOAT4X4					= 0x04 | FLAG_STRUCTURED,

		// Compatible Types
		TYPE_TIMESTAMP					= 0x00 | FLAG_COMPATIBLE,					// int64

		TYPE_VECTOR2					= 0x00 | FLAG_STRUCTURED | FLAG_COMPATIBLE, // Float2
		TYPE_SIZE2						= 0x01 | FLAG_STRUCTURED | FLAG_COMPATIBLE, // Float2
		TYPE_VECTOR3					= 0x02 | FLAG_STRUCTURED | FLAG_COMPATIBLE, // Float3
		TYPE_SIZE3						= 0x03 | FLAG_STRUCTURED | FLAG_COMPATIBLE, // Float3
		TYPE_VECTOR4					= 0x04 | FLAG_STRUCTURED | FLAG_COMPATIBLE, // Float4
		TYPE_QUAT						= 0x05 | FLAG_STRUCTURED | FLAG_COMPATIBLE, // Float4
		TYPE_MATRIX3					= 0x06 | FLAG_STRUCTURED | FLAG_COMPATIBLE, // Float3x3
		TYPE_MATRIX4					= 0x07 | FLAG_STRUCTURED | FLAG_COMPATIBLE, // Float4x4
	};

public:
	inline DataValue()									: _meta(0)				{ /* init to NULL */ }
	inline ~DataValue()															{ release(); }

public:
	inline DataValue(const DataValue& other)			: _full(other._full)	{ if (_isRef) _ref->incRefCount(); if (_isChunked) _chunk->incRefCount(); }

	inline DataValue(bool value)						: _meta(0)				{ setData(TYPE_BOOL, (uint8)value); }
	inline DataValue(int value)							: _meta(0)				{ setData(TYPE_INT, value); }
	inline DataValue(int64 value)						: _meta(0)				{ setData(TYPE_INT64, value); }
	inline DataValue(float value)						: _meta(0)				{ setData(TYPE_FLOAT, value); }
	inline DataValue(double value)						: _meta(0)				{ setData(TYPE_DOUBLE, value); }

	inline DataValue(const Float2& value)				: _meta(0)				{ setData(TYPE_FLOAT2, value); }
	inline DataValue(const Float3& value)				: _meta(0)				{ setData(TYPE_FLOAT3, value); }
	inline DataValue(const Float4& value)				: _meta(0)				{ setData(TYPE_FLOAT4, value); }

	inline DataValue(const Float3x3& value)				: _meta(0)				{ setData(TYPE_FLOAT3X3, value); }
	inline DataValue(const Float4x4& value)				: _meta(0)				{ setData(TYPE_FLOAT4X4, value); }

	inline DataValue(const Timestamp& value)			: _meta(0)				{ setData(TYPE_TIMESTAMP, value); }

	inline DataValue(const Vector2& value)				: _meta(0)				{ setData(TYPE_VECTOR2, value); }
	inline DataValue(const Vector3& value)				: _meta(0)				{ setData(TYPE_VECTOR3, value); }
	inline DataValue(const Vector4& value)				: _meta(0)				{ setData(TYPE_VECTOR4, value); }
	inline DataValue(const Quat& value)					: _meta(0)				{ setData(TYPE_QUAT, value); }
	inline DataValue(const Matrix3& value)				: _meta(0)				{ setData(TYPE_MATRIX3, value); }
	inline DataValue(const Matrix4& value)				: _meta(0)				{ setData(TYPE_MATRIX4, value); }

	inline DataValue(const void* blob, size_t size)		: _meta(0)				{ setBlob(blob, size); }
	inline DataValue(const char* str, int len = -1)		: _meta(0)				{ setString(str, len); }
	inline DataValue(const String& value)				: _meta(0)				{ setString(value.c_str(), (int)value.length()); }

	inline DataValue(DataArray* array)					: _meta(0)				{ setRef(TYPE_ARRAY, array); }
	inline DataValue(DataRecord* record)				: _meta(0)				{ setRef(TYPE_RECORD, record); }
	inline DataValue(DataObject* object)				: _meta(0)				{ setRef(TYPE_OBJECT, object); }
	inline DataValue(MemoryBuffer* buffer)				: _meta(0)				{ setRef(TYPE_BUFFER, buffer); }
	inline DataValue(DataKey* key)						: _meta(0)				{ setRef(TYPE_KEY, key); }

	inline DataValue(Ref<DataArray> array)				: _meta(0)				{ setRef(TYPE_ARRAY, array.get()); }
	inline DataValue(Ref<DataRecord> record)			: _meta(0)				{ setRef(TYPE_RECORD, record.get()); }
	inline DataValue(Ref<DataObject> object)			: _meta(0)				{ setRef(TYPE_OBJECT, object.get()); }
	inline DataValue(Ref<MemoryBuffer> buffer)			: _meta(0)				{ setRef(TYPE_BUFFER, buffer.get()); }
	inline DataValue(Ref<DataKey> key)					: _meta(0)				{ setRef(TYPE_KEY, key.get()); }

public:
	inline DataValue& operator = (const DataValue& other)						{ share(other); return *this; }
	inline DataValue& operator = (const char* str)								{ setString(str, -1); return *this; }
	inline DataValue& operator = (const String& value)							{ setString(value.c_str(), (int)value.length()); return *this; }

public:
	// TODO: Following casting operators are reported to be dangerous on certain condition. Should we remove them?
	inline operator bool ()														{ return toBool(); }
	inline operator int ()														{ return toInt(); }
	inline operator int64 ()													{ return toInt64(); }
	inline operator float ()													{ return toFloat(); }
	inline operator double ()													{ return toDouble(); }
	inline operator const Float2& ()											{ return toFloat2(); }
	inline operator const Float3& ()											{ return toFloat3(); }
	inline operator const Float4& ()											{ return toFloat4(); }
	inline operator const Float3x3& ()											{ return toFloat3x3(); }
	inline operator const Float4x4& ()											{ return toFloat4x4(); }

	inline operator Timestamp ()												{ return toTimestamp(); }

	inline operator DataArray* ()												{ return toArray(); }
	inline operator DataRecord* ()												{ return toRecord(); }
	inline operator DataObject* ()												{ return toObject(); }
	inline operator MemoryBuffer* ()											{ return toBuffer(); }
	inline operator DataKey* ()													{ return toKey(); }

	inline operator Ref<DataArray> ()											{ return toArray(); }
	inline operator Ref<DataRecord> ()											{ return toRecord(); }
	inline operator Ref<DataObject> ()											{ return toObject(); }
	inline operator Ref<MemoryBuffer> ()										{ return toBuffer(); }
	inline operator Ref<DataKey> ()												{ return toKey(); }

public:
	inline Type							getType() const							{ return (Type)_type; }
	inline bool							isRef() const							{ return _isRef; }
	inline bool							isChunked() const						{ return _isChunked; }

	inline bool							isVoid() const							{ return _type == TYPE_VOID; }
	inline bool							isNull() const							{ return _type == TYPE_NULL; }
	inline bool							isVoidOrNull() const					{ return _type == TYPE_VOID || _type == TYPE_NULL; }
	inline bool							isBlob() const							{ return _type == TYPE_BLOB || _type == TYPE_BUFFER; }

	inline static DataValue				Void()									{ DataValue v; v._type = TYPE_VOID; return v; }
	inline static DataValue				Null()									{ return DataValue(); }

public:
	DataValue							clone() const;

public:
	template <typename TValue>
	inline DataValue&					Default(const TValue& defValue)			{ if (_type == TYPE_VOID || _type == TYPE_NULL) *this = defValue; return *this; }
	
	inline void							toVoid()								{ release(); _type = TYPE_VOID; }
	inline void							toNull()								{ release(); _type = TYPE_NULL; }

	inline const void*					toBlob(size_t* outSize);
	inline const char*					c_str(size_t* outLen = NULL);
	inline String						toString()								{ size_t len; const char* str = c_str(&len); return String(str, len); }

	inline bool							toBool()								{ return _type == TYPE_BOOL ? getData<bool>() : convertToBool(); }
	inline int							toInt()									{ return _type == TYPE_INT ? getData<int>() : convertToInt(); }
	inline int64						toInt64()								{ return _type == TYPE_INT64 ? getData<int64>() : convertToInt64(); }
	inline float						toFloat()								{ return _type == TYPE_FLOAT ? getData<float>() : convertToFloat(); }
	inline double						toDouble()								{ return _type == TYPE_DOUBLE ? getData<double>() : convertToDouble(); }

	inline const Float2&				toFloat2()								{ return _type == TYPE_FLOAT2 ? getData<Float2>() : convertToFloat2(); }
	inline const Float3&				toFloat3()								{ return _type == TYPE_FLOAT3 ? getData<Float3>() : convertToFloat3(); }
	inline const Float4&				toFloat4()								{ return _type == TYPE_FLOAT4 ? getData<Float4>() : convertToFloat4(); }
	inline const Float3x3&				toFloat3x3()							{ return _type == TYPE_FLOAT3X3 ? getData<Float3x3>() : convertToFloat3x3(); }
	inline const Float4x4&				toFloat4x4()							{ return _type == TYPE_FLOAT4X4 ? getData<Float4x4>() : convertToFloat4x4(); }

	inline const Timestamp&				toTimestamp()							{ return _type == TYPE_TIMESTAMP ? getData<Timestamp>() : convertToTimestamp(); }

	inline DataArray*					toArray()								{ return _type == TYPE_ARRAY ? getRef<DataArray>() : convertToArray(); }
	inline DataRecord*					toRecord()								{ return _type == TYPE_RECORD ? getRef<DataRecord>() : convertToRecord(); }
	inline DataObject*					toObject()								{ return _type == TYPE_OBJECT ? getRef<DataObject>() : convertToObject(); }
	inline MemoryBuffer*				toBuffer()								{ return _type == TYPE_BUFFER ? getRef<MemoryBuffer>() : ConvertToBuffer(); }
	inline DataKey*						toKey()									{ return _type == TYPE_KEY ? getRef<DataKey>() : ConvertToKey(); }

	inline const Vector2&				toVector2()								{ if (_type == TYPE_FLOAT2) return getData<Vector2>(); convertToFloat2(); return getData<Vector2>(); }
	inline const Vector3&				toVector3()								{ if (_type == TYPE_FLOAT3) return getData<Vector3>(); convertToFloat3(); return getData<Vector3>(); }
	inline const Vector4&				toVector4()								{ if (_type == TYPE_FLOAT4) return getData<Vector4>(); convertToFloat4(); return getData<Vector4>(); }
	inline const Quat&					toQuat()								{ if (_type == TYPE_FLOAT4) return getData<Quat>(); convertToFloat4(); return getData<Quat>(); }
	inline const Matrix3&				toMatrix3()								{ if (_type == TYPE_FLOAT3X3) return getData<Matrix3>(); convertToFloat3x3(); return getData<Matrix3>(); }
	inline const Matrix4&				toMatrix4()								{ if (_type == TYPE_FLOAT4X4) return getData<Matrix4>(); convertToFloat3x3(); return getData<Matrix4>(); }

public:
	inline DataValue&					convertTo(Type type)					{ return (type != TYPE_ANY && _type != type) ? doConvert(type) : *this; }

public:
	static DataValue					fromJson(const String& str)				{ return fromJson(str.c_str(), str.length()); }
	static DataValue					fromJson(const char* str, int len = -1);
	String								toJson(bool compact = true);

	static DataValue					load(StreamSource* source, ContentType treatAs = ContentType::UNKNOWN);
	void								save(StreamWriter* w);
	void								saveCompressed(StreamWriter* w);
	void								saveJson(StreamWriter* w);
	void								savePlist(StreamWriter* w);

	void								loadString(StreamReader* r, size_t len);
	void								loadBlob(StreamReader* r, size_t size);
	void								loadData(Type type, StreamReader* r, size_t size);

public:									
	DataValue							get(const String& key);					// Record, Object: Get(key)
	DataValue							get(DataKey* key);						// Record, Object: Get(key)
	DataValue							get(uint index);						// Array: Get(index), otherwise: index == 0 as self
	uint								getCount();								// Record or Array: count, null or void : 0, else : 1

public:
	static const char*					typeToStr(Type t);
	static Type							strToType(const char* str);
	static void							allTypes(vector<std::pair<std::string, Type> >::type& outResults);

public:
	template <typename TValue>
	static inline TValue				parse(const char* str)					{ TValue ret; parse(str, ret); return ret; }

	static bool							parse(const char* str, int& outValue);
	static bool							parse(const char* str, int64& outValue);
	static bool							parse(const char* str, float& outValue);
	static bool							parse(const char* str, double& outValue);
	static bool							parse(const char* str, Float2& outValue);
	static bool							parse(const char* str, Float3& outValue);
	static bool							parse(const char* str, Float4& outValue);
	static bool							parse(const char* str, Float3x3& outValue);
	static bool							parse(const char* str, Float4x4& outValue);
	static bool							parse(const char* str, bool& outValue);

public:
	String								dumpString();

public:
#if defined(NIT_DATA_RELOCATABLE_CHUNK)
	template <typename TValue>
	inline const TValue*				getDataPtr() const						{ return (TValue*) (_isChunked ? _chunkAddr : _smallData); }

	template <typename TValue>
	inline size_t						getDataSize() const						{ return _isChunked ? _chunkSize : _smallDataSize; }
#else
	template <typename TValue>
	inline const TValue*				getDataPtr() const						{ return (TValue*) (sizeof(TValue) > MAX_SMALLDATA_SIZE ? _chunkAddr : _smallData); }

	template <typename TValue>
	inline size_t						getDataSize() const						{ return sizeof(TValue); }
#endif

	template <typename TRefClass>
	inline TRefClass*					getRef() const							{ return (TRefClass*) _ref; }

	inline const void*					getBlobPtr() const						{ return _isChunked ? _chunkAddr : _smallData; }
	inline size_t						getBlobSize() const						{ return _isChunked ? _chunkSize : _smallDataSize; }
	inline const char*					getStringPtr() const					{ return (const char*)getBlobPtr(); }
	inline size_t						getStringSize() const					{ return getBlobSize() - 1; }

	size_t								copyBlobInto(void* buf, size_t bufLen) const;

	template <typename TValue>
	inline const TValue&				getData() const							{ return *getDataPtr<TValue>(); }

private:								// coercions
	bool								convertToBool();
	int									convertToInt();
	int64								convertToInt64();
	float								convertToFloat();
	double								convertToDouble();
	const Float2&						convertToFloat2();
	const Float3&						convertToFloat3();
	const Float4&						convertToFloat4();
	const Float3x3&						convertToFloat3x3();
	const Float4x4&						convertToFloat4x4();
	const Timestamp&					convertToTimestamp();
	const char*							convertToString();
	const void*							convertToBlob();
	DataArray*							convertToArray();
	DataRecord*							convertToRecord();
	DataObject*							convertToObject();
	MemoryBuffer*						ConvertToBuffer();
	DataKey*							ConvertToKey();

	DataValue&							doConvert(Type type);

	inline const Float2&				convertToFloat2(Type type) { const Float2& ret = convertToFloat2(); _type = type; return ret; }
	inline const Float3&				convertToFloat3(Type type) { const Float3& ret = convertToFloat3(); _type = type; return ret; }
	inline const Float4&				convertToFloat4(Type type) { const Float4& ret = convertToFloat4(); _type = type; return ret; }
	inline const Float3x3&				convertToFloat3x3(Type type) { const Float3x3& ret = convertToFloat3x3(); _type = type; return ret; }
	inline const Float4x4&				convertToFloat4x4(Type type) { const Float4x4& ret = convertToFloat4x4(); _type = type; return ret; }

private:
	union								// 16 bytes
	{
		RefCounted*						_ref;

		struct
		{
			uint64						_a;
			uint64						_b;
		} _full;

		struct 
		{
			uint8						_smallData[MAX_SMALLDATA_SIZE];
			uint						_type			: 8;
			uint						_isRef			: 1;
			uint						_isChunked		: 1;
			uint						_reservedFlags	: 2;
			uint						_smallDataSize	: 4; // when data occupies _smallData buffer
		};

		struct
		{
			uint8						_unused[MAX_SMALLDATA_SIZE];
			uint32						_meta;
		};

		struct 
		{
			uint8*						_chunkAddr;
			DataChunk*					_chunk;
			size_t						_chunkSize;
		};
	};

	friend class						ScriptDataValue;
	friend class						NB_DataValue;

	template <typename TValue>
	inline void							setData(Type type, const TValue& value);

	inline void							setString(const String& str)			{ setString(str.c_str(), str.length()); }
	inline void							setString(const char* str, int len = -1);
	inline void							setBlob(const void* blob, size_t size);
	inline void							setRef(Type type, RefCounted* obj);
	inline void							setChunk(Type type, DataChunk* chunk, int offset, int size);
	inline void							share(const DataValue& other);

	inline void							release()								{ if (_isRef || _isChunked) doRelease(); }
	void*								allocChunk(size_t sz);
	void								doRelease();
};

////////////////////////////////////////////////////////////////////////////////

template <typename TValue>
inline void DataValue::setData(Type type, const TValue& value)
{
	release();

	// value-type only
	size_t sz = sizeof(value);

	if (sz <= MAX_SMALLDATA_SIZE)
	{
		*(TValue*)_smallData = value;
	}
	else
	{
		*(TValue*)allocChunk(sz) = value;
	}

	_type = type;
}

inline void DataValue::setString(const char* str, int len /*= -1*/)
{
	release();

	if (str)
	{
		_type = TYPE_STRING;

		if (len == -1) len = strlen(str);

		// make sure null-terminated,
		// string needs len+1 for '\0'
		if (len < MAX_SMALLDATA_SIZE)
		{
			_smallData[len] = 0;
			_smallDataSize = len+1;
			memcpy(_smallData, str, len);
		}
		else
		{
			char* data = (char*)allocChunk(len+1);
			data[len] = 0;
			memcpy(data, str, len);
		}
	}
	else
		_type = TYPE_NULL;
}

inline void DataValue::setBlob(const void* blob, size_t size)
{
	release();

	if (blob)
	{
		_type = TYPE_BLOB;

		if (size <= MAX_SMALLDATA_SIZE)
		{
			_smallDataSize = size;
			memcpy(_smallData, blob, size);
		}
		else
		{
			memcpy(allocChunk(size), blob, size);
		}
	}
	else
		_type = TYPE_NULL;
}

inline void DataValue::setRef(Type type, RefCounted* obj)
{
	if (obj) obj->incRefCount();

	release();

	if (obj)
	{
		_type = type;
		_isRef = true;
		_ref = obj;
	}
	else
		_type = TYPE_NULL;
}

inline void DataValue::setChunk(Type type, DataChunk* chunk, int offset, int size)
{
	chunk->incRefCount();

	release();

	if (chunk)
	{
		_chunk = chunk;
		_chunkAddr = (uint8*)_chunk->getMemory(offset);
		_chunkSize = size;

		_type = type;
	}
	else
		_type = TYPE_NULL;
}

inline void DataValue::share(const DataValue& other)
{
	if (other._isRef) other._ref->incRefCount();
	if (other._isChunked) other._chunk->incRefCount();

	release();

	_full = other._full;
}

inline const void* DataValue::toBlob(size_t* outSize)
{
	if (_type != TYPE_BLOB) convertToBlob();
	if (outSize) *outSize = getBlobSize();
	return getBlobPtr();
}

inline const char* DataValue::c_str(size_t* outLen)
{
	if (_type != TYPE_STRING) convertToString();
	if (outLen) *outLen = getStringSize();
	return getStringPtr();
}

////////////////////////////////////////////////////////////////////////////////

inline DataValue DataArray::get(uint index)
{
	return this && index < _array.size() ? _array[index] : DataValue::Void();
}

inline void DataArray::set(uint index, const DataValue& value)
{
	ASSERT_THROW(this, EX_NULL); 
	ASSERT_THROW(index < _array.size(), EX_INVALID_RANGE);
	_array[index] = value;
}

////////////////////////////////////////////////////////////////////////////////

class HashTable::HashNode
{
public:
	HashNode() : _next(NULL)			{ }

	Pair								_pair;
	HashNode*							_next;
};

inline DataValue HashTable::get(DataKey* key)
{
	if (key)
	{
		HashNode* n = getNode(key, key->getHashValue());
		if (n) return n->_pair.second;
	}

	return DataValue::Void();
}

inline DataValue& HashTable::set(DataKey* key, const DataValue& value)
{
	ASSERT_THROW(key != NULL, EX_INVALID_PARAMS);

	HashNode* n = getNode(key, key->getHashValue());

	if (n)
		return (n->_pair.second = value);
	else
		return newSlot(key, value);
}

inline HashTable::HashNode* HashTable::getNode(DataKey* key, uint hash)
{
	if (_nodes)
	{
		for (HashNode* n = &_nodes[hash & (_numNodes - 1)]; n; n = n->_next)
			if (n->_pair.first == key) return n;
	}

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////

inline DataValue& DataRecord::set(const String& name, const DataValue& value)
{
	ASSERT_THROW(this, EX_NULL);

	Ref<DataKey> key = _namespace->add(name);

	return _table.set(key, value);
}

inline DataValue& DataRecord::set(DataKey* key, const DataValue& value)
{
	ASSERT_THROW(this, EX_NULL);
	ASSERT_THROW(key && key->getNamespace() == _namespace, EX_INVALID_PARAMS);

	return _table.set(key, value);
}

inline DataValue DataRecord::get(const String& name)
{
	return get(this ? _namespace->get(name) : NULL);
}

inline DataValue DataRecord::get(DataKey* key)
{
	if (this == NULL || key == NULL) 
		return DataValue::Void();
	return _table.get(key);
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;