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

#include "nit/io/FileLocator.h"
#include "nit/net/Socket.h"

#include <Iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")

#include <shlobj.h>

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NitRuntime::NitRuntime()
{
}

bool NitRuntime::initPlatform()
{
#ifndef NIT_NO_LOG
	if (IsDebuggerPresent())
		LogManager::getSingleton().attach(new DebuggerLogger());

	if (GetConsoleWindow())
		LogManager::getSingleton().attach(new StdLogger(stdout, stderr));
#endif

#if defined(NIT_WIN32)
	_config->set("platform", "win32");
	// TODO: impl
	_config->set("device_model", "<unknown>");
	_config->set("device_form", "desktop");
	_config->set("os_version", "<unknown>");
#endif

	// obtain bundle path = program path
	if (!_arguments.empty())
	{
		String exeName;
		String appArchive;
		StringUtil::splitFilename(_arguments[0].c_str(), exeName, appArchive);
		FileUtil::normalizeSeparator(appArchive);
		_config->set("app_path", appArchive);
	}

	WCHAR path[MAX_PATH];

	HRESULT hr;

	// obtain library path
	hr = SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path);
	if (hr == S_OK)
	{
		String appDataPath = Unicode::toUtf8(path);
		FileUtil::normalizeSeparator(appDataPath);
		_config->set("app_data_path", appDataPath);
		_config->set("app_cache_path", appDataPath + "\\cache");
	}

	// obtain document path
	hr = SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, path);
	if (hr == S_OK)
	{
		String userDataPath = Unicode::toUtf8(path);
		FileUtil::normalizeSeparator(userDataPath);
		_config->set("user_data_path", userDataPath);
		_config->set("user_cache_path", userDataPath + "\\cache");
	}

	// obtain temp path
	if (::GetTempPathW(COUNT_OF(path), path))
	{
		String tempPath = Unicode::toUtf8(path);
		FileUtil::normalizeSeparator(tempPath);
		_config->set("sys_temp_path", tempPath);
	}

	return true;
}

void NitRuntime::finishPlatform()
{
}

void NitRuntime::updateEnv()
{
	// en-us as default on win32
	_config->set("language", "en");
	_config->set("country", "us");

	// TODO: Detect language code
}

