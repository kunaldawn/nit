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

#include "nit/io/Archive.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NIT_API FileUtil
{
public:
	static bool							hasFileSystem();
	static bool							isReadOnly();
	static bool							isCaseSensitive();

public:
	static void							normalizeSeparator(String& varPath);
	static bool							isReservedName(const String& path);
	static bool							isAbsolutePath(const String& path);

	static char							getPathSeparator()						{ return s_PathSeparator; }
	static char							getPathAntiSeparator()					{ return s_PathAntiSeparator; }
	static String						getDamnedPattern1()						{ return s_DamnedPattern1; }

public:
	static bool							exists(const String& path);
	static bool							isDirectory(const String& path);

public:
	static void							createDir(const String& path, bool throwIfExists = false);

	static void							writeFile(const String& path, const void* buf, size_t len);
	static void							writeFile(const String& path, MemoryBuffer* buffer);
	static void							writeFile(const String& path, const String& text);
	static void							writeFile(const String& path, StreamReader* from, size_t bufSize = 4096);

	static void							readFile(const String& path, MemoryBuffer* into);
	static void							readFile(const String& path, String& outText);
	static void							readFile(const String& path, void* buf, size_t len);
	static void							readFile(const String& path, StreamWriter* into, size_t bufSize = 4096);

	static void							remove(const String& pattern);
	static void							move(const String& from, const String& to);
	static void							copy(const String& from, const String& to);

private:
	static char							s_PathSeparator;
	static char							s_PathAntiSeparator;
	static const char*					s_DamnedPattern1;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API FileLocator : public Archive
{
public:
	FileLocator(const String& name, const String& path, bool readOnly = true, bool findRecursive = false);

public:
	const String&						getBaseUrl()							{ return this ? _baseUrl : StringUtil::BLANK(); }
	virtual String						makeUrl(const String& sourceName);

public:									// StreamLocator Implementation
	virtual bool						isCaseSensitive()						{ return FileUtil::isCaseSensitive(); }

	virtual StreamSource*				locateLocal(const String& streamName);
	virtual void						findLocal(const String& pattern, StreamSourceMap& varResults);

public:									// Archive Implementation
	virtual void						load();
	virtual void						unload();

	virtual bool						isReadOnly()							{ return _readOnly; }

	virtual StreamWriter*				create(const String& streamName);
	virtual StreamWriter*				modify(const String& streamName);
	virtual void						remove(const String& pattern);
	virtual void						rename(const String& streamName, const String& newName);

public:
	void								findFiles(const String& pattern, StreamSourceMap& varResults, bool recursive = false);
	void								findDirs(const String& pattern, StringVector& varResults);

	String								normalizePath(const String& path);
	String								extractFirstDirectory(const String& path, size_t* splitPos = NULL);

public:
	bool								isFilteredOnly()						{ return _filteredOnly; }
	void								setFilteredOnly(bool flag)				{ _filteredOnly = flag; }

	void								addFiltered(const String& pattern, bool recursive = false);
	void								removeFiltered(const String& pattern);

protected:
	typedef map<String, RefCache<StreamSource>, StringUtil::LessIgnoreCase>::type FilteredSources;
	FilteredSources						_filtered;
	String								_baseUrl;

	bool								_readOnly : 1;
	bool								_filteredOnly : 1;
	bool								_findRecursive : 1;

	StreamSource*						locateFiltered(const String& streamName);
	void								findFiltered(const String& pattern, StreamSourceMap& varResults);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API File : public StreamSource
{
protected:
	File(StreamLocator* locator, const String& name);

public:
	virtual size_t						getStreamSize()							{ return _streamSize; }
	virtual size_t						getMemorySize()							{ return _streamSize; }
	virtual Timestamp					getTimestamp()							{ return _timestamp; }

	virtual StreamReader*				open();

public:
	virtual StreamReader*				openRange(size_t offset, size_t size, StreamSource* source = NULL);

protected:
	size_t								_streamSize;
	Timestamp							_timestamp;

	friend class						FileLocator;

private:
	NIT_FILE_HANDLE						openHandle(size_t* outOffset = NULL, size_t* outLen = NULL);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API FileReader : public StreamReader
{
public:
	FileReader(StreamSource* source, NIT_FILE_HANDLE fileHandle, size_t offset = 0, size_t* inSize = NULL);

public:
	virtual StreamSource*				getSource()								{ return _source; }

public:
	virtual bool						isBuffered()							{ return false; }
	virtual bool						isSized()								{ return _size != (size_t)-1; }
	virtual bool						isSeekable()							{ return true; }

public:
	virtual bool						isEof()									{ return _pos >= _size; }
	virtual size_t						getSize()								{ return _size; }
	virtual void						skip(int count);
	virtual void						seek(size_t pos);
	virtual size_t						tell()									{ return _pos; }

	virtual size_t						readRaw(void* buf, size_t size);

public:
	NIT_FILE_HANDLE						releaseHandle();
	void								close();

protected:
	Ref<StreamSource>					_source;
	NIT_FILE_HANDLE						_file;
	size_t								_offset;
	size_t								_size;
	size_t								_pos;

	virtual void						onDelete();
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API FileWriter : public StreamWriter
{
public:
	FileWriter(StreamSource* source, NIT_FILE_HANDLE fileHandle, size_t offset = 0);

public:
	virtual StreamSource*				getSource()								{ return _source; }

public:
	virtual bool						isBuffered()							{ return false; }
	virtual bool						isSized()								{ return true; }
	virtual bool						isSeekable()							{ return true; }

public:
	virtual size_t						getSize();
	virtual void						skip(int count);
	virtual void						seek(size_t pos);
	virtual size_t						tell();

	virtual size_t						writeRaw(const void* buf, size_t size);
	virtual bool						flush();

public:
	NIT_FILE_HANDLE						releaseHandle();
	void								close();

protected:
	Ref<StreamSource>					_source;
	NIT_FILE_HANDLE						_file;
	size_t								_offset;

	virtual void						onDelete();
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
