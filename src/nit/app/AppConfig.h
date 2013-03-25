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

#include "nit/data/Settings.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class Session;
class Settings;

class NIT_API AppConfig : public RefCounted
{
public:
	AppConfig(Settings* fullSettings);
	virtual ~AppConfig();

public:
	static Settings*					locate(const char* exePath, const char* filename = NULL);

public:
	void								link(Settings* settings);

public:
	Settings*							getSettings()							{ return _active; }
	Settings*							getFullSettings()						{ return _fullSettings; }

	// TODO: cache string and return const String&. Otherwise corruption may happen!
	String								get(const String& keypath, const String& defValue = StringUtil::BLANK());

public:
	String								getName()								{ return get("app_name"); }
	String								getTitle()								{ return get("app_title"); }
	String								getEdition()							{ return get("app_edition"); }
	String								getVersion()							{ return get("app_version"); }

	String								getPlatform()							{ return get("platform"); }
	String								getBuild()								{ return get("build"); }
	String								getDeviceModel()						{ return get("device_model"); }
	String								getDeviceForm()							{ return get("device_form"); }
	String								getOsVersion()							{ return get("os_version"); }

	String								getBundlePath()							{ return get("app_bundle_path"); }
	String								getPluginPath()							{ return get("plugin_path"); }
	String								getPatchPath()							{ return get("patch_path"); }
	String								getAppSavePath()						{ return get("app_save_path"); }
	String								getUserSavePath()						{ return get("user_save_path"); }

	String								getBootSession()						{ return get("boot_session", "startup"); }
	String								getBootArgument()						{ return get("boot_argument"); }

public:
	int									getExitCode()							{ return _exitCode; }
	void								setExitCode(int exitCode)				{ _exitCode = exitCode; }

protected:
	virtual void						init(Settings* settings);
	void								applySettings(Settings* settings);

	Ref<Settings>						_fullSettings;
	Ref<Settings>						_active;
	int									_exitCode;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
