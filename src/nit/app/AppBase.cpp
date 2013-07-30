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

#include "nit/app/AppBase.h"

#include "nit/app/AppConfig.h"

#include "nit/app/Session.h"
#include "nit/app/SessionService.h"
#include "nit/app/Package.h"
#include "nit/app/PackageService.h"

#include "nit/content/Content.h"
#include "nit/content/ContentManager.h"
#include "nit/content/ContentsService.h"

#include "nit/input/InputService.h"
#include "nit/input/InputUser.h"
#include "nit/input/InputDevice.h"
#include "nit/input/InputSource.h"

#include "nit/app/Plugin.h"

#include "nit/runtime/MemManager.h"
#include "nit/data/Settings.h"
#include "nit/net/DebugServer.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NIT_EVENT_DEFINE(APP_NATIVE_INIT, Event);
NIT_EVENT_DEFINE(APP_NATIVE_FINISH, Event);

NIT_EVENT_DEFINE(APP_BOOT, Event);
NIT_EVENT_DEFINE(APP_LOOP, Event);
NIT_EVENT_DEFINE(APP_STOP, Event);

NIT_EVENT_DEFINE(APP_ACTIVE, Event);
NIT_EVENT_DEFINE(APP_RESUME, Event);
NIT_EVENT_DEFINE(APP_SUSPEND, Event);
NIT_EVENT_DEFINE(APP_INACTIVE, Event);

NIT_EVENT_DEFINE(APP_LOW_MEMORY, Event);
NIT_EVENT_DEFINE(APP_SAVE_NOW, Event);

NIT_EVENT_DEFINE(APP_CONFIG_CHANGE, Event);

NIT_EVENT_DEFINE(APP_HANDLE_URL, AppURLEvent);

////////////////////////////////////////////////////////////////////////////////

class AppDataSchemaLookup : public DataSchemaLookup
{
public:
	virtual DataSchema* onLoad(DataKey* key)
	{
		// Try to load by using lookup information of PackageService
		
		PackageService::LookupResult r;

		if (!g_Package->lookup("schema", key->getName(), r))
			return NULL;

		const String& packname = r.pack;
		Package* pack = g_Package->load(packname.c_str());

		if (pack == NULL) return NULL;

		if (!r.entry.empty())
		{
			// process an entry whose subtype specified as 'script'
			if (r.subType.empty() || r.subType == "script")
			{
				ScriptRuntime* script = g_Session->getScript();
				Ref<ScriptUnit> unit;
				script->require(r.entry, unit, pack);
			}
		}

		// May the pack process the entry by subtype during loading
		// or may the Plugin or required script register related things again,
		// so we have to Get() again.

		DataSchema* schema = get(key);

		if (schema == NULL)
		{
			LOG(0, "*** package '%s': can't find schema '%s' even after loaded\n", packname.c_str(), key->getName().c_str());
		}

		return schema;
	}
};

////////////////////////////////////////////////////////////////////////////////

AppBase* AppBase::s_Current;
std::vector<AppBase*> AppBase::s_AppStack;

AppBase::AppBase()
{
	_config			= NULL;

	_debugging			= false;
	_stopping			= false;
	_restarting		= false;
	_booting			= false;

	_active			= false;
	_suspended			= false;

	_channel			= new EventChannel();
	_clock				= new Clock();
	_timer				= new TickTimer();
	_scheduler 		= new TimeScheduler();

	_debuggerFileSystem = NULL;

	_lastProcessTime	= 0;
}

AppBase::~AppBase(void)
{
}

class _RefCountedLeakChecker
{
public:
	static void LogManagerFinish()
	{
		int count = 0;
		RefCounted::_debuglistVisitAll(Visitor, &count);

		LOG(0, "%sRefCounted: total %d leak(s).\n", count > 0 ? "*** " : "++ ", count);
	}

	static bool Visitor(RefCounted* obj, void* up)
	{
		int& count = *(int*)up;

		if (count == 0)
		{
			LOG(0, "*** RefCounted object leak detected:\n");
		}

		++count;

		if (count == 20) 
		{
			LOG(0, "*** ... and more\n");
			return true;
		}
		else if (count > 20)
		{
			return true;
		}
		
		if (dynamic_cast<Object*>(obj))
		{
			Object* o = (Object*)obj;
			LOG(0, "*** Leak: %08x %s '%s' (ref=%d)\n", obj, typeid(*obj).name(), o->getName().c_str(), obj->getRefCount());
		}
		else if (dynamic_cast<Component*>(obj))
		{
			Component* c = (Component*)obj;
			Object* o = (Object*)c->getObject();
			LOG(0, "*** Leak: %08x %s '%s.%s' (ref=%d)\n", obj, typeid(*obj).name(), o ? o->getName().c_str() : "(null)", c->getName().c_str(), obj->getRefCount());
		}
		else
		{
			LOG(0, "*** Leak: %08x %s (ref=%d)\n", obj, typeid(*obj).name(), obj->getRefCount());
		}

		return true;
	}
};

