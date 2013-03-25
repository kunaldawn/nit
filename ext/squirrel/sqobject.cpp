/*
	see copyright notice in squirrel.h
*/
#include "sqpcheader.h"
#include "sqvm.h"
#include "sqstring.h"
#include "sqarray.h"
#include "sqtable.h"
#include "squserdata.h"
#include "sqfuncproto.h"
#include "sqclass.h"
#include "sqclosure.h"


const SQChar *IdType2Name(SQObjectType type)
{
	switch(_RAW_TYPE(type))
	{
	case _RT_NULL:return _SC("null");
	case _RT_INTEGER:return _SC("integer");
	case _RT_FLOAT:return _SC("float");
	case _RT_BOOL:return _SC("bool");
	case _RT_STRING:return _SC("string");
	case _RT_TABLE:return _SC("table");
	case _RT_ARRAY:return _SC("array");
	case _RT_GENERATOR:return _SC("generator");
	case _RT_CLOSURE:
	case _RT_NATIVECLOSURE:
		return _SC("function");
	case _RT_USERDATA:
	case _RT_USERPOINTER:
		return _SC("userdata");
	case _RT_THREAD: return _SC("thread");
	case _RT_FUNCPROTO: return _SC("function");
	case _RT_CLASS: return _SC("class");
	case _RT_INSTANCE: return _SC("instance");
	case _RT_WEAKREF: return _SC("weakref");
	case _RT_NATIVEWEAKREF: return _SC("nativeweakref");
	case _RT_OUTER: return _SC("outer");
	default:
		return NULL;
	}
}

const SQChar *GetTypeName(const SQObjectPtr &obj1)
{
	return IdType2Name(sqi_type(obj1));	
}

SQString *SQString::Create(SQSharedState *ss,const SQChar *s,SQInteger len)
{
	assert(len >= -1);
	SQString *str=ADD_STRING(ss,s,len);
	return str;
}

void SQString::Release()
{
	REMOVE_STRING(_sharedstate,this);
}

SQInteger SQString::Next(const SQObjectPtr &refpos, SQObjectPtr &outkey, SQObjectPtr &outval)
{
	SQInteger idx = (SQInteger)TranslateIndex(refpos);
	while(idx < _len){
		outkey = (SQInteger)idx;
		outval = SQInteger(_val[idx]);
		//return idx for the next iteration
		return ++idx;
	}
	//nothing to iterate anymore
	return -1;
}

SQUnsignedInteger TranslateIndex(const SQObjectPtr &idx)
{
	switch(sqi_type(idx)){
		case OT_NULL:
			return 0;
		case OT_INTEGER:
			return (SQUnsignedInteger)sqi_integer(idx);
		default: assert(0); break;
	}
	return 0;
}

SQWeakRef *SQRefCounted::GetWeakRef(SQObjectType type)
{
	if(!_weakref) {
		sq_new(_weakref,SQWeakRef);
		_weakref->_obj._type = type;
		_weakref->_obj._unVal.pRefCounted = this;
	}
	return _weakref;
}

SQRefCounted::~SQRefCounted()
{
	if(_weakref) {
		_weakref->_obj._type = OT_NULL;
		_weakref->_obj._unVal.pRefCounted = NULL;
	}
}

void SQWeakRef::Release() { 
	if(SQ_ISREFCOUNTED(_obj._type)) { 
		_obj._unVal.pRefCounted->_weakref = NULL;
	} 
	sq_delete(this,SQWeakRef);
}

bool SQDelegable::GetMetaMethod(SQVM *v,SQMetaMethod mm,SQObjectPtr &res) {
	if(_delegate) {
		return _delegate->Get((*_ss(v)->_metamethods)[mm],res);
	}
	return false;
}

bool SQDelegable::SetDelegate(SQTable *mt)
{
	SQTable *temp = mt;
	if(temp == this) return false;
	while (temp) {
		if (temp->_delegate == this) return false; //cycle detected
		temp = temp->_delegate;
	}
	if (mt)	__ObjAddRef(mt);
	__ObjRelease(_delegate);
	_delegate = mt;
	GC_MUTATED(this);
	return true;
}

bool SQGenerator::Yield(SQVM *v,SQInteger target)
{
	if(_state==eSuspended) { v->Raise_Error(_SC("internal vm error, yielding dead generator"));  return false;}
	if(_state==eDead) { v->Raise_Error(_SC("internal vm error, yielding a dead generator")); return false; }
	SQInteger size = v->_top-v->_stackbase;
	
	_stack.resize(size);
	for(SQInteger n =0; n<target; n++) {
		_stack._vals[n] = v->_stack[v->_stackbase+n];
	}
	GC_MUTATED(this);
	for(SQInteger j =0; j < size; j++)
	{
		v->_stack[v->_stackbase+j] = _null_;
	}

	_ci = *v->ci;
	_ci._generator=NULL;
	for(SQInteger i=0;i<_ci._etraps;i++) {
		_etraps.push_back(v->_etraps.top());
		v->_etraps.pop_back();
	}
	_state=eSuspended;
	return true;
}

