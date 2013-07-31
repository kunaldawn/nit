
::nitdev := try ::nitdev : {}

import nitdev

nitdev.TOKEN :=
{	
	// Terms
	IDENTIFIER		= 258
	STRING_LITERAL	= 259
	INTEGER			= 260
	FLOAT			= 261
	
	// Operators
	EQ				= 262
	NE				= 263
	LE				= 264
	GE				= 265
	ARROW			= 266
	AND				= 270
	OR				= 271
	NEWSLOT			= 272
	UMINUS			= 273
	PLUSEQ			= 274
	MINUSEQ			= 275
	SHIFTL			= 276
	SHIFTR			= 277
	DOUBLE_COLON	= 278
	PLUSPLUS		= 279
	MINUSMINUS		= 280
	THREEWAYSCMP	= 281
	USHIFTR			= 282
	VARPARAMS		= 283
	MULEQ			= 284
	DIVEQ			= 285
	MODEQ			= 286
	LAMBDA			= 287
	WITHREF			= 288
	
	// Keywords
	BASE			= 300
	SWITCH			= 301
	IF				= 302
	ELSE			= 303
	WHILE			= 304
	BREAK			= 305
	FOR				= 306
	DO				= 307
	NULL			= 308
	FOREACH			= 309
	IN				= 310
	CLONE			= 311
	FUNCTION		= 312
	RETURN			= 313
	TYPEOF			= 314
	CONTINUE		= 315
	YIELD			= 316
	TRY				= 317
	CATCH			= 318
	THROW			= 319
	RESUME			= 320
	CASE			= 321
	DEFAULT			= 322
	THIS			= 323
	CLASS			= 324
	CONSTRUCTOR		= 325
	IS				= 326
	TRUE			= 327
	FALSE			= 328
	STATIC			= 329
	ENUM			= 330
	CONST			= 331
	PROPERTY		= 332
	REQUIRE			= 333
	INTDIV			= 334
	INTMOD			= 335
	DESTRUCTOR		= 336
	VAR				= 337
	WITH			= 338
	FINALLY			= 340
	IMPORT			= 341
	BY				= 342
}


nitdev.TOKEN with 
{
	foreach (key in keys()) this[this[key]] := key
}

nitdev.TOKEN with
	for (var i=32; i<=127; ++i) this[i] := format("'%c'", i)

import nitdev

class nitdev.Lexer
{
	static _keywords = 
	{	
		"base" 			= TOKEN.BASE
		"break" 		= TOKEN.BREAK
		"case" 			= TOKEN.CASE
		"catch" 		= TOKEN.CATCH
		"class" 		= TOKEN.CLASS
		"clone" 		= TOKEN.CLONE
		"constructor" 	= TOKEN.CONSTRUCTOR
		"continue"		= TOKEN.CONTINUE
		"default"		= TOKEN.DEFAULT
		"destructor"	= TOKEN.DESTRUCTOR
		"div"			= TOKEN.INTDIV
		"do"			= TOKEN.DO
		"else"			= TOKEN.ELSE
		"false"			= TOKEN.FALSE
		"for"			= TOKEN.FOR
		"foreach"		= TOKEN.FOREACH
		"function"		= TOKEN.FUNCTION
		"if"			= TOKEN.IF
		"in"			= TOKEN.IN
		"is"			= TOKEN.IS
		"mod"			= TOKEN.INTMOD
		"null"			= TOKEN.NULL
		"property"		= TOKEN.PROPERTY
		"return"		= TOKEN.RETURN
		"resume"		= TOKEN.RESUME
		"require"		= TOKEN.REQUIRE
		"static"		= TOKEN.STATIC
		"switch"		= TOKEN.SWITCH
		"this"			= TOKEN.THIS
		"throw"			= TOKEN.THROW
		"true"			= TOKEN.TRUE
		"try"			= TOKEN.TRY
		"typeof"		= TOKEN.TYPEOF
		"var"			= TOKEN.VAR
		"while"			= TOKEN.WHILE
		"yield"			= TOKEN.YIELD
		"with"			= TOKEN.WITH
		"finally"		= TOKEN.FINALLY
		"import"		= TOKEN.IMPORT
		"by"			= TOKEN.BY
	}
	
	var _line: int
	var _column: int
	var _ch: int
	var _token: TOKEN
	var _value

	var _prevToken: TOKEN
	var _lastTokenLine: int
	
	var _reader: generator
	
	constructor(source)
	{
		switch (typeof(source))
		{
			case 'string':
				_reader = @{ foreach (ch in source) yield ch }()
				break
			
			case 'function':
				_reader = @{ while (true) yield source() }()
				break
				
			case 'generator':
				_reader = source
				break
				
			default:
				throw "not supported source: " + typeof(source)
		}
		
		_line = 1
		_column = 0
		_token = null
		_value = null

		_lastTokenLine = 1
		_prevToken = null
		
		next()
	}
	
	function next()
	{
		_ch = resume _reader
		++_column
	}
	
	property token get _token set { _prevToken = _token; _token = value }
	
	function isdigit(ch: int)
	{
		return $'0' <= ch && ch <= $'9'
	}
	