void AppBase::enter()
{
	s_Current = this;
	s_AppStack.push_back(this);

	DataSchemaLookup::setCurrent(_schemaLookup);

	_serviceContext.enter();
}

void AppBase::leave()
{
	ASSERT_THROW(s_Current == this, EX_INVALID_STATE);

	if (_schemaLookup) // TODO: Fails when a _schemaLookup deleted!
		DataSchemaLookup::setCurrent(_schemaLookup->getUplink());

	s_AppStack.pop_back();
	s_Current = s_AppStack.empty() ? NULL : s_AppStack.back();

	_serviceContext.leave();
}

static void initMemPools(AppConfig* config)
{
	StringVector entries;
	config->getSettings()->find("mem/pool", entries);

	MemManager::RawArenas arenas;

	for (size_t i=0; i < entries.size(); ++i)
	{
		int entrySize, align, mega;
		sscanf(entries[i].c_str(), "%d,%d,%d", &entrySize, &align, &mega);

		MemManager::RawArena arena;
		arena.alignment = align;
		arena.entrySize = entrySize;
		arena.size = mega * 1024 * 1024;

		arenas.push_back(arena);
	}

	MemManager::getInstance()->initPools(arenas);
}

void AppBase::init(AppConfig* config)
{
	if (config == NULL)
		NIT_THROW(EX_INVALID_PARAMS);

	_config = config;
	
	LOG_TIMESCOPE(0, "++ App '%s' Initialize", _config->getName().c_str());

	_channel->bind(EVT::APP_LOOP, _clock->loopHandler());

	_clock->channel()->bind(EVT::CLOCK, _timer->sourceTimeHandler());
	_clock->channel()->bind(EVT::CLOCK, _scheduler->sourceTimeHandler());
	_clock->channel()->bind(EVT::CLOCK, this, &AppBase::onClock);
	
	// Cause PackBundle could override an app.cfg,
	// Prior to use _config, onInit() should be called first
	onInit();

	initMemPools(config);

	float timeSpeed = DataValue(getConfig("time_speed", "1.0"));
	float tickFrequency = DataValue(getConfig("tick_frequency", "60.0"));
	int tickLimit = DataValue(getConfig("tick_limit", "3"));

	_timer->setSpeed(timeSpeed);
	_timer->setTickInterval(1.0f / tickFrequency);
	_timer->setTickLimit(tickLimit);
}

class AppBase::DebuggerFileSystem : public IDebuggerFileSystem
{
public:
	DebuggerFileSystem(PackageService* packSvc) : _packageService(packSvc)
	{
	}

public:
	// IDebuggerFileSystem Impl

	virtual void getPacks(StreamLocatorList& outPacks)
	{
		// TODO: Do we need a app.Enter() ?

		if (_packageService == NULL)
			return;
		
		Ref<PackageService> pkgSvc = _packageService;

		Database* db = pkgSvc->getLookupDB();
		Database::Query* q = db->prepare("SELECT name, timestamp FROM packs");

		// TODO: just return names?
		while (q->step())
		{
			const char* packName = q->getText(0);
			Package* tempPack = pkgSvc->getBundle()->link(packName);
			outPacks.push_back(tempPack);
		}
	}

	virtual Ref<StreamSource> getFile(const String& packname, const String& filename, uint32& outCRC32)
	{
		// TODO: Do we need a app.Enter() ?

		if (_packageService == NULL)
			return NULL;

		Package* pack = _packageService->link(packname.c_str()); // TODO: link instead of load?

		if (pack == NULL) return NULL;

		Ref<StreamSource> source = pack->locate(filename, false);

		if (source == NULL) return NULL;

		PackArchive::File* f = dynamic_cast<PackArchive::File*>(source.get());
		if (f) outCRC32 = f->getSourceCRC32();

		return source;
	}

	Weak<PackageService>				_packageService;
};

