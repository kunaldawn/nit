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
#include "nit/io/Stream.h"
#include "nit/data/DataValue.h"

#define SQLITE_API NIT_API
#include "sqlite3/sqlite3.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class BlobLocator;
class BlobSource;
class FileLocator;

////////////////////////////////////////////////////////////////////////////////

class NIT_API Database : public RefCounted
{
public:
	virtual ~Database()															{ assert(_db == NULL); }

public:
	static void							initialize();

	static Database*					open(const String& uriPath);
	static Database*					open(FileLocator* locator, const String& filename);

	// TODO: memory usage, release memory
	// TODO: query progress callback -> Event
	// TODO: StartBackup(Database* from);

public:
	sqlite3*							getPeer()								{ return _db; }
	const String&						getUriPath()							{ return _uriPath; }

	class Query;
	Query*								prepare(const char* sql);
	void								exec(const char* sql);

	typedef int							(*ExecCallback)(void*,int,char**,char**);
	void								exec(const char* sql, ExecCallback callback, void* context);

public:
	// path: '[db.]table/blob_column[.id_column]'
	BlobLocator*						newLocator(const String& name, const String& path); 

public:
	sqlite3_stmt*						prepareStmt(const char* sql);

protected:
	Database(const String& uriPath, sqlite3* db);
	void								close();
	virtual void						onDelete()								{ close(); }

private:
	class Initializer;

	sqlite3*							_db;

	String								_uriPath;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API Database::Query : public RefCounted
{
public:
	Query(Database* db, sqlite3_stmt* stmt);
	virtual ~Query()															{ assert(_stmt == NULL); }

public:
	void								reset();
	void								clearBindings();
	bool								step();									// true if you can call GetXXX()
	int									exec();

	bool								canGet()								{ return _stepResult == SQLITE_ROW; }
	int									getStepResult()							{ return _stepResult; }

	sqlite3_stmt*						getPeer()								{ return _stmt; }

public:									// parameter binding
	int									getNumParams();
	int									getParamIndex(const char* paramName);
	const char*							getParamName(int paramIndex, bool goodName = true);

	void								bindNull(const char* paramName)				{ return bindNull(getParamIndex(paramName)); }
	void								bind(const char* paramName, int value)		{ return bind(getParamIndex(paramName), value); }
	void								bind(const char* paramName, int64 value)	{ return bind(getParamIndex(paramName), value); }
	void								bind(const char* paramName, double value)	{ return bind(getParamIndex(paramName), value); }
	void								bind(const char* paramName, const char* str, int strLen = -1)	{ return bind(getParamIndex(paramName), str, strLen); }
	void								bind(const char* paramName, const String& str)					{ return bind(getParamIndex(paramName), str); }
	void								bindBlob(const char* paramName, const void* blob, int numBytes)	{ return bindBlob(getParamIndex(paramName), blob, numBytes); }
	void								bind(const char* paramName, Ref<MemoryBuffer> buf)				{ return bind(getParamIndex(paramName), buf); }
	void								bindValue(const char* paramName, const DataValue& value)		{ return bindValue(getParamIndex(paramName), value); }
	void								bindZeroBlob(const char* paramName, int numBytes)				{ return bindZeroBlob(getParamIndex(paramName), numBytes); }

	void								bindNull(int paramIndex);
	void								bind(int paramIndex, int value);
	void								bind(int paramIndex, int64 value);
	void								bind(int paramIndex, double value);
	void								bind(int paramIndex, const char* str, int strLen = -1);
	void								bind(int paramIndex, const String& str);
	void								bindBlob(int paramIndex, const void* blob, int numBytes);
	void								bind(int paramIndex, Ref<MemoryBuffer> buf);
	void								bindValue(int paramIndex, const DataValue& value);
	void								bindZeroBlob(int paramIndex, int numBytes);

public:									// meta data
	int									getNumColumns();
	bool								isReadOnly();

	const char*							getDatabaseName(int column);
	const char*							getTableName(int column);
	const char*							getOriginName(int column);

	const char*							getSql();
	void								setSql(const char* sql);

public:									// column access
	enum ColumnType
	{
		TYPE_INTEGER					= SQLITE_INTEGER,
		TYPE_FLOAT						= SQLITE_FLOAT,
		TYPE_BLOB						= SQLITE_BLOB,
		TYPE_NULL						= SQLITE_NULL,
		TYPE_TEXT						= SQLITE3_TEXT,
	};

	ColumnType							getType(int column);
	const char*							getColumnName(int column);

	int									getInt(int column);
	int64								getInt64(int column);
	double								getDouble(int column);
	DataValue							getValue(int column);

	const char*							getText(int column, int* outNumBytes = NULL);
	const char*							getText(int column, String& outText);
	const void*							getBlob(int column, int* outNumBytes = NULL);

//	void								getStatus(int* outNumSteps, int& outNumSorts, int& outNumAutoIndices);

public:
	Ref<Database>						_db;
	sqlite3_stmt*						_stmt;
	int									_stepResult;

protected:
	void								finalize();
	virtual void						onDelete()								{ finalize(); }

	sqlite3*							getSqlite3()							{ return _stmt && _db ? _db->getPeer() : NULL; }
	void								bindError(int paramIndex);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API BlobLocator : public StreamLocator
{
public:
	BlobLocator(const String& name, Database* db, const String& tblName, const String& blobColumn, const String& idColumn="rowid", const String& dbName="");

public:
	Database*							getDatabase()							{ return _database; }

	StreamReader*						open(const String& streamName);
	StreamWriter*						modify(const String& streamName);
	StreamWriter*						update(const String& streamName, size_t newSize);

	sqlite3_blob*						openBlob(const String& streamName, bool readonly, bool throwEx = true);
	sqlite3_blob*						openBlob(const char* dbName, const char* tblName, const char* colName, sqlite3_int64 rowID, bool readonly, bool throwEx = true);

protected:								// StreamLocator impl
	virtual bool						isCaseSensitive()						{ return false; }
	virtual StreamSource*				locateLocal(const String& streamName);
	virtual void						findLocal(const String& pattern, StreamSourceMap& varResults);
	virtual String						makeUrl(const String& sourceName);

private:
	Ref<Database>						_database;
	Ref<Database::Query>				_selectQuery;
	Ref<Database::Query>				_updateQuery;

	String								_path;
	String								_databaseName;
	String								_tableName;
	String								_idColumnName;
	String								_blobColumnName;

	Database::Query*					prepareSelect();
	Database::Query*					prepareUpdate();
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API BlobSource : public StreamSource
{
public:
	BlobSource(Ref<BlobLocator> locator, const String& name, sqlite3_blob* blob);

public:									// StreamSource Impl
	virtual size_t						getStreamSize()							{ return _size; }
	virtual size_t						getMemorySize()							{ return _size; }
	virtual Timestamp					getTimestamp();

public:
	Database*							getDatabase();

public:
	virtual StreamReader*				open();
	StreamWriter*						modify();
	StreamWriter*						update(size_t newSize);

private:
	size_t								_size;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
