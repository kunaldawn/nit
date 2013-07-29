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
#include "squserdata.h"
#include "sqcompiler.h"
#include "sqfuncstate.h"
#include "sqclass.h"

#include <stdarg.h>

bool sq_aux_gettypedarg(HSQUIRRELVM v,SQInteger idx,SQObjectType type,SQObjectPtr **o)
{
	*o = &stack_get(v,idx);
	if(sqi_type(**o) != type){
		SQObjectPtr oval = v->PrintObjVal(**o);
		v->Raise_Error(_SC("wrong argument type, expected '%s' got '%.50s'"),IdType2Name(type),sqi_stringval(oval));
		return false;
	}
	return true;
}

#define _GETSAFE_OBJ(v,idx,sqi_type,o) { if(!sq_aux_gettypedarg(v,idx,sqi_type,&o)) return SQ_ERROR; }

#define sq_aux_paramscheck(v,count) \
{ \
	if(sq_gettop(v) < count){ v->Raise_Error(_SC("not enough params in the stack")); return SQ_ERROR; }\
}		

SQInteger sq_aux_throwobject(HSQUIRRELVM v,SQObjectPtr &e)
{
	v->_lasterror = e;
	return SQ_ERROR;
}

SQInteger sq_aux_invalidtype(HSQUIRRELVM v,SQObjectType type)
{
	scsprintf(_ss(v)->GetScratchPad(100), _SC("unexpected type %s"), IdType2Name(type));
	return sq_throwerror(v, _ss(v)->GetScratchPad(-1));
}

HSQUIRRELVM sq_open(SQInteger initialstacksize)
{
	SQSharedState *ss;
	SQVM *v;
	sq_new(ss, SQSharedState);
	ss->Init();
	v = (SQVM *)SQ_MALLOC(sizeof(SQVM));
	new (v) SQVM(ss);
	ss->_root_vm = v;
	ss->_curr_thread = v;
	if(v->Init(NULL, initialstacksize)) {
		return v;
	} else {
		sq_delete(v, SQVM);
		return NULL;
	}
	return v;
}

HSQUIRRELVM sq_newthread(HSQUIRRELVM friendvm, SQInteger initialstacksize)
{
	SQSharedState *ss;
	SQVM *v;
	ss=_ss(friendvm);
	
	v= (SQVM *)SQ_MALLOC(sizeof(SQVM));
	new (v) SQVM(ss);
	
	if(v->Init(friendvm, initialstacksize)) {
		friendvm->Push(v);
		return v;
	} else {
		sq_delete(v, SQVM);
		return NULL;
	}
}

SQInteger sq_getvmstate(HSQUIRRELVM v)
{
	if(v->_suspended)
		return SQ_VMSTATE_SUSPENDED;
	else { 
		if(v->_callsstacksize != 0) return SQ_VMSTATE_RUNNING;
		else return SQ_VMSTATE_IDLE;
	}
}

void sq_setrequirehandler(HSQUIRRELVM v, SQREQUIREHANDLER reqfunc)
{
	v->_requirehandler = reqfunc;
}

void sq_seterrorhandler(HSQUIRRELVM v)
{
	SQObject o = stack_get(v, -1);
	if(sq_isclosure(o) || sq_isnativeclosure(o) || sq_isnull(o)) {
		v->_errorhandler = o;
		v->Pop();
	}
}

void sq_setnativedebughook(HSQUIRRELVM v, SQUserPointer up, SQDEBUGHOOK hook)
{
	v->_debughook_native = hook;
	v->_debughook_native_up = up;
	v->_debughook_closure.Null();
	v->_debughook = hook?true:false;
}

void sq_setdebughook(HSQUIRRELVM v)
{
	SQObject o = stack_get(v,-1);
	if(sq_isclosure(o) || sq_isnativeclosure(o) || sq_isnull(o)) {
		v->_debughook_closure = o;
		v->_debughook_native = NULL;
		v->_debughook_native_up = NULL;
		v->_debughook = !sq_isnull(o);
		v->Pop();
	}
}

void sq_setoplimit(HSQUIRRELVM v, SQUnsignedInteger limit)
{
	_ss(v)->_oplimit = limit;
}

void sq_closethread(HSQUIRRELVM v)
{
	sq_settop(v, 0);
	v->Finalize();
}

void sq_close(HSQUIRRELVM v, SQGCFinalizeReport* optReport)
{
	SQSharedState *ss = _ss(v);
#ifndef NO_GARBAGE_COLLECTOR
		ss->_gc_finalize_report = optReport;
#endif
	sqi_table(ss->_root_table)->Finalize();
	sqi_thread(ss->_root_vm)->Finalize();
	sq_delete(ss, SQSharedState);
}

SQRESULT sq_compile(HSQUIRRELVM v,SQLEXREADFUNC read,SQUserPointer p,const SQChar *sourcename,SQBool raiseerror)
{
	SQObjectPtr o;
	if(Compile(v, read, p, sourcename, o, raiseerror?true:false, _ss(v)->_debuginfo)) {
		v->Push(SQClosure::Create(_ss(v), sqi_funcproto(o)));
		return SQ_OK;
	}
	return SQ_ERROR;
}

void sq_enabledebuginfo(HSQUIRRELVM v, SQBool enable)
{
	_ss(v)->_debuginfo = enable?true:false;
}

void sq_enableasserts(HSQUIRRELVM v, SQBool enable)
{
	_ss(v)->_enableasserts = enable ? true : false;
}

void sq_enablehelp(HSQUIRRELVM v, SQBool enable)
{
	_ss(v)->_enablehelp = enable ? true : false;
}

void sq_notifyallexceptions(HSQUIRRELVM v, SQBool enable)
{
	_ss(v)->_notifyallexceptions = enable?true:false;
}

void sq_addref(HSQUIRRELVM v,HSQOBJECT *po)
{
	if(!SQ_ISREFCOUNTED(sqi_type(*po))) return;
#ifdef NO_GARBAGE_COLLECTOR
	__AddRef(po->_type,po->_unVal);
#else
	_ss(v)->_refs_table.AddRef(*po);
#endif
}

SQBool sq_release(HSQUIRRELVM v,HSQOBJECT *po)
{
	if(!SQ_ISREFCOUNTED(sqi_type(*po))) return SQTrue;
#ifdef NO_GARBAGE_COLLECTOR
	__Release(po->_type,po->_unVal);
	return SQFalse; //the ret val doesn't work(and cannot be fixed)
#else
	return _ss(v)->_refs_table.Release(*po);
#endif
}

const SQChar *sq_objtostring(HSQOBJECT *o) 
{
	if(sq_type(*o) == OT_STRING) {
		return sqi_stringval(*o);
	}
	return NULL;
}

SQInteger sq_objtointeger(HSQOBJECT *o) 
{
	if(sq_isnumeric(*o)) {
		return sqi_tointeger(*o);
	}
	return 0;
}

SQFloat sq_objtofloat(HSQOBJECT *o) 
{
	if(sq_isnumeric(*o)) {
		return sqi_tofloat(*o);
	}
	return 0;
}

SQBool sq_objtobool(HSQOBJECT *o) 
{
	if(sq_isbool(*o)) {
		return sqi_integer(*o);
	}
	return SQFalse;
}

HSQUIRRELVM sq_objtothread(HSQOBJECT* o)
{
	if(sq_isthread(*o)) {
		return sqi_thread(*o);
	}
	return SQFalse;
}

void sq_pushnull(HSQUIRRELVM v)
{
	v->Push(_null_);
}

