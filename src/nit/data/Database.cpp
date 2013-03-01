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

#include "nit/data/Database.h"

#include "nit/io/FileLocator.h"
#include "nit/io/MemoryBuffer.h"
#include "nit/data/DataLoader.h"
#include "nit/data/DataChannel.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class Database::Initializer
{
public:
	Initializer()
	{
		int err = sqlite3_initialize();

		if (err)
		{
			NIT_THROW_FMT(EX_DATABASE, "can't initialize sqlite3: %d\n", err);
		}
	}

	~Initializer()
	{
		sqlite3_shutdown();
	}
};

void Database::initialize()
{
	static Initializer s_Init;
}

////////////////////////////////////////////////////////////////////////////////

class BlobReader : public StreamReader
{
public:
	BlobReader(BlobSource* source, sqlite3_blob* blob);

public:									// StreamReader impl
	virtual StreamSource*				getSource()								{ return _source; }
	
	virtual bool						isBuffered()							{ return false; }
	virtual bool						isSized()								{ return true; }
	virtual bool						isSeekable()							{ return true; }

	virtual bool						isEof()									{ return _pos >= _size; }
	virtual size_t						getSize()								{ return _size; }
	virtual void						skip(int count)							{ _pos = Math::clamp(_pos + count, size_t(0), _size); }
	virtual void						seek(size_t pos)						{ _pos = Math::clamp(pos, (size_t)0, _size); }
	virtual size_t						tell()									{ return _pos; }

	virtual size_t						readRaw(void* buf, size_t size);

protected:
	Ref<BlobSource>						_source;
	sqlite3_blob*						_blob;

	size_t								_size;
	size_t								_pos;

	virtual void						OnDelete();
};

////////////////////////////////////////////////////////////////////////////////

class BlobWriter : public StreamWriter
{
public:
	BlobWriter(BlobSource* source, sqlite3_blob* blob);

public:									// StreamWriter impl
	virtual StreamSource*				getSource()								{ return _source; }

	virtual bool						isBuffered()							{ return false; }
	virtual bool						isSized()								{ return true; }
	virtual bool						isSeekable()							{ return true; }

	virtual size_t						getSize()								{ return _size; }
	virtual void						skip(int count)							{ _pos = Math::clamp(_pos + count, size_t(0), _size); }
	virtual void						seek(size_t pos)						{ _pos = Math::clamp(pos, size_t(0), _size); }
	virtual size_t						tell()									{ return _pos; }

	virtual size_t						writeRaw(const void* buf, size_t size);
	virtual bool						flush()									{ return true; }							

protected:
	Ref<BlobSource>						_source;
	sqlite3_blob*						_blob;

	size_t								_size;
	size_t								_pos;

	virtual void						OnDelete();
};

////////////////////////////////////////////////////////////////////////////////

BlobReader::BlobReader(BlobSource* source, sqlite3_blob* blob)
: _source(source), _blob(blob)
{
	_pos = 0;
	_size = source->getStreamSize();
}

size_t BlobReader::readRaw(void* buf, size_t size)
{
	if (_pos + size >= _size)
		size = _size - _pos;

	if (size == 0) return 0;

	int err = sqlite3_blob_read(_blob, buf, size, _pos);
	if (err) return 0;

	_pos += size;
	return size;
}