bool SQGenerator::Resume(SQVM *v,SQObjectPtr &dest)
{
	if(_state==eDead){ v->Raise_Error(_SC("resuming dead generator")); return false; }
	if(_state==eRunning){ v->Raise_Error(_SC("resuming active generator")); return false; }
	SQInteger size = _stack.size();
	SQInteger target = &dest - &(v->_stack._vals[v->_stackbase]);
	assert(target>=0 && target<=255);
	if(!v->EnterFrame(v->_top, v->_top + size, false)) 
		return false;
	v->ci->_generator   = this;
	v->ci->_target      = (SQInt32)target;
	v->ci->_closure     = _ci._closure;
	v->ci->_ip          = _ci._ip;
	v->ci->_literals    = _ci._literals;
	v->ci->_ncalls      = _ci._ncalls;
	v->ci->_etraps      = _ci._etraps;
	v->ci->_root        = _ci._root;


	for(SQInteger i=0;i<_ci._etraps;i++) {
		v->_etraps.push_back(_etraps.top());
		_etraps.pop_back();
	}

	for(SQInteger n = 0; n<size; n++) {
		v->_stack[v->_stackbase+n] = _stack._vals[n];
		_stack._vals[n] = _null_;
	}

	_state=eRunning;
	if (v->_debughook)
		v->CallDebugHook(_SC('c'));

	return true;
}

void SQArray::Extend(const SQArray *a){
	SQInteger xlen;
	if((xlen=a->Size()))
	{
		for(SQInteger i=0;i<xlen;i++)
			_values.push_back(a->_values[i]);
		GC_MUTATED(this);
	}
}

const SQChar* SQFunctionProto::GetLocal(SQVM *vm,SQUnsignedInteger stackbase,SQUnsignedInteger nseq,SQUnsignedInteger nop)
{
	SQUnsignedInteger nvars=_nlocalvarinfos;
	const SQChar *res=NULL; 
	if(nvars>=nseq){
 		for(SQUnsignedInteger i=0;i<nvars;i++){
			if(_localvarinfos[i]._start_op<=nop && _localvarinfos[i]._end_op>=nop)
			{
				if(nseq==0){
					vm->Push(vm->_stack[stackbase+_localvarinfos[i]._pos]);
					res=sqi_stringval(_localvarinfos[i]._name);
					break;
				}
				nseq--;
			}
		}
	}
	return res;
}

SQInteger SQFunctionProto::GetLine(SQInstruction *curr)
{
	SQInteger op = (SQInteger)(curr-_instructions);
	SQInteger line=_lineinfos[0]._line;
	for(SQInteger i=1;i<_nlineinfos;i++){
		if(_lineinfos[i]._op>=op)
			return line;
		line=_lineinfos[i]._line;
	}
	return line;
}

SQClosure::~SQClosure()
{
	REMOVE_FROM_CHAIN(&_ss(this)->_gc_chain,this);
	Finalize();
}

void SQClosure::Finalize()
{
	_env.Null();
	__ObjRelease(_base);
	SQFunctionProto *f = _function;
	if (f)
	{
		for(SQInteger i = 0; i < f-> _noutervalues; i++)
			_outervalues[i].Null();
		for(SQInteger j = 0; j < f-> _ndefaultparams; j++)
			_defaultparams[j].Null();
	}
}

#define _CHECK_IO(exp)  { if(!exp)return false; }

bool SafeWrite(HSQUIRRELVM v,SQWRITEFUNC write,SQUserPointer up,SQUserPointer dest,SQInteger size)
{
	if(write(up,dest,size) != size) {
		v->Raise_Error(_SC("io error (write function failure)"));
		return false;
	}
	return true;
}

