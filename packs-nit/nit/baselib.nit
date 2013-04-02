////////////////////////////////

// define dump funcs

function dump(o=null, full=false)	"(obj, showhidden=false)\n(obj, filter: wildcard)"
{
	var meta, name, filter
	
	switch (o)
	{
	case "@string":		o = getdefaultdelegate(""); break
	case "@table":		o = getdefaultdelegate({}); break
	case "@array":		o = getdefaultdelegate([]); break
	case "@class":		o = getdefaultdelegate(app.getclass()); break
	case "@instance":	o = getdefaultdelegate(app); break
	}
	
	if (type(full) == "string")
	{
		filter = "*" + full + "*"
		full = true
	}
	
	if (type(o) == "table" && o.getdelegate())
	{
		dump(o.getdelegate())
	}
	
	switch (type(o))
	{
	case "null":
	case "bool":
	case "integer":
	case "float":
	case "string":
		printf("++ %s: %s", type(o), o)
		return
		
	case "instance":
		meta = o.getclass()
		name = meta._classname + " " + o.tostring()
		break
		
	case "class":
		meta = o
		name = o.tostring()
		break
		
	case "function":
		o = o.getinfos()
		name = "function " + o.name
		if (o.help)
		{
			foreach (help in o.help.split("\n"))
				name += "\n    " + help
			o.delete("help")
		}
		else
		{
			if ("parameters" in o)
			{
				name += "("
				for (var i=1; i<o.parameters.len(); ++i)
				{
					if (i > 1) name += ", "
					name += o.parameters[i]
				}
				name += ")"
			}
			else
			{
				name += "(?)"
			}
		}
		meta = o
		break
		
	case "thread":
		name = o.tostring();
		meta = getdefaultdelegate(o)
		break
		
	default:
		meta = o
		name = type(o)
	}
	
	if (meta == null) return;
	
	var e = []
	var m = meta
		
	while (m != null)
	{
		foreach (k, v in m)
		{			
			if (!full && type(k) == "string" && k.ascii_find("_") == 0) continue;
			if (!full && (type(k) == "userdata" || type(v) == "userdata")) continue;
			
			try { v = o[k]; } catch(ex) { v = "<ERROR: " + ex + ">" }

			if (type(v) == "string")
				v = "\"" + v + "\""
			
			var rw = ""
			var prophelp = ""
			
			if (type(m) == "class")
			{
				if (m.getgetter(k)) 
				{ 
					rw += "r"; 
					prophelp = m.getgetter(k).gethelp() 
					prophelp = prophelp ? prophelp : ""
				}
				if (m.getsetter(k)) rw += "w"
				if (type(o) == "class" && rw.len()) v = "<property>"
			}
			
			var staticVal = false
			if (type(o) == "class" && v != null && rw.len() == 0)
				staticVal = true
			else if (type(o) == "instance" && m[k])
				staticVal = true
				
			if (type(v) == "function")
			{
				var help = v.gethelp()
				help = type(help) == "table" && ("help" in help) ? help["help"] : help
				if (type(help) == "string")
				{
					foreach (help in help.split("\n"))
					{
						help = help.strip()
						if (k == "constructor")
							e.append(format("3%-20s %s", k, help))
						else if (help.ascii_find("class") == 0)
							e.append(format("4%-20s %s", k, help))
						else
							e.append(format("5%-20s %s", k, help))
					}
				}
				else
					e.append(format("5%-20s %s", k, "(?): ?") );
			}
 			else if (staticVal)
 				e.append(format("0%-20s = [class] %s %s", k, v, prophelp) );
 			else if (rw.len())
				e.append(format("2%-20s = [%-2s] %s %s", k, rw, v, prophelp) );
			else
				e.append(format("1%-20s = %s %s", k, v, prophelp) );
		}
		
		m = null;
	}
	
	print(format("++ %s:", name))

	if ( (type(o) == "instance" || type(o) == "table" || type(o) == "thread") && "_dump" in o )
	{
		if (!o._dump() && !full) return;
	}

	if (type(o) != "array")
		e.sort(@(a,b) => a <=> b );
		
	for (var i=0; i < e.len(); ++i)
	{
		if (filter && !e[i].wildcard(filter, true))
			continue
			
		if (e[i].len() == 0)
			continue

		print(format("-- %s", e[i].slice(1)) )
	}
}

