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

#include "nit/async/AsyncJob.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

#define g_Bundler						static_cast<nit::BundlerService*>(g_Service->getService(::nit::Service::SVC_BUNDLER))

////////////////////////////////////////////////////////////////////////////////

class NITBUNDLER_API BundlerService : public Service
{
public:
	BundlerService(Package* pack);

public:
	bundler::Platform*					getPlatform(const String& platform);
	void								allPlatforms(std::vector<bundler::Platform*>& outPlatforms);

public:
	bundler::Builder*					newBuilder(const String& appCfgPath, const String& platformID, const String& buildTarget, const String& outPath="");

	AsyncJobManager*					getJobManager()							{ return _jobManager; }

	AsyncJob*							build(const String& appCfgPath, const String& platformID, const String& buildTarget);
	AsyncJob*							buildDelta(Ref<StreamSource> topZBundle, Ref<StreamSource> baseZBundle, Ref<Archive> outPath);

public:
	static String						getBuildHost();

	static String						svnRevision(const String& path);

	static String						executeHack(const String& cmdline, int timeoutMillis = 5000);

protected:								// Module Impl
	virtual void						onRegister();
	virtual void						onUnregister();
	virtual void						onInit();
	virtual void						onFinish();

	void								onAppLoop(const Event* evt);

	Ref<AsyncJobManager>				_jobManager;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
