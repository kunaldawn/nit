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

#include "squirrel.h"
#include "sqxapi.h"
#include "sqvm.h"
#include "sqstring.h"
#include "sqtable.h"
#include "sqarray.h"
#include "sqfuncproto.h"
#include "sqclosure.h"
#include "squserdata.h"
#include "sqcompiler.h"
#include "sqfuncstate.h"
#include "sqclass.h"
#include "sqgc.h"

#include "nit/nit.h"
#include "nit/runtime/Exception.h"


#include <stdarg.h>

#pragma warning (disable: 4297)  // function assumed not to throw an exception but does

inline SQObjectPtr& sqx_checktype(HSQUIRRELVM v, SQInteger idx, SQObjectType type)
{
	SQObjectPtr& o = stack_get(v, idx);

	if (sqi_type(o) != type)
	{
		SQObjectPtr oval = v->PrintObjVal(o);
		v->Raise_Error(_SC("%s expected, got %s"), IdType2Name(type), IdType2Name(sqi_type(o)));
		NIT_THROW_FMT(nit::EX_SCRIPT, "%s", sqi_stringval(v->_lasterror));
	}

	return o;
}

inline SQObjectPtr& sqx_checknumeric(HSQUIRRELVM v, SQInteger idx)
{
	SQObjectPtr& o = stack_get(v, idx);

	if (!sq_isnumeric(o))
	{
		v->Raise_Error(_SC("number expected, got %s"), IdType2Name(sqi_type(o)));
		NIT_THROW_FMT(nit::EX_SCRIPT, sqi_stringval(v->_lasterror));
	}

	return o;
}

SQInteger sqx_throw(HSQUIRRELVM v, const char* err)
{
	sq_throwerror(v, err);
	NIT_THROW_FMT(nit::EX_SCRIPT, "%s", err);
	return SQ_ERROR;
}

SQBool sqx_getbool(HSQUIRRELVM v, SQInteger idx)
{
	return sqi_integer(sqx_checktype(v, idx, OT_BOOL));
}

SQInteger sqx_getinteger(HSQUIRRELVM v, SQInteger idx)
{
	return sqi_tointeger(sqx_checknumeric(v, idx));
}

SQFloat sqx_getfloat(HSQUIRRELVM v, SQInteger idx)
{
	return sqi_tofloat(sqx_checknumeric(v, idx));
}

SQInteger sqx_getonlyinteger(HSQUIRRELVM v, SQInteger idx)
{
	return sqi_tointeger(sqx_checktype(v, idx, OT_INTEGER));
}

SQFloat sqx_getonlyfloat(HSQUIRRELVM v, SQInteger idx)
{
	return sqi_tofloat(sqx_checktype(v, idx, OT_FLOAT));
}

const SQChar* sqx_getstring(HSQUIRRELVM v, SQInteger idx)
{
	return sqi_stringval(sqx_checktype(v, idx, OT_STRING));
}

HSQUIRRELVM sqx_getthread(HSQUIRRELVM v, SQInteger idx)
{
	return sqi_thread(sqx_checktype(v, idx, OT_THREAD));
}

SQUserPointer sqx_getuserpointer(HSQUIRRELVM v, SQInteger idx)
{
	return sqi_userpointer(sqx_checktype(v, idx, OT_USERPOINTER));
}

SQUIRREL_API SQInteger sqx_optinteger(HSQUIRRELVM v, SQInteger idx, SQInteger def)
{
	if (sq_isnull(stack_get(v, idx)) || (idx > sq_gettop(v)) )
		return def;

	return sqx_getinteger(v, idx);
}

SQUIRREL_API SQFloat sqx_optfloat(HSQUIRRELVM v, SQInteger idx, SQFloat def)
{
	if (sq_isnull(stack_get(v, idx)) || (idx > sq_gettop(v)) )
		return def;

	return sqx_getfloat(v, idx);
}

SQUIRREL_API const SQChar* sqx_optstring(HSQUIRRELVM v, SQInteger idx, const SQChar* def)
{
	if (sq_isnull(stack_get(v, idx)) || (idx > sq_gettop(v)) )
		return def;
	
	return sqx_getstring(v, idx);
}

SQUIRREL_API SQUserPointer sqx_getinstanceup(HSQUIRRELVM v, SQInteger idx, SQUserPointer typetag)
{
	SQObjectPtr &o = stack_get(v, idx);
	if (sqi_type(o) != OT_INSTANCE)
		sqx_throw(v, _SC("the object is not a class instance"));

	SQUserPointer p = sqi_instance(o)->_userpointer;
	if (typetag != 0) {
		SQClass *cl = sqi_instance(o)->_class;
		if (cl == NULL) return p;
		do{
			if(cl->_typetag == typetag)
				return p;
			cl = cl->_base;
		}while(cl != NULL);
		sqx_throw(v, _SC("invalid type tag"));
	}
	return p;
}

SQBool sqx_instanceof(HSQUIRRELVM v, SQInteger idx, SQUserPointer typetag)
{
	SQObjectPtr &o = stack_get(v, idx);
	if (sqi_type(o) != OT_INSTANCE) return SQFalse;

	SQUserPointer p = sqi_instance(o)->_userpointer;
	SQClass *cl = sqi_instance(o)->_class;
	if (cl == NULL) return false;
	do{
		if(cl->_typetag == typetag)
			return SQTrue;
		cl = cl->_base;
	}while(cl != NULL);

	return SQFalse;
}

void sqx_pushstringf(HSQUIRRELVM v, const SQChar* s, ...)
{
	va_list vl;
	va_start(vl, s);
	SQInteger bufSize = rsl((SQInteger)scstrlen(s)+(NUMBER_MAX_CHAR*2));
	SQChar* buf = v->_sharedstate->GetScratchPad(bufSize);
	SQInteger len = scvsnprintf(buf, bufSize, s, vl);
	if (len >= bufSize)
	{
		bufSize = len + 1;
		buf = v->_sharedstate->GetScratchPad(bufSize);
		len = scvsnprintf(buf, bufSize, s, vl);
	}
	assert(len < bufSize);
	va_end(vl);
	sq_pushstring(v, buf, len);
}

SQInteger sqx_throwfmt(HSQUIRRELVM v, const SQChar* s, ...)
{
	va_list vl;
	va_start(vl, s);
	SQInteger bufSize = rsl((SQInteger)scstrlen(s)+(NUMBER_MAX_CHAR*2));
	SQChar* buf = v->_sharedstate->GetScratchPad(bufSize);
	SQInteger len = scvsnprintf(buf, bufSize, s, vl);
	if (len >= bufSize)
	{
		bufSize = len + 1;
		buf = v->_sharedstate->GetScratchPad(bufSize);
		len = scvsnprintf(buf, bufSize, s, vl);
	}
	assert(len < bufSize);
	va_end(vl);
	sq_throwerror(v, buf);
	NIT_THROW_FMT(nit::EX_SCRIPT, "%s", buf);
	return SQ_ERROR;
}