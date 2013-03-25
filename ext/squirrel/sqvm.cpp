/*
	see copyright notice in squirrel.h
*/
#include "sqpcheader.h"
#include <math.h>
#include <stdlib.h>
#include "sqopcodes.h"
#include "sqvm.h"
#include "sqfuncproto.h"
#include "sqclosure.h"
#include "sqstring.h"
#include "sqtable.h"
#include "squserdata.h"
#include "sqarray.h"
#include "sqclass.h"

#include "nit/nit.h"
#include "nit/runtime/Exception.h"
#undef Yield

#define TOP() (_stack._vals[_top-1])

bool SQVM::BW_OP(SQUnsignedInteger op,SQObjectPtr &trg,const SQObjectPtr &o1,const SQObjectPtr &o2)
{
	SQInteger res;
	if((sqi_type(o1)|sqi_type(o2)) == OT_INTEGER)
	{
		SQInteger i1 = sqi_integer(o1), i2 = sqi_integer(o2);
		switch(op) {
			case BW_AND:	res = i1 & i2; break;
			case BW_OR:		res = i1 | i2; break;
			case BW_XOR:	res = i1 ^ i2; break;
			case BW_SHIFTL:	res = i1 << i2; break;
			case BW_SHIFTR:	res = i1 >> i2; break;
			case BW_USHIFTR:res = (SQInteger)(*((SQUnsignedInteger*)&i1) >> i2); break;
			default: { Raise_Error(_SC("internal vm error bitwise op failed")); return false; }
		}
	} 
	else { Raise_Error(_SC("bitwise op between '%s' and '%s'"),GetTypeName(o1),GetTypeName(o2)); return false;}
	trg = res;
	return true;
}

#define _ARITH_(op,trg,o1,o2) \
{ \
	SQInteger tmask = sqi_type(o1)|sqi_type(o2); \
	switch(tmask) { \
		case OT_INTEGER: trg = sqi_integer(o1) op sqi_integer(o2);break; \
		case (OT_FLOAT|OT_INTEGER): \
		case (OT_FLOAT): trg = sqi_tofloat(o1) op sqi_tofloat(o2); break;\
		default: _GUARD(ARITH_OP((#op)[0],trg,o1,o2)); break;\
	} \
}

#define _INTDIV(trg,o1,o2) \
{ \
	SQInteger tmask = sqi_type(o1) | sqi_type(o2); \
	switch (tmask) { \
		case OT_INTEGER: \
		case (OT_FLOAT|OT_INTEGER): \
		case OT_FLOAT: trg = sqi_tointeger(o1) / sqi_tointeger(o2); break; \
		default: Raise_Error(_SC("int div with non-numeric")); SQ_THROW(); break; \
	} \
}

#define _INTMOD(trg,o1,o2) \
{ \
	SQInteger tmask = sqi_type(o1) | sqi_type(o2); \
	switch (tmask) { \
		case OT_INTEGER: \
		case (OT_FLOAT|OT_INTEGER): \
		case OT_FLOAT: trg = sqi_tointeger(o1) % sqi_tointeger(o2); break; \
		default: Raise_Error(_SC("int mod with non-numeric")); SQ_THROW(); break; \
	} \
}

bool SQVM::ARITH_OP(SQUnsignedInteger op,SQObjectPtr &trg,const SQObjectPtr &o1,const SQObjectPtr &o2)
{
	SQInteger tmask = sqi_type(o1) | sqi_type(o2);

	switch(tmask) 
	{
	case OT_INTEGER:
		switch (op) 
		{
		case '+': trg = sqi_integer(o1) + sqi_integer(o2); return true;
		case '-': trg = sqi_integer(o1) - sqi_integer(o2); return true;
		case '*': trg = sqi_integer(o1) * sqi_integer(o2); return true;
		case '/': case '%': tmask = OT_FLOAT; break;
		}
		break;
	}

	switch(tmask)
	{
	case (OT_FLOAT|OT_INTEGER):
	case (OT_FLOAT):
		switch (op)
		{
		case '+': trg = sqi_tofloat(o1) + sqi_tofloat(o2); return true;
		case '-': trg = sqi_tofloat(o1) - sqi_tofloat(o2); return true;
		case '*': trg = sqi_tofloat(o1) * sqi_tofloat(o2); return true;
		case '/': trg = sqi_tofloat(o1) / sqi_tofloat(o2); return true;
		case '%': trg = fmodf(sqi_tofloat(o1), sqi_tofloat(o2)); return true;
		default: Raise_Error(_SC("invalid arith op %c"), op); return false;
		}
	}

	// non-numeric arith
	if (op == '+' && (tmask & _RT_STRING))
	{
		if (!StringCat(o1, o2, trg)) return false;
	}
	else if (!ArithMetaMethod(op, o1, o2, trg))
	{
		Raise_Error(_SC("arith op %c failed between '%s' and '%s'"),op,GetTypeName(o1),GetTypeName(o2)); 
		return false; 
	}

	return true;
}

SQVM::SQVM(SQSharedState *ss) : _oplimit(ss->_oplimit)
{
	_sharedstate=ss;
	_suspended = SQFalse;
	_suspended_target = -1;
	_suspended_root = SQFalse;
	_suspended_traps = -1;
	_foreignptr = NULL;
	_nnativecalls = 0;
	_nmetamethodscall = 0;
	_lasterror = _null_;
	_errorhandler = _null_;
	_threadname = _null_;
	_threadlocal = _null_;
	_requirehandler=NULL;
	_debughook = false;
	_debughook_native = NULL;
	_debughook_native_up = NULL;
	_debughook_closure = _null_;
	_openouters = NULL;
	ci = NULL;
	INIT_CHAIN();ADD_TO_CHAIN(&_ss(this)->_gc_chain,this);
}

void SQVM::Finalize()
{
	if(_openouters) CloseOuters(&_stack._vals[0]);
	_lasterror = _null_;
	_errorhandler = _null_;
	_threadname = _null_;
	_threadlocal = _null_;
	_requirehandler=NULL;
	_debughook = false;
	_debughook_native = NULL;
	_debughook_closure = _null_;
	temp_reg = _null_;
	_callstackdata.resize(0);
	SQInteger size=_stack.size();
	for(SQInteger i=0;i<size;i++)
		_stack[i]=_null_;
}

SQVM::~SQVM()
{
	Finalize();
	REMOVE_FROM_CHAIN(&_ss(this)->_gc_chain,this);
}

bool SQVM::ArithMetaMethod(SQInteger op,const SQObjectPtr &o1,const SQObjectPtr &o2,SQObjectPtr &dest)
{
	SQMetaMethod mm;
	switch(op){
		case _SC('+'): mm=MT_ADD; break;
		case _SC('-'): mm=MT_SUB; break;
		case _SC('/'): mm=MT_DIV; break;
		case _SC('*'): mm=MT_MUL; break;
		case _SC('%'): mm=MT_MODULO; break;
		default: mm = MT_ADD; assert(0); break; //shutup compiler
	}
	if(sqi_isdelegable(o1) && sqi_delegable(o1)->_delegate) {
		Push(o1);Push(o2);
		return CallMetaMethod(sqi_delegable(o1),mm,2,dest) == META_SUCCESS;
	}
	return false;
}

bool SQVM::NEG_OP(SQObjectPtr &trg,const SQObjectPtr &o)
{
	
	switch(sqi_type(o)) {
	case OT_INTEGER:
		trg = -sqi_integer(o);
		return true;
	case OT_FLOAT:
		trg = -sqi_float(o);
		return true;
	case OT_TABLE:
	case OT_USERDATA:
	case OT_INSTANCE:
		if(sqi_delegable(o)->_delegate) {
			Push(o);
			if(CallMetaMethod(sqi_delegable(o), MT_UNM, 1, temp_reg) == META_SUCCESS) {
				trg = temp_reg;
				return true;
			}
		}
	default:break; //shutup compiler
	}
	Raise_Error(_SC("attempt to negate a %s"), GetTypeName(o));
	return false;
}

#define _RET_SUCCEED(exp) { result = (exp); return true; } 
bool SQVM::ObjCmp(const SQObjectPtr &o1,const SQObjectPtr &o2,SQInteger &result)
{
	SQObjectType t1 = sqi_type(o1), t2 = sqi_type(o2);
	if(t1 == t2){
		if(sqi_userpointer(o1)==sqi_userpointer(o2))_RET_SUCCEED(0);
		SQObjectPtr res;
		switch(t1){
		case OT_STRING:
			_RET_SUCCEED(scstrcmp(sqi_stringval(o1),sqi_stringval(o2)));
		case OT_INTEGER:
			_RET_SUCCEED(sqi_integer(o1)-sqi_integer(o2));
		case OT_FLOAT:
			_RET_SUCCEED((sqi_float(o1)<sqi_float(o2))?-1:1);
		case OT_TABLE:
		case OT_USERDATA:
		case OT_INSTANCE:
			if(sqi_delegable(o1)->_delegate) {
				Push(o1);Push(o2);
				if(CallMetaMethod(sqi_delegable(o1),MT_CMP,2,res) == META_SUCCESS) break;
			}
			//continues through (no break needed)
		default: 
			_RET_SUCCEED( sqi_userpointer(o1) < sqi_userpointer(o2)?-1:1 );
		}
		if(sqi_type(res)!=OT_INTEGER) { Raise_CompareError(o1,o2); return false; }
			_RET_SUCCEED(sqi_integer(res));
		
	}
	else{
		if(sq_isnumeric(o1) && sq_isnumeric(o2)){
			if((t1==OT_INTEGER) && (t2==OT_FLOAT)) { 
				if( sqi_integer(o1)==sqi_float(o2) ) { _RET_SUCCEED(0); }
				else if( sqi_integer(o1)<sqi_float(o2) ) { _RET_SUCCEED(-1); }
				_RET_SUCCEED(1);
			}
			else{
				if( sqi_float(o1)==sqi_integer(o2) ) { _RET_SUCCEED(0); }
				else if( sqi_float(o1)<sqi_integer(o2) ) { _RET_SUCCEED(-1); }
				_RET_SUCCEED(1);
			}
		}
		else if(t1==OT_NULL) {_RET_SUCCEED(-1);}
		else if(t2==OT_NULL) {_RET_SUCCEED(1);}
		else { Raise_CompareError(o1,o2); return false; }
		
	}
	assert(0);
	_RET_SUCCEED(0); //cannot happen
}

bool SQVM::CMP_OP(CmpOP op, const SQObjectPtr &o1,const SQObjectPtr &o2,SQObjectPtr &res)
{
	SQInteger r;
	if(ObjCmp(o1,o2,r)) {
		switch(op) {
			case CMP_G: res = (r > 0); return true;
			case CMP_GE: res = (r >= 0); return true;
			case CMP_L: res = (r < 0); return true;
			case CMP_LE: res = (r <= 0); return true;
			case CMP_3W: res = r; return true;
		}
		assert(0);
	}
	return false;
}

SQInteger SQVM::ToRawID(const SQObjectPtr &o)
{
	switch (sqi_type(o))
	{
	case OT_USERDATA:
		return (SQInteger)sqi_userdata(o)->_val;

	case OT_STRING:
		return (SQInteger)sqi_stringval(o);

	case OT_CLASS:
		if (sqi_class(o)->_typetag) return (SQInteger)sqi_class(o)->_typetag;
		else return (SQInteger)sqi_rawval(o);

	case OT_INSTANCE:
		if (sqi_instance(o)->IsPurged()) return (SQInteger)sqi_rawval(o);
		if (sqi_instance(o)->_userpointer) return (SQInteger)sqi_instance(o)->_userpointer;
		return (SQInteger)sqi_rawval(o);

	default:
		return (SQInteger)sqi_rawval(o);
	}
}

void SQVM::ToString(const SQObjectPtr &o,SQObjectPtr &res)
{
	switch(sqi_type(o)) {
	case OT_NULL:
		res = SQString::Create(_sharedstate, "(null)", 6);
		return;
	case OT_STRING:
		res = o;
		return;
	case OT_FLOAT:
		scsprintf(_sp(rsl(NUMBER_MAX_CHAR+1)),_SC("%g"),sqi_float(o));
		break;
	case OT_INTEGER:
		scsprintf(_sp(rsl(NUMBER_MAX_CHAR+1)),_SC("%d"),sqi_integer(o));
		break;
	case OT_BOOL:
		scsprintf(_sp(rsl(6)),sqi_integer(o)?_SC("true"):_SC("false"));
		break;
	case OT_CLASS:
		{
			void* tag = sqi_class(o)->_typetag;
			SQChar* cname = sqi_stringval(sqi_class(o)->_methods[0].val);
			if (tag)
				scsprintf(_sp(rsl(scstrlen(cname) + NUMBER_MAX_CHAR)),_SC("(class %s -> 0x%p)"),cname,tag);
			else
				scsprintf(_sp(rsl(scstrlen(cname) + NUMBER_MAX_CHAR)),_SC("(class %s : 0x%p)"),cname,(void*)sqi_rawval(o));
		}
		break;
	case OT_THREAD:
		{
			SQVM* th = sqi_thread(o);
			if (sqi_type(th->_threadname) == OT_STRING)
			{
				SQChar* name = sqi_stringval(th->_threadname);
				SQStackInfos si;
				if (SQ_SUCCEEDED(sq_stackinfos(th, 0, &si)))
				{
					if (si.line >= 0)
					{
						scsprintf(_sp(1000), _SC("(thread '%s' : 0x%p at %s() from '%s' line %d)"), name, th, si.funcname, si.source, si.line);
					}
					else
					{
						scsprintf(_sp(1000), _SC("(thread '%s' : 0x%p at %s() from '%s')"), name, th, si.funcname, si.source);
					}
				}
				else
				{
					scsprintf(_sp(1000), _SC("(thread '%s' : 0x%p)"), name, th);
				}
			}
			else
			{
				SQStackInfos si;
				if (SQ_SUCCEEDED(sq_stackinfos(th, 0, &si)))
				{
					if (si.line >= 0)
					{
						scsprintf(_sp(1000), _SC("(thread : 0x%p at %s() from '%s' line %d)"), th, si.funcname, si.source, si.line);
					}
					else
					{
						scsprintf(_sp(1000), _SC("(thread : 0x%p at %s() from '%s')"), th, si.funcname, si.source);
					}
				}
				else
				{
					scsprintf(_sp(1000), _SC("(thread : 0x%p)"), th);
				}
			}
		}
		break;
	case OT_TABLE:
	case OT_USERDATA:
	case OT_INSTANCE:
		if(sqi_delegable(o)->_delegate) {
			Push(o);
			if(CallMetaMethod(sqi_delegable(o),MT_TOSTRING,1,res) == META_SUCCESS) {
				if(sqi_type(res) == OT_STRING)
					return;
				//else keeps going to the default
			}
		}
	default:
		if (sqi_type(o) == OT_INSTANCE)
			if (sqi_instance(o)->IsPurged())
				scsprintf(_sp(rsl(NUMBER_MAX_CHAR)),_SC("(purged : 0x%p)"),(void*)sqi_rawval(o));
			else
			{
				void* ptr = sqi_instance(o)->_userpointer; 
				SQChar* cname = sqi_stringval(sqi_instance(o)->_class->_methods[0].val);
				if (ptr)
					scsprintf(_sp(rsl(scstrlen(cname)+NUMBER_MAX_CHAR)),_SC("(%s -> 0x%p)"),cname,ptr);
				else
					scsprintf(_sp(rsl(scstrlen(cname)+NUMBER_MAX_CHAR)),_SC("(%s : 0x%p)"),cname,(void*)sqi_rawval(o));
			}
		else
		{
			const SQChar* tname = GetTypeName(o);
			scsprintf(_sp(rsl(scstrlen(tname)+NUMBER_MAX_CHAR)),_SC("(%s : 0x%p)"),tname,(void*)sqi_rawval(o));
		}
	}
	res = SQString::Create(_ss(this),_spval);
}


bool SQVM::StringCat(const SQObjectPtr &str,const SQObjectPtr &obj,SQObjectPtr &dest)
{
	SQObjectPtr a, b;
	ToString(str, a);
	ToString(obj, b);
	SQInteger l = sqi_string(a)->_len , ol = sqi_string(b)->_len;
	SQChar *s = _sp(rsl(l + ol + 1));
	memcpy(s, sqi_stringval(a), rsl(l)); 
	memcpy(s + l, sqi_stringval(b), rsl(ol));
	dest = SQString::Create(_ss(this), _spval, l + ol);
	return true;
}

void SQVM::TypeOf(const SQObjectPtr &obj1,SQObjectPtr &dest)
{
	if(sqi_isdelegable(obj1) && sqi_delegable(obj1)->_delegate) {
		Push(obj1);
		if(CallMetaMethod(sqi_delegable(obj1),MT_TYPEOF,1,dest) == META_SUCCESS)
			return;
	}
	dest = SQString::Create(_ss(this),GetTypeName(obj1));
}

bool SQVM::Init(SQVM *friendvm, SQInteger stacksize)
{
	_stack.resize(stacksize);
	_alloccallsstacksize = 4;
	_callstackdata.resize(_alloccallsstacksize);
	_callsstacksize = 0;
	_callsstack = &_callstackdata[0];
	_threadlocal = SQTable::Create(_ss(this), 0);
	_stackbase = 0;
	_top = 0;
	if(!friendvm) 
	{
		sq_base_register(this);
	}
	else {
		_requirehandler = friendvm->_requirehandler;
		_errorhandler = friendvm->_errorhandler;
		_debughook = friendvm->_debughook;
		_debughook_native = friendvm->_debughook_native;
		_debughook_native_up = friendvm->_debughook_native_up;
		_debughook_closure = friendvm->_debughook_closure;
		_foreignptr = friendvm->_foreignptr;
	}
	
	return true;
}


bool SQVM::StartCall(SQClosure *closure,SQInteger target,SQInteger args,SQInteger stackbase,bool tailcall)
{
	SQFunctionProto *func = closure->_function;

	SQInteger paramssize = func->_nparameters;
	const SQInteger newtop = stackbase + func->_stacksize;
	SQInteger nargs = args;
	if(func->_varparams)
	{
		paramssize--;
		if (nargs < paramssize) {
			Raise_Error(_SC("wrong number of parameters"));
			return false;
		}

		//dumpstack(stackbase);
		SQInteger nvargs = nargs - paramssize;
		SQArray *arr = SQArray::Create(_ss(this),nvargs);
		SQInteger pbase = stackbase+paramssize;
		for(SQInteger n = 0; n < nvargs; n++) {
			arr->_values[n] = _stack._vals[pbase];
			_stack._vals[pbase] = _null_;
			pbase++;

		}
		_stack._vals[stackbase+paramssize] = arr;
		//dumpstack(stackbase);
	}
	else if (paramssize != nargs) {
		SQInteger ndef = func->_ndefaultparams;
		SQInteger diff;
		if(ndef && nargs < paramssize && (diff = paramssize - nargs) <= ndef) {
			for(SQInteger n = ndef - diff; n < ndef; n++) {
				_stack._vals[stackbase + (nargs++)] = closure->_defaultparams[n];
			}
		}
		else {
			Raise_Error(_SC("wrong number of parameters"));
			return false;
		}
	}

	if (sqi_type(closure->_env) != OT_NULL) {
		_getrealval(closure->_env, this, _stack._vals[stackbase]);
	}

	if(!EnterFrame(stackbase, newtop, tailcall)) return false;

	ci->_closure  = closure;
	ci->_literals = func->_literals;
	ci->_ip       = func->_instructions;
	ci->_imports  = func->_imports;
	ci->_target   = (SQInt32)target;

	if (_debughook) {
		CallDebugHook(_SC('c'));
	}

	if (closure->_function->_bgenerator) {
		SQFunctionProto *f = closure->_function;
		SQGenerator *gen = SQGenerator::Create(_ss(this), closure);
		if(!gen->Yield(this,f->_stacksize))
			return false;
		SQObjectPtr temp;
		Return(1, target, temp);
		STK(target) = gen;
	}


	return true;
}

bool SQVM::Return(SQInteger _arg0, SQInteger _arg1, SQObjectPtr &retval)
{
	SQBool    _isroot      = ci->_root;
	SQInteger callerbase   = _stackbase - ci->_prevstkbase;

	if (_debughook) {
		for(SQInteger i=0; i<ci->_ncalls; i++) {
			CallDebugHook(_SC('r'));
		}
	}

	SQObjectPtr *dest;
	if (_isroot) {
		dest = &(retval);
	} else if (ci->_target == -1) {
		dest = NULL;
	} else {
		dest = &_stack._vals[callerbase + ci->_target];
	}
	if (dest) {
		if (_arg0 == 0xFE)
		{
			*dest = temp_ret;
			temp_ret.Null();
		}
		else if (_arg0 == 0xFF)
			dest->Null();
		else
			*dest = _stack._vals[_stackbase+_arg1];
	}
	LeaveFrame();

	if (_debughook && ci == NULL)
		CallDebugHook(_SC('q'));

	return _isroot != 0;
}

#define _RET_ON_FAIL(exp) { if(!exp) return false; }

bool SQVM::PLOCAL_INC(SQInteger op,SQObjectPtr &target, SQObjectPtr &a, SQObjectPtr &incr)
{
 	SQObjectPtr trg;
	_RET_ON_FAIL(ARITH_OP( op , trg, a, incr));
	target = a;
	a = trg;
	return true;
}

bool SQVM::DerefInc(SQInteger op,SQObjectPtr &target, SQObjectPtr &self, SQObjectPtr &key, SQObjectPtr &incr, bool postfix,SQInteger selfidx)
{
	SQObjectPtr tmp, tself = self, tkey = key;
	if (!Get(tself, tkey, tmp, false, selfidx)) { return false; }
	_RET_ON_FAIL(ARITH_OP( op , target, tmp, incr))
	if (!Set(tself, tkey, target,false,selfidx)) { return false; }
	if (postfix) target = tmp;
	return true;
}

#define arg0 (_i_._arg0)
#define arg1 (_i_._arg1)
#define sarg1 (*((SQInt32 *)&_i_._arg1))
#define arg2 (_i_._arg2)
#define arg3 (_i_._arg3)
#define sarg3 ((SQInteger)*((signed char *)&_i_._arg3))

SQRESULT SQVM::Suspend()
{
	if (_suspended)
		return sq_throwerror(this, _SC("cannot suspend an already suspended vm"));
	if (_nnativecalls!=2)
		return sq_throwerror(this, _SC("cannot suspend through native calls/metamethods"));
	return SQ_SUSPEND_FLAG;
}


#define _FINISH(howmuchtojump) {jump = howmuchtojump; return true; }
bool SQVM::FOREACH_OP(SQObjectPtr &o1,SQObjectPtr &o2,SQObjectPtr 
&o3,SQObjectPtr &o4,SQInteger arg_2,int exitpos,int &jump)
{
	SQInteger nrefidx;
	switch(sqi_type(o1)) {
	case OT_TABLE:
		if((nrefidx = sqi_table(o1)->Next(false,o4, o2, o3)) == -1) _FINISH(exitpos);
		o4 = (SQInteger)nrefidx; _FINISH(1);
	case OT_ARRAY:
		if((nrefidx = sqi_array(o1)->Next(o4, o2, o3)) == -1) _FINISH(exitpos);
		o4 = (SQInteger) nrefidx; _FINISH(1);
	case OT_STRING:
		if((nrefidx = sqi_string(o1)->Next(o4, o2, o3)) == -1)_FINISH(exitpos);
		o4 = (SQInteger)nrefidx; _FINISH(1);
	case OT_CLASS:
		if((nrefidx = sqi_class(o1)->Next(o4, o2, o3)) == -1)_FINISH(exitpos);
		o4 = (SQInteger)nrefidx; _FINISH(1);
	case OT_USERDATA:
	case OT_INSTANCE:
		if(sqi_delegable(o1)->_delegate) {
			SQObjectPtr itr;
			Push(o1);
			Push(o4);
			if(CallMetaMethod(sqi_delegable(o1), MT_NEXTI, 2, itr) == META_SUCCESS){
				if (sqi_type(itr) == OT_GENERATOR || sqi_type(itr) == OT_NATIVE_ITR)
				{
					// when _nexti returns a generator or an iterator
					o1 = itr;
					// execute FOREACH_OP as if a generator given
					return FOREACH_OP(o1, o2, o3, o4, arg_2, exitpos, jump);
					// TODO: following code fails assertion:
					/*	function foo() { for (var i=0; i<10; ++i) yield i; }
  						function _nexti(idx) { return foo() } */
				}
				o4 = o2 = itr;
				if(sqi_type(itr) == OT_NULL) _FINISH(exitpos);
				if(Get(o1, itr, o3, false, DONT_FALL_BACK))
				{
					// TODO: "_nexti" is not so neat, make a new "_begin", "_next" iteration meta-method
					_FINISH(1);
				}
				else
				{
					_FINISH(exitpos);
				}
			}
			Raise_Error(_SC("_nexti failed"));
			return false;
		}
		break;
	case OT_GENERATOR:
		if(sqi_generator(o1)->_state == SQGenerator::eDead) _FINISH(exitpos);
		if(sqi_generator(o1)->_state == SQGenerator::eSuspended) {
			SQInteger idx = 0;
			if(sqi_type(o4) == OT_INTEGER) {
				idx = sqi_integer(o4) + 1;
			}
			o2 = idx;
			o4 = idx;
			sqi_generator(o1)->Resume(this, o3);
			_FINISH(0);
		}
		break;
	case OT_NATIVE_ITR:
		{
			SQNativeItr* itr = sqi_nativeitr(o1);
			SQInteger nret = itr->_itrfunc(this, itr->_val);
			if (SQ_FAILED(nret))
				return false;
			if (nret == 0)
				_FINISH(exitpos);
			if (nret == 1)
			{
				o3 = Top();
				Pop(1);
				SQInteger idx = 0;
				if(sqi_type(o2) == OT_INTEGER) {
					idx = sqi_integer(o2) + 1;
				}
				o2 = idx;
				_FINISH(1);
			}
			else if (nret == 2)
			{
				o3 = Top();
				Pop(1);
				o2 = Top();
				Pop(1);
				_FINISH(1);
			}
			else
			{
				Raise_Error(_SC("too many ret from native iterator"));
				return false;
			}
		}
		break;
	default: 
		Raise_Error(_SC("cannot iterate %s"), GetTypeName(o1));
	}
	return false; //cannot be hit(just to avoid warnings)
}

#define COND_LITERAL (arg3!=0?ci->_literals[arg1]:STK(arg1))

#define _GUARD(exp) { if(!exp) { Raise_Error(_lasterror); SQ_THROW();} }

#define SQ_THROW() { goto exception_trap; }

bool SQVM::CLOSURE_OP(SQObjectPtr &target, SQFunctionProto *func)
{
	SQInteger nouters;
	SQClosure *closure = SQClosure::Create(_ss(this), func);
	if((nouters = func->_noutervalues)) {
		for(SQInteger i = 0; i<nouters; i++) {
			SQOuterVar &v = func->_outervalues[i];
			switch(v._type){
			case otLOCAL:
				FindOuter(closure->_outervalues[i], &STK(sqi_integer(v._src)));
				break;
			case otOUTER:
				closure->_outervalues[i] = sqi_closure(ci->_closure)->_outervalues[sqi_integer(v._src)];
				break;
			}
		}
	}
	SQInteger ndefparams;
	if((ndefparams = func->_ndefaultparams)) {
		for(SQInteger i = 0; i < ndefparams; i++) {
			SQInteger spos = func->_defaultparams[i];
			closure->_defaultparams[i] = _stack._vals[_stackbase + spos];
		}
	}
	target = closure;
	return true;

}


bool SQVM::CLASS_OP(SQObjectPtr &target,SQInteger baseclass,SQInteger attributes)
{
	SQClass *base = NULL;
	SQObjectPtr attrs;
	if(baseclass != -1) {
		if(sqi_type(_stack._vals[_stackbase+baseclass]) != OT_CLASS) { Raise_Error(_SC("trying to inherit from a %s"),GetTypeName(_stack._vals[_stackbase+baseclass])); return false; }
		base = sqi_class(_stack._vals[_stackbase + baseclass]);
	}
	if(attributes != MAX_FUNC_STACKSIZE) {
		attrs = _stack._vals[_stackbase+attributes];
	}
	target = SQClass::Create(_ss(this),base);
	if(sqi_type(sqi_class(target)->_metamethods[MT_INHERITED]) != OT_NULL) {
		int nparams = 2;
		SQObjectPtr ret;
		Push(target); Push(attrs);
		bool ok = Call(sqi_class(target)->_metamethods[MT_INHERITED],nparams,_top - nparams, ret, false);
		Pop(nparams);
		if (!ok)
		{
			target = _null_;
			return false;
		}
	}
	sqi_class(target)->_attributes = attrs;
	return true;
}

bool SQVM::IsEqual(const SQObjectPtr &o1,const SQObjectPtr &o2,bool &res)
{
	if(sqi_type(o1) == sqi_type(o2)) {
		res = (sqi_userpointer(o1) == sqi_userpointer(o2));
		// TODO: add '_eq' metamethod
		// TODO: true if purged instance 'IsEqual' to null
	}
	else {
		if(sq_isnumeric(o1) && sq_isnumeric(o2)) {
			res = (sqi_tofloat(o1) == sqi_tofloat(o2));
		}
		else {
			res = false;
		}
	}
	return true;
}

bool SQVM::IsFalse(SQObjectPtr &o)
{
	if(((sqi_type(o) & SQOBJECT_CANBEFALSE) 
		&& ( (sqi_type(o) == OT_FLOAT) && (sqi_float(o) == SQFloat(0.0)) ))
		|| ( sqi_type(o) == OT_INSTANCE && sqi_instance(o)->IsPurged() )
#if !defined(SQUSEDOUBLE) || defined(SQUSEDOUBLE) && defined(_SQ64)
		|| (sqi_integer(o) == 0) )  //OT_NULL|OT_INTEGER|OT_BOOL
#else
		|| ((sqi_type(o) != OT_FLOAT) && sqi_integer(o) == 0) )  //OT_NULL|OT_INTEGER|OT_BOOL
#endif
	{
		return true;
	}
	return false;
}

bool SQVM::Execute(SQObjectPtr &closure, SQInteger nargs, SQInteger stackbase,SQObjectPtr &outres, SQBool raiseerror,ExecutionType et)
{
	if ((_nnativecalls + 1) > MAX_NATIVE_CALLS) 
	{ 
		_sharedstate->_errorfunc(this, "*** [FATAL] too many native calls");
		Raise_Error(_SC("Native stack overflow")); return false; 
	}
	_nnativecalls++;
	AutoDec ad(&_nnativecalls);
	SQInteger traps = 0;
	CallInfo *prevci = ci;

	
	GC_MUTATED(this);
		
	switch(et) {
		case ET_CALL: {
			temp_reg = closure;
			if(!StartCall(sqi_closure(temp_reg), _top - nargs, nargs, stackbase, false)) { 
				//call the handler if there are no calls in the stack, if not relies on the previous node
				if(ci == NULL) CallErrorHandler(_lasterror);
				return false;
			}
			if(ci == prevci) {
				outres = STK(_top-nargs);
				return true;
			}
			ci->_root = SQTrue;
					  }
			break;
		case ET_RESUME_GENERATOR: sqi_generator(closure)->Resume(this, outres); ci->_root = SQTrue; traps += ci->_etraps; break;
		case ET_RESUME_VM:
		case ET_RESUME_THROW_VM:
			traps = _suspended_traps;
			ci->_root = _suspended_root;
			_suspended = SQFalse;
			if(et  == ET_RESUME_THROW_VM) { SQ_THROW(); }
			break;
	}
	
exception_restore:
	//
	{
		for(;;)
		{
			const SQInstruction &_i_ = *ci->_ip++;
			if (_oplimit && --_oplimit == 0) { Raise_Error("too many instructions"); SQ_THROW(); return false; }
			//dumpstack(_stackbase);
			//scprintf("\n[%d] %s %d %d %d %d\n",ci->_ip-ci->_iv->_vals,g_InstrDesc[_i_.op].name,arg0,arg1,arg2,arg3);
			switch(_i_.op)
			{
			case _OP_LINE: if (_debughook) CallDebugHook(_SC('l'),arg1); continue;
			case _OP_LOAD: TARGET = ci->_literals[arg1]; continue;
			case _OP_LOADINT: 
#ifndef _SQ64
				TARGET = (SQInteger)arg1; continue;
#else
				TARGET = (SQInteger)((SQUnsignedInteger32)arg1); continue;
#endif
			case _OP_LOADFLOAT: TARGET = *((SQFloat *)&arg1); continue;
			case _OP_DLOAD: TARGET = ci->_literals[arg1]; STK(arg2) = ci->_literals[arg3];continue;
			case _OP_TAILCALL:
				if (sqi_type(STK(arg1)) == OT_CLOSURE){
					SQObjectPtr clo = STK(arg1);
					if(_openouters) CloseOuters(&(_stack._vals[_stackbase]));
					for (SQInteger i = 0; i < arg3; i++) STK(i) = STK(arg2 + i);
					_GUARD(StartCall(sqi_closure(clo), ci->_target, arg3, _stackbase, true));
					continue;
				}
			case _OP_CALL: {
					SQObjectPtr clo = STK(arg1);
					switch (sqi_type(clo)) {
					case OT_CLOSURE:
						_GUARD(StartCall(sqi_closure(clo), arg0, arg3, _stackbase+arg2, false));
						continue;
					case OT_NATIVECLOSURE: {
						bool suspend;
						_GUARD(CallNative(sqi_nativeclosure(clo), arg3, _stackbase+arg2, clo,suspend));
						if(suspend){
							_suspended = SQTrue;
							_suspended_target = arg0;
							_suspended_root = ci->_root;
							_suspended_traps = traps;
							outres = clo;
							return true;
						}
						STK(arg0) = clo;
						}
						continue;
					case OT_CLASS:
						_GUARD(Call(clo, arg3, _stackbase+arg2, clo, raiseerror));
						STK(arg0) = clo;
						continue;
					case OT_TABLE:
					case OT_USERDATA:
					case OT_INSTANCE:{
						Push(clo);
						for (SQInteger i = 0; i < arg3; i++) Push(STK(arg2 + i));
						if (sqi_delegable(clo))
						{
							MetaMethodResult mmr = CallMetaMethod(sqi_delegable(clo), MT_CALL, arg3+1, clo);
							if (mmr == META_SUCCESS){
								STK(arg0) = clo;
								break;
							}
							else if (mmr == META_ERROR && sqi_type(_lasterror) != OT_NULL)
								SQ_THROW();
						}
						// not delegable or meta not found or clean failure
						Raise_Error(_SC("attempt to call '%s'"), GetTypeName(clo));
						SQ_THROW();
					  }
					default:
						Raise_Error(_SC("attempt to call '%s'"), GetTypeName(clo));
						SQ_THROW();
					}
				}
				  continue;
			case _OP_PREPCALL:
			case _OP_PREPCALLK:	{
					SQObjectPtr &key = _i_.op == _OP_PREPCALLK?(ci->_literals)[arg1]:STK(arg1);
					SQObjectPtr &o = STK(arg2);
					if (!Get(o, key, temp_reg,false,arg2)) {
						SQ_THROW();
					}
					STK(arg3) = o;
					TARGET = temp_reg;
				}
				continue;
			case _OP_GETK:
				if (!Get(STK(arg2), ci->_literals[arg1], temp_reg, false,arg2)) { SQ_THROW();}
				TARGET = temp_reg;
				continue;
			case _OP_MOVE: TARGET = STK(arg1); continue;
			case _OP_SWAP: temp_reg = STK(arg0); STK(arg0) = STK(arg1); STK(arg1) = temp_reg; continue;
			case _OP_DSWAP: 
				temp_reg = STK(arg0); STK(arg0) = STK(arg1); STK(arg1) = temp_reg;
				temp_reg = STK(arg3); STK(arg2) = STK(arg3); STK(arg3) = temp_reg; 
				continue;
			case _OP_NEWSLOT:
				_GUARD(NewSlot(STK(arg1), STK(arg2), STK(arg3),false));
				if(arg0 != arg3) TARGET = STK(arg3);
				continue;
			case _OP_SET:
				if (!Set(STK(arg1), STK(arg2), STK(arg3),false,arg1)) { SQ_THROW(); }
				if (arg0 != arg3) TARGET = STK(arg3);
				continue;
			case _OP_GET:
				if (!Get(STK(arg1), STK(arg2), temp_reg, false,arg1)) { SQ_THROW(); }
				TARGET = temp_reg;
				continue;
			case _OP_ASSIGN:
				{
					SQObjectPtr o = TARGET;
					MetaMethodResult mmr = META_NOT_FOUND;
					if (sqi_isdelegable(o) && sqi_delegable(o))
					{
						Push(o);
						Push(STK(arg1));
						mmr = CallMetaMethod(sqi_delegable(o), MT_ASSIGN, 2, o);
						if (mmr == META_SUCCESS)
							TARGET = STK(arg1);
					}
					if (mmr != META_SUCCESS)
					{
						if (mmr == META_ERROR && sqi_type(_lasterror) != OT_NULL)
							SQ_THROW();
						Raise_Error(_SC("invalid assignment to '%s'"), GetTypeName(o));
						SQ_THROW();
					}
				}

			case _OP_EQ:{
				bool res;
				if(!IsEqual(STK(arg2),COND_LITERAL,res)) { SQ_THROW(); }
				TARGET = res?true:false;
				}continue;
			case _OP_NE:{ 
				bool res;
				if(!IsEqual(STK(arg2),COND_LITERAL,res)) { SQ_THROW(); }
				TARGET = (!res)?true:false;
				} continue;
			case _OP_ADD: _ARITH_(+,TARGET,STK(arg2),STK(arg1)); continue;
			case _OP_SUB: _ARITH_(-,TARGET,STK(arg2),STK(arg1)); continue;
			case _OP_MUL: _ARITH_(*,TARGET,STK(arg2),STK(arg1)); continue;
			case _OP_INTDIV: _INTDIV(TARGET, STK(arg2), STK(arg1)); continue;
			case _OP_INTMOD: _INTMOD(TARGET, STK(arg2), STK(arg1)); continue;
			case _OP_DIV: ARITH_OP('/',TARGET,STK(arg2),STK(arg1)); continue;
			case _OP_MOD: ARITH_OP('%',TARGET,STK(arg2),STK(arg1)); continue;
			case _OP_BITW:	_GUARD(BW_OP( arg3,TARGET,STK(arg2),STK(arg1))); continue;
			case _OP_RETURN:
				if((ci)->_generator) {
					(ci)->_generator->Kill();
				}
				if(Return(arg0, arg1, temp_reg))
				{
					assert(traps==0);
					outres = temp_reg;
					return true;
				}
				continue;
			case _OP_LOADNULLS:{ for(SQInt32 n=0; n < arg1; n++) STK(arg0+n).Null(); }continue;
			case _OP_LOADROOT:	TARGET = _ss(this)->_root_table; continue;
			case _OP_LOADBOOL: TARGET = arg1?true:false; continue;
			case _OP_DMOVE: STK(arg0) = STK(arg1); STK(arg2) = STK(arg3); continue;
			case _OP_JMP: ci->_ip += (sarg1); continue;
			case _OP_JNZ: if(!IsFalse(STK(arg0))) ci->_ip+=(sarg1); continue;
			case _OP_JZ: if(IsFalse(STK(arg0))) ci->_ip+=(sarg1); continue;
			case _OP_GETOUTER: {
				SQClosure *cur_cls = sqi_closure(ci->_closure);
				SQOuter *otr = sqi_outer(cur_cls->_outervalues[arg1]);
				TARGET = *(otr->_valptr);
				}
			continue;
			case _OP_SETOUTER: {
				SQClosure *cur_cls = sqi_closure(ci->_closure);
				SQOuter   *otr = sqi_outer(cur_cls->_outervalues[arg1]);
				*(otr->_valptr) = STK(arg2);
				if(arg0 != arg2) {
					TARGET = STK(arg2);
				}
				}
			continue;
			case _OP_NEWOBJ: 
				switch(arg3) {
					case NOT_TABLE: TARGET = SQTable::Create(_ss(this), arg1); continue;
					case NOT_ARRAY: TARGET = SQArray::Create(_ss(this), 0); sqi_array(TARGET)->Reserve(arg1); continue;
					case NOT_CLASS: _GUARD(CLASS_OP(TARGET,arg1,arg2)); continue;
					default: assert(0); continue;
				}
			case _OP_APPENDARRAY: sqi_array(STK(arg0))->Append(COND_LITERAL);	continue;
			case _OP_COMPARITH: {
				SQInteger selfidx = (((SQUnsignedInteger)arg1&0xFFFF0000)>>16);
				_GUARD(DerefInc(arg3, TARGET, STK(selfidx), STK(arg2), STK(arg1&0x0000FFFF), false, selfidx)); 
								}
				continue;
			case _OP_INC: {SQObjectPtr o(sarg3); _GUARD(DerefInc('+',TARGET, STK(arg1), STK(arg2), o, false, arg1));} continue;
			case _OP_INCL: {
				SQObjectPtr &a = STK(arg1);
				if(sqi_type(a) == OT_INTEGER) {
					a._unVal.nInteger = sqi_integer(a) + sarg3;
				}
				else {
					SQObjectPtr o(sarg3); //_GUARD(LOCAL_INC('+',TARGET, STK(arg1), o));
					_ARITH_(+,a,a,o);
				}
						   } continue;
			case _OP_PINC: {SQObjectPtr o(sarg3); _GUARD(DerefInc('+',TARGET, STK(arg1), STK(arg2), o, true, arg1));} continue;
			case _OP_PINCL:	{
				SQObjectPtr &a = STK(arg1);
				if(sqi_type(a) == OT_INTEGER) {
					TARGET = a;
					a._unVal.nInteger = sqi_integer(a) + sarg3;
				}
				else {
					SQObjectPtr o(sarg3); _GUARD(PLOCAL_INC('+',TARGET, STK(arg1), o));
				}
				
						} continue;
			case _OP_CMP:	_GUARD(CMP_OP((CmpOP)arg3,STK(arg2),STK(arg1),TARGET))	continue;
			case _OP_EXISTS: TARGET = Get(STK(arg1), STK(arg2), temp_reg, false,DONT_FALL_BACK)?true:false;continue;
			case _OP_INSTANCEOF: 
				if(sqi_type(STK(arg1)) != OT_CLASS)
				{Raise_Error(_SC("cannot apply instanceof between a %s and a %s"),GetTypeName(STK(arg1)),GetTypeName(STK(arg2))); SQ_THROW();}
				TARGET = (sqi_type(STK(arg2)) == OT_INSTANCE) ? (sqi_instance(STK(arg2))->InstanceOf(sqi_class(STK(arg1)))?true:false) : false;
				continue;
			case _OP_AND: 
				if(IsFalse(STK(arg2))) {
					TARGET = STK(arg2);
					ci->_ip += (sarg1);
				}
				continue;
			case _OP_OR:
				if(!IsFalse(STK(arg2))) {
					TARGET = STK(arg2);
					ci->_ip += (sarg1);
				}
				continue;
			case _OP_NEG: _GUARD(NEG_OP(TARGET,STK(arg1))); continue;
			case _OP_NOT: TARGET = IsFalse(STK(arg1)); continue;
			case _OP_BWNOT:
				if(sqi_type(STK(arg1)) == OT_INTEGER) {
					SQInteger t = sqi_integer(STK(arg1));
					TARGET = SQInteger(~t);
					continue;
				}
				Raise_Error(_SC("attempt to perform a bitwise op on a %s"), GetTypeName(STK(arg1)));
				SQ_THROW();
			case _OP_CLOSURE: {
				SQClosure *c = ci->_closure._unVal.pClosure;
				SQFunctionProto *fp = c->_function;
				if(!CLOSURE_OP(TARGET,fp->_functions[arg1]._unVal.pFunctionProto)) { SQ_THROW(); }
				continue;
			}
			case _OP_YIELD:{
				if(ci->_generator) {
					if(sarg1 != MAX_FUNC_STACKSIZE) temp_reg = STK(arg1);
					_GUARD(ci->_generator->Yield(this,arg2));
					traps -= ci->_etraps;
					if(sarg1 != MAX_FUNC_STACKSIZE) STK(arg1) = temp_reg;
				}
				else { Raise_Error(_SC("trying to yield a '%s',only genenerator can be yielded"), GetTypeName(ci->_generator)); SQ_THROW();}
				if(Return(arg0, arg1, temp_reg)){
					assert(traps == 0);
					outres = temp_reg;
					return true;
				}
					
				}
				continue;
			case _OP_RESUME:
				if(sqi_type(STK(arg1)) != OT_GENERATOR){ Raise_Error(_SC("trying to resume a '%s',only genenerator can be resumed"), GetTypeName(STK(arg1))); SQ_THROW();}
				_GUARD(sqi_generator(STK(arg1))->Resume(this, TARGET));
				traps += ci->_etraps;
                continue;
			case _OP_FOREACH:{ int tojump;
				_GUARD(FOREACH_OP(STK(arg0),STK(arg2),STK(arg2+1),STK(arg2+2),arg2,sarg1,tojump));
				ci->_ip += tojump; }
				continue;
			case _OP_POSTFOREACH:
				assert(sqi_type(STK(arg0)) == OT_GENERATOR);
				if(sqi_generator(STK(arg0))->_state == SQGenerator::eDead) 
					ci->_ip += (sarg1 - 1);
				continue;
			case _OP_CLONE:
				if(!Clone(STK(arg1), TARGET)) SQ_THROW(); 
				continue;
			case _OP_TYPEOF: TypeOf(STK(arg1), TARGET); continue;
			case _OP_PUSHTRAP:{
				SQInstruction* iv = sqi_closure(ci->_closure)->_function->_instructions;
				SQInstruction *ip = NULL;
				SQInstruction* finpos = NULL;
				if (arg1) ip = &iv[(ci->_ip-iv)+arg1];
				if (arg2) finpos = &iv[(ci->_ip-iv)+arg2];
				_etraps.push_back(SQExceptionTrap(_top,_stackbase, ip, arg0, finpos)); traps++;
				ci->_etraps++;
							  }
				continue;
			case _OP_POPTRAP: 
				if (arg1) temp_ret = STK(arg1);
				if (arg0 == 1)
				{
					--traps; --ci->_etraps;
					SQInstruction* finPos = _etraps.top()._finallyPos;
					if (finPos)
					{
						_traprets.push_back(ci->_ip);
						ci->_ip = finPos;
					}
					_etraps.pop_back();
				}
				else if (arg0)
				{
					SQUnsignedInteger numTraps = _etraps.size();
					traps -= arg0;
					ci->_etraps -= arg0;
					for (SQUnsignedInteger i=numTraps-arg0; i < numTraps; ++i)
					{
						SQInstruction* finPos = _etraps[i]._finallyPos;
						if (finPos)
						{
							_traprets.push_back(ci->_ip);
							ci->_ip = finPos;
						}
					}
					_etraps.resize(numTraps-arg0);
				}
				continue;

			case _OP_RETTRAP:
				{
					if (!_traprets.empty())
					{
						SQInstruction* retpos = _traprets.top();
						_traprets.pop_back();
						if (retpos)
							ci->_ip = retpos;
					}
					else
					{
						bool restore = UnwindTrap(traps, _traperror, _top);
						if (restore) goto exception_restore;
						return false;
					}
				}
				continue;

			case _OP_THROW:	Raise_Error(TARGET); SQ_THROW(); continue;
			case _OP_NEWSLOTA: {
				bool bstatic = (arg0&NEW_SLOT_STATIC_FLAG)?true:false;
				if(sqi_type(STK(arg1)) == OT_CLASS) {
					if(sqi_type(sqi_class(STK(arg1))->_metamethods[MT_NEWMEMBER]) != OT_NULL ) {
						Push(STK(arg1)); Push(STK(arg2)); Push(STK(arg3));
						Push((arg0&NEW_SLOT_ATTRIBUTES_FLAG) ? STK(arg2-1) : _null_);
						int nparams = 4;
						if(Call(sqi_class(STK(arg1))->_metamethods[MT_NEWMEMBER], nparams, _top - nparams, temp_reg,SQFalse)) {
							Pop(nparams);
							continue;
						}
					}
				}
				_GUARD(NewSlot(STK(arg1), STK(arg2), STK(arg3),bstatic));
				if((arg0&NEW_SLOT_ATTRIBUTES_FLAG)) {
					sqi_class(STK(arg1))->SetAttributes(STK(arg2),STK(arg2-1));
				}
							   }
				continue;
			case _OP_NEWPROP: {
				_GUARD(NewProp(STK(arg1), STK(arg2), STK(arg3), STK(arg3+1)));
				if((arg0&NEW_SLOT_ATTRIBUTES_FLAG)) {
					sqi_class(STK(arg1))->SetAttributes(STK(arg2),STK(arg2-1));
				}
							  }
				continue;
			case _OP_GETBASE:{
				SQClosure *clo = sqi_closure(ci->_closure);
				if(clo->_base) {
					TARGET = clo->_base;
				}
				else {
					TARGET.Null();
				}
				continue;
			}
			case _OP_CLOSE:
				if(_openouters) CloseOuters(&(STK(arg1)));
				continue;
			case _OP_REQUIRE:
				if (_requirehandler == NULL)
				{
					Raise_Error(_SC("no require handler")); SQ_THROW();
				}

				Push(STK(arg0));
				_GUARD((_requirehandler(this)!=SQ_ERROR));
				Pop(1);
				continue;

			case _OP_IMPORT:
				{
					SQObjectPtr t = STK(arg0);
					if (sqi_type(t) != OT_TABLE)
					{
						Raise_Error(_SC("not a table")); SQ_THROW();
					}
					
					sqi_table(ci->_imports)->import(sqi_table(t));
				}
				continue;

			} // switch(_i_.op)
		} // for (;;)
	}

exception_trap:
	{
		bool restore = ExceptionTrap(traps, _lasterror, _top, raiseerror);
		if (restore) goto exception_restore;
		return false;
	}
	assert(0);
}

bool SQVM::ExceptionTrap(SQInteger& traps, SQObjectPtr currerror, SQInteger last_top, SQBool raiseerror)
{
//	dumpstack(_stackbase);
//	SQInteger n = 0;

	if(_ss(this)->_notifyallexceptions || (!traps && raiseerror)) 
	{
		if (_debughook)
			CallDebugHook(_SC('e'));

		CallErrorHandler(currerror);
	}

	return UnwindTrap(traps, currerror, last_top);
}

bool SQVM::UnwindTrap(SQInteger& traps, SQObjectPtr currerror, SQInteger last_top)
{
	assert(_traprets.empty());

	while( ci ) {
		if(ci->_etraps > 0) {
			SQExceptionTrap &et = _etraps.top();

			assert(et._ip || et._finallyPos);
	
			if (et._ip && et._finallyPos)
			{
				SQExceptionTrap catchTrap(et._stacksize, et._stackbase, et._ip, et._extarget, NULL);
				et._ip = NULL;
				_etraps.push_back(catchTrap);
				++traps; ++ci->_etraps;
				continue;
			}

			if (et._ip)
			{
				ci->_ip = et._ip;
			}
			else
			{
				ci->_ip = et._finallyPos;
			}
			_top = et._stacksize;
			_stackbase = et._stackbase;
			_stack._vals[_stackbase + et._extarget] = _traperror = currerror;
			while(last_top >= _top) _stack._vals[last_top--].Null();
			--traps; --ci->_etraps; _etraps.pop_back();
			return true;
		}
		if(ci->_generator) ci->_generator->Kill();
		bool mustbreak = ci && ci->_root;
		LeaveFrame();
		if(mustbreak) break;
	}

	_lasterror = currerror;
	return false;
}

void SQVM::CallErrorHandler(SQObjectPtr &error)
{
	if(sqi_type(_errorhandler) != OT_NULL) {
		SQObjectPtr out;
		Push(_ss(this)->_root_table); Push(error);
		Call(_errorhandler, 2, _top-2, out,SQFalse);
		Pop(2);
	}
}


void SQVM::CallDebugHook(SQInteger type,SQInteger forcedline)
{
	_debughook = false;

	if (ci)
	{
		SQFunctionProto *func=sqi_closure(ci->_closure)->_function;
		if(_debughook_native) {
			const SQChar *src = sqi_type(func->_sourcename) == OT_STRING?sqi_stringval(func->_sourcename):NULL;
			const SQChar *fname = sqi_type(func->_name) == OT_STRING?sqi_stringval(func->_name):NULL;
			SQInteger line = forcedline?forcedline:func->GetLine(ci->_ip);
			_debughook_native(this,type,src,line,fname,_debughook_native_up);
		}
		else {
			SQObjectPtr temp_reg;
			SQInteger nparams=5;
			Push(_ss(this)->_root_table); Push(type); Push(func->_sourcename); Push(forcedline?forcedline:func->GetLine(ci->_ip)); Push(func->_name);
			Call(_debughook_closure,nparams,_top-nparams,temp_reg,SQFalse);
			Pop(nparams);
		}
	}
	else
	{
		if(_debughook_native)
			_debughook_native(this, type, NULL, 0, NULL, _debughook_native_up);
		else
		{
			SQObjectPtr temp_reg;
			SQInteger nparams=5;
			Push(_ss(this)->_root_table); Push(type); Push(_null_); Push(0); Push(_null_);
			Call(_debughook_closure,nparams,_top-nparams,temp_reg,SQFalse);
			Pop(nparams);
		}
	}

	_debughook = true;
}

bool SQVM::CallNative(SQNativeClosure *nclosure, SQInteger nargs, SQInteger newbase, SQObjectPtr &retval, bool &suspend)
{
	SQInteger nparamscheck = nclosure->_nparamscheck;
	SQInteger newtop = newbase + nargs + nclosure->_outervalues.size();

	if (_nnativecalls + 1 > MAX_NATIVE_CALLS) {
		_sharedstate->_errorfunc(this, "*** [FATAL] too many native calls");
		Raise_Error(_SC("Native stack overflow"));
		return false;
	}

	if(nparamscheck && (((nparamscheck > 0) && (nparamscheck != nargs)) ||
		((nparamscheck < 0) && (nargs < (-nparamscheck))))) // negative means: at least n.
	{
		Raise_Error(_SC("wrong number of parameters"));
		return false;
	}

	SQInteger tcs;
	SQIntVec &tc = nclosure->_typecheck;
	if((tcs = tc.size())) {
		for(SQInteger i = 0; i < nargs && i < tcs; i++) {
			if((tc._vals[i] != -1) && !(sqi_type(_stack._vals[newbase+i]) & tc._vals[i])) {
				Raise_ParamTypeError(i,tc._vals[i],sqi_type(_stack._vals[newbase+i]));
				return false;
			}
		}
	}

	if(!EnterFrame(newbase, newtop, false)) return false;
	GC_MUTATED(this);
	ci->_closure  = nclosure;

	SQInteger outers = nclosure->_outervalues.size();
	for (SQInteger i = 0; i < outers; i++) {
		_stack._vals[newbase+nargs+i] = nclosure->_outervalues[i];
	}
	if (sqi_type(nclosure->_env) != OT_NULL) {
		_getrealval(nclosure->_env, this, _stack._vals[newbase]);
	}

	_nnativecalls++;
	SQVM* prev_curr = _sharedstate->_curr_thread;
	_sharedstate->_curr_thread = this;
 	SQInteger ret = SQ_ERROR;

	try {
		_nativeException = true;
		ret = (nclosure->_function)(this); 
	} 
	catch (nit::ScriptException& )
	{
		ret = SQ_ERROR;
	}
	catch (nit::Exception& e)
	{
//		if (_nativeException)
			_lasterror = SQString::Create(_sharedstate, e.getFullDescription().c_str());
		ret = SQ_ERROR;
	}
	catch (...) 
	{ 
		// _lasterror was specified by thrower if not a native exception
//		if (_nativeException)
			_lasterror = SQString::Create(_sharedstate, _SC("native exception")); 
		ret = SQ_ERROR; 
	}

	_sharedstate->_curr_thread = prev_curr;
	_nnativecalls--;

	suspend = false;
	if (ret == SQ_SUSPEND_FLAG) {
		suspend = true;
	}
	else if (ret < 0) {
		LeaveFrame();
		Raise_Error(_lasterror);
		return false;
	}
	retval = ret ? _stack._vals[_top-1] : _null_;
	LeaveFrame();
	return true;
}

#define FALLBACK_OK			0
#define FALLBACK_NO_MATCH	1
#define FALLBACK_ERROR		2

bool SQVM::Get(const SQObjectPtr &self,const SQObjectPtr &key,SQObjectPtr &dest,bool raw, SQInteger selfidx)
{
	switch(sqi_type(self)){
	case OT_TABLE:
		if(sqi_table(self)->Get(key,dest))return true;
		break;
	case OT_ARRAY:
		if(sq_isnumeric(key)) 
		{ 
			if (sqi_array(self)->Get(sqi_tointeger(key),dest)) return true;
			Raise_IdxError(key);
			return false;
		}
		break;
	case OT_INSTANCE:
		{
			SQInstance::AccessResult r = sqi_instance(self)->Get(this, key,dest);
			if (r == SQInstance::IA_OK) return true;
			if (r == SQInstance::IA_ERROR) return false;
		}
		break;
	case OT_CLASS: 
		if(sqi_class(self)->Get(key,dest)) return true;
		break;
	case OT_STRING:
		if(sq_isnumeric(key)){
			SQInteger n=sqi_tointeger(key);
			if(abs((int)n)<sqi_string(self)->_len){
				if(n<0)n=sqi_string(self)->_len-n;
				dest=SQInteger(sqi_stringval(self)[n]);
				return true;
			}
			//Raise_IdxError(key);
			return false;
		}
		break;
	case OT_THREAD:
		// try to get from thread env
		if (Get(sqi_thread(self)->_threadlocal, key, dest, true, selfidx)) return true;
		break;
	default:break; //shut up compiler
	}
	if(!raw) {
		switch(FallBackGet(self,key,dest)) {
			case FALLBACK_OK: return true; //okie
			case FALLBACK_NO_MATCH: break; //keep falling back
			case FALLBACK_ERROR: return false; // the metamethod failed
		}
		if(InvokeDefaultDelegate(self,key,dest)) {
			return true;
		}
		if(selfidx == 0) {
			if(sqi_table(ci->_imports)->Get(key,dest)) return true;
			if(sqi_table(_ss(this)->_root_table)->Get(key,dest)) return true;
		}
	}
	Raise_IdxError(key);
	return false;
}

bool SQVM::InvokeDefaultDelegate(const SQObjectPtr &self,const SQObjectPtr &key,SQObjectPtr &dest)
{
	SQTable *ddel = NULL;
	switch(sqi_type(self)){
		case OT_NULL: ddel = _null_ddel; break;
		case OT_CLASS: ddel = _class_ddel; break;
		case OT_TABLE: ddel = _table_ddel; break;
		case OT_ARRAY: ddel = _array_ddel; break;
		case OT_STRING: ddel = _string_ddel; break;
		case OT_INSTANCE: ddel = _instance_ddel; break;
		case OT_INTEGER:case OT_FLOAT:case OT_BOOL: ddel = _number_ddel; break;
		case OT_GENERATOR: ddel = _generator_ddel; break;
		case OT_CLOSURE: case OT_NATIVECLOSURE:	ddel = _closure_ddel; break;
		case OT_THREAD: ddel = _thread_ddel; break;
		case OT_WEAKREF: ddel = _weakref_ddel; break;
		case OT_NATIVEWEAKREF: ddel = _nativeweakref_ddel; break;
		default: return false;
		}
	return  ddel->Get(key,dest);
}


SQInteger SQVM::FallBackGet(const SQObjectPtr &self,const SQObjectPtr &key,SQObjectPtr &dest)
{
	switch(sqi_type(self)){
	case OT_TABLE:
	case OT_USERDATA:
        //delegation
		if(sqi_delegable(self)->_delegate) {
			if(Get(SQObjectPtr(sqi_delegable(self)->_delegate),key,dest,false,DONT_FALL_BACK)) return FALLBACK_OK;	
		}
		else {
			return FALLBACK_NO_MATCH;
		}
		//go through
	case OT_INSTANCE: {
		SQObjectPtr closure;
		if(sqi_delegable(self)->GetMetaMethod(this, MT_GET, closure)) {
			Push(self);Push(key);
			_nmetamethodscall++;
			AutoDec ad(&_nmetamethodscall);
			if(Call(closure, 2, _top - 2, dest, SQTrue)) {
				Pop(2);
				return FALLBACK_OK;
			}
			else {
				if(sqi_type(_lasterror) != OT_NULL) { //NULL means "clean failure" (not found)
					//error
					Pop(2); // avoid side-effect
					return FALLBACK_ERROR;
				}
				Pop(2);
			}
		}
		}
		break;
	default:
		break;
	}
	// no metamethod or no fallback type
	return FALLBACK_NO_MATCH;
}

bool SQVM::Set(const SQObjectPtr &self,const SQObjectPtr &key,const SQObjectPtr &val,bool raw,SQInteger selfidx)
{
	switch(sqi_type(self)){
	case OT_TABLE:
		if(sqi_table(self)->Set(key,val)) return true;
		break;
	case OT_INSTANCE:
		{
			SQInstance::AccessResult r = sqi_instance(self)->Set(this, key,val);
			if (r == SQInstance::IA_OK) return true;
			if (r == SQInstance::IA_ERROR) return false;
		}
		break;
	case OT_ARRAY:
		if(!sq_isnumeric(key)) {Raise_Error(_SC("indexing %s with %s"),GetTypeName(self),GetTypeName(key)); return false; }
		return sqi_array(self)->Set(sqi_tointeger(key),val);
	case OT_THREAD:
		// try to set into thread local
		if (Set(sqi_thread(self)->_threadlocal, key, val, true, selfidx)) return true;
		break;
	default:
		Raise_Error(_SC("trying to set '%s'"),GetTypeName(self));
		return false;
	}

	if (!raw) {
		switch(FallBackSet(self,key,val)) {
			case FALLBACK_OK: return true; //okie
			case FALLBACK_NO_MATCH: break; //keep falling back
			case FALLBACK_ERROR: return false; // the metamethod failed
		}
		if(selfidx == 0) {
			if(sqi_table(_ss(this)->_root_table)->Set(key,val))
			return true;
		}
	}
	Raise_IdxError(key);
	return false;
}

SQInteger SQVM::FallBackSet(const SQObjectPtr &self,const SQObjectPtr &key,const SQObjectPtr &val)
{
	switch(sqi_type(self)){
	case OT_TABLE:
		if(sqi_table(self)->_delegate) {
			if(Set(sqi_table(self)->_delegate,key,val,false,DONT_FALL_BACK))	return FALLBACK_OK;
			}
		//keps on going
	case OT_INSTANCE:
	case OT_USERDATA:{
		SQObjectPtr closure;
			SQObjectPtr t;
		if(sqi_delegable(self)->GetMetaMethod(this, MT_SET, closure)) {
			Push(self);Push(key);Push(val);
			_nmetamethodscall++;
			AutoDec ad(&_nmetamethodscall);
			if(Call(closure, 3, _top - 3, t, SQTrue)) {
				Pop(3);
				return FALLBACK_OK;
		}
			else {
				if(sqi_type(_lasterror) != OT_NULL) { //NULL means "clean failure" (not found)
					//error
					Pop(3); // avoid side-effect
					return FALLBACK_ERROR;
		}
				Pop(3);
	}
			}
	}
		break;
	default: break;
}
	// no metamethod or no fallback type
	return FALLBACK_NO_MATCH;
}

bool SQVM::Clone(const SQObjectPtr &self,SQObjectPtr &target)
{
	SQObjectPtr temp_reg;
	SQObjectPtr newobj;
	switch(sqi_type(self)){
	case OT_NULL:
	case OT_INTEGER:
	case OT_FLOAT:
	case OT_BOOL:
	case OT_STRING:
	case OT_USERPOINTER:
		// safe types
		target = self;
		return true;
	case OT_TABLE:
		newobj = sqi_table(self)->Clone();
		goto cloned_mt;
	case OT_INSTANCE:
		if (sqi_instance(self)->IsPurged())
		{
			Raise_Error(_SC("cloning a purged instance"));
			return false;
		}
		else
			newobj = sqi_instance(self)->Clone(_ss(this));
cloned_mt:
		if(sqi_delegable(newobj)->_delegate){
			Push(newobj);
			Push(self);
			if (CallMetaMethod(sqi_delegable(newobj),MT_CLONED,2,temp_reg) != META_SUCCESS)
			{
				Raise_Error(_SC("cloning a %s failed"), GetTypeName(self));
				return false;
			}
		}
		target = newobj;
		return true;
	case OT_ARRAY: 
		target = sqi_array(self)->Clone();
		return true;
	default: 
		Raise_Error(_SC("cloning a %s"), GetTypeName(self));
		return false;
	}
}

bool SQVM::NewSlot(const SQObjectPtr &self,const SQObjectPtr &key,const SQObjectPtr &val,bool bstatic)
{
	if(sqi_type(key) == OT_NULL) { Raise_Error(_SC("null cannot be used as index")); return false; }
	switch(sqi_type(self)) {
	case OT_TABLE: {
		bool rawcall = true;
		if(sqi_table(self)->_delegate) {
			SQObjectPtr res;
			if(!sqi_table(self)->Get(key,res)) {
				Push(self);Push(key);Push(val);
				rawcall = CallMetaMethod(sqi_table(self),MT_NEWSLOT,3,res) != META_SUCCESS;
			}
		}
		if(rawcall) sqi_table(self)->NewSlot(key,val); //cannot fail
		
		break;}
	case OT_INSTANCE: {
		SQObjectPtr res;
		Push(self);Push(key);Push(val);
		if(CallMetaMethod(sqi_instance(self),MT_NEWSLOT,3,res) != META_SUCCESS) {
			Raise_Error(_SC("class instances do not support the new slot operator"));
			return false;
		}
		break;}
	case OT_CLASS: 
		if(!sqi_class(self)->NewSlot(_ss(this),key,val,bstatic)) {
			if(sqi_class(self)->_locked) {
				Raise_Error(_SC("trying to modify a class that has already been instantiated"));
				return false;
			}
			else {
				Raise_Error(_SC("property with same name already exists"));
				return false;
			}
		}
		break;
	case OT_THREAD:
		return NewSlot(sqi_thread(self)->_threadlocal, key, val, bstatic);

	default:
		Raise_Error(_SC("indexing %s with %s"),GetTypeName(self),GetTypeName(key));
		return false;
		break;
	}
	return true;
}

bool SQVM::NewProp(const SQObjectPtr& self, const SQObjectPtr& name, const SQObjectPtr& getter, const SQObjectPtr& setter)
{
	if (sqi_type(name) == OT_NULL) { Raise_Error(_SC("null cannot be used as property name")); return false; }
	if (sqi_type(self) != OT_CLASS) { Raise_Error(_SC("property can only be declared to a class")); return false; }
	if (!sqi_class(self)->NewProperty(_ss(this), name, getter, setter))
	{
		if (sqi_class(self)->_locked) { Raise_Error(_SC("trying to modify a class that has already been instantiated")); return false; }
		else if ( (sqi_type(getter) == OT_NULL || sqi_type(getter) == OT_CLOSURE || sqi_type(getter) == OT_NATIVECLOSURE) 
			   && (sqi_type(setter) == OT_NULL || sqi_type(setter) == OT_CLOSURE || sqi_type(setter) == OT_NATIVECLOSURE) )
			{ Raise_Error(_SC("field or method with same name already exists")); return false; }
		else { Raise_Error(_SC("property getter/setter should be a closure/native-closure")); return false; }
	}
	return true;
}

bool SQVM::DeleteSlot(const SQObjectPtr &self,const SQObjectPtr &key,SQObjectPtr &res)
{
	switch(sqi_type(self)) {
	case OT_TABLE:
	case OT_INSTANCE:
	case OT_USERDATA: {
		SQObjectPtr t;
		bool handled = false;
		if(sqi_delegable(self)->_delegate) {
			Push(self);Push(key);
			handled = CallMetaMethod(sqi_delegable(self),MT_DELSLOT,2,t) == META_SUCCESS;
		}

		if(!handled) {
			if(sqi_type(self) == OT_TABLE) {
				if(sqi_table(self)->Get(key,t)) {
					sqi_table(self)->Remove(key);
				}
				else {
					Raise_IdxError((SQObject &)key);
					return false;
				}
			}
			else {
				Raise_Error(_SC("cannot delete a slot from %s"),GetTypeName(self));
				return false;
			}
		}
		res = t;
				}
		break;
	case OT_THREAD:
		return DeleteSlot(sqi_thread(self)->_threadlocal, key, res);
	default:
		Raise_Error(_SC("attempt to delete a slot from a %s"),GetTypeName(self));
		return false;
	}
	return true;
}

bool SQVM::CallInitializerChain(SQClass* cls, SQInteger stackbase, SQBool raiseerror)
{
	SQObjectPtr fn;
	SQObjectPtr temp;

	bool ok = true;
	
	if (cls->_base && cls->_base->_needinitializer)
		ok = CallInitializerChain(cls->_base, stackbase, raiseerror);
	
	if (ok && cls->GetInitializer(fn))
	{
		ok = Call(fn, 1, stackbase, temp, raiseerror);
	}
		
	return ok;
}

void SQVM::CallDestructorChain(SQInstance* instance, SQClass* cls)
{
	SQObjectPtr fn;
	SQObjectPtr temp;

	if (cls->GetDestructor(fn))
		Call(fn, 1, _top-1, temp, true);

	if (cls->_base && cls->_base->_needdestructor)
		CallDestructorChain(instance, cls->_base);
}

bool SQVM::Call(SQObjectPtr &closure,SQInteger nparams,SQInteger stackbase,SQObjectPtr &outres,SQBool raiseerror)
{
#ifdef _DEBUG
SQInteger prevstackbase = _stackbase;
#endif
	switch(sqi_type(closure)) {
	case OT_CLOSURE:
		return Execute(closure, nparams, stackbase, outres, raiseerror);
		break;
	case OT_NATIVECLOSURE:{
		bool suspend;
		return CallNative(sqi_nativeclosure(closure), nparams, stackbase, outres,suspend);
		
						  }
		break;
	case OT_CLASS: 
		{
			SQClass* cls = sqi_class(closure);
			outres = cls->CreateInstance();
			_stack[stackbase] = outres;
			SQObjectPtr fn;
			SQObjectPtr temp;

			bool ok = true;
			if (cls->_needinitializer)
			{
				assert(_top + 1 <= (SQInteger)_stack.size());
				Push(outres);
				ok = CallInitializerChain(cls, _top-1, raiseerror);
				Pop();
			}
			
			if (ok && cls->GetConstructor(fn))
				ok = Call(fn, nparams, stackbase, temp, raiseerror);

			return ok;
		}
		break;
	default:
		return false;
	}
#ifdef _DEBUG
	if(!_suspended) {
		assert(_stackbase == prevstackbase);
	}
#endif
	return true;
}

SQVM::MetaMethodResult SQVM::CallMetaMethod(SQDelegable *del,SQMetaMethod mm,SQInteger nparams,SQObjectPtr &outres)
{
	SQObjectPtr closure;
	if(del->GetMetaMethod(this, mm, closure)) 
	{
		_nmetamethodscall++;
		if(Call(closure, nparams, _top - nparams, outres, SQTrue)) {
			_nmetamethodscall--;
			Pop(nparams);
			return META_SUCCESS;
		}
		_nmetamethodscall--;
		Pop(nparams);
		return META_ERROR;
	}
	else
	{
		Pop(nparams);
		return META_NOT_FOUND;
	}
}

void SQVM::FindOuter(SQObjectPtr &target, SQObjectPtr *stackindex)
{
	SQOuter **pp = &_openouters;
	SQOuter  *p;
	SQOuter  *otr;

	while ((p = *pp) != NULL && p->_valptr >= stackindex) {
		if (p->_valptr == stackindex) {
			target = SQObjectPtr(p);
			return;
		}
		pp = &p->_next;
	}
	otr = SQOuter::Create(_ss(this), stackindex);
	otr->_next = *pp;
	otr->_idx  = (stackindex - _stack._vals);
	__ObjAddRef(otr);
	*pp = otr;
	target = SQObjectPtr(otr);
	return;
}

bool SQVM::EnterFrame(SQInteger newbase, SQInteger newtop, bool tailcall)
{
	if( !tailcall ) {
		if( _callsstacksize == _alloccallsstacksize ) {
			GrowCallStack();
		}
		ci = &_callsstack[_callsstacksize++];
		ci->_prevstkbase = (SQInt32)(newbase - _stackbase);
		ci->_prevtop = (SQInt32)(_top - _stackbase);
		ci->_etraps      = 0;
		ci->_ncalls      = 1;
		ci->_generator   = NULL;
		ci->_root        = SQFalse;
	}
	else {
		ci->_ncalls++;
	}

	_stackbase = newbase;
	_top       = newtop;
	if(newtop + MIN_STACK_OVERHEAD > (SQInteger)_stack.size()) {
		if(_nmetamethodscall) {
			_sharedstate->_errorfunc(this, "*** [FATAL] stack overflow in a metamethod");
			Raise_Error(_SC("stack overflow, cannot resize stack while in a metamethod"));
			return false;
		}
		_stack.resize(_stack.size() + (MIN_STACK_OVERHEAD << 2));
		RelocateOuters();
	}
	return true;
}

void SQVM::LeaveFrame() {
	SQInteger last_top = _top;
	SQInteger last_stackbase = _stackbase;
	SQInteger css = --_callsstacksize;

	/* First clean out the call stack frame */
	ci->_closure.Null();
	ci->_imports.Null();
	_stackbase -= ci->_prevstkbase;
	_top        = _stackbase + ci->_prevtop;
	ci = (css) ? &_callsstack[css-1] : NULL;

	if(_openouters) CloseOuters(&(_stack._vals[last_stackbase]));
	while (last_top >= _top) {
		_stack._vals[last_top--].Null();
	}
}

void SQVM::RelocateOuters()
{
	SQOuter *p = _openouters;
	while (p) {
		p->_valptr = _stack._vals + p->_idx;
		p = p->_next;
	}
}

void SQVM::CloseOuters(SQObjectPtr *stackindex) {
  SQOuter *p;
  while ((p = _openouters) != NULL && p->_valptr >= stackindex) {
    p->_value = *(p->_valptr);
	GC_MUTATED(p);
    p->_valptr = &p->_value;
    _openouters = p->_next;
  	__ObjRelease(p);
  }
}

void SQVM::Remove(SQInteger n) {
	n = (n >= 0)?n + _stackbase - 1:_top + n;
	for(SQInteger i = n; i < _top; i++){
		_stack[i] = _stack[i+1];
	}
	_stack[_top].Null();
	_top--;
}

void SQVM::Insert(SQInteger n, SQInteger count) {
	// NOTE: n == 0 is treated as absolute index
	n = (n > 0) ? n + _stackbase - 1 : _top + n;
	for (SQInteger i = _top - 1; i >= n; --i) {
		_stack[i+count] = _stack[i];
	}
	for (SQInteger i = n+count-1; i >= n; --i) {
		_stack[i].Null();
	}
	_top += count;
}

void SQVM::Replace(SQInteger n)
{
	n = (n >= 0)?n + _stackbase - 1:_top + n;
	_stack[n] = _stack[--_top];
	_stack[_top].Null();
}

void SQVM::Pop() {
	_stack[--_top].Null();
}

void SQVM::Pop(SQInteger n) {
	for(SQInteger i = 0; i < n; i++){
		_stack[--_top].Null();
	}
}

void SQVM::Push(const SQObjectPtr &o) { _stack[_top++] = o; }
SQObjectPtr &SQVM::Top() { return _stack[_top-1]; }
SQObjectPtr &SQVM::PopGet() { return _stack[--_top]; }
SQObjectPtr &SQVM::GetUp(SQInteger n) { return _stack[_top+n]; }
SQObjectPtr &SQVM::GetAt(SQInteger n) { return _stack[n]; }

#ifdef _DEBUG_DUMP
void SQVM::dumpstack(SQInteger stackbase,bool dumpall)
{
	SQInteger size=dumpall?_stack.size():_top;
	SQInteger n=0;
	scprintf(_SC("\n>>>>stack dump<<<<\n"));
	CallInfo &ci=_callsstack[_callsstacksize-1];
	scprintf(_SC("IP: %p\n"),ci._ip);
	scprintf(_SC("prev stack base: %d\n"),ci._prevstkbase);
	scprintf(_SC("prev top: %d\n"),ci._prevtop);
	for(SQInteger i=0;i<size;i++){
		SQObjectPtr &obj=_stack[i];	
		if(stackbase==i)scprintf(_SC(">"));else scprintf(_SC(" "));
		scprintf(_SC("[%d]:"),n);
		switch(sqi_type(obj)){
		case OT_FLOAT:			scprintf(_SC("FLOAT %.3f"),sqi_float(obj));break;
		case OT_INTEGER:		scprintf(_SC("INTEGER %d"),sqi_integer(obj));break;
		case OT_BOOL:			scprintf(_SC("BOOL %s"),sqi_integer(obj)?"true":"false");break;
		case OT_STRING:			scprintf(_SC("STRING %s"),sqi_stringval(obj));break;
		case OT_NULL:			scprintf(_SC("NULL"));	break;
		case OT_TABLE:			scprintf(_SC("TABLE %p[%p]"),sqi_table(obj),sqi_table(obj)->_delegate);break;
		case OT_ARRAY:			scprintf(_SC("ARRAY %p"),sqi_array(obj));break;
		case OT_CLOSURE:		scprintf(_SC("CLOSURE [%p]"),sqi_closure(obj));break;
		case OT_NATIVECLOSURE:	scprintf(_SC("NATIVECLOSURE"));break;
		case OT_USERDATA:		scprintf(_SC("USERDATA %p[%p]"),sqi_userdataval(obj),sqi_userdata(obj)->_delegate);break;
		case OT_GENERATOR:		scprintf(_SC("GENERATOR %p"),sqi_generator(obj));break;
		case OT_THREAD:			scprintf(_SC("THREAD [%p]"),sqi_thread(obj));break;
		case OT_USERPOINTER:	scprintf(_SC("USERPOINTER %p"),sqi_userpointer(obj));break;
		case OT_CLASS:			scprintf(_SC("CLASS %p"),sqi_class(obj));break;
		case OT_INSTANCE:		scprintf(_SC("INSTANCE %p"),sqi_instance(obj));break;
		case OT_WEAKREF:		scprintf(_SC("WEAKERF %p"),_weakref(obj));break;
		case OT_NATIVEWEAKREF:	scprintf(_SC("NATIVEWEAKERF %p"),sqi_nativeweakref(obj));break;
		default:
			assert(0);
			break;
		};
		scprintf(_SC("\n"));
		++n;
	}
}



#endif