bool SafeRead(HSQUIRRELVM v,SQWRITEFUNC read,SQUserPointer up,SQUserPointer dest,SQInteger size)
{
 	if(size && read(up,dest,size) != size) {
 		v->Raise_Error(_SC("io error, read function failure, the origin stream could be corrupted/trucated"));
 		return false;
 	}
 	return true;
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

bool WriteInt(HSQUIRRELVM v,SQWRITEFUNC write,SQUserPointer up,SQInteger val, SQBool swapEndian)
{
	if (swapEndian) flipEndian(&val, sizeof(val));
	return SafeWrite(v,write,up,&val,sizeof(val));
}

bool WriteTag(HSQUIRRELVM v,SQWRITEFUNC write,SQUserPointer up,SQInteger tag, SQBool swapEndian)
{
	return WriteInt(v, write, up, tag, swapEndian);
}

bool CheckTag(HSQUIRRELVM v,SQWRITEFUNC read,SQUserPointer up,SQInteger tag)
{
	SQInteger t;
	_CHECK_IO(SafeRead(v,read,up,&t,sizeof(t)));
	if(t != tag){
		v->Raise_Error(_SC("invalid or corrupted closure stream"));
		return false;
	}
	return true;
}

bool WriteObject(HSQUIRRELVM v,SQUserPointer up,SQWRITEFUNC write,SQObjectPtr &o, SQBool swapEndian)
{
	SQInteger ot = sqi_type(o);
	if (swapEndian) flipEndian(&ot, sizeof(ot));
	_CHECK_IO(SafeWrite(v,write,up,&ot, sizeof(ot)));
	switch(sqi_type(o)){
	case OT_STRING:
		{
			SQInteger len = sqi_string(o)->_len;
			if (swapEndian) flipEndian(&len, sizeof(len));
			_CHECK_IO(SafeWrite(v,write,up,&len,sizeof(len)));
			_CHECK_IO(SafeWrite(v,write,up,sqi_stringval(o),rsl(sqi_string(o)->_len)));
		}
		break;
	case OT_INTEGER:
		{
			SQInteger val = sqi_integer(o);
			if (swapEndian) flipEndian(&val, sizeof(val));
			_CHECK_IO(SafeWrite(v,write,up,&val,sizeof(val)));break;
		}
	case OT_FLOAT:
		{
			SQFloat val = sqi_float(o);
			if (swapEndian) flipEndian(&val, sizeof(val));
			_CHECK_IO(SafeWrite(v,write,up,&val,sizeof(val)));break;
		}
	case OT_NULL:
		break;
	default:
		v->Raise_Error(_SC("cannot serialize a %s"),GetTypeName(o));
		return false;
	}
	return true;
}

bool ReadObject(HSQUIRRELVM v,SQUserPointer up,SQREADFUNC read,SQObjectPtr &o)
{
	SQInteger t;
	_CHECK_IO(SafeRead(v,read,up,&t,sizeof(t)));
	switch(t){
	case OT_STRING:{
		SQInteger len;
		_CHECK_IO(SafeRead(v,read,up,&len,sizeof(SQInteger)));
		_CHECK_IO(SafeRead(v,read,up,_ss(v)->GetScratchPad(rsl(len)),rsl(len)));
		o=SQString::Create(_ss(v),_ss(v)->GetScratchPad(-1),len);
				   }
		break;
	case OT_INTEGER:{
		SQInteger i;
		_CHECK_IO(SafeRead(v,read,up,&i,sizeof(SQInteger))); o = i; break;
					}
	case OT_FLOAT:{
		SQFloat f;
		_CHECK_IO(SafeRead(v,read,up,&f,sizeof(SQFloat))); o = f; break;
				  }
	case OT_NULL:
		o=_null_;
		break;
	default:
		v->Raise_Error(_SC("cannot serialize a %s"),IdType2Name((SQObjectType)t));
		return false;
	}
	return true;
}

bool SQClosure::Save(SQVM *v,SQUserPointer up,SQWRITEFUNC write, SQBool swapEndian)
{
	_CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_HEAD, swapEndian));
	_CHECK_IO(WriteTag(v,write,up,sizeof(SQChar), swapEndian));
	_CHECK_IO(_function->Save(v,up,write, swapEndian));
	_CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_TAIL, swapEndian));
	return true;
}

bool SQClosure::Load(SQVM *v,SQUserPointer up,SQREADFUNC read, SQObjectPtr imports, SQObjectPtr &ret)
{
	_CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_HEAD));
	_CHECK_IO(CheckTag(v,read,up,sizeof(SQChar)));
	SQObjectPtr func;
	_CHECK_IO(SQFunctionProto::Load(v,up,read,imports,func));
	_CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_TAIL));
	ret = SQClosure::Create(_ss(v),sqi_funcproto(func));
	return true;
}

SQFunctionProto::SQFunctionProto(SQSharedState *ss)
{
	_stacksize=0;
	_bgenerator=false;
	INIT_CHAIN();ADD_TO_CHAIN(&_ss(this)->_gc_chain,this);
}

SQFunctionProto::~SQFunctionProto()
{
	REMOVE_FROM_CHAIN(&_ss(this)->_gc_chain,this);
}

