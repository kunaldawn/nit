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

#include "nitdevkit_pch.h"

#include "nitdevkit/NitDevParser.h"

NS_NITDEV_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NitLexer::NitLexer()
{
	static Keywords keywords;

	if (keywords.empty())
	{
		keywords["base"]			= TK_BASE;
		keywords["switch"]			= TK_SWITCH;
		keywords["if"]				= TK_IF;
		keywords["else"]			= TK_ELSE;
		keywords["while"]			= TK_WHILE;
		keywords["break"]			= TK_BREAK;
		keywords["for"]				= TK_FOR;
		keywords["do"]				= TK_DO;
		keywords["null"]			= TK_NULL;
		keywords["foreach"] 		= TK_FOREACH;
		keywords["in"] 				= TK_IN;
		keywords["clone"] 			= TK_CLONE;
		keywords["function"] 		= TK_FUNCTION;
		keywords["return"] 			= TK_RETURN;
		keywords["typeof"] 			= TK_TYPEOF;
		keywords["continue"] 		= TK_CONTINUE;
		keywords["yield"] 			= TK_YIELD;
		keywords["try"] 			= TK_TRY;
		keywords["catch"] 			= TK_CATCH;
		keywords["throw"] 			= TK_THROW;
		keywords["resume"] 			= TK_RESUME;
		keywords["case"] 			= TK_CASE;
		keywords["default"] 		= TK_DEFAULT;
		keywords["this"] 			= TK_THIS;
		keywords["class"] 			= TK_CLASS;
		keywords["constructor"] 	= TK_CONSTRUCTOR;
		keywords["is"] 				= TK_IS;
		keywords["true"] 			= TK_TRUE;
		keywords["false"] 			= TK_FALSE;
		keywords["static"] 			= TK_STATIC;
		keywords["enum"] 			= TK_ENUM;
		keywords["const"] 			= TK_CONST;
		keywords["property"] 		= TK_PROPERTY;
		keywords["require"] 		= TK_REQUIRE;
		keywords["div"] 			= TK_INTDIV;
		keywords["mod"] 			= TK_INTMOD;
		keywords["destructor"] 		= TK_DESTRUCTOR;
		keywords["var"] 			= TK_VAR;
		keywords["with"] 			= TK_WITH;
		keywords["finally"] 		= TK_FINALLY;
		keywords["import"] 			= TK_IMPORT;
		keywords["by"] 				= TK_BY;
	}

	setKeywords(&keywords);
}

NitLexer::Token NitLexer::lex()
{
	int tk;

	while (_ch != CHAR_EOS)
	{
		whitespace();

		switch (_ch)
		{
		case CHAR_EOS:
			return token(TK_EOS);

		case '/':
			next();
			switch (_ch)
			{
			case '*': next(); blockComment(); continue;
			case '/': lineComment(); continue;
			case '=': next(); return token(TK_DIVEQ);
			default:  return token('/');
			}
			break;

		case '=':
			next();
			if (_ch == '>') { next(); return token(TK_LAMBDA); }
			if (_ch == '=') { next(); return token(TK_EQ); }
			return token('=');

		case '<':
			next();
			if (_ch == '<') { next(); return token(TK_SHIFTL); }
			if (_ch != '=') return token('<');
			next();
			if (_ch == '>') { next(); return token(TK_THREEWAYSCMP); }
			return token(TK_LE);

		case '>':
			next();
			if (_ch == '=') { next(); return token(TK_GE); }
			if (_ch != '>') return token('>');
			next();
			if (_ch == '>') { next(); return token(TK_USHIFTR); }
			return token(TK_SHIFTR);

		case '!':
			next();
			if (_ch == '=') { next(); return token(TK_NE); }
			return token('!');

		case '@':
			next();
			if (_ch != '"' && _ch != '\'') return token('@');
			if ((tk = readString(_ch, '@')) != -1) return token(tk);
			return error("error parsing verbatim string");

		case '"':
		case '\'':
			if (readString(_ch) != -1) return token(TK_STRING_VALUE);
			return error("error parsing the string");

		case '{': case '}': case '(': case ')': case '[': case ']':
		case ';': case ',': case '?': case '^': case '~': case '$':
			return token(next());

		case '.':
			next();
			if (_ch != '.') return token('.');
			next();
			if (_ch != '.') return error("invalid token '..'");
			next();
			return token(TK_VARPARAMS);

		case '&':
			next();
			if (_ch == '&') { next(); return token(TK_AND); }
			return token('&');

		case '|':
			next();
			if (_ch == '|') { next(); return token(TK_OR); }
			return token('|');

		case ':':
			next();
			if (_ch == '=') { next(); return token(TK_NEWSLOT); }
			if (_ch == '>') { next(); return token(TK_WITHREF); }
			if (_ch == ':') { next(); return token(TK_DOUBLE_COLON); }
			return token(':');

		case '*':
			next();
			if (_ch == '=') { next(); return token(TK_MULEQ); }
			return token('*');

		case '%':
			next();
			if (_ch == '=') { next(); return token(TK_MODEQ); }
			return token('%');

		case '-':
			next();
			if (_ch == '=') { next(); return token(TK_MINUSEQ); }
			if (_ch == '-') { next(); return token(TK_MINUSMINUS); }
			return token('-');

		case '+':
			next();
			if (_ch == '=') { next(); return token(TK_PLUSEQ); }
			if (_ch == '+') { next(); return token(TK_PLUSPLUS); }
			return token('+');

		default:
			if (isdigit(_ch))
				return token(readNumber());
			else if (isId(_ch))
				return token(readId());
			else
				return error("unexpected character '%c'", _ch);
		}		
	}

	return token(TK_EOS);
}

void NitLexer::start(const void* buf, size_t len)
{
	Ref<MemoryBuffer> mem = new MemoryBuffer(buf, len);
	Ref<MemorySource> src = new MemorySource("$string", mem);
	return start(src->open());
}

////////////////////////////////////////////////////////////////////////////////

NS_NITDEV_END;