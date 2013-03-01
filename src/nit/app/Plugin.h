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

#include "nit/app/SessionService.h"
#include "nit/io/Archive.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class PluginDLL;
class Module;
class NitBindLibRegistry;

////////////////////////////////////////////////////////////////////////////////

class NIT_API Plugin : public RefCounted
{
public:
	Plugin();
	virtual ~Plugin();

protected:
	const String&						getName();

	PluginDLL*							getDll()								{ return _dll; }
	Package*							getPackage()							{ return _package; }

public:
	Package*							require(const char* packageName, bool first = false);
	void								Register(Module* module);
	void								Register(const String& sessionName, SessionService::Constructor cons);
	void								Register(NitBindLibRegistry* lib);

protected:
	virtual void						onInstall() = 0;
	virtual void						onUninstall() = 0;

private:
	Weak<Package>						_package;

	friend class PluginDLL;
	friend class AppBase;

	Weak<PluginDLL>						_dll;
	void								dllInstall(PluginDLL* dll);
	void								dllUninstall();

	vector<Ref<Module> >::type			_myModules;
	StringVector						_mySessionClasses;
	std::vector<NitBindLibRegistry*>	_myNitLibs;
	vector<EventInfo*>::type			_myEventInfos;

	RefCounted::_DebuglistSnapshot		_refSnapshot;
};

////////////////////////////////////////////////////////////////////////////////

#	define NIT_DLL_EXTENSION			"dll"
#	define NIT_DLL_HANDLE				hInstance
#	define NIT_DLL_LOAD(a)				LoadLibraryExW(a, NULL, 0)
#	define NIT_DLL_GETSYM(a, b)			GetProcAddress(a, b)
#	define NIT_DLL_UNLOAD(a)			!FreeLibrary(a)
#	define NIT_DLL_PLUGIN_FN			NIT_create_plugin
#	define NIT_DLL_PLUGIN_FNNAME		"NIT_create_plugin"

#	ifdef NIT_DLL
#		define NIT_PLUGIN_DECLARE(NAME)			extern "C" API ::nit::Plugin* NIT_DLL_PLUGIN_FN();
#		define NIT_PLUGIN_DEFINE(API,NAME)		extern "C" API ::nit::Plugin* NIT_DLL_PLUGIN_FN() { return new NAME(); }
#		define NIT_PLUGIN_CREATE(NAME)			NULL
#	endif

#	ifdef NIT_STATIC
#		define NIT_PLUGIN_DECLARE(NAME)			extern "C" ::nit::Plugin* __NIT_CreatePlugin##NAME();
#		define NIT_PLUGIN_DEFINE(API,NAME)		extern "C" ::nit::Plugin* __NIT_CreatePlugin##NAME() { return new NAME(); }
#		define NIT_PLUGIN_CREATE(NAME)			__NIT_CreatePlugin##NAME()
#	endif

struct HSINSTANCE__;
typedef struct HINSTANCE__* hInstance;

typedef Plugin* (*DllCreatePluginFunction) (void);

////////////////////////////////////////////////////////////////////////////////

class NIT_API PluginDLL : public CustomArchive
{
public:
	// Dynamic Plugin
	PluginDLL(const String& name, const String& path); 

	// Static Plugin
	PluginDLL(const String& name, Plugin* plugin);

	virtual ~PluginDLL();

public:
	Plugin*								getPlugin()								{ return _plugin; }
	const String&						getPath()								{ return _path; }

	bool								isStatic()								{ return _path.empty(); }

public:									// CustomArchive overrides
	virtual void						load();
	virtual void						unload();

public:
private:
	Ref<Plugin>							_plugin;
	String								_path;
	NIT_DLL_HANDLE						_dllHandle;

	String								dllError();

private:
	friend class Package;
	bool								link(Package* package);
	static void							onEventInfoRegister(EventInfo* ei, void* context);
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
