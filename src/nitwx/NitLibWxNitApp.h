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

#include "nitwx/nitwx.h"

#include "nit/app/AppBase.h"

#include "nit/net//Remote.h"

#include <wx/filesys.h>

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class DefaultRuntime;

class NITWX_API wxNitApp : public wxApp, public WeakSupported, public IEventSink
{
public:
	wxNitApp();
	virtual ~wxNitApp();

public:
	AppBase*							getMainApp()							{ return _mainApp; }

public:									// IEventSink
	virtual bool						isEventActive()							{ return true; }
	virtual bool						isDisposed()							{ return false; } // TODO: Is this sufficient?

protected:								// NitRuntime Impl
	virtual String						getTitle();
	virtual const char*					getMainAppConfigName()					{ return NULL; }
	virtual bool						onRuntimeInit();
	virtual bool						onRuntimeStart();
	virtual bool						onRuntimeMainLoop();
	virtual int							onRuntimeFinish();

protected:								// wxApp Impl
	virtual bool						OnInit();
	virtual int							OnRun();
	virtual bool						ProcessIdle();
	virtual int							OnExit();

	int									_exitCode;
	NitRuntime*							_runtime;
	nit::Ref<AppConfig>					_mainAppConfig;
	Weak<AppBase>						_mainApp;
	wxFileSystemHandler*				_packFileSysHandler;
	bool								_processingIdle;

	void								debugCommand(const String& command);

public:
	class NITWX_API DefaultRuntime : public NitRuntime
	{
	public:
		DefaultRuntime(wxNitApp* app)											{ _app = app; }

	protected:
		virtual String					getTitle()								{ return _app->getTitle(); }
		virtual bool					onInit()								{ return _app->onRuntimeInit(); }
		virtual bool					onStart()								{ return _app->onRuntimeStart(); }
		virtual bool					onMainLoop()							{ return _app->onRuntimeMainLoop(); }
		virtual int						onFinish()								{ return _app->onRuntimeFinish(); }

		virtual void					debugCommand(const String& command)		{ if (_app) _app->debugCommand(command); }

		Weak<wxNitApp>					_app;
	};

protected:
	virtual NitRuntime*					createRuntime();
	virtual AppBase*					createMainApp()							{ return new AppBase(); }

public:
	static Session*						defaultSessionFactory(const String& name, const String& args);
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
