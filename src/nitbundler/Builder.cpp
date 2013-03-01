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

#include "nitbundler/Builder.h"
#include "nitbundler/Job.h"

#include "nit/app/PackArchive.h"
#include "nit/app/PackBundle.h"

#include "nit/runtime/NitRuntime.h"

NS_BUNDLER_BEGIN;

////////////////////////////////////////////////////////////////////////////////

Builder::Builder(Settings* appCfg, Platform* platform, const String& buildTarget, String outPath)
{
	_appCfg		= appCfg;
	_platform		= platform;

	_platformID	= platform->getPlatformID();
	_buildTarget	= buildTarget;

	_sectionName	= "bundler";
	_platformSectionName = String("bundler/") + _platformID;

	Ref<StreamLocator> locator = appCfg->getSource()->getLocator();

	// Obtain build host
	_buildHost = g_Bundler->getBuildHost();

	// Check if app.cfg is valid
	if (appCfg->get("app_name").empty())
		NIT_THROW_FMT(EX_SYNTAX, "'app_name' not found in '%s'", appCfg->getName());

	if (appCfg->get(_platformSectionName + "/app_compat_ver").empty())
		NIT_THROW_FMT(EX_SYNTAX, "'app_compat_ver' not found in '%s'", _platformSectionName.c_str());

	// Setup out_path
	outPath = _appCfg->get(_sectionName + "/out_path", outPath, false);
	outPath = _appCfg->get(_platformSectionName + "/out_path", outPath, false);

	if (outPath.empty())
		NIT_THROW_FMT(EX_INVALID_PARAMS, "'out_path' not specified");

	if (!FileUtil::isAbsolutePath(outPath))
		outPath = locator->makeUrl(outPath);

	FileUtil::createDir(outPath);
	_outPath = new FileLocator("$out_path", outPath, false, true);
	_outPath->load();

	// Setup dump_path
	String dumpPath = StringUtil::format("%s/%s_dump", outPath.c_str(), buildTarget.c_str());

	FileUtil::createDir(dumpPath);
	_dumpPath = new FileLocator("$dump_path", dumpPath, false, true);
	_dumpPath->load();

	// Obtain revision number from subversion
	String appCfgFilename, appCfgPath;
	StringUtil::splitFilename(_appCfg->getUrl(), appCfgFilename, appCfgPath);
	if (!appCfgPath.empty()) 
		appCfgPath.resize(appCfgPath.size()-1);

	_revision = g_Bundler->svnRevision(appCfgPath);

	if (_revision.empty())
		NIT_THROW_FMT(EX_SYSTEM, "can't get revision from '%s'", appCfgPath.c_str());

	if (_revision.find_first_not_of("0123456789") != _revision.npos)
	{
		LOG(0, "*** warning: '%s' is not up-to-date revision: %s\n", appCfgPath.c_str(), _revision.c_str());

		// ':' is not appropriate letter for a filename
		for (uint i=0; i<_revision.length(); ++i)
			if (_revision[i] == ':')
				_revision[i] = 't';
	}

	// Include files specified by bundler_include
	_packLocator	= new CustomArchive("$pack_root");

	StringVector includes;
	_appCfg->find("bundler/bundler_include", includes);

	for (uint i=0; i<includes.size(); ++i)
	{
		if (_appCfgInclude)
			NIT_THROW_FMT(EX_NOT_SUPPORTED, "only one 'budnler_include' supported yet");

		_appCfgInclude = Settings::load(locator->locate(includes[i]));

		if (_appCfgInclude == NULL)
			NIT_THROW_FMT(EX_NOT_FOUND, "can't include '%s'", includes[i].c_str());
	}

	// Setuup pack_path
	if (_appCfgInclude)
	{
		collectPackLocators(_appCfgInclude->getSection(_sectionName));
		collectPackLocators(_appCfgInclude->getSection(_platformSectionName));
	}

	collectPackLocators(_appCfg->getSection(_sectionName));
	collectPackLocators(_appCfg->getSection(_platformSectionName));

	// Collect packs list
	collect("*");
}

