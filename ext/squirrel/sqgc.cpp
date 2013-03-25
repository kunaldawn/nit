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

#include "sqpcheader.h"
#include "sqstate.h"
#include "sqvm.h"
#include "sqfuncproto.h"
#include "sqclosure.h"
#include "sqstring.h"
#include "sqtable.h"
#include "sqarray.h"
#include "squserdata.h"
#include "sqclass.h"

#include "nit/nit.h"

#ifndef NO_GARBAGE_COLLECTOR

extern void GCFinale(SQGC* gc);
extern void GCCredit(SQGC* gc);

////////////////////////////////////////////////////////////////////////////////////

SQGC::SQGC()
{
	_objtotal		= 0;
	_entercount		= 0;
	_leavecount		= 0;
	_mutatecount	= 0;

	_totalEnterCount	= 0;
	_totalLeaveCount	= 0;

	_markcount		= 0;
	_marktotal		= 0;

	_sweepcount		= 0;
	_sweeptotal		= 0;

	_markratio		= 5;		// in every mark step, scan 5% of whole objects
	_sweepratio		= 30;		// in every sweep step, free 30% of white objects

	_curwhite		= COLOR0;
	_curblack		= COLOR1;

	_fullcycle		= false;
	_finalizing		= false;
}

SQGC::~SQGC()
{
// 	GCCredit(this);
	assert(_finalizing);
}

void SQGC::step(SQGCINFOCALLBACK infoCallback, SQUserPointer infoUP)
{
	// mark if possible
	markLoop();

	// sweep if possible
	sweepLoop();

	if (infoCallback)
		infoCallback(this, infoUP);

	// start cycle if needed
	startCycle();

	_entercount = 0;
	_leavecount = 0;
	_mutatecount = 0;
}

void SQGC::startCycle()
{
	// Check end of previous cycle
	if (!_graylist.isEmpty() || !_whitelist.isEmpty()) return;

	// swap black & white
	_blacklist.swap(_whitelist);
	int t = _curblack; _curblack = _curwhite; _curwhite = t;

	// reset counters
	_marktotal = 0;
	_sweeptotal = 0;

	// mark root set
	if (!_finalizing)
	{
		_markcount = 0;
		_ss->MarkRootSet(this);
		_marktotal += _markcount;
	}
}

void SQGC::markLoop()
{
	_markcount = 0;

	SQInteger marklimit = (_objtotal) * _markratio / 100 + _mutatecount + 1;

	while (!_graylist.isEmpty())
	{
		if (!_fullcycle && _markcount >= marklimit) break;

		SQCollectable* obj = _graylist.pop();
		assert(obj->_gcmark == GRAY);
		obj->_gcmark = _curblack;
		_blacklist.push(obj);
		obj->Mark(this);
	}

	_marktotal += _markcount;
}

#ifndef _SHIPPING
#	define _REPORT_STICKY 1

#include "nit/runtime/LogManager.h"

static std::string ToString(SQObjectPtr o)
{
	static char buf[512];

	const char* name;
	const char* ns;
	const char* srcname;
	SQClass* cls;
	SQInstance* inst;
	SQClosure* cl;
	SQFunctionProto* fn;

	switch (sqi_type(o))
	{
	case OT_NULL:						return "null";
	case OT_STRING:						sprintf(buf, "\"%s\"", sqi_stringval(o)); return buf;
	case OT_INTEGER:					sprintf(buf, "%d", sqi_integer(o)); return buf;
	case OT_FLOAT:						sprintf(buf, "%f", sqi_float(o)); return buf;
	case OT_BOOL:						return sqi_integer(o) ? "true" : "false";

	case OT_INSTANCE:
		inst = sqi_instance(o);
		cls = inst->_class;
		if (cls == NULL)
		{
			sprintf(buf, "<instance %08x: purged>", (SQUnsignedInteger)inst);
			return buf;
		}

		name = sqi_type(cls->_methods[0].val) == OT_STRING ? sqi_stringval(cls->_methods[0].val) : "???";
		ns = sqi_type(cls->_methods[1].val) == OT_STRING ? sqi_stringval(cls->_methods[1].val) : "";
		sprintf(buf, "<instance %08x: '%s%s%s'%s>", (SQUnsignedInteger)inst, ns, *ns ? "." : "", name, cls->_typetag ? " (native)" : "");
		return buf;

	case OT_CLASS:						sprintf(buf, "<class %08x>", (SQUnsignedInteger)sqi_class(o)); return buf;
	case OT_TABLE:						sprintf(buf, "<table %08x>", (SQUnsignedInteger)sqi_table(o)); return buf;
	case OT_ARRAY:						sprintf(buf, "<array %08x>", (SQUnsignedInteger)sqi_array(o)); return buf;

	case OT_CLOSURE:
		cl = sqi_closure(o);
		fn = cl->_function;
		name = fn && sqi_type(fn->_name) == OT_STRING ? sqi_stringval(fn->_name) : "???";
		srcname = fn && sqi_type(fn->_sourcename) == OT_STRING ? sqi_stringval(fn->_sourcename) : "???";
		sprintf(buf, "<closure %08x: %s from %s line %d>", (SQUnsignedInteger)sqi_closure(o), name, srcname, fn && fn->_nlineinfos ? fn->_lineinfos[0]._line : 0); 
		return buf;

	case OT_NATIVECLOSURE:				sprintf(buf, "<nativeclosure %08x>", (SQUnsignedInteger)sqi_nativeclosure(o)); return buf;
	case OT_THREAD:						sprintf(buf, "<thread %08x>", (SQUnsignedInteger)sqi_thread(o)); return buf;

	default:							return "???";
	}
}

