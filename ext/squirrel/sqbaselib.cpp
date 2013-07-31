/*
	see copyright notice in squirrel.h
*/
#include "sqpcheader.h"
#include "sqvm.h"
#include "sqstring.h"
#include "sqtable.h"
#include "sqarray.h"
#include "sqfuncproto.h"
#include "sqclosure.h"
#include "sqclass.h"
#include "squserdata.h"
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "nit/nit.h"

bool str2num(const SQChar *s,SQObjectPtr &res)
{
	SQChar *end;
	if(scstrstr(s,_SC("."))){
		SQFloat r = SQFloat(scstrtod(s,&end));
		if(s == end) return false;
		res = r;
		return true;
	}
	else{
		SQInteger r = SQInteger(scstrtol(s,&end,10));
		if(s == end) return false;
		res = r;
		return true;
	}
}

static SQInteger base_dummy(HSQUIRRELVM v)
{
	return 0;
}

#ifndef NO_GARBAGE_COLLECTOR
static SQInteger base_collectgarbage(HSQUIRRELVM v)
{
	sq_pushinteger(v, sq_collectgarbage(v));
	return 1;
}
#endif

static SQInteger base_getroottable(HSQUIRRELVM v)
{
	v->Push(_ss(v)->_root_table);
	return 1;
}

static SQInteger base_getconsttable(HSQUIRRELVM v)
{
	v->Push(_ss(v)->_consts);
	return 1;
}


static SQInteger base_setroottable(HSQUIRRELVM v)
{
	SQObjectPtr o = _ss(v)->_root_table;
	if(SQ_FAILED(sq_setroottable(v))) return SQ_ERROR;
	v->Push(o);
	return 1;
}

static SQInteger base_setconsttable(HSQUIRRELVM v)
{
	SQObjectPtr o = _ss(v)->_consts;
	if(SQ_FAILED(sq_setconsttable(v))) return SQ_ERROR;
	v->Push(o);
	return 1;
}

static SQInteger base_seterrorhandler(HSQUIRRELVM v)
{
	sq_seterrorhandler(v);
	return 0;
}

static SQInteger base_setdebughook(HSQUIRRELVM v)
{
	sq_setdebughook(v);
	return 0;
}

static SQInteger base_enabledebuginfo(HSQUIRRELVM v)
{
	SQObjectPtr &o=stack_get(v,2);
	
	sq_enabledebuginfo(v,v->IsFalse(o)?SQFalse:SQTrue);
	return 0;
}

static SQInteger base_getstackinfos(HSQUIRRELVM v)
{
	SQInteger level;
	SQStackInfos si;
	SQInteger seq = 0;
	const SQChar *name = NULL;
	sq_getinteger(v, -1, &level);
	if (SQ_SUCCEEDED(sq_stackinfos(v, level, &si)))
	{
		const SQChar *fn = _SC("unknown");
		const SQChar *src = _SC("unknown");
		if(si.funcname)fn = si.funcname;
		if(si.source)src = si.source;
		sq_newtable(v);
		sq_pushstring(v, _SC("func"), -1);
		sq_pushstring(v, fn, -1);
		sq_createslot(v, -3);
		sq_pushstring(v, _SC("src"), -1);
		sq_pushstring(v, src, -1);
		sq_createslot(v, -3);
		sq_pushstring(v, _SC("line"), -1);
		sq_pushinteger(v, si.line);
		sq_createslot(v, -3);
		sq_pushstring(v, _SC("locals"), -1);
		sq_newtable(v);
		seq=0;
		while ((name = sq_getlocal(v, level, seq))) {
			sq_pushstring(v, name, -1);
			sq_push(v, -2);
			sq_createslot(v, -4);
			sq_pop(v, 1);
			seq++;
		}
		sq_createslot(v, -3);
		return 1;
	}

	return 0;
}

static SQInteger base_assert(HSQUIRRELVM v)
{
	if(v->IsFalse(stack_get(v,2))){
		if (sq_gettop(v) > 2)
		{
			v->_lasterror = stack_get(v, 3);
			return SQ_ERROR;
		}
		else
			return sq_throwerror(v,_SC("assertion failed"));
	}
	return 0;
}

static SQInteger get_slice_params(HSQUIRRELVM v,SQInteger &sidx,SQInteger &eidx,SQObjectPtr &o)
{
	SQInteger top = sq_gettop(v);
	sidx=0;
	eidx=0;
	o=stack_get(v,1);
	SQObjectPtr &start=stack_get(v,2);
	if(sqi_type(start)!=OT_NULL && sq_isnumeric(start)){
		sidx=sqi_tointeger(start);
	}
	if(top>2){
		SQObjectPtr &end=stack_get(v,3);
		if(sq_isnumeric(end)){
			eidx=sqi_tointeger(end);
		}
	}
	else {
		eidx = sq_getsize(v,1);
	}
	return 1;
}

static SQInteger base_print(HSQUIRRELVM v)
{
	const SQChar *str;
	sq_tostring(v,2);
	sq_getstring(v,-1,&str);
	if(_ss(v)->_printfunc) _ss(v)->_printfunc(v,_SC("%s"),str);
	return 0;
}

static SQInteger base_error(HSQUIRRELVM v)
{
	const SQChar *str;
	sq_tostring(v,2);
	sq_getstring(v,-1,&str);
	if(_ss(v)->_errorfunc) _ss(v)->_errorfunc(v,_SC("%s"),str);
	return 0;
}

static SQInteger base_compilestring(HSQUIRRELVM v)
{
	SQInteger nargs=sq_gettop(v);
	const SQChar *src=NULL,*name=_SC("unnamedbuffer");
	SQInteger size;
	sq_getstring(v,2,&src);
	size=sq_getsize(v,2);
	if(nargs>2){
		sq_getstring(v,3,&name);
	}
	if(SQ_SUCCEEDED(sq_compilebuffer(v,src,size,name,SQFalse)))
		return 1;
	else
		return SQ_ERROR;
}

static SQInteger base_newthread(HSQUIRRELVM v)
{
	SQObjectPtr &func = stack_get(v,2);
	SQInteger stksize = (sqi_closure(func)->_function->_stacksize << 1) +2;
	HSQUIRRELVM newv = sq_newthread(v, (stksize < MIN_STACK_OVERHEAD + 2)? MIN_STACK_OVERHEAD + 2 : stksize);
	sq_move(newv,v,-2);
	return 1;
}

static SQInteger base_suspend(HSQUIRRELVM v)
{
	return sq_suspendvm(v);
}

static SQInteger base_array(HSQUIRRELVM v)
{
	SQArray *a;
	SQObject &size = stack_get(v,2);
	if(sq_gettop(v) > 2) {
		a = SQArray::Create(_ss(v),0);
		a->Resize(sqi_tointeger(size),stack_get(v,3));
	}
	else {
		a = SQArray::Create(_ss(v),sqi_tointeger(size));
	}
	v->Push(a);
	return 1;
}

static SQInteger base_type(HSQUIRRELVM v)
{
	SQObjectPtr &o = stack_get(v,2);
	v->Push(SQString::Create(_ss(v),GetTypeName(o),-1));
	return 1;
}

static SQInteger base_rawid(HSQUIRRELVM v)
{
	v->Push(v->ToRawID(stack_get(v, 2)));
	return 1;
}

static SQInteger base_getthread(HSQUIRRELVM v)
{
	v->Push(v);
	return 1;
}

