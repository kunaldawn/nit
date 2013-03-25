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

#include "nit/data/ParserUtil.h"

#include "nit/io/MemoryBuffer.h"

#define XML_BUILDING_EXPAT 1
#include "expat/expat.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

CmdLineParser::CmdLineParser()
{
	_allowUnknownOptions	= false;
	_allowUnknownParams	= false;
	_allowOmitMandatory	= false;
}

void CmdLineParser::addOption(const String& name, const String& abbr, const String& desc, bool optional, bool multiple)
{
	ASSERT_THROW(!name.empty() && !abbr.empty(), EX_INVALID_PARAMS);

	OptionMap::iterator itr = _optionMap.find(name);
	if (itr != _optionMap.end())
		NIT_THROW(EX_DUPLICATED);

	itr = _optionMap.find(abbr);
	if (itr != _optionMap.end())
		NIT_THROW(EX_DUPLICATED);

	_options.push_back(OptionEntry());

	OptionEntry& opt = _options.back();
	opt.name		= name;
	opt.abbr		= abbr;
	opt.desc		= desc;
	opt.found		= false;
	opt.isSwitch	= false;
	opt.optional	= optional;
	opt.multiple	= multiple;

	_optionMap.insert(std::make_pair(name, &opt));

	if (!abbr.empty())
		_optionMap.insert(std::make_pair(abbr, &opt));
}

void CmdLineParser::addSwitch(const String& name, const String& abbr, const String& desc, bool optional)
{
	OptionMap::iterator itr = _optionMap.find(name);
	if (itr != _optionMap.end())
		NIT_THROW(EX_DUPLICATED);

	itr = _optionMap.find(abbr);
	if (itr != _optionMap.end())
		NIT_THROW(EX_DUPLICATED);

	_options.push_back(OptionEntry());

	OptionEntry& opt = _options.back();
	opt.name		= name;
	opt.abbr		= abbr;
	opt.desc		= desc;
	opt.found		= false;
	opt.isSwitch	= true;
	opt.optional	= optional;
	opt.multiple	= false;

	_optionMap.insert(std::make_pair(name, &opt));

	if (!abbr.empty())
		_optionMap.insert(std::make_pair(abbr, &opt));
}

void CmdLineParser::addParam(const String& name, const String& desc, bool variadic)
{
	if (!_params.empty() && _params.back().isVariadic)
		NIT_THROW_FMT(EX_INVALID_STATE, "can't add param after variadic");

	ParamEntry param;
	param.name			= name;
	param.desc			= desc;
	param.found			= false;
	param.isVariadic	= variadic;

	_params.push_back(param);
}

void CmdLineParser::split(const String& cmdline, StringVector& outArgs)
{
	// Excerpts from ConvertStringToArgs() on cmdline.cpp of wxWidgets

	String arg;
	arg.reserve(1024);

	const String::const_iterator end = cmdline.end();
	String::const_iterator p = cmdline.begin();

	for ( ;; )
	{
		// skip white space
		while ( p != end && (*p == ' ' || *p == '\t') )
			++p;

		// anything left?
		if ( p == end )
			break;

		// parse this parameter
		bool lastBS = false,
			isInsideQuotes = false;
		char chDelim = '\0';
		for ( arg.clear(); p != end; ++p )
		{
			const char ch = *p;

			if ( !lastBS )
			{
				if ( isInsideQuotes )
				{
					if ( ch == chDelim )
					{
						isInsideQuotes = false;

						continue;   // don't use the quote itself
					}
				}
				else // not in quotes and not escaped
				{
					if ( ch == '\'' || ch == '"' )
					{
						isInsideQuotes = true;
						chDelim = ch;

						continue;   // don't use the quote itself
					}

					if ( ch == ' ' || ch == '\t' )
					{
						++p;    // skip this space anyhow
						break;
					}
				}

				lastBS = ch == '\\';
				if ( lastBS )
					continue;
			}
			else // escaped by backslash, just use as is
			{
				lastBS = false;
			}

			arg += ch;
		}

		outArgs.push_back(arg);
	}
}

bool CmdLineParser::parse(Ref<Settings> settings, int argc, char** argv, bool throwEx, uint begin)
{
	StringVector args;
	for (int i=0; i < argc; ++i)
		args.push_back(argv[i]);

	return parse(settings, args, throwEx, begin);
}

bool CmdLineParser::parse(Ref<Settings> settings, NitRuntime* rt, bool throwEx, uint begin)
{
	StringVector args;

	const std::vector<std::string>& s_args = rt->getArguments();
	for (uint i=0; i < s_args.size(); ++i)
		args.push_back(s_args[i].c_str());

	return parse(settings, args, throwEx, begin);
}

bool CmdLineParser::parse(Ref<Settings> settings, const String& cmdline, bool throwEx, uint begin)
{
	StringVector args;
	split(cmdline, args);

	return parse(settings, args, throwEx, begin);
}

bool CmdLineParser::parse(Ref<Settings> settings, const StringVector& args, bool throwEx, uint begin)
{
	uint paramIdx = 0;

	Context ctx(args);
	ctx.settings = settings;
	ctx.throwEx = throwEx;
	ctx.hasError = false;

	// reset found states
	for (Options::iterator itr = _options.begin(), end = _options.end(); itr != end; ++itr)
	{
		itr->found = false;
	}

	for (uint i = 0; i < _params.size(); ++i)
	{
		_params[i].found = false;
	}

	for (uint i = begin; i < args.size(); ++i)
	{
		const String& arg = args[i];

		if (parseOption(i, ctx, arg))
			continue;

		if (paramIdx >= _params.size())
		{
			if (!_allowUnknownParams)
			{
				if (throwEx) NIT_THROW_FMT(EX_SYNTAX, "unexpected param: '%s'", arg.c_str());
				ctx.hasError = true;
				break;
			}
			else continue;
		}

		_params[paramIdx].found = true;
		settings->add(_params[paramIdx].name, arg);

		if (!_params[paramIdx].isVariadic)
			++paramIdx;
	}

	// check madantories
	if (!_allowOmitMandatory)
	{
		for (uint i=0; i<_params.size(); ++i)
		{
			if (!_params[i].isVariadic && !_params[i].found)
			{
				if (throwEx) NIT_THROW_FMT(EX_SYNTAX, "<%s> expected", _params[i].name.c_str());
				ctx.hasError = true;
				break;
			}
		}

		for (Options::iterator itr = _options.begin(), end = _options.end(); itr != end; ++itr)
		{
			OptionEntry& opt = *itr;
			if (!opt.optional && !opt.found)
			{
				if (throwEx)
                {
					if (opt.isSwitch)
						NIT_THROW_FMT(EX_SYNTAX, "-%s expected", opt.abbr.c_str());
					else
						NIT_THROW_FMT(EX_SYNTAX, "-%s <%s> expected", opt.abbr.c_str(), opt.name.c_str());
                }
				ctx.hasError = true;
				break;
			}
		}
	}

	return !ctx.hasError;
}

