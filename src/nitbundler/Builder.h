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

#include "nitbundler/PackSource.h"

NS_BUNDLER_BEGIN;

////////////////////////////////////////////////////////////////////////////////

// The class encapsulates whole building process for a bundle target
class NITBUNDLER_API Builder : public MTRefCounted
{
public:
	Builder(Settings* appCfg, Platform* platform, const String& buildTarget, String outPath);

public:
	Platform*							getPlatform()							{ return _platform; }

	const String&						getPlatformID()							{ return _platformID; }
	const String&						getBuildTarget()						{ return _buildTarget; }
	const String&						getRevision()							{ return _revision; }

	Settings*							getAppCfg()								{ return _appCfg; }
	Settings*							getAppCfgInclude()						{ return _appCfgInclude; }

	FileLocator*						getOutPath()							{ return _outPath; }
	FileLocator*						getDumpPath()							{ return _dumpPath; }

public:
	typedef map<String, Ref<PackSource> >::type PackSources;

	void								collect(const String& filter = "*");
	PackSources&						allPacks()								{ return _packs; }
	PackSource*							getPack(const String& packname);

public:
	class Job;
	class BundleJob;

	Job*								newJob(const String& packFilter = "*", const String& fileFilter = "*", bool makeBundle = true);

private:
	Ref<Platform>						_platform;

	Ref<StreamLocator>					_packLocator;

	Ref<Settings>						_appCfg;
	Ref<Settings>						_appCfgInclude;

	PackSources							_packs;

	String								_platformID;
	String								_buildTarget;
	String								_revision;
	String								_buildHost;

	Ref<FileLocator>					_outPath;
	Ref<FileLocator>					_dumpPath;

	String								_sectionName;
	String								_platformSectionName;

	StringVector						_packIgnoreFilters;

	friend class PackSource;

	void								collectPackLocators(Settings* section);
	Settings*							createBundleSection(const String& uid, const String& baseUID);
};

////////////////////////////////////////////////////////////////////////////////

NS_BUNDLER_END;