static SQRegFunction base_funcs[]={
	//generic
	{_SC("seterrorhandler"),base_seterrorhandler,2, NULL,__FILE__,		_SC("(func)")},
	{_SC("setdebughook"),base_setdebughook,2, NULL,__FILE__,			_SC("(hook_func)")},
	{_SC("enabledebuginfo"),base_enabledebuginfo,2, NULL,__FILE__,		_SC("(flag: bool)")},
	{_SC("getthread"),base_getthread,1, NULL,__FILE__,					_SC("(): currentthread")},
	{_SC("getstackinfos"),base_getstackinfos,2, _SC(".n"),__FILE__,		_SC("(level: int): table")},
	{_SC("getroottable"),base_getroottable,1, NULL,__FILE__,			_SC("(): table")},
	{_SC("setroottable"),base_setroottable,2, NULL,__FILE__,			_SC("(table): table // returns original")},
	{_SC("getconsttable"),base_getconsttable,1, NULL,__FILE__,			_SC("(): table")},
	{_SC("setconsttable"),base_setconsttable,2, NULL,__FILE__,			_SC("(table): table // returns original")},
	{_SC("assert"),base_assert,-1, NULL,__FILE__,						_SC("(cond: bool[, msg: string])")},
	{_SC("print"),base_print,2, NULL,__FILE__,							_SC("(msg: string) // to stdout")},
	{_SC("error"),base_error,2, NULL,__FILE__,							_SC("(msg: string) // to stderr")},
	{_SC("compilestring"),base_compilestring,-2, _SC(".ss"),__FILE__,	_SC("(string,[buffername]): closure")},
	{_SC("newthread"),base_newthread,2, _SC(".c"),__FILE__,				_SC("(threadfunc: closure): thread")},
	{_SC("suspend"),base_suspend,-1, NULL,__FILE__,						_SC("(): wakeupval")},
	{_SC("array"),base_array,-2, _SC(".n"),__FILE__,					_SC("(size, fill=null): array[size]")},
	{_SC("type"),base_type,2, NULL,__FILE__,							_SC("(obj): string")},
	{_SC("rawid"),base_rawid,2,NULL,__FILE__,							_SC("(obj): int")},
#ifndef NO_GARBAGE_COLLECTOR
	{_SC("collectgarbage"),base_collectgarbage,1, _SC("t"),__FILE__,	_SC("()")},
#endif
	{0,0}
};

void sq_base_register(HSQUIRRELVM v)
{
	SQInteger i=0;
	sq_pushroottable(v);
	while(base_funcs[i].name!=0) {
		sq_pushstring(v,base_funcs[i].name,-1);
		sq_newclosure(v,base_funcs[i].f,0);
		sq_setnativeclosureinfo(v,-1,base_funcs[i].name,base_funcs[i].file);
		sq_setparamscheck(v,base_funcs[i].nparamscheck,base_funcs[i].typemask);
		if (base_funcs[i].help)
		{
			sq_pushstring(v,base_funcs[i].help,-1);
			sq_sethelp(v,-2);
		}
		sq_createslot(v,-3);
		i++;
	}
	sq_pop(v,1);
	sq_pushconsttable(v);
	sq_pushstring(v,_SC("_version_"),-1);
	sq_pushstring(v,SQUIRREL_VERSION,-1);
	sq_createslot(v,-3);
	sq_pushstring(v,_SC("_charsize_"),-1);
	sq_pushinteger(v,sizeof(SQChar));
	sq_createslot(v,-3);
	sq_pushstring(v,_SC("_intsize_"),-1);
	sq_pushinteger(v,sizeof(SQInteger));
	sq_createslot(v,-3);
	sq_pushstring(v,_SC("_floatsize_"),-1);
	sq_pushinteger(v,sizeof(SQFloat));
	sq_createslot(v,-3);
	sq_pushstring(v,_SC("_objsize_"),-1);
	sq_pushinteger(v,sizeof(SQObject));
	sq_createslot(v,-3);
	sq_pushstring(v,_SC("_stringsize_"),-1);
	sq_pushinteger(v,sizeof(SQString));
	sq_createslot(v,-3);
	sq_pushstring(v,_SC("_valuesize_"),-1);
	sq_pushinteger(v,sizeof(SQObjectValue));
	sq_createslot(v,-3);
	sq_pushstring(v,_SC("_instancesize_"),-1);
	sq_pushinteger(v,sizeof(SQInstance));
	sq_createslot(v,-3);
	sq_pushstring(v,_SC("_tablesize_"),-1);
	sq_pushinteger(v,sizeof(SQTable));
	sq_createslot(v,-3);
	sq_pushstring(v,_SC("_closuresize_"),-1);
	sq_pushinteger(v,sizeof(SQClosure));
	sq_createslot(v,-3);
	sq_pushstring(v,_SC("_outersize_"),-1);
	sq_pushinteger(v,sizeof(SQOuter));
	sq_createslot(v,-3);
	sq_pushstring(v,_SC("_arraysize_"),-1);
	sq_pushinteger(v,sizeof(SQArray));
	sq_createslot(v,-3);
	sq_pushstring(v,_SC("_threadsize_"),-1);
	sq_pushinteger(v,sizeof(SQVM));
	sq_createslot(v,-3);
	sq_pushstring(v,_SC("_weakrefsize_"),-1);
	sq_pushinteger(v,sizeof(SQWeakRef));
	sq_createslot(v,-3);
	sq_pushstring(v,_SC("_nativeclosuresize_"),-1);
	sq_pushinteger(v,sizeof(SQNativeClosure));
	sq_createslot(v,-3);
	sq_pushstring(v,_SC("_generatorsize_"),-1);
	sq_pushinteger(v,sizeof(SQGenerator));
	sq_createslot(v,-3);
	sq_pushstring(v,_SC("_userdatasize_"),-1);
	sq_pushinteger(v,sizeof(SQUserData));
	sq_createslot(v,-3);
	sq_pushstring(v,_SC("_collectablesize_"),-1);
	sq_pushinteger(v,sizeof(SQCollectable));
	sq_createslot(v,-3);
	sq_pushstring(v,_SC("_refcountedsize_"),-1);
	sq_pushinteger(v,sizeof(SQRefCounted));
	sq_createslot(v,-3);
	sq_pop(v,1);
}

static SQInteger default_delegate_len(HSQUIRRELVM v)
{
	v->Push(SQInteger(sq_getsize(v,1)));
	return 1;
}

static SQInteger default_delegate_tofloat(HSQUIRRELVM v)
{
	SQObjectPtr &o=stack_get(v,1);
	switch(sqi_type(o)){
	case OT_STRING:{
		SQObjectPtr res;
		if(str2num(sqi_stringval(o),res)){
			v->Push(SQObjectPtr(sqi_tofloat(res)));
			break;
		}}
		return sq_throwerror(v, _SC("cannot convert the string"));
		break;
	case OT_INTEGER:case OT_FLOAT:
		v->Push(SQObjectPtr(sqi_tofloat(o)));
		break;
	case OT_BOOL:
		v->Push(SQObjectPtr((SQFloat)(sqi_integer(o)?1:0)));
		break;
	default:
		v->Push(_null_);
		break;
	}
	return 1;
}

static SQInteger default_delegate_tointeger(HSQUIRRELVM v)
{
	SQObjectPtr &o=stack_get(v,1);
	switch(sqi_type(o)){
	case OT_STRING:{
		SQObjectPtr res;
		if(str2num(sqi_stringval(o),res)){
			v->Push(SQObjectPtr(sqi_tointeger(res)));
			break;
		}}
		return sq_throwerror(v, _SC("cannot convert the string"));
		break;
	case OT_INTEGER:case OT_FLOAT:
		v->Push(SQObjectPtr(sqi_tointeger(o)));
		break;
	case OT_BOOL:
		v->Push(SQObjectPtr(sqi_integer(o)?(SQInteger)1:(SQInteger)0));
		break;
	default:
		v->Push(_null_);
		break;
	}
	return 1;
}

