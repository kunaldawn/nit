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

#include "nit/app/Plugin.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

Plugin::Plugin()
{

}

Plugin::~Plugin()
{

}

const String& Plugin::getName()
{
	return _dll->getName();
}

void Plugin::dllInstall(PluginDLL* dll)
{
	_refSnapshot.update(__FUNCTION__);

	_dll = dll;

	LOG_TIMESCOPE(0, ".. Plugin '%s' Install", getName().c_str());

	onInstall();

	if (g_Session && !_myNitLibs.empty())
	{
		g_Session->getRuntime()->updateEventBindings();
	}
}

void Plugin::dllUninstall()
{
	LOG_TIMESCOPE(0, ".. Plugin '%s' Uninstall", getName().c_str());

	onUninstall();

	for (uint i = 0; i < _myModules.size(); ++i)
	{
		Module* m = _myModules[i];
		g_App->Unregister(m);
	}
	_myModules.clear();

	std::for_each(_myNitLibs.begin(), _myNitLibs.end(), DeleteFunc());
	_myNitLibs.clear();

	for (uint i = 0; i < _myEventInfos.size(); ++i)
	{
		EventInfo::_DLLForceUnregister(_myEventInfos[i]);
	}
	_myEventInfos.clear();

	for (uint i = 0; i < _mySessionClasses.size(); ++i)
	{
		g_SessionService->Unregister(_mySessionClasses[i]);
	}
	_mySessionClasses.clear();

	_refSnapshot.dumpSince();
}

Package* Plugin::require(const char* packageName, bool first)
{
	Package* required = _package->getService()->link(packageName);

	if (required == NULL)
	{
		NIT_THROW_FMT(EX_LINK, "plugin '%s' can't link required package '%s'", getDll()->getName().c_str(), packageName);
		return NULL;
	}

	_package->require(required, first);

	return required;
}

void Plugin::Register(Module* module)
{
	// it's better and safe to make a Plugin's module require session & package service.
	if (g_SessionService)
		module->require(g_SessionService);

	if (g_Package)
		module->require(g_Package);

	_myModules.push_back(module);
	g_App->Register(module);
}

void Plugin::Register(NitBindLibRegistry* lib)
{
	_myNitLibs.push_back(lib);

	// Perform NitBind Install if Register happens with an opened-session.
	if (g_SessionService && g_Session)
	{
		ScriptRuntime* runtime = g_Session->getRuntime();

		if (runtime)
		{
			HSQUIRRELVM v = runtime->getRoot();
			NitBindLibRegistry::install(v, lib);

			// When a plugin uses NitBind during their loading, session script runtime holds their binding.
			// These plugins may occur problem on their uninstall provoked by Compact() if session not closed.
			// To prevent this, mark them to stay current session's lifetime.
			// After session's dead, it's safe to release NitBind-ings.
			_package->setStayForCurrent(true);
		}
	}
}

void Plugin::Register( const String& sessionName, SessionService::Constructor cons )
{
	g_SessionService->Register(sessionName, cons);

	_mySessionClasses.push_back(sessionName);
}

////////////////////////////////////////////////////////////////////////////////

PluginDLL::PluginDLL(const String& name, const String& path)
: CustomArchive(name), _path(path)
{
}

PluginDLL::PluginDLL(const String& name, Plugin* plugin)
: CustomArchive(name), _plugin(plugin)
{
}

PluginDLL::~PluginDLL()
{

}

void PluginDLL::onEventInfoRegister(EventInfo* ei, void* context)
{
	vector<EventInfo*>::type* vec = (vector<EventInfo*>::type*)context;
	vec->push_back(ei);
}

bool PluginDLL::link(Package* package)
{
	try
	{
		LOG(0, "++ linking plugin dll '%s'\n", getName().c_str());
		_plugin->_package = package;
		_plugin->dllInstall(this);
		return true;
	}
	catch (Exception& e)
	{
		LOG(0, "*** Can't link plugin '%s': %s\n", _plugin->getName().c_str(), e.getFullDescription().c_str());
		return false;
	}
	catch (...)
	{
		LOG(0, "*** Can't link plugin '%s': unknown exception\n", _plugin->getName().c_str());	
		return false;
	}
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
