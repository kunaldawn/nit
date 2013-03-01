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

#include "nitbundler/BundlerService.h"
#include "nitbundler/Job.h"

#include "nit/app/AppBase.h"
#include "nit/app/PackageService.h"
#include "nit/app/PackBundle.h"

#include "nit/io/ZStream.h"

#include "freeimage/FreeImage.h"
#include "PvrTexLib.h"

NS_NIT_BEGIN;

using namespace bundler;

////////////////////////////////////////////////////////////////////////////////

BundlerService::BundlerService(Package* pack)
: Service("BundlerService", pack, SVC_BUNDLER)
{

}

Platform* BundlerService::getPlatform(const String& platform)
{
	return g_App->getModules()->get<Platform>(String("bundler_") + platform);
}

void BundlerService::allPlatforms(std::vector<Platform*>& outPlatforms)
{
	g_App->getModules()->find("bundler_*", outPlatforms);
}

void BundlerService::onRegister()
{

}

void BundlerService::onUnregister()
{

}

static void freeimage_errhandler(FREE_IMAGE_FORMAT fif, const char* message)
{
	LOG(0, "*** [FreeImage] %s\n", message);
}

void BundlerService::onInit()
{
	LOG(0, "++ sizeof(PackArchive::Header): %d\n", sizeof(PackArchive::Header));
	LOG(0, "++ sizeof(PackArchive::FileEntry): %d\n", sizeof(PackArchive::FileEntry));

	// TODO: Implement a codec mechanism to nit::Image

	// Check FreeImage DLL
	LOG(0, "++ FreeImage Version: %s\n", FreeImage_GetVersion());
	LOG(0, "++ %s\n", FreeImage_GetCopyrightMessage());

	FreeImage_SetOutputMessage(freeimage_errhandler);

	// Check PVRTEXLIB DLL
	uint pvrTexLibVerMajor;
	uint pvrTexLibVerMinor;

	pvrtexlib::PVRTextureUtilities ut;
	ut.getPVRTexLibVersion(pvrTexLibVerMajor, pvrTexLibVerMinor);

	LOG(0, "++ PVRTexLibVersion: %d.%d\n", pvrTexLibVerMajor, pvrTexLibVerMinor);

	// Intialize Job Manager
	_jobManager = new AsyncJobManager("bdlr", Thread::getMaxConcurrency() - 1);
	_jobManager->resume();

	g_App->channel()->bind(Events::OnAppLoop, this, &BundlerService::onAppLoop);
}

void BundlerService::onFinish()
{
	g_App->channel()->unbind(Events::OnAppLoop, this);

	_jobManager->stop();
	_jobManager = NULL;
}

void BundlerService::onAppLoop(const Event* evt)
{
	if (_jobManager)
		_jobManager->update();
}

Builder* BundlerService::newBuilder(const String& appCfgPath, const String& platformID, const String& buildTarget, const String& outPath)
{
	g_Package->load(StringUtil::format("nitbundler/%s", platformID.c_str()).c_str());

	Platform* platform = getPlatform(platformID);
	if (platform == NULL)
		NIT_THROW_FMT(EX_NOT_FOUND, "Can't find bundler platform '%s'", platformID.c_str());

	String cfgFilename, path;

	StringUtil::splitFilename(appCfgPath, cfgFilename, path);

	Ref<FileLocator> fl = new FileLocator("", path);
	Ref<StreamSource> s = fl->locate(cfgFilename);
	Ref<Settings> appCfg = Settings::load(s);

	return new Builder(appCfg, platform, buildTarget, outPath);
}

String BundlerService::executeHack(const String& cmdline, int timeout)
{
	String result;

	Ref<FileLocator> fl = new FileLocator("$temp", NitRuntime::getSingleton()->getSysTempPath());

	String outFilename = fl->makeUrl("bundler_cmd_out.txt");
	Ref<StreamWriter> w = fl->create("bundler_cmd_out.txt");
	w = NULL;

	if (FileUtil::exists(outFilename))
		FileUtil::remove(outFilename);

	String cmd = StringUtil::format("%s > \"%s\"", cmdline.c_str(), outFilename.c_str());

	system(cmd.c_str());

	int tryCount = timeout / 100;

	bool success = false;

	for (int i=0; i<tryCount; ++i)
	{
		if (FileUtil::exists(outFilename))
		{
			FileUtil::readFile(outFilename, result);
			FileUtil::remove(outFilename);
			success = true;
			break;
		}
		Thread::sleep(100);
	}

	if (!success)
		NIT_THROW_FMT(EX_SYSTEM, "can't execute '%s'", cmd.c_str());

	return result;
}

String BundlerService::svnRevision(const String& path)
{
	NitRuntime* rt = NitRuntime::getSingleton();

	Ref<MemoryBuffer::Writer> w = new MemoryBuffer::Writer();
	int err = rt->execute(StringUtil::format("svnversion \"%s\"", path.c_str()), NULL, w.get(), NULL);

	if (err)
		NIT_THROW_FMT(EX_SYSTEM, "svnversion returns %d", err);

	String revision = w->getBuffer()->toString();
	StringUtil::trim(revision);

	return revision;
}

String BundlerService::getBuildHost()
{
	String buildHost = NitRuntime::getSingleton()->getHostName();

	// Check if buildHost contains illegal lettters
	for (uint i=0; i<buildHost.size(); ++i)
	{
		char ch = buildHost[i];
		if ('0' <= ch && ch <= '9') continue;
		if ('a' <= ch && ch <= 'z') continue;
		if ('A' <= ch && ch <= 'Z') continue;
		if (ch == '_' || ch == '-') continue;

		NIT_THROW_FMT(EX_SYNTAX, "Invalid build host name: '%s', rename your pc's name first!\n", buildHost.c_str());
	}

	return buildHost;
}

AsyncJob* BundlerService::build(const String& appCfgPath, const String& platformID, const String& buildTarget)
{
	Ref<Builder> builder = newBuilder(appCfgPath, platformID, buildTarget);

	AsyncJob* job = builder->newJob();
	_jobManager->enqueue(job);

	return job;
}

AsyncJob* BundlerService::buildDelta(Ref<StreamSource> topZBundle, Ref<StreamSource> baseZBundle, Ref<Archive> outPath)
{
	AsyncJob* job = new DeltaBuildJob(topZBundle, baseZBundle, outPath);
	_jobManager->enqueue(job);

	return job;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