void NitRuntime::updateNet()
{
	_ipAddrs.clear();

	if (!SocketBase::initialize())
	{
		LOG(0, "*** Can't initialize socket\n");
		return;
	}

	// retrieve hostname

	char hostname[40];
	if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR)
	{
		LOG(0, "*** Can't get local host name\n");
		return;
	}

	LOG(0, "++ Host name: '%s'\n", hostname);
	_config->set("host_name", hostname);
	_config->set("device_name", hostname);

	// retrieve ip addresses & identify their adapter
	IP_ADAPTER_INFO info[32];
	DWORD bufLen = sizeof(info);
	DWORD ret = GetAdaptersInfo(info, &bufLen);

	int otherCount = 0;
	int enCount = 0;
	int pppCount = 0;
	int loopbackCount = 0;
	int wifiCount = 0;

	for (PIP_ADAPTER_INFO adapter = info; adapter; adapter = adapter->Next)
	{
		for (PIP_ADDR_STRING iaddr = &adapter->IpAddressList; iaddr; iaddr = iaddr->Next)
		{
			String ipAddr = iaddr->IpAddress.String;
			if (ipAddr == "0.0.0.0") continue;
			String adapterCode;
			String macAddr;

			if (adapter->AddressLength == 6)
			{
				macAddr = StringUtil::format("%02X:%02X:%02X:%02X:%02X:%02X",
					adapter->Address[0], adapter->Address[1], adapter->Address[2], adapter->Address[3], adapter->Address[4], adapter->Address[5]);
			}

			switch (adapter->Type)
			{
			case MIB_IF_TYPE_OTHER:
				adapterCode = StringUtil::format("etc%d", otherCount++);
				break;

			case MIB_IF_TYPE_ETHERNET:
				adapterCode = StringUtil::format("en%d", enCount++);
				break;

			case MIB_IF_TYPE_PPP:
				adapterCode = StringUtil::format("ppp%d", pppCount++);
				break;

			case MIB_IF_TYPE_LOOPBACK:
				adapterCode = StringUtil::format("lo%d", loopbackCount++);
				break;

			case IF_TYPE_IEEE80211:
				adapterCode = StringUtil::format("wifi%d", wifiCount++);
				break;

			default:
				// ignore
				continue;
			}

			_ipAddrs.insert(std::make_pair(adapterCode.c_str(), ipAddr.c_str()));
			LOG(0, "++ %s (%s) = %s '%s'\n", macAddr.c_str(), adapterCode.c_str(), ipAddr.c_str(), adapter->Description);
		}
	}

	// set main adapter
	if (_ipAddrs.find("en0") != _ipAddrs.end())
		_ipAddrs["main"] = _ipAddrs["en0"];
	else if (_ipAddrs.find("wifi0") != _ipAddrs.end())
		_ipAddrs["main"] = _ipAddrs["wifi0"];

	// Simple method (without adapter info)
	if (false)
	{
		struct hostent* entry = gethostbyname(hostname);
		if (entry == 0)
		{
			LOG(0, "*** Bad local host name: %s\n", hostname);
			return;
		}

		for (int i=0; entry->h_addr_list[i] != 0; ++i)
		{
			struct in_addr addr;
			memcpy(&addr, entry->h_addr_list[i], sizeof(struct in_addr));
			String ipAddr = inet_ntoa(addr);
			LOG(0, "++ IP address[%d] %s\n", i, ipAddr.c_str());
		}
	}
}

void NitRuntime::info(const String& title, const String& message, bool userInfo)
{
	LOG(0, "++ %s: %s\n", title.c_str(), message.c_str());
	if (userInfo || _infoEnabled)
		MessageBoxW(NULL, Unicode::toUtf16(message).c_str(), Unicode::toUtf16(title).c_str(), MB_ICONINFORMATION);
}

void NitRuntime::alert(const String& title, const String& message, bool fatal)
{
	LOG(0, "!!! %s: %s\n", title.c_str(), message.c_str());
	if (fatal || _alertEnabled)
		MessageBoxW(NULL, Unicode::toUtf16(message).c_str(), Unicode::toUtf16(title).c_str(), MB_ICONERROR);
}

