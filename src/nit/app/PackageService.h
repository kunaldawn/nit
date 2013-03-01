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
#include "nit/data/Database.h"

#include "nit/app/Service.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

#define g_Package						static_cast<nit::PackageService*>(g_Service->getService(::nit::Service::SVC_PACKAGE))

////////////////////////////////////////////////////////////////////////////////

class Package;
class PackBundle;
class Archive;
class Settings;
class SessionEvent;

////////////////////////////////////////////////////////////////////////////////

class NIT_API PackageService : public Service
{
public:
	PackageService(Ref<PackBundle> topBundle);
	virtual ~PackageService();

public:
	typedef map<String, Ref<Package>, StringUtil::LessIgnoreCase>::type PackageNameMap;
	typedef map<String, String>::type UIDLookup;

public:
	Package*							link(const char* name, bool optional = false);
	Package*							linkCustom(const String& name, Archive* archive, Settings* settings);

	Package*							load(const char* name);
	Package*							loadAsync(const char* name, bool hurry=true);
	bool								isAsyncLoading();
	bool								isAsyncLoadingEnabled()					{ return _asyncLoader != NULL; }

	void								find(const String& pattern, vector<Package*>::type& varResults);

public:
	const PackageNameMap&				allLinked()								{ return _linkedPackages; }
	void								compact();

	EventChannel*						channel()								{ return _channel ? _channel : _channel = new EventChannel(); }

public:
	PackBundle*							getBundle()								{ return _bundle; }

public:
	Package*							getProcessing();

public:
	struct NIT_API LookupResult
	{
		String							subType;
		String							pack;
		String							entry;
	};

	bool								lookup(const String& type, const String& key, LookupResult& outResult);
	Database*							getLookupDB();

	static void							splitLookupEntry(String& varType, String& varKey, String& varSubType, String& varEntry);

	static void							filterPackCfgs(Ref<Settings> appCfg, StreamSourceMap& varPackCfgs);
	Ref<Database>						buildLookupDB(const String& path, StreamSourceMap& packCfgs);

public:									// Module implementation
	virtual void						onRegister();
	virtual void						onUnregister();

	virtual void						onInit();
	virtual void						onFinish();

private:
	class								ProcessScope;
	friend class						ProcessScope;
	vector<Package*>::type				_processing;

	void								onPackageProcessBegin(Package* pack);
	void								onPackageProcessEnd(Package* pack);

private:
	void								onClock(const Event* evt);
	void								onSessionStart(const SessionEvent* evt);
	void								onSessionStop(const SessionEvent* evt);
	void								onSessionChange(const SessionEvent* evt);

	void								updatePreloads(float timeLimit = 0.1f);
	void								queuePreload(Package* pack);
	void								joinAsyncLoader(Package* pack);
private:
	class								AsyncLoader;
	friend class						AsyncLoader;

	AsyncLoader*						_asyncLoader;

private:
	friend class						AppBase;
	friend class						Package;

	Ref<PackBundle>						_bundle;

	PackageNameMap						_linkedPackages;

	Ref<Database>						_lookupDB;
	Ref<Database::Query>				_lookupSelect;

	Ref<EventChannel>					_channel;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
