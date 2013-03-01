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

void PluginDLL::load()
{
	if (isStatic()) return;

	if (_plugin) return;

	// Mostly ~EventInfo will be called on DLL's unload,
	// But there are exceptions so we added forced removal mechanism.
	vector<EventInfo*>::type eis;
	EventInfo::_DllLoaderSetRegisterCallback(onEventInfoRegister, &eis);

	_dllHandle = (NIT_DLL_HANDLE)NIT_DLL_LOAD(Unicode::toUtf16(_path).c_str());

	EventInfo::_DllLoaderSetRegisterCallback(NULL, NULL);

	if (!_dllHandle)
	{
		_dllHandle = 0;
		NIT_THROW_FMT(EX_INTERNAL, "Can't load dll '%s': %s", _path.c_str(), dllError().c_str());
	}

	LOG(0, ".. Plugin DLL '%s' loaded\n", _path.c_str());

	DllCreatePluginFunction pFunc = (DllCreatePluginFunction)
		NIT_DLL_GETSYM(_dllHandle, NIT_DLL_PLUGIN_FNNAME);

	if (pFunc == NULL)
	{
		NIT_THROW_FMT(EX_NOT_FOUND, "Can't find %s() in '%s'", NIT_DLL_PLUGIN_FNNAME, _path.c_str());
	}

	_plugin = pFunc();

	if (_plugin == NULL)
	{
		NIT_THROW_FMT(EX_INTERNAL, "Can't create plugin from '%s'", _path.c_str());
	}

	_plugin->_myEventInfos = eis;
}

void PluginDLL::unload()
{
	if (isStatic()) return;

	if (_plugin)
	{
		_plugin->dllUninstall();
		_plugin = NULL;
	}

	if (_dllHandle)
	{
		if (NIT_DLL_UNLOAD(_dllHandle))
		{
			NIT_THROW_FMT(EX_INTERNAL, "Can't unload '%s'", _path.c_str());
		}

		LOG(0, ".. Plugin DLL '%s' unloaded\n", _path.c_str());
		_dllHandle = 0;
	}
}

String PluginDLL::dllError()
{
	LPVOID lpMsgBuf; 
	FormatMessageW( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS, 
		NULL, 
		GetLastError(), 
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
		(LPTSTR) &lpMsgBuf, 
		0, 
		NULL 
		); 
	String ret = Unicode::toUtf8((UniChar*)lpMsgBuf);
	// Free the buffer.
	LocalFree( lpMsgBuf );
	return ret;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
