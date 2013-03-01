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

#include "nit/data/RegExp.h"
#include "nit/runtime/MemManager.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

// HACK: Just compile a single sqstdrex.cpp file with nor its context neither dll/lib setting

// TODO: Find alternative (origami of ruby?)

// prevent include
#define _SQPCHEADER_H_
#define _SQUIRREL_H_
#define _SQSTD_STRING_H_

// Minimal necessary declarations
#ifndef MAX_CHAR
#	define MAX_CHAR 0xFFFF
#endif

#define SQTrue	(1)
#define SQFalse	(0)
#define scstrlen strlen

#ifndef _SC
#	define _SC(x) x
#endif

typedef int SQInteger;
typedef unsigned int SQUnsignedInteger;
typedef char SQChar;
typedef bool SQBool;

typedef struct { const SQChar* begin; SQInteger len; } SQRexMatch;
struct SQRex;
static void sqstd_rex_free(SQRex* exp);

static void *sq_malloc(SQUnsignedInteger size)	
{ 
	return NIT_ALLOC(size); 
}

static void *sq_realloc(void* p,SQUnsignedInteger oldsize,SQUnsignedInteger newsize)
{
	return NIT_REALLOC(p,oldsize,newsize);
}

static void sq_free(void *p,SQUnsignedInteger size)
{
	NIT_DEALLOC(p,size);
}

#include "squirrel/sqstdrex.cpp"

////////////////////////////////////////////////////////////////////////////////

RegExp::RegExp()
: _peer(NULL)
{
}

RegExp::RegExp(const char* pattern)
: _peer(NULL)
{
	_peer = compile(pattern);
}

RegExp::RegExp(const String& pattern)
: _peer(NULL)
{
	_peer = compile(pattern.c_str());
}

RegExp& RegExp::operator=(const char* pattern)
{
	void* peer = compile(pattern);

	if (_peer)
		sqstd_rex_free((SQRex*)_peer);
	_peer = peer;

	return *this;
}

RegExp& RegExp::operator=(const String& pattern)
{
	void* peer = compile(pattern.c_str());

	if (_peer)
		sqstd_rex_free((SQRex*)_peer);
	_peer = peer;

	return *this;
}

RegExp::~RegExp()
{
	if (_peer)
		sqstd_rex_free((SQRex*)_peer);
}

void* RegExp::compile(const char* pattern)
{
	void* peer = NULL;

	const SQChar* error = NULL;
	if (pattern == NULL || *pattern == 0)
		error = "empty regexp pattern";
	else
		peer = sqstd_rex_compile(pattern, &error);

	if (peer == NULL)
		NIT_THROW_FMT(EX_SYNTAX, "regexp '%s' error: %s", pattern ? pattern : "", error);

	return peer;
}

uint RegExp::getSubExpCount()
{
	return _peer ? sqstd_rex_getsubexpcount((SQRex*)_peer) : 0;
}

bool RegExp::match(const char* str, uint start)
{
	if (_peer == NULL) return false;

	return sqstd_rex_match((SQRex*)_peer, str+start);
}

bool RegExp::search(const char* str, const char** outBegin, const char** outEnd, uint start)
{
	if (_peer == NULL) return false;

	bool found = sqstd_rex_search((SQRex*)_peer, str+start, outBegin, outEnd);
	return found;
}

bool RegExp::search(const String& str, size_t* outBegin, size_t* outEnd, uint start)
{
	if (_peer == NULL) return false;

	const char *cstr, *begin, *end;

	cstr = str.c_str();
	bool found = sqstd_rex_search((SQRex*)_peer, cstr+start, &begin, &end);

	if (found)
	{
		if (outBegin) *outBegin = begin - cstr;
		if (outEnd) *outEnd = end - cstr;
	}
	return found;
}

bool RegExp::capture(const char* str, String& outResult, uint subExpIndex, uint start)
{
	if (_peer == NULL) return false;
	ASSERT_THROW(subExpIndex == 0 || subExpIndex < getSubExpCount(), EX_INVALID_RANGE);

	SQRex* rex = (SQRex*)_peer;
	bool found = sqstd_rex_search(rex, str+start, NULL, NULL);
	if (!found) return false;

	SQRexMatch match;
	sqstd_rex_getsubexp(rex, subExpIndex, &match);
	outResult.assign(match.begin, match.begin + match.len);

	return true;
}

bool RegExp::capture(const char* str, SubExp& outResult, uint subExpIndex, uint start)
{
	if (_peer == NULL) return false;
	ASSERT_THROW(subExpIndex == 0 || subExpIndex < getSubExpCount(), EX_INVALID_RANGE);

	SQRex* rex = (SQRex*)_peer;
	bool found = sqstd_rex_search(rex, str+start, NULL, NULL);
	if (!found) return false;

	SQRexMatch match;
	sqstd_rex_getsubexp(rex, subExpIndex, &match);
	outResult.begin = match.begin;
	outResult.end = match.begin + match.len;

	return true;
}

