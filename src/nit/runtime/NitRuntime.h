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

#include "nit/event/Event.h"
#include "nit/runtime/MemManager.h"
#include "nit/data/Settings.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NitRuntime;
class Archive;
class DebugServer;

class NIT_API NitRuntimeBase : public WeakSupported
{
public:
	NitRuntimeBase();
	virtual ~NitRuntimeBase();

	static NitRuntime*					getSingleton()							{ return (NitRuntime*)(s_Singleton); }

public:
	bool								init();
	bool								mainLoop();
	bool								restart();
	int									finish();
	int									run();

	bool								isInfoEnabled()							{ return _infoEnabled; }
	bool								isAlertEnabled()						{ return _alertEnabled; }
	void								setInfoEnabled(bool flag)				{ _infoEnabled = flag; }
	void								setAlertEnabled(bool flag)				{ _alertEnabled = flag; }

	void								setArguments(int argc, char** argv);
	const std::vector<std::string>&		getArguments()							{ return _arguments; }

	bool								isStarted()								{ return _started; }
	bool								isFinished()							{ return _finished; }
	bool								isRestarting()							{ return _restarting; }

public:
	Mutex&								getMutex()								{ return _mutex; }

public:									// Runtime Configuration
	Settings*							getConfig()								{ return _config; }

	virtual String						getTitle() = 0;

	const String&						getPlatform()							{ return _config->get("platform"); }
	const String&						getBuild()								{ return _config->get("build"); }
	const String&						getDeviceName()							{ return _config->get("device_name"); }
	const String&						getDeviceModel()						{ return _config->get("device_model"); }
	const String&						getDeviceForm()							{ return _config->get("device_form"); }
	const String&						getOsVersion()							{ return _config->get("os_version"); }

	const String&						getLanguage()							{ return _config->get("language"); }
	const String&						getCountry()							{ return _config->get("country"); }

	const String&						getAppPath()							{ return _config->get("app_path"); }
	const String&						getAppDataPath()						{ return _config->get("app_data_path"); }
	const String&						getAppCachePath()						{ return _config->get("app_cache_path"); }
	const String&						getUserDataPath()						{ return _config->get("user_data_path"); }
	const String&						getUserCachePath()						{ return _config->get("user_cache_path"); }
	const String&						getSysTempPath()						{ return _config->get("sys_temp_path"); }

	bool								isNetAvailable()						{ return !_ipAddrs.empty(); }
	const String&						getHostName()							{ return _config->get("host_name"); }
	const std::string&					getMainIp();
	const std::string&					getIPAddr(const String& adapter);

	typedef std::map<std::string, std::string> IPAddressMap;
	const IPAddressMap&					getIpAddrs()							{ return _ipAddrs;}

	EventChannel*						channel()								{ return _channel ? _channel : _channel = new EventChannel(); }

public:
	virtual bool						replaceAlias(String& varAlias);
	virtual Archive*					openArchive(const String& name, const String& url);

public:
	DebugServer*						getDebugServer()						{ return _debugServer; }
	virtual void						debugCommand(const String& command) = 0;

protected:
	virtual bool						onInit() = 0;
	virtual bool						onStart() = 0;
	virtual bool						onMainLoop() = 0;
	virtual int							onFinish() = 0;

	virtual bool						onSystemLoop()							{ return true; }

public:
	virtual void						onException(Exception& ex);
	virtual void						onException(std::exception& ex);
	virtual void						onUnknownException();
	virtual void						info(const String& title, const String& message, bool userInfo=false) = 0;
	virtual void						alert(const String& title, const String& message, bool fatal=false) = 0;

public:
	virtual int							execute(const String& cmdline, Ref<StreamReader> input, Ref<StreamWriter> output, Ref<StreamWriter> err);

protected:
	MemManager*							_memManager;
	std::vector<std::string>			_arguments;
	bool								_needDebugger;
	bool								_started;
	bool								_finished;
	bool								_restarting;
	bool								_infoEnabled;
	bool								_alertEnabled;
	int									_exitCode;

protected:
	Mutex								_mutex;

protected:
	DebugServer*						_debugServer;
	friend class DebugServer;
	void								debuggerLoop();

protected:
	Ref<EventChannel>					_channel;
	Ref<Settings>						_config;
	IPAddressMap						_ipAddrs;

protected:
	virtual bool						initPlatform() = 0;
	virtual void						finishPlatform() = 0;
	virtual void						updateEnv() = 0;
	virtual void						updateNet() = 0;

private:
	static NitRuntimeBase*				s_Singleton;
};

NIT_EVENT_DECLARE(NIT_API, OnRuntimeEnvChanged, Event);
NIT_EVENT_DECLARE(NIT_API, OnRuntimeInit, Event);
NIT_EVENT_DECLARE(NIT_API, OnRuntimeFinish, Event);

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;

#if defined(NIT_WIN32)
#	include "nit/runtime/NitRuntime_win32.h"

#elif defined(NIT_IOS)
#	include "nit/runtime/NitRuntime_ios.h"

#elif defined(NIT_MAC32)
#	include "nit/runtime/NitRuntime_mac32.h"

#elif defined(NIT_ANDROID)
#	include "nit/runtime/NitRuntime_android.h"

#endif

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

// This Singleton class provides init()/Finish() callback synchronized to Runtime's init()/finish()
// Several headaches from memory manager & app.suspend/resume can be solved by this

template <typename TClass>
class TRuntimeSingleton: public WeakSupported, public IEventSink
{
public:									// IEventSink Impl
	virtual bool						isEventActive()							{ return true; }
	virtual bool						isDisposed()							{ return false; }

protected:
	virtual void						onInit() = 0;
	virtual void						onFinish() = 0;
	virtual void						onEnvChanged()							{ }

public:
	static TClass& getSingleton()
	{
		// ThreadLocal looks so ineffecient for a method like this which called very frequently,
		// using just a simple lock.
		
		// TODO: Assume that initialization only on main thread at now, find more safe way
		ScopedLock<Mutex> lock(NitRuntime::getSingleton()->getMutex());

		static TClass s_Singleton;

		if (!s_Singleton._initialized)
		{
			s_Singleton.onInit();
			s_Singleton._initialized = true;
		}

		return s_Singleton;
	}

protected:
	TRuntimeSingleton()
	{
		_initialized = false;

		NitRuntime* rt = NitRuntime::getSingleton();
		assert(rt);

		if (rt)
		{
			rt->channel()->bind(Events::OnRuntimeInit, this, &TClass::onRuntimeInit);
			rt->channel()->bind(Events::OnRuntimeEnvChanged, this, &TClass::onRuntimeEnvChanged);
			rt->channel()->bind(Events::OnRuntimeFinish, this, &TClass::onRuntimeFinish);
		}
	}

	~TRuntimeSingleton()
	{
		if (_initialized)
		{
			NitRuntime* rt = NitRuntime::getSingleton();

			if (rt)
			{
				rt->channel()->unbind(0, this);
			}
		}
	}

private:
	void onRuntimeInit(const Event* evt)
	{
		onInit();
		_initialized = true;
	}

	void onRuntimeEnvChanged(const Event* evt)
	{
		onEnvChanged();
	}

	void onRuntimeFinish(const Event* evt)
	{
		onFinish();
		_initialized = false;
	}

	bool								_initialized;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;