void sq_pushstring(HSQUIRRELVM v,const SQChar *s,SQInteger len)
{
	if(s)
		v->Push(SQObjectPtr(SQString::Create(_ss(v), s, len)));
	else v->Push(_null_);
}

void sq_pushinteger(HSQUIRRELVM v,SQInteger n)
{
	v->Push(n);
}

void sq_pushbool(HSQUIRRELVM v,SQBool b)
{
	v->Push(b?true:false);
}

void sq_pushfloat(HSQUIRRELVM v,SQFloat n)
{
	v->Push(n);
}

void sq_pushuserpointer(HSQUIRRELVM v,SQUserPointer p)
{
	v->Push(p);
}

void sq_pushthread(HSQUIRRELVM v, HSQUIRRELVM thread)
{
	v->Push(thread);
}

SQUserPointer sq_newuserdata(HSQUIRRELVM v,SQUnsignedInteger size)
{
	SQUserData *ud = SQUserData::Create(_ss(v), size);
	v->Push(ud);
	return ud->_val;
}

void sq_newtable(HSQUIRRELVM v)
{
	v->Push(SQTable::Create(_ss(v), 0));	
}

void sq_newarray(HSQUIRRELVM v,SQInteger size)
{
	v->Push(SQArray::Create(_ss(v), size));	
}

SQRESULT sq_newclass(HSQUIRRELVM v,SQBool hasbase, const SQChar* name)
{
	SQClass *baseclass = NULL;
	if(hasbase) {
		SQObjectPtr &base = stack_get(v,-1);
		if(sqi_type(base) != OT_CLASS)
			return sq_throwerror(v,_SC("invalid base type"));
		baseclass = sqi_class(base);
	}
	SQClass *newclass = SQClass::Create(_ss(v), baseclass);
	if (name) newclass->_methods[0].val = SQString::Create(_ss(v), name, -1);
	if(baseclass) v->Pop();
	v->Push(newclass);	
	return SQ_OK;
}

SQBool sq_instanceof(HSQUIRRELVM v)
{
	SQObjectPtr &inst = stack_get(v,-1);
	SQObjectPtr &cl = stack_get(v,-2);
	if(sqi_type(inst) != OT_INSTANCE || sqi_type(cl) != OT_CLASS)
		return sq_throwerror(v,_SC("invalid param type"));
	return sqi_instance(inst)->InstanceOf(sqi_class(cl))?SQTrue:SQFalse;
}

SQRESULT sq_arrayappend(HSQUIRRELVM v,SQInteger idx)
{
	sq_aux_paramscheck(v,2);
	SQObjectPtr *arr;
	_GETSAFE_OBJ(v, idx, OT_ARRAY,arr);
	sqi_array(*arr)->Append(v->GetUp(-1));
	v->Pop(1);
	return SQ_OK;
}

SQRESULT sq_arraypop(HSQUIRRELVM v,SQInteger idx,SQBool pushval)
{
	sq_aux_paramscheck(v, 1);
	SQObjectPtr *arr;
	_GETSAFE_OBJ(v, idx, OT_ARRAY,arr);
	if(sqi_array(*arr)->Size() > 0) {
        if(pushval != 0){ v->Push(sqi_array(*arr)->Top()); }
		sqi_array(*arr)->Pop();
		return SQ_OK;
	}
	return sq_throwerror(v, _SC("empty array"));
}

SQRESULT sq_arrayresize(HSQUIRRELVM v,SQInteger idx,SQInteger newsize)
{
	sq_aux_paramscheck(v,1);
	SQObjectPtr *arr;
	_GETSAFE_OBJ(v, idx, OT_ARRAY,arr);
	if(newsize >= 0) {
		sqi_array(*arr)->Resize(newsize);
		return SQ_OK;
	}
	return sq_throwerror(v,_SC("negative size"));
}


SQRESULT sq_arrayreverse(HSQUIRRELVM v,SQInteger idx)
{
	sq_aux_paramscheck(v, 1);
	SQObjectPtr *o;
	_GETSAFE_OBJ(v, idx, OT_ARRAY,o);
	SQArray *arr = sqi_array(*o);
	if(arr->Size() > 0) {
		SQObjectPtr t;
		SQInteger size = arr->Size();
		SQInteger n = size >> 1; size -= 1;
		for(SQInteger i = 0; i < n; i++) {
			t = arr->_values[i];
			arr->_values[i] = arr->_values[size-i];
			arr->_values[size-i] = t;
		}
		return SQ_OK;
	}
	return SQ_OK;
}

SQRESULT sq_arrayremove(HSQUIRRELVM v,SQInteger idx,SQInteger itemidx)
{
	sq_aux_paramscheck(v, 1); 
	SQObjectPtr *arr;
	_GETSAFE_OBJ(v, idx, OT_ARRAY,arr); 
	return sqi_array(*arr)->Remove(itemidx) ? SQ_OK : sq_throwerror(v,_SC("index out of range")); 
}

SQRESULT sq_arrayinsert(HSQUIRRELVM v,SQInteger idx,SQInteger destpos)
{
	sq_aux_paramscheck(v, 1); 
	SQObjectPtr *arr;
	_GETSAFE_OBJ(v, idx, OT_ARRAY,arr);
	SQRESULT ret = sqi_array(*arr)->Insert(destpos, v->GetUp(-1)) ? SQ_OK : sq_throwerror(v,_SC("index out of range"));
	v->Pop();
	return ret;
}

SQUserPointer sq_newnativeitr(HSQUIRRELVM v, SQ_NATIVEITR_FUNC func, SQUnsignedInteger memsize)
{
	SQNativeItr* itr = SQNativeItr::Create(memsize, func);
	v->Push(SQObjectPtr(itr));
	return itr->_val;
}

void sq_newclosure(HSQUIRRELVM v,SQFUNCTION func,SQUnsignedInteger nfreevars)
{
	SQNativeClosure *nc = SQNativeClosure::Create(_ss(v), func);
	nc->_nparamscheck = 0;
	for(SQUnsignedInteger i = 0; i < nfreevars; i++) {
		nc->_outervalues.push_back(v->Top());
		v->Pop();
	}
	v->Push(SQObjectPtr(nc));	
}

SQRESULT sq_getclosureinfo(HSQUIRRELVM v,SQInteger idx,SQUnsignedInteger *nparams,SQUnsignedInteger *nfreevars)
{
	SQObject o = stack_get(v, idx);
	if(sq_isclosure(o)) {
		SQClosure *c = sqi_closure(o);
		SQFunctionProto *proto = c->_function;
		*nparams = (SQUnsignedInteger)proto->_nparameters;
		*nfreevars = (SQUnsignedInteger)proto->_noutervalues;
		return SQ_OK;
	}
	return sq_throwerror(v,_SC("the object is not a closure"));
}

SQRESULT sq_setnativeclosureinfo(HSQUIRRELVM v, SQInteger idx, const SQChar* name, const SQChar* file)
{
	SQObject o = stack_get(v, idx);
	if(sq_isnativeclosure(o)) {
		SQNativeClosure *nc = sqi_nativeclosure(o);
		nc->_name = name;
		nc->_file = file;
		return SQ_OK;
	}
	return sq_throwerror(v,_SC("the object is not a nativeclosure"));
}