static void ReportSticky(SQCollectable* obj)
{
	if (SQClass* o = dynamic_cast<SQClass*>(obj))
	{
		const char* name = sqi_type(o->_methods[0].val) == OT_STRING ? sqi_stringval(o->_methods[0].val) : "<noname>";
		const char* ns = sqi_type(o->_methods[1].val) == OT_STRING ? sqi_stringval(o->_methods[1].val) : "";
		LOG(0, "*** sticky class %08x '%s%s%s' -> %08x\n", o, ns, *ns ? "." : "", name, o->_typetag);
	}
	else if (SQInstance* o = dynamic_cast<SQInstance*>(obj))
	{
		SQClass* c = o->_class;
		const char* name = sqi_type(c->_methods[0].val) == OT_STRING ? sqi_stringval(c->_methods[0].val) : "<noname>";
		const char* ns = sqi_type(c->_methods[1].val) == OT_STRING ? sqi_stringval(c->_methods[1].val) : "";
		LOG(0, "*** sticky instance %08x of '%s%s%s'%s\n", o, ns, *ns ? "." : "", name, c->_typetag ? " (native)" : "");
	}
	else if (SQClosure* o = dynamic_cast<SQClosure*>(obj))
	{
// 		const char* name = type(o->_function->_name) == OT_STRING ? _stringval(o->_function->_name) : "<noname>";
// 		const char* src = type(o->_function->_sourcename) == OT_STRING ? _stringval(o->_function->_sourcename) : "<nosrc>";
// 		int line = o->_function->_nlineinfos ? o->_function->_lineinfos[0]._line : 0;
// 		LOG(0, "*** sticky closure %08x '%s' from '%s' line %d\n", o, name, src, line);
	}
	else if (SQTable* t = dynamic_cast<SQTable*>(obj))
	{
		LOG(0, "*** sticky table %08x:\n", t);
		SQObjectPtr okey, oval;
		SQInteger idx = 0;

		while (true)
		{
			idx = t->Next(false, idx, okey, oval);
			if (idx < 0) break;

			std::string key, val;

			key = ToString(okey);
			val = ToString(oval);

			LOG(0, "***  - %-20s = %s\n", key.c_str(), val.c_str());
		}
	}
	else if (SQArray* a = dynamic_cast<SQArray*>(obj))
	{
		LOG(0, "*** sticky array %08x:\n", a);
		for (SQInteger i = 0; i < a->Size(); ++i)
		{
			SQObjectPtr oval;
			a->Get(i, oval);
			std::string val = ToString(oval);
			LOG(0, "***  [%03d] = %s\n", i, val.c_str());
		}
	}
	else if (SQClosure* c = dynamic_cast<SQClosure*>(obj))
	{
		LOG(0, "*** sticky closure %08x:\n", c);
		if (sqi_type(c->_env) != OT_NULL)
		{
			std::string env = ToString(c->_env);
			LOG(0, "***  - env: %s\n", env.c_str());
		}
	}
	else if (SQNativeClosure* o = dynamic_cast<SQNativeClosure*>(obj))
	{
// 		LOG(0, "*** sticky native closure %08x from '%s'\n", o, o->_file);
	}
	else if (SQOuter* o = dynamic_cast<SQOuter*>(obj))
	{
		LOG(0, "*** sticky outer 08x\n", o);
	}
	else if (SQVM* o = dynamic_cast<SQVM*>(obj))
	{
		LOG(0, "*** sticky vm %08x\n", o);
	}
	else
	{
// 		LOG(0, "*** sticky <%s> %08x\n", typeid(*obj).name(), obj);
	}
}

#else // ifndef _SHIPPING
#	define _REPORT_STICKY 0

static void ReportSticky(SQCollectable* obj)
{
}

#endif // ifndef _SHIPPING