bool CmdLineParser::parseOption(uint& i, Context& ctx, const String& arg)
{
	size_t argLen = arg.length();

	if (argLen < 2) return false;
	if (arg[0] != '-') return false;
	if (argLen == 2 && arg[1] == '-') return false;

	bool abbr = true;
	String optStr;

	if (argLen > 2 && arg[1] == '-')
	{
		abbr = false;
		optStr = arg.substr(2);
	}
	else
	{
		optStr = arg.substr(1);
	}

	bool switchOn = true;
	bool mustBeSwitch = false;
	if (optStr.length() > 0 && *--optStr.end() == '-')
	{
		optStr.resize(optStr.length()-1);
		switchOn = false;
		mustBeSwitch = true;
	}

	OptionMap::iterator itr = _optionMap.find(optStr);

	if (itr == _optionMap.end())
	{
		if (!_allowUnknownOptions)
		{
			if (ctx.throwEx) NIT_THROW_FMT(EX_SYNTAX, "unknown option '%s'", arg.c_str());
			ctx.hasError = true;
		}
		return true;
	}

	OptionEntry& opt = *itr->second;

	if (! ((abbr && optStr == opt.abbr) || (!abbr && optStr == opt.name)) )
	{
		if (!_allowUnknownOptions)
		{
			if (ctx.throwEx) NIT_THROW_FMT(EX_SYNTAX, "unknown option '%s'", arg.c_str());
			ctx.hasError = true;
		}
		return true;
	}

	if (!opt.isSwitch && mustBeSwitch)
	{
		if (ctx.throwEx) NIT_THROW_FMT(EX_SYNTAX, "unknown option '%s'", arg.c_str());
		ctx.hasError = true;
		return true;
	}

	if (opt.isSwitch)
	{
		if (opt.found)
		{
			if (ctx.throwEx) NIT_THROW_FMT(EX_SYNTAX, "switch '%s' duplicated", arg.c_str());
			ctx.hasError = true;
			return true;
		}

		opt.found = true;
		ctx.settings->set(opt.name, switchOn ? "true" : "false");
		return true;
	}

	if (!opt.multiple && opt.found)
	{
		if (ctx.throwEx) NIT_THROW_FMT(EX_SYNTAX, "option '%s' duplicated", arg.c_str());
		ctx.hasError = true;
		return true;
	}

	if (i+1 >= ctx.args.size())
	{
		if (ctx.throwEx) NIT_THROW_FMT(EX_SYNTAX, "option '%s' needs value", arg.c_str());
		ctx.hasError = true;
		return true;
	}

	const String& nextArg = ctx.args[i+1];
	if (nextArg != "--" && nextArg[0] == '-')
	{
		if (ctx.throwEx) NIT_THROW_FMT(EX_SYNTAX, "option '%s' needs value", nextArg.c_str());
		ctx.hasError = true;
		return true;
	}

	opt.found = true;
	if (opt.multiple)
		ctx.settings->add(opt.name, nextArg);
	else
		ctx.settings->set(opt.name, nextArg);
	++i;

	return true;
}

String CmdLineParser::makeUsage(const String& appname, const String& logo, const String& text)
{
	String usage;

	if (!logo.empty())
	{
		usage.append(logo);
		usage.append("\n\n");
	}

	usage.append("Usage:");

	if (!appname.empty())
	{
		usage.push_back(' ');
		usage.append(appname);
	}

	for (uint i=0; i < _params.size(); ++i)
	{
		usage.push_back(' ');

		ParamEntry& param = _params[i];

		if (!param.isVariadic)
		{
			usage.push_back('<');
			usage.append(param.name);
			usage.push_back('>');
		}
		else
		{
			usage.append("[<");
			usage.append(param.name);
			usage.append("> ...]");
		}
	}

	for (Options::iterator itr = _options.begin(), end = _options.end(); itr != end; ++itr)
	{
		OptionEntry& opt = *itr;

		usage.push_back(' ');

		if (opt.optional)
			usage.push_back('[');

		usage.push_back('-');
		usage.append(opt.abbr);

		if (!opt.isSwitch)
		{
			usage.append(" <");
			usage.append(opt.name);
			usage.push_back('>');

			if (opt.multiple)
			{
				usage.append(" [-");
				usage.append(opt.abbr);
				usage.append(" ...]");
			}
		}
		else
		{
			if (!opt.optional)
				usage.append("[-]");
		}

		if (opt.optional)
			usage.push_back(']');
	}

	usage.push_back('\n');

	for (uint i=0; i < _params.size(); ++i)
	{
		ParamEntry& param = _params[i];

		usage.push_back('\n');
		usage.append(StringUtil::format("  %-20s : %s", (String("<") + param.name + ">").c_str(), param.desc.c_str()));
	}

	usage.push_back('\n');

	for (Options::iterator itr = _options.begin(), end = _options.end(); itr != end; ++itr)
	{
		OptionEntry& opt = *itr;

		usage.push_back('\n');

		String temp = StringUtil::format("-%s, --%s", opt.abbr.c_str(), opt.name.c_str());

		usage.append(StringUtil::format("  %-20s : %s", temp.c_str(), opt.desc.c_str()));
	}

	if (!text.empty())
	{
		usage.push_back('\n');
		usage.push_back('\n');
		usage.append(text);
	}

	usage.push_back('\n');

	return usage;
}

