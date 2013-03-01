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

#include "nit/app/Service.h"
#include "nit/app/AppConfig.h"
#include "nit/app/Plugin.h"
#include "nit/app/Session.h"

#include "nit/runtime/NitRuntime.h"
#include "nit/data/DataValue.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

#ifndef NIT_APP_CLASS
#define NIT_APP_CLASS					nit::AppBase
#endif

#define g_App							static_cast<NIT_APP_CLASS*>(::nit::AppBase::getCurrent())

class Plugin;

class AppConfig;
class GameData;

class Clock;
class TickTimer;
class TimeScheduler;
class TimeEvent;

class Session;
class Object;

class DataSchemaLookup;

////////////////////////////////////////////////////////////////////////////////

class NIT_API AppBase : public WeakSupported
{
	friend class GameAppEventHandler;
	friend class FileService;

public:
	AppBase();
	virtual ~AppBase();

public:
	typedef nit::ScopedEnter<AppBase> ScopedEnter;

	static AppBase*						getCurrent()							{ return s_Current; }

	int									run(); // Init -> Start -> Loop -> Finish
	bool								loop();

public:
	void								init(AppConfig* config = NULL);
	void								start();
	void								finish();

public:									// Application Config
	AppConfig*							getConfigs()							{ return _config; }
	String								getConfig(const String& key, const String& defValue)	{ return _config->get(key, defValue); }

public:									// Application Lifetime
	bool								isBooting()								{ return _booting; }
	bool								isActive()								{ return _active; }
	bool								isSuspended()							{ return _suspended; }
	bool								isStopping()							{ return _stopping; }
	bool								isRestarting()							{ return _restarting; }

	void								saveNow();

	void								stop(int exitCode = 0);
	void								restart();

public:									// internal implementation only
	void								_setActive(bool flag);					
	void								_setSuspended(bool flag);
	void								_notifyLowMemory();
	void								_notifyConfigChange();
	void								_notifyHandleURL(const String& url, const String& source, DataValue* annotation = NULL);

public:									// Event & Timer
	EventChannel*						channel()								{ return _channel; }

	Clock*								getClock()								{ return _clock; }
	TickTimer*							getTimer()								{ return _timer; }
	TimeScheduler*						getScheduler()							{ return _scheduler; }

public:									// Module & Service management
	void								Register(Module* module);
	void								Unregister(Module* module);

	void								Register(const String& name, Plugin* plugin);

	Module*								getModule(const String& name)			{ return _modules.get(name); }
	ModuleRegistry*						getModules()							{ return &_modules; }

	template <typename TModClass>
	TModClass*							getModule(const String& name)			{ return _modules.get<TModClass>(name); }

	Plugin*								getStaticPlugin(const String& name);

	DataSchemaLookup*					getSchemaLookup()						{ return _schemaLookup; }

	ServiceContext*						getServiceContext()						{ return &_serviceContext; }

protected:
	virtual void						onInit();
	virtual void						onLoop();
	virtual void						onFinish();
	virtual bool						onUnhandled(Exception& e);
	virtual bool						onUnhandledUnknown();

protected:								
	static AppBase*						s_Current;
	static std::vector<AppBase*>		s_AppStack;

public:									// WARN: DO NOT CALL FROM USER CODE
	void								enter();
	void								leave();

protected:
	Ref<AppConfig>						_config;

protected:
	bool								_debugging;
	bool								_stopping;
	bool								_restarting;
	bool								_booting;

	bool								_active;
	bool								_suspended;

protected:
	Ref<EventChannel>					_channel;

	Ref<Clock>							_clock;
	Ref<TickTimer>						_timer;
	Ref<TimeScheduler>					_scheduler;

	SystemTimer::Tick					_lastProcessTime;

	ModuleRegistry						_modules;
	ServiceContext						_serviceContext;
	Ref<DataSchemaLookup>				_schemaLookup;

	typedef std::map<String, Ref<Plugin> > StaticPluginLookup;
	StaticPluginLookup					_staticPlugins;

	class DebuggerFileSystem;
	DebuggerFileSystem*					_debuggerFileSystem;

	virtual void						registerStaticPlugins()					{ }
	void								initStaticPlugins();
	void								finishStaticPlugins();

	void								onClock(const TimeEvent* evt);

	void								checkLag();
};

////////////////////////////////////////////////////////////////////////////////

NIT_EVENT_DECLARE(NIT_API, OnAppNativeInit, Event);
NIT_EVENT_DECLARE(NIT_API, OnAppNativeFinish, Event);

NIT_EVENT_DECLARE(NIT_API, OnAppBoot, Event);
NIT_EVENT_DECLARE(NIT_API, OnAppLoop, Event);
NIT_EVENT_DECLARE(NIT_API, OnAppStop, Event);

NIT_EVENT_DECLARE(NIT_API, OnAppActive, Event);
NIT_EVENT_DECLARE(NIT_API, OnAppSuspend, Event);
NIT_EVENT_DECLARE(NIT_API, OnAppResume, Event);
NIT_EVENT_DECLARE(NIT_API, OnAppInactive, Event);

NIT_EVENT_DECLARE(NIT_API, OnAppLowMemory, Event);
NIT_EVENT_DECLARE(NIT_API, OnAppSaveNow, Event);

NIT_EVENT_DECLARE(NIT_API, OnAppConfigChange, Event);

////////////////////////////////////////////////////////////////////////////////

class NIT_API AppURLEvent : public Event
{
public:
	AppURLEvent()						{ }
	AppURLEvent(const String& url, const String& source, DataValue* annotation) 
		: URL(url), Source(source), Annotation(annotation) { }

	String								URL;
	String								Source;
	DataValue*							Annotation;
};

NIT_EVENT_DECLARE(NIT_API, OnAppHandleURL, AppURLEvent);

////////////////////////////////////////////////////////////////////////////////

class NIT_API AppRuntime : public NitRuntime
{
public:
	AppRuntime();

public:
	void								prepare(AppBase* app, AppConfig* config);
	void								prepare(AppBase* app, const String& cfgFilename);

public:
	AppBase*							getApp()								{ return _app; }

public:
	static Session*						defaultSessionFactory(const String& name, const String& args);

protected:
	virtual String						getTitle();
	virtual bool						onInit();
	virtual bool						onStart();
	virtual bool						onMainLoop();
	virtual int							onFinish();
	virtual	void						debugCommand(const String& command);

protected:
	Weak<AppBase>						_app;
	Weak<AppBase>						_entered;
	Ref<AppConfig>						_appConfig;
	String								_cfgFilename;
};

////////////////////////////////////////////////////////////////////////////////

// TODO: remove obsoleted codes!

class MemManager;
class PooledAllocator;
class HeapAllocator;
struct MemDebugInfo;

class GameAppMemory
{
public:
	static void BindNewtonMemorySystem();

private:
	GameAppMemory();
	~GameAppMemory();

private:
	static GameAppMemory				g_Instance;

	struct RawArena
	{
		void*							RawBase;
		u16								EntrySize;
		u16								Alignment;
		uint							NumEntries;
		MemDebugInfo*					InfoBase;
		size_t							Size;
	};

	typedef std::vector<RawArena> RawArenas;	// we need std::vector here to avoid conflict with mem manager
	RawArenas							_arenas;
	
	void								InitMemManager();
	void								MakePool(PooledAllocator* pool, u16 entrySize, u16 align, size_t mega);
	void								FinishMemManager();
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