static SQInteger default_delegate_tostring(HSQUIRRELVM v)
{
	sq_tostring(v,1);
	return 1;
}

static SQInteger obj_delegate_weakref(HSQUIRRELVM v)
{
	sq_weakref(v, 1, true);
	return 1;
}

static SQInteger obj_clear(HSQUIRRELVM v)
{
	return sq_clear(v,-1);
}


static SQInteger number_delegate_tochar(HSQUIRRELVM v)
{
	SQObject &o=stack_get(v,1);
	SQChar c = (SQChar)sqi_tointeger(o);
	v->Push(SQString::Create(_ss(v),(const SQChar *)&c,1));
	return 1;
}

/////////////////////////////////////////////////////////////////
//NULL DEFAULT DELEGATE

static SQInteger null_weakref(HSQUIRRELVM v)
{
	v->Push(_null_);
	return 1;
}

static SQInteger null_tofloat(HSQUIRRELVM v)
{
	v->Push(SQFloat(0.0f));
	return 1;
}

static SQInteger null_tointeger(HSQUIRRELVM v)
{
	v->Push(SQInteger(0));
	return 1;
}

static SQInteger null_tostring(HSQUIRRELVM v)
{
	v->Push(SQString::Create(_ss(v), _SC("(null)"), 6));
	return 1;
}

SQRegFunction SQSharedState::_null_default_delegate_funcz[]={
	{_SC("weak"),null_weakref,1,NULL,			__FILE__, _SC("() // dummy")},
	{_SC("tofloat"),null_tofloat,1,NULL,		__FILE__, _SC("() // 0.0")},
	{_SC("tointeger"),null_tointeger,1,NULL,	__FILE__, _SC("() // 0")},
	{_SC("tostring"),null_tostring,1,NULL,		__FILE__, _SC("() // \"(null)\"")},
	{0,0}
};

/////////////////////////////////////////////////////////////////
//TABLE DEFAULT DELEGATE

static SQInteger table_rawdelete(HSQUIRRELVM v)
{
	if(SQ_FAILED(sq_rawdeleteslot(v,1,SQTrue)))
		return SQ_ERROR;
	return 1;
}


static SQInteger container_rawexists(HSQUIRRELVM v)
{
	if(SQ_SUCCEEDED(sq_rawget(v,-2))) {
		sq_pushbool(v,SQTrue);
		return 1;
	}
	sq_pushbool(v,SQFalse);
	return 1;
}

static SQInteger table_get(HSQUIRRELVM v)
{
	SQInteger n = sq_gettop(v);
	if (n < 2) return sq_throwerror(v, _SC("not enough params")); 
	if (n > 3) return sq_throwerror(v, _SC("too many params")); 

	SQObjectPtr& self = stack_get(v, 1);
	if (sqi_type(self) != OT_TABLE)
		return sq_throwerror(v, _SC("invalid this"));

	SQObjectPtr res;
	if (n == 3) res = stack_get(v, 3);
	v->Get(self, stack_get(v,2), res, false, DONT_FALL_BACK);
	v->Push(res);
	return 1;
}

static SQInteger table_set(HSQUIRRELVM v)
{
	SQObjectPtr& self = stack_get(v, 1);
	v->NewSlot(self, stack_get(v, 2), stack_get(v, 3), false);
	
	return 0;
}

static SQInteger table_delete(HSQUIRRELVM v)
{
	SQObjectPtr res;
	if (!v->DeleteSlot(stack_get(v, 1), stack_get(v, 2), res))
		return 0;
	v->Push(res);

	return 1;
}

static SQInteger table_rawset(HSQUIRRELVM v)
{
	return sq_rawset(v,-3);
}

static SQInteger table_rawget(HSQUIRRELVM v)
{
	return SQ_SUCCEEDED(sq_rawget(v,-2))?1:SQ_ERROR;
}

static SQInteger table_setdelegate(HSQUIRRELVM v)
{
	if(SQ_FAILED(sq_setdelegate(v,-2)))
		return SQ_ERROR;
	sq_push(v,-1); // -1 because sq_setdelegate pops 1
	return 1;
}

static SQInteger table_getdelegate(HSQUIRRELVM v)
{
	return SQ_SUCCEEDED(sq_getdelegate(v,-1))?1:SQ_ERROR;
}

SQRegFunction SQSharedState::_table_default_delegate_funcz[]={
	{_SC("len"),default_delegate_len,1, _SC("t"),			__FILE__, _SC("(): int")},
	{_SC("get"),table_get,0, _SC("t"),						__FILE__, _SC("(key, def=null): obj // safe: returns def if no such index")},
	{_SC("set"),table_set,3, _SC("t"),						__FILE__, _SC("(key, val) // safe: newslot if no such index")},
	{_SC("delete"),table_delete,2, _SC("t"),				__FILE__, _SC("(key): old // safe: null and nothing if no such index")},
	{_SC("rawget"),table_rawget,2, _SC("t"),				__FILE__, _SC("(key): obj")},
	{_SC("rawset"),table_rawset,3, _SC("t"),				__FILE__, _SC("(key, val)")},
	{_SC("rawdelete"),table_rawdelete,2, _SC("t"),			__FILE__, _SC("(key): obj")},
	{_SC("rawin"),container_rawexists,2, _SC("t"),			__FILE__, _SC("(key): bool")},
	{_SC("weak"),obj_delegate_weakref,1, NULL,				__FILE__, _SC("(): weakref") },
	{_SC("tostring"),default_delegate_tostring,1, _SC("."),	__FILE__, _SC("(): string")},
	{_SC("clear"),obj_clear,1, _SC("."),					__FILE__, _SC("()")},
	{_SC("setdelegate"),table_setdelegate,2, _SC(".t|o"),	__FILE__, _SC("(table)")},
	{_SC("getdelegate"),table_getdelegate,1, _SC("."),		__FILE__, _SC("(): table")},
	{0,0}
};

//ARRAY DEFAULT DELEGATE///////////////////////////////////////

static SQInteger array_append(HSQUIRRELVM v)
{
	sq_arrayappend(v,1);
	v->Push(stack_get(v,1));
	return 1;
}

static SQInteger array_extend(HSQUIRRELVM v)
{
	SQObject &o=stack_get(v,1);
	sqi_array(o)->Extend(sqi_array(stack_get(v,2)));
	v->Push(o);
	return 1;
}

static SQInteger array_reverse(HSQUIRRELVM v)
{
	sq_arrayreverse(v,1);
	v->Push(stack_get(v, 1));
	return 1;
}

static SQInteger array_pop(HSQUIRRELVM v)
{
	return SQ_SUCCEEDED(sq_arraypop(v,1,SQTrue))?1:SQ_ERROR;
}

static SQInteger array_top(HSQUIRRELVM v)
{
	SQObject &o=stack_get(v,1);
	if(sqi_array(o)->Size()>0){
		v->Push(sqi_array(o)->Top());
		return 1;
	}
	else return sq_throwerror(v,_SC("top() on a empty array"));
}

static SQInteger array_insert(HSQUIRRELVM v)
{
	SQObject &o=stack_get(v,1);
	SQObject &idx=stack_get(v,2);
	SQObject &val=stack_get(v,3);
	if(!sqi_array(o)->Insert(sqi_tointeger(idx),val))
		return sq_throwerror(v,_SC("index out of range"));
	v->Push(o);
	return 1;
}