SQRESULT sq_setparamscheck(HSQUIRRELVM v,SQInteger nparamscheck,const SQChar *typemask)
{
	SQObject o = stack_get(v, -1);
	if(!sq_isnativeclosure(o))
		return sq_throwerror(v, _SC("native closure expected"));
	SQNativeClosure *nc = sqi_nativeclosure(o);
	nc->_nparamscheck = nparamscheck;
	if(typemask) {
		SQIntVec res;
		if(!CompileTypemask(res, typemask))
			return sq_throwerror(v, _SC("invalid typemask"));
		nc->_typecheck.copy(res);
	}
	else {
		nc->_typecheck.resize(0);
	}
	if(nparamscheck == SQ_MATCHTYPEMASKSTRING) {
		nc->_nparamscheck = nc->_typecheck.size();
	}
	return SQ_OK;
}

SQRESULT sq_gethelp(HSQUIRRELVM v, SQInteger idx)
{
	SQObject o = stack_get(v, idx);
	if (sq_isnativeclosure(o))
	{
		SQNativeClosure* nc = sqi_nativeclosure(o);
		v->Push(nc->_help);
		return SQ_OK;
	}
	else if (sq_isclosure(o))
	{
		SQClosure* c = sqi_closure(o);
		v->Push(c->_function->_help);
		return SQ_OK;
	}
	else
	{
		return sq_throwerror(v, _SC("closure or native closure expected"));
	}
}

SQRESULT sq_sethelp(HSQUIRRELVM v, SQInteger idx)
{
	SQObject o = stack_get(v, idx);
	if (sq_isnativeclosure(o))
	{
		SQNativeClosure* nc = sqi_nativeclosure(o);
		nc->_help = stack_get(v, -1);
		v->Pop();
		return SQ_OK;
	}
	else if (sq_isclosure(o))
	{
		SQClosure* c = sqi_closure(o);
		c->_function->_help = stack_get(v, -1);
		v->Pop();
		return SQ_OK;
	}
	else
	{
		return sq_throwerror(v, _SC("closure or native closure expected"));
	}
}

SQRESULT sq_bindenv(HSQUIRRELVM v,SQInteger idx)
{
	SQObjectPtr &o = stack_get(v,idx);
	if(!sq_isnativeclosure(o) &&
		!sq_isclosure(o))
		return sq_throwerror(v,_SC("the target is not a closure"));
    SQObjectPtr &env = stack_get(v,-1);

	SQObjectPtr realval;
	_getrealval(env, v, realval);
	if(!sq_istable(realval) &&
		!sq_isclass(realval) &&
		!sq_isinstance(realval))
		return sq_throwerror(v,_SC("invalid environment"));

	SQObjectPtr ret;
	if(sq_isclosure(o)) {
		SQClosure *c = sqi_closure(o)->Clone();
		c->_env = env;
		GC_MUTATED(c);
		ret = c;
	}
	else { //then must be a native closure
		SQNativeClosure *c = sqi_nativeclosure(o)->Clone();
		c->_env = env;
		GC_MUTATED(c);
		ret = c;
	}
	v->Pop();
	v->Push(ret);
	return SQ_OK;
}

SQRESULT sq_getenv(HSQUIRRELVM v, SQInteger idx)
{
	SQObjectPtr &o = stack_get(v, idx);

	if (sq_isclosure(o))
	{
		v->Push(sqi_closure(o)->_env);
		return SQ_OK;
	}

	if (sq_isnativeclosure(o))
	{
		v->Push(sqi_nativeclosure(o)->_env);
		return SQ_OK;
	}

	return sq_throwerror(v,_SC("the target is not a closure"));
}

SQRESULT sq_clear(HSQUIRRELVM v,SQInteger idx)
{
	SQObject &o=stack_get(v,idx);
	switch(sqi_type(o)) {
		case OT_TABLE: sqi_table(o)->Clear();	break;
		case OT_ARRAY: sqi_array(o)->Resize(0); break;
		default:
			return sq_throwerror(v, _SC("clear only works on table and array"));
		break;

	}
	return SQ_OK;
}

void sq_pushroottable(HSQUIRRELVM v)
{
	v->Push(_ss(v)->_root_table);
}

SQRESULT sq_setthreadname(HSQUIRRELVM v, const SQChar* name, SQInteger len)
{
	v->_threadname = SQObjectPtr(SQString::Create(_ss(v), name, len));
	return SQ_OK;
}

SQRESULT sq_getthreadname(HSQUIRRELVM v, const SQChar **c)
{
	if (sqi_type(v->_threadname) == OT_STRING)
	{
		*c = sqi_stringval(v->_threadname);
		return SQ_OK;
	}

	return SQ_ERROR;
}

void sq_pushthreadlocal(HSQUIRRELVM v, HSQUIRRELVM th)
{
	v->Push(th->_threadlocal);
}

void sq_pushregistrytable(HSQUIRRELVM v)
{
	v->Push(_ss(v)->_registry);
}

void sq_pushconsttable(HSQUIRRELVM v)
{
	v->Push(_ss(v)->_consts);
}

SQRESULT sq_setroottable(HSQUIRRELVM v)
{
	SQObject o = stack_get(v, -1);
	if(sq_istable(o) || sq_isnull(o)) {
		_ss(v)->_root_table = o;
		v->Pop();
		return SQ_OK;
	}
	return sq_throwerror(v, _SC("invalid type"));
}

SQRESULT sq_setconsttable(HSQUIRRELVM v)
{
	SQObject o = stack_get(v, -1);
	if(sq_istable(o)) {
		_ss(v)->_consts = o;
		v->Pop();
		return SQ_OK;
	}
	return sq_throwerror(v, _SC("invalid type, expected table"));
}

void sq_setforeignptr(HSQUIRRELVM v,SQUserPointer p)
{
	v->_foreignptr = p;
}

SQUserPointer sq_getforeignptr(HSQUIRRELVM v)
{
	return v->_foreignptr;
}

void sq_push(HSQUIRRELVM v,SQInteger idx)
{
	v->Push(stack_get(v, idx));
}

SQObjectType sq_gettype(HSQUIRRELVM v,SQInteger idx)
{
	return sqi_type(stack_get(v, idx));
}


void sq_tostring(HSQUIRRELVM v,SQInteger idx)
{
	SQObjectPtr &o = stack_get(v, idx);
	SQObjectPtr res;
	v->ToString(o,res);
	v->Push(res);
}

void sq_tobool(HSQUIRRELVM v, SQInteger idx, SQBool *b)
{
	SQObjectPtr &o = stack_get(v, idx);
	*b = v->IsFalse(o)?SQFalse:SQTrue;
}

SQRESULT sq_getinteger(HSQUIRRELVM v,SQInteger idx,SQInteger *i)
{
	SQObjectPtr &o = stack_get(v, idx);
	if(sq_isnumeric(o)) {
		*i = sqi_tointeger(o);
		return SQ_OK;
	}
	return SQ_ERROR;
}

SQRESULT sq_getfloat(HSQUIRRELVM v,SQInteger idx,SQFloat *f)
{
	SQObjectPtr &o = stack_get(v, idx);
	if(sq_isnumeric(o)) {
		*f = sqi_tofloat(o);
		return SQ_OK;
	}
	return SQ_ERROR;
}

SQRESULT sq_getbool(HSQUIRRELVM v,SQInteger idx,SQBool *b)
{
	SQObjectPtr &o = stack_get(v, idx);
	if(sq_isbool(o)) {
		*b = sqi_integer(o);
		return SQ_OK;
	}
	return SQ_ERROR;
}

SQRESULT sq_getstring(HSQUIRRELVM v,SQInteger idx,const SQChar **c)
{
	SQObjectPtr *o = NULL;
	_GETSAFE_OBJ(v, idx, OT_STRING,o);
	*c = sqi_stringval(*o);
	return SQ_OK;
}

