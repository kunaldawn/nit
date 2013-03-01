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

#include "nitbundler_pch.h"

#include "nitbundler/Job.h"

#include "nit/script/NitBind.h"
#include "nit/script/NitBindMacro.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NITBUNDLER_API, nit::BundlerService, Service, incRefCount, decRefCount);

class NB_BundlerService : TNitClass<BundlerService>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(JobManager),
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY_H(GetPlatform,	"(name: string): bundler.Platform"),
			FUNC_ENTRY_H(AllPlatforms,	"(): bundler.Platform[]"),

			FUNC_ENTRY_H(NewBuilder,	"(appCfgFullPath: string, platformID: string, buildTarget: string, outPath=\"\"): bundler.Builder"),

			FUNC_ENTRY_H(Build,			"(appCfgFullPath: string, platformID: string, buildTarget: string): AsyncJob"),
			FUNC_ENTRY_H(BuildDelta,	"(topZBundle: StreamSource, baseZBundle: StreamSource, outPath: Archive): AsyncJob"),

			FUNC_ENTRY_H(SvnRevision,	"[class] (path: string): string"),
			FUNC_ENTRY_H(ExecuteHack,	"[class] (cmdline: string, timeout = 5000): string"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(JobManager)				{ return push(v, self(v)->getJobManager()); }

	NB_FUNC(GetPlatform)				{ return push(v, self(v)->getPlatform(getString(v, 2))); }

	NB_FUNC(AllPlatforms)
	{
		std::vector<bundler::Platform*> platforms;
		self(v)->allPlatforms(platforms);

		sq_newarray(v, 0);
		for (uint i=0; i<platforms.size(); ++i)
			arrayAppend(v, -1, platforms[i]);

		return 1;
	}

	NB_FUNC(NewBuilder)					{ return push(v, self(v)->newBuilder(getString(v, 2), getString(v, 3), getString(v, 4), optString(v, 5, ""))); }

	NB_FUNC(Build)						{ return push(v, self(v)->build(getString(v, 2), getString(v, 3), getString(v, 4))); }
	NB_FUNC(BuildDelta)					{ return push(v, self(v)->buildDelta(get<StreamSource>(v, 2), get<StreamSource>(v, 3), get<Archive>(v, 4))); }

	NB_FUNC(SvnRevision)				{ return push(v, type::svnRevision(getString(v, 2))); }
	NB_FUNC(ExecuteHack)				{ return push(v, type::executeHack(getString(v, 2), optInt(v, 3, 5000))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NITBUNDLER_API, nit::bundler::Platform, Module, incRefCount, decRefCount);

class NB_BundlerPlatform : TNitClass<bundler::Platform>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(BigEndian),
			PROP_ENTRY_R(PlatformID),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY	(_dump),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(BigEndian)				{ return push(v, self(v)->isBigEndian()); }
	NB_PROP_GET(PlatformID)				{ return push(v, self(v)->getPlatformID()); }

	NB_FUNC(_dump)
	{
//		Self(v)->Help();
		return push(v, true);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NITBUNDLER_API, nit::bundler::Builder, NULL, incRefCount, decRefCount);

class NB_BundlerBuilder : TNitClass<bundler::Builder>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(Platform),
			PROP_ENTRY_R(PlatformID),
			PROP_ENTRY_R(BuildTarget),
			PROP_ENTRY_R(Revision),
			PROP_ENTRY_R(AppCfg),
			PROP_ENTRY_R(AppCfgInclude),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(AllPacks,		"(): { name = bundler.PackSource }"),
			FUNC_ENTRY_H(Build,			"(packFilter=\"*\", fileFilter=\"*\", makeBundle=true): AsyncJob"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(Platform)				{ return push(v, self(v)->getPlatform()); }
	NB_PROP_GET(PlatformID)				{ return push(v, self(v)->getPlatformID()); }
	NB_PROP_GET(BuildTarget)			{ return push(v, self(v)->getBuildTarget()); }
	NB_PROP_GET(Revision)				{ return push(v, self(v)->getRevision()); }
	NB_PROP_GET(AppCfg)					{ return push(v, self(v)->getAppCfg()); }
	NB_PROP_GET(AppCfgInclude)			{ return push(v, self(v)->getAppCfgInclude()); }

	NB_FUNC(AllPacks)
	{
		sq_newtable(v);
		type::PackSources& packs = self(v)->allPacks();

		for (type::PackSources::iterator itr = packs.begin(), end = packs.end(); itr != end; ++itr)
		{
			newSlot(v, -1, itr->first, itr->second.get());
		}

		return 1;
	}

	NB_FUNC(Build)						
	{ 
		AsyncJob* job = self(v)->newJob(optString(v, 2, "*"), optString(v, 3, "*"), optBool(v, 4, true));
		g_Bundler->getJobManager()->enqueue(job);
		return push(v, job); 
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NITBUNDLER_API, nit::bundler::PackSource, NULL, incRefCount, decRefCount);

class NB_BundlerPackSource : TNitClass<bundler::PackSource>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(Name),
			PROP_ENTRY_R(Locator),
			PROP_ENTRY_R(Builder),

			PROP_ENTRY_R(PackCfg),
			PROP_ENTRY_R(PackCfgInclude),

			PROP_ENTRY_R(Alias),
			PROP_ENTRY_R(Linked),
			PROP_ENTRY_R(Ignored),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(FindSource,	"(pattern: string): StreamSource[]"),
			FUNC_ENTRY_H(FindTarget,	"(pattern: string): string[]"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(Name)					{ return push(v, self(v)->getName()); }
	NB_PROP_GET(Locator)				{ return push(v, self(v)->getLocator()); }
	NB_PROP_GET(Builder)				{ return push(v, self(v)->getBuilder()); }
	NB_PROP_GET(PackCfg)				{ return push(v, self(v)->getPackCfg()); }
	NB_PROP_GET(PackCfgInclude)			{ return push(v, self(v)->getPackCfgInclude()); }
	NB_PROP_GET(Alias)					{ return push(v, self(v)->isAlias()); }
	NB_PROP_GET(Linked)					{ return push(v, self(v)->isLinked()); }
	NB_PROP_GET(Ignored)				{ return push(v, self(v)->isIgnored()); }

	NB_FUNC(FindSource)
	{
		StreamSourceMap sl;

		self(v)->findSource(getString(v, 2), sl);

		sq_newarray(v, 0);
		for (StreamSourceMap::iterator itr = sl.begin(), end = sl.end(); itr != end; ++itr)
			arrayAppend(v, -1, itr->second.get());
		return 1;
	}

	NB_FUNC(FindTarget)
	{
		StringVector files;

		self(v)->findTarget(getString(v, 2), files);

		sq_newarray(v, 0);
		for (uint i=0; i < files.size(); ++i)
			arrayAppend(v, -1, files[i]);
		return 1;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NITBUNDLER_API, nit::bundler::Packer, NULL, incRefCount, decRefCount);

class NB_BundlerPacker : TNitClass<bundler::Packer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NITBUNDLER_API, nit::bundler::Builder::Job, AsyncJob, incRefCount, decRefCount);

class NB_BundlerBuilderJob : TNitClass<bundler::Builder::Job>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(Output),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(Output)					{ return push(v, self(v)->getOutput()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NITBUNDLER_API, nit::bundler::DeltaBuildJob, AsyncJob, incRefCount, decRefCount);

class NB_BundlerDeltaBuildJob : TNitClass<bundler::DeltaBuildJob>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(Output),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(Output)					{ return push(v, self(v)->getOutput()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NITBUNDLER_API, nit::bundler::ZBundleBuildJob, AsyncJob, incRefCount, decRefCount);

class NB_BundlerZBundleBuilderJob : TNitClass<bundler::ZBundleBuildJob>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(Output),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(Output)					{ return push(v, self(v)->getOutput()); }
};

////////////////////////////////////////////////////////////////////////////////

NITBUNDLER_API SQRESULT NitLibBundler(HSQUIRRELVM v)
{
	NB_BundlerService::Register(v);
	NB_BundlerPlatform::Register(v);
	NB_BundlerBuilder::Register(v);
	NB_BundlerPackSource::Register(v);
	NB_BundlerPacker::Register(v);

	NB_BundlerBuilderJob::Register(v);
	NB_BundlerDeltaBuildJob::Register(v);
	NB_BundlerZBundleBuilderJob::Register(v);

	if (g_Service)
	{
		sq_pushroottable(v);
		NitBind::newSlot(v, -1, "bundler", g_Bundler);
		sq_poptop(v);
	}

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
