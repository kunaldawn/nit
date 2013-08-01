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
	
	KEYWORD_START	= 300
	
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
	
	property line: int 		get _line
	property column: int	get _column
	property token: TOKEN	get _token set { _prevToken = _token; _token = value }
	property value 			get _value
	
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
				case 65279 : next(); continue // UTF-8 BOM
				
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
							if (_ch == $'>') { next(); return token = TOKEN.THREEWAYSCMP }
							return token = TOKEN.LE
						case $'<':
							next(); return token = TOKEN.SHIFTL
					}
					return token = $'<'
				
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
					next(); return token = TOKEN.AND
				
				case $'|':
					next()
					if (_ch != $'|') return token = $'|'
					next(); return token = TOKEN.OR
					
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
						return token = readNumber()
						
					if (isalpha(_ch) || _ch == $'_')
						return token = readIdentifier()
					
					if (_ch < $' ')
						throw "unexpected character(control)"

					token = _ch; next()
					return token
			}
		}
	}
	
	function lexBlockComment()
	{
		var done = false
		while (!done)
		{
			switch (_ch)
			{
				case $'*': 
					next()
					if (_ch == $'/') { done = true; next() }
					continue
					
				case $'\n':
					++_line
					next()
					continue
					
				case null:
					throw 'missing "*/" in coment'
				
				default:
					next()
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
		if (first == $'0' && (_ch == $'X' || _ch == $'x' || isodigit(_ch)))
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
					error("invalid octal number")
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
					error("too many digits for an hex number")
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
					if (type != NUMTYPE.FLOAT) error("invalid numeric format")
					type = NUMTYPE.SCIENTIIC
					temp += format("%c", _ch)
					next()
					if (_ch == $'+' || _ch == '-')
					{
						temp += format("%c", _ch)
						next()
					}
					if (!isdigit(_ch)) error("exponent expected")
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

import nitdev
	
class nitdev.Parser
{
	constructor(source)
	{
		_lexer = Lexer(source)
	}

	var _lexer: Lexer
	var _token: TOKEN

	var _stepTimeLimit: float = null
	var _stepTime: float = null
	
	function compile(stepTimeLimit = 0.001)
	{
		_stepTimeLimit = stepTimeLimit
		
		if (_stepTimeLimit != null)
			_stepTime = system.clock() + _stepTimeLimit
	
		lex()
		while (_token != null)
		{
			statement()
			if (_lexer._prevToken != $'}' && _lexer._prevToken != $';')
				optionalSemicolon()
		}
	}

	function lex()
	{
		if (_stepTimeLimit != null && system.clock() > _stepTime)
		{
//			print("++ sleeping parser")
			sleep()
			_stepTime = system.clock() + _stepTimeLimit
		}
		
		_token = _lexer.lex()
		
		// if (_token)
			// printf("#%d (%d): %s", _lexer.line, _lexer.column, _token < 500 ? TOKEN[_token] : _token)
		// else
			// print("<end>")
	}
	
	
	function statements()
	{
		while (_token != $'}' && _token != TOKEN.DEFAULT && _token != TOKEN.CASE)
		{
			statement()
			if (_lexer._prevToken != $'}' && _lexer._prevToken != $';')
				optionalSemicolon()
		}
	}

	function statement(closeFrame = true)
	{
		switch (_token)
		{
			case $';': 					lex(); break

			case TOKEN.REQUIRE:			requireStmt(); break
			case TOKEN.IMPORT:			importStmt(); break
			case TOKEN.IF:				ifStmt(); break
			case TOKEN.WHILE:			whileStmt(); break
			case TOKEN.DO:				doWhileStmt(); break
			case TOKEN.FOR:				forStmt(); break
			case TOKEN.FOREACH:			foreachStmt(); break
			case TOKEN.SWITCH:			switchStmt(); break
			case TOKEN.VAR:				localDeclStmt(); break
			case TOKEN.FUNCTION:		functionStmt(); break
			case TOKEN.CLASS:			classStmt(); break
			case TOKEN.ENUM:			enumStmt(); break
			case TOKEN.TRY:				tryCatchFinallyStmt(); break

			case TOKEN.THROW:			lex(); commaExpr(); break

			case TOKEN.CONST:			error("deprecated 'const' keyword")
			
			case TOKEN.RETURN:
			case TOKEN.YIELD:
				lex()
				if (!isEndOfStmt())
					commaExpr()
				break

			case $'{':
				beginScope()
				lex()
				statements()
				expect($'}')
				if (closeFrame)
					endScope()
				else
					endScopeNoClose()
				break
				
			case TOKEN.BREAK:			lex(); break
			case TOKEN.CONTINUE:		lex(); break
			
			default:					commaExpr()
		}
	}
	
	function requireStmt()
	{
		lex()
		expression()
	}
	
	function importStmt()
	{
		lex()
		expression()
	}
	
	function commaExpr()
	{
		expression()
		
		while (_token == $',')
		{
			lex()
			commaExpr()
		}
	}
	
	function expression()
	{
		logicalOrExpr()
		
		switch (_token)
		{
			case $'=':
			case TOKEN.NEWSLOT:
			case TOKEN.MINUSEQ:
			case TOKEN.PLUSEQ:
			case TOKEN.MULEQ:
			case TOKEN.DIVEQ:
			case TOKEN.MODEQ:
				lex(); expression()
				break
				
			case $'?':
				lex()
				expression()
				expect($':')
				expression()
		}
	}
	
	function withRefExpr()
	{
		lex()
		expression()
	}
	
	function withExpr()
	{
		lex()
		statement()
	}
	
	function logicalOrExpr()
	{
		logicalAndExpr()
		
		while (true)
		{
			if (_token != TOKEN.OR) return
			lex()
			logicalOrExpr()
		}
	}
	
	function logicalAndExpr()
	{
		bitwiseOrExpr()
		
		while (true)
		{
			if (_token != TOKEN.AND) return
			lex()
			logicalAndExpr()
		}
	}
	
	function binaryExpr(exprFunc: closure)
	{
		lex()
		exprFunc()
	}
	
	function bitwiseOrExpr()
	{
		bitwiseXorExpr()
		
		while (true)
		{
			if (_token != $'|') return
			binaryExpr(bitwiseXorExpr)
		}
	}
	
	function bitwiseXorExpr()
	{
		bitwiseAndExpr()
		
		while (true)
		{
			if (_token != $'^') return
			binaryExpr(bitwiseAndExpr)
		}
	}
	
	function bitwiseAndExpr()
	{
		compExpr()
		
		while (true)
		{
			if (_token != $'&') return
			binaryExpr(compExpr)
		}
	}
	
	function compExpr()
	{
		shiftExpr()
		
		while (true)
		{
			switch (_token)
			{
				case TOKEN.EQ:
				case TOKEN.THREEWAYSCMP:
				case $'>':
				case $'<':
				case TOKEN.GE:
				case TOKEN.LE:
				case TOKEN.NE:
					binaryExpr(shiftExpr)
					break
					
				default:
					return
			}
		}
	}
	
	function shiftExpr()
	{
		plusExpr()
		
		while (true)
		{
			switch (_token)
			{
				case TOKEN.USHIFTR:
				case TOKEN.SHIFTL:
				case TOKEN.SHIFTR:
					binaryExpr(plusExpr)
					break
					
				default:
					return
			}
		}
	}
	
	function plusExpr()
	{
		multExpr()
		
		while (true)
		{
			switch (_token)
			{
				case $'+':
				case $'-':
					binaryExpr(multExpr)
					break
					
				default:
					return
			}
		}
	}
	
	function multExpr()
	{
		prefixedExpr()
		
		while (true)
		{
			switch (_token)
			{
				case $'*':
				case $'/':
				case $'%':
				case TOKEN.INTDIV:
				case TOKEN.INTMOD:
					binaryExpr(prefixedExpr)
					break
					
				default:
					return
			}
		}
	}
	
	function prefixedExpr()
	{
		factor()
		
		while (true)
		{
			switch (_token)
			{
				case TOKEN.IN:
				case TOKEN.IS:
					binaryExpr(prefixedExpr)
					break
				
				case $'.':
					lex()
					expect(TOKEN.IDENTIFIER, true)
					break
					
				case $'[':
					if (isEndOfStmt()) return // "a = b \n [10] = 20"
					lex()
					expression()
					expect($']')
					break
					
				case TOKEN.MINUSMINUS:
				case TOKEN.PLUSPLUS:
					if (isEndOfStmt()) return
					lex()
					break
					
				case $'(':0
				case TOKEN.BY:
					if (_token != TOKEN.BY)
						lex()
					functionCallArgs()
					break
					
				case TOKEN.WITHREF:
					withRefExpr()
					break
					
				case TOKEN.WITH:
					withExpr()
					break
				
				default:
					return
			}
		}
	}
	
	function factor()
	{
		switch (_token)
		{
			case TOKEN.STRING_LITERAL:
				lex(); break
				
			case TOKEN.BASE:
				lex()
				return
				
			case TOKEN.IDENTIFIER:
			case TOKEN.CONSTRUCTOR:
			case TOKEN.VARPARAMS:
			case TOKEN.THIS:
				lex()
				break
				
			case TOKEN.DOUBLE_COLON:
				_token = $'.' // HACK: drop into prefixExpr, case $'.'
				return
				
			case TOKEN.NULL: lex(); break
			case TOKEN.INTEGER: lex(); break
			case TOKEN.FLOAT: lex(); break
			
			case TOKEN.TRUE: 
			case TOKEN.FALSE:
				lex(); break
				
			case $'$':
				lex()
				if (_token != TOKEN.STRING_LITERAL)
					error("invalid character literal")
				lex()
				break
				
			case $'[':
				lex()
				while (_token != $']')
				{
					expression()
					if (_token == $',') lex()
				}
				lex()
				break
				
			case $'{':
				lex()
				parseTable()
				if (_token == $':') // delegate shortcut
				{
					lex()
					expression()
				}
				break
				
			case TOKEN.FUNCTION:
			case $'@':
				functionExpr(); break
				
			case TOKEN.CLASS:
				classExpr(); break
				
			case $'-':
				lex()
				switch (_token)
				{
					case TOKEN.INTEGER: lex(); break
					case TOKEN.FLOAT: lex(); break
					default: unaryOp('-')
				}
				break
				
			case $'!': lex(); unaryOp('!'); break
			
			case TOKEN.TRY: tryExpr(); break
			case $'~': 
				lex()
				if (_token == TOKEN.INTEGER) lex()
				else unaryOp('~')
				break
				
			case TOKEN.TYPEOF: lex(); unaryOp('typeof'); break
			case TOKEN.RESUME: lex(); unaryOp('resume'); break
			case TOKEN.CLONE: lex(); unaryOp('close'); break
			
			case TOKEN.MINUSMINUS: lex(); unaryOp('--'); break
			case TOKEN.PLUSPLUS: lex(); unaryOp('++'); break
			
			case $'(':
				lex()
				commaExpr()
				expect($')')
				break
				
			default:
				error("expression expected")
		}
		
		return -1
	}
	
	function unaryOp(op: string)
	{
		prefixedExpr()
	}
	
	function needGet()
	{
		switch (_token)
		{
			case $'=': case $'(': case TOKEN.BY: case TOKEN.NEWSLOT:
			case TOKEN.MODEQ: case TOKEN.MULEQ: case TOKEN.DIVEQ: case TOKEN.MINUSEQ:
			case PLUSEQ:
				return false
				
			case TOKEN.PLUSPLUS: case TOKEN.MINUSMINUS:
				if (!isEndOfStmt())
					return false
		}
		
//		return (!_es.donot_get || ( _es.donot_get && (_token == _SC('.') || _token == _SC('['))));
	}
	
	function functionCallArgs()
	{
		var nargs = 1 // this
		
		if (_token != TOKEN.BY)
		{
			while (_token != $')')
			{
				expression()
				++nargs
				if (_token == $')')
					break
				expect($',')
				if (_token == $')')
					error("expression expected, found ')'")
			}
			lex()
		}
		
		if (_token == TOKEN.BY)
		{
			byFunctionExpr()
			++nargs
		}
	}
	
	function byFunctionExpr()
	{
		functionExpr()
	}
	
	function parseTable(terminator = $'}')
	{
		var enumValue = 0
		var typeTag = {}
		
		while (_token != terminator)
		{
			switch (_token)
			{
				case TOKEN.FUNCTION:
					var tk = _token
					lex()
					var id = expect(TOKEN.IDENTIFIER, true)
					expect($'(')
					createFunction(id)
					break
					
				case $'[':
					lex(); commaExpr(); expect($']')
					expect($'=')
					expression()
					break
					
				case TOKEN.STRING_LITERAL: // JSON
					var id = expect(TOKEN.STRING_LITERAL)
					if (_token == $'=')
						lex()
					else
						expect($':')
					expression()
					break
					
				default:
					var tid = typedId(true)
					
					if (_token == $'=')
					{
						lex()
						if (_token == TOKEN.INTEGER) enumValue = _lexer.value + 1
						expression()
					}
					else enumValue++  // use enum value here
			}
			
			if (_token == $',') lex() // optional comma
		}
		
		lex()
	}
	
	class TypedId
	{
		var type
		var id
	}
	
	function addTagStr(tid: TypedId, delim: int, st: string)
	{
		if (tid == null) return
		
		if (tid.type == null)
			tid.type = ""
			
		if (delim)
			tid.type += format("c", delim) + st
		else
			tid.type + st
	}
	
	function typeTag(tid: TypedId, delim: int = 0)
	{
		if (_token == $':') lex()
		
		var id = expect(TOKEN.IDENTIFIER)
		addTagStr(tid, delim, id)
		
		while (_token == $'.')
		{
			lex()
			id = expect(TOKEN.IDENTIFIER)
			addTagStr(tid, $'.', id)
		}
		
		if (_token == $'<')
		{
			lex()
			typeTag(tid, $'<')
			
			while (_token == $',' || _token == $'.')
			{
				var delim = _token
				lex()
				typeTag(tid, delim)
			}
			expect($'>')
			addTagStr(tid, $'>', "")
		}
	}
	
	function typedId(treatKeywordAsIdentifier = false): TypedId
	{
		var tid = TypedId()
		tid.id = expect(TOKEN.IDENTIFIER, treatKeywordAsIdentifier)
		
		if (_token == $':')
			typeTag(tid)
			
		return tid
	}
	
	function parseClass()
	{
		var propOrder = 1000.0
		
		while (_token != $'}')
		{
			var hasAttrs = false
			var isStatic = false
			var isProperty = false
			
			if (_token == $'[')
			{
				lex()
				parseTable($']')
				hasAttrs = true
			}
			
			if (_token == TOKEN.STATIC)
			{
				isStatic = true
				lex()
			}
			
			switch (_token)
			{
				case TOKEN.FUNCTION:
				case TOKEN.CONSTRUCTOR:
				case TOKEN.DESTRUCTOR:
					var tk = _token
					lex()
					var id
					if (tk == TOKEN.CONSTRUCTOR)
						id = "constructor"
					else if (tk == TOKEN.DESTRUCTOR)
						id = "destructor"
					else
						id = expect(TOKEN.IDENTIFIER, true)
					expect($'(')
					createFunction(id)
					break
					
				case TOKEN.PROPERTY:
					if (isStatic) warning("'property' can't be static")
					lex()
					
					if (_token == $'#')
					{
						if (!hasAttrs) warning("'#' needs attributes")
						lex()
						if (_token == TOKEN.INTEGER || _token == TOKEN.FLOAT)
							propOrder = _lexer.value
						else
							warning("invalid property #")
						lex()
					}
					
					var tid = typedId(true)
					
					var hasGetter = propertyGetSet(tid.id, "get")
					var hasSetter = propertyGetSet(tid.id, "set")
					
					if (!hasGetter && !hasSetter) warning("expected 'get' or 'set'")
					isProperty = true
					break
					
				case TOKEN.CLASS:
					isStatic = true
					lex()
					expect(TOKEN.IDENTIFIER)
					classExp()
					break
					
				case TOKEN.VAR:
					if (isStatic) warning("'var' can't be static")
					
					lex()
					var tid = typedId(true)
					
					if (_token == $'=')
					{
						lex()
						expression()
					}
					break
					
				default:
					var tid = typedId(true)
					
					if (isStatic)
					{
						expect($'=')
						expression()
					}
					else
					{
						expect($'=')
						expression()
					}
			}
			
			if (_token == $';') lex() // optional semicolon
		}
		
		lex()
	}
	
	function propertyGetSet(id: string, allowed: string)
	{
		if (_token == TOKEN.IDENTIFIER)
		{
			if (_lexer.value == "get" && allowed == "get")
			{
				lex()
				if (_token != $'{')
					createLambdaReturnExpr()
				else
					createFunction(id, false, false, false)
				return true
			}
			
			if (_lexer.value == "set" && allowed == "set")
			{
				lex()
				createFunction(id, false, false, true)
				return true
			}
		}
		return false
	}
	
	function localDeclStmt()
	{
		lex()
		if (_token == TOKEN.FUNCTION)
		{
			lex()
			var id = expect(TOKEN.IDENTIFIER)
			expect($'(')
			createFunction(id, true)
			return
		}
		
		while (true)
		{
			var tid = typedId()
			
			if (_token == $'=')
			{
				lex(); expression()
			}
			if (_token == $',') 
				lex()
			else
				break
		}
	}
	
	function ifStmt()
	{
		var hasElse = false
		
		lex()
		expect($'(')
		commaExpr()
		expect($')')
		
		beginScope()
		statement()
		if (_token != $'}' && _token != TOKEN.ELSE)
			optionalSemicolon()
		endScope()
		
		if (_token == TOKEN.ELSE)
		{
			hasElse = true
			beginScope()
			lex()
			statement()
			optionalSemicolon()
			endScope()
		}
	}
	
	function whileStmt()
	{
		lex()
		expect($'(')
		commaExpr()
		expect($')')
		
		beginScope()
		statement()
		endScope()
	}
	
	function doWhileStmt()
	{
		lex()
		beginScope()
		statement()
		endScope()
		
		if (_token == TOKEN.WHILE)
		{
			lex()
			expect($'('); commaExpr(); expect($')')
		}
		else
		{
		}
	}
	
	function forStmt()
	{
		lex()

		beginScope()
		expect($'(')
		if (_token == TOKEN.VAR)
			localDeclStmt()
		else if (_token != $';')
			commaExpr()
		expect($';')
		if (_token != $';')
			commaExpr()
		expect($';')
		if (_token != $')')
			commaExpr()
		expect($')')

		statement()

		endScope()
	}
	
	function foreachStmt()
	{
		var idxName
		
		lex()
		expect($'(')
		
		var tid = typedId()
		
		if (_token == $',')
		{
			idxName = tid.id
			lex(); tid = typedId()
		}
		else
			idxName = "@INDEX@"
			
		expect(TOKEN.IN)
		
		beginScope()
		expression()
		expect($')')
		
		statement()
		
		endScope()
	}
	
	function switchStmt()
	{
		lex()
		expect($'('); commaExpr(); expect($')')
		expect($'{')
		
		while (_token == TOKEN.CASE)
		{
			lex(); expression(); expect($':')
			beginScope()
			statements()
			endScope()
		}
		
		if (_token == TOKEN.DEFAULT)
		{
			lex(); expect($':')
			beginScope()
			statements()
			endScope()
		}

		expect($'}')
	}
	
	function functionStmt()
	{
		lex()
		var id = expect(TOKEN.IDENTIFIER)
		
		while (_token == $'.' || _token == TOKEN.DOUBLE_COLON)
		{
			if (_token == TOKEN.DOUBLE_COLON)
				warning('function definition via :: is deprecated')
			
			lex()
			id = expect(TOKEN.IDENTIFIER)
		}
		expect($'(')
		createFunction(id)
	}
	
	function classStmt()
	{
		lex()
		
		while (true)
		{
			var clsname = expect(TOKEN.IDENTIFIER)
			if (_token != $'.')
				break
			lex()
		}
		
		classExp()
	}
	
	function tryCatchFinallyStmt()
	{
		lex()
		beginScope()
		statement()
		endScope()
		
		if (_token == TOKEN.CATCH)
		{
			lex(); expect($'('); 
			var exid = expect(TOKEN.IDENTIFIER); 
			expect($')')
			
			beginScope()
			statement()
			endScope()
		}
		
		if (_token == TOKEN.FINALLY)
		{
			lex()
			beginScope()
			statement()
			endScope()
		}
	}
	
	function tryExpr()
	{
		lex()
		prefixedExpr()
		if (_token == $':')
		{
			lex()
			prefixedExpr()
		}
	}
	
	function functionExpr()
	{
		if (_token == $'@' || _token == TOKEN.BY)
		{
			lex()
			if (_token == $'(')
			{
				lex()
				createFunction(null, true, true)
			}
			else if (_token == $'{')
			{
				createFunction(null, false, false)
			}
			else
			{
				createLambdaReturnExpr()
			}
		}
		else
		{
			expect(TOKEN.FUNCTION)
			expect($'(')
			createFunction(null, true, false)
		}
	}
	
	function classExp()
	{
		if (_token == $':')
		{
			lex()
			expression()
		}
		
		if (_token == $'[')
		{
			lex()
			parseTable($']')
		}
		
		expect($'{')
		parseClass()
	}
	
	function createFunction(name: string, hasParams=true, lambda=false, isSetter=false)
	{
		var defParams = 0
		var params = [ ]
		params.push({ name = "this" })
		
		if (isSetter)
			params.push({ name = "value" })
		
		if (hasParams)
		{
			while (_token != $')')
			{
				if (_token == TOKEN.VARPARAMS)
				{
					if (defParams > 0)
						warning("function with default parameters cannot have variable number of parameters")
					params.push({ name = "..." })
					lex()
					if (_token != $')') error("expected ')')")
					break
				}
		
				var tid = typedId()
				params.push({ name = tid.id, type = tid.type })
				if (_token == $'=')
				{
					lex()
					expression()
					++defParams
				}
				else if (defParams > 0) warning("expected '='")
				
				if (_token == $',') lex()
				else if (_token != $')') warning("expected ')' or ','")
			}
			expect($')')
			
			if (_token == $':')
				typeTag(null)
				
			if (_token == TOKEN.STRING_LITERAL)
			{
				var help = expect(TOKEN.STRING_LITERAL)
			}
		}
		
		if (lambda)
		{
			if (_token == $'{')
				statement(false)
			else
			{
				expect(TOKEN.LAMBDA)
				expression()
			}
		}
		else
		{
			statement(false)
		}
	}
	
	function createLambdaReturnExpr()
	{
		if (_token == TOKEN.LAMBDA) lex()
		expression()
	}
	
	function isEndOfStmt() : bool
	{
		return (_lexer._prevToken == $'\n' || _token == null || _token == $'}' || _token == $';' )
	}
	
	function optionalSemicolon()
	{
		if (_token == $';') { lex(); return }
		if (!isEndOfStmt())
			error("end of statement expected (; or lf)")
	}
	
	function beginScope()
	{
	}
	
	function endScope()
	{
	}
	
	function endScopeNoClose()
	{
	}
	
	function error(msg)
	{
		throw msg + format(" at line %d, column %d", _lexer.line, _lexer.column)
	}
	
	function warning(msg)
	{
		throw msg + format(" at line %d, column %d", _lexer.line, _lexer.column)
	}
	
	function expect(token: TOKEN, treatKeywordAsIdentifier = false): value
	{
		if (treatKeywordAsIdentifier && _token >= TOKEN.KEYWORD_START)
			_token = TOKEN.IDENTIFIER

		if (_token != token)
		{
			throw format("expected '%s'", TOKEN.rawget(token))
		}
		
		var value = _lexer.value
		lex()
		return value
	}
}

function lexTest(text: string=null)
{
	if (text == null)
		text = session.package.open("*.nit").buffer().toString()
		
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

function parserTest(text: string=null)
{
	if (text == null)
		text = session.package.open("nitdev.parser.nit").buffer().toString()
		
	a := nitdev.Parser(text)
	
	try
		a.compile()
	catch (ex)
		printf("*** %s at line %d column %d", ex, a._lexer.line, a._lexer.column)
}

function parserTests()
{
	session.require("test/cocos")
	
	costart by
	{
		foreach (nitfile in session.package.find("*.nit"))
		{
			var tw = TimeWatch(nitfile.tostring())
			var text = nitfile.open().buffer().toString()
			parserTest(text)
		}
	}
}