static SQInteger array_remove(HSQUIRRELVM v)
{
	SQObject &o = stack_get(v, 1);
	SQObject &idx = stack_get(v, 2);
	if(!sq_isnumeric(idx)) return sq_throwerror(v, _SC("wrong type"));
	SQObjectPtr val;
	if(sqi_array(o)->Get(sqi_tointeger(idx), val)) {
		sqi_array(o)->Remove(sqi_tointeger(idx));
		v->Push(val);
		return 1;
	}
	return sq_throwerror(v, _SC("idx out of range"));
}

static SQInteger array_resize(HSQUIRRELVM v)
{
	SQObject &o = stack_get(v, 1);
	SQObject &nsize = stack_get(v, 2);
	SQObjectPtr fill;
	if(sq_isnumeric(nsize)) {
		if(sq_gettop(v) > 2)
			fill = stack_get(v, 3);
		sqi_array(o)->Resize(sqi_tointeger(nsize),fill);
		v->Push(o);
		return 1;
	}
	return sq_throwerror(v, _SC("size must be a number"));
}

static SQInteger __map_array(SQArray *dest,SQArray *src,HSQUIRRELVM v) {
	SQObjectPtr temp;
	SQInteger size = src->Size();
	for(SQInteger n = 0; n < size; n++) {
		src->Get(n,temp);
		v->Push(src);
		v->Push(temp);
		if(SQ_FAILED(sq_call(v,2,SQTrue,SQFalse))) {
			return SQ_ERROR;
		}
		dest->Set(n,v->GetUp(-1));
		v->Pop();
	}
	return 0;
}

static SQInteger array_map(HSQUIRRELVM v)
{
	SQObject &o = stack_get(v,1);
	SQInteger size = sqi_array(o)->Size();
	SQObjectPtr ret = SQArray::Create(_ss(v),size);
	if(SQ_FAILED(__map_array(sqi_array(ret),sqi_array(o),v)))
		return SQ_ERROR;
	v->Push(ret);
	return 1;
}

static SQInteger array_apply(HSQUIRRELVM v)
{
	SQObject &o = stack_get(v,1);
	if(SQ_FAILED(__map_array(sqi_array(o),sqi_array(o),v)))
		return SQ_ERROR;
	v->Push(o);
	return 1;
}

static SQInteger array_each(HSQUIRRELVM v)
{
	SQObject &o = stack_get(v, 1);
	SQArray *a = sqi_array(o);
	SQInteger size = a->Size();
	SQObjectPtr temp;
	for (SQInteger n = 0; n < size; ++n)
	{
		a->Get(n, temp);
		v->Push(a);
		v->Push(temp);
		if (SQ_FAILED(sq_call(v, 2, SQTrue, SQFalse)))
			return SQ_ERROR;

		SQObjectPtr& ret = v->GetUp(-1);
		if (sqi_type(ret) == OT_BOOL && sqi_integer(ret) == 0)
		{
			v->Pop(); break; 
		}
		v->Pop();
	}
	v->Push(o);
	return 1;
}

static SQInteger array_reduce(HSQUIRRELVM v)
{
	SQObject &o = stack_get(v,1);
	SQArray *a = sqi_array(o);
	SQInteger size = a->Size();
	if(size == 0) {
		return 0;
	}
	SQObjectPtr res;
	a->Get(0,res);
	if(size > 1) {
		SQObjectPtr other;
		for(SQInteger n = 1; n < size; n++) {
			a->Get(n,other);
			v->Push(o);
			v->Push(res);
			v->Push(other);
			if(SQ_FAILED(sq_call(v,3,SQTrue,SQFalse))) {
				return SQ_ERROR;
			}
			res = v->GetUp(-1);
			v->Pop();
		}
	}
	v->Push(res);
	return 1;
}

static SQInteger array_filter(HSQUIRRELVM v)
{
	SQObject &o = stack_get(v,1);
	SQArray *a = sqi_array(o);
	SQObjectPtr ret = SQArray::Create(_ss(v),0);
	SQInteger size = a->Size();
	SQObjectPtr val;
	for(SQInteger n = 0; n < size; n++) {
		a->Get(n,val);
		v->Push(o);
		v->Push(n);
		v->Push(val);
		if(SQ_FAILED(sq_call(v,3,SQTrue,SQFalse))) {
			return SQ_ERROR;
		}
		if(!v->IsFalse(v->GetUp(-1))) {
			sqi_array(ret)->Append(val);
		}
		v->Pop();
	}
	v->Push(ret);
	return 1;
}

static SQInteger array_find(HSQUIRRELVM v)
{
	SQObject &o = stack_get(v,1);
	SQObjectPtr &val = stack_get(v,2);
	SQArray *a = sqi_array(o);
	SQInteger size = a->Size();
	SQObjectPtr temp;
	for(SQInteger n = 0; n < size; n++) {
		bool res = false;
		a->Get(n,temp);
		if(v->IsEqual(temp,val,res) && res) {
			v->Push(n);
			return 1;
		}
	}
	return 0;
}

//QSORT ala Sedgewick
bool _qsort_compare(HSQUIRRELVM v,SQObjectPtr &arr,SQObjectPtr &a,SQObjectPtr &b,SQInteger func,SQInteger &ret)
{
	if(func < 0) {
		if(!v->ObjCmp(a,b,ret)) return false;
	}
	else {
		SQInteger top = sq_gettop(v);
		sq_push(v, func);
		sq_pushroottable(v);
		v->Push(a);
		v->Push(b);
		if(SQ_FAILED(sq_call(v, 3, SQTrue, SQFalse))) {
			if(!sq_isstring( v->_lasterror)) 
				v->Raise_Error(_SC("compare func failed"));
			return false;
		}
		sq_getinteger(v, -1, &ret);
		sq_settop(v, top);
		return true;
	}
	return true;
}
//QSORT ala Sedgewick
bool _qsort(HSQUIRRELVM v,SQObjectPtr &arr, SQInteger l, SQInteger r,SQInteger func)
{
	SQInteger i, j;
	SQArray *a=sqi_array(arr);
	SQObjectPtr pivot,t;
	if( l < r ){
		pivot = a->_values[l];
		i = l; j = r+1;
		while(1){
			SQInteger ret;
			do { 
				++i; 
				if(i > r) break;
				if(!_qsort_compare(v,arr,a->_values[i],pivot,func,ret))
					return false;
			} while( ret <= 0);
			do {
				--j;
				if ( j < 0 ) {
					v->Raise_Error( _SC("Invalid qsort, probably compare function defect") ); 
					return false; 
				}
				if(!_qsort_compare(v,arr,a->_values[j],pivot,func,ret))
					return false;
			}
			while( ret > 0 );
			if( i >= j ) break;
			t = a->_values[i]; a->_values[i] = a->_values[j]; a->_values[j] = t;
		}
		t = a->_values[l]; a->_values[l] = a->_values[j]; a->_values[j] = t;
		if(!_qsort( v, arr, l, j-1,func)) return false;
		if(!_qsort( v, arr, j+1, r,func)) return false;
	}
	return true;
}

