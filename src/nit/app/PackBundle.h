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
#include "nit/app/PackArchive.h"

#include "nit/io/Stream.h"
#include "nit/io/FileLocator.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

#define NIT_ZBUNDLE_SIGNATURE			NIT_MAKE_CC('Z', 'B', 'N', 'D')
#define NIT_ZBUNDLE_SIGNATURE_FLIP		NIT_MAKE_CC('D', 'N', 'B', 'Z')
#define NIT_ZBUNDLE_VERSION				NIT_MAKE_CC('1', '.', '0', '0')

////////////////////////////////////////////////////////////////////////////////

class PackBundle;
class AppConfig;

typedef map<String, Ref<PackBundle> >::type BundleUIDLookup;

class NIT_API PackBundle : public StreamLocator
{
public:
	struct NIT_API ZBundleSigHeader
	{
		uint32							signature;
		uint32							version;

		void							flipEndian();
	};

	struct NIT_API ZBundleHeader
	{
		uint16							payloadType;		//  2
		uint16							flags;				//  4
		uint16							uidLen;				//  6
		uint16							baseUidLen;			//  8
		uint32							bundleCfgSize;		// 12
		uint32							extDataSize;		// 16
		uint64							sourceSize;			// 24
		uint64							payloadSize;		// 32
		uint32							payloadCRC32;		// 36
		uint32							payloadParam0;		// 40
		uint32							payloadParam1;		// 44
		uint32							_reserved0;			// 48

		void							flipEndian();
	};

	class NIT_API ZBundleInfo : public RefCounted
	{
	public:
		const ZBundleHeader&			getHeader()								{ return _header; }
		const String&					getUid()								{ return _uid; }
		const String&					getBaseUid()							{ return _baseUid; }
		StreamSource*					getBundleCfg()							{ return _bundleCfg; }
		StreamSource*					getExtData()							{ return _extData; }

	private:
		friend class PackBundle;
		ZBundleHeader					_header;
		String							_uid;
		String							_baseUid;
		Ref<StreamSource>				_bundleCfg;
		Ref<StreamSource>				_extData;
	};

	static Ref<ZBundleInfo>				unpackZBundle(Ref<StreamReader> r, Ref<StreamWriter> w);

public:
	PackBundle(Archive* bundleRoot, const String& uid);
	PackBundle(StreamSource* bundleFile, PackBundle* base = NULL);
	virtual ~PackBundle();

	static Ref<PackBundle>				linkBundles(AppConfig* appConfig);

public:
	PackBundle*							getBase()								{ return _base; }
	const String&						getUid()								{ return _uid; }
	const String&						getBaseUid()							{ return _baseUid; }
	const StringVector&					getCompats()							{ return _compats; }
	const String&						getRevision()							{ return _revision; }

	File*								getRealFile()							{ return _realFile; }
	StreamLocator*						getLocator()							{ return _locator; }
	PackArchive*						getPack()								{ return _pack; }

	bool								isActive()								{ return _active; }
	bool								isValid()								{ return _valid; }
	bool								isCompatible(const String& version);
	bool								isRecentThan(PackBundle* other);

public:
	Package*							link(const String& packName);

public:									// StreamLocator impl
	virtual bool						isCaseSensitive()						{ return _required.empty() ? true : _required[0]->isCaseSensitive(); }

	virtual StreamSource*				locateLocal(const String& streamName);
	virtual void						findLocal(const String& pattern, StreamSourceMap& varResults);

public:									// Development Utility (not for shipping version)
	void								collectPackCfgs(StreamSourceMap& varResults);

private:
	friend class PackageService;

	Ref<PackBundle>						_base;
	vector<Ref<PackBundle> >::type		_expansions;

	Ref<File>							_realFile;
	Ref<StreamLocator>					_locator;
	Ref<PackArchive>					_pack;

	String								_uid;
	String								_baseUid;
	StringVector						_compats;
	String								_revision;
	bool								_active : 1;
	bool								_valid : 1;

	Package*							linkLocal(const String& packName);

	bool								loadCfg();
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