void CmdLineParser::showUsage(const String& appname, const String& logo, const String& text)
{
#if defined(NIT_WIN32)
	if (GetConsoleWindow())
	{
		fputws(Unicode::toUtf16(makeUsage(appname, logo, text)).c_str(), stderr);
	}
	else
	{
		MessageBoxW(NULL, 
			Unicode::toUtf16(makeUsage(appname, logo, text)).c_str(), 
			Unicode::toUtf16(appname).c_str(), 
			MB_ICONERROR);
	}
#else
	fputs(makeUsage(appname, logo, text).c_str(), stderr);
#endif
}

void CmdLineParser::addSection(Ref<Settings> shellSection)
{
	if (shellSection == NULL) return;

	StringVector list;
	shellSection->find("param", list);

	for (uint i=0; i < list.size(); ++i)
	{
		const String& line = list[i];
		size_t p = line.find(':');

		String name, desc;
		bool variadic;

		if (p == line.npos)
			name = line;
		else
		{
			name = line.substr(0, p);
			desc = line.substr(p+1);

			StringUtil::trim(name);
			StringUtil::trim(desc);
		}

		variadic = Wildcard::match("[*...]", name);

		if (variadic)
		{
			name = name.substr(1, name.length()-5);
			StringUtil::trim(name);
		}
		else if (Wildcard::match("*...", name) || Wildcard::match("[*]", name))
			NIT_THROW_FMT(EX_SYNTAX, "invalid param: '%s'", name.c_str());

		addParam(name, desc, variadic);
	}

	list.resize(0);
	shellSection->find("option", list);

	for (uint i=0; i < list.size(); ++i)
	{
		const String& line = list[i];
		size_t p = line.find(':');

		String name, abbr, desc;
		bool optional, multiple;

		if (p == line.npos)
			name = line;
		else
		{
			name = line.substr(0, p);
			desc = line.substr(p+1);

			StringUtil::trim(name);
			StringUtil::trim(desc);
		}

		optional = Wildcard::match("[*]", name);
		if (optional)
		{
			name = name.substr(1, name.length()-2);
			StringUtil::trim(name);
		}

		multiple = Wildcard::match("*...", name);
		if (multiple)
		{
			name = name.substr(0, name.length()-3);
			StringUtil::trim(name);
		}

		if (Wildcard::match("-* *", name))
		{
			p = name.find(' ');
			abbr = name.substr(1, p-1);
			name = name.substr(p);
			StringUtil::trim(name);
			StringUtil::trim(abbr);
		}
		else NIT_THROW_FMT(EX_SYNTAX, "invalid option: %s", name.c_str());

		addOption(name, abbr, desc, optional, multiple);
	}

	list.resize(0);
	shellSection->find("switch", list);

	for (uint i=0; i < list.size(); ++i)
	{
		const String& line = list[i];
		size_t p = line.find(':');

		String name, abbr, desc;
		bool optional;

		if (p == line.npos)
			name = line;
		else
		{
			name = line.substr(0, p);
			desc = line.substr(p+1);

			StringUtil::trim(name);
			StringUtil::trim(desc);
		}

		optional = Wildcard::match("[*]", name);
		if (optional)
		{
			name = name.substr(1, name.length()-2);
			StringUtil::trim(name);
		}

		if (Wildcard::match("-* *", name))
		{
			p = name.find(' ');
			abbr = name.substr(1, p-1);
			name = name.substr(p);
			StringUtil::trim(name);
			StringUtil::trim(abbr);
		}
		else NIT_THROW_FMT(EX_SYNTAX, "invalid switch: %s", name.c_str());

		addSwitch(name, abbr, desc, optional);
	}
}

////////////////////////////////////////////////////////////////////////////////

LexerBase::LexerBase()
{
	_source							= NULL;

	_currCh							= TK_NONE;
	_stringValue						= NULL;
	_floatValue						= 0.0f;
	_intValue							= 0;

	_line = 1;
	_column = 0;
}

LexerBase::~LexerBase()
{
}

enum
{
	NT_INT,
	NT_FLOAT,
	NT_HEX,
	NT_SCIENTIFIC,
	NT_OCTAL,

	MAX_HEX_DIGITS = sizeof(int) * 2
};

void LexerBase::start(LexerSource* source)
{
	_source = source;

	next();
}

static inline int isoctdigit(int c) 
{ 
	return c >= '0' && c <= '7'; 
}

static inline bool isexponent(int c)
{
	return c == 'e' || c == 'E';
}

static int ParseDec(const char* s, int radix = 10)
{
	int value = 0;
	while (*s)
	{
		value = value * radix + (*s++) - '0';
	}
	return value;
}

static int ParseHex(const char* s, int radix = 16)
{
	int value = 0;
	while (*s)
	{
		if (*s <= '9') value = value * radix + (*s++) - '0';
		else if (*s <= 'Z') value = value * radix + (*s++) - 'A' + 10;
		else if (*s <= 'z') value = value * radix + (*s++) - 'a' + 10;
		else assert(0);
	}
	return value;
}