static SQInteger array_sort(HSQUIRRELVM v)
{
	SQInteger func = -1;
	SQObjectPtr &o = stack_get(v,1);
	if (sqi_array(o)->Size() > 1) 
	{
		if (sq_gettop(v) > 1)
		{
			SQObject &funcobj = stack_get(v,2);
			if(sqi_type(funcobj) == OT_CLOSURE || sqi_type(funcobj) == OT_NATIVECLOSURE) func = 2;
		}
		if (!_qsort(v, o, 0, sqi_array(o)->Size()-1, func))
			return SQ_ERROR;
	}
	v->Push(o);
	return 1;
}
static SQInteger array_slice(HSQUIRRELVM v)
{
	SQInteger sidx,eidx;
	SQObjectPtr o;
	if(get_slice_params(v,sidx,eidx,o)==-1)return -1;
	SQInteger alen = sqi_array(o)->Size();
	if(sidx < 0)sidx = alen + sidx;
	if(eidx < 0)eidx = alen + eidx;
	if(eidx < sidx)return sq_throwerror(v,_SC("wrong indexes"));
	if(eidx > alen)return sq_throwerror(v,_SC("slice out of range"));
	SQArray *arr=SQArray::Create(_ss(v),eidx-sidx);
	SQObjectPtr t;
	SQInteger count=0;
	for(SQInteger i=sidx;i<eidx;i++){
		sqi_array(o)->Get(i,t);
		arr->Set(count++,t);
	}
	v->Push(arr);
	return 1;
	
}

SQRegFunction SQSharedState::_array_default_delegate_funcz[]={
	{_SC("len"),default_delegate_len,1, _SC("a"),	__FILE__, _SC("(): int")},
	{_SC("append"),array_append,2, _SC("a"),		__FILE__, _SC("(val): this // same with push()")},
	{_SC("extend"),array_extend,2, _SC("aa"),		__FILE__, _SC("(array): this")},
	{_SC("push"),array_append,2, _SC("a"),			__FILE__, _SC("(val): this")},
	{_SC("pop"),array_pop,1, _SC("a"),				__FILE__, _SC("(): obj")},
	{_SC("top"),array_top,1, _SC("a"),				__FILE__, _SC("(): obj")},
	{_SC("insert"),array_insert,3, _SC("an"),		__FILE__, _SC("(idx, val): this")},
	{_SC("remove"),array_remove,2, _SC("an"),		__FILE__, _SC("(idx): obj")},
	{_SC("resize"),array_resize,-2, _SC("an"),		__FILE__, _SC("(size[, fill]): this")},
	{_SC("reverse"),array_reverse,1, _SC("a"),		__FILE__, _SC("(): this")},
	{_SC("sort"),array_sort,-1, _SC("ac"),			__FILE__, _SC("([compare]): this // compare = @(a,b) => a <=> b")},
	{_SC("slice"),array_slice,-2, _SC("ann"),		__FILE__, _SC("(start[, end]): array")},
	{_SC("weak"),obj_delegate_weakref,1, NULL,		__FILE__, _SC("(): weakref") },
	{_SC("tostring"),default_delegate_tostring,1, _SC("."), __FILE__, _SC("(): string")},
	{_SC("clear"),obj_clear,1, _SC("."),			__FILE__, _SC("()")},
	{_SC("map"),array_map,2, _SC("ac"),				__FILE__, _SC("(func): array // func = @(v) => nv, returns new array with return values") }, 
	{_SC("apply"),array_apply,2, _SC("ac"),			__FILE__, _SC("(func): this // func = @(v) => nv, replaces the original value")}, 
	{_SC("each"),array_each,2, _SC("ac"),			__FILE__, _SC("(func): this // func = @(v) => b, if b == false stops iteration")},
	{_SC("reduce"),array_reduce,2, _SC("ac"),		__FILE__, _SC("(func): value // func = @(v0, v) => v1, returns last v1")}, 
	{_SC("filter"),array_filter,2, _SC("ac"),		__FILE__, _SC("(func): array // func = @(i, v) => true/false, returns new array with filtered values")},
	{_SC("find"),array_find,2, _SC("a."),			__FILE__, _SC("(value): index or null")},
	{0,0}
};

//STRING DEFAULT DELEGATE//////////////////////////
static SQInteger string_ascii_slice(HSQUIRRELVM v)
{
	SQInteger sidx,eidx;
	SQObjectPtr o;
	if(SQ_FAILED(get_slice_params(v,sidx,eidx,o)))return -1;
	SQInteger slen = sqi_string(o)->_len;
	if(sidx < 0)sidx = slen + sidx;
	if(eidx < 0)eidx = slen + eidx;
	if(eidx < sidx)	return sq_throwerror(v,_SC("wrong indexes"));
	if(eidx > slen)	
		return sq_throwerror(v,_SC("slice out of range"));
	v->Push(SQString::Create(_ss(v),&sqi_stringval(o)[sidx],eidx-sidx));
	return 1;
}

static SQInteger string_ascii_find(HSQUIRRELVM v)
{
	SQInteger top,start_idx=0;
	const SQChar *str,*substr,*ret;
	if(((top=sq_gettop(v))>1) && SQ_SUCCEEDED(sq_getstring(v,1,&str)) && SQ_SUCCEEDED(sq_getstring(v,2,&substr)))
	{
		SQInteger len = sq_getsize(v, 1);
		if (top>2)
		{
			sq_getinteger(v,3,&start_idx);
			if (start_idx < 0)
				start_idx = len + start_idx;
		}

		if((sq_getsize(v,1)>start_idx) && (start_idx>=0)){
			ret=scstrstr(&str[start_idx],substr);
			if(ret){
				sq_pushinteger(v,(SQInteger)(ret-str));
				return 1;
			}
		}
		return 0;
	}
	return sq_throwerror(v,_SC("invalid param"));
}

#define STRING_TOFUNCZ(func) static SQInteger string_##func(HSQUIRRELVM v) \
{ \
	SQObject str=stack_get(v,1); \
	SQInteger len=sqi_string(str)->_len; \
	const SQChar *sThis=sqi_stringval(str); \
	SQChar *sNew=(_ss(v)->GetScratchPad(rsl(len))); \
	for(SQInteger i=0;i<len;i++) sNew[i]=func(sThis[i]); \
	v->Push(SQString::Create(_ss(v),sNew,len)); \
	return 1; \
}


STRING_TOFUNCZ(tolower)
STRING_TOFUNCZ(toupper)

static SQInteger string_utf8_len(HSQUIRRELVM v)
{
	SQObject self = stack_get(v, 1);
	SQInteger utf8len = sqi_string(self)->_utf8_len;
	if (utf8len < 0)
		utf8len = sqi_string(self)->_utf8_len = (SQInteger)nit::Unicode::utf8Length(sqi_stringval(self));
	v->Push(utf8len);
	return 1;
}

static SQInteger string_ascii(HSQUIRRELVM v)
{
	SQObject self = stack_get(v, 1);
	SQInteger n = sqi_integer(stack_get(v, 2));

	if(abs((int)n)<sqi_string(self)->_len){
		if(n<0)n=sqi_string(self)->_len-n;
		v->Push(SQInteger(sqi_stringval(self)[n]));
		return 1;
	}
	return 0;
}

