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

#include <exception>
#include <string>

NS_NIT_BEGIN; 

////////////////////////////////////////////////////////////////////////////////

#ifndef NIT_NO_LOG
#	define NIT_THROW(CODE) throw ::nit::createException( \
		::nit::ExceptionCodeType<CODE>(), __FUNCTION__, __FILE__, __LINE__)

#	define NIT_THROW_FMT(CODE, ...) throw ::nit::createException( \
		::nit::ExceptionCodeType<CODE>(), __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
#else
#	define NIT_THROW(CODE)				throw ::nit::createException(::nit::ExceptionCodeType<CODE>(), "", "", 0)
#	define NIT_THROW_FMT(CODE, ...)		throw ::nit::createException(::nit::ExceptionCodeType<CODE>(), "", "", 0)
#endif 

////////////////////////////////////////////////////////////////////////////////

enum ExceptionCode
{
	EX_NULL,
	EX_INTERNAL,
	EX_NOT_IMPLEMENTED,
	EX_NOT_SUPPORTED,
	EX_NOT_FOUND,
	EX_INVALID_STATE,
	EX_INVALID_PARAMS,
	EX_INVALID_RANGE,
	EX_DUPLICATED,
	EX_ACCESS,
	EX_READ,
	EX_WRITE,
	EX_CORRUPTED,
	EX_SYNTAX,
	EX_MATH,
	EX_IO,
	EX_INPUT,
	EX_RENDER,
	EX_SOUND,
	EX_DATABASE,
	EX_NET,
	EX_SYSTEM,
	EX_ASSERT,
	EX_TIMEOUT,
	EX_OVERFLOW,
	EX_MEMORY,
	EX_CASTING,
	EX_SCRIPT,
	EX_LINK,
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API Exception : public std::exception
{
public:
	Exception(int code);
	Exception(int code, const String& desc);
	Exception(int code, const String& desc, const String& source, const String& file, int line);
	Exception(const Exception& other);
	~Exception() throw() { } // needed for compatibility with std::exception
	Exception& operator = (const Exception& other);

	virtual const char* name() const throw() { return "Exception"; }
	const char* what() const throw() { return getFullDescription().c_str(); }

public:
	int									getCode() const							{ return _code; }
	int									getLine() const							{ return _line; }
	const String&						getDescription() const					{ return _description; }
	const String&						getSource() const						{ return _source; }
	const String&						getFile() const							{ return _file; }
	const String&						getFullDescription() const;

public:
	static int							getLastExceptionCode()					{ return s_LastExceptionCode; }

protected:
	int									_code;
	int									_line;
	String								_description;
	String								_source;
	String								_file;
	mutable String						_fullDesc;

private:
	static int							s_LastExceptionCode;
};

////////////////////////////////////////////////////////////////////////////////

template <int code>
struct ExceptionCodeType
{
	enum { CODE = code };
};

////////////////////////////////////////////////////////////////////////////////

#define NIT_DECLARE_EXCEPTION(CODE, CLS, BASE) \
class NIT_API CLS : public BASE \
{ \
public: \
	CLS() : BASE(CODE) { } \
	CLS(const String& desc) : BASE(CODE, desc) { } \
	CLS(const String& desc, const String& source, const String& file, int line) : BASE(CODE, desc, source, file, line) { } \
	CLS(const CLS& other) : BASE(other) { } \
	~CLS() throw() { } \
	CLS& operator = (const CLS& other) { BASE::operator = (other); return *this; } \
	const char* name() const throw() { return #CLS; } \
}; \
inline CLS createException(ExceptionCodeType<CODE> ct, const String& source, const String& file, int line) { \
	return CLS("", source, file, line); \
} \
inline CLS createException(ExceptionCodeType<CODE> ct, const String& source, const String& file, int line, const char* fmt, ...) { \
	va_list args; va_start(args, fmt); \
	String desc = StringUtil::vformat(fmt, args); \
	va_end(args); \
	return CLS(desc, source, file, line); \
}


////////////////////////////////////////////////////////////////////////////////

NIT_DECLARE_EXCEPTION(EX_NULL,				NullException,				Exception);
NIT_DECLARE_EXCEPTION(EX_INTERNAL,			InternalException,			Exception);
NIT_DECLARE_EXCEPTION(EX_NOT_IMPLEMENTED,	NotImplementedException,	Exception);
NIT_DECLARE_EXCEPTION(EX_NOT_SUPPORTED,		NotSupportedException,		Exception);
NIT_DECLARE_EXCEPTION(EX_NOT_FOUND,			NotFoundException,			Exception);
NIT_DECLARE_EXCEPTION(EX_INVALID_STATE,		InvalidStateException,		Exception);
NIT_DECLARE_EXCEPTION(EX_INVALID_PARAMS,	InvalidParamsException,		Exception);
NIT_DECLARE_EXCEPTION(EX_INVALID_RANGE,		InvalidRangeException,		Exception);
NIT_DECLARE_EXCEPTION(EX_DUPLICATED,		DuplicatedException,		Exception);
NIT_DECLARE_EXCEPTION(EX_ACCESS,			AccessException,			Exception);
NIT_DECLARE_EXCEPTION(EX_READ,				ReadException,				Exception);
NIT_DECLARE_EXCEPTION(EX_WRITE,				WriteException,				Exception);
NIT_DECLARE_EXCEPTION(EX_CORRUPTED,			CorruptedException, 		Exception);
NIT_DECLARE_EXCEPTION(EX_SYNTAX,			SyntaxException,			Exception);
NIT_DECLARE_EXCEPTION(EX_MATH,				MathException,				Exception);
NIT_DECLARE_EXCEPTION(EX_IO,				IOException,				Exception);
NIT_DECLARE_EXCEPTION(EX_INPUT,				InputException,				Exception);
NIT_DECLARE_EXCEPTION(EX_RENDER,			RenderException,			Exception);
NIT_DECLARE_EXCEPTION(EX_SOUND,				SoundException,				Exception);
NIT_DECLARE_EXCEPTION(EX_DATABASE,			DatabaseException,			Exception);
NIT_DECLARE_EXCEPTION(EX_NET,				NetException,				Exception);
NIT_DECLARE_EXCEPTION(EX_SYSTEM,			SystemException,			Exception);
NIT_DECLARE_EXCEPTION(EX_ASSERT,			AssertException,			Exception);
NIT_DECLARE_EXCEPTION(EX_TIMEOUT,			TimeoutException,			Exception);
NIT_DECLARE_EXCEPTION(EX_OVERFLOW,			OverflowException,			Exception);
NIT_DECLARE_EXCEPTION(EX_MEMORY,			MemoryException,			Exception);
NIT_DECLARE_EXCEPTION(EX_CASTING,			CastingException,			Exception);
NIT_DECLARE_EXCEPTION(EX_SCRIPT,			ScriptException,			Exception);
NIT_DECLARE_EXCEPTION(EX_LINK,				LinkException,				Exception);

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