void BlobReader::OnDelete()
{
	if (_blob)
	{
		sqlite3_blob_close(_blob);
		_blob = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////

BlobWriter::BlobWriter(BlobSource* source, sqlite3_blob* blob)
: _source(source), _blob(blob)
{
	_pos = 0;
	_size = source->getStreamSize();
}

size_t BlobWriter::writeRaw(const void* buf, size_t size)
{
	if (_pos + size >= _size)
		size = _size - _pos;

	if (size == 0) return 0;

	int err = sqlite3_blob_write(_blob, buf, size, _pos);
	if (err) return 0;

	_pos += size;
	return size;
}

void BlobWriter::OnDelete()
{
	if (_blob)
	{
		sqlite3_blob_close(_blob);
		_blob = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////

BlobLocator::BlobLocator(const String& name, Database* db, const String& tblName, const String& blobColumn, const String& idColumn, const String& dbName)
: StreamLocator(name)
{
	_database = db;
	
	if (dbName.empty())
	{
		_path = tblName + "/" + blobColumn + "." + idColumn;
	}
	else
	{
		_path = dbName + "." + tblName + "/" + blobColumn + "." + idColumn;
	}

	_databaseName = dbName;
	_tableName = tblName;
	_blobColumnName = blobColumn;
	_idColumnName = idColumn;
}

StreamSource* BlobLocator::locateLocal(const String& streamName)
{
	BlobSource* source = NULL;

	sqlite3_blob* blob = openBlob(streamName, true, false);

	if (blob)
	{
		source = new BlobSource(this, streamName, blob);
		sqlite3_blob_close(blob);
	}

	return source;
}

void BlobLocator::findLocal(const String& pattern, StreamSourceMap& varResults)
{
	String sqlLike = StringUtil::replaceAll(pattern, "*", "%");

	Ref<Database::Query> stmt =
		_database->prepare(
			StringUtil::format("select rowid, %s FROM %s WHERE %s like ?", _idColumnName.c_str(), _tableName.c_str(), _idColumnName.c_str()).c_str());

	stmt->reset();
	stmt->bind(1, sqlLike);

	while (stmt->step())
	{
		const char* db = stmt->getDatabaseName(0);
		const char* tbl = stmt->getTableName(0);
		const char* column = _blobColumnName.c_str();
		int64 rowid = stmt->getInt64(0);
		String streamName = stmt->getText(1);

		sqlite3_blob* blob = openBlob(db, tbl, column, rowid, true, false);
		if (blob)
		{
			varResults.insert(std::make_pair(streamName, new BlobSource(this, streamName, blob)));
			sqlite3_blob_close(blob);
		}
	}
}

Database::Query* BlobLocator::prepareSelect()
{
	if (_selectQuery == NULL)
	{
		if (_databaseName.empty())
			_selectQuery = _database->prepare(
				StringUtil::format("SELECT rowid FROM %s WHERE %s = ?", 
					_tableName.c_str(), 
					_idColumnName.c_str()
				).c_str());
		else
			_selectQuery = _database->prepare(
				StringUtil::format("SELECT rowid FROM %s.%s WHERE %s = ?", 
					_databaseName.c_str(), 
					_tableName.c_str(), 
					_idColumnName.c_str()
				).c_str());
	}

	return _selectQuery;
}

Database::Query* BlobLocator::prepareUpdate()
{
	if (_updateQuery == NULL)
	{
		if (_databaseName.empty())
			_updateQuery = _database->prepare(
				StringUtil::format("UPDATE %s SET %s = zeroblob(?) WHERE %s = ?", 
					_tableName.c_str(), 
					_blobColumnName.c_str(), 
					_idColumnName.c_str()
				).c_str());
		else
			_updateQuery = _database->prepare(
				StringUtil::format("UPDATE %s.%s SET %s = zeroblob(?) WHERE %s = ?", 
					_databaseName.c_str(), 
					_tableName.c_str(),
					_blobColumnName.c_str(), 
					_idColumnName.c_str()
				).c_str());
	}

	return _updateQuery;
}

StreamReader* BlobLocator::open(const String& streamName)
{
	sqlite3_blob* blob = openBlob(streamName, true, true);

	return new BlobReader(new BlobSource(this, streamName, blob), blob);
}

StreamWriter* BlobLocator::modify(const String& streamName)
{
	sqlite3_blob* blob = openBlob(streamName, false, true);

	return new BlobWriter(new BlobSource(this, streamName, blob), blob);
}

sqlite3_blob* BlobLocator::openBlob(const String& streamName, bool readonly, bool throwEx)
{
	Database::Query* query = prepareSelect();
	query->reset();
	query->bind(1, streamName);

	sqlite3_blob* blob = NULL;

	if (query->step())
	{
		const char* db = query->getDatabaseName(0);
		const char* tbl = query->getTableName(0);
		const char* column = _blobColumnName.c_str();
		int64 rowid = query->getInt64(0);

		blob = openBlob(db, tbl, column, rowid, readonly, throwEx);
	}

	query->reset();

	if (blob) return blob;

	if (throwEx)
	{
		NIT_THROW_FMT(EX_NOT_FOUND, "can't find blob '%s'", makeUrl(streamName).c_str());
	}

	return NULL;
}

sqlite3_blob* BlobLocator::openBlob(const char* dbName, const char* tblName, const char* colName, sqlite3_int64 rowID, bool readonly, bool throwEx)
{
	sqlite3* db = _database->getPeer();
	sqlite3_blob* blob = NULL;
	int err = sqlite3_blob_open(db, dbName, tblName, colName, rowID, readonly ? 0 : 1, &blob);
	if (err && throwEx)
	{
		String msg = sqlite3_errmsg(db);
		NIT_THROW_FMT(EX_DATABASE, "can't open blob '%s.%s.%s[%lld]': %s", dbName, tblName, colName, rowID, msg.c_str());
	}

	return blob;
}

StreamWriter* BlobLocator::update(const String& streamName, size_t newSize)
{
	Database::Query* query = prepareUpdate();
	query->reset();
	query->bind(1, (int64)newSize);
	query->bind(2, streamName);
	query->step();
	query->reset();

	return modify(streamName);
}

String BlobLocator::makeUrl(const String& sourceName)
{
	return _database->getUriPath() + "/" + _path + "/" + sourceName;
}

////////////////////////////////////////////////////////////////////////////////

BlobSource::BlobSource(Ref<BlobLocator> locator, const String& name, sqlite3_blob* blob)
: StreamSource(locator, name)
{
	_size = sqlite3_blob_bytes(blob);
}

StreamReader* BlobSource::open()
{
	BlobLocator* loc = dynamic_cast<BlobLocator*>(getRealLocator());
	if (loc == NULL)
		NIT_THROW(EX_CASTING);

	return new BlobReader(this, loc->openBlob(_name, true, true));
}

StreamWriter* BlobSource::modify()
{
	BlobLocator* loc = dynamic_cast<BlobLocator*>(getRealLocator());
	if (loc == NULL)
		NIT_THROW(EX_CASTING);

	return new BlobWriter(this, loc->openBlob(_name, false, true));
}

StreamWriter* BlobSource::update(size_t newSize)
{
	BlobLocator* loc = dynamic_cast<BlobLocator*>(getRealLocator());
	if (loc == NULL)
		NIT_THROW(EX_CASTING);

	return loc->update(_name, newSize);
}

Timestamp BlobSource::getTimestamp()
{
	return Timestamp::ZERO();
}

Database* BlobSource::getDatabase()
{
	return dynamic_cast<BlobLocator*>(getRealLocator())->getDatabase();
}

////////////////////////////////////////////////////////////////////////////////

Database* Database::open(const String& uri)
{
	initialize();

	sqlite3* db = NULL;
	int flags = SQLITE_OPEN_URI | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
	int err = sqlite3_open_v2(uri.c_str(), &db, flags, NULL);

	if (err)
	{
		if (db)
		{
			String errmsg = sqlite3_errmsg(db);
			sqlite3_close(db);
			NIT_THROW_FMT(EX_DATABASE, "can't open '%s': %s", uri.c_str(), errmsg.c_str());
		}
		else
			NIT_THROW_FMT(EX_DATABASE, "can't open '%s': memory alloc fail", uri.c_str());
	}

	return new Database(uri, db);
}

Database* Database::open(FileLocator* fl, const String& filename)
{
	String extname = filename;
	if (StreamSource::getExtension(filename).empty())
		extname += ".sqlite";

	Ref<StreamSource> src;

	src = fl->locate(extname, false);

	if (src)
		return open(src->getUrl());
	else
		return open(fl->makeUrl(extname).c_str());
}

Database::Database(const String& uriPath, sqlite3* db)
: _db(db), _uriPath(uriPath)
{
}

Database::Query* Database::prepare(const char* sql)
{
	return new Query(this, prepareStmt(sql));
}

sqlite3_stmt* Database::prepareStmt(const char* sql)
{
	if (_db == NULL)
	{
		NIT_THROW_FMT(EX_DATABASE, "can't prepare '%s': db closed");
	}

	sqlite3_stmt* sq3stmt = NULL;

	const char* tail = NULL;
	int err = sqlite3_prepare_v2(_db, sql, -1, &sq3stmt, &tail);
	if (err)
	{
		NIT_THROW_FMT(EX_DATABASE, "can't prepare '%s': %s", sql, sqlite3_errmsg(_db));
		return NULL;
	}

	if (strlen(tail) != 0)
	{
		sqlite3_finalize(sq3stmt);
		NIT_THROW_FMT(EX_NOT_SUPPORTED, "can't prepare '%s': multiple statments", sql);
		return NULL;
	}

	return sq3stmt;
}

void Database::exec(const char* sql)
{
	if (_db == NULL)
	{
		NIT_THROW_FMT(EX_DATABASE, "can't exec '%s': db closed");
	}

	char* errmsg = NULL;
	int err = sqlite3_exec(_db, sql, NULL, NULL, &errmsg);
	if (err)
	{
		String msg = errmsg;
		sqlite3_free(errmsg);
		NIT_THROW_FMT(EX_DATABASE, "can't exec '%s': %s", sql, msg.c_str());
	}
}

void Database::exec(const char* sql, ExecCallback callback, void* context)
{
	if (_db == NULL)
	{
		NIT_THROW_FMT(EX_DATABASE, "can't exec '%s': db closed");
	}

	char* errmsg = NULL;
	int err = sqlite3_exec(_db, sql, callback, context, &errmsg);
	if (err)
	{
		String msg = errmsg;
		sqlite3_free(errmsg);
		NIT_THROW_FMT(EX_DATABASE, "can't exec '%s': %s", sql, msg.c_str());
	}
}

void Database::close()
{
	if (_db)
		sqlite3_close(_db);

	_db = NULL;
}

BlobLocator* Database::newLocator(const String& name, const String& path)
{
	if (path.empty())
		NIT_THROW(EX_INVALID_PARAMS);

	size_t pos;

	pos = path.find('/');

	bool ok = true;

	if (pos == path.npos)
		ok = false;

	String dbName;
	String tblName;
	String blobColumn;
	String idColumn = "rowid";

	if (ok)
	{
		tblName = path.substr(0, pos);
		blobColumn = path.substr(pos+1);

		pos = tblName.find('.');

		if (pos != tblName.npos)
		{
			dbName = tblName.substr(0, pos);
			tblName = tblName.substr(pos + 1);
		}

		pos = blobColumn.find('.');

		if (pos != blobColumn.npos)
		{
			idColumn = blobColumn.substr(pos + 1);
			blobColumn = blobColumn.substr(0, pos);
		}
	}

	ok = ok && !tblName.empty();
	ok = ok && !blobColumn.empty();
	ok = ok && !idColumn.empty();

	if (!ok)
		NIT_THROW_FMT(EX_INVALID_PARAMS, "invalid db locator path: '%s: %s'", name.c_str(), path.c_str());

	return new BlobLocator(name, this, tblName, blobColumn, idColumn, dbName);
}

////////////////////////////////////////////////////////////////////////////////

Database::Query::Query(Database* db, sqlite3_stmt* stmt)
: _db(db), _stmt(stmt), _stepResult(SQLITE_DONE)
{

}

void Database::Query::finalize()
{
	if (_stmt)
		sqlite3_finalize(_stmt);

	_stmt = NULL;
	_db = NULL;
}

void Database::Query::setSql(const char* sql)
{
	sqlite3_stmt* newStmt = _db->prepareStmt(sql);

	if (_stmt)
		sqlite3_finalize(_stmt);

	_stmt = newStmt;
	_stepResult = SQLITE_DONE;
}

void Database::Query::reset()
{
	sqlite3* db = getSqlite3();
	if (db == NULL) return;

	int err;
	err = sqlite3_clear_bindings(_stmt);
	if (err)
		NIT_THROW_FMT(EX_DATABASE, "can't clear binding: %s", sqlite3_errmsg(db));

	err = sqlite3_reset(_stmt);
	if (err)
		NIT_THROW_FMT(EX_DATABASE, "can't reset statement: %s", sqlite3_errmsg(db));

	_stepResult = SQLITE_DONE;
}

void Database::Query::clearBindings()
{
	sqlite3* db = getSqlite3();
	if (db == NULL) return;

	int err;
	err = sqlite3_clear_bindings(_stmt);
	if (err)
		NIT_THROW_FMT(EX_DATABASE, "can't clear binding: %s", sqlite3_errmsg(db));
}

bool Database::Query::step()
{
	if (_stmt == NULL)
		NIT_THROW_FMT(EX_DATABASE, "can't step: finalized");

	_stepResult = sqlite3_step(_stmt);

	if (_stepResult != SQLITE_DONE && _stepResult != SQLITE_ROW)
		NIT_THROW_FMT(EX_DATABASE, "can't step statement: %s", sqlite3_errmsg(getSqlite3()));

	if (_stepResult == SQLITE_DONE)
		reset();

	return _stepResult == SQLITE_ROW;
}

int Database::Query::exec()
{
	sqlite3* db = getSqlite3();
	if (db == NULL)
		NIT_THROW_FMT(EX_INVALID_STATE, "can't exec : database closed");

	int pre = sqlite3_total_changes(db);
	while (step())
	{
	}

	return sqlite3_total_changes(db) - pre;;
}

int Database::Query::getNumParams()
{
	return _stmt ? sqlite3_bind_parameter_count(_stmt) : 0;
}

int Database::Query::getParamIndex(const char* paramName)
{
	return _stmt ? sqlite3_bind_parameter_index(_stmt, paramName) : 0;
}

const char* Database::Query::getParamName(int paramIndex, bool goodName)
{
	if (_stmt == NULL) return NULL;

	const char* paramName = sqlite3_bind_parameter_name(_stmt, paramIndex);
	if (!goodName) return paramName;

	switch (paramName[0])
	{
	case '@' :	return paramName + 1;
	case '$' :	return paramName + 1;
	case ':' :	return paramName + 1;
	default:	return paramName;
	}
}

void Database::Query::bindError(int paramIndex)
{
	if (_stmt == NULL)
		NIT_THROW_FMT(EX_INVALID_STATE, "can't bind : statement finalized");

	sqlite3* db = getSqlite3();
	if (db == NULL)
		NIT_THROW_FMT(EX_INVALID_STATE, "can't bind : database closed");

	// If any previous step() we should reset().
	if (_stepResult == SQLITE_ROW)
		sqlite3_reset(_stmt);

	const char* paramName = sqlite3_bind_parameter_name(_stmt, paramIndex);
	if (paramName)
	{
		NIT_THROW_FMT(EX_DATABASE, "can't bind '%s': %s", paramName, sqlite3_errmsg(db));
	}
	else
	{
		NIT_THROW_FMT(EX_DATABASE, "can't bind '?%d': %s", paramIndex, sqlite3_errmsg(db));
	}
}

void Database::Query::bindNull(int paramIndex)
{
	if (_stmt == NULL || sqlite3_bind_null(_stmt, paramIndex))
		bindError(paramIndex);
}

void Database::Query::bind(int paramIndex, int value)
{
	if (_stmt == NULL || sqlite3_bind_int(_stmt, paramIndex, value))
		bindError(paramIndex);
}

void Database::Query::bind(int paramIndex, int64 value)
{
	if (_stmt == NULL || sqlite3_bind_int64(_stmt, paramIndex, value))
		bindError(paramIndex);
}

void Database::Query::bind(int paramIndex, double value)
{
	if (_stmt == NULL || sqlite3_bind_double(_stmt, paramIndex, value))
		bindError(paramIndex);
}

void Database::Query::bind(int paramIndex, const char* str, int strLen /*= -1*/)
{
	if (_stmt == NULL || sqlite3_bind_text(_stmt, paramIndex, str, strLen, SQLITE_TRANSIENT))
		bindError(paramIndex);
}

void Database::Query::bind(int paramIndex, const String& str)
{
	if (_stmt == NULL || sqlite3_bind_text(_stmt, paramIndex, str.c_str(), str.length(), SQLITE_TRANSIENT))
		bindError(paramIndex);
}

void Database::Query::bindBlob(int paramIndex, const void* blob, int numBytes)
{
	if (_stmt == NULL || sqlite3_bind_blob(_stmt, paramIndex, blob, numBytes, SQLITE_TRANSIENT))
		bindError(paramIndex);
}

typedef std::map<void*, MemoryAccess*> TempBlobLinks;
static TempBlobLinks s_TempBlobLinks;

static void DeleteTempBlob(void* mem)
{
	TempBlobLinks::iterator itr = s_TempBlobLinks.find(mem);
	if (itr != s_TempBlobLinks.end())
	{
		delete itr->second;
		s_TempBlobLinks.erase(itr);
	}
}

void Database::Query::bind(int paramIndex, Ref<MemoryBuffer> buf)
{
	if (_stmt == NULL)
		bindError(paramIndex);

	MemoryBuffer::Access* acc = new MemoryBuffer::Access(buf);
	s_TempBlobLinks.insert(std::make_pair(acc->getMemory(), acc));

	if (sqlite3_bind_blob(_stmt, paramIndex, acc->getMemory(), acc->getSize(), DeleteTempBlob))
	{
		// If any error, sqlite did called DeleteTempBlob appropriately.
		bindError(paramIndex);
	}
}

void Database::Query::bindValue(int paramIndex, const DataValue& value)
{
	switch (value.getType())
	{
	case DataValue::TYPE_VOID:
	case DataValue::TYPE_NULL:			bindNull(paramIndex); break;

	case DataValue::TYPE_BOOL:			bind(paramIndex, DataValue(value).toInt()); break;
	case DataValue::TYPE_INT:			bind(paramIndex, value.getData<int>()); break;

	case DataValue::TYPE_INT64:			bind(paramIndex, value.getData<int64>()); break;

	case DataValue::TYPE_FLOAT:			bind(paramIndex, value.getData<float>()); break;
	case DataValue::TYPE_DOUBLE:		bind(paramIndex, value.getData<double>()); break;

	case DataValue::TYPE_STRING:		bind(paramIndex, value.getStringPtr(), value.getStringSize()); break;
	case DataValue::TYPE_BLOB:			bindBlob(paramIndex, value.getBlobPtr(), value.getBlobSize()); break;

	case DataValue::TYPE_TIMESTAMP:		bind(paramIndex, value.getData<Timestamp>().getUnixTime64()); break;

	// types which sqlite doesn't support natively are treated as blob
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
			DataValue copy(value); 
			size_t size; 
			const void* blob = copy.toBlob(&size); 
			bindBlob(paramIndex, blob, size); 
		} 
		break;

	case DataValue::TYPE_ARRAY:			bind(paramIndex, DataValue(value).toBuffer());
	case DataValue::TYPE_RECORD:		bind(paramIndex, DataValue(value).toBuffer());
	case DataValue::TYPE_OBJECT:		bind(paramIndex, DataValue(value).toBuffer());
	case DataValue::TYPE_BUFFER:		bind(paramIndex, value.getRef<MemoryBuffer>()); break;

	default:							NIT_THROW(EX_NOT_SUPPORTED);
	}
}

void Database::Query::bindZeroBlob(int paramIndex, int numBytes)
{
	if (_stmt == NULL || sqlite3_bind_zeroblob(_stmt, paramIndex, numBytes))
		bindError(paramIndex);
}

int Database::Query::getNumColumns()
{
	return _stmt ? sqlite3_column_count(_stmt) : 0;
}

bool Database::Query::isReadOnly()
{
	return _stmt ? sqlite3_stmt_readonly(_stmt) != 0 : true;
}

const char* Database::Query::getDatabaseName(int column)
{
	return _stmt ? sqlite3_column_database_name(_stmt, column) : "";
}

const char* Database::Query::getTableName(int column)
{
	return _stmt ? sqlite3_column_table_name(_stmt, column) : "";
}

const char* Database::Query::getOriginName(int column)
{
	return _stmt ? sqlite3_column_origin_name(_stmt, column) : "";
}

Database::Query::ColumnType Database::Query::getType(int column)
{
	return _stmt ? ColumnType(sqlite3_column_type(_stmt, column)) : TYPE_NULL;
}

const char* Database::Query::getColumnName(int column)
{
	return _stmt ? sqlite3_column_name(_stmt, column) : NULL;
}

int Database::Query::getInt(int column)
{
	return _stmt ? sqlite3_column_int(_stmt, column) : 0;
}

int64 Database::Query::getInt64(int column)
{
	return _stmt ? sqlite3_column_int64(_stmt, column) : (int64)0L;
}

double Database::Query::getDouble(int column)
{
	return _stmt ? sqlite3_column_double(_stmt, column) : 0.0;
}

DataValue Database::Query::getValue(int column)
{
	if (_stmt == NULL) return DataValue::Void();

	switch (sqlite3_column_type(_stmt, column))
	{
	case SQLITE_INTEGER:				return sqlite3_column_int64(_stmt, column);

	case SQLITE_FLOAT:					return sqlite3_column_double(_stmt, column);

	case SQLITE_NULL:					return DataValue::Null();

	case SQLITE_BLOB:
		{ 
			const void* blob = sqlite3_column_blob(_stmt, column);
			size_t size = sqlite3_column_bytes(_stmt, column);

			if (size < sizeof(uint32) + sizeof(uint8))
				return DataValue(blob, size);

			// check DATA or ZDATA signature and try to convert to (or load) a DataValue
			uint32 signature = *(uint32*)blob;
			if (signature == NIT_DATA_SIGNATURE || signature == NIT_ZDATA_SIGNATURE)
			{
				DataValue blobValue(blob, size);

				try
				{
					uint8 type = ((uint8*)blob)[4];
					blobValue.convertTo((DataValue::Type)type);
				}
				catch (...)
				{
					// Conversion or load fail: return as just a plain blob
				}

				return blobValue;
			}

			// If signature doesn't tell it's a DataValue, return a plain blob
			return DataValue(blob, size);
		}

	case SQLITE3_TEXT:
		{
			const char* text = (const char*)sqlite3_column_text(_stmt, column);
			int len = sqlite3_column_bytes(_stmt, column);
			return DataValue(text, len);
		}

	default:							NIT_THROW(EX_NOT_SUPPORTED);
	}
}

const char* Database::Query::getText(int column, int* outNumBytes)
{
	if (_stmt == NULL) return NULL;

	const char* text = (const char*)sqlite3_column_text(_stmt, column);
	if (outNumBytes)
		*outNumBytes = sqlite3_column_bytes(_stmt, column);
	return text;
}

const char* Database::Query::getText(int column, String& outText)
{
	if (_stmt == NULL) return NULL;

	const char* text = (const char*)sqlite3_column_text(_stmt, column);
	outText.assign(text, sqlite3_column_bytes(_stmt, column));
	return outText.c_str();
}

const void* Database::Query::getBlob(int column, int* outNumBytes)
{
	if (_stmt == NULL) return NULL;

	const void* blob = sqlite3_column_blob(_stmt, column);
	if (outNumBytes)
		*outNumBytes = sqlite3_column_bytes(_stmt, column);
	return blob;
}

const char* Database::Query::getSql()
{
	return _stmt ? sqlite3_sql(_stmt) : "<finalized>";
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