static SQInteger string_utf8_chars(HSQUIRRELVM v)
{
	using namespace nit;

	SQObject self = stack_get(v, 1);
	const SQChar* str = sqi_stringval(self);
	SQInteger len = sqi_string(self)->_len;

	SQInteger top = sq_gettop(v);

	SQInteger begin = 0;
	if (top >= 2) 
	{
		sq_getinteger(v, 2, &begin);
		if (begin > 0)
			begin = Unicode::utf8Next(str, str + len, begin) - str;
		else if (begin < 0)
			begin = Unicode::utf8Prev(str + len, str, -begin) - str;
	}

	SQInteger end;
	if (top >= 3)
	{
		sq_getinteger(v, 3, &end);
		if (end > 0)
			end = Unicode::utf8Next(str, str + len, end) - str;
		else if (end < 0)
			end = Unicode::utf8Prev(str + len, str, -end) - str;
	}
	else end = len;

	if (begin < 0 || end < 0 || end < begin || begin > len) return sq_throwerror(v,_SC("wrong indexes"));

	SQArray* arr = SQArray::Create(_ss(v), 0);
	const SQChar* itr = str + begin;
	const SQChar* iend = str + end;
	while (itr < iend)
	{
		int ch = Unicode::utf8Advance(itr);
		arr->_values.push_back(ch);
	}
	GC_MUTATED(arr);
	v->Push(arr);
	return 1;
}

static SQInteger string_utf8_slice(HSQUIRRELVM v)
{
	using namespace nit;

	SQObject self = stack_get(v, 1);
	const SQChar* str = sqi_stringval(self);
	SQInteger len = sqi_string(self)->_len;

	SQInteger top = sq_gettop(v);

	SQInteger begin = 0;
	if (top >= 2) 
	{
		sq_getinteger(v, 2, &begin);
		if (begin > 0)
			begin = Unicode::utf8Next(str, str + len, begin) - str;
		else if (begin < 0)
			begin = Unicode::utf8Prev(str + len, str, -begin) - str;
	}

	SQInteger end;
	if (top >= 3)
	{
		sq_getinteger(v, 3, &end);
		if (end > 0)
			end = Unicode::utf8Next(str, str + len, end) - str;
		else if (end < 0)
			end = Unicode::utf8Prev(str + len, str, -end) - str;
	}
	else end = len;

	if (begin < 0 || end < 0 || end < begin || begin > len) return sq_throwerror(v,_SC("wrong indexes"));

	v->Push(SQString::Create(_ss(v), str + begin, end - begin));
	return 1;
}

static SQInteger string_utf8_find(HSQUIRRELVM v)
{
	using namespace nit;

	SQInteger top,start_idx=0;
	const SQChar *str,*substr,*ret;
	if (((top=sq_gettop(v))>1) && SQ_SUCCEEDED(sq_getstring(v,1,&str)) && SQ_SUCCEEDED(sq_getstring(v,2,&substr)))
	{
		SQInteger ascii_len = sq_getsize(v, 1);
		if (top>2)
		{
			sq_getinteger(v,3,&start_idx);
			if (start_idx > 0)
				start_idx = Unicode::utf8Next(str, str + ascii_len, start_idx) - str;
			else if (start_idx < 0)
				start_idx = Unicode::utf8Prev(str + ascii_len, str, -start_idx) - str;
		}

		if ((ascii_len > start_idx) && (start_idx>=0))
		{
			ret=scstrstr(&str[start_idx],substr);
			if(ret) {
				sq_pushinteger(v, Unicode::utf8CharCount(str, ret));
				return 1;
			}
		}
		return 0;
	}
	return sq_throwerror(v,_SC("invalid param"));
}

SQRegFunction SQSharedState::_string_default_delegate_funcz[]={
	{_SC("tointeger"),default_delegate_tointeger,1, _SC("s"),	__FILE__, _SC("(): int")},
	{_SC("tofloat"),default_delegate_tofloat,1, _SC("s"),		__FILE__, _SC("(): float")},
	{_SC("tostring"),default_delegate_tostring,1, _SC("."),		__FILE__, _SC("(): string")},
	{_SC("tolower"),string_tolower,1, _SC("s"),					__FILE__, _SC("(): string")},
	{_SC("toupper"),string_toupper,1, _SC("s"),					__FILE__, _SC("(): string")},
	{_SC("weak"),obj_delegate_weakref,1, NULL,					__FILE__, _SC("(): weakref") },

	{_SC("len"),string_utf8_len,1, _SC("s"),					__FILE__, _SC("(): int")},
	{_SC("chars"),string_utf8_chars, -1, _SC("snn"),			__FILE__, _SC("([start, end]): array")},
	{_SC("find"), string_utf8_find, -2, _SC("ssnn"),			__FILE__, _SC("(substr: string[, start]): unicode_index // int or null (slow)")},
	{_SC("slice"), string_utf8_slice, -2, _SC("snn"),			__FILE__, _SC("(start, [end]): string")},

	{_SC("ascii"),string_ascii, 2, _SC("sn"),					__FILE__, _SC("(index): int")},
	{_SC("ascii_len"),default_delegate_len, 1, _SC("s"),		__FILE__, _SC("(): int")},
	{_SC("ascii_find"),string_ascii_find,-2, _SC("ssnn"),		__FILE__, _SC("(substr: string[, start]): ascii_index // int or null")},
	{_SC("ascii_slice"),string_ascii_slice,-2, _SC("snn"),		__FILE__, _SC("(start[, end]): string")},
	{0,0}
};

//INTEGER DEFAULT DELEGATE//////////////////////////
SQRegFunction SQSharedState::_number_default_delegate_funcz[]={
	{_SC("tointeger"),default_delegate_tointeger,1, _SC("n|b"),	__FILE__, _SC("(): int")},
	{_SC("tofloat"),default_delegate_tofloat,1, _SC("n|b"),		__FILE__, _SC("(): float")},
	{_SC("tostring"),default_delegate_tostring,1, _SC("."),		__FILE__, _SC("(): string")},
	{_SC("tochar"),number_delegate_tochar,1, _SC("n|b"),		__FILE__, _SC("(): int")},
	{_SC("weak"),obj_delegate_weakref,1, NULL,					__FILE__, _SC("(): this // dummy") },
	{0,0}
};

//CLOSURE DEFAULT DELEGATE//////////////////////////
static SQInteger closure_pcall(HSQUIRRELVM v)
{
	return SQ_SUCCEEDED(sq_call(v,sq_gettop(v)-1,SQTrue,SQFalse))?1:SQ_ERROR;
}

static SQInteger closure_call(HSQUIRRELVM v)
{
	return SQ_SUCCEEDED(sq_call(v,sq_gettop(v)-1,SQTrue,SQTrue))?1:SQ_ERROR;
}

static SQInteger _closure_acall(HSQUIRRELVM v,SQBool raiseerror)
{
	SQArray *aparams=sqi_array(stack_get(v,3));
	SQInteger nparams=aparams->Size();
	v->Push(stack_get(v,1));
	v->Push(stack_get(v,2));
	for(SQInteger i=0;i<nparams;i++)v->Push(aparams->_values[i]);
	return SQ_SUCCEEDED(sq_call(v,nparams+1,SQTrue,raiseerror))?1:SQ_ERROR;
}

static SQInteger closure_acall(HSQUIRRELVM v)
{
	return _closure_acall(v,SQTrue);
}

static SQInteger closure_pacall(HSQUIRRELVM v)
{
	return _closure_acall(v,SQFalse);
}

static SQInteger closure_bindenv(HSQUIRRELVM v)
{
	if(SQ_FAILED(sq_bindenv(v,1)))
		return SQ_ERROR;
	return 1;
}

static SQInteger closure_getenv(HSQUIRRELVM v)
{
	SQObject o = stack_get(v,1);
	if (sq_isclosure(o)) {
		SQObject env = sqi_closure(o)->_env;
		if (sq_isweakref(env))
			v->Push(sqi_weakref(env)->_obj);
		else
			v->Push(env);
		return 1;
	}
	else if (sq_isnativeclosure(o)) {
		SQObject env = sqi_nativeclosure(o)->_env;
		if (sq_isweakref(env))
			v->Push(sqi_weakref(env)->_obj);
		else
			v->Push(env);
		return 1;
	}
	else return SQ_ERROR;
}