void AppBase::onInit()
{
	// TODO: Execute file-job by filejob.cfg (decompress, create/remove directory, delete/copy/move files, etc.)

	// Link bundle according to initial config
	Ref<PackBundle> topBundle = PackBundle::linkBundles(_config);

	if (topBundle == NULL)
	{
		// TODO: Terminate Restart() process when there's no topBundle (maybe error)
		_stopping = true;
		_restarting = true;
		return;
	}

	// If the app.cfg on bundle has overrides, apply them
	Ref<StreamSource> overridedCfgFile = topBundle->locate("app.cfg", false);
	if (overridedCfgFile)
	{
		Ref<Settings> ov = Settings::load(overridedCfgFile);
		if (ov == NULL)
			LOG(0, "*** can't load : %s\n", overridedCfgFile->getUrl().c_str());
		else
			_config->link(ov);
	}

	// Install SchemaLookup
	_schemaLookup = new AppDataSchemaLookup();
	_schemaLookup->setUplink(DataSchemaLookup::getCurrent());

	// Retain usage count for important modules
	if (g_Package == NULL)
		Register(new PackageService(topBundle));

	if (g_SessionService == NULL)
		Register(new SessionService());

	if (g_Contents == NULL)
		Register(new ContentsService());

	if (svc_Input == NULL)
		Register(new InputService());

	g_Package->incUseCount();
	g_SessionService->incUseCount();
	g_Contents->incUseCount();
	svc_Input->incUseCount();

	registerStaticPlugins();

	_modules.init();

	{
		LOG_TIMESCOPE(0, "++ OnAppNativeInit");
		_channel->send(EVT::APP_NATIVE_INIT, new Event());
	}

	// NOTE: static plugins can't receive OnAppNativeInit (to be consistent with Dll Behavior)
	initStaticPlugins();

	// Install DebuggerFileSystem

	NitRuntime* rt = NitRuntime::getSingleton();
	DebugServer* ds = rt->getDebugServer();

	if (ds)
	{
		if (_debuggerFileSystem == NULL)
			_debuggerFileSystem = new DebuggerFileSystem(g_Package);

		ds->setFileSystem(_debuggerFileSystem);
	}
}

void AppBase::initStaticPlugins()
{
	LOG_TIMESCOPE(0, "++ InitStaticPlugins");
	// Register static plugins

	for (StaticPluginLookup::iterator itr = _staticPlugins.begin(), end = _staticPlugins.end(); itr != end; ++itr)
	{
		const String& name = itr->first;
		Plugin* plugin = itr->second;
		PluginDLL* dll = new PluginDLL(name, plugin);
		String packname = name + ".plugin";

		LOG(0, "++ static plugin: '%s'\n", packname.c_str());

		Package* pack = g_Package->linkCustom(packname, dll, NULL);
		if (pack) pack->incUseCount();
	}
}

void AppBase::start()
{
	_stopping	= false;

	_active	= true;
	_suspended = false;

	LOG(0, "++ Start: '%s' %s %s %s\n", 
		_config->getTitle().c_str(), 
		_config->getPlatform().c_str(),
		_config->getBuild().c_str(), 
		_config->getVersion().c_str());

	LOG_TIMESCOPE(0, "++ App '%s' Start", _config->getName().c_str());

	ScopedEnter sc(this);

	const String& bootSession = _config->getBootSession();
	const String& bootArgument = _config->getBootArgument();

	_booting = true;

	Ref<Session> startup = g_SessionService->create(bootSession, bootArgument);

	if (startup == NULL)
	{
		NitRuntime::getSingleton()->
			alert("boot fail", StringUtil::format("can't create boot session: %s(%s)\n", bootSession.c_str(), bootArgument.c_str()), true);

		_stopping = true;
	}

	g_SessionService->start(startup);
	_restarting = false;
}

void AppBase::finish()
{
	LOG_TIMESCOPE(0, "++ App '%s' Finish", _config ? _config->getName().c_str() : "<unknown>");

	ScopedEnter sc(this);

	onFinish();

	_config = NULL;
}

void AppBase::onFinish()
{
	// Release DebuggerFileSystem
	NitRuntime* rt = NitRuntime::getSingleton();
	DebugServer* ds = rt->getDebugServer();

	if (ds && _debuggerFileSystem)
	{
		ds->setFileSystem(NULL);

		safeDelete(_debuggerFileSystem);
	}

	{
		LOG_TIMESCOPE(0, "++ OnAppNativeFinish");
		_channel->send(EVT::APP_NATIVE_FINISH, new Event());
	}

	// Static plugin can receive onFinish() also (To be consistent with Dll behavior)
	finishStaticPlugins();

	_modules.finish();

	_serviceContext.unregisterAll();

	_schemaLookup = NULL;
}

