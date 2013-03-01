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

#include "nit/nit.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

// Hack of squirrel 3.0 regexp class without squirrel-VM for C++ usage

// WARN: Lock or ThreadLocal needed when use as a static one

/* PATTERN
**
**  \\         Quote the next metacharacter
**  ^          Match the beginning of the string
**  .          Match any character
**  $          Match the end of the string
**  |          Alternation
**  (subexp)   Grouping (creates a capture)
**  (?:subexp) No Capture Grouping (no capture)
**  []         Character class
**
**  GREEDY CLOSURES
**  *     Match 0 or more times
**  +     Match 1 or more times
**  ?     Match 1 or 0 times
**  {n}   Match exactly n times
**  {n,}  Match at least n times
**  {n,m} Match at least n but not more than m times
**
**  ESCAPE CHARACTERS
**  \\t    tab (HT, TAB)
**  \\n    newline (LF, NL)
**  \\r    return (CR)
**  \\f    form feed (FF)
**
**  PREDEFINED CLASSES
**  \\l    lowercase next char
**  \\u    uppercase next char
**  \\a    letters
**  \\A    non letters
**  \\w    alphanumeric [_0-9a-zA-Z]
**  \\W    non alphanumeric [^_0-9a-zA-Z]
**  \\s    space
**  \\S    non space
**  \\d    digits
**  \\D    non digits
**  \\x    hexadecimal digits
**  \\X    non hexadecimal digits
**  \\c    control characters
**  \\C    non control characters
**  \\p    punctation
**  \\P    non punctation
**  \\b    word boundary
**  \\B    non word boundary
*/

////////////////////////////////////////////////////////////////////////////////

class NIT_API RegExp : public DefaultAlloc
{
public:
	RegExp();
	RegExp(const String& pattern);
	RegExp(const char* pattern);
	~RegExp();

	RegExp& operator= (const String& pattern);
	RegExp& operator= (const char* pattern);

	static void							Help();

public:
	struct SubExp
	{
		const char*						begin;
		const char*						end;
		uint							length()								{ return end - begin; }
		operator						String()								{ return String(begin, end); }
	};

	uint								getSubExpCount();

public:

	bool								match(const String& str, uint start=0)	{ return match(str.c_str(), start); }
	bool								match(const char* str, uint start=0);

	bool								search(const String& str, size_t* outBegin=NULL, size_t* outEnd=NULL, uint start=0);
	bool								search(const char* str, const char** outBegin=NULL, const char** outEnd=NULL, uint start=0);

	String								capture(const String& str, uint subExpIndex=0, uint start=0);
	bool								capture(const char* str, SubExp& outResult, uint subExpIndex=0, uint start=0);
	bool								capture(const char* str, String& outResult, uint subExpIndex=0, uint start=0);
	bool								captureAll(const char* str, vector<SubExp>::type& outResults, uint start=0);
	bool								captureAll(const char* str, StringVector& outResults, uint start=0);

	String								substitute(const String& str, const String& replacement, uint subExpIndex=0, uint start=0);
	String								substitute(const String& str, const char* replacement, uint subExpIndex=0, uint start=0);
	String								substitute(const char* str, const char* replacement, uint subExpIndex=0, uint start=0);

	// Similar to printf in addition to following specs:
	// %* : whole str
	// %0 ~ %9 : captureed substring
	// %< : substring before the %0 range
	// %> : substring after the %0 range
	String								format(const String& str, const char* fmt, uint start=0);
	String								format(const char* str, const char* fmt, uint start=0);

private:
	void*								_peer;

	RegExp(const RegExp& other)													{ NIT_THROW(EX_NOT_SUPPORTED); }
	RegExp& operator= (const RegExp& other)										{ NIT_THROW(EX_NOT_SUPPORTED); }

	void*								compile(const char* pattern);
	SubExp								getSubExp(uint subExpIndex);
};

////////////////////////////////////////////////////////////////////////////////

inline String RegExp::substitute(const String& str, const String& replacement, uint subExpIndex, uint start)
{
	return substitute(str.c_str(), replacement.c_str(), subExpIndex, start);
}

inline String RegExp::substitute(const String& str, const char* replacement, uint subExpIndex, uint start)
{
	return substitute(str.c_str(), replacement, subExpIndex, start);
}

////////////////////////////////////////////////////////////////////////////////
NS_NIT_END;
