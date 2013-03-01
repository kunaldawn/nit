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

#include "nit/app/Module.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

Module::Module(const String& name, Package* package)
: _name(name), _package(package)
{
	_registered	= false;
	_unregistering = false;
	_initializing	= false;
	_initialized	= false;
	_finishing		= false;
	_finished		= false;

	_useCount		= 0;
}

Module::~Module()
{
	if (_initialized && !_finished)
		LOG(0, "*** '%s' destroyed without finished\n", _name.c_str());
}

void Module::init()
{
	if (_initializing)
		NIT_THROW_FMT(EX_INVALID_STATE, "'%s' init again, possible circular init-dependancy", _name.c_str());

	if (_initializing || _initialized) 
		return;

	_initializing = true;

	{

		for (uint i = 0; i < _required.size(); ++i)
		{
			_required[i]->init();
		}

		LOG_TIMESCOPE(0, ".. '%s' Initializing", _name.c_str());
		onInit();
	}

	_initializing	= false;
	_initialized	= true;
	_finishing		= false;
	_finished		= false;
}

void Module::finish()
{
	if (_useCount > 0) 
		return;

 	if (_finishing)
 		NIT_THROW_FMT(EX_INVALID_STATE, "'%s' finish again, possible circular finish-dependancy", _name.c_str());

	if (_finishing || _finished)
		return;

	_finishing = true;

	if (_initialized || _initializing)
	{
		{
			LOG_TIMESCOPE(0, ".. '%s' Finishing", _name.c_str());
			onFinish();
		}

		for (uint i = 0; i < _required.size(); ++i)
		{
			_required[i]->decUseCount();
			_required[i]->finish();
		}
		_required.clear();
	}

	_initializing	= false;
	_initialized	= false;
	_finishing		= false;
	_finished		= true;

	if (_unregistering && _registry)
		_registry->Unregister(this);
}

bool Module::isRequired(Module* mod)
{
	for (uint i=0; i<_required.size(); ++i)
	{
		if (_required[i] == mod) return true;
		if (_required[i]->isRequired(mod)) return true;
	}

	return false;
}

bool Module::require(Module* mod)
{
	if (_initializing || _initialized)
	{
		NIT_THROW_FMT(EX_INVALID_STATE, "'%s': require() allowed only on Register()", _name.c_str());
	}

	if (mod == NULL) return false;
	if (isRequired(mod)) return true;

	mod->incUseCount();
	_required.push_back(mod);

	return true;
}

void Module::printDependancy(int depth)
{
	for (uint i=0; i<_required.size(); ++i)
	{
		for (int d=0; d<depth; ++d)
		{
			LOG(0, "  ");
		}

		LOG(0, "- %s\n", _required[i]->getName().c_str());
		_required[i]->printDependancy(depth+1);
	}
}

////////////////////////////////////////////////////////////////////////////////

ModuleRegistry::ModuleRegistry()
{
	_initializing	= false;
	_initialized	= false;
}

ModuleRegistry::~ModuleRegistry()
{
	for (ModuleLookup::iterator itr = _nameLookup.begin(); itr != _nameLookup.end(); ++itr)
	{
		Module* mod = itr->second;
		if (mod == NULL) continue;

		if (!mod->isFinished())
		{
			LOG(0, "*** module '%s' not finished yet\n", mod->getName().c_str());
		}
	}
}

Module* ModuleRegistry::get(const String& name)
{
	ModuleLookup::iterator itr = _nameLookup.find(name);
	Module* mod = itr != _nameLookup.end() ? itr->second : NULL;
	if (mod && mod->_unregistering) mod = NULL;

	return mod;
}

void ModuleRegistry::find(const String& pattern, std::vector<Module*>& varResults)
{
	for (ModuleLookup::iterator itr = _nameLookup.begin(); itr != _nameLookup.end(); ++itr)
	{
		if (Wildcard::match(pattern, itr->first))
		{
			Module* mod = itr->second.get();
			if (mod && mod->_unregistering) mod = NULL;
			if (mod) varResults.push_back(itr->second.get());
		}
	}
}

void ModuleRegistry::Register(Module* module)
{
	if (module->_registered) return;

	if (module->_registry != NULL && module->_registry != this)
		NIT_THROW_FMT(EX_INVALID_STATE, "module '%s' already registered to another registry", module->getName().c_str());

	module->_registry = this;
	_nameLookup.insert(std::make_pair(module->getName(), module));
	module->onRegister();

	module->_registered = true;
	LOG(0, ".. '%s' registered\n", module->getName().c_str());

	if (_initialized)
	{
		module->init();
	}
}

void ModuleRegistry::Unregister(Module* module)
{
	if (!module->_registered) return;

	module->_unregistering = true;
	Ref<Module> safe = module;
	
	if (_initialized)
	{
		module->finish();
	}

	if (!module->isFinished())
		return; // Not finish-able yet. Will unregistered automatically at next finish.

	module->onUnregister();

	module->_unregistering = false;
	module->_registered = false;
	module->_registry = NULL;
	LOG(0, ".. '%s' unregistered\n", module->getName().c_str());

	_nameLookup.erase(module->getName());
}

void ModuleRegistry::init()
{
	if (_initializing || _initialized) 
		return;

	_initializing	= true;
	_initialized	= false;

	for (ModuleLookup::iterator itr = _nameLookup.begin(); itr != _nameLookup.end(); ++itr)
	{
		Module* module = itr->second;

		module->init();
	}

	_initializing	= false;
	_initialized	= true;
}

void ModuleRegistry::compact()
{
	std::vector<Module*> remaining; // Ref<> inappropriate here because Dll modules may throw problems on Ref<> instances after unloaded

	while (true)
	{
		for (ModuleLookup::iterator itr = _nameLookup.begin(); itr != _nameLookup.end(); ++itr)
		{
			Module* mod = itr->second;
			if (mod == NULL) continue;

			if (mod->getUseCount() > 0) continue;

			remaining.push_back(mod);
		}

		if (remaining.empty())
			break;

		for (uint i = 0; i < remaining.size(); ++i)
		{
			Module* mod = remaining[i];

			Unregister(mod);
		}

		remaining.clear();
	}
}

void ModuleRegistry::finish()
{
	bool done = false;

	while (!done)
	{
		compact();

		// To finish forcibly, decrease the UseCount of remaining mods

		done = true;
		for (ModuleLookup::iterator itr = _nameLookup.begin(); itr != _nameLookup.end(); ++itr)
		{
			Module* mod = itr->second;

			if (mod == NULL) continue;

			mod->decUseCount();
			// Forced finish fails, continue to try compact
			done = false;
		}
	}

	_initializing	= false;
	_initialized	= false;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