bool RegExp::captureAll(const char* str, StringVector& outResults, uint start)
{
	if (_peer == NULL) return false;

	SQRex* rex = (SQRex*)_peer;
	bool found = sqstd_rex_search(rex, str+start, NULL, NULL);
	if (!found) return false;

	SQInteger n = sqstd_rex_getsubexpcount(rex);
	for (SQInteger i=0; i < n; ++i)
	{
		SQRexMatch match;
		sqstd_rex_getsubexp(rex, i, &match);
		outResults.push_back(String());
		outResults.back().assign(match.begin, match.begin + match.len);
	}

	return true;
}

bool RegExp::captureAll(const char* str, vector<SubExp>::type& outResults, uint start)
{
	if (_peer == NULL) return false;

	SQRex* rex = (SQRex*)_peer;
	bool found = sqstd_rex_search(rex, str+start, NULL, NULL);
	if (!found) return false;

	SQInteger n = sqstd_rex_getsubexpcount(rex);
	for (SQInteger i=0; i < n; ++i)
	{
		SQRexMatch match;
		sqstd_rex_getsubexp(rex, i, &match);
		outResults.push_back(SubExp());
		SubExp& e = outResults.back();
		e.begin = match.begin;
		e.end = match.begin + match.len;
	}

	return true;
}

String RegExp::format(const char* str, const char* fmt, uint start)
{
	if (_peer == NULL) return fmt;

	String ret;

	SQRex* rex = (SQRex*)_peer;
	const char *begin, *end;
	bool found = sqstd_rex_search(rex, str+start, &begin, &end);
	if (!found) return ret;

	uint n = sqstd_rex_getsubexpcount(rex);

	for (const char* f = fmt; *f; ++f)
	{
		int ch = *f;
		if (ch != '%')
		{
			ret.push_back(ch);
			continue;
		}

		ch = *++f;

		if (ch == '*')
			ret.append(str);
		else if (ch == '<')
			ret.append(str, begin);
		else if (ch == '>')
			ret.append(end);
		else if ('0' <= ch && ch <= '9')
		{
			uint subIdx = ch - '0';
			if (subIdx < n)
			{
				SQRexMatch match;
				sqstd_rex_getsubexp(rex, subIdx, &match);
				ret.append(match.begin, match.len);
			}
			else
				NIT_THROW_FMT(EX_INVALID_PARAMS, "no subexp: %d", subIdx);
		}
		else ret.push_back(ch);
	}

	return ret;
}

String RegExp::substitute(const char* str, const char* replacement, uint subExpIndex, uint start)
{
	if (_peer == NULL) return str;
	ASSERT_THROW(subExpIndex == 0 || subExpIndex < getSubExpCount(), EX_INVALID_RANGE);

	String ret;

	SQRex* rex = (SQRex*)_peer;
	bool found = sqstd_rex_search(rex, str+start, NULL, NULL);
	if (!found) return str;

	SQRexMatch match;
	sqstd_rex_getsubexp(rex, subExpIndex, &match);

	ret.assign(str, match.begin);
	ret.append(replacement);
	ret.append(match.begin + match.len);

	return ret;
}

void RegExp::Help()
{
	LOG(0,
		"PATTERN\n"
		"  \\\\          Quote the next metacharacter\n"
		"  ^          Match the beginning of the string\n"
		"  .          Match any character\n"
		"  $          Match the end of the string\n"
		"  |          Alternation\n"
		"  (subexp)   Grouping (creates a capture)\n"
		"  (?:subexp) No Capture Grouping (no capture)\n"
		"  []         Character class\n"
		"  \nGREEDY CLOSURES\n"
		"  *     Match 0 or more times\n"
		"  +     Match 1 or more times\n"
		"  ?     Match 1 or 0 times\n"
		"  {n}   Match exactly n times\n"
		"  {n,}  Match at least n times\n"
		"  {n,m} Match at least n but not more than m times\n"
		"  \nESCAPE CHARACTERS\n"
		"  \\\\t    tab (HT, TAB)\n"
		"  \\\\n    newline (LF, NL)\n"
		"  \\\\r    return (CR)\n"
		"  \\\\f    form feed (FF)\n"
		"  \nPREDEFINED CLASSES\n"
		"  \\\\l    lowercase next char\n"
		"  \\\\u    uppercase next char\n"
		"  \\\\a    letters\n"
		"  \\\\A    non letters\n"
		"  \\\\w    alphanumeric [_0-9a-zA-Z]\n"
		"  \\\\W    non alphanumeric [^_0-9a-zA-Z]\n"
		"  \\\\s    space\n"
		"  \\\\S    non space\n"
		"  \\\\d    digits\n"
		"  \\\\D    non digits\n"
		"  \\\\x    hexadecimal digits\n"
		"  \\\\X    non hexadecimal digits\n"
		"  \\\\c    control characters\n"
		"  \\\\C    non control characters\n"
		"  \\\\p    punctation\n"
		"  \\\\P    non punctation\n"
		"  \\\\b    word boundary\n"
		"  \\\\B    non word boundary" );
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