SQRESULT sq_getthread(HSQUIRRELVM v,SQInteger idx,HSQUIRRELVM *thread)
{
	SQObjectPtr *o = NULL;
	_GETSAFE_OBJ(v, idx, OT_THREAD,o);
	*thread = sqi_thread(*o);
	return SQ_OK;
}

HSQUIRRELVM sq_getcurrentthread(HSQUIRRELVM v)
{
	return _ss(v)->_curr_thread;
}

SQRESULT sq_clone(HSQUIRRELVM v,SQInteger idx)
{
	SQObjectPtr &o = stack_get(v,idx);
	v->Push(_null_);
	if(!v->Clone(o, stack_get(v, -1))){
		v->Pop();
		return sq_aux_invalidtype(v, sqi_type(o));
	}
	return SQ_OK;
}

SQInteger sq_getsize(HSQUIRRELVM v, SQInteger idx)
{
	SQObjectPtr &o = stack_get(v, idx);
	SQObjectType type = sqi_type(o);
	switch(type) {
	case OT_STRING:		return sqi_string(o)->_len;
	case OT_TABLE:		return sqi_table(o)->CountUsed();
	case OT_ARRAY:		return sqi_array(o)->Size();
	case OT_USERDATA:	return sqi_userdata(o)->_size;
	case OT_INSTANCE:	return sqi_instance(o)->_class ? sqi_instance(o)->_class->_udsize : 0;
	case OT_CLASS:		return sqi_class(o)->_udsize;
	default:
		return sq_aux_invalidtype(v, type);
	}
}

SQRESULT sq_getuserdata(HSQUIRRELVM v,SQInteger idx,SQUserPointer *p,SQUserPointer *typetag)
{
	SQObjectPtr *o = NULL;
	_GETSAFE_OBJ(v, idx, OT_USERDATA,o);
	(*p) = sqi_userdataval(*o);
	if(typetag) *typetag = sqi_userdata(*o)->_typetag;
	return SQ_OK;
}

SQRESULT sq_settypetag(HSQUIRRELVM v,SQInteger idx,SQUserPointer typetag)
{
	SQObjectPtr &o = stack_get(v,idx);
	switch(sqi_type(o)) {
		case OT_USERDATA:	sqi_userdata(o)->_typetag = typetag;	break;
		case OT_CLASS:		sqi_class(o)->_typetag = typetag;		break;
		default:			return sq_throwerror(v,_SC("invalid object type"));
	}
	return SQ_OK;
}

SQRESULT sq_getobjtypetag(HSQOBJECT *o,SQUserPointer * typetag)
{
  switch(sqi_type(*o)) {
	case OT_INSTANCE: { SQClass* cls = sqi_instance(*o)->_class; *typetag = cls ? cls->_typetag : NULL; } break;
    case OT_USERDATA: *typetag = sqi_userdata(*o)->_typetag; break;
    case OT_CLASS:    *typetag = sqi_class(*o)->_typetag; break;
    default: return SQ_ERROR;
  }
  return SQ_OK;
}

SQRESULT sq_gettypetag(HSQUIRRELVM v,SQInteger idx,SQUserPointer *typetag)
{
	SQObjectPtr &o = stack_get(v,idx);
	if(SQ_FAILED(sq_getobjtypetag(&o,typetag)))
		return sq_throwerror(v,_SC("invalid object type"));
	return SQ_OK;
}

SQRESULT sq_getuserpointer(HSQUIRRELVM v, SQInteger idx, SQUserPointer *p)
{
	SQObjectPtr *o = NULL;
	_GETSAFE_OBJ(v, idx, OT_USERPOINTER,o);
	(*p) = sqi_userpointer(*o);
	return SQ_OK;
}

SQRESULT sq_setinstanceup(HSQUIRRELVM v, SQInteger idx, SQUserPointer p)
{
	SQObjectPtr &o = stack_get(v,idx);
	if(sqi_type(o) != OT_INSTANCE) return sq_throwerror(v,_SC("the object is not a class instance"));
	sqi_instance(o)->_userpointer = p;
	return SQ_OK;
}

SQRESULT sq_setclassudsize(HSQUIRRELVM v, SQInteger idx, SQInteger udsize)
{
	SQObjectPtr &o = stack_get(v,idx);
	if(sqi_type(o) != OT_CLASS) return sq_throwerror(v,_SC("the object is not a class"));
	if(sqi_class(o)->_locked) return sq_throwerror(v,_SC("the class is locked"));
	sqi_class(o)->_udsize = udsize;
	return SQ_OK;
}


SQRESULT sq_getinstanceup(HSQUIRRELVM v, SQInteger idx, SQUserPointer *p,SQUserPointer typetag)
{
	SQObjectPtr &o = stack_get(v,idx);
	if(sqi_type(o) != OT_INSTANCE) return sq_throwerror(v,_SC("the object is not a class instance"));
	(*p) = sqi_instance(o)->_userpointer;
	if(typetag != 0) {
		SQClass *cl = sqi_instance(o)->_class;
		if (cl == NULL) return SQ_OK;
		do{
			if(cl->_typetag == typetag)
				return SQ_OK;
			cl = cl->_base;
		}while(cl != NULL);
		return sq_throwerror(v,_SC("invalid type tag"));
	}
	return SQ_OK;
}

SQInteger sq_gettop(HSQUIRRELVM v)
{
	return (v->_top) - v->_stackbase;
}

void sq_settop(HSQUIRRELVM v, SQInteger newtop)
{
	SQInteger top = sq_gettop(v);
	if(top > newtop)
		sq_pop(v, top - newtop);
	else
		while(top++ < newtop) sq_pushnull(v);
}

void sq_pop(HSQUIRRELVM v, SQInteger nelemstopop)
{
	assert(v->_top >= nelemstopop);
	v->Pop(nelemstopop);
}

void sq_poptop(HSQUIRRELVM v)
{
	assert(v->_top >= 1);
    v->Pop();
}

void sq_insert(HSQUIRRELVM v, SQInteger idx, SQInteger count)
{
	assert(count > 0);
	v->Insert(idx, count);
}

void sq_remove(HSQUIRRELVM v, SQInteger idx)
{
	v->Remove(idx);
}

void sq_replace(HSQUIRRELVM v, SQInteger idx)
{
	v->Replace(idx);
}

SQInteger sq_cmp(HSQUIRRELVM v)
{
	SQInteger res;
	v->ObjCmp(stack_get(v, -1), stack_get(v, -2),res);
	return res;
}

SQRESULT sq_newslot(HSQUIRRELVM v, SQInteger idx, SQBool bstatic)
{
	sq_aux_paramscheck(v, 3);
	SQObjectPtr &self = stack_get(v, idx);
	if(sqi_type(self) == OT_TABLE || sqi_type(self) == OT_CLASS || sqi_type(self) == OT_THREAD) {
		SQObjectPtr &key = v->GetUp(-2);
		if(sqi_type(key) == OT_NULL) return sq_throwerror(v, _SC("null is not a valid key"));
		if (!v->NewSlot(self, key, v->GetUp(-1),bstatic?true:false)) return SQ_ERROR;
		v->Pop(2);
		return SQ_OK;
	}
	return sq_throwerror(v, _SC("newslot on non-table/class/thread"));
}