void Builder::collectPackLocators(Settings* section)
{
	if (section == NULL)
		return;

	StringVector paths;
	section->find("pack_path", paths);

	Ref<FileLocator> pathLocator = dynamic_cast<FileLocator*>(section->getSource()->getLocator());

	for (uint i=0; i<paths.size(); ++i)
	{
		const String& path = paths[i];

		Ref<FileLocator> packLocator = new FileLocator(path, pathLocator->getBaseUrl() + "/" + path, true, true);

		packLocator->load();

		_packLocator->require(packLocator);
	}

	section->find("pack_ignore", _packIgnoreFilters);
}

// TODO: Replace to use PackageService::FilterPackCFGs()

void Builder::collect(const String& filter)
{
	// Collect every single pack from all recursive pack locators
	StreamSourceMap packCfgs;
	_packLocator->find("pack.cfg", packCfgs);
	_packLocator->find("*.pack.cfg", packCfgs);

	for (StreamSourceMap::iterator itr = packCfgs.begin(), end = packCfgs.end(); itr != end; ++itr)
	{
		bool isAlias = false;

		size_t tailPos = itr->first.find("/pack.cfg");
		if (tailPos == String::npos) 
		{
			tailPos = itr->first.find(".pack.cfg");
			isAlias = tailPos != String::npos;
		}

		if (tailPos == String::npos)
			continue;

		if (tailPos == 0) // bad name
			continue;

		String packname = itr->first.substr(0, tailPos);
		if (!Wildcard::match(filter, packname))
			continue;

		StringUtil::toLowerCase(packname);

		Ref<Settings> packCfg = Settings::load(itr->second);
		if (packCfg == NULL)
		{
			LOG(0, "*** can't load %s\n", packCfg->getUrl().c_str());
			continue;
		}

		Ref<PackSource> pack = new PackSource(this, packCfg, packname, isAlias);

		_packs.insert(std::make_pair(packname, pack));
	}

	// Collecting completed, do link
	for (PackSources::iterator itr = _packs.begin(), end = _packs.end(); itr != end; ++itr)
	{
		PackSource* pack = itr->second;

		bool ignored = false;

		for (uint i=0; !ignored && i<_packIgnoreFilters.size(); ++i)
		{
			if (Wildcard::match(_packIgnoreFilters[i], itr->first))
				ignored = true;
		}

		pack->_ignored = ignored;

		pack->link();
	}

	// Check dependancy for ignored tagets, warn if missing dependancy
	for (PackSources::iterator itr = _packs.begin(), end = _packs.end(); itr != end; ++itr)
	{
		PackSource* pack = itr->second;

		if (!pack->isIgnored())
			continue;

		PackSource::Dependants& deps = pack->getDependants();
		for (PackSource::Dependants::iterator ditr = deps.begin(), dend = deps.end(); ditr != dend; ++ditr)
		{
			if ((*ditr)->isIgnored())
				continue;

			LOG(0, "*** ignored '%s' is required by non-ignored '%s'\n",
				pack->getName().c_str(), (*ditr)->getName().c_str());
		}
	}
}

PackSource* Builder::getPack(const String& packname)
{
	PackSources::iterator itr = _packs.find(packname);
	return itr != _packs.end() ? itr->second : NULL;
}

Builder::Job* Builder::newJob(const String& packFilter, const String& fileFilter, bool makeBundle)
{
	return new Job(this, packFilter, fileFilter, makeBundle);
}

Settings* Builder::createBundleSection(const String& uid, const String& baseUID)
{
	Settings* sect = new Settings();
	sect->add("uid", uid);
	sect->add("base_uid", baseUID);
	sect->add("bundle_title", _appCfg->get(_platformSectionName + "/bundle_title"));

	StringVector descs;
	_appCfg->find(_platformSectionName + "/bundle_desc", descs);
	for (uint i=0; i<descs.size(); ++i)
		sect->add("bundle_desc", descs[i]);

	sect->add("app_name", _appCfg->get("app_name"));
	sect->add("app_edition", _appCfg->get("app_edition"));
	sect->add("platform", _platformID);
	sect->add("build_target", _buildTarget);
	sect->add("revision", _revision);

	StringVector compats;
	_appCfg->find(_platformSectionName + "/app_compat_ver", compats);
	for (uint i=0; i<compats.size(); ++i)
		sect->add("app_compat_ver", compats[i]);

	return sect;
}

////////////////////////////////////////////////////////////////////////////////

NS_BUNDLER_END;
