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

#include "nit/runtime/NitRuntime.h"

#include "nit/io/FileLocator.h"
#include "nit/net/RemoteDebugger.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NIT_EVENT_DEFINE(OnRuntimeEnvChanged,	Event);
NIT_EVENT_DEFINE(OnRuntimeInit,			Event);
NIT_EVENT_DEFINE(OnRuntimeFinish,		Event);

////////////////////////////////////////////////////////////////////////////////

NitRuntimeBase* NitRuntimeBase::s_Singleton = NULL;

NitRuntimeBase::NitRuntimeBase()
{
	// ThreadLocalStorage should be initialized first before LogManager
	nit::ThreadLocalStorage::current();

	// initialize default MemManager if not installed
	if (MemManager::getInstance() == NULL)
	{
		_memManager = new MemManager();
		MemManager::initialize(_memManager);
	}

	_memManager = NULL;
	_started = false;
	_finished = false;
	_restarting = false;
	_infoEnabled = false;
	_alertEnabled = true;

	_debugServer = NULL;

	_config = new Settings();

#if defined(NIT_SHIPPING)
	_needDebugger = false;
#else
	_needDebugger = true;
#endif

#if defined(NIT_SHIPPING)
	_config->set("build", "shipping");
#elif defined(NIT_RELEASE)
	_config->set("build", "release");
#elif defined(NIT_DEVELOP)
	_config->set("build", "fastdebug");
#elif defined(NIT_DEBUG)
	_config->set("build", "debug");
#else
	_config->set("build", "<unknown>");
#endif

	if (s_Singleton == NULL)
		s_Singleton = this;
}

NitRuntimeBase::~NitRuntimeBase()
{
	if (s_Singleton == this)
		s_Singleton = NULL;

	// clean up before delete MemManager

	_config = NULL;
	_channel = NULL;

	if (_memManager)
		_memManager->shutdown();
	safeDelete(_memManager);
}

void NitRuntimeBase::setArguments(int argc, char** argv)
{
	_arguments.clear();
	for (int i=0; i<argc; ++i)
	{
		_arguments.push_back(argv[i]);
	}
}

bool NitRuntimeBase::init()
{
	_started = false;
	_finished = false;
	_restarting = false;

	try
	{
		////////////////////////////////////

		initPlatform();

		LOG(0, "++ platform : %s\n", getPlatform().c_str());
		LOG(0, "++ build : %s\n", getBuild().c_str());
		LOG(0, "++ os_version : %s\n", getOsVersion().c_str());

		////////////////////////////////////

		updateEnv();

		updateNet();
		
		////////////////////////////

		if (_needDebugger)
		{
			_debugServer = new DebugServer();
		}
		
		////////////////////////////

		if (_channel) _channel->send(Events::OnRuntimeInit, new Event());

		if (!onInit())
			return false;

		_started = onStart();
		return _started;
	}
	catch (Exception& ex)
	{
		onException(ex);
	}
	catch (std::exception& ex)
	{
		onException(ex);
	}
	catch (...)
	{
		onUnknownException();
	}
	
	return false;
}

bool NitRuntimeBase::mainLoop()
{
	try
	{
		if (_debugServer)
			_debugServer->update();

		bool alive = onMainLoop();
		
		if (!alive)
		{
			info(getTitle(), "Loop Terminated");
			finish();
		}
		
		return alive;
	}
	catch (Exception& ex)
	{
		onException(ex);
	}
	catch (std::exception& ex)
	{
		onException(ex);
	}
	catch (...)
	{
		onUnknownException();
	}
	
	return false;
}

void NitRuntimeBase::debuggerLoop()
{
	try
	{
		if (_debugServer)
			_debugServer->update();
		
		bool alive = onSystemLoop();

		// TODO: handle alive
	}
	catch (Exception& ex)
	{
		onException(ex);
	}
	catch (std::exception& ex)
	{
		onException(ex);
	}
	catch (...)
	{
		onUnknownException();
	}
}

int NitRuntimeBase::finish()
{
	if (_finished) return _exitCode;
	
	try
	{
		_started = false;
		_finished = true;
		_exitCode = onFinish();

		if (_restarting)
			return 0;
		
		finishPlatform();

		if (_channel) _channel->send(Events::OnRuntimeFinish, new Event());

		_config = NULL;

		safeDelete(_debugServer);
		
		_channel = NULL;

		return _exitCode;
	}
	catch (Exception& ex)
	{
		onException(ex);
	}
	catch (std::exception& ex)
	{
		onException(ex);
	}
	catch (...)
	{
		onUnknownException();
	}
	return -1;
}

bool NitRuntimeBase::restart()
{
	_finished = false;
	_restarting = false;
	
	bool ok = onInit();
	
	ok = ok && onStart();
	
	_started = ok;
	_finished = !ok;
	return ok;
}

int NitRuntimeBase::run()
{
	bool ok = init();

	if (!ok)
		return finish();

	while (!_finished)
	{
		bool alive = mainLoop();

		if (!alive && _restarting)
		{
			alive = restart();
		}
	}
	
	return finish();
}

void NitRuntimeBase::onException(Exception &ex)
{
	alert(getTitle(), ex.getFullDescription());
}

void NitRuntimeBase::onException(std::exception& ex)
{
	alert(getTitle(), (String("std::exception: ") + ex.what()));
}

void NitRuntimeBase::onUnknownException()
{
	alert(getTitle(), "Unknown Exception");
}

bool NitRuntimeBase::replaceAlias(String& varAlias)
{
	varAlias = _config->expand(varAlias);

	return true;
}

Archive* NitRuntimeBase::openArchive(const String& name, const String& url)
{
	return new FileLocator(name, url);
}

const std::string& NitRuntimeBase::getMainIp()
{
	static std::string s_Blank;

	if (_ipAddrs.empty())
		return s_Blank;

	return getIPAddr("main");
}

const std::string& NitRuntimeBase::getIPAddr(const String& adapter)
{
	static std::string s_Blank;

	IPAddressMap::iterator itr = _ipAddrs.find(adapter.c_str());
	if (itr != _ipAddrs.end())
		return itr->second;

	return s_Blank;
}

int NitRuntimeBase::execute(const String& cmdline, Ref<StreamReader> input, Ref<StreamWriter> output, Ref<StreamWriter> err)
{
	NIT_THROW(EX_NOT_SUPPORTED);
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