SQRESULT sq_deleteslot(HSQUIRRELVM v,SQInteger idx,SQBool pushval)
{
	sq_aux_paramscheck(v, 2);
	SQObjectPtr& self = stack_get(v, idx);
	if (sqi_type(self) == OT_TABLE || sqi_type(self) == OT_THREAD)
	{
		SQObjectPtr &key = v->GetUp(-1);
		if(sqi_type(key) == OT_NULL) return sq_throwerror(v, _SC("null is not a valid key"));
		SQObjectPtr res;
		if(!v->DeleteSlot(self, key, res)){
			return SQ_ERROR;
		}
		if(pushval)	v->GetUp(-1) = res;
		else v->Pop(1);
		return SQ_OK;
	}
	return sq_throwerror(v, _SC("delete on non-table/thread"));
}

SQRESULT sq_newproperty(HSQUIRRELVM v, SQInteger idx)
{
	sq_aux_paramscheck(v, 4);
	SQObjectPtr &self = stack_get(v, idx);
	if (sqi_type(self) != OT_CLASS) return sq_throwerror(v, _SC("only classes can have a property"));

	SQObjectPtr &name = v->GetUp(-3);
	if(sqi_type(name) == OT_NULL) return sq_throwerror(v, _SC("null is not a valid property name"));
	if (!v->NewProp(self, name, v->GetUp(-2), v->GetUp(-1))) return SQ_ERROR;
	v->Pop(3);

	return SQ_OK;
}

SQRESULT sq_getproperty(HSQUIRRELVM v, SQInteger idx)
{
	SQObjectPtr &self = stack_get(v,idx);
	if (sqi_type(self) != OT_CLASS) return sq_throwerror(v, _SC("only classes can have a property"));

	SQObjectPtr& name = v->GetUp(-1);
	if(sqi_type(name) == OT_NULL) return sq_throwerror(v, _SC("null is not a valid property name"));

	SQObjectPtr getter = _null_;
	SQObjectPtr setter = _null_;
	bool hasProperty = sqi_class(self)->GetProperty(name, getter, setter);

	v->Pop();
	if (!hasProperty) return sq_throwerror(v, _SC("property not foud"));

	v->Push(getter);
	v->Push(setter);
	return SQ_OK;
}

SQRESULT sq_set(HSQUIRRELVM v,SQInteger idx)
{
	SQObjectPtr &self = stack_get(v, idx);
	if(v->Set(self, v->GetUp(-2), v->GetUp(-1),false,DONT_FALL_BACK)) {
		v->Pop(2);
		return SQ_OK;
	}
	return SQ_ERROR;
}

SQRESULT sq_rawset(HSQUIRRELVM v,SQInteger idx)
{
	SQObjectPtr &self = stack_get(v, idx);
	if(sqi_type(v->GetUp(-2)) == OT_NULL) return sq_throwerror(v, _SC("null key"));
	switch(sqi_type(self)) {
	case OT_TABLE:
		sqi_table(self)->NewSlot(v->GetUp(-2), v->GetUp(-1));
		v->Pop(2);
		return SQ_OK;
	break;
	case OT_CLASS:
		sqi_class(self)->NewSlot(_ss(v), v->GetUp(-2), v->GetUp(-1),false);
		v->Pop(2);
		return SQ_OK;
	break;
	case OT_INSTANCE:
		if(sqi_instance(self)->Set(v, v->GetUp(-2), v->GetUp(-1)) == SQInstance::IA_OK) {
			v->Pop(2);
			return SQ_OK;
		}
	break;
	case OT_ARRAY:
		if(v->Set(self, v->GetUp(-2), v->GetUp(-1),true,0)) {
			v->Pop(2);
			return SQ_OK;
		}
	break;
	default:
		v->Pop(2);
		return sq_throwerror(v, _SC("rawset works only on array/table/class and instance"));
	}
	v->Raise_IdxError(v->GetUp(-2));return SQ_ERROR;
}

SQRESULT sq_setdelegate(HSQUIRRELVM v,SQInteger idx)
{
	SQObjectPtr &self = stack_get(v, idx);
	SQObjectPtr &mt = v->GetUp(-1);
	SQObjectType type = sqi_type(self);
	switch(type) {
	case OT_TABLE:
		if(sqi_type(mt) == OT_TABLE) {
			if(!sqi_table(self)->SetDelegate(sqi_table(mt))) return sq_throwerror(v, _SC("delagate cycle")); v->Pop();}
		else if(sqi_type(mt)==OT_NULL) {
			sqi_table(self)->SetDelegate(NULL); v->Pop(); }
		else return sq_aux_invalidtype(v,type);
		break;
	case OT_USERDATA:
		if(sqi_type(mt)==OT_TABLE) {
			sqi_userdata(self)->SetDelegate(sqi_table(mt)); v->Pop(); }
		else if(sqi_type(mt)==OT_NULL) {
			sqi_userdata(self)->SetDelegate(NULL); v->Pop(); }
		else return sq_aux_invalidtype(v, type);
		break;
	default:
			return sq_aux_invalidtype(v, type);
		break;
	}
	return SQ_OK;
}

SQRESULT sq_rawdeleteslot(HSQUIRRELVM v,SQInteger idx,SQBool pushval)
{
	sq_aux_paramscheck(v, 2);
	SQObjectPtr *self;
	_GETSAFE_OBJ(v, idx, OT_TABLE,self);
	SQObjectPtr &key = v->GetUp(-1);
	SQObjectPtr t;
	if(sqi_table(*self)->Get(key,t)) {
		sqi_table(*self)->Remove(key);
	}
	if(pushval != 0)
		v->GetUp(-1) = t;
	else
		v->Pop(1);
	return SQ_OK;
}

SQInteger sq_rawid(HSQUIRRELVM v, SQInteger idx)
{
	return v->ToRawID(stack_get(v, idx));
}

SQRESULT sq_getdelegate(HSQUIRRELVM v,SQInteger idx)
{
	SQObjectPtr &self=stack_get(v,idx);
	switch(sqi_type(self)){
	case OT_TABLE:
	case OT_USERDATA:
		if(!sqi_delegable(self)->_delegate){
			v->Push(_null_);
			break;
		}
		v->Push(SQObjectPtr(sqi_delegable(self)->_delegate));
		break;
	default: return sq_throwerror(v,_SC("wrong type")); break;
	}
	return SQ_OK;
	
}

SQRESULT sq_get(HSQUIRRELVM v,SQInteger idx)
{
	SQObjectPtr &self=stack_get(v,idx);
	if(v->Get(self,v->GetUp(-1),v->GetUp(-1),false,DONT_FALL_BACK))
		return SQ_OK;
	v->Pop(1);
	return SQ_ERROR;
}

SQRESULT sq_rawget(HSQUIRRELVM v,SQInteger idx)
{
	SQObjectPtr &self=stack_get(v,idx);
	switch(sqi_type(self)) {
	case OT_TABLE:
		if(sqi_table(self)->Get(v->GetUp(-1),v->GetUp(-1)))
			return SQ_OK;
		break;
	case OT_CLASS:
		if(sqi_class(self)->Get(v->GetUp(-1),v->GetUp(-1)))
			return SQ_OK;
		break;
	case OT_INSTANCE:
		if(sqi_instance(self)->Get(v, v->GetUp(-1),v->GetUp(-1)) == SQInstance::IA_OK)
			return SQ_OK;
		break;
	case OT_ARRAY:{
		SQObjectPtr& key = v->GetUp(-1);
		if(sq_isnumeric(key)){
			if(sqi_array(self)->Get(sqi_tointeger(key),v->GetUp(-1))) {
			return SQ_OK;
			}
		}
		else {
			v->Pop(1);
			return sq_throwerror(v,_SC("invalid index type for an array"));
		}
				  }
		break;
	default:
		v->Pop(1);
		return sq_throwerror(v,_SC("rawget works only on array/table/instance and class"));
	}	
	v->Pop(1);
	return sq_throwerror(v,_SC("the index doesn't exist"));
}