static SQInteger closure_getinfos(HSQUIRRELVM v) {
	SQObject o = stack_get(v,1);
	SQTable *res = SQTable::Create(_ss(v),4);
	if(sqi_type(o) == OT_CLOSURE) {
		SQFunctionProto *f = sqi_closure(o)->_function;
		SQInteger nparams = f->_nparameters + (f->_varparams?1:0);
		SQObjectPtr params = SQArray::Create(_ss(v),nparams);
		for(SQInteger n = 0; n<f->_nparameters; n++) {
			sqi_array(params)->Set((SQInteger)n,f->_parameters[n]);
		}
		if(f->_varparams) {
			sqi_array(params)->Set(nparams-1,SQString::Create(_ss(v),_SC("..."),-1));
		}
		res->NewSlot(SQString::Create(_ss(v),_SC("native"),-1),false);
		res->NewSlot(SQString::Create(_ss(v),_SC("name"),-1),f->_name);
		res->NewSlot(SQString::Create(_ss(v),_SC("src"),-1),f->_sourcename);
		res->NewSlot(SQString::Create(_ss(v),_SC("parameters"),-1),params);
		res->NewSlot(SQString::Create(_ss(v),_SC("varargs"),-1),f->_varparams);
		res->NewSlot(SQString::Create(_ss(v),_SC("help"),-1),f->_help);
		res->NewSlot(SQString::Create(_ss(v),_SC("imports"),-1),f->_imports);
	}
	else { //OT_NATIVECLOSURE 
		SQNativeClosure *nc = sqi_nativeclosure(o);
		res->NewSlot(SQString::Create(_ss(v),_SC("native"),-1),true);
		res->NewSlot(SQString::Create(_ss(v),_SC("name"),-1), nc->_name ? SQString::Create(_ss(v),nc->_name) : _null_);
		res->NewSlot(SQString::Create(_ss(v),_SC("src"),-1), nc->_file ? SQString::Create(_ss(v),nc->_file) : _null_);
		res->NewSlot(SQString::Create(_ss(v),_SC("paramscheck"),-1),nc->_nparamscheck);
		SQObjectPtr typecheck;
		if(nc->_typecheck.size() > 0) {
			typecheck =
				SQArray::Create(_ss(v), nc->_typecheck.size());
			for(SQUnsignedInteger n = 0; n<nc->_typecheck.size(); n++) {
					sqi_array(typecheck)->Set((SQInteger)n,nc->_typecheck[n]);
			}
		}
		res->NewSlot(SQString::Create(_ss(v),_SC("typecheck"),-1),typecheck);
		res->NewSlot(SQString::Create(_ss(v),_SC("help"),-1),nc->_help);
	}
	v->Push(res);
	return 1;
}

static SQInteger closure_gethelp(HSQUIRRELVM v) {
	sq_gethelp(v, 1);
	return 1;
}

static SQInteger closure_sethelp(HSQUIRRELVM v) {
	sq_push(v, 2);
	sq_sethelp(v, 1);
	return 1;
}

extern "C" { SQRESULT sqx_dumpfunc(HSQUIRRELVM v, SQInteger idx); }

static SQInteger closure_dump(HSQUIRRELVM v) {
	return sqx_dumpfunc(v, 1);
}

SQRegFunction SQSharedState::_closure_default_delegate_funcz[]={
	{_SC("call"),closure_call,-1, _SC("c"),					__FILE__, _SC("(this, ...): return val")},
	{_SC("pcall"),closure_pcall,-1, _SC("c"),				__FILE__, _SC("(this, ...): return val")},
	{_SC("acall"),closure_acall,3, _SC("c.a"),				__FILE__, _SC("(this, args: array): return val")},
	{_SC("pacall"),closure_pacall,3, _SC("c.a"),			__FILE__, _SC("(this, args: array): return val")},
	{_SC("weak"),obj_delegate_weakref,1, NULL ,				__FILE__, _SC("(): weakref")},
	{_SC("tostring"),default_delegate_tostring,1, _SC("."),	__FILE__, _SC("(): string")},
	{_SC("bind"),closure_bindenv,2, _SC("c x|y|t|r|R"),		__FILE__, _SC("(object)")},
	{_SC("getbind"),closure_getenv,1, _SC("c"),				__FILE__, _SC("(): object")},
	{_SC("getinfos"),closure_getinfos,1, _SC("c"),			__FILE__, _SC("(): table")},
	{_SC("gethelp"),closure_gethelp,1, _SC("c"),			__FILE__, _SC("(): string")},
	{_SC("sethelp"),closure_sethelp,2, _SC("c."),			__FILE__, _SC("(help: string)")},
	{_SC("dump"),closure_dump,1, _SC("c"),					__FILE__, _SC("()")},
	{0,0}
};

//GENERATOR DEFAULT DELEGATE
static SQInteger generator_getstatus(HSQUIRRELVM v)
{
	SQObject &o=stack_get(v,1);
	switch(sqi_generator(o)->_state){
		case SQGenerator::eSuspended:v->Push(SQString::Create(_ss(v),_SC("suspended")));break;
		case SQGenerator::eRunning:v->Push(SQString::Create(_ss(v),_SC("running")));break;
		case SQGenerator::eDead:v->Push(SQString::Create(_ss(v),_SC("dead")));break;
	}
	return 1;
}

SQRegFunction SQSharedState::_generator_default_delegate_funcz[]={
	{_SC("getstatus"),generator_getstatus,1, _SC("g"),		__FILE__, _SC("(): string // running | dead | suspended")},
	{_SC("weak"),obj_delegate_weakref,1, NULL ,				__FILE__, _SC("(): weakref")},
	{_SC("tostring"),default_delegate_tostring,1, _SC("."),	__FILE__, _SC("(): string")},
	{0,0}
};

//THREAD DEFAULT DELEGATE

static SQInteger thread_call(HSQUIRRELVM v)
{
	SQObjectPtr o = stack_get(v,1);
	if(sqi_type(o) == OT_THREAD) {
		SQInteger nparams = sq_gettop(v);
		sqi_thread(o)->Push(_ss(v)->_root_table);
		for(SQInteger i = 2; i<(nparams+1); i++)
			sq_move(sqi_thread(o),v,i);
		if(SQ_SUCCEEDED(sq_call(sqi_thread(o),nparams,SQTrue,SQTrue))) {
			sq_move(v,sqi_thread(o),-1);
			sq_pop(sqi_thread(o),1);
			return 1;
		}
		v->_lasterror = sqi_thread(o)->_lasterror;
		return SQ_ERROR;
	}
	return sq_throwerror(v,_SC("wrong parameter"));
}

static SQInteger thread_wakeup(HSQUIRRELVM v)
{
	SQObjectPtr o = stack_get(v,1);
	if(sqi_type(o) == OT_THREAD) {
		SQVM *thread = sqi_thread(o);
		SQInteger state = sq_getvmstate(thread);
		if(state != SQ_VMSTATE_SUSPENDED) {
			switch(state) {
				case SQ_VMSTATE_IDLE:
					return sq_throwerror(v,_SC("cannot wakeup a idle thread"));
				break;
				case SQ_VMSTATE_RUNNING:
					return sq_throwerror(v,_SC("cannot wakeup a running thread"));
				break;
			}
		}
			
		SQInteger wakeupret = sq_gettop(v)>1?1:0;
		if(wakeupret) {
			sq_move(thread,v,2);
		}
		if(SQ_SUCCEEDED(sq_wakeupvm(thread,wakeupret,SQTrue,SQTrue,SQFalse))) {
			sq_move(v,thread,-1);
			sq_pop(thread,1); //pop retval
			if(sq_getvmstate(thread) == SQ_VMSTATE_IDLE) {
				sq_settop(thread,1); //pop roottable
			}
			return 1;
		}
		sq_settop(thread,1);
		v->_lasterror = thread->_lasterror;
		return SQ_ERROR;
	}
	return sq_throwerror(v,_SC("wrong parameter"));
}