void AppBase::finishStaticPlugins()
{
	LOG_TIMESCOPE(0, "++ FinishStaticPlugins");
	for (StaticPluginLookup::iterator itr = _staticPlugins.begin(), end = _staticPlugins.end(); itr != end; ++itr)
	{
		Plugin* plugin = itr->second;
		if (plugin->getPackage())
			plugin->getPackage()->decUseCount();
		plugin->dllUninstall();
	}
	_staticPlugins.clear();
}

void AppBase::checkLag()
{
	SystemTimer::Tick now = SystemTimer::currentTick();

	if (_lastProcessTime == 0)
		_lastProcessTime = now;

	SystemTimer::Tick delta = now - _lastProcessTime;
	float dt = float(delta * SystemTimer::secondsPerTick());

	_lastProcessTime = now;

	if (dt > 0.1f)
	{
		LOG(0, "++ App::Process(): Lag %.3f\n", dt);
	}
}

bool AppBase::loop()
{
	if (_stopping) 
		return false;

	ScopedEnter sc(this);

	onLoop();

	if (_booting)
	{
		LOG_TIMESCOPE(0, "++ OnAppBoot");
		_channel->send(EVT::APP_BOOT, new Event());
		_booting = false;
	}

	return !_stopping;
}

void AppBase::onLoop()
{
	// TODO: g_ProfilerService->ResetTimers();

	// Change active session if 'Next' session set.
	g_SessionService->changeIfNeeded();

	_channel->send(EVT::APP_LOOP, new Event());

	// Yield to another thread and loosen CPU burden
	Thread::sleep(0);
}

int AppBase::run()
{
	ScopedEnter sc(this);

	try
	{
		init();

		start();

		while (!isStopping())
		{
			try 
			{ 
				loop(); 
			}
			catch (Exception& e)
			{
				_stopping = !onUnhandled(e);
			}
			catch (...)
			{
				_stopping = !onUnhandledUnknown();
			}
		}

		finish();
	}
	catch (Exception& e)
	{
		onUnhandled(e);
	}
	catch (...)
	{
		onUnhandledUnknown();
	}

	return _config->getExitCode();
}

bool AppBase::onUnhandled(Exception& e)
{
	LOG(0, "!!! Unhandled exception : %s\n", e.getFullDescription().c_str());

	return false;
}

bool AppBase::onUnhandledUnknown()
{
	LOG(0, "!!! Unhandled unknown exception\n");

	return false;
}

void AppBase::onClock(const TimeEvent* evt)
{
	// TODO: _profilerService->ResetTimers();
}

void AppBase::stop(int exitCode)
{
	// Only exitCode of the first Stop() effective
	if (_stopping) return;

	LOG(0, "%s App Stop request with code: %d\n", exitCode == 0 ? "++" : "***", exitCode);

	_stopping = true;
	_setSuspended(true);
	_setActive(false);

	_config->setExitCode(exitCode);

	LOG_TIMESCOPE(0, "++ OnAppStop");
	_channel->send(EVT::APP_STOP, new Event());
}

void AppBase::restart()
{
	if (_restarting) return;

	LOG(0, "*** App Restart request\n");

	_restarting = true;

	_stopping = true; // stop, then AppRuntime restarts

	_setSuspended(true);
	_setActive(false);

	LOG_TIMESCOPE(0, "++ OnAppStop");
	_channel->send(EVT::APP_STOP, new Event());
}

void AppBase::Register(Module* module)
{
	ScopedEnter sc(this);

	_modules.Register(module);

	if (dynamic_cast<Service*>(module))
		_serviceContext.Register(static_cast<Service*>(module));
}

void AppBase::Register(const String& name, Plugin* plugin)
{
#ifndef NIT_STATIC
	NIT_THROW_FMT(EX_INVALID_STATE, "only static build can use static plugin ");
#endif

	if (_modules.isInitialized())
		NIT_THROW_FMT(EX_INVALID_STATE, "can't register static plugin '%s' after initialized", name.c_str());

	if (_staticPlugins.find(name) != _staticPlugins.end())
		NIT_THROW_FMT(EX_DUPLICATED, "static plugin '%s' already registered", name.c_str());

	_staticPlugins.insert(std::make_pair(name, plugin));
}

Plugin* AppBase::getStaticPlugin(const String& name)
{
	StaticPluginLookup::iterator itr = _staticPlugins.find(name);
	return itr != _staticPlugins.end() ? itr->second : NULL;
}

