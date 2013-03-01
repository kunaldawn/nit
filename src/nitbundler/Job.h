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

#include "nitbundler/Builder.h"
#include "nitbundler/Packer.h"

#include "nit/async/AsyncJob.h"

NS_BUNDLER_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class ZBundleBuildJob : public AsyncJob
{
public:
	ZBundleBuildJob(Ref<StreamSource> bundle, Ref<Settings> bundleCfg, bool bigEndian, Ref<Archive> outPath, Ref<MemoryBuffer> extData);

public:
	StreamSource*						getOutput()								{ return _output; }

protected:
	virtual bool						onPrepare();
	virtual bool						onExecute(bool async);
	virtual void						onFinish();

	Ref<StreamSource>					_bundle;
	Ref<Settings>						_bundleCfg;
	bool								_bigEndian;
	Ref<Archive>						_outPath;
	Ref<MemoryBuffer>					_extData;

	String								_bundleUID;
	String								_baseUID;

	Ref<MemoryBuffer>					_bundleCfgBuf;
	String								_bundleName;
	uint32								_sourceHash;

	Ref<StreamSource>					_output;
};

////////////////////////////////////////////////////////////////////////////////

class BundleDiffJob : public AsyncJob
{
public:
	BundleDiffJob(Ref<StreamSource> topZBundle, Ref<StreamSource> baseZBundle, Ref<Archive> outPath);

public:
	struct DiffEntry
	{
		enum DiffStatus
		{
			UNKNOWN,
			ADDED,
			MODIFIED,
			DELETED,
		};

		DiffEntry(DiffStatus status, PackArchive::File* file)
			: status(status), file(file)
		{
		}

		const char* StatusStr()
		{
			switch (status)
			{
			case ADDED:					return "a";
			case MODIFIED:				return "m";
			case DELETED:				return "d";
			default:					return "?";
			}
		}

		DiffStatus						status;
		Ref<PackArchive::File>			file;
	};

	typedef multimap<Ref<StreamLocator>, DiffEntry>::type DiffResult;

public:
	PackBundle::ZBundleInfo*			getTopInfo()								{ return _topInfo; }
	PackBundle*							getTopBundle()								{ return _topBundle; }
	Settings*							getTopBundleCfg()							{ return _topBundleCfg; }
	Settings*							getTopAppCfg()								{ return _topAppCfg; }

	PackBundle::ZBundleInfo*			getBaseInfo()								{ return _baseInfo; }
	PackBundle*							getBaseBundle()								{ return _baseBundle; }
	Settings*							getBaseBundleCfg()							{ return _baseBundleCfg; }
	Settings*							getBaseAppCfg()								{ return _baseAppCfg; }

	DiffResult&							getDiffResult()								{ return _result; }

protected:
	virtual bool						onPrepare();
	virtual bool						onExecute(bool async);
	virtual void						onFinish();

	Ref<StreamSource>					_topZBundle;
	Ref<StreamSource>					_baseZBundle;
	Ref<Archive>						_outPath;

	Ref<PackBundle::ZBundleInfo>		_topInfo;
	Ref<PackBundle>						_topBundle;
	Ref<Settings>						_topBundleCfg;
	Ref<Settings>						_topAppCfg;

	Ref<PackBundle::ZBundleInfo>		_baseInfo;
	Ref<PackBundle>						_baseBundle;
	Ref<Settings>						_baseBundleCfg;
	Ref<Settings>						_baseAppCfg;

	DiffResult							_result;

	static void							testDiff(Package* basePack, Package* topPack, DiffResult& result, bool checkStream);
	static void							testDiff(PackBundle* baseBundle, PackBundle* topBundle, DiffResult& result, bool checkStream);
};

////////////////////////////////////////////////////////////////////////////////

class DeltaBuildJob : public AsyncJob
{
public:
	DeltaBuildJob(Ref<StreamSource> topZBundle, Ref<StreamSource> baseZBundle, Ref<Archive> outPath);

public:
	StreamSource*						getOutput();

protected:
	virtual bool						onPrepare();
	virtual bool						onExecute(bool async);
	virtual void						onSubJobFinished(AsyncJob* subJob, Status status);
	virtual void						onFinish();

	Ref<StreamSource>					_topZBundle;
	Ref<StreamSource>					_baseZBundle;
	Ref<Archive>						_outPath;

	bool								_bigEndian;
	String								_newBundleName;
	Ref<bundler::Packer>				_bundlePacker;

	Ref<BundleDiffJob>					_bundleDiffJob;
	Ref<AsyncJob>						_bundlePackJob;
	Ref<ZBundleBuildJob>				_zBundleBuildJob;
};

////////////////////////////////////////////////////////////////////////////////

class Builder::Job : public AsyncJob
{
public:
	Job(Builder* builder, const String& packFilter, const String& fileFilter, bool makeBundle);

public:
	StreamSource*						getOutput();

protected:
	virtual bool						onPrepare();
	virtual bool						onExecute(bool async);
	virtual void						onFinish();
	virtual void						onSubJobFinished(AsyncJob* subJob, Status status);

	Ref<Builder>						_builder;
	String								_packFilter;
	String								_fileFilter;
	bool								_makeBundle;
	Ref<BundleJob>						_bundleJob;
};

////////////////////////////////////////////////////////////////////////////////

class Builder::BundleJob : public AsyncJob
{
public:
	BundleJob(Builder* builder, const String& packFilter);

public:
	StreamSource*						getOutput();

protected:
	virtual bool						onPrepare();
	virtual bool						onExecute(bool async);
	virtual void						onFinish();
	virtual void						onSubJobFinished(AsyncJob* subJob, Status status);

	Ref<Builder>						_builder;
	String								_packFilter;
	Ref<Settings>						_bundleCfg;
	Ref<Packer>							_packer;
	Ref<StreamSource>					_outBundle;

	Ref<Packer::Job>					_packJob;
	Ref<ZBundleBuildJob>				_zBundleBuildJob;
};

////////////////////////////////////////////////////////////////////////////////

class Packer::Job : public AsyncJob
{
public:
	Job(Packer* packer, const String& fileFilter, bool rebuild, Ref<StreamWriter> packWriter);

public:
	const String&						getPackName()							{ return _packer->getName(); }
	uint								getEntryCount()							{ return _entryCount; }
	uint								getWriteCount()							{ return _writeCount; }

	StreamSource*						getOutput()								{ return _output; }

protected:
	virtual bool						onPrepare();
	virtual bool						onExecute(bool async);
	virtual void						onSubJobFinished(AsyncJob* subJob, Status status);
	virtual void						onFinish();

	Ref<Packer>							_packer;
	Ref<StreamWriter>					_packWriter;
	String								_fileFilter;
	bool								_rebuild;
	EntryList							_entries;

	list<Ref<EntryJob> >::type			_writeQueue;

	uint								_entryCount;
	uint								_writeCount;

	Ref<StreamSource>					_output;

	void 								writeFileEntries();
	void 								writeDummyEntries();
	void 								writeHeader();
};

////////////////////////////////////////////////////////////////////////////////

class Packer::EntryJob : public AsyncJob
{
public:
	EntryJob(FileEntry* entry);

public:
	FileEntry*							getEntry()								{ return _entry; }
	MemoryBuffer*						getBuffer()								{ return _buffer; }

protected:
	virtual bool						onPrepare();
	virtual bool						onExecute(bool async);
	virtual void						onFinish();

	Ref<MemoryBuffer::Writer>			_writer;
	Ref<MemoryBuffer>					_buffer;

	FileEntry*							_entry;
};

////////////////////////////////////////////////////////////////////////////////

NS_BUNDLER_END;