bool NitRuntime::onSystemLoop()
{
	Thread::sleep(1);

	MSG msg;
	while (PeekMessageA(&msg, NULL, 0U, 0U, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
		{
			LOG(0, "*** WM_QUIT requested by system\n");
			return false;
		}
	}
	return true;
}

int NitRuntime::execute(const String& cmdline, Ref<StreamReader> in, Ref<StreamWriter> out, Ref<StreamWriter> err)
{
	// TODO: Consider "cmd /u /c cmdline" instead

	HANDLE childStdInReader = INVALID_HANDLE_VALUE;
	HANDLE childStdInWriter = INVALID_HANDLE_VALUE;
	HANDLE childStdOutReader = INVALID_HANDLE_VALUE;
	HANDLE childStdOutWriter = INVALID_HANDLE_VALUE;
	HANDLE childStdErrReader = INVALID_HANDLE_VALUE;
	HANDLE childStdErrWriter = INVALID_HANDLE_VALUE;

	SECURITY_ATTRIBUTES saAttr = { 0 };

	saAttr.nLength = sizeof(saAttr);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	DWORD exitCode = -1;

	PROCESS_INFORMATION procInfo = { 0 };
	procInfo.hProcess = INVALID_HANDLE_VALUE;
	procInfo.hThread = INVALID_HANDLE_VALUE;

	try
	{
		// setup pipes

		if (!CreatePipe(&childStdInReader, &childStdInWriter, &saAttr, 0))
			NIT_THROW_FMT(EX_SYSTEM, "can't create child stdin pipe");

		if (!SetHandleInformation(childStdInWriter, HANDLE_FLAG_INHERIT, 0))
			NIT_THROW_FMT(EX_SYSTEM, "can't set handle information to child stdin");

		if (!CreatePipe(&childStdOutReader, &childStdOutWriter, &saAttr, 0))
			NIT_THROW_FMT(EX_SYSTEM, "can't create child stdout pipe");

		if (!SetHandleInformation(childStdOutReader, HANDLE_FLAG_INHERIT, 0))
			NIT_THROW_FMT(EX_SYSTEM, "can't set handle information to child stdout");

		if (!CreatePipe(&childStdErrReader, &childStdErrWriter, &saAttr, 0))
			NIT_THROW_FMT(EX_SYSTEM, "can't create child stderr pipe");

		if (!SetHandleInformation(childStdErrReader, HANDLE_FLAG_INHERIT, 0))
			NIT_THROW_FMT(EX_SYSTEM, "can't set handle information to child stderr");

		// create child process

		STARTUPINFO startupInfo = { 0 };

		startupInfo.cb			= sizeof(startupInfo);
		startupInfo.hStdError	= childStdErrWriter;
		startupInfo.hStdOutput	= childStdOutWriter;
		startupInfo.hStdInput	= childStdInReader;
		startupInfo.dwFlags		|= STARTF_USESTDHANDLES;

		DWORD crFlags = 0;
		crFlags |= CREATE_NO_WINDOW; // Prevent command console window to appear

		BOOL ok = CreateProcess(NULL,
			(LPWSTR)Unicode::toUtf16(cmdline).c_str(),
			NULL,			// process security attributes
			NULL,			// primary thread security attributes
			TRUE,			// handles are inherited
			crFlags,		// creation flags
			NULL,			// use parent's environment
			NULL,			// use parent's current directory
			&startupInfo,
			&procInfo );

		if (!ok)
			NIT_THROW_FMT(EX_SYSTEM, "can't create child process");

		// We should close the handles sent to the child unless it'll loop forever below ReadFile()
		CloseHandle(childStdInReader);	
		childStdInReader = INVALID_HANDLE_VALUE;

		CloseHandle(childStdOutWriter); 
		childStdOutWriter = INVALID_HANDLE_VALUE;

		CloseHandle(childStdErrWriter);
		childStdErrWriter = INVALID_HANDLE_VALUE;

		if (in == NULL)
		{
			// Close std-in handle of the child if no 'in' stream provided
			CloseHandle(childStdInWriter);
			childStdInWriter = INVALID_HANDLE_VALUE;
		}

		char inBuf[4096];
		char outBuf[4096];
		size_t remainingInBytes = 0;
		size_t remainingOffset = 0;

		while (true)
		{
			DWORD bytesRead, bytesWritten;

			// Send data to std-in of the child process
			if (in && childStdInWriter != INVALID_HANDLE_VALUE)
			{
				while (true)
				{
					// If there're previous remaining bytes, process them first
					if (remainingInBytes)
					{
						ok = WriteFile(childStdInWriter, &inBuf[remainingOffset], remainingInBytes, &bytesWritten, NULL);
						
						if (!ok)
							NIT_THROW_FMT(EX_SYSTEM, "can't write to child in");

						remainingInBytes -= bytesWritten;
						if (remainingInBytes > 0)
						{
							remainingOffset += bytesWritten;
							continue;
						}

						remainingOffset = 0;
					}

					// Fill buffer from 'in' stream
					bytesRead = in->readRaw(inBuf, sizeof(inBuf));
					if (bytesRead)
						ok = WriteFile(childStdInWriter, inBuf, bytesRead, &bytesWritten, NULL);

					if (!ok)
						NIT_THROW_FMT(EX_SYSTEM, "can't write to child in");

					if (bytesRead == 0 && in->isEof())
					{
						// Close the handle of exhausted stream
						// It's like that sends a ctrl-z to the child process.
						CloseHandle(childStdInWriter);
						childStdInWriter = INVALID_HANDLE_VALUE;
						break;
					}

					// If there're remaining bytes, handle them at next loop
					remainingInBytes = bytesRead - bytesWritten;
					if (remainingInBytes > 0)
					{
						remainingOffset = bytesWritten;
						continue;
					}
				}
			}

			// Receive child's std-out (blocking-operation)
			if (out && childStdOutReader != INVALID_HANDLE_VALUE)
			{
				while (true)
				{
					ok = ReadFile(childStdOutReader, outBuf, sizeof(outBuf), &bytesRead, NULL);

					if (!ok || bytesRead == 0) break;

					bytesWritten = out->writeRaw(outBuf, bytesRead);

					if (bytesWritten == 0)
						NIT_THROW_FMT(EX_WRITE, "can't write to out");
				}
			}

			// Receive child's std-err (blocking-operation)
			if (err && childStdErrReader != INVALID_HANDLE_VALUE)
			{
				while (true)
				{
					ok = ReadFile(childStdErrReader, outBuf, sizeof(outBuf), &bytesRead, NULL);

					if (!ok || bytesRead == 0) break;

					bytesWritten = err->writeRaw(outBuf, bytesRead);

					if (bytesWritten == 0)
						NIT_THROW_FMT(EX_WRITE, "can't write to err");
				}
			}

			// Get the exit code of the child process
			ok = GetExitCodeProcess(procInfo.hProcess, &exitCode);

			if (!ok)
				NIT_THROW_FMT(EX_SYSTEM, "can't get exit code from child");

			if (exitCode != STILL_ACTIVE)
				break;

			// The child process is still running while STILL_ACTIVE
			// Need some sleep to cool down CPU usage
			Thread::sleep(1);
		}

		// Release the hand of the thread and the process
		if (procInfo.hProcess != INVALID_HANDLE_VALUE) CloseHandle(procInfo.hProcess);
		if (procInfo.hThread != INVALID_HANDLE_VALUE) CloseHandle(procInfo.hThread);

		if (childStdInReader != INVALID_HANDLE_VALUE) CloseHandle(childStdInReader);
		if (childStdInWriter != INVALID_HANDLE_VALUE) CloseHandle(childStdInWriter);
		if (childStdOutReader != INVALID_HANDLE_VALUE) CloseHandle(childStdOutReader);
		if (childStdOutWriter != INVALID_HANDLE_VALUE) CloseHandle(childStdOutWriter);
		if (childStdErrReader != INVALID_HANDLE_VALUE) CloseHandle(childStdErrReader);
		if (childStdErrWriter != INVALID_HANDLE_VALUE) CloseHandle(childStdErrWriter);
	}
	catch (...)
	{
		if (procInfo.hProcess != INVALID_HANDLE_VALUE) 
		{
			// Forcibly terminate child if any exception
			BOOL ok = TerminateProcess(procInfo.hProcess, 0);
			CloseHandle(procInfo.hProcess);
		}

		if (procInfo.hThread != INVALID_HANDLE_VALUE) CloseHandle(procInfo.hThread);

		if (childStdInReader != INVALID_HANDLE_VALUE) CloseHandle(childStdInReader);
		if (childStdInWriter != INVALID_HANDLE_VALUE) CloseHandle(childStdInWriter);
		if (childStdOutReader != INVALID_HANDLE_VALUE) CloseHandle(childStdOutReader);
		if (childStdOutWriter != INVALID_HANDLE_VALUE) CloseHandle(childStdOutWriter);
		if (childStdErrReader != INVALID_HANDLE_VALUE) CloseHandle(childStdErrReader);
		if (childStdErrWriter != INVALID_HANDLE_VALUE) CloseHandle(childStdErrWriter);

		throw;
	}

	return exitCode;
}


////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;