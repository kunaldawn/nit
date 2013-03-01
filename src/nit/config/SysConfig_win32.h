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

////////////////////////////////////////////////////////////////////////////////

#define NOMINMAX
#include <winsock2.h> // should include this prior to windows.h
#include <Windows.h>

////////////////////////////////////////////////////////////////////////////////

// disable: "<type> needs to have dll-interface to be used by clients'
// Happens on STL member variables which are not public therefore is ok
#pragma warning (disable : 4251)

// disable: '<function>' This function or variable may be unsafe.
#pragma warning (disable : 4996)

// disable: decorated name length exceeded
#pragma warning (disable : 4503)

////////////////////////////////////////////////////////////////////////////////

#define NIT_DLL_EXPORT					__declspec(dllexport)
#define NIT_DLL_IMPORT					__declspec(dllimport)

////////////////////////////////////////////////////////////////////////////////

#define NIT_FILE_HANDLE					HANDLE

////////////////////////////////////////////////////////////////////////////////
