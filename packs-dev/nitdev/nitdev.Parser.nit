::nitdev := try ::nitdev : {}

////////////////////////////////////////////////////////////////////////////////

import nitdev

nitdev.TOKEN := clone nitdev.NitLexer.TOKEN

nitdev.TOKEN with 
{
	foreach (key in keys()) this[this[key]] := key
}

nitdev.TOKEN with
	for (var i=32; i<=127; ++i) this[i] := format("'%c'", i)

////////////////////////////////////////////////////////////////////////////////

import nitdev
	
class nitdev.NitParser
{
	constructor(source)
	{
		_lexer = NitLexer()
		_lexer.start(source)
	}

	var _lexer: Lexer
	var _token: TOKEN

	var _stepTimeLimit: float = null
	var _stepTime: float = null
	
	var _prevEndLine: int
	var _prevEndColumn: int
	
	var root: SyntaxNode

	function compile(stepTimeLimit = 0.001)
	{
		_stepTimeLimit = stepTimeLimit
		
		if (_stepTimeLimit != null)
			_stepTime = system.clock() + _stepTimeLimit
			
		root = beginNode('root') with
		{
			startLine = 1
			startColumn = 1
		}
	
		lex()
		while (_token)
		{
			statement()
			if (_lexer.prevToken != $'}' && _lexer.prevToken != $';')
				optionalSemicolon()
		}
		
		endNode(root)
	}

	function lex()
	{
		if (_stepTimeLimit != null && system.clock() > _stepTime)
		{
//			print("++ sleeping parser")
			sleep()
			_stepTime = system.clock() + _stepTimeLimit
		}
	
		_prevEndLine = _lexer.endLine
		_prevEndColumn = _lexer.endColumn
		
		_token = _lexer.lex()
		
		// if (_token)
			// printf("#%d (%d): %s", _lexer.line, _lexer.column, _token < 500 ? TOKEN[_token] : _token)
		// else
			// print("<end>")
	}
	
	class SyntaxNode
	{
		var parent: weak<SyntaxNode>
		
		var kind: string
		var namespace: string
		var name: string
		
		var type: TypeTag
		var params: string
		
		var startLine: int
		var startColumn: int

		var endLine: int
		var endColumn: int
		
		var children: array<SyntaxNode> = null
		
		function qualifiedName(): string
		{
			var qn = name
			if (namespace)
				qn = namespace + "." + qn
			if (parent && parent.name)
				qn = parent.qualifiedName() + "." + qn
			return qn
		}
		
		function _tostring()
		{
			var paramsStr = null
			
			if (params)
			{
				if (params.len())
					paramsStr = (params.reduce by (r, p) => r + ", " + p)
				else
					paramsStr = ""
			}
			
			
			return format("[%s] %s%s%s (%d:%d - %d:%d)",
				kind,
				qualifiedName(),
				paramsStr ? "(" + paramsStr + ")" : "",
				type ? ": " + type : "",
				startLine, startColumn,
				endLine, endColumn)
		}
	}
	
	var _nodeStack: array<SyntaxNode> = [ ]
	var _nodes: table<string, array<SyntaxNode> > = { }
	
	function beginNode(kind: string): SyntaxNode
	{
		var startLine = _lexer.startLine
		var startColumn = _lexer.startColumn
		var parent = currNode()
		
		var node = SyntaxNode() with
		{
			this.parent = parent.weak()
			this.kind = kind
			
			this.startLine = startLine
			this.startColumn = startColumn
		}
		
		_nodeStack.push(node)
		
		if (parent)
		{
			var children = parent.children ? parent.children : (parent.children = [])
			children.push(node)
		}
		
		var nodesOfKind = kind in _nodes ? _nodes[kind] : _nodes[kind] := [ ]
		nodesOfKind.push(node)
		
		return node
	}
	
	function currNode(): SyntaxNode
	{
		return _nodeStack.len() ? _nodeStack.top() : null
	}
	
	function endNode(node: SyntaxNode)
	{
		if (currNode() != node)
			throw "Invalid node stack state"
			
		_nodeStack.pop()
		
		var endLine = _prevEndLine
		var endColumn = _prevEndColumn
		
		node with
		{
			this.endLine = endLine
			this.endColumn = endColumn
		}
	}
	
	class TypeTag
	{
		var namespace : string
		var type : string
		var params : array<TypedId>
		
		function _tostring()
		{
			var str = namespace
			str = str ? str + "." + type : type
			
			if (params != null)
			{
				str += "<"
				str +=  params.reduce by (s, p) => s + ", " + p.tostring()
				str += ">"
			}
			
			return str
		}
	}
	
	function typeTag(): TypeTag
	{
		var tag = TypeTag()
		
		if (_token == $':') lex()
		
		var ns = null
		var id = expect(TOKEN.IDENTIFIER)
		
		while (_token == $'.')
		{
			lex()
			ns = ns ? (ns + "." + id) : id
			id = expect(TOKEN.IDENTIFIER)
		}

		tag.namespace = ns
		tag.type = id
		
		if (_token == $'<')
		{
			lex()
			tag.params = []
			
			var param = typeTag()
			tag.params.push(param)
			
			while (_token == $',')
			{
				lex()
				param = typeTag()
				tag.params.push(param)
			}
			
			expect($'>')
		}
		
		return tag
	}
	