void SQGC::sweepLoop()
{
	_sweepcount = 0;

	SQInteger sweeplimit = (_entercount - _leavecount) * _sweepratio / 100;
	if (sweeplimit < 0) sweeplimit = 0;
	sweeplimit += _mutatecount + 1;

	while (_graylist.isEmpty() && !_whitelist.isEmpty())
	{
		if (!_fullcycle && _sweepcount >= sweeplimit) break;

		SQUnsignedInteger pre = _leavecount;

		SQCollectable* obj = _whitelist.pop();
		assert(obj->_gcmark == _curwhite);
		++obj->_uiRef;

		++_leavecount;

		SQGC_DEBUG_STMT(--_objtotal; ++_totalLeaveCount);

		if (_REPORT_STICKY && _finalizing)
		{
			ReportSticky(obj);
		}

		obj->Finalize();
		if (--obj->_uiRef == 0)
			obj->Release();

		SQUnsignedInteger stepcount = _leavecount - pre;

		if (stepcount)
		{
			_sweepcount += stepcount;
			_sweeptotal += stepcount;
		}
	}
}

SQInteger SQGC::fullSweep()
{
	// transfer every gray to black
	while (!_graylist.isEmpty())
	{
		SQCollectable* obj = _graylist.pop();
		obj->_gcmark = _curblack;
		_blacklist.push(obj);
	}

	// transfer every white to black
	while (!_whitelist.isEmpty())
	{
		SQCollectable* obj = _whitelist.pop();
		obj->_gcmark = _curblack;
		_blacklist.push(obj);
	}

	// do a full cycle without interruption

	_fullcycle = true;

	_entercount = 0;
	_leavecount = 0;

	startCycle();
	markLoop();
	sweepLoop();

	_fullcycle = false;

	assert(_graylist.isEmpty() && _whitelist.isEmpty());

	return _sweeptotal;
}

SQInteger SQGC::finalize()
{
	_finalizing = true; // make MarkLoop() to have no effect -> no gray & everything white -> sweep all
	SQInteger sweepCount = fullSweep();
	assert(_whitelist.isEmpty() && _graylist.isEmpty() && _blacklist.isEmpty());

	return sweepCount;
}

SQBool SQGC::debugVisitAll(SQGCVISITOR visitor, SQUserPointer up)
{
	SQBool cont = true;
	cont = cont && _blacklist.debugVisit(visitor, up);
	cont = cont && _graylist.debugVisit(visitor, up);
	cont = cont && _whitelist.debugVisit(visitor, up);
	return cont;
}

////////////////////////////////////////////////////////////////////////////////////

SQGCList::SQGCList()
{
	_head._gcprev = NULL;
	_head._gcnext = &_tail;
	_tail._gcprev = &_head;
	_tail._gcnext = NULL;
}

SQGCList::~SQGCList()
{
	assert(isEmpty());
}

int SQGCList::debugCountAll()
{
	int count = 0;
	for (SQCollectable* obj = _head._gcnext; obj != &_tail; obj = obj->_gcnext, ++count);
	return count;
}

SQBool SQGCList::debugVisit(SQGCVISITOR visitor, SQUserPointer up)
{
	SQBool cont = true;
	for (SQCollectable* obj = _head._gcnext; cont && obj != &_tail; obj = obj->_gcnext)
		cont = visitor(obj, up);
	return cont;
}

void SQGCList::swap(SQGCList& other)
{
	SQCollectable* temp;

	temp = _head._gcnext;
	_head._gcnext = other._head._gcnext;
	other._head._gcnext = temp;
	_head._gcnext->_gcprev = &_head;
	other._head._gcnext->_gcprev = &other._head;

	temp = _tail._gcprev;
	_tail._gcprev = other._tail._gcprev;
	other._tail._gcprev = temp;
	_tail._gcprev->_gcnext = &_tail;
	other._tail._gcprev->_gcnext = &other._tail;
}

////////////////////////////////////////////////////////////////////////////////////

void SQGC::mark(SQObjectPtr &o)
{
	switch(sqi_type(o)){
	case OT_TABLE: mark(sqi_table(o));break;
	case OT_ARRAY: mark(sqi_array(o));break;
	case OT_USERDATA: mark(sqi_userdata(o));break;
	case OT_CLOSURE: mark(sqi_closure(o));break;
	case OT_NATIVECLOSURE: mark(sqi_nativeclosure(o));break;
	case OT_GENERATOR: mark(sqi_generator(o));break;
	case OT_THREAD: mark(sqi_thread(o));break;
	case OT_CLASS: mark(sqi_class(o));break;
	case OT_INSTANCE: mark(sqi_instance(o));break;
	case OT_OUTER: mark(sqi_outer(o));break;
	case OT_FUNCPROTO: mark(sqi_funcproto(o));break;
	default: break; //shutup compiler
	}
}

