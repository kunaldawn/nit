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

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class Module;
class Package;
class ModuleRegistry;

////////////////////////////////////////////////////////////////////////////////

class NIT_API Module : public RefCounted, public IEventSink
{
public:
	Module(const String& name, Package* package);
	virtual ~Module();

public:
	const String&						getName()								{ return _name; }
	Package*							getPackage()							{ return _package; }

public:									// IEventSink implementation
	virtual bool						isEventActive()							{ return isInitialized(); }
	virtual bool						isDisposed()							{ return isFinished(); }

public:
	void								init();
	void								finish();

	bool								isInitializing()						{ return _initializing; }
	bool								isInitialized()							{ return _initialized; }
	bool								isFinishing()							{ return _finishing; }
	bool								isFinished()							{ return _finished; }

public:
	bool								isRequired(Module* mod);
	bool								require(Module* mod);
	void								printDependancy(int depth);

protected:
	virtual void						onRegister() = 0;
	virtual void						onUnregister() = 0;

	virtual void						onInit() = 0;
	virtual void						onFinish() = 0;

public:									// UseCount management
	int									getUseCount()							{ return _useCount; }
	void								incUseCount()							{ ++_useCount; }
	void								decUseCount()							{ --_useCount; }

protected:
	friend class						ModuleRegistry;

	virtual void						onDelete()								{ finish(); }

private:
	String								_name;
	Weak<Package>						_package;
	Weak<ModuleRegistry>				_registry;

	std::vector<Ref<Module> >			_required;
	int									_useCount;

	bool								_registered : 1;
	bool								_unregistering : 1;
	bool								_initializing : 1;
	bool								_initialized : 1;
	bool								_finishing : 1;
	bool								_finished : 1;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API ModuleRegistry : public WeakSupported
{
public:
	ModuleRegistry();
	virtual ~ModuleRegistry();

public:
	Module*								get(const String& name);
	void								find(const String& pattern, std::vector<Module*>& varResults);

	template <typename TModClass>
	TModClass*							get(const String& name);
	
	template <typename TModClass>
	void								find(const String& pattern, std::vector<TModClass*>& varResults);

public:
	virtual void						Register(Module* module);
	virtual void						Unregister(Module* module);

public:
	bool								isInitialized()							{ return _initialized; }

	void								init();
	void								finish();

protected:
	typedef map<String, Ref<Module> >::type ModuleLookup;
	ModuleLookup						_nameLookup;

	bool								_initializing : 1;
	bool								_initialized : 1;

private:
	void								compact();
};

////////////////////////////////////////////////////////////////////////////////

template <typename TModClass>
TModClass* ModuleRegistry::get(const String& name)
{
	ModuleLookup::iterator itr = _nameLookup.find(name);
	if (itr == _nameLookup.end()) return NULL;

	Module* mod = itr != _nameLookup.end() ? itr->second : NULL;
	if (mod && mod->_unregistering) mod = NULL;

	TModClass* ret = dynamic_cast<TModClass*>(mod);
	return ret;
}

template <typename TModClass>
void ModuleRegistry::find(const String& pattern, std::vector<TModClass*>& varResults)
{
	for (ModuleLookup::iterator itr = _nameLookup.begin(); itr != _nameLookup.end(); ++itr)
	{
		if (Wildcard::match(pattern, itr->first))
		{
			TModClass* mod = dynamic_cast<TModClass*>(itr->second.get());
			if (mod && mod->_unregistering) mod = NULL;
			if (mod) varResults.push_back(mod);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