	function isodigit(ch: int)
	{
		return $'0' <= ch && ch <= $'7'
	}
	
	function isxdigit(ch: int)
	{
		return $'0' <= ch && ch <= $'9' || $'a' <= ch && ch <= $'f' || $'A' <= ch && ch <= $'F'
	}
	
	function isalpha(ch: int)
	{
		return $'a' <= ch && ch <= $'z' || $'A' <= ch && ch <= $'Z'
	}
	
	function isalnum(ch: int)
	{
		return $'0' <= ch && ch <= $'9' || $'a' <= ch && ch <= $'z' || $'A' <= ch && ch <= $'Z'
	}
	
	function lex() : TOKEN
	{
		_lastTokenLine = _line
		while (_ch != null)
		{
			switch (_ch)
			{
				case $'\t': case $'\r': case $' ': next(); continue
				
				case $'\n':
					token = $'\n'
					next(); 
					++_line; _column = 1
					continue
					
				case $'/':
					next()
					switch (_ch)
					{
						case $'*': 
							next()
							lexBlockComment()
							continue
						case $'/':
							do { next() } while (_ch != $'\n' && _ch != null)
							continue
						case $'=':
							next()
							return token = TOKEN.DIVEQ
						default:
							return token = $'/'
					}
					
				case $'=':
					next()
					if (_ch == $'>') { next(); return token = TOKEN.LAMBDA }
					if (_ch != $'=') { return token = $'=' }
					else { next(); return token = TOKEN.EQ }
					
				case $'<':
					next()
					switch (_ch)
					{
						case $'=':
							next(); 
							if (_ch == '>') { next(); return token = TOKEN.THREEWAYSCMP }
							return token = TOKEN.LE
						case $'<':
							next(); return token = TOKEN.SHIFTL
					}
				
				case $'>':
					next()
					if (_ch == $'=') { next(); return token = TOKEN.GE }
					if (_ch != $'>') return token = $'>'
					next()
					if (_ch == $'>') { next(); return token = TOKEN.USHIFTR }
					return token = TOKEN.SHIFTR
					
				case $'!':
					next()
					if (_ch != $'=') return token = $'!'
					next(); return token = TOKEN.NE
					
				case $'@':
					next()
					if (_ch != $'"' && _ch != $"'") return token = $'@'
					var stype = readString(_ch, true)
					if (stype != -1) return token = stype
					throw "error parsing the string"
					
				case $'"':
				case $"'":
					var stype = readString(_ch, false)
					if (stype != -1) return token = stype
					throw "error parsing the string"
					
				case $'{': case $'}': case $'(': case $')': case $'[': case $']': 
				case $';': case $',': case $'?': case $'^': case $'~':
					var tk = _ch; next(); return token = tk
					
				case $'.':
					next()
					if (_ch != $'.') return token = $'.'
					next()
					if (_ch != $'.') throw "invalid token '..'"
					next()
					return token = TOKEN.VARPARAMS
				
				case $'&':
					next()
					if (_ch != $'&') return token = $'&'
					return token = TOKEN.AND
				
				case $'|':
					next()
					if (_ch != $'|') return token = $'|'
					return token = TOKEN.OR
					
				case $':':
					next()
					if (_ch == $'=') { next(); return token = TOKEN.NEWSLOT }
					if (_ch == $'>') { next(); return token = TOKEN.WITHREF }
					if (_ch != $':') return token = $':'
					next(); return token = TOKEN.DOUBLE_COLON
					
				case $'*':
					next()
					if (_ch == $'=') { next(); return token = TOKEN.MULEQ }
					return token = $'*'
					
				case $'%':
					next()
					if (_ch == $'=') { next(); return token = TOKEN.MODEQ }
					return token = $'%'
					
				case $'-':
					next()
					if (_ch == $'=') { next(); return token = TOKEN.MINUSEQ }
					if (_ch == $'-') { next(); return token = TOKEN.MINUSMINUS }
					return token = $'-'
					
				case $'+':
					next()
					if (_ch == $'=') { next(); return token = TOKEN.PLUSEQ }
					if (_ch == $'+') { next(); return token = TOKEN.PLUSPLUS }
					return token = $'+'
				
				case null:
					return null
					
				default:
					if (isdigit(_ch))
						return readNumber()
						
					if (isalpha(_ch) || _ch == $'_')
						return readIdentifier()
					
					if (_ch < $' ')
						throw "unexpected character(control)"

					token = ch; next()
					return token
			}
		}
	}
	
	function hexToInt(hex: string)
	{
		var value = 0
		
		foreach (ch in hex)
		{
			var digit = 0
			if ($'0' <= _ch && _ch <= $'9')
				digit = _ch - $'0'
			else if ($'a' <= _ch && _ch <= 'f')
				digit = _ch - $'a' + 10
			else if ($'A' <= _ch && _ch <= 'F')
				digit = _ch - $'A' + 10
				
			value = value * 16 + digit
		}
		
		return value
	}

	function octToInt(oct: string)
	{
		var value = 0
		
		foreach (ch in oct)
		{
			var digit = 0
			if ($'0' <= _ch && _ch <= $'9')
				digit = _ch - $'0'
				
			value = value * 8 + digit
		}
		
		return value
	}
	