LexerBase::Token LexerBase::readNumber()
{
	int type = NT_INT;
	int firstChar = _currCh;
	_stringBuf.resize(0);
	next();

	if (firstChar == '0' && (toupper(_currCh) == 'X' || isoctdigit(_currCh)))
	{
		if (isoctdigit(_currCh))
		{
			type = NT_OCTAL;
			while (isoctdigit(_currCh))
			{
				_stringBuf.push_back(_currCh);
				next();
			}
			if (isdigit(_currCh))
				error("invalid octal number");
		}
		else
		{
			next();
			type = NT_HEX;
			while (isxdigit(_currCh))
			{
				_stringBuf.push_back(_currCh);
				next();
			}
			if (_stringBuf.size() > MAX_HEX_DIGITS)
				error("too many digits for an hex number");
		}
	}
	else
	{
		_stringBuf.push_back(firstChar);
		while (_currCh == '.' || isdigit(_currCh) || isexponent(_currCh))
		{
			if (_currCh == '.')
				type = NT_FLOAT;
			if (isexponent(_currCh))
			{
				if (type == NT_INT) type = NT_FLOAT;
				if (type != NT_FLOAT) error("invalid numeric format");
				type = NT_SCIENTIFIC;
				_stringBuf.push_back(_currCh);
				next();
				if (_currCh == '+' || _currCh == '-')
				{
					_stringBuf.push_back(_currCh);
					next();
				}
				if (!isdigit(_currCh))
					error("exponent expected");
			}
			_stringBuf.push_back(_currCh);
			next();
		}
	}

	char* sTemp;

	_stringBuf.push_back(0);
	switch (type)
	{
	case NT_SCIENTIFIC:
	case NT_FLOAT:
		_floatValue = (float)strtod(&_stringBuf[0], &sTemp);
		return TK_FLOAT;
	case NT_INT:
		_intValue = ParseDec(&_stringBuf[0], 10);
		return TK_INT;
	case NT_HEX:
		_intValue = ParseHex(&_stringBuf[0]);
		return TK_INT;
	case NT_OCTAL:
		_intValue = ParseDec(&_stringBuf[0], 8);
		return TK_INT;
	}
	return 0;
}

LexerBase::Token LexerBase::readId()
{
	_stringBuf.resize(0);
	do
	{
		_stringBuf.push_back(_currCh);
		next();
	}
	while (isId(_currCh));

	_stringBuf.push_back(0);

	_stringValue = &_stringBuf[0];

	if (_keywords == NULL)
		return TK_ID;

	Keywords::iterator itr = _keywords->find(_stringValue);

	if (itr == _keywords->end())
		return TK_ID;

	return itr->second;
}

LexerBase::Token LexerBase::readString(CharType delim, bool verbatim)
{
	_stringBuf.resize(0);
	next();

	if (isEos()) return -1;

	while (true)
	{
		while (_currCh != delim)
		{
			switch (_currCh)
			{
			case CHAR_EOS:
				error("unfinished string");
				return -1;

			case '\n':
				if (!verbatim) error("newline in a constant");
				_stringBuf.push_back(_currCh);
				newLine();
				break;

			case '\\':
				if (verbatim)
				{
					_stringBuf.push_back(_currCh);
					next();
				}
				else
				{
					next();
					switch (_currCh)
					{
					case '"': _stringBuf.push_back('\"'); next(); break;
					case '\\': _stringBuf.push_back('\\'); next(); break;
					case '\'': _stringBuf.push_back('\''); next(); break;
					case 'b': _stringBuf.push_back('\b'); next(); break;
					case 'f': _stringBuf.push_back('\f'); next(); break;
					case 'n': _stringBuf.push_back('\n'); next(); break;
					case 'r': _stringBuf.push_back('\r'); next(); break;
					case 't': _stringBuf.push_back('\t'); next(); break;
					case 'x': case 'u':
						{
							next();
							if (!isxdigit(_currCh)) error("hexadecimal number expected");
							const int maxDigits = 4;
							char temp[maxDigits+1];
							int n = 0;
							while (isxdigit(_currCh) && n < maxDigits)
							{
								temp[n] = _currCh;
								++n;
								next();
							}
							temp[n] = 0;
							char* sTemp;
							uint ch = strtoul(temp, &sTemp, 16);
							if (ch < 128)
								_stringBuf.push_back(ch);
							else
							{
								char utf8seq[4];
								size_t utf8len = Unicode::toUtf8Char(ch, utf8seq);
								_stringBuf.insert(_stringBuf.end(), utf8seq, utf8seq + utf8len);
							}
						}
						break;
					default:
						error("unrecognized escape char");
						break;
					}
				}
				break;
			default:
				_stringBuf.push_back(_currCh);
				next();
			}
		}
		next();
		if (verbatim && _currCh == '"')
		{
			_stringBuf.push_back(_currCh);
			next();
		}
		else
		{
			break;
		}
	}

	_stringBuf.push_back(0);
	_stringValue = &_stringBuf[0];
	return TK_STRING;
}

void LexerBase::blockComment()
{
	bool done = false;
	while (!done)
	{
		switch (_currCh)
		{
		case '*': 
			next(); 
			if (_currCh == '/')
			{
				done = true;
				next();
			}
			continue;
		case '\n':
			newLine();
			continue;
		case CHAR_EOS:
			warning("missing \"*/\" in comment");
			break;
		default:
			next();
		}
	}
}

void LexerBase::lineComment()
{
	do { next(); } while (_currCh != '\n' && !isEos());
}

void LexerBase::error(const char* fmt, ...)
{
	va_list args; 
	va_start(args, fmt);
	String desc = StringUtil::vformat(fmt, args);
	va_end(args);

	desc += StringUtil::format(" at line %d, column %d", _line, _column);

	throw SyntaxException(desc);
}

void LexerBase::warning(const char* fmt, ...)
{
	bool _treatWarningAsError = true;

	if (_treatWarningAsError)
	{
		va_list args; 
		va_start(args, fmt);
		String desc = StringUtil::vformat(fmt, args);
		va_end(args);

		desc += StringUtil::format(" at line %d, column %d", _line, _column);

		throw SyntaxException(desc);
	}
}

////////////////////////////////////////////////////////////////////////////////

ParserBase::ParserBase()
{
}

ParserBase::~ParserBase()
{
}

void ParserBase::error(const char* fmt, ...)
{
	va_list args; 
	va_start(args, fmt);
	String desc = StringUtil::vformat(fmt, args);
	va_end(args);

	desc += StringUtil::format(" at line %d, column %d", _lexer->getLine(), _lexer->getColumn());

	throw SyntaxException(desc);
}