////////////////////////////////

// temporary Queue class - in pure script impl

class Queue
{
	_front = 1
	_back = 0
	_data = null
	
	property size 			get (_back - _front + 1)
	property empty			get (_front - 1 == _back)
	property front			get (!empty ? _data[_front] : null)
	property back			get (!empty ? _data[_back] : null)
	
	function at(index) 		"(index)"	{ return _data[_front+index]; }
	function push_front(x)	"(obj)"		{ _data[--_front] := x; }
	function push_back(x)	"(obj)"		{ _data[++_back] := x; }
	function pop_front()	"(): obj"	{ return empty ? null : _data.delete(_front++); }
	function pop_back()		"(): obj"	{ return empty ? null : _data.delete(_back--); }
	
	constructor(...)		"(...) // returns queue initialized with given params"
	{
		_data = {}
		
		foreach (x in vargv)
		{
			push_back(x)
		}
	}
	
	function all()			"(): generator // use with foreach"
	{
		for (var i=0; i<size; ++i)
		{
			yield at(i)
		}
	}
	
	function _nexti(idx)
	{
		for (var i=0; i<size; ++i)
		{
			yield at(i)
		}
	}
	
	function clear()		"()"
	{
		_data.clear()
		_front = 1
		_back = 0
	}
	
	function _dump()
	{
		print("  contents:");
		for (var i=0; i < size; ++i)
		{
			printf("  %-20d = %s", i, at(i));
		}
		print("");
		return true
	}
}

////////////////////////////////

class Stack
{
	_top = 0
	_data = null
	
	property size		get _top
	property empty		get (_top == 0)
	property top		get (!empty ? _data[_top-1] : null)
	
	function at(index)	"(index)"	{ return (!empty && index < _top) ? _data[index] : null }
	function push(x)	"(obj)"		{ _data[_top++] := x }
	function pop()		"() : obj"	{ return empty ? null : _data.delete(--_top) }
	
	constructor(...)		"(...) // returns queue initialized with given params"
	{
		_data = {}
		
		foreach (x in vargv)
		{
			push(x)
		}
	}
	
	function all()			"(): generator // use with foreach"
	{
		for (var i=0; i<size; ++i)
		{
			yield at(i);
		}
	}	
	
	function clear()		"()"
	{
		_data.clear()
		_top = 0
	}	
	
	function _dump()
	{
		print("  contents:");
		for (var i=0; i < size; ++i)
		{
			printf("  %-20d = %s", i, at(i));
		}
		print("");
		return true
	}	
}

////////////////////////////////

if (app.build != "shipping")
{
	class TimeWatch
	{
		constructor(title: string)
		{
			_title = title
			_start = system.clock()
			_lapstart = _start
			printf("$$ %s started", _title)
		}

		destructor()
		{
			var t = system.clock()
			var elapsed = t - _start
			printf("$$ %s end: elapsed %.3f", _title, elapsed)
		}
		
		function lap(str)
		{
			var t = system.clock()
			var elapsed = t - _lapstart
			var totalElapsed = t - _start
			printf("$$ %s lap '%s': elapsed %.3f, total elapsed %.3f", _title, str, elapsed, totalElapsed)
			_lapstart = t
		}

		var _title: 	string
		var _start:		float
		var _lapstart: 	float
	}
}
else
{
	function TimeWatch(ignored: string) { }
}

////////////////////////////////

// define useful functions & aliases

