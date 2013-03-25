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

#include "nit_pch.h"

#include "nit/app/AppConfig.h"

#include "nit/app/Session.h"

#include "nit/data/Settings.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

AppConfig::AppConfig(Settings* fullSettings)
{
	assert(fullSettings);
	assert(fullSettings->getSource());

	init(fullSettings);
}

AppConfig::~AppConfig()
{
}

void AppConfig::init(Settings* fullSettings)
{
	_fullSettings = fullSettings;

	_exitCode = 0;

	link(fullSettings);
}

void AppConfig::link(Settings* fullSettings)
{
	if (fullSettings == NULL) return;

	if (fullSettings != _fullSettings)
	{
		LOG(0, "++ '%s' overrided by '%s'\n", 
			_fullSettings->getSource()->getName().c_str(), 
			fullSettings->getUrl().c_str());

		_fullSettings = fullSettings;
	}

	NitRuntime* rt = NitRuntime::getSingleton();

	const String& build = fullSettings->get("build", rt->getBuild(), false);
	const String& platform = fullSettings->get("platform", rt->getPlatform(), false);

	Ref<Settings> base = fullSettings;
	Ref<Settings> active;

	fullSettings->setBase(rt->getConfig());

	active = fullSettings->getSection(build);

	if (active)
	{
		active->setBase(base);
		base = active;
	}
	else active = base;

	active = fullSettings->getSection(platform);

	if (active)
	{
		active->setBase(base);
		base = active;
	}
	else active = base;

	active = fullSettings->getSection(platform + "_" + build);
	
	if (active)
	{
		active->setBase(base);
		base = active;
	}
	else active = base;

	_active = active;
}

Settings* AppConfig::locate(const char* givenExePath, const char* filename /*= NULL*/)
{
	String exeName;
	String workPath;
	String exePath;
	String cfgName;

	StringUtil::splitFilename(givenExePath, exeName, exePath);

	if (filename)
		cfgName = filename;
	else
		cfgName = exeName + ".cfg";

	LOG(0, "-- $(exe_name) detected as '%s' from '%s'\n", exeName.c_str(), givenExePath);

	Ref<FileLocator> fl = new FileLocator("$work_path", ".");
	workPath = fl->getBaseUrl();
	FileUtil::normalizeSeparator(workPath);
	LOG(0, "-- $(work_path) located to '%s'\n", workPath.c_str());

	fl = new FileLocator("$exe_path", exePath);
	exePath = fl->getBaseUrl();
	FileUtil::normalizeSeparator(exePath);
	LOG(0, "-- $(exe_path) located to '%s'\n", exePath.c_str());

	// Regret that we can't use Find() here because that '..' included in search cases

	String search;
	Ref<StreamSource> source;

	// 1. Assume it's in $exe_path
	search = "";
	LOG(0, ".. locating: %s\n", (exePath + "/" + search + cfgName).c_str());
	source = fl->locate(search + cfgName, false);
	if (source) goto found;

	// 2. Assume it's in $exe_path/bin, check ../
	search = "../";
	LOG(0, ".. locating: %s\n", (exePath + "/" + search + cfgName).c_str());
	source = fl->locate(search + cfgName, false);
	if (source) goto found;

	// 3. Assume it's in $exe_path/bin/debug/, check ../../
	search = "../../";
	LOG(0, ".. locating: %s\n", (exePath + "/" + search + cfgName).c_str());
	source = fl->locate(search + cfgName, false);
	if (source) goto found;

	// 4. Assume it's in $exe_path/build-xxx/bin/debug/, check ../../../
	search = "../../../";
	LOG(0, ".. locating: %s\n", (exePath + "/" + search + cfgName).c_str());
	source = fl->locate(search + cfgName, false);
	if (source) goto found;

found:
	if (source == NULL)
	{
		NIT_THROW_FMT(EX_NOT_FOUND, "Can't find app config '%s' from any search path", cfgName.c_str());
	}

	// Setup locator again
	fl = new FileLocator("$cfg_path", exePath + "/" + search);
	String cfgPath = fl->getBaseUrl();
	FileUtil::normalizeSeparator(cfgPath);
	LOG(0, "-- $(cfg_path) located to '%s'\n", cfgPath.c_str());

	source = fl->locate(cfgName);

	if (source == NULL)
	{
		NIT_THROW_FMT(EX_NOT_FOUND, "can't find '%s' from '%s'", cfgName.c_str(), cfgPath.c_str());
	}

	Settings* ret = Settings::load(source);

	ret->set("work_path", workPath);
	ret->set("exe_name", exeName);
	ret->set("exe_path", exePath);
	ret->set("cfg_path", cfgPath);

	return ret;
}

String AppConfig::get(const String& keypath, const String& defValue)
{
	const String& val = _active->get(keypath, defValue);
	return _active->expand(val);
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