void ParserBase::warning(const char* fmt, ...)
{
	bool _treatWarningAsError = true;

	if (_treatWarningAsError)
	{
		va_list args; 
		va_start(args, fmt);
		String desc = StringUtil::vformat(fmt, args);
		va_end(args);

		desc += StringUtil::format(" at line %d, column %d", _lexer->getLine(), _lexer->getColumn());

		throw SyntaxException(desc);
	}
}

////////////////////////////////////////////////////////////////////////////////

template <typename TReader>
static int LexerReaderASCII(void* context)
{
	TReader* r = (TReader*)context;
	return r->isEof() ? 0 : r->readInt8();
}

template <typename TReader>
static int LexerReaderUTF16LE(void* context)
{
	TReader* r = (TReader*)context;
	int ch = r->isEof() ? 0 : r->readInt16();

#if NIT_ENDIAN == NIT_ENDIAN_BIG
	ch = ((ch >> 8) & 0x00FF) | ((ch << 8) & 0xFF00);
#endif

	return ch;
}

template <typename TReader>
static int LexerReaderUTF16BE(void* context)
{
	TReader* r = (TReader*)context;
	int ch = r->isEof() ? 0 : r->readInt16();

#if NIT_ENDIAN == NIT_ENDIAN_LITTLE
	ch = ((ch >> 8) & 0x00FF) | ((ch << 8) & 0xFF00);
#endif

	return ch;
}

template <typename TReader>
static int LexerReaderUTF8(void* context)
{
	StreamReader* s = (StreamReader*)context;
#define READ() if (s->readRaw(&inchar, 1) != sizeof(inchar)) return 0

	static const int utf8_lengths[16] =
	{
		1,1,1,1,1,1,1,1,        /* 0000 to 0111 : 1 byte (plain ASCII) */
		0,0,0,0,                /* 1000 to 1011 : not valid */
		2,2,                    /* 1100, 1101 : 2 bytes */
		3,                      /* 1110 : 3 bytes */
		4                       /* 1111 :4 bytes */
	};

	static unsigned char byte_masks[5] = {0,0,0x1f,0x0f,0x07};
	unsigned char inchar;
	int c = 0;
	READ();
	c = inchar;
	//
	if(c >= 0x80) {
		int tmp;
		int codelen = utf8_lengths[c>>4];
		if(codelen == 0) 
			return 0;
		//"invalid UTF-8 stream";
		tmp = c&byte_masks[codelen];
		for(int n = 0; n < codelen-1; n++) {
			tmp<<=6;
			READ();
			tmp |= inchar & 0x3F;
		}
		c = tmp;
	}
	return c;

#undef READ
}

template <typename TReader>
static LexerSource::Reader LexerSelectReader(TReader* reader)
{
	LexerSource::Reader func = LexerReaderASCII<TReader>;

	uint16 us;
	uint8 uc;

	reader->getReader()->seek(0);
	int ret = reader->readRaw(&us, sizeof(us));
	if (ret != 2)
	{
		// probably an empty file
		us = 0;
	}

	switch (us)
	{
#if NIT_ENDIAN == NIT_ENDIAN_LITTLE
	case 0xFFFE: func = LexerReaderUTF16LE<TReader>; break;//UTF-16 little endian;
	case 0xFEFF: func = LexerReaderUTF16BE<TReader>; break;//UTF-16 big endian;
	case 0xBBEF: 
#endif

#if NIT_ENDIAN == NIT_ENDIAN_BIG
	case 0xFEFF: func = LexerReaderUTF16LE<TReader>; break;//UTF-16 little endian;
	case 0xFFFE: func = LexerReaderUTF16BE<TReader>; break;//UTF-16 big endian;
	case 0xEFBB: 
#endif

		if (reader->readRaw(&uc, sizeof(uc)) == 0)
		{
			NIT_THROW(EX_IO);
		}
		if (uc != 0xBF) 
		{ 
			NIT_THROW(EX_IO);
		}

		{
			// TODO: At now, we will treat BOM prefixed utf-8 as plain ascii.
			// TODO: Decide this later!
			bool treatBOMPrefiedUTF8AsAscii = true;

			if (treatBOMPrefiedUTF8AsAscii)
				func = LexerReaderASCII<TReader>;
			else
				func = LexerReaderUTF8<TReader>;
		}
		break;
	default: 
		// ascii
		reader->getReader()->seek(0);
	}

	return func;
}

LexerSource::LexerSource(Reader reader, void* context)
{
	_reader = reader;
	_context = context;
}

////////////////////////////////////////////////////////////////////////////////

void IParser::parse(StreamReader* reader)
{
	if (reader->isSeekable())
	{
		TBinaryReader<StreamReader> r(reader);

		parse(LexerSelectReader(&r), &r);
	}
	else
	{
		Ref<MemoryBuffer> buf = reader->buffer();
		MemoryBuffer::Access m(buf);
		TBinaryReader<MemoryAccess> r(&m);

		parse(LexerSelectReader(&r), &r);
	}
}

void IParser::parse(const void* buf, size_t len)
{
	MemoryAccess m(const_cast<void*>(buf), len);
	TBinaryReader<MemoryAccess> r(&m);
	parse(LexerSelectReader(&r), &r);
}

void IParser::parse(const String& str)
{
	parse(str.c_str(), str.length());
}

void IParser::parse(LexerSource::Reader reader, void* context)
{
	LexerSource source(reader, context);
	parse(&source);
}

void IParser::parse(LexerSource* source)
{
	doParse(source);
}

////////////////////////////////////////////////////////////////////////////////

enum JSONValueType
{
	VT_STRING,
	VT_INT,
	VT_FLOAT,
	VT_OBJECT,
	VT_ARRAY,
	VT_BOOL,
	VT_NULL,
};

enum JSONToken
{
	TK_ERROR							= LexerBase::TK_ERROR,
	TK_EOS								= LexerBase::TK_EOS,
	TK_STRING							= LexerBase::TK_STRING,
	TK_INT								= LexerBase::TK_INT,
	TK_FLOAT							= LexerBase::TK_FLOAT,
	TK_TRUE								= 256,
	TK_FALSE							= 257,
	TK_NULL								= 258,
};

