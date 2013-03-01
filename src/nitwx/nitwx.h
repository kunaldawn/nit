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

#ifdef _WIN32
#	define NOMINMAX
#	include <winsock2.h> // should include this prior to windows.h or wx.h
#	include <Windows.h>
#endif

#ifdef _WIN32
#   pragma warning (push)
#   pragma warning (disable: 4996)
#endif

#include <wx/wx.h>

#ifdef _WIN32
#   pragma warning (pop)
#endif

#include "nit/nit.h"

////////////////////////////////////////////////////////////////////////////////

// Determine DLL Spec

#if defined(NIT_DLL)
#	if defined(NITWX_BUILD_DLL)
#		define NITWX_API NIT_DLL_EXPORT
#	else
#		define NITWX_API NIT_DLL_IMPORT
#	endif
#else
#	define NITWX_API
#endif

////////////////////////////////////////////////////////////////////////////////

// TODO: Move to a document file

/// NOTE:
/// 
/// To use NitWx, setup as following: (on win32. It's possible on mac32 also but need some document about it)
/// 
/// 1. Clone wxWidgets for nit
/// 1-1. git clone git://github.com/ellongrey/wxWidgets.git <myproj>/wx
/// 
/// 2. Build <myproj>/wx/build/msw/wx_vc9.sln
/// 2-1. Build both 'DLL Debug', 'DLL Release' configuration. (static builds are not necessary)
/// 2-2. Check if vc_dll folder and *.dll generated at <myproj>/wx/lib
/// 
/// 3. Setup Visual Studio path
/// 3-1. Navigate VS menu: Tools -> Options -> Projects & Solutions -> VC++ Directories
/// 3-2. included files : add <myproj>/wx/lib/vc_dll/mswu
/// 3-3. library files  : add <myproj>/wx/lib/vc_dll
/// 
/// 4. Setup PATH environment variable
/// 4-1. Right click on 'My computer' -> Properties -> Advanced -> Environment variables -> 
///      System variables -> add a path : <myproj>/wx/lib/vc_dll (prefix ';' if necessary)
/// 4-2. If Visual Studio is running, restart Visual Studio. (To reflect environment change)
///      
/// 5. wx setup completed : Once setup you can build any project on same computer as its system-wide setup.
///
/// 6. wxFormBuilder for nit build (optional)
/// 6-1. git clone git://github.com/ellongrey/wxfb.git <myproj>/wxfb
/// 6-2. Build <myproj>/wx/wxfb.sln (I suggest a release build)
/// 6-3. Associate *.fbp file to the executable
/// 
/// When you distribute your nit-dev tool app using wx you should distribute wx's dll files
/// in <myproj>/wx/lib/vc_dll also.
/// For release build, just distribute *u*.dll,
/// For debug build, distribute *ud*.dll also.

#include "nit/nit.h"
