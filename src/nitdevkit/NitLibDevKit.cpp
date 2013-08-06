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

#include "nit/script/NitBind.h"
#include "nit/script/NitBindMacro.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_AUTODELETE(NITDEVKIT_API, nitdev::NitLexer, NULL, delete);

class NB_NitLexer : TNitClass<nitdev::NitLexer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(line),
			PROP_ENTRY_R(column),

			PROP_ENTRY_R(prevToken),

			PROP_ENTRY_R(token),
			PROP_ENTRY_R(startLine),
			PROP_ENTRY_R(startColumn),
			PROP_ENTRY_R(endLine),
			PROP_ENTRY_R(endColumn),

			PROP_ENTRY_R(stringValue),
			PROP_ENTRY_R(intValue),
			PROP_ENTRY_R(floatValue),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"),
			FUNC_ENTRY_H(start,			"(reader: StreamReader)"
			"\n"						"(str: string)"),
			FUNC_ENTRY_H(lex,			"(): TOKEN"),
			NULL
		};

		bind(v, props, funcs);

#define TOK(TOKEN) newSlot(v, -1, #TOKEN, (int)type::TK_##TOKEN)

		addStaticTable(v, "TOKEN");

		TOK(EOS);
		TOK(EOL);
		TOK(STRING_VALUE);
		TOK(IDENTIFIER);
		TOK(INT_VALUE);
		TOK(FLOAT_VALUE);
		TOK(ERROR);

		TOK(OPERATOR_START);
		TOK(EQ);
		TOK(NE);
		TOK(LE);
		TOK(GE);
		TOK(AND);
		TOK(OR);
		TOK(NEWSLOT);
		TOK(PLUSEQ);
		TOK(MINUSEQ);
		TOK(SHIFTL);
		TOK(SHIFTR);
		TOK(DOUBLE_COLON);
		TOK(PLUSPLUS);
		TOK(MINUSMINUS);
		TOK(THREEWAYSCMP);
		TOK(USHIFTR);
		TOK(VARPARAMS);
		TOK(MULEQ);
		TOK(DIVEQ);
		TOK(MODEQ);
		TOK(LAMBDA);
		TOK(WITHREF);

		TOK(KEYWORD_START);
		TOK(BASE);
		TOK(SWITCH);
		TOK(IF);
		TOK(ELSE);
		TOK(WHILE);
		TOK(BREAK);
		TOK(FOR);
		TOK(DO);
		TOK(NULL);
		TOK(FOREACH);
		TOK(IN);
		TOK(CLONE);
		TOK(FUNCTION);
		TOK(RETURN);
		TOK(TYPEOF);
		TOK(CONTINUE);
		TOK(YIELD);
		TOK(TRY);
		TOK(CATCH);
		TOK(THROW);
		TOK(RESUME);
		TOK(CASE);
		TOK(DEFAULT);
		TOK(THIS);
		TOK(CLASS);
		TOK(CONSTRUCTOR);
		TOK(IS);
		TOK(TRUE);
		TOK(FALSE);
		TOK(STATIC);
		TOK(ENUM);
		TOK(CONST);
		TOK(PROPERTY);
		TOK(REQUIRE);
		TOK(INTDIV);
		TOK(INTMOD);
		TOK(DESTRUCTOR);
		TOK(VAR);
		TOK(WITH);
		TOK(FINALLY);
		TOK(IMPORT);
		TOK(BY);

		sq_poptop(v);

#undef TOK
	}

	NB_PROP_GET(line)					{ return push(v, self(v)->getLine()); }
	NB_PROP_GET(column)					{ return push(v, self(v)->getColumn()); }
	NB_PROP_GET(prevToken)				{ return push(v, self(v)->getPrevToken()); }
	NB_PROP_GET(token)					{ return push(v, self(v)->getTokenInfo().token); }
	NB_PROP_GET(startLine)				{ return push(v, self(v)->getTokenInfo().startLine); }
	NB_PROP_GET(startColumn)			{ return push(v, self(v)->getTokenInfo().startColumn); }
	NB_PROP_GET(endLine)				{ return push(v, self(v)->getTokenInfo().endLine); }
	NB_PROP_GET(endColumn)				{ return push(v, self(v)->getTokenInfo().endColumn); }
	NB_PROP_GET(stringValue)			{ return push(v, self(v)->getTokenInfo().stringValue); }
	NB_PROP_GET(intValue)				{ return push(v, self(v)->getTokenInfo().intValue); }
	NB_PROP_GET(floatValue)				{ return push(v, self(v)->getTokenInfo().floatValue); }

	NB_CONS()							{ setSelf(v, new type()); return SQ_OK; }

	NB_FUNC(start)
	{
		if (isString(v, 2))
		{
			size_t len = sq_getsize(v, 2);
			self(v)->start(getString(v, 2), len);
		}
		else
		{
			self(v)->start(get<StreamReader>(v, 2));
		}
		return 0;
	}

	NB_FUNC(lex)						{ return push(v, (int) self(v)->lex()); }
};

////////////////////////////////////////////////////////////////////////////////

NITDEVKIT_API SQRESULT NitLibDevKit(HSQUIRRELVM v)
{
	nitdev::NB_NitLexer::Register(v);
	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