////////////////////////////////////////////////////////////////////////////////

class JSONLexer : public LexerBase
{
public:
	JSONLexer()
	{
		static Keywords keywords;

		if (keywords.empty())
		{
			keywords["true"] = TK_TRUE;
			keywords["false"] = TK_FALSE;
			keywords["null"] = TK_NULL;
		}

		setKeywords(&keywords);
	}

	virtual Token lex()
	{
		while (_currCh != CHAR_EOS)
		{
			switch (_currCh)
			{
			case '\t': case '\r': case ' ': next(); continue;
			case '\n': newLine(); continue;
			case '/':
				next();
				switch (_currCh)
				{
				case '*':
					next();
					blockComment();
					continue;
				case '/':
					lineComment();
					continue;
				default:
					return Token('/');

				}
			case ':':
			case '=':
			case '{':
			case '}':
			case '[':
			case ']':
			case ',':
				{
					int token = _currCh;
					next();
					return Token(token);
				}
			case '"':
			case '\'':
				if (readString(_currCh) != -1)
					return Token(TK_STRING);
				error("error parsing the string");
				break;

			default:
				if (isdigit(_currCh))
					return Token(readNumber());
				else if (isId(_currCh))
					return Token(readId());
				else
					error("unexpected character");
			}
		}

		return TK_EOS;
	}
};

////////////////////////////////////////////////////////////////////////////////

class JSONParser : public ParserBase
{
public:
	JSONParser(Json::IHandler* emitter)
		: _handler(emitter)
	{
	}

	virtual void parse(LexerSource* source)
	{
		JSONLexer lexer;
		_lexer = &lexer;
		_lexer->start(source);

		_handler->documentBegin();

		lex();
		if (_token == '{')
		{
			_handler->elementObjectBegin();
			Object();
			_handler->elementObjectEnd();
		}
		else
			Element();

		Expect(TK_EOS);

		_handler->documentEnd();
	}

	void Object()
	{
		if (_token == '{')
			lex();

		Members();

		Expect('}');
	}

	void Expect(int token)
	{
		if (_token != token)
			error("%c expected", token);

		lex();
	}

	void Members()
	{
		while (true)
		{
			Pair();

			if (_token != ',')
				break;

			lex();
		}
	}

	void Pair()
	{
		if (_token != TK_STRING && _token != LexerBase::TK_ID)
			error("key string expected");

		String key = _lexer->getTokenString();

		lex();

		if (_token != ':' && _token != '=')
			error("colon expected");

		lex();

		switch (_token)
		{
		case TK_STRING:					_handler->pair(key, _lexer->getTokenString()); lex(); break;
		case TK_INT:					_handler->pair(key, _lexer->getTokenInt()); lex(); break;
		case TK_FLOAT:					_handler->pair(key, _lexer->getTokenFloat()); lex(); break;
		case TK_NULL:					_handler->pairNull(key); lex(); break;
		case TK_TRUE:					_handler->pair(key, true); lex(); break;
		case TK_FALSE:					_handler->pair(key, false); lex(); break;

		case '{':						_handler->pairObjectBegin(key); Object(); _handler->pairObjectEnd(key); break;
		case '[':						_handler->pairArrayBegin(key); Array(); _handler->pairArrayEnd(key); break;

		// Non-quoted literal is not a standard JSON, added as a syntax sugar
		case LexerBase::TK_ID:			_handler->pair(key, _lexer->getTokenString()); lex(); break;

		default:						error("value expected");
		}
	}

	void Array()
	{
		Expect('[');

		while (true)
		{
			Element();

			if (_token != ',')
				break;

			lex();
		}

		Expect(']');
	}

	void Element()
	{
		switch (_token)
		{
		case TK_STRING:					_handler->element(_lexer->getTokenString()); lex(); break;
		case TK_INT:					_handler->element(_lexer->getTokenInt()); lex(); break;
		case TK_FLOAT:					_handler->element(_lexer->getTokenFloat()); lex(); break;
		case TK_NULL:					_handler->elementNull(); lex(); break;
		case TK_TRUE:					_handler->element(true); lex(); break;
		case TK_FALSE:					_handler->element(false); lex(); break;

		case '{':						_handler->elementObjectBegin(); Object(); _handler->elementObjectEnd(); break;
		case '[':						_handler->elementArrayBegin(); Array(); _handler->elementArrayEnd(); break;

		// Non-quoted literal is not a standard JSON, added as a syntax sugar
		case LexerBase::TK_ID:			_handler->element(_lexer->getTokenString()); lex(); break;

		default:						error("value expected");
		}
	}

	Json::IHandler*						_handler;
};

////////////////////////////////////////////////////////////////////////////////

Json::Json(IHandler* emitter)
{
	_emitter = emitter;
}

void Json::doParse(LexerSource* source)
{
	JSONParser parser(_emitter);
	parser.parse(source);
}

void Json::writeEscaped(StreamWriter* w, const char* str)
{
	char digits[16];

	const char* ch = str;

	while (*ch)
	{
		if (*ch == '\"')
			w->writeRaw("\\\"", 2), ch++;
		else if (*ch == '\\')
			w->writeRaw("\\\\", 2), ch++;
		else if (*ch == '\b')
			w->writeRaw("\\b", 2), ch++;
		else if (*ch == '\f')
			w->writeRaw("\\f", 2), ch++;
		else if (*ch == '\n')
			w->writeRaw("\\n", 2), ch++;
		else if (*ch == '\r')
			w->writeRaw("\\r", 2), ch++;
		else if (*ch == '\t')
			w->writeRaw("\\t", 2), ch++;
		else if (32 <= *ch && *ch <= 127)
			w->writeRaw(ch++, 1);
		else if (*ch < 0)
		{
			int unichar = Unicode::utf8Advance(ch) & 0xFFFF;
			sprintf(digits, "%04X", unichar);
			w->writeRaw("\\u", 2); 
			w->writeRaw(digits, 4);
		}
		else
		{
			sprintf(digits, "%04X", *ch++);
			w->writeRaw("\\u", 2);
			w->writeRaw(digits, 4);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

XmlParser::XmlParser()
{
	_parser = NULL;

	XML_Memory_Handling_Suite mm;
	mm.malloc_fcn	= expat_malloc;
	mm.realloc_fcn	= expat_realloc;
	mm.free_fcn		= expat_free;

	_parser = XML_ParserCreate_MM(NULL, &mm, NULL);

	// make a circular queue
	for (int i = 0; i < COUNT_OF(_tokens) - 1; ++i)
		_tokens[i].ahead = &_tokens[i+1];
	_tokens[COUNT_OF(_tokens)-1].ahead = &_tokens[0];
}

XmlParser::~XmlParser()
{
	if (_parser)
		XML_ParserFree((XML_Parser)_parser);
}

void XmlParser::reset()
{
	XML_Parser parser = (XML_Parser)_parser;

	XML_ParserReset(parser, NULL);
	XML_SetUserData(parser, this);

	XML_SetStartElementHandler(parser, startElementHandler);
	XML_SetEndElementHandler(parser, endElementHandler);
	XML_SetCharacterDataHandler(parser, characterDataHandler);
	XML_SetCommentHandler(parser, commentHandler);

	cleanup();
}

void XmlParser::checkStatus(int st)
{
	XML_Parser parser = (XML_Parser)_parser;

	if (st == XML_STATUS_OK)
	{
		if (_next->type == Token::NONE)
		{
			_next->type = Token::FINISH;
			cleanup();
		}
	}
	else if (st == XML_STATUS_ERROR)
	{
		XML_Error err = XML_GetErrorCode(parser);

		NIT_THROW_FMT(EX_SYNTAX, "xml: %s error(%d) at line %d column %d",
			XML_ErrorString(err), err, XML_GetErrorLineNumber(parser), XML_GetErrorColumnNumber(parser));
	}
}

void XmlParser::cleanup()
{
	_reader = NULL;

	_tagStack.clear();
	_attrsStack.clear();

	_text.clear();
	_comment.clear();

	for (int i=0; i < COUNT_OF(_tokens); ++i)
		_tokens[i].clear();

	_next = &_tokens[0];
	_ahead = _next;

	_line = 0;
	_column = 0;
	_bytes = 0;
}

void XmlParser::init(const char* xml, int len)
{
	reset();

	XML_Status st = XML_Parse((XML_Parser)_parser, xml, len, true);

	checkStatus(st);
}

void XmlParser::init(StreamReader* reader)
{
	reset();

	XML_Parser parser = (XML_Parser)_parser;

	_reader = reader;

	XML_Status st = (XML_Status)feedParser();

	checkStatus(st);
}

int XmlParser::feedParser()
{
	XML_Parser parser = (XML_Parser)_parser;

	XML_Status st;

	static const int bufSize = 4096; // TODO: remove magic number

	while (true)
	{
		void* buf = XML_GetBuffer(parser, bufSize);
		if (buf == NULL)
		{
			_reader = NULL;
			NIT_THROW_FMT(EX_MEMORY, "xml: can't obtain buffer");
		}

		int bytesRead = _reader->readRaw(buf, bufSize);
		if (bytesRead < 0)
		{
			_reader = NULL;
			NIT_THROW_FMT(EX_READ, "xml: read error");
		}

		bool isFinal = bytesRead == 0;
		if (isFinal) _reader = NULL;
		st = XML_ParseBuffer(parser, bytesRead, isFinal);

		if (isFinal || st != XML_STATUS_OK)
			break;
	}

	return st;
}

bool XmlParser::next()
{
	if (_next->type == Token::FINISH) return false;

	if (_next->ahead->type != Token::NONE)
	{
		_next->clear();
		_next = _next->ahead;
		return _next->type != Token::FINISH;
	}

	XML_Parser parser = (XML_Parser)_parser;

	_next->type = Token::NONE;
	XML_Status st = XML_ResumeParser(parser);

	if (_reader && st == XML_STATUS_OK)
	{
		XML_ParsingStatus ps;
		XML_GetParsingStatus(parser, &ps);
		if (!ps.finalBuffer)
			st = (XML_Status) feedParser();
	}

	checkStatus(st);

	return _next->type != Token::FINISH;
}

bool XmlParser::open(const char** tagPatterns, int numPatterns, bool throwEx)
{
	if (_next->type == Token::NONE)
	{
		if (!throwEx) return false;
		NIT_THROW_FMT(EX_INVALID_STATE, "xml: parser not started");
	}

	if (_next->type == Token::FINISH)
	{
		if (!throwEx) return false;
		NIT_THROW_FMT(EX_INVALID_STATE, "xml: parser finished");
	}

	do
	{
		if (_next->type == Token::TAG_OPEN)
		{
			for (int i=0; i < numPatterns; ++i)
			{
				if (Wildcard::match(tagPatterns[i], _next->text))
				{
					_tagStack.push_back(_next->text);
					_attrsStack.push_back(_next->attrs);
					_line = _next->line;
					_column = _next->column;
					_bytes = _next->bytes;

					next();

					return true;
				}
			}

			break;
		}

		if (_next->type == Token::TAG_CLOSE)
			break;

	} while (next());

	if (!throwEx) return false;

	String tags;
	for (int i=0; i < numPatterns; ++i)
	{
		if (i == 0)
			tags.push_back('\'');
		else
			tags.append(", '");
		tags.append(tagPatterns[i]);
		tags.push_back('\'');
	}
	NIT_THROW_FMT(EX_SYNTAX, "xml: no matching any open [%s]", tags.c_str());
}

bool XmlParser::open(const char* tagPattern, bool throwEx)
{
	return open(&tagPattern, 1, throwEx);
}

bool XmlParser::close(const char* tag, bool throwEx)
{
	if (_next->type == Token::NONE)
		NIT_THROW_FMT(EX_INVALID_STATE, "xml: parser not started");

	if (_next->type == Token::FINISH)
		NIT_THROW_FMT(EX_INVALID_STATE, "xml: parser finished");

	int childDepth = 0;
	bool s = false;

	do
	{
		if (_next->type == Token::TAG_OPEN)
			++childDepth;

		if (childDepth == 0 && (_next->type == Token::TAG_CLOSE))
		{
			if (tag && strcmp(tag, _next->text.c_str()) != 0)
				break;

			_line = _next->line;
			_column = _next->column;
			_bytes = _next->bytes;

			_tagStack.pop_back();
			_attrsStack.pop_back();
			next();
			return true;
		}

		if (_next->type == Token::TAG_CLOSE)
			--childDepth;

		s = true;

	} while (next());

	if (!throwEx) return false;
	if (tag == NULL) tag = _tagStack.empty() ? "" : _tagStack.back().c_str();
	NIT_THROW_FMT(EX_SYNTAX, "xml: no matching close '%s'", tag);
}

const String& XmlParser::getTag()
{
	if (_tagStack.empty()) 
		return StringUtil::BLANK();

	return _tagStack.back();
}

DataRecord* XmlParser::getAttrs()
{
	if (_attrsStack.empty())
		return NULL;

	return _attrsStack.back();
}

int XmlParser::getLine()
{
	return _line;
}

int XmlParser::getColumn()
{
	return _column;
}

int XmlParser::getBytes()
{
	return _bytes;
}

const String& XmlParser::text()
{
	if (_next->type == Token::NONE)
		NIT_THROW_FMT(EX_INVALID_STATE, "xml: parser not started");

	if (_next->type == Token::FINISH)
		NIT_THROW_FMT(EX_INVALID_STATE, "xml: parser finished");

	if (_next->type == Token::TEXT)
	{
		_text = _next->text;
		_line = _next->line;
		_column = _next->column;
		_bytes = _next->bytes;
		next();
		return _text;
	}

	return StringUtil::BLANK();
}

const String& XmlParser::comment()
{
	if (_next->type == Token::NONE)
		NIT_THROW_FMT(EX_INVALID_STATE, "xml: parser not started");

	if (_next->type == Token::FINISH)
		NIT_THROW_FMT(EX_INVALID_STATE, "xml: parser finished");

	if (_next->type == Token::COMMENT)
	{
		_comment = _next->text;
		_line = _next->line;
		_column = _next->column;
		_bytes = _next->bytes;
		next();
		return _comment;
	}

	return StringUtil::BLANK();
}

void XmlParser::onStartElement(const char* name, const char** attrs)
{
	XML_Parser parser = (XML_Parser)_parser;

	Token* token = _ahead;
	assert(token->type == Token::NONE);
	_ahead = token->ahead;

	token->type = Token::TAG_OPEN;
	token->text = name;
	token->line = XML_GetCurrentLineNumber(parser);
	token->column = XML_GetCurrentColumnNumber(parser);
	token->bytes = XML_GetCurrentByteCount(parser);

	token->attrs = new DataRecord();
	for (const char** itr = attrs; *itr; ++itr)
	{
		const char* attrName = *itr++;
		const char* attrValue = *itr;

		token->attrs->set(attrName, attrValue);
	}

	XML_StopParser(parser, true);
}

void XmlParser::onEndElement(const char* name)
{
	XML_Parser parser = (XML_Parser)_parser;

	Token* token = _ahead;
	assert(token->type == Token::NONE);
	_ahead = token->ahead;

	token->type = Token::TAG_CLOSE;

	token->text = name;
	token->attrs = NULL;

	token->line = XML_GetCurrentLineNumber(parser);
	token->column = XML_GetCurrentColumnNumber(parser);
	token->bytes = XML_GetCurrentByteCount(parser);

	XML_StopParser(parser, true);
}

void XmlParser::onText(const char* s, int len)
{
	XML_Parser parser = (XML_Parser)_parser;

	Token* token = _next;

	do
	{
		if (token->type == Token::TEXT)
		{
			token->text.append(s, len);
			return;
		}

		if (token->type == Token::NONE)
			break;

		token = token->ahead;
	}
	while (token != _next);

	assert(token->type == Token::NONE);
	_ahead = token->ahead;

	token->type = Token::TEXT;
	token->text.assign(s, len);
	token->attrs = NULL;

	token->line = XML_GetCurrentLineNumber(parser);
	token->column = XML_GetCurrentColumnNumber(parser);
	token->bytes = XML_GetCurrentByteCount(parser);
}

void XmlParser::onComment(const char* data)
{
	XML_Parser parser = (XML_Parser)_parser;

	Token* token = _ahead;
	assert(token->type == Token::NONE);
	_ahead = token->ahead;

	token->type = Token::COMMENT;
	token->text = data;
	token->attrs = NULL;

	token->line = XML_GetCurrentLineNumber(parser);
	token->column = XML_GetCurrentColumnNumber(parser);
	token->bytes = XML_GetCurrentByteCount(parser);

	XML_StopParser(parser, true);
}

void XmlParser::startElementHandler(void* ctx, const char* name, const char** attrs)
{
	XmlParser* self = (XmlParser*)ctx;
	return self->onStartElement(name, attrs);
}

void XmlParser::endElementHandler(void* ctx, const char* name)
{
	XmlParser* self = (XmlParser*)ctx;
	return self->onEndElement(name);
}

void XmlParser::characterDataHandler(void* ctx, const char* s, int len)
{
	XmlParser* self = (XmlParser*)ctx;
	return self->onText(s, len);
}

void XmlParser::commentHandler(void* ctx, const char* data)
{
	XmlParser* self = (XmlParser*)ctx;
	return self->onComment(data);
}

void* XmlParser::expat_malloc(size_t size)
{
	return NIT_ALLOC(size);
}

void* XmlParser::expat_realloc(void* ptr, size_t size)
{
	return NIT_REALLOC(ptr, 0, size);
}

void XmlParser::expat_free(void* ptr)
{
	NIT_DEALLOC(ptr, 0);
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
