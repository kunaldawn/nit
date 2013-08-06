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

#include "nit/data/ParserUtil.h"

NS_NITDEV_BEGIN

////////////////////////////////////////////////////////////////////////////////

class NITDEVKIT_API NitLexer : public LexerBase
{
public:
	enum NitToken
	{
		TK_OPERATOR_START = 256,

		TK_EQ,				// ==
		TK_NE,				// !=
		TK_LE,				// <=
		TK_GE,				// >=
		TK_AND,				// &&
		TK_OR,				// ||
		TK_NEWSLOT,			// :=
		TK_PLUSEQ,			// +=
		TK_MINUSEQ,			// -=
		TK_SHIFTL,			// <<
		TK_SHIFTR,			// >>
		TK_DOUBLE_COLON,	// ::
		TK_PLUSPLUS,		// ++
		TK_MINUSMINUS,		// --
		TK_3WAYSCMP,		// <=>
		TK_USHIFTR,			// >>>
		TK_VARPARAMS,		// ...
		TK_MULEQ,			// *=
		TK_DIVEQ,			// /=
		TK_MODEQ,			// %=
		TK_LAMBDA,			// =>
		TK_WITHREF,			// :>

		TK_KEYWORD_START = 1024,

		TK_BASE,
		TK_SWITCH,
		TK_IF,
		TK_ELSE,
		TK_WHILE,
		TK_BREAK,
		TK_FOR,
		TK_DO,
		TK_NULL,
		TK_FOREACH,
		TK_IN,
		TK_CLONE,
		TK_FUNCTION,
		TK_RETURN,
		TK_TYPEOF,
		TK_CONTINUE,
		TK_YIELD,
		TK_TRY,
		TK_CATCH,
		TK_THROW,
		TK_RESUME,
		TK_CASE,
		TK_DEFAULT,
		TK_THIS,
		TK_CLASS,
		TK_CONSTRUCTOR,
		TK_IS,
		TK_TRUE,
		TK_FALSE,
		TK_STATIC,
		TK_ENUM,
		TK_CONST,
		TK_PROPERTY,
		TK_REQUIRE,
		TK_INTDIV,
		TK_INTMOD,
		TK_DESTRUCTOR,
		TK_VAR,
		TK_WITH,
		TK_FINALLY,
		TK_IMPORT,
		TK_BY,
	};

public:
	NitLexer();

public:
	virtual Token						lex();

public:
	void								start(StreamReader* reader)				{ return LexerBase::start(reader); }
	void								start(const void* buf, size_t len);
	void								start(const String& str)				{ start(str.c_str(), str.size()); }
};
////////////////////////////////////////////////////////////////////////////////

NS_NITDEV_END