	class TypedId
	{
		var type: TypeTag
		var id : string
	}
	
	function typedId(treatKeywordAsIdentifier = false): TypedId
	{
		var tid = TypedId()
		tid.id = expect(TOKEN.IDENTIFIER, treatKeywordAsIdentifier)
		
		if (_token == $':')
			tid.type = typeTag()
			
		return tid
	}

	function statements()
	{
		while (_token != $'}' && _token != TOKEN.DEFAULT && _token != TOKEN.CASE)
		{
			statement()
			if (_lexer.prevToken != $'}' && _lexer.prevToken != $';')
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
			case TOKEN.STRING_VALUE:
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
			case TOKEN.INT_VALUE: lex(); break
			case TOKEN.FLOAT_VALUE: lex(); break
			
			case TOKEN.TRUE: 
			case TOKEN.FALSE:
				lex(); break
				
			case $'$':
				lex()
				if (_token != TOKEN.STRING_VALUE)
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
					case TOKEN.INT_VALUE: lex(); break
					case TOKEN.FLOAT_VALUE: lex(); break
					default: unaryOp('-')
				}
				break
				
			case $'!': lex(); unaryOp('!'); break
			
			case TOKEN.TRY: tryExpr(); break
			case $'~': 
				lex()
				if (_token == TOKEN.INT_VALUE) lex()
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
	
	function needGet(): bool
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
					
				case TOKEN.STRING_VALUE: // JSON
					var id = expect(TOKEN.STRING_VALUE)
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
						if (_token == TOKEN.INT_VALUE) enumValue = _lexer.intValue + 1
						expression()
					}
					else enumValue++  // use enum value here
			}
			
			if (_token == $',') lex() // optional comma
		}
		
		lex()
	}
	
	function parseClass(info: ClassInfo = null)
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
					var node = beginNode('function')
					var tk = _token
					lex()
					var id
					if (tk == TOKEN.CONSTRUCTOR)
						id = "constructor"
					else if (tk == TOKEN.DESTRUCTOR)
						id = "destructor"
					else
						id = expect(TOKEN.IDENTIFIER, true)
					node.name = id
					expect($'(')
					var funcInfo = createFunction(id)
					node.type = funcInfo.type
					node.params = funcInfo.params
					endNode(node)
					break
					
				case TOKEN.PROPERTY:
					if (isStatic) warning("'property' can't be static")
					var node = beginNode('property')
					lex()
					
					if (_token == $'#')
					{
						if (!hasAttrs) warning("'#' needs attributes")
						lex()
						if (_token == TOKEN.INT_VALUE)
							propOrder = _lexer.intValue
						else if(_token == TOKEN.FLOAT_VALUE)
							propOrder = _lexer.floatValue
						else
							warning("invalid property #")
						lex()
					}
					
					var tid = typedId(true)
					node.name = tid.id
					node.type = tid.type
					
					var hasGetter = propertyGetSet(tid.id, "get")
					var hasSetter = propertyGetSet(tid.id, "set")
					
					if (!hasGetter && !hasSetter) warning("expected 'get' or 'set'")
					isProperty = true
					endNode(node)
					break
					
				case TOKEN.CLASS:
					isStatic = true
					var node = beginNode('class')
					lex()
					var id = expect(TOKEN.IDENTIFIER)
					node.name = id
					classExpr()
					endNode(node)
					break
					
				case TOKEN.VAR:
					if (isStatic) warning("'var' can't be static")
					
					var node = beginNode(isStatic ? 'static' : 'var')
					lex()
					var tid = typedId(true)
					node.name = tid.id
					node.type = tid.type
					if (_token == $'=')
					{
						lex()
						expression()
					}
					endNode(node)
					break
					
				default:
					var node = beginNode(isStatic ? 'static' : 'var')
					var tid = typedId(true)
					
					node.name = tid.id
					node.type = tid.type
					
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
					endNode(node)
			}
			
			if (_token == $';') lex() // optional semicolon
		}
		
		lex()
	}
	
	function propertyGetSet(id: string, allowed: string): bool
	{
		if (_token == TOKEN.IDENTIFIER)
		{
			var tokstr = _lexer.stringValue
			if (tokstr == "get" && allowed == "get")
			{
				lex()
				if (_token != $'{')
					createLambdaReturnExpr()
				else
					createFunction(id, false, false, false)
				return true
			}
			
			if (tokstr == "set" && allowed == "set")
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
		var node = beginNode('function')
		
		lex()
		var ns = null
		var id = expect(TOKEN.IDENTIFIER)
		
		while (_token == $'.' || _token == TOKEN.DOUBLE_COLON)
		{
			if (_token == TOKEN.DOUBLE_COLON)
				warning('function definition via :: is deprecated')
			
			lex()
			ns = ns ? (ns + "." + id) : (id)
			id = expect(TOKEN.IDENTIFIER)
		}
		
		node with
		{
			this.namespace = ns
			this.name = id
		}
		
		expect($'(')
		var funcInfo = createFunction(id)

		node.type = funcInfo.type
		node.params = funcInfo.params
		
		endNode(node)
	}
	
	function classStmt()
	{
		var node = beginNode('class')

		lex()
		
		var ns = null
		var id = expect(TOKEN.IDENTIFIER)
		
		while (_token == $'.')
		{
			lex()
			ns = ns ? (ns + "." + id) : id
			id = expect(TOKEN.IDENTIFIER)
		}
		
		node with
		{
			this.namespace = ns
			this.name = id
		}
		
		var info = ClassInfo() with
		{
			this.namespace = ns
			this.name = id
		}
		
		classExpr(info)
		
		node.type = info.base
		
		endNode(node)
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
	
	class ClassInfo
	{
		var base: TypeTag
		var namespace: string
		var name: string
	}
	
	function classExpr(info: ClassInfo=null)
	{
		if (_token == $':')
		{
			lex()
			var tag = typeTag()
			if (info)
				info.base = tag
		}
		
		if (_token == $'[')
		{
			lex()
			parseTable($']')
		}
		
		expect($'{')
		parseClass(info)
	}
	
	class FunctionInfo
	{
		class Param
		{
			var name: string
			var type: TypeTag
			var default: string
			
			function _tostring()
			{
				var str = name
				if (type != null)
					str += ": " + type
				if (this.default != null)
					str += "=" + this.default
				return str
			}
		}
		
		var type: TypeTag
		var params: array<Param> = []
		var help: string
	}
	
	function createFunction(name: string, hasParams=true, lambda=false, isSetter=false): FunctionInfo
	{
		var funcInfo = FunctionInfo()
		var params = funcInfo.params
		
		var defParams = 0
		
		if (isSetter)
			params.push(FunctionInfo.Param() with { this.name = "value" })
		
		if (hasParams)
		{
			while (_token != $')')
			{
				if (_token == TOKEN.VARPARAMS)
				{
					if (defParams > 0)
						warning("function with default parameters cannot have variable number of parameters")
					params.push(FunctionInfo.Param() with { this.name = "..." })
					lex()
					if (_token != $')') error("expected ')')")
					break
				}
		
				var param = FunctionInfo.Param()
				var tid = typedId()
				params.push(param with { this.name = tid.id, type = tid.type })
				if (_token == $'=')
				{
					// TODO capture tokens and convert to param.default
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
			{
				var type = typeTag()
				funcInfo.type = type
			}
				
			if (_token == TOKEN.STRING_VALUE)
			{
				funcInfo.help = expect(TOKEN.STRING_VALUE)
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
		
		return funcInfo
	}
	
	function createLambdaReturnExpr()
	{
		if (_token == TOKEN.LAMBDA) lex()
		expression()
	}
	
	function isEndOfStmt() : bool
	{
		return (_lexer.prevToken == $'\n' || _token == TOKEN.EOS || _token == $'}' || _token == $';' )
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
		
		var value = _lexer.stringValue
		lex()
		return value
	}
}

////////////////////////////////////////////////////////////////////////////////

function lexTest(text: string=null)
{
	if (text == null)
		text = session.package.open("*.nit").buffer().toString()
		
	l := nitdev.NitLexer()
	l.start(text)
	
	var token
	while (token = l.lex())
	{
		switch (token)
		{
			case TOKEN.IDENTIFIER: print("id(" + l.stringValue + ")"); break
			case TOKEN.STRING_VALUE: print("str(" + l.stringValue + ")"); break
			default:  print(TOKEN[token])
		}
	}
}

var indentStr = "                                                              "
function listNodes(node, indent = 0)
{
	print("-- " + indentStr.slice(0, indent*2) + node)
	if (node.children)
	{
		//var children = node.children.sort by (a,b) => a.kind != b.kind ? a.kind <=> b.kind : a.name <=> b.name
		var children = node.children
		foreach (child in children)
			listNodes(child, indent + 1)
	}
}

function parserTest(text: string=null)
{
	if (text == null)
		text = session.package.open("nitdev.parser.nit").buffer().toString()
		
	a := nitdev.NitParser(text)
	
	try
		a.compile()
	catch (ex)
		printf("*** %s at line %d column %d", ex, a._lexer.line, a._lexer.column)

	// listNodes(a.root)

	// foreach (kind, nodesOfKind in a._nodes)
		// nodesOfKind.each by (n) => print(n)
}

function parserTests()
{
	session.require("test/cocos")
	
	costart by
	{
		foreach (nitfile in session.package.find("*.nit"))
		{
			var tw = TimeWatch(nitfile.tostring())
			//var text = nitfile.open().buffer().toString(); parserTest(text)
//			parserTest(nitfile.open())
			try loadstream(nitfile.open())
		}
	}
}