void RefTable::Mark(SQGC* gc)
{
	RefNode *nodes = (RefNode *)_nodes;
	for(SQUnsignedInteger n = 0; n < _numofslots; n++) {
		if(sqi_type(nodes->obj) != OT_NULL) {
			gc->mark(nodes->obj);
		}
		nodes++;
	}
}

void SQSharedState::MarkRootSet(SQGC* gc)
{
	gc->mark(_root_vm);
	gc->mark(_root_table);
	gc->mark(_registry);
	gc->mark(_consts);
	gc->mark(_metamethodsmap);
	gc->mark(_null_default_delegate);
 	gc->mark(_table_default_delegate);
	gc->mark(_array_default_delegate);
	gc->mark(_string_default_delegate);
	gc->mark(_number_default_delegate);
	gc->mark(_generator_default_delegate);
	gc->mark(_thread_default_delegate);
	gc->mark(_closure_default_delegate);
	gc->mark(_class_default_delegate);
	gc->mark(_instance_default_delegate);
	gc->mark(_weakref_default_delegate);
	gc->mark(_nativeweakref_default_delegate);
	_refs_table.Mark(gc);
}

void SQVM::Mark(SQGC* gc)
{
	gc->mark(_lasterror);
	gc->mark(_errorhandler);
	gc->mark(_debughook_closure);
	gc->mark(_threadlocal);
	gc->mark(temp_reg);
	for(SQUnsignedInteger i = 0; i < _stack.size(); i++) gc->mark(_stack[i]);
	for(SQInteger k = 0; k < _callsstacksize; k++) gc->mark(_callsstack[k]._closure);
}

void SQArray::Mark(SQGC* gc)
{
	SQInteger len = _values.size();
	for(SQInteger i = 0;i < len; i++) gc->mark(_values[i]);
}

void SQTable::Mark(SQGC* gc)
{
	gc->mark(_delegate);
	SQInteger len = _numofnodes;
	for(SQInteger i = 0; i < len; i++){
		gc->mark(_nodes[i].key);
		gc->mark(_nodes[i].val);
	}
}

void SQClass::Mark(SQGC* gc)
{
	gc->mark(_members);
	gc->mark(_base);
	gc->mark(_attributes);
	for(SQUnsignedInteger i =0; i< _defaultvalues.size(); i++) {
		gc->mark(_defaultvalues[i].val);
		gc->mark(_defaultvalues[i].attrs);
	}
	for(SQUnsignedInteger j =0; j< _methods.size(); j++) {
		gc->mark(_methods[j].val);
		gc->mark(_methods[j].attrs);
	}
	for(SQUnsignedInteger k =0; k< _metamethods.size(); k++) {
		gc->mark(_metamethods[k]);
	}
	for(SQUnsignedInteger l=0; l< _getters.size(); l++) {
		gc->mark(_getters[l].val);
		gc->mark(_getters[l].attrs);
		gc->mark(_setters[l].val);
	}
}

void SQInstance::Mark(SQGC* gc)
{
	if (IsPurged()) return;
	gc->mark(_class);
	SQUnsignedInteger nvalues = _class->_defaultvalues.size();
	for(SQUnsignedInteger i =0; i< nvalues; i++) gc->mark(_values[i]);
}

void SQGenerator::Mark(SQGC* gc)
{
	for(SQUnsignedInteger i = 0; i < _stack.size(); i++) gc->mark(_stack[i]);
	gc->mark(_closure);
}

void SQClosure::Mark(SQGC* gc)
{
	gc->mark(_base);
	gc->mark(_env);
	gc->mark(_function);
	SQFunctionProto *fp = _function;
	for(SQInteger i = 0; i < fp->_noutervalues; i++) gc->mark(_outervalues[i]);
	for(SQInteger i = 0; i < fp->_ndefaultparams; i++) gc->mark(_defaultparams[i]);
}

void SQNativeClosure::Mark(SQGC* gc)
{
	for(SQUnsignedInteger i = 0; i < _outervalues.size(); i++) gc->mark(_outervalues[i]);
	gc->mark(_env);
	gc->mark(_help);
}

void SQOuter::Mark(SQGC* gc)
{
	// If the valptr points to a closed value, that value is alive
	if(_valptr == &_value)
		gc->mark(_value);
}

void SQUserData::Mark(SQGC* gc)
{
	gc->mark(_delegate);
}

void SQFunctionProto::Mark(SQGC* gc)
{
	for(SQInteger i = 0; i < _nliterals; i++) gc->mark(_literals[i]);
	for(SQInteger i = 0; i < _nfunctions; i++) gc->mark(_functions[i]);
	gc->mark(_help);
	gc->mark(_imports);
}

#endif // NO_GARBAGE_COLLECTOR
