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

#include "nitbundler/nitbundler.h"

#include "nit/app/PackArchive.h"

NS_BUNDLER_BEGIN;

////////////////////////////////////////////////////////////////////////////////

// The class encapsulates a single pack file generation
class NITBUNDLER_API Packer : public MTRefCounted
{
public:
	Packer(PackSource* source, const String& filename = StringUtil::BLANK());
	Packer(const String& bundleName, Builder* builder);
	Packer(const String& packName, const String& filename, uint32 platformCode, bool bigEndian, FileLocator* dumpPath = NULL);

public:
	const String&						getName()								{ return _name; }
	const String&						getFilename()							{ return _filename; }

	FileLocator*						getDumpPath()							{ return _dumpPath; }
	bool								isBigEndian()							{ return _bigEndian; }

public:
	class FileEntry;

	Handler*							assign(const String& filename, StreamSource* source);
	Handler*							assign(const String& filename, StreamSource* source, Handler* handler);

public:
	class Job;
	class EntryJob;

	Job*								newJob(const String& fileFilter = "*", bool rebuild = false, Ref<StreamWriter> packWriter = NULL);

protected:
	virtual void						onDelete();

	String								_name;
	String								_filename;
	uint32								_platformCode;
	bool								_bigEndian;

	typedef unordered_map<String, FileEntry>::type FileEntries;
	typedef vector<FileEntry*>::type EntryList;

	FileEntries							_fileEntries;
	EntryList							_pendingEntries;

	Ref<Builder>						_builder;

	Ref<FileLocator>					_dumpPath;

	bool								_preparing;
	bool								_building;
	bool								_built;
	bool								_deploying;
	bool								_deployed;

	Ref<StreamWriter>					_targetWriter;

	void								init(Builder* builder);

	void								prepare();
	bool								checkNeedBuild(const String& packfile);
};

////////////////////////////////////////////////////////////////////////////////

class NITBUNDLER_API Packer::FileEntry : public WeakSupported
{
public:
	const String&						getFilename()							{ return _filename; }
	Packer*								getPacker()								{ return _packer; }
	StreamSource*						getSource()								{ return _source; }
	Handler*							getHandler()							{ return _handler; }
	StreamWriter*						getWriter()								{ return _writer; }

	PackArchive::FileEntry*				getData()								{ return &_data; }

	bool								isPrepared()							{ return _prepared; }
	bool								isDeletedMark()							{ return _deletedMark; }

private:
	friend class Packer;

	FileEntry()																	{ /* No op - Init() mandatory! */ }

	void								init(const String& filename, Packer* packer, StreamSource* source, Handler* handler);

	String								_filename;
	Weak<Packer>						_packer;
	Ref<StreamSource>					_source;
	Ref<Handler>						_handler;
	Ref<StreamWriter>					_writer;

	uint32								_entryOffset;

	PackArchive::FileEntry				_data;

	bool								_prepared;
	bool								_deletedMark;	// for 'DELETED' marked files
};

////////////////////////////////////////////////////////////////////////////////

NS_BUNDLER_END;