void AppBase::Unregister(Module* module)
{
	ScopedEnter sc(this);

	_modules.Unregister(module);

	if (dynamic_cast<Service*>(module))
		_serviceContext.Unregister(static_cast<Service*>(module));
}

void AppBase::saveNow()
{
	LOG_TIMESCOPE(0, "++ OnAppSaveNow");
	_channel->send(EVT::APP_SAVE_NOW, new Event());
}

void AppBase::_setActive(bool flag)
{
	if (_active == flag)
		return;

	_active = flag;

	if (flag)
	{
		LOG_TIMESCOPE(0, "++ OnAppActive");
		_channel->send(EVT::APP_ACTIVE, new Event());
	}
	else
	{
		LOG_TIMESCOPE(0, "++ OnAppInactive");
		_channel->send(EVT::APP_INACTIVE, new Event());
	}
}

void AppBase::_setSuspended(bool flag)
{
	if (_suspended == flag)
		return;

	_suspended = flag;

	if (flag)
	{
		LOG_TIMESCOPE(0, "++ OnAppSuspend"); 
		_channel->send(EVT::APP_SUSPEND, new Event());
	}
	else
	{
		LOG_TIMESCOPE(0, "++ OnAppResume"); 
		_channel->send(EVT::APP_RESUME, new Event());
	}
}

void AppBase::_notifyLowMemory()
{
	LOG_TIMESCOPE(0, "++ OnAppLowMemory");
	_channel->send(EVT::APP_LOW_MEMORY, new Event());
}

void AppBase::_notifyConfigChange()
{
	LOG_TIMESCOPE(0, "++ OnAppConfigChange");
	_channel->send(EVT::APP_CONFIG_CHANGE, new Event());
}

void AppBase::_notifyHandleURL(const String& url, const String& source, DataValue* annotation)
{
	LOG_TIMESCOPE(0, "++ OnAppHandleURL");
	LOG(0, "++ url: '%s' from '%s'\n", url.c_str(), source.c_str());
	LOG(0, "++ url annotation: %s\n", annotation ? "" : "null");
	// TODO: dump annotation

	Ref<AppURLEvent> evt = new AppURLEvent(url, source, annotation);
	_channel->send(EVT::APP_HANDLE_URL, evt);
}

////////////////////////////////////////////////////////////////////////////////

AppRuntime::AppRuntime()
{

}

void AppRuntime::prepare(AppBase* app, AppConfig* config)
{
	_app = app;
	_appConfig = config;
}

void AppRuntime::prepare(AppBase* app, const String& cfgFilename)
{
	_app = app;
	_cfgFilename = cfgFilename;
}

String AppRuntime::getTitle()
{
	if (_app && _app->getConfigs())
		return _app->getConfigs()->getTitle();
	else if (_appConfig)
		return _appConfig->getTitle();
	else
		return "AppRuntime";
}

bool AppRuntime::onInit()
{
	if (_appConfig == NULL)
	{
#if defined(NIT_ANDROID)
		LOG(0, ".. loading '%s' from asset manager\n", _cfgFilename.c_str());
		Ref<Settings> ss = Settings::load(getAssetArchive()->locate(_cfgFilename));
		_appConfig = new AppConfig(ss);
#else
		_appConfig = new AppConfig(AppConfig::locate(_arguments[0].c_str(), _cfgFilename.c_str()));
#endif
	}
	
	// Assume that single app, perform Enter() of _app on startup
	_entered = _app;
	_app->enter();

	_app->init(_appConfig);

	return true;

}

bool AppRuntime::onStart()
{
	_app->start();

	return true;
}

bool AppRuntime::onMainLoop()
{
	if (_app == NULL)
		return false;

	AppBase::ScopedEnter sc(_app);

	bool alive = onSystemLoop();

	if (!alive)
	{
		LOG(0, "++ AppRuntime: OnSystemLoop() no more alive\n");
		_app->stop(0);
	}

	if (_app->isStopping())
		return false;

	_app->loop();

	return !_app->isStopping();
}

int AppRuntime::onFinish()
{
	int exitCode = 0;

	if (_appConfig)
		exitCode = _appConfig->getExitCode();

	if (_app)
	{
		_restarting = _app->isRestarting();
		_app->finish();
	}

	if (_entered)
		_entered->leave();

	_appConfig = NULL;

	return exitCode;
}

void AppRuntime::debugCommand(const String& command)
{
	if (_app)
	{
		AppBase::ScopedEnter sc(_app);

		if (g_Session)
			g_Session->getScript()->command(command);
	}
}

Session* AppRuntime::defaultSessionFactory(const String& name, const String& args)
{
	return new Session();
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