static SQInteger thread_getstatus(HSQUIRRELVM v)
{
	SQObjectPtr &o = stack_get(v,1);
	switch(sq_getvmstate(sqi_thread(o))) {
		case SQ_VMSTATE_IDLE:
			sq_pushstring(v,_SC("idle"),-1);
		break;
		case SQ_VMSTATE_RUNNING:
			sq_pushstring(v,_SC("running"),-1);
		break;
		case SQ_VMSTATE_SUSPENDED:
			sq_pushstring(v,_SC("suspended"),-1);
		break;
		default:
			return sq_throwerror(v,_SC("internal VM error"));
	}
	return 1;
}

static SQInteger thread_getlocal(HSQUIRRELVM v)
{
	SQObjectPtr &o = stack_get(v, 1);
	if(sqi_type(o) != OT_THREAD)
		return sq_throwerror(v,_SC("wrong parameter"));

	SQVM *thread = sqi_thread(o);
	sq_pushthreadlocal(v, thread);
	return 1;
}

SQRegFunction SQSharedState::_thread_default_delegate_funcz[] = {
	{_SC("call"), thread_call, -1, _SC("v"),				__FILE__, _SC("(...)")},
	{_SC("wakeup"), thread_wakeup, -1, _SC("v"),			__FILE__, _SC("([wakeupval])")},
	{_SC("getstatus"), thread_getstatus, 1, _SC("v"),		__FILE__, _SC("(): string // idle | running | suspended")},
	{_SC("getlocal"), thread_getlocal, 1, _SC("v"),			__FILE__, _SC("(): table")},
	{_SC("weak"),obj_delegate_weakref,1, NULL,				__FILE__, _SC("(): weakref")},
	{_SC("tostring"),default_delegate_tostring,1, _SC("."),	__FILE__, _SC("(): string")},
	{0,0},
};

static SQInteger class_getattributes(HSQUIRRELVM v)
{
	return SQ_SUCCEEDED(sq_getattributes(v,-2))?1:SQ_ERROR;
}

static SQInteger class_setattributes(HSQUIRRELVM v)
{
	return SQ_SUCCEEDED(sq_setattributes(v,-3))?1:SQ_ERROR;
}

static SQInteger class_getgetter(HSQUIRRELVM v)
{
	if (SQ_FAILED(sq_getproperty(v, -2))) return 0;
	v->Pop();
	return 1;
}

static SQInteger class_getsetter(HSQUIRRELVM v)
{
	if (SQ_FAILED(sq_getproperty(v, -2))) return 0;
	v->Replace(-2);
	return 1;
}

static SQInteger class_instance(HSQUIRRELVM v)
{
	return SQ_SUCCEEDED(sq_createinstance(v,-1))?1:SQ_ERROR;
}

static SQInteger class_getbase(HSQUIRRELVM v)
{
	return SQ_SUCCEEDED(sq_getbase(v,-1))?1:SQ_ERROR;
}

static SQInteger class_new(HSQUIRRELVM v)
{
	SQObjectPtr o = stack_get(v, 1);
	SQClass* cls = sqi_class(o);

	SQInteger params = sq_gettop(v);

	SQObjectPtr inst;
	bool ok = v->CallNew(cls, params, v->_top-params, inst, true);

	if (!ok) return SQ_ERROR;

	v->Push(inst);
	return 1;
}

SQRegFunction SQSharedState::_class_default_delegate_funcz[] = {
	{_SC("new"), class_new, -1, _SC("y"),						__FILE__, _SC("(...): instance // same with <class>(...)")},
	{_SC("getattributes"), class_getattributes, 2, _SC("y."),	__FILE__, _SC("(member: string): obj // if member==null, returns class level attr")},
	{_SC("setattributes"), class_setattributes, 3, _SC("y.."),	__FILE__, _SC("(member: string, attr: obj): obj // returns old attr")},
	{_SC("getgetter"), class_getgetter, 2, _SC("y."),			__FILE__, _SC("(property: string): closure")},
	{_SC("getsetter"), class_getsetter, 2, _SC("y."),			__FILE__, _SC("(property: string): closure")},
	{_SC("rawin"),container_rawexists,2, _SC("y"),				__FILE__, _SC("(key: string): bool // same with 'in' but no delegation")},
	{_SC("weak"),obj_delegate_weakref,1, NULL,					__FILE__, _SC("(): weakref") },
	{_SC("tostring"),default_delegate_tostring,1, _SC("."),		__FILE__, _SC("(): string")},
	{_SC("instance"),class_instance,1, _SC("y"),				__FILE__, _SC("(): instance // without call constructor")},
	{_SC("getbase"),class_getbase,1, _SC("y"),					__FILE__, _SC("(): class")},
	{0,0}
};

static SQInteger instance_getclass(HSQUIRRELVM v)
{
	if(SQ_SUCCEEDED(sq_getclass(v,1)))
		return 1;
	return SQ_ERROR;
}

static SQInteger instance_purge(HSQUIRRELVM v)
{
	if(SQ_SUCCEEDED(sq_purgeinstance(v,1)))
		return 0;
	return SQ_ERROR;
}

SQRegFunction SQSharedState::_instance_default_delegate_funcz[] = {
	{_SC("getclass"), instance_getclass, 1, _SC("x"),		__FILE__, _SC("(): class")},
//	{_SC("purge"), instance_purge, 1, _SC("x"),				__FILE__, _SC("()")},
	{_SC("rawin"),container_rawexists,2, _SC("x"),			__FILE__, _SC("(key): bool")},
	{_SC("weak"),obj_delegate_weakref,1, NULL ,				__FILE__, _SC("(): weakref")},
	{_SC("tostring"),default_delegate_tostring,1, _SC("."),	__FILE__, _SC("(): string")},
	{0,0}
};

static SQInteger weakref_ref(HSQUIRRELVM v)
{
	if(SQ_FAILED(sq_getweakrefval(v,1)))
		return SQ_ERROR;
	return 1;
}

SQRegFunction SQSharedState::_weakref_default_delegate_funcz[] = {
	{_SC("ref"),weakref_ref,1, _SC("r"),					__FILE__, _SC("(): obj")},
	{_SC("weak"),obj_delegate_weakref,1, NULL ,				__FILE__, _SC("(): weakref // != this")},
	{_SC("tostring"),default_delegate_tostring,1, _SC("."),	__FILE__, _SC("(): string")},
	{0,0}
};

static SQInteger nativeweakref_ref(HSQUIRRELVM v)
{
	if(SQ_FAILED(sq_getweakrefval(v, 1)))
		return SQ_ERROR;
	return 1;
}

SQRegFunction SQSharedState::_nativeweakref_default_delegate_funcz[] =
{
	{_SC("ref"),nativeweakref_ref,1, _SC("R"),				__FILE__, _SC("(): obj")},
	{_SC("weak"),obj_delegate_weakref,1, NULL ,				__FILE__, _SC("(): weakref // != this")},
	{_SC("tostring"),default_delegate_tostring,1, _SC("."),	__FILE__, _SC("(): string")},
	{0,0}
};