void SQFunctionProto::Finalize()
{
	for(SQInteger i = 0; i < _nliterals; i++)
		_literals[i].Null();
	_help.Null();
	_imports.Null();
}

bool SQFunctionProto::Save(SQVM *v,SQUserPointer up,SQWRITEFUNC write, SQBool swapEndian)
{
	SQInteger i,nliterals = _nliterals,nparameters = _nparameters;
	SQInteger noutervalues = _noutervalues,nlocalvarinfos = _nlocalvarinfos;
	SQInteger nlineinfos=_nlineinfos,ninstructions = _ninstructions,nfunctions=_nfunctions;
	SQInteger ndefaultparams = _ndefaultparams;
	_CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART, swapEndian));
	_CHECK_IO(WriteObject(v,up,write,_sourcename, swapEndian));
	_CHECK_IO(WriteObject(v,up,write,_name, swapEndian));
	_CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART, swapEndian));
	_CHECK_IO(WriteInt(v,write,up,nliterals,swapEndian));
	_CHECK_IO(WriteInt(v,write,up,nparameters,swapEndian));
	_CHECK_IO(WriteInt(v,write,up,noutervalues,swapEndian));
	_CHECK_IO(WriteInt(v,write,up,nlocalvarinfos,swapEndian));
	_CHECK_IO(WriteInt(v,write,up,nlineinfos,swapEndian));
	_CHECK_IO(WriteInt(v,write,up,ndefaultparams,swapEndian));
	_CHECK_IO(WriteInt(v,write,up,ninstructions,swapEndian));
	_CHECK_IO(WriteInt(v,write,up,nfunctions,swapEndian));
	_CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART, swapEndian));
	for(i=0;i<nliterals;i++){
		_CHECK_IO(WriteObject(v,up,write,_literals[i], swapEndian));
	}

	_CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART, swapEndian));
	for(i=0;i<nparameters;i++){
		_CHECK_IO(WriteObject(v,up,write,_parameters[i], swapEndian));
	}

	_CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART, swapEndian));
	for(i=0;i<noutervalues;i++){
		_CHECK_IO(WriteInt(v,write,up,_outervalues[i]._type,swapEndian));
		_CHECK_IO(WriteObject(v,up,write,_outervalues[i]._src, swapEndian));
		_CHECK_IO(WriteObject(v,up,write,_outervalues[i]._name, swapEndian));
	}

	_CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART, swapEndian));
	for(i=0;i<nlocalvarinfos;i++){
		SQLocalVarInfo &lvi=_localvarinfos[i];
		_CHECK_IO(WriteObject(v,up,write,lvi._name, swapEndian));
		_CHECK_IO(WriteInt(v,write,up,lvi._pos,swapEndian));
		_CHECK_IO(WriteInt(v,write,up,lvi._start_op,swapEndian));
		_CHECK_IO(WriteInt(v,write,up,lvi._end_op,swapEndian));
	}

	_CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART, swapEndian));
	for (i=0; i<nlineinfos; ++i)
	{
		_CHECK_IO(WriteInt(v,write,up,_lineinfos[i]._line, swapEndian));
		_CHECK_IO(WriteInt(v,write,up,_lineinfos[i]._op, swapEndian));
	}

	_CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART, swapEndian));
	for (i=0; i<ndefaultparams; ++i)
	{
		_CHECK_IO(WriteInt(v,write,up,_defaultparams[i], swapEndian));
	}

	_CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART, swapEndian));
	for (i=0; i<ninstructions; ++i)
	{
		_CHECK_IO(WriteInt(v,write,up,_instructions[i]._arg1, swapEndian));
		_CHECK_IO(SafeWrite(v,write,up,&_instructions[i].op, sizeof(unsigned char)));
		_CHECK_IO(SafeWrite(v,write,up,&_instructions[i]._arg0, sizeof(unsigned char)));
		_CHECK_IO(SafeWrite(v,write,up,&_instructions[i]._arg2, sizeof(unsigned char)));
		_CHECK_IO(SafeWrite(v,write,up,&_instructions[i]._arg3, sizeof(unsigned char)));
	}

	_CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART, swapEndian));
	for(i=0;i<nfunctions;i++){
		_CHECK_IO(sqi_funcproto(_functions[i])->Save(v,up,write, swapEndian));
	}
	_CHECK_IO(WriteInt(v,write,up,_stacksize, swapEndian));
	_CHECK_IO(WriteInt(v,write,up,_bgenerator, swapEndian));
	_CHECK_IO(WriteInt(v,write,up,_varparams, swapEndian));
	return true;
}

