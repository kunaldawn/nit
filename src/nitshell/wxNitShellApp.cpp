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

#include "nitwx/nitwx.h"

#include "nitwx/NitLibWxNitApp.h"

#include "nit/data/ParserUtil.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class wxNitShellApp : public wxNitApp
{
public:
	wxNitShellApp();
	virtual ~wxNitShellApp();

protected:
	NitRuntime*							createRuntime();

	virtual const char*					getMainAppConfigName();
	
	virtual bool						onRuntimeInit();
	virtual bool						onRuntimeStart();
	virtual int							OnExit();

	// This is set before NitRuntime intiailzed so do not nit::string or nit::vector
	std::string							_appConfigFilename;
	std::vector<std::string>			_packPaths;

	nit::Ref<Settings>					_shellConfig;

	class ShellRuntime : public DefaultRuntime
	{
	public:
		ShellRuntime(wxNitShellApp* shellApp);
	};
};

////////////////////////////////////////////////////////////////////////////////

wxNitShellApp::wxNitShellApp()
{

}

wxNitShellApp::~wxNitShellApp()
{

}

NitRuntime* wxNitShellApp::createRuntime()
{
    NitRuntime* rt = new DefaultRuntime(this);
    rt->setArguments(this->argc, this->argv);

	// NOTE: rt may be destroyed within this function.
	// So do not use nit::string or RefCounted which is related MemManager

	std::string error;

	_shellConfig = new Settings();
	nit::Ref<Settings> cs = new Settings();
	nit::Ref<CmdLineParser> parser = new CmdLineParser();

	LogManager::getSingleton().setDefaultLogLevel(LOG_LEVEL_ERROR);

	try
	{
		parser->allowUnknownOptions(true);
		parser->allowUnknownParams(true);

		_shellConfig->add("switch", "[-v verbose]			: full verbosity (overrides log_filter)");
		_shellConfig->add("switch", "[-q quiet]			: quiet mode (overrides log_filter, verbose)");
		_shellConfig->add("option", "[-w log_filter]		: show log level equal or higher than this (default: ***)");
		_shellConfig->add("option", "[-p pack_path ...]	: path to pack path (can use $(macro) in app.Settings), relative to work_path");

		parser->addParam("app.cfg", "path to app.cfg");

		parser->addSection(_shellConfig);

		parser->parse(cs, rt, true);
	}
	catch (Exception& ex)
	{
		error = ex.getDescription().c_str();
	}

	if (error.empty() && !cs->has("app.cfg"))
	{
		error = "app.cfg path expected";
	}

	cs->dump();

	if (!error.empty())
	{
		const char* appname = "nit.exe";

		const char* logo =
			"nit-shell: command line Noriter shell v1.0\n"
			"nit: Noriter game-oriented app framework v1.0\n"
			"Copyright (c) 2013 Jun-hyeok Jang.";

		parser->showUsage(appname, logo, String("*** " ) + error.c_str());

		// Release references before delete Mem Manager
		_shellConfig		= NULL;
		cs					= NULL;
		parser				= NULL;

		safeDelete(rt);

		_exitCode = -1;

		return NULL;
	}

	LogManager& logMgr = LogManager::getSingleton();

	if (cs->has("log_filter"))
	{
		const String& filter = _shellConfig->get("log_filter");
		LogLevel lvl = logMgr.getLogLevel(logMgr.tagId(filter.c_str()));
		logMgr.setDefaultLogLevel(lvl);
	}

	if (cs->get("verbose", "false") == "true")
		logMgr.setDefaultLogLevel(LOG_LEVEL_VERBOSE);

	if (cs->get("quiet", "false") == "true")
		logMgr.setDefaultLogLevel(LOG_LEVEL_QUIET);

	StringVector paths;
	cs->find("pack_path", paths);
	for (uint i=0; i<paths.size(); ++i)
		_packPaths.push_back(paths[i].c_str());

	_appConfigFilename = cs->get("app.cfg").c_str();

	if (!StringUtil::endsWith(_appConfigFilename.c_str(), ".app.cfg"))
		_appConfigFilename += ".app.cfg";

	return rt;
}

const char* wxNitShellApp::getMainAppConfigName()
{
	return _appConfigFilename.empty() ? NULL : (const char*)_appConfigFilename.c_str();
}

bool wxNitShellApp::onRuntimeInit()
{
	if (!wxNitApp::onRuntimeInit())
		return false;

	// Set the base of shell to app.cfg
	nit::Ref<Settings> shell = _mainAppConfig->getSettings()->getSection("shell");
	if (shell)
		shell->setBase(_shellConfig);	// the base is weak, it must be retained here
	else
		_shellConfig = NULL;			// no shell config found, so release

	return true;
}

bool wxNitShellApp::onRuntimeStart()
{
	if (!wxNitApp::onRuntimeStart())
		return false;

	AppBase::ScopedEnter sc(_mainApp);

	nit::Ref<FileLocator> workDir = new FileLocator("$work", _mainApp->getConfig("win32/work_path", "."));

	for (uint i=0; i<_packPaths.size(); ++i)
	{
		String path = _mainAppConfig->getSettings()->expand(_packPaths[i].c_str(), true);

		if (FileUtil::isAbsolutePath(path))
			g_Package->getBundle()->getLocator()->require(new FileLocator("$pack_path", path));
		else
			g_Package->getBundle()->getLocator()->require(new FileLocator("$pack_path", workDir->makeUrl(path)));
	}

	return true;
}

int wxNitShellApp::OnExit()
{
	_shellConfig = NULL;

	return wxNitApp::OnExit();
}

////////////////////////////////////////////////////////////////////////////////

wxNitShellApp::ShellRuntime::ShellRuntime(wxNitShellApp* shellApp)
: DefaultRuntime(shellApp)
{
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;

wxDECLARE_APP(nit::wxNitShellApp);

#ifdef _CONSOLE_APP
wxIMPLEMENT_APP_CONSOLE(nit::wxNitShellApp);
#else
wxIMPLEMENT_APP(nit::wxNitShellApp);
#endif

////////////////////////////////////////////////////////////////////////////////
