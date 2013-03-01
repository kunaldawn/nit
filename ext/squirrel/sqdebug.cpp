/*
	see copyright notice in squirrel.h
*/
#include "sqpcheader.h"
#include <stdarg.h>
#include "sqvm.h"
#include "sqfuncproto.h"
#include "sqclosure.h"
#include "sqstring.h"

SQRESULT sq_getfunctioninfo(HSQUIRRELVM v,SQInteger level,SQFunctionInfo *fi)
{
	SQInteger cssize = v->_callsstacksize;
	if (cssize > level) {
		SQVM::CallInfo &ci = v->_callsstack[cssize-level-1];
		if(sq_isclosure(ci._closure)) {
			SQClosure *c = sqi_closure(ci._closure);
			SQFunctionProto *proto = c->_function;
			fi->funcid = proto;
			fi->name = sqi_type(proto->_name) == OT_STRING?sqi_stringval(proto->_name):_SC("unknown");
			fi->source = sqi_type(proto->_name) == OT_STRING?sqi_stringval(proto->_sourcename):_SC("unknown");
			return SQ_OK;
		}
	}
	return sq_throwerror(v,_SC("the object is not a closure"));
}

SQRESULT sq_stackinfos(HSQUIRRELVM v, SQInteger level, SQStackInfos *si)
{
	SQInteger cssize = v->_callsstacksize;
	if (cssize > level) {
		memset(si, 0, sizeof(SQStackInfos));
		SQVM::CallInfo &ci = v->_callsstack[cssize-level-1];
		switch (sqi_type(ci._closure)) {
		case OT_CLOSURE:{
			SQFunctionProto *func = sqi_closure(ci._closure)->_function;
			if (sqi_type(func->_name) == OT_STRING)
				si->funcname = sqi_stringval(func->_name);
			if (sqi_type(func->_sourcename) == OT_STRING)
				si->source = sqi_stringval(func->_sourcename);
			si->line = func->GetLine(ci._ip);
						}
			break;
		case OT_NATIVECLOSURE:
			si->source = _SC("NATIVE");
			si->funcname =  _SC("unknown");
			if(sqi_nativeclosure(ci._closure)->_name)
				si->funcname = sqi_nativeclosure(ci._closure)->_name;
			if(sqi_nativeclosure(ci._closure)->_file)
				si->source = sqi_nativeclosure(ci._closure)->_file;
			si->line = -1;
			break;
		default: break; //shutup compiler
		}
		return SQ_OK;
	}
	return SQ_ERROR;
}

void SQVM::Raise_Error(const SQChar *s, ...)
{
	va_list vl;
	va_start(vl, s);
	SQInteger bufSize = rsl((SQInteger)scstrlen(s)+(NUMBER_MAX_CHAR*2));
	SQChar* buf = _sp(bufSize);
	SQInteger len = scvsnprintf(buf, bufSize, s, vl);
	if (len >= bufSize)
	{
		bufSize = len + 1;
		buf = _sp(bufSize);
		len = scvsnprintf(buf, bufSize, s, vl);
	}
	assert(len < bufSize);
	va_end(vl);
	_nativeException = false;
	_lasterror = SQString::Create(_ss(this),_spval,-1);
}

void SQVM::Raise_Error(const SQObjectPtr &desc)
{
	_lasterror = desc;
}

SQString *SQVM::PrintObjVal(const SQObjectPtr &o)
{
	switch(sqi_type(o)) {
	case OT_STRING: return sqi_string(o);
	case OT_INTEGER:
		scsprintf(_sp(rsl(NUMBER_MAX_CHAR+1)), _SC("%d"), sqi_integer(o));
		return SQString::Create(_ss(this), _spval);
		break;
	case OT_FLOAT:
		scsprintf(_sp(rsl(NUMBER_MAX_CHAR+1)), _SC("%.14g"), sqi_float(o));
		return SQString::Create(_ss(this), _spval);
		break;
	default:
		return SQString::Create(_ss(this), GetTypeName(o));
	}
}

void SQVM::Raise_IdxError(const SQObjectPtr &o)
{
	SQObjectPtr oval = PrintObjVal(o);
	Raise_Error(_SC("the index '%.50s' does not exist"), sqi_stringval(oval));
}

void SQVM::Raise_CompareError(const SQObject &o1, const SQObject &o2)
{
	SQObjectPtr oval1 = PrintObjVal(o1), oval2 = PrintObjVal(o2);
	Raise_Error(_SC("comparsion between '%.50s' and '%.50s'"), sqi_stringval(oval1), sqi_stringval(oval2));
}


void SQVM::Raise_ParamTypeError(SQInteger nparam,SQInteger typemask,SQInteger type)
{
	SQObjectPtr exptypes = SQString::Create(_ss(this), _SC(""), -1);
	SQInteger found = 0;	
	for(SQInteger i=0; i<16; i++)
	{
		SQInteger mask = 0x00000001 << i;
		if(typemask & (mask)) {
			if(found>0) StringCat(exptypes,SQString::Create(_ss(this), _SC("|"), -1), exptypes);
			found ++;
			StringCat(exptypes,SQString::Create(_ss(this), IdType2Name((SQObjectType)mask), -1), exptypes);
		}
	}
	Raise_Error(_SC("parameter %d has an invalid type '%s' ; expected: '%s'"), nparam, IdType2Name((SQObjectType)type), sqi_stringval(exptypes));
}
