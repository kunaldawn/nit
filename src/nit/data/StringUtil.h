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

#include "nit/data/NitString.h"

////////////////////////////////////////////////////////////////////////////////

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

#if defined(NIT_WIN32)
// On windows, wchar_t counts 2-bytes. We may use uint16 instead of wchar_t but this causes head-aches so use it as is.
typedef wchar_t UniChar;
#else
// On other platforms (especially nix clones), wchar_t counts 4-bytes, so replace them as 2-bytes wide string.
typedef uint16 UniChar;
#endif

typedef std::basic_string<UniChar, std::char_traits<UniChar>, STLAllocator<UniChar, DefaultAllocPolicy > >	_UniStringBase;

typedef _UniStringBase					UniString;

class NIT_API Unicode
{
public:
	// Platform independant code pages (according to win32 code page)
	enum CodePage
	{
		CODE_DEFAULT					= -1,
		CODE_ANSI						= 0,		// CP_ACP
		CODE_SYSTEM						= 1,		// CP_OEMCP

		CODE_UTF8						= 65001,

		CODE_UTF16LE					= 1200,		// little endian
		CODE_UTF16BE					= 1201,		// big endian

		CODE_UTF16						= CODE_UTF16LE,

		// TODO: Include more code page. (if you need other code pages on windows, just cast to CodePage enum (ex: CodePage(949))
	};

	static const uint32 UNI_REPLACEMENT_CHAR	= 0xFFFD;

	static const uint32 UNI_SUR_HIGH_START		= 0xD800;
	static const uint32 UNI_SUR_HIGH_END		= 0xDBFF;
	static const uint32 UNI_SUR_LOW_START		= 0xDC00;
	static const uint32 UNI_SUR_LOW_END			= 0xDFFF;
	static const uint32 UNI_MAX_LEGAL_UTF32		= 0x10FFFF;


public:									// Unicode conversion routines
	static String						toUtf8(const UniChar* utf16str, int len = 0);
	static String						toUtf8(const UniString& utf16str)		{ return toUtf8(utf16str.c_str(), utf16str.length()); }

	static UniString					toUtf16(const char* utf8str, int len = 0, CodePage srcCodePage = CODE_DEFAULT);
	static UniString					toUtf16(const String& str, CodePage srcCodePage = CODE_DEFAULT) { return toUtf16(str.c_str(), str.length(), srcCodePage); }

	static String						reencode(const char* srcStr, CodePage srcCodePage, CodePage destCodePage, int len = 0);
	static String						reencode(const String& srcStr, CodePage srcCodePage, CodePage destCodePage) { return reencode(srcStr.c_str(), srcCodePage, destCodePage, srcStr.length()); }

	static int							toUniChar(const char* utf8Char);
	inline static int					uniCharAt(const char* utf8str, int pos)	{ return toUniChar(utf8str + utf8ByteCount(utf8str, pos)); }

	static size_t						toUtf8Char(uint32 uniChar, char outUtf8Char[4]);

	static uint32						getUniReplacementChar()					{ return UNI_REPLACEMENT_CHAR; }
	static void							setUniReplacementChar(uint32 ch);

public:									// UTF8 utility
	static size_t						utf8Length(const char* utf8);
	static size_t						utf8CharCount(const char* start, const char* end);
	static int							utf8ByteCount(const char* utf8, int charCount);
	static int							utf8Advance(const char*& utf8);
	static const char*					utf8Next(const char* utf8);
	static const char*					utf8Next(const char* utf8, const char* end, size_t count);
	static const char*					utf8Prev(const char* utf8);
	static const char*					utf8Prev(const char* utf8, const char* start);
	static const char*					utf8Prev(const char* utf8, const char* start, size_t count);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API Wildcard
{
public:
	static bool							has(const char* pattern);

	inline static bool					has(const String& pattern)				{ return has(pattern.c_str()); }

public:
	static bool							match(const char* pattern, const char* test, bool ignoreCase = true);

	inline static bool					match(const String& pattern, const char* test, bool ignoreCase = true)		{ return match(pattern.c_str(), test, ignoreCase); }
	inline static bool					match(const char* pattern, const String& test, bool ignoreCase = true)		{ return match(pattern, test.c_str(), ignoreCase); }
	inline static bool					match(const String& pattern, const String& test, bool ignoreCase = true)	{ return match(pattern.c_str(), test, ignoreCase); }
};

NS_NIT_END;

////////////////////////////////////////////////////////////////////////////////