getdefaultdelegate({}).import := function(t: table, overwrite=false)		"(table, overwrite=false)"
{
	foreach (k, v in t)
	{
		if (!overwrite && (k in this) && this[k] != v)
		{
			printf("++ '%s' exists, not imported", k)
			continue
		}
		
		this[k] := v
	}
}

function deep_clone(t)			"(table): table // copies one by one within all nested tables"
{
	var r = clone t;
	
	if (type(r) == "table")
	{
		foreach (k, v in r)
		{
			if (type(v) == "table")
				r[k] = deep_clone(v)
			else
				r[k] = v
		}
	}
	
	return r
}

// returns a random float value [0..1]
// TODO: make native

function math.random()				"(): float // returns random value 0~1"
{
	return math.rand() / math.RAND_MAX
}

////////////////////////////////

// extend default delegates

var table = getdefaultdelegate({})

table.keys := function ()		"(): array of keys"
{
	var ret = []
	foreach (k, v in this) { ret.append(k) }
	return ret
}

table.values := function ()		"(): array of values"
{
	var ret = []
	foreach (k, v in this) { ret.append(v) }
	return ret
}

////////////////////////////////

// Extend Database.Query class

function nit.Database.Query._nexti(idx)
{
	if (stepResult != 100) return
	
	var map = null
	var cons = null
	var numColumns = this.numColumns
	
	switch (type(mapping))
	{
	case "table":
		map = []
		for (var i=0; i < numColumns; ++i)
			map.push(try mapping[getColumnName(i)] : getColumnName(i))
		cons = try mapping._call
		break
	case "array":
		map = mapping
		break
	case "class":
		cons = mapping
		break
	default:
		if (mapping is DataSchema)
			cons = @DataValue(mapping.create())
	}
	
	if (map == null)
	{
		map = []
		for (var i=0; i < numColumns; ++i)
			map.push(getColumnName(i))
	}

	do
	{
		var v = cons ? cons() : {}
		
		for (var i=0; i < numColumns; ++i)
		{
			var f = map[i]
			if (f == null) continue
			
			if (type(f) == "function")
				f.call(v, getValue(i))
			else if (type(v) == "instance")
				v[f] = getValue(i)
			else
				v[f] := getValue(i)
		}
		
		yield v // make generator
	} while (step())
}

function nit.Database.Query.bindWith(obj) "(obj) // need Binding"
{
	if (binding == null)
		throw "binding needed"
		
	switch (type(binding))
	{
	case "array":
		for (var i=1; i <= numParams; ++i)
		{
			var b = try binding[i-1]
			if (b == null) continue
			
			if (type(b) == "function")
				bind(i, try b.call(obj, getParamName(i)))
			else
				bind(i, try obj[b])
		}
		break
		
	case "function":
		for (var i=1; i <= numParams; ++i)
		{
			var param = getParamName(i)
			
			bind(i, try binding.call(obj, param))
		}
		break
		
	default:
		for (var i=1; i <= numParams; ++i)
		{
			var param = getParamName(i)
			
			var b = try binding[param]
			if (b == null) continue
			
			if (type(b) == "function")
				bind(i, try b.call(obj, param))
			else
				bind(i, try obj[b])
		}
	}
	
	return this
}

function nit.Database.Query._call(...)
{
	var nargs = vargv.len()
	
	Reset()

	if (binding && nargs == 2)
		bindWith(vargv[1])
	else if (nargs-1 != numParams)
		throw "wrong number of params"
	else
		for (var i=1; i < nargs; ++i)
			bind(i, vargv[i])
	
	step()
	
	return this
}

function nit.Database.Query.fetch(...) "(...) : []"
{
	var nargs = vargv.len()

	reset()
	
	if (binding && nargs == 1)
		bindWith(vargv[0])
	else if (nargs != numParams)
		throw "wrong number of params"
	else
		for (var i=0; i < nargs; ++i)
			bind(i+1, vargv[i])
	
	step()
	
	var ret = []
	foreach (v in this) 
		ret.push(v)
		
	return ret
}
