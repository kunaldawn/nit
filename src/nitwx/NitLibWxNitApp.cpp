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

#include "nitwx_pch.h"

#include "nitwx/NitLibWxNitApp.h"
#include "nitwx/NitLibWx.h"

#include "nit/app/Session.h"
#include "nit/app/Package.h"

#ifdef NIT_WIN32
// HACK: for GetAdaptersInfo()
#   include <Iphlpapi.h>
#   pragma comment(lib, "IPHLPAPI.lib")
#endif

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

// location format: <packname>#pack:<streamname>

class wxPackageFileSystemHandler : public wxFileSystemHandler
{
public:
	virtual bool CanOpen(const wxString& location)
	{
		return GetProtocol(location) == wxT("pack");
	}

	virtual wxFSFile* OpenFile(wxFileSystem& fs, const wxString& location)
	{
		wxString packname = GetLeftLocation(location);
		wxString streamName = GetRightLocation(location);

		Package* pack = g_Package->link(packname.utf8_str());
		if (pack == NULL) return NULL;

		StreamSource* source = pack->locate(streamName.utf8_str().data(), false);
		if (source == NULL) return NULL;

		wxNitInputStream* inputStream = new wxNitInputStream(source->open());

		return new wxFSFile(
			inputStream, 
			location, 
			source->getContentType().getMimeType().c_str(), 
			GetAnchor(location), 
			wxDateTime((time_t)source->getTimestamp().getUnixTime64())
			);
	}
};

////////////////////////////////////////////////////////////////////////////////

wxNitApp::wxNitApp()
{
	_exitCode		= 0;
	_runtime		= NULL;
	_mainAppConfig = NULL;

	_packFileSysHandler = NULL;

	_processingIdle = false;
}

wxNitApp::~wxNitApp()
{
	if (_runtime)
		_runtime->finish();

	if (_mainApp)
		delete _mainApp;

	safeDelete(_runtime);
}

bool wxNitApp::OnInit()
{
	// NOTE: We don't use original wxApp::OnInit() code for wxCmdLine.
	// Use nit::CmdLineParser instead.

#ifdef NIT_WIN32
	// HACK: The exit code will not be returned without this code
	// It seems that IPHlpAPI is not initialized property so initialize it here by calling a function.
	if (true)
	{
		IP_ADAPTER_INFO info[32];
		DWORD bufLen = sizeof(info);
		DWORD ret = GetAdaptersInfo(info, &bufLen);
	}
#endif

	// Intialize all image handlers manually at startup
	wxInitAllImageHandlers();

	// Install wxPackageFileSystemHandler
	_packFileSysHandler = new wxPackageFileSystemHandler();
	wxFileSystem::AddHandler(_packFileSysHandler);

	// Prevent to exit when top-level frame destroyed.
	SetExitOnFrameDelete(false);

	_runtime = createRuntime();

	if (_runtime == NULL)
		return false;

	return _runtime->init();
}

String wxNitApp::getTitle()
{
	if (_mainApp && _mainApp->getConfigs())
		return _mainApp->getConfigs()->getTitle();
	if (_mainAppConfig)
		return _mainAppConfig->getTitle();
	else
		return "WxNitApp::Runtime";
}

bool wxNitApp::onRuntimeInit()
{
#ifdef NIT_WIN32
	TCHAR buf[MAX_PATH];
	GetModuleFileNameW(NULL, buf, COUNT_OF(buf));

	String exePath = Unicode::toUtf8(buf);
#elif defined(NIT_MAC32)
    String exePath;
    
    bool useCurrPath = true;
    
    if (useCurrPath)
    {
        char resolved_path[PATH_MAX];
        if (realpath(getprogname(), resolved_path) == 0)
        {
            exePath = resolved_path;
        }
    }
    else
    {
        exePath = NitRuntime::getExecutablePath();
    }
#endif

	const char* appConfigName = getMainAppConfigName();

	if (appConfigName == NULL)
		return false;

	Settings* settings = AppConfig::locate(exePath.c_str(), getMainAppConfigName());

	_mainAppConfig = new AppConfig(settings);

	_mainAppConfig->getSettings()->set("async_loading", "false"); // TODO: Conflict with DLL plug-in. Disable for now.

	_mainApp = createMainApp();

	// Startup the app
	AppBase::ScopedEnter sc(_mainApp);

	_mainApp->init(_mainAppConfig);

	return true;
}

extern void* RegisterStaticNitWxModule();

bool wxNitApp::onRuntimeStart()
{
	AppBase::ScopedEnter sc(_mainApp);
	_mainApp->start();

	// Register NitLibWxControl module
	RegisterStaticNitWxModule();

	return true;
}

bool wxNitApp::onRuntimeMainLoop()
{
	if (_mainApp == NULL)
		return false;

	AppBase::ScopedEnter sc(_mainApp);

	if (_mainApp->isStopping())
		return false;

	_mainApp->loop();

	return !_mainApp->isStopping();
}

bool wxNitApp::ProcessIdle()
{
	Thread::sleep(1);

	// Re-entry occurs at ProcessIdle() when popup dialog etc.
	// To handle re-entry, pause runtime a while and focus on the processiong of the dialog
	// OnIdle() function etc will run at this time.

	// TODO: Script thread or event handler should run even so!

	if (_processingIdle) 
		return wxApp::ProcessIdle();

	_processingIdle = true;

	if (_runtime)
	{
		if (_runtime->isFinished())
			this->Exit();
		else
			_runtime->mainLoop();

		WakeUpIdle();
	}
	else
	{
		this->Exit();
	}

	_processingIdle = false;

	return wxApp::ProcessIdle();
}

int wxNitApp::OnExit()
{
	if (_runtime)
		_runtime->finish();

	if (_mainApp)
		delete _mainApp;

	safeDelete(_runtime);

	if (_packFileSysHandler)
	{
		wxFileSystem::RemoveHandler(_packFileSysHandler);
		safeDelete(_packFileSysHandler);
	}

	return wxApp::OnExit();
}

int wxNitApp::onRuntimeFinish()
{
	AppBase::ScopedEnter sc(_mainApp);

	if (_exitCode == 0 && _mainApp)
		_exitCode = _mainApp->getConfigs()->getExitCode();

	if (_mainApp)
		_mainApp->finish();

	return _exitCode;
}

Session* wxNitApp::defaultSessionFactory(const String& name, const String& args)
{
	return new Session();
}

void wxNitApp::debugCommand(const String& command)
{
	AppBase::ScopedEnter sc(_mainApp);

	if (g_Session)
	{
		g_Session->getScript()->command(command);
	}
}

int wxNitApp::OnRun()
{
	AppBase::ScopedEnter sc(_mainApp);

	wxApp::OnRun();
	return _exitCode;
}

NitRuntime* wxNitApp::createRuntime()
{
	NitRuntime* rt = new DefaultRuntime(this);
    rt->setArguments(this->argc, this->argv);

	return rt;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