SQRESULT sq_getstackobj(HSQUIRRELVM v,SQInteger idx,HSQOBJECT *po)
{
	*po=stack_get(v,idx);
	return SQ_OK;
}

const SQChar *sq_getlocal(HSQUIRRELVM v,SQUnsignedInteger level,SQUnsignedInteger idx)
{
	SQUnsignedInteger cstksize=v->_callsstacksize;
	SQUnsignedInteger lvl=(cstksize-level)-1;
	SQInteger stackbase=v->_stackbase;
	if(lvl<cstksize){
		for(SQUnsignedInteger i=0;i<level;i++){
			SQVM::CallInfo &ci=v->_callsstack[(cstksize-i)-1];
			stackbase-=ci._prevstkbase;
		}
		SQVM::CallInfo &ci=v->_callsstack[lvl];
		if(sqi_type(ci._closure)!=OT_CLOSURE)
			return NULL;
		SQClosure *c=sqi_closure(ci._closure);
		SQFunctionProto *func=c->_function;
		if(func->_noutervalues > (SQInteger)idx) {
			v->Push(*sqi_outer(c->_outervalues[idx])->_valptr);
			return sqi_stringval(func->_outervalues[idx]._name);
		}
		idx -= func->_noutervalues;
		return func->GetLocal(v,stackbase,idx,(SQInteger)(ci._ip-func->_instructions)-1);
	}
	return NULL;
}

void sq_pushobject(HSQUIRRELVM v,HSQOBJECT obj)
{
	v->Push(SQObjectPtr(obj));
}

void sq_resetobject(HSQOBJECT *po)
{
	po->_unVal.pUserPointer=NULL;po->_type=OT_NULL;
}

SQRESULT sq_throwerror(HSQUIRRELVM v,const SQChar *err)
{
	v->_nativeException = false;
	v->_lasterror=SQString::Create(_ss(v),err);
	return -1;
}

SQRESULT sq_throwfmt(HSQUIRRELVM v, const SQChar* s, ...)
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
	return sq_throwerror(v, buf);
}

void sq_reseterror(HSQUIRRELVM v)
{
	v->_lasterror = _null_;
}

void sq_getlasterror(HSQUIRRELVM v)
{
	v->Push(v->_lasterror);
}

void sq_setlasterror(HSQUIRRELVM v)
{
	v->_lasterror = v->GetUp(-1);
	v->Pop();
}

void sq_callerrorhandler(HSQUIRRELVM v)
{
	v->CallErrorHandler(v->_lasterror);
}

SQRESULT sq_reservestack(HSQUIRRELVM v,SQInteger nsize)
{
	if (((SQUnsignedInteger)v->_top + nsize) > v->_stack.size()) {
		if(v->_nmetamethodscall) {
			_ss(v)->_errorfunc(v, "*** [FATAL] stack overflow in a metamethod");
			return sq_throwerror(v,_SC("cannot resize stack while in a metamethod"));
		}
		v->_stack.resize(v->_stack.size() + ((v->_top + nsize) - v->_stack.size()));
		v->RelocateOuters();
	}
	return SQ_OK;
}

SQRESULT sq_resume(HSQUIRRELVM v,SQBool retval,SQBool raiseerror)
{
	if(sqi_type(v->GetUp(-1))==OT_GENERATOR){
		v->Push(_null_); //retval
		if(!v->Execute(v->GetUp(-2),0,v->_top,v->GetUp(-1),raiseerror,SQVM::ET_RESUME_GENERATOR))
		{v->Raise_Error(v->_lasterror); return SQ_ERROR;}
		if(!retval)
			v->Pop();
		return SQ_OK;
	}
	return sq_throwerror(v,_SC("only generators can be resumed"));
}

SQRESULT sq_call(HSQUIRRELVM v,SQInteger params,SQBool retval,SQBool raiseerror)
{
	SQObjectPtr res;
	if(v->Call(v->GetUp(-(params+1)),params,v->_top-params,res,raiseerror?true:false)){
		if(!v->_suspended) {
			v->Pop(params);//pop closure and args
		}
		if(retval){
			v->Push(res); return SQ_OK;
		}
		return SQ_OK;
	}
	else {
		v->Pop(params);
		return SQ_ERROR;
	}
	if(!v->_suspended)
		v->Pop(params);
	return sq_throwerror(v,_SC("call failed"));
}

SQRESULT sq_suspendvm(HSQUIRRELVM v)
{
	return v->Suspend();
}

SQRESULT sq_wakeupvm(HSQUIRRELVM v,SQBool wakeupret,SQBool retval,SQBool raiseerror,SQBool throwerror)
{
	SQObjectPtr ret;
	if(!v->_suspended)
		return sq_throwerror(v,_SC("cannot resume a vm that is not running any code"));
	if(wakeupret) {
		v->GetAt(v->_stackbase+v->_suspended_target)=v->GetUp(-1); //retval
		v->Pop();
	} else v->GetAt(v->_stackbase+v->_suspended_target)=_null_;
	if(!v->Execute(_null_,-1,-1,ret,raiseerror,throwerror?SQVM::ET_RESUME_THROW_VM : SQVM::ET_RESUME_VM)) {
		return SQ_ERROR;
	}
	if(retval)
		v->Push(ret);
	return SQ_OK;
}

void sq_setreleasehook(HSQUIRRELVM v,SQInteger idx,SQRELEASEHOOK hook)
{
	if(sq_gettop(v) >= 1){
		SQObjectPtr &ud=stack_get(v,idx);
		switch( sqi_type(ud) ) {
		case OT_USERDATA:	sqi_userdata(ud)->_hook = hook;	break;
		case OT_INSTANCE:	sqi_instance(ud)->_hook = hook;	break;
		case OT_CLASS:		sqi_class(ud)->_hook = hook;		break;
		default: break; //shutup compiler
		}
	}
}

void sq_setcompilererrorhandler(HSQUIRRELVM v,SQCOMPILERERROR f)
{
	_ss(v)->_compilererrorhandler = f;
}

static void flipEndian(void * pData, size_t size)
{
	char swapByte;
	for(unsigned int byteIndex = 0; byteIndex < size/2; byteIndex++)
	{
		swapByte = *(char *)((size_t)pData + byteIndex);
		*(char *)((size_t)pData + byteIndex) = *(char *)((size_t)pData + size - byteIndex - 1);
		*(char *)((size_t)pData + size - byteIndex - 1) = swapByte;
	}
}

SQRESULT sq_writeclosure(HSQUIRRELVM v,SQWRITEFUNC w,SQUserPointer up, SQBool swapEndian)
{
	SQObjectPtr *o = NULL;
	_GETSAFE_OBJ(v, -1, OT_CLOSURE,o);
	unsigned short tag = SQ_BYTECODE_STREAM_TAG;
	if (swapEndian) flipEndian(&tag, sizeof(tag));
	if(w(up,&tag,2) != 2)
		return sq_throwerror(v,_SC("io error"));
	if(!sqi_closure(*o)->Save(v,up,w, swapEndian))
		return SQ_ERROR;
	return SQ_OK;
}

