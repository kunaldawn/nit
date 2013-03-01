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

#include "nit/app/PackageService.h"
#include "nit/app/PackBundle.h"

#include "nit/io/Archive.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class PackageService;
class PackBundle;
class Package;
class Content;
class ContentSource;
class Database;

typedef vector<Ref<Package> >::type PackageList;

////////////////////////////////////////////////////////////////////////////////

class NIT_API Package : public Archive
{
public:
	enum Type
	{
		PK_GOLFPACK,
		PK_PLUGIN,
		PK_FOLDER,
		PK_ALIAS,
		PK_CUSTOM,
		PK_DELETED,
	};

	enum PrepareOrder
	{
		RO_POSTLOAD,
		RO_SCRIPT,
		RO_PROGRAM,
		RO_TEXTURE,
		RO_SKELETON,
		RO_MATERIAL,
		RO_FONT,
		RO_COMPOSITOR,
		RO_MESH,
		RO_OVERLAY,
		RO_PARTICLE,
	};

public:
	Type								getType()								{ return _type; }
	PackageService*						getService()							{ return _service; }
	Archive*							getArchive()							{ return _archive; }
	PackBundle*							getBundle()								{ return _bundle; }
	StreamSource*						getSettingsSource()						{ return _settingsSource; }

public:									// Status
	bool								isLinked()								{ return _linked; }
	bool								isLoading()								{ Mutex::ScopedLock lock(_mutex); return _loading; }
	bool								isPrepared()							{ Mutex::ScopedLock lock(_mutex); return _prepared; }
	bool								isLoaded()								{ return _loaded; }
	bool								isScriptReady()							{ return _scriptReady; }

public:									// Resident control
	bool								isStayResident()						{ return _stayResident; }
	bool								isStayForCurrent()						{ return _stayForCurrent; }
	bool								isStayForNext()							{ return _stayForNext; }

	void								setStayResident(bool flag)				{ _stayResident = flag; }
	void								setStayForCurrent(bool flag)			{ _stayForCurrent = flag; }
	void								setStayForNext(bool flag)				{ _stayForNext = flag; }

public:									// Content Support (we don't provide load() due to it may propagate inefficiency)
	Ref<Content>						link(const String& streamName, bool throwEx = true);
	Ref<Content>						link(const String& streamName, const String& locatorPattern, bool throwEx = true);

	ContentSource*						linkCustom(const String& streamName, Ref<Content> content, StreamSource* otherSource = NULL, bool throwEx = true);

public:									// Database support
	Database*							openDb(const String& streamName);

public:									// StreamLocator Implementation
	virtual StreamSource*				locate(const String& streamName, bool throwEx = true);
	virtual StreamSource*				locate(const String& streamName, const String& locatorPattern, bool throwEx = true);
	virtual void						find(const String& pattern, StreamSourceMap& varResults);
	virtual void						find(const String& streamPattern, const String& locatorPattern, StreamSourceMap& varResults);

	virtual StreamSource*				locateLocal(const String& streamName);
	virtual void						findLocal(const String& pattern, StreamSourceMap& varResults);
	virtual Ref<ContentBase>			linkContent(StreamSource* stream);

public:									// Archive Implemenatation
	virtual void						load();
	virtual void						unload();

	virtual bool						isReadOnly()							{ return _archive ? _archive->isReadOnly() : true; }
	virtual bool						isCaseSensitive()						{ return _archive ? _archive->isCaseSensitive() : false; }

	virtual StreamReader*				open(const String& streamName);
	virtual StreamWriter*				create(const String& streamName)		{ ASSERT_THROW(_archive, EX_NOT_SUPPORTED); return _archive->create(streamName); }
	virtual StreamWriter*				modify(const String& streamName)		{ ASSERT_THROW(_archive, EX_NOT_SUPPORTED); return _archive->modify(streamName); }
	virtual void						remove(const String& pattern)			{ ASSERT_THROW(_archive, EX_NOT_SUPPORTED); return _archive->remove(pattern); }
	virtual void						rename(const String& streamName, const String& newName)	{ ASSERT_THROW(_archive, EX_NOT_SUPPORTED); return _archive->rename(streamName, newName); }

public:									// Async Support
	void								loadAsync(bool hurry=false);

public:
	virtual void						require(StreamLocator* req, bool first = false);

public:
	EventChannel*						channel()								{ return _channel ? _channel : _channel = new EventChannel(); }

protected:
	friend class PackBundle;
	friend class PackageService;

	Package(const String& name, Archive* archive, Package* base = NULL);
	virtual ~Package();

protected:
	void								checkLinked();
	void								checkLoaded();
	void								checkPrepared();

	bool								link(Settings* ss);
	void								unlink();
	void								destroy()								{ delete this; }

private:
	bool								initScripts();
	bool								finishScripts();

private:
	PackageService*						_service;
	Ref<Archive>						_archive;
	Ref<Package>						_base;
	Weak<PackBundle>					_bundle;
	Ref<StreamSource>					_settingsSource;

private:
	Type								_type;

	StringVector						_requiredScripts;
	StringVector						_postLoads;

	StringVector						_onLoadScripts;
	StringVector						_onUnloadScripts;

	StringVector						_dataSchemas;
	StringVector						_scriptSchemas;

	StringVector						_scriptSpawns; // TODO: remove

	Ref<EventChannel>					_channel;

	Mutex								_mutex;

	bool								_linking : 1;
	bool								_linked : 1;
	bool								_loading : 1;	
	bool								_prepared : 1;
	bool								_hurry : 1;
	bool								_scriptOpening : 1;
	bool								_scriptReady : 1;
	bool								_loaded : 1;
	bool								_stayResident : 1;
	bool								_stayForCurrent : 1;
	bool								_stayForNext : 1;
	bool								_allowSubDir : 1;

private:
	typedef map<String, RefCache<ContentSource>, StringUtil::LessIgnoreCase>::type ContentSources;
	ContentSources						_localContents;

	friend class ContentSource;
	Ref<Content>						linkLocal(const String& streamName);
	Content*							locateLocalContent(const String& streamName);
	Content*							locateContent(const String& streamName, const String& locatorPattern);
	Ref<Content>						createLocalContent(StreamSource* stream, bool throwEx);

private:
	typedef multimap<PrepareOrder, Ref<StreamSource> >::type PreloadFiles;

	bool								isHurry() { Mutex::ScopedLock lock(_mutex); return _hurry; }

	static PrepareOrder					getPrepareOrder(StreamSource* source);

	void								prepare(bool async);

	void								afterPrepared();
	void								collectPrepareFiles(PreloadFiles& varFiles, bool async);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API PackageEvent : public Event
{
public:
	PackageEvent() { }
	PackageEvent(nit::Package* pack) : Package(pack) { }
	Ref<nit::Package> Package;
};

NIT_EVENT_DECLARE(NIT_API, OnPackageLinked, PackageEvent);
NIT_EVENT_DECLARE(NIT_API, OnPackageLoaded, PackageEvent);
NIT_EVENT_DECLARE(NIT_API, OnPackageUnloaded, PackageEvent);
NIT_EVENT_DECLARE(NIT_API, OnPackageUnlinked, PackageEvent);

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