	function readString(delim: int, verbatim: bool)
	{
		next()
		if (_ch == null) return -1
		
		var str = ""
		
		while (true)
		{
			while (_ch != delim)
			{
				switch (_ch)
				{
					case null:
						throw "unfinished string"
						
					case $'\n':
						if (!verbatim) throw "newline in a constant"
						str += format("%c", _ch) // TODO: slow
						next()
						++_line
						break
						
					case $'\\':
						if (verbatim) { str += '\\'; next() }
						else 
						{
							next()
							switch (_ch)
							{
								case $'x': case $'u':
									next()
									if (!isxdigit(_ch)) throw "hexadecimal number expected"
									var temp = ""
									for (var n = 0; n < 4; ++n)
										temp += format("%c", _ch)
									var unichar = hexToInt(temp)
									str += format("%c", unichar)
									break
									
								case $'t': str += '\t'; next(); break
								case $'a': str += '\a'; next(); break
								case $'b': str += '\b'; next(); break
								case $'n': str += '\n'; next(); break
								case $'r': str += '\r'; next(); break
								case $'v': str += '\v'; next(); break
								case $'f': str += '\f'; next(); break
								case $'0': str += '\0'; next(); break
								case $'\\': str += '\\'; next(); break
								case $'"': str += '"'; next(); break
								case $"'": str += "'"; next(); break
								case $'/': str += '/'; next(); break
								default: throw "unrecognised escaper char"
							}
						}
						break
						
					default:
						str += format("%c", _ch)
						next()
				}
			}
			next()
			
			if (!verbatim) break
			
			if (_ch == $'@')
			{
				next()
				break
			}
			
			str += format("%c", delim)
		}
		
		_value = str
		return TOKEN.STRING_LITERAL
	}

	function readIdentifier()
	{
		var id = ""
		do
		{
			id += format("%c", _ch) // TODO: slow, implement addChar(ch) to string
			next()
		}
		while (isalnum(_ch) || _ch == $'_')
		
		_value = id
		return getIdentifierType(_value)
	}
	
	function getIdentifierType(value)
	{
		var tk = try _keywords.rawget(value)
		
		return tk ? tk : TOKEN.IDENTIFIER
	}
	
	static NUMTYPE = 
	{
		INT = 1
		FLOAT = 2
		HEX = 3
		SCIENTIFIC = 4
		OCTAL = 5
	}
	
	function isexponent(ch: int) { return ch == 'e' || ch == 'E' }
	
	function readNumber()
	{
		var first = _ch
		var type = NUMTYPE.INT
		var temp = ""
		next()
		if (first == $'0' && (_ch == $'X' || _ch == $'x') || isodigit(_ch))
		{
			if (isodigit(_ch))
			{
				type = NUMTYPE.OCTAL
				while (isodigit(_ch))
				{
					temp += format("%c", _ch)
					next()
				}
				
				if (isdigit(_ch))
					throw "invalid octal number"
			}
			else
			{
				next()
				type = NUMTYPE.HEX
				while (isxdigit(_ch))
				{
					temp += format("%c", _ch)
					next()
				}
				if (temp.len() > 8)
					throw "too many digits for an hex number"
			}
		}
		else
		{
			temp += format("%c", first)
			while (_ch == $'.' || isdigit(_ch) || isexponent(_ch))
			{
				if (_ch == $'.') type = NUMTYPE.FLOAT
				else if (isexponent(_ch))
				{
					if (type == NUMTYPE.INT) type = NUMTYPE.FLOAT
					if (type != NUMTYPE.FLOAT) throw "invalid numeric format"
					type = NUMTYPE.SCIENTIIC
					temp += format("%c", _ch)
					next()
					if (_ch == $'+' || _ch == '-')
					{
						temp += format("%c", _ch)
						next()
					}
					if (!isdigit(_ch)) throw "exponent expected"
				}
				
				temp += format("%c", _ch)
				next()
			}
		}
		
		switch (type)
		{
			case NUMTYPE.SCIENTIFIC:
			case NUMTYPE.FLOAT:
				_value = temp.tofloat()
				return TOKEN.FLOAT
				
			case NUMTYPE.INT:
				_value = temp.tointeger()
				return TOKEN.INTEGER
				
			case NUMTYPE.HEX:
				_value = hexToInt(temp)
				return TOKEN.INTEGER
				
			case NUMTYPE.OCTAL:
				_value = octToInt(temp)
				return TOKEN.INTEGER
		}
	}
}

nitdev.Lexer._keywords with
{
	foreach (key in keys()) this[this[key]] := key
}
	
class nitdev.Analyser
{
}

function test()
{
	text := session.package.open("*.nit").buffer().toString()
	l := nitdev.Lexer(text)
	
	var token
	while (token = l.lex())
	{
		switch (token)
		{
			case TOKEN.IDENTIFIER: print("id(" + l._value + ")"); break
			case TOKEN.STRING_LITERAL: print("str(" + l._value + ")"); break
			default:  print(TOKEN[token])
		}
	}
}