SQRESULT sq_readclosure(HSQUIRRELVM v,SQREADFUNC r,SQUserPointer up)
{
	SQObjectPtr closure;
	SQObjectPtr imports = SQTable::Create(_ss(v), 0);
	unsigned short tag;
	if(r(up,&tag,2) != 2)
		return sq_throwerror(v,_SC("io error"));
	if(tag != SQ_BYTECODE_STREAM_TAG)
		return sq_throwerror(v,_SC("invalid stream"));
	if(!SQClosure::Load(v,up,r,imports,closure))
		return SQ_ERROR;
	v->Push(closure);
	return SQ_OK;
}

SQChar *sq_getscratchpad(HSQUIRRELVM v,SQInteger minsize)
{
	return _ss(v)->GetScratchPad(minsize);
}

void sq_pushscratchpad(HSQUIRRELVM v)
{
	_ss(v)->PushScratchPad();
}

void sq_popscratchpad(HSQUIRRELVM v)
{
	_ss(v)->PopScratchPad();
}

SQInteger sq_collectgarbage(HSQUIRRELVM v)
{
#ifndef NO_GARBAGE_COLLECTOR
	return _ss(v)->CollectGarbage(v);
#else
	return -1;
#endif
}

#ifndef NO_GARBAGE_COLLECTOR
void sq_gcsetparam(HSQUIRRELVM v, SQInteger markratio, SQInteger sweepratio)
{
	SQGC& gc = _ss(v)->_gc;

	gc._markratio = markratio;
	gc._sweepratio = sweepratio;
}

static void stepinfocallback(SQGC* gc, SQUserPointer up)
{
	SQGCInfo* info = (SQGCInfo*)up;

	info->objecttotal = gc->_objtotal;
	info->entercount = gc->_entercount;
	info->leavecount = gc->_leavecount;
	info->mutatecount = gc->_mutatecount;
	info->markcount = gc->_markcount;
	info->sweepcount = gc->_sweepcount;
	info->marktotal = gc->_marktotal;
	info->sweeptotal = gc->_sweeptotal;
}

void sq_gcstep(HSQUIRRELVM v, SQGCInfo* outInfo)
{
	SQGC& gc = _ss(v)->_gc;

	if (outInfo)
		gc.step(stepinfocallback, outInfo);
	else
		gc.step();
}
#else
void sq_gcsetparam(HSQUIRRELVM v, SQInteger markratio, SQInteger sweepratio)
{
}

void sq_gcstep(HSQUIRRELVM v, SQGCInfo* outInfo)
{
	memset(outInfo, 0, sizeof(outInfo));
	outInfo->state = "disabled";
	outInfo->bool = SQFalse;
}
#endif

const SQChar *sq_getfreevariable(HSQUIRRELVM v,SQInteger idx,SQUnsignedInteger nval)
{
	SQObjectPtr &self = stack_get(v,idx);
	const SQChar *name = NULL;
	if(sqi_type(self) == OT_CLOSURE) {
		SQClosure *clo = sqi_closure(self);
		SQFunctionProto *fp = clo->_function;
		if(((SQUnsignedInteger)fp->_noutervalues) > nval) {
			v->Push(*(sqi_outer(clo->_outervalues[nval])->_valptr));
			SQOuterVar &ov = fp->_outervalues[nval];
			name = sqi_stringval(ov._name);
		}
	}
	return name;
}

SQRESULT sq_setfreevariable(HSQUIRRELVM v,SQInteger idx,SQUnsignedInteger nval)
{
	SQObjectPtr &self=stack_get(v,idx);
	switch(sqi_type(self))
	{
	case OT_CLOSURE:{
		SQFunctionProto *fp = sqi_closure(self)->_function;
		if(((SQUnsignedInteger)fp->_noutervalues) > nval){
			SQOuter* outer = sqi_outer(sqi_closure(self)->_outervalues[nval]);
			GC_MUTATED(outer);
			*(outer->_valptr) = stack_get(v,-1);
		}
		else return sq_throwerror(v,_SC("invalid free var index"));
					}
		break;
	case OT_NATIVECLOSURE:
		if(sqi_nativeclosure(self)->_outervalues.size()>nval){
			SQNativeClosure* nc = sqi_nativeclosure(self);
			GC_MUTATED(nc);
			nc->_outervalues[nval]=stack_get(v,-1);
		}
		else return sq_throwerror(v,_SC("invalid free var index"));
		break;
	default:
		return sq_aux_invalidtype(v,sqi_type(self));
	}
	v->Pop(1);
	return SQ_OK;
}

SQRESULT sq_setattributes(HSQUIRRELVM v,SQInteger idx)
{
	SQObjectPtr *o = NULL;
	_GETSAFE_OBJ(v, idx, OT_CLASS,o);
	SQObjectPtr &key = stack_get(v,-2);
	SQObjectPtr &val = stack_get(v,-1);
	SQObjectPtr attrs;
	if(sqi_type(key) == OT_NULL) {
		attrs = sqi_class(*o)->_attributes;
		sqi_class(*o)->_attributes = val;
		v->Pop(2);
		v->Push(attrs);
		return SQ_OK;
	}else if(sqi_class(*o)->GetAttributes(key,attrs)) {
		sqi_class(*o)->SetAttributes(key,val);
		v->Pop(2);
		v->Push(attrs);
		return SQ_OK;
	}
	return sq_throwerror(v,_SC("wrong index"));
}

SQRESULT sq_getattributes(HSQUIRRELVM v,SQInteger idx)
{
	SQObjectPtr *o = NULL;
	_GETSAFE_OBJ(v, idx, OT_CLASS,o);
	SQObjectPtr &key = stack_get(v,-1);
	SQObjectPtr attrs;
	if(sqi_type(key) == OT_NULL) {
		attrs = sqi_class(*o)->_attributes;
		v->Pop();
		v->Push(attrs); 
		return SQ_OK;
	}
	else if(sqi_class(*o)->GetAttributes(key,attrs)) {
		v->Pop();
		v->Push(attrs);
		return SQ_OK;
	}
	return sq_throwerror(v,_SC("wrong index"));
}

SQRESULT sq_getmemberhandle(HSQUIRRELVM v,SQInteger idx,HSQMEMBERHANDLE *handle)
{
	SQObjectPtr *o = NULL;
	_GETSAFE_OBJ(v, idx, OT_CLASS,o);
	SQObjectPtr &key = stack_get(v,-1);
	SQClass *c = sqi_class(*o);
	SQObjectPtr val;
	if(c->Get(key,val)) {
		handle->_static = _isfield(val) ? SQFalse : SQTrue;
		handle->_index = _member_idx(val);
		v->Pop();
		return SQ_OK;
	}
	return sq_throwerror(v,_SC("wrong index"));
}

SQRESULT _getmemberbyhandle(HSQUIRRELVM v,SQObjectPtr &self,HSQMEMBERHANDLE *handle,SQObjectPtr *val)
{
	switch(sqi_type(self)) {
		case OT_INSTANCE: {
				SQInstance *i = sqi_instance(self);
				if(handle->_static) {
					SQClass *c = i->_class;
					val = &c->_methods[handle->_index].val;
				}
				else {
					val = &i->_values[handle->_index];
					
				}
			}
			break;
		case OT_CLASS: {
				SQClass *c = sqi_class(self);
				if(handle->_static) {
					val = &c->_methods[handle->_index].val;
				}
				else {
					val = &c->_defaultvalues[handle->_index].val;
				}
			}
			break;
		default:
			return sq_throwerror(v,_SC("wrong type(expected class or instance)"));
	}
	return SQ_OK;
}

SQRESULT sq_getbyhandle(HSQUIRRELVM v,SQInteger idx,HSQMEMBERHANDLE *handle)
{
	SQObjectPtr &self = stack_get(v,idx);
	SQObjectPtr *val = NULL;
	if(SQ_FAILED(_getmemberbyhandle(v,self,handle,val))) {
		return SQ_ERROR;
	}
	SQObjectPtr value;
	_getrealval(*val, v, value);
	v->Push(value);
	return SQ_OK;
}

SQRESULT sq_setbyhandle(HSQUIRRELVM v,SQInteger idx,HSQMEMBERHANDLE *handle)
{
	SQObjectPtr &self = stack_get(v,idx);
	SQObjectPtr &newval = stack_get(v,-1);
	SQObjectPtr *val = NULL;
	if(SQ_FAILED(_getmemberbyhandle(v,self,handle,val))) {
		return SQ_ERROR;
	}
	*val = newval;
	v->Pop();
	return SQ_OK;
}

SQRESULT sq_getbase(HSQUIRRELVM v,SQInteger idx)
{
	SQObjectPtr *o = NULL;
	_GETSAFE_OBJ(v, idx, OT_CLASS,o);
	if(sqi_class(*o)->_base)
		v->Push(SQObjectPtr(sqi_class(*o)->_base));
	else
		v->Push(_null_);
	return SQ_OK;
}

SQRESULT sq_getclass(HSQUIRRELVM v,SQInteger idx)
{
	SQObjectPtr *o = NULL;
	_GETSAFE_OBJ(v, idx, OT_INSTANCE,o);
	SQClass* c = sqi_instance(*o)->_class;
	if (c)
		v->Push(c);
	else
		v->Push(_null_);
	return SQ_OK;
}

SQRESULT sq_createinstance(HSQUIRRELVM v,SQInteger idx)
{
	SQObjectPtr *o = NULL;
	_GETSAFE_OBJ(v, idx, OT_CLASS,o);
	v->Push(sqi_class(*o)->CreateInstance());
	return SQ_OK;
}

SQRESULT sq_purgeinstance(HSQUIRRELVM v, SQInteger idx)
{
	SQObjectPtr &o = stack_get(v,idx);
	if(sqi_type(o) != OT_INSTANCE) return sq_throwerror(v,_SC("the object is not a class instance"));
	sqi_instance(o)->Purge();
	return SQ_OK;
}

void sq_weakref(HSQUIRRELVM v, SQInteger idx, SQBool useMetamethod)
{
	SQObject &o=stack_get(v,idx);

	switch (sqi_type(o))
	{
	case OT_TABLE:
	case OT_USERDATA:
	case OT_INSTANCE:
		if (useMetamethod && sqi_delegable(o)->_delegate)
		{
			v->Push(o);
			SQObjectPtr res;
			if (v->CallMetaMethod(sqi_delegable(o), MT_WEAKREF, 1, res) == SQVM::META_SUCCESS)
			{
				v->Push(res);
				return;
			}
		}
		// no break, go through

	default:
		if(SQ_ISREFCOUNTED(sqi_type(o))) {
			v->Push(sqi_refcounted(o)->GetWeakRef(sqi_type(o)));
			return;
		}
		v->Push(o);
	}
}

SQRESULT sq_getweakrefval(HSQUIRRELVM v,SQInteger idx)
{
	SQObjectPtr &o = stack_get(v,idx);

	if (sqi_type(o) == OT_WEAKREF)
	{
		v->Push(sqi_weakref(o)->_obj);
		return SQ_OK;
	}
	else if (sqi_type(o) == OT_NATIVEWEAKREF)
	{
		SQObjectPtr val;
		SQRESULT sr = sqi_nativeweakref(o)->GetRef(v, val);
		if (SQ_SUCCEEDED(sr))
			v->Push(val);
		return sr;
	}

	return sq_throwerror(v,_SC("the object must be a (native) weakref"));
}

SQRESULT sq_getdefaultdelegate(HSQUIRRELVM v,SQObjectType t)
{
	SQSharedState *ss = _ss(v);
	switch(t) {
	case OT_TABLE: v->Push(ss->_table_default_delegate); break;
	case OT_ARRAY: v->Push(ss->_array_default_delegate); break;
	case OT_STRING: v->Push(ss->_string_default_delegate); break;
	case OT_INTEGER: case OT_FLOAT: v->Push(ss->_number_default_delegate); break;
	case OT_GENERATOR: v->Push(ss->_generator_default_delegate); break;
	case OT_CLOSURE: case OT_NATIVECLOSURE: v->Push(ss->_closure_default_delegate); break;
	case OT_THREAD: v->Push(ss->_thread_default_delegate); break;
	case OT_CLASS: v->Push(ss->_class_default_delegate); break;
	case OT_INSTANCE: v->Push(ss->_instance_default_delegate); break;
	case OT_WEAKREF: v->Push(ss->_weakref_default_delegate); break;
	case OT_NATIVEWEAKREF: v->Push(ss->_nativeweakref_default_delegate); break;
	default: return sq_throwerror(v,_SC("the type doesn't have a default delegate"));
	}
	return SQ_OK;
}

SQRESULT sq_next(HSQUIRRELVM v,SQInteger idx)
{
	SQObjectPtr o=stack_get(v,idx),&refpos = stack_get(v,-1),realkey,val;
	if(sqi_type(o) == OT_GENERATOR) {
		return sq_throwerror(v,_SC("cannot iterate a generator"));
	}
	int faketojump;
	if(!v->FOREACH_OP(o,realkey,val,refpos,0,666,faketojump))
		return SQ_ERROR;
	if(faketojump != 666) {
		v->Push(realkey);
		v->Push(val);
		return SQ_OK;
	}
	return SQ_ERROR;
}

struct BufState{
	const SQChar *buf;
	SQInteger ptr;
	SQInteger size;
};

SQInteger buf_lexfeed(SQUserPointer file)
{
	BufState *buf=(BufState*)file;
	if(buf->size<(buf->ptr+1))
		return 0;
	return buf->buf[buf->ptr++];
}

SQRESULT sq_compilebuffer(HSQUIRRELVM v,const SQChar *s,SQInteger size,const SQChar *sourcename,SQBool raiseerror) {
	BufState buf;
	buf.buf = s;
	buf.size = size;
	buf.ptr = 0;
	return sq_compile(v, buf_lexfeed, &buf, sourcename, raiseerror);
}

void sq_move(HSQUIRRELVM dest,HSQUIRRELVM src,SQInteger idx)
{
	dest->Push(stack_get(src,idx));
}

void sq_setprintfunc(HSQUIRRELVM v, SQPRINTFUNCTION printfunc,SQPRINTFUNCTION errfunc)
{
	_ss(v)->_printfunc = printfunc;
	_ss(v)->_errorfunc = errfunc;
}

SQPRINTFUNCTION sq_getprintfunc(HSQUIRRELVM v)
{
	return _ss(v)->_printfunc;
}

SQPRINTFUNCTION sq_geterrorfunc(HSQUIRRELVM v)
{
	return _ss(v)->_errorfunc;
}

void *sq_malloc(SQUnsignedInteger size)
{
	return SQ_MALLOC(size);
}

void *sq_realloc(void* p,SQUnsignedInteger oldsize,SQUnsignedInteger newsize)
{
	return SQ_REALLOC(p,oldsize,newsize);
}

void sq_free(void *p,SQUnsignedInteger size)
{
	SQ_FREE(p,size);
}
