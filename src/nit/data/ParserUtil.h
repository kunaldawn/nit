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

class NIT_API LexerBase
{
public:
	typedef int							Char;
	typedef char						CharType;
	typedef String						StringType;
	typedef int							Token;

	typedef unordered_map<StringType, int>::type Keywords;

	LexerBase();
	virtual ~LexerBase();

public:
	enum
	{ 
		CHAR_EOS = 0, 

		TK_EOS = 0, 
		TK_EOL = 10,

		TK_ERROR = 512, 
		TK_IDENTIFIER, 
		TK_STRING_VALUE, 
		TK_FLOAT_VALUE, 
		TK_INT_VALUE 
	};

	struct TokenInfo
	{
		Token							token;
		String							stringValue;
		int								intValue;
		float							floatValue;

		int								startLine;
		int								startColumn;
		int								endLine;
		int								endColumn;
	};

	virtual void						start(StreamReader* reader);

	virtual Token						lex();
	const TokenInfo&					getTokenInfo()							{ return _curr; }
	Token								getPrevToken()							{ return _prevToken; }

	void								setKeywords(Keywords* keywords)			{ _keywords = keywords; }

	inline int							getLine()								{ return _line; }
	inline int							getColumn()								{ return _column; }

	Token								error(const char* fmt, ...);
	Token								warning(const char* fmt, ...);

protected:
	Char								next();

	inline Char							current()								{ return _ch; }
	inline bool							isEos()									{ return _ch == CHAR_EOS; }
	inline void							newLine()								{ token(TK_EOL); ++_line; next(); _column = 1; }

	Token								readNumber();
	Token								readId();
	Token								readString(Char delim, Char verbatim = 0);

	void								bufClear()								{ _stringBuf.resize(0); }
	void								bufAddChar(Char ch);
	void								bufToString(String& outString)			{ outString.assign(_stringBuf.begin(), _stringBuf.end()); }

	void								blockComment();
	void								lineComment();
	void								whitespace();

	inline bool							isId(Char ch)							{ return isalnum(ch) || ch == '_'; }
	Token								token(int tok);

protected:
	Char								_ch;
	int									_column;
	int									_line;
	vector<CharType>::type				_stringBuf;

	Token								_prevToken;
	TokenInfo							_curr;

	Keywords*							_keywords;

	Ref<StreamReader>					_reader;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API ParserBase
{
public:
	ParserBase();
	virtual ~ParserBase();

protected:
	virtual void						parse(StreamReader* reader) = 0;

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
	void								parse(StreamReader* reader);
	void								parse(const void* buf, size_t len);
	void								parse(const String& str);

protected:
	virtual void						doParse(StreamReader* reader) = 0;
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
	virtual void						doParse(StreamReader* reader);

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

class NIT_API XmlParser : public RefCounted
{
public:
	XmlParser();
	virtual ~XmlParser();

public:
	void								init(const char* xml, int len=-1);
	void								init(StreamReader* reader);

	bool								open(const char** tagPatterns, int numPatterns, bool throwEx = true);
	bool								open(const char* tagPattern="*", bool throwEx = true);
	bool 								close(const char* tag = NULL, bool throwEx = true);
	const String&						text();
	const String&						comment();

public:
	const String&						getTag();
	DataRecord*							getAttrs();

	size_t								getDepth()								{ return _tagStack.size(); }

public:
	int									getLine();
	int									getColumn();
	int									getBytes();

private:
	void								onStartElement(const char* name, const char** attrs);
	void								onEndElement(const char* name);
	void								onText(const char* s, int len);
	void								onComment(const char* data);

private:
	static void							startElementHandler(void* ctx, const char* name, const char** attrs);
	static void							endElementHandler(void* ctx, const char* name);
	static void							characterDataHandler(void* ctx, const char* s, int len);
	static void							commentHandler(void* ctx, const char* data);

	static void*						expat_malloc(size_t size);
	static void*						expat_realloc(void* ptr, size_t size);
	static void							expat_free(void* ptr);

private:
	void*								_parser;

	struct Token
	{
		enum TokenType
		{
			NONE,
			TAG_OPEN,
			TAG_CLOSE,
			TEXT,
			COMMENT,
			FINISH
		};

		Token*							ahead;
		TokenType						type;
		String							text;
		Ref<DataRecord>					attrs;
		int								line;
		int								column;
		int								bytes;

		void							clear()		{ type = NONE; text.clear(); attrs = NULL; }
	};

	Ref<StreamReader>					_reader;

	StringVector						_tagStack;
	vector<Ref<DataRecord> >::type		_attrsStack;

	String								_text;
	String								_comment;
	int									_line;
	int									_column;
	int									_bytes;

	Token								_tokens[3];
	Token*								_next;
	Token*								_ahead;

	void								reset();
	void								cleanup();
	int									feedParser();
	void								checkStatus(int st);
	bool								next();
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;