bool SQFunctionProto::Load(SQVM *v,SQUserPointer up,SQREADFUNC read, SQObjectPtr imports, SQObjectPtr &ret)
{
	SQInteger i, nliterals,nparameters;
	SQInteger noutervalues ,nlocalvarinfos ;
	SQInteger nlineinfos,ninstructions ,nfunctions,ndefaultparams ;
	SQObjectPtr sourcename, name;
	SQObjectPtr o;
	_CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));
	_CHECK_IO(ReadObject(v, up, read, sourcename));
	_CHECK_IO(ReadObject(v, up, read, name));
	
	_CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));
	_CHECK_IO(SafeRead(v,read,up, &nliterals, sizeof(nliterals)));
	_CHECK_IO(SafeRead(v,read,up, &nparameters, sizeof(nparameters)));
	_CHECK_IO(SafeRead(v,read,up, &noutervalues, sizeof(noutervalues)));
	_CHECK_IO(SafeRead(v,read,up, &nlocalvarinfos, sizeof(nlocalvarinfos)));
	_CHECK_IO(SafeRead(v,read,up, &nlineinfos, sizeof(nlineinfos)));
	_CHECK_IO(SafeRead(v,read,up, &ndefaultparams, sizeof(ndefaultparams)));
	_CHECK_IO(SafeRead(v,read,up, &ninstructions, sizeof(ninstructions)));
	_CHECK_IO(SafeRead(v,read,up, &nfunctions, sizeof(nfunctions)));
	

	SQFunctionProto *f = SQFunctionProto::Create(_opt_ss(v),ninstructions,nliterals,nparameters,
			nfunctions,noutervalues,nlineinfos,nlocalvarinfos,ndefaultparams);
	SQObjectPtr proto = f; //gets a ref in case of failure
	f->_sourcename = sourcename;
	f->_name = name;
	f->_imports = imports;

	_CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));

	for(i = 0;i < nliterals; i++){
		_CHECK_IO(ReadObject(v, up, read, o));
		f->_literals[i] = o;
	}
	_CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));

	for(i = 0; i < nparameters; i++){
		_CHECK_IO(ReadObject(v, up, read, o));
		f->_parameters[i] = o;
	}
	_CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));

	for(i = 0; i < noutervalues; i++){
		SQUnsignedInteger type;
		SQObjectPtr name;
		_CHECK_IO(SafeRead(v,read,up, &type, sizeof(SQUnsignedInteger)));
		_CHECK_IO(ReadObject(v, up, read, o));
		_CHECK_IO(ReadObject(v, up, read, name));
		f->_outervalues[i] = SQOuterVar(name,o, (SQOuterType)type);
	}
	_CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));

	for(i = 0; i < nlocalvarinfos; i++){
		SQLocalVarInfo lvi;
		_CHECK_IO(ReadObject(v, up, read, lvi._name));
		_CHECK_IO(SafeRead(v,read,up, &lvi._pos, sizeof(SQUnsignedInteger)));
		_CHECK_IO(SafeRead(v,read,up, &lvi._start_op, sizeof(SQUnsignedInteger)));
		_CHECK_IO(SafeRead(v,read,up, &lvi._end_op, sizeof(SQUnsignedInteger)));
		f->_localvarinfos[i] = lvi;
	}
	_CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));
	_CHECK_IO(SafeRead(v,read,up, f->_lineinfos, sizeof(SQLineInfo)*nlineinfos));

	_CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));
	_CHECK_IO(SafeRead(v,read,up, f->_defaultparams, sizeof(SQInteger)*ndefaultparams));

	_CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));
	_CHECK_IO(SafeRead(v,read,up, f->_instructions, sizeof(SQInstruction)*ninstructions));

	_CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));
	for(i = 0; i < nfunctions; i++){
		_CHECK_IO(sqi_funcproto(o)->Load(v, up, read, imports, o));
		f->_functions[i] = o;
	}
	_CHECK_IO(SafeRead(v,read,up, &f->_stacksize, sizeof(f->_stacksize)));
	_CHECK_IO(SafeRead(v,read,up, &f->_bgenerator, sizeof(f->_bgenerator)));
	_CHECK_IO(SafeRead(v,read,up, &f->_varparams, sizeof(f->_varparams)));
	
	ret = f;
	return true;
}

void SQNativeWeakRef::Release()
{
	if (_hook) { _sharedstate->CallReleaseHook(_hook, _userpointer, 0);}
	sq_delete(this, SQNativeWeakRef);
}

SQVM* SQNativeWeakRef::GetWorkerThread()
{
	return _sharedstate->GetWorkThread();
}
