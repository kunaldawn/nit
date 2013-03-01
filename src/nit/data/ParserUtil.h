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

#include "nit/io/Stream.h"
#include "nit/data/Settings.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NitRuntime;

class NIT_API CmdLineParser : public RefCounted
{
public:
	CmdLineParser();

public:
	void								allowUnknownOptions(bool flag)			{ _allowUnknownOptions = flag; }
	void								allowUnknownParams(bool flag)			{ _allowUnknownParams = flag; }
	void								allowOmitMandatory(bool flag)			{ _allowOmitMandatory = flag; }

	void								addOption(const String& name, const String& abbr, const String& desc, bool optional = true, bool multiple = false);
	void								addSwitch(const String& name, const String& abbr, const String& desc, bool optional = true);
	void								addParam(const String& name, const String& desc, bool variadic = false);

	void								addSection(Ref<Settings> shellSection);

	String								makeUsage(const String& appname, const String& logo, const String& text);

	void								showUsage(const String& appname = "", const String& logo = "", const String& text = "");

public:
	bool								parse(Ref<Settings> settings, const String& cmdline, bool throwEx = false, uint begin=0);
	bool								parse(Ref<Settings> settings, const StringVector& args, bool throwEx = false, uint begin=0);
	bool								parse(Ref<Settings> settings, NitRuntime* rt, bool throwEx = false, uint begin=1);
	bool								parse(Ref<Settings> settings, int argc, char** argv, bool throwEx = false, uint begin=1);

	static void							split(const String& cmdline, StringVector& outArgs);

protected:
	struct OptionEntry
	{
		String							name;
		String							abbr;
		String							desc;
		bool							found;
		bool							isSwitch;
		bool							optional;
		bool							multiple;
	};

	struct ParamEntry
	{
		String							name;
		String							desc;
		bool							found;
		bool							isVariadic;
	};

	struct Context
	{
		Context(const StringVector& args) : args(args) { }

		Ref<Settings>					settings;
		const StringVector&				args;
		bool							throwEx;
		bool							hasError;
	};

	typedef map<String, OptionEntry*>::type	OptionMap;
	typedef list<OptionEntry>::type			Options;
	typedef vector<ParamEntry>::type		Params;

	OptionMap							_optionMap;
	Options								_options;
	Params								_params;

	bool								_allowUnknownOptions;
	bool								_allowUnknownParams;
	bool								_allowOmitMandatory;

	bool								parseOption(uint& i, Context& ctx, const String& arg);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API LexerSource
{
public:
	typedef int							Char;
	typedef char						CharType;
	typedef String						StringType;

	typedef Char						(*Reader) (void* context);

public:
	LexerSource(Reader reader, void* context);

public:
	Char								read()									{ return _reader(_context); }

private:
	Reader								_reader;
	void*								_context;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API LexerBase
{
public:
	typedef LexerSource::Char			Char;
	typedef LexerSource::CharType		CharType;
	typedef LexerSource::StringType		StringType;
	typedef int							Token;

	typedef unordered_map<StringType, int>::type Keywords;

	LexerBase();
	virtual ~LexerBase();

public:
	enum
	{ 
		TK_NONE = -1, TK_EOS = 0, CHAR_EOS = 0,
		TK_ERROR = 512, TK_ID, TK_STRING, TK_FLOAT, TK_INT 
	};

	virtual void						start(LexerSource* source);

	virtual Token						lex() = 0;
	inline const CharType*				getTokenString()						{ return _stringValue; }
	inline float						getTokenFloat()							{ return _floatValue; }
	inline int							getTokenInt()							{ return _intValue; }

	void								setKeywords(Keywords* keywords)			{ _keywords = keywords; }

	inline int							getLine()								{ return _line; }
	inline int							getColumn()								{ return _column; }

	void								error(const char* fmt, ...);
	void								warning(const char* fmt, ...);

protected:
	inline Char							current()								{ return _currCh; }
	inline bool							isEos()									{ return _currCh == 0; }
	inline void							next()									{ _currCh = _source->read(); ++_column; }
	inline void							newLine()								{ _line++; next(); _column = 1; }

	Token								readNumber();
	Token								readId();
	Token								readString(CharType delim, bool verbatim = false);

	void								blockComment();
	void								lineComment();

	inline bool							isId(Char ch)							{ return isalnum(ch) || ch == '_'; }

protected:
	Char								_currCh;
	int									_column;
	int									_line;
	vector<CharType>::type				_stringBuf;

	const CharType*						_stringValue;
	float								_floatValue;
	int									_intValue;

	Keywords*							_keywords;

	LexerSource*						_source;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API ParserBase
{
public:
	ParserBase();
	virtual ~ParserBase();

protected:
	virtual void						parse(LexerSource* source) = 0;

	inline void							lex()									{ _token = _lexer->lex(); }

	void								error(const char* fmt, ...);
	void								warning(const char* fmt, ...);

	LexerBase::Token					_token;
	LexerBase*							_lexer;

public:
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API IParser
{
public:
	virtual ~IParser()					{ }

public:
	void								parse(LexerSource* source);
	void								parse(StreamReader* reader);
	void								parse(const void* buf, size_t len);
	void								parse(const String& str);
	void								parse(LexerSource::Reader reader, void* context);

protected:
	virtual void						doParse(LexerSource* source) = 0;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API Json : public IParser
{
public:
	class IHandler;

	Json(IHandler* emitter);

public:
	static void							writeEscaped(StreamWriter* w, const char* str);

protected:
	virtual void						doParse(LexerSource* source);

	IHandler*							_emitter;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API Json::IHandler
{
public:
	~IHandler()							{ }

public:
	virtual void 						documentBegin() = 0;
	virtual void 						documentEnd() = 0;

	virtual void 						pairObjectBegin(const String& key) = 0;
	virtual void 						pair(const String& key, const char* value) = 0;
	virtual void 						pair(const String& key, int value) = 0;
	virtual void 						pair(const String& key, float value) = 0;
	virtual void 						pair(const String& key, bool value) = 0;
	virtual void 						pairNull(const String& key) = 0;
	virtual void 						pairArrayBegin(const String& key) = 0;
	virtual void 						pairArrayEnd(const String& key) = 0;
	virtual void 						pairObjectEnd(const String& key) = 0;

	virtual void 						elementArrayBegin() = 0;
	virtual void 						element(const char* value) = 0;
	virtual void 						element(int value) = 0;
	virtual void 						element(float value) = 0;
	virtual void 						element(bool value) = 0;
	virtual void 						elementNull() = 0;
	virtual void 						elementObjectBegin() = 0;
	virtual void 						elementObjectEnd() = 0;
	virtual void 						elementArrayEnd() = 0;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;