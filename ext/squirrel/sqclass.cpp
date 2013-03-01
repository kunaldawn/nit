/*
	see copyright notice in squirrel.h
*/
#include "sqpcheader.h"
#include "sqvm.h"
#include "sqtable.h"
#include "sqclass.h"
#include "sqfuncproto.h"
#include "sqclosure.h"

SQClass::SQClass(SQSharedState *ss,SQClass *base)
{
	_base = base;
	_typetag = 0;
	_hook = NULL;
	_udsize = 0;
	_locked = false;
	_initializeridx = -1;
	_constructoridx = -1;
	_destructoridx = -1;
	_needinitializer = false;
	_needdestructor = false;
	_metamethods.resize(MT_LAST); //size it to max size
	if(_base) {
		_base->Lock();
		// NOTE: initializer & destructor will be chained so no copy needed
		_constructoridx = _base->_constructoridx;
		_needinitializer = _base->_needinitializer;
		_needdestructor = _base->_needdestructor;
		_defaultvalues.copy(base->_defaultvalues);
		_methods.copy(base->_methods);
		_getters.copy(base->_getters);
		_setters.copy(base->_setters);
		_metamethods.copy(base->_metamethods);
		__ObjAddRef(_base);
	}
	_members = base?base->_members->Clone() : SQTable::Create(ss,0);
	__ObjAddRef(_members);
	
	INIT_CHAIN();
	ADD_TO_CHAIN(&_sharedstate->_gc_chain, this);

	// member[0] will be always the class name
	NewSlot(ss, ss->_classnameidx, ss->_unnamed, true);
	// member[1] will be always the namespace of the class
	NewSlot(ss, ss->_namespaceidx, ss->_unnamed, true);
}

void SQClass::Finalize() { 
	_attributes = _null_;
	_defaultvalues.resize(0);
	_methods.resize(0);
	_getters.resize(0);
	_setters.resize(0);
	_metamethods.resize(0);
	__ObjRelease(_members);
	if(_base) {
		__ObjRelease(_base);
	}
}

SQClass::~SQClass()
{
	REMOVE_FROM_CHAIN(&_sharedstate->_gc_chain, this);
	Finalize();
}

bool SQClass::NewSlot(SQSharedState *ss,const SQObjectPtr &key,const SQObjectPtr &val,bool bstatic)
{
	SQObjectPtr temp;
	bool belongs_to_static_table = sqi_type(val) == OT_CLOSURE || sqi_type(val) == OT_NATIVECLOSURE || bstatic;
	if(_locked && !belongs_to_static_table) 
		return false; //the class already has an instance so cannot be modified
	GC_MUTATED(this);
	if(_members->Get(key,temp))
	{
		if (_isfield(temp)) //overrides the default value
		{
			_defaultvalues[_member_idx(temp)].val = val;
			return true;
		}
		if(_isproperty(temp))
		{
			return false;
		}
	}
	if(belongs_to_static_table) {
		SQInteger mmidx;
		if((sqi_type(val) == OT_CLOSURE || sqi_type(val) == OT_NATIVECLOSURE) && 
			(mmidx = ss->GetMetaMethodIdxByName(key)) != -1) {
			_metamethods[mmidx] = val;
		} 
		else {
			SQObjectPtr theval = val;
			if(_base && sqi_type(val) == OT_CLOSURE) {
				theval = sqi_closure(val)->Clone();
				sqi_closure(theval)->_base = _base;
				__ObjAddRef(_base); //ref for the closure
			}
			if(sqi_type(temp) == OT_NULL) {
				SQClassMember m;
				m.val = theval;
				mmidx = (SQInteger)_methods.size();
				_members->NewSlot(key,SQObjectPtr(_make_method_idx(mmidx)));
				_methods.push_back(m);
			}
			else {
				mmidx = _member_idx(temp);
				_methods[mmidx].val = theval;
			}

			SQVM* v = sqi_thread(ss->_root_vm);
			bool eq;

			v->IsEqual(ss->_initializeridx, key, eq);
			if(eq) {
				_initializeridx = mmidx;
				_needinitializer = true;
			}

			v->IsEqual(ss->_constructoridx, key, eq);
			if(eq) {
				_constructoridx = mmidx;
			}

			v->IsEqual(ss->_destructoridx, key, eq);
			if(eq) {
				_destructoridx = mmidx;
				_needdestructor = true;
			}
		}
		return true;
	}
	SQClassMember m;
	m.val = val;
	_members->NewSlot(key,SQObjectPtr(_make_field_idx(_defaultvalues.size())));
	_defaultvalues.push_back(m);
	return true;
}

bool SQClass::NewProperty(SQSharedState *ss, const SQObjectPtr &name, const SQObjectPtr &getter, const SQObjectPtr &setter)
{
	if (_locked)
		return false;
	SQObjectPtr temp;
	GC_MUTATED(this);
	if(_members->Get(name,temp))
	{
		if(_isproperty(temp))
		{
			_getters[_member_idx(temp)].val = getter;
			_setters[_member_idx(temp)].val = setter;
			return true;
		}
		return false;
	}
	if (sqi_type(getter) != OT_NULL && sqi_type(getter) != OT_CLOSURE && sqi_type(getter) != OT_NATIVECLOSURE)
		return false;
	if (sqi_type(setter) != OT_NULL && sqi_type(setter) != OT_CLOSURE && sqi_type(setter) != OT_NATIVECLOSURE)
		return false;

	SQObjectPtr gtr = getter;
	SQObjectPtr str = setter;
	if (_base && sqi_type(gtr) == OT_CLOSURE)
	{
		gtr = sqi_closure(getter)->Clone();
		sqi_closure(gtr)->_base = _base;
		__ObjAddRef(_base);
	}
	if (_base && sqi_type(str) == OT_CLOSURE)
	{
		str = sqi_closure(setter)->Clone();
		sqi_closure(str)->_base = _base;
		__ObjAddRef(_base);
	}
	if (sqi_type(temp) == OT_NULL)
	{
		SQClassMember g, s;
		g.val = gtr;
		s.val = str;
		_members->NewSlot(name, SQObjectPtr(_make_property_idx(_getters.size())));
		_getters.push_back(g);
		_setters.push_back(s);
	}
	else
	{
		_getters[_member_idx(temp)].val = gtr;
		_setters[_member_idx(temp)].val = str;
	}

	return true;
}

SQInstance *SQClass::CreateInstance()
{
	if(!_locked) Lock();
	return SQInstance::Create(_opt_ss(this),this);
}

SQInteger SQClass::Next(const SQObjectPtr &refpos, SQObjectPtr &outkey, SQObjectPtr &outval)
{
	SQObjectPtr oval;
	SQInteger idx = _members->Next(false,refpos,outkey,oval);
	if(idx != -1) {
		if(_ismethod(oval)) {
			outval = _methods[_member_idx(oval)].val;
		}
		else if (_isproperty(oval)) {
			outval = _null_;
		}
		else {
			SQObjectPtr &o = _defaultvalues[_member_idx(oval)].val;
			_getrealval(o, NULL, outval);
		}
	}
	return idx;
}

bool SQClass::SetAttributes(const SQObjectPtr &key,const SQObjectPtr &val)
{
	SQObjectPtr idx;
	if(_members->Get(key,idx)) {
		GC_MUTATED(this);
		if(_isfield(idx))
			_defaultvalues[_member_idx(idx)].attrs = val;
		else if(_isproperty(idx))
			_getters[_member_idx(idx)].attrs = val;
		else
			_methods[_member_idx(idx)].attrs = val;
		return true;
	}
	return false;
}

bool SQClass::GetAttributes(const SQObjectPtr &key,SQObjectPtr &outval)
{
	SQObjectPtr idx;
	if(_members->Get(key,idx)) {
		if (_isfield(idx))
			outval = _defaultvalues[_member_idx(idx)].attrs;
		else if (_isproperty(idx))
			outval = _getters[_member_idx(idx)].attrs;
		else
			outval = _methods[_member_idx(idx)].attrs;
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////
void SQInstance::Init(SQSharedState *ss)
{
	_userpointer = NULL;
	_hook = NULL;
	__ObjAddRef(_class);
	_delegate = _class->_members;
	INIT_CHAIN();
	ADD_TO_CHAIN(&_sharedstate->_gc_chain, this);
}

SQInstance::SQInstance(SQSharedState *ss, SQClass *c, SQInteger memsize)
{
	_memsize = memsize;
	_class = c;
	SQUnsignedInteger nvalues = _class->_defaultvalues.size();
	for(SQUnsignedInteger n = 0; n < nvalues; n++) {
		new (&_values[n]) SQObjectPtr(_class->_defaultvalues[n].val);
	}
	Init(ss);
}

SQInstance::SQInstance(SQSharedState *ss, SQInstance *i, SQInteger memsize)
{
	_memsize = memsize;
	_class = i->_class;
	SQUnsignedInteger nvalues = _class->_defaultvalues.size();
	for(SQUnsignedInteger n = 0; n < nvalues; n++) {
		new (&_values[n]) SQObjectPtr(i->_values[n]);
	}
	Init(ss);
}

void SQInstance::Finalize() 
{
	if (IsPurged()) return;
	
	// call destructors if needed
	if (_class->_needdestructor)
	{
		SQVM* v = _sharedstate->GetWorkThread();

		assert(v->_top + MIN_STACK_OVERHEAD <= (SQInteger)v->_stack.size());

		// when the object popped and during Null() -> Release(),
		// subsequent all Push(), Pop() should not affected by this destructor.
		// Avoid the stack slot potentialy Null()ifying
		SQInteger oldTop = v->_top++;

		v->Push(this);
		v->CallDestructorChain(this, _class); // TODO: Investigate side-effects occurred by this Call()
		v->Pop();

		v->_top = oldTop;
	}

	// clean up nvalues
	SQUnsignedInteger nvalues = _class->_defaultvalues.size();
	for(SQUnsignedInteger i = 0; i < nvalues; i++) {
		_values[i] = _null_;
	}

	// release base
	__ObjRelease(_class);
}

void SQInstance::Release()
{
	assert(_uiRef == 0);
	_uiRef++;
	Finalize();
	if (_hook) { _sharedstate->CallReleaseHook(_hook, _userpointer, 0);}
	_uiRef--;
	assert(_uiRef == 0);
	SQInteger size = _memsize;
	this->~SQInstance();
	SQ_FREE(this, size);
}

SQInstance::~SQInstance()
{
	REMOVE_FROM_CHAIN(&_sharedstate->_gc_chain, this);
	assert(_class==NULL);
}

void SQInstance::Purge()
{
	if (IsPurged()) return;
	Finalize();
	if (_hook) { _sharedstate->CallReleaseHook(_hook, _userpointer, 0);}
	_hook = NULL;
	_userpointer = NULL;
}

bool SQInstance::GetMetaMethod(SQVM *v,SQMetaMethod mm,SQObjectPtr &res)
{
	if(IsPurged()) return false;
	if(sqi_type(_class->_metamethods[mm]) != OT_NULL) {
		res = _class->_metamethods[mm];
		return true;
	}
	return false;
}

bool SQInstance::InstanceOf(SQClass *trg)
{
	SQClass *parent = _class;
	while(parent != NULL) {
		if(parent == trg)
			return true;
		parent = parent->_base;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////

void SQNativeItr::Release()
{
	assert(_uiRef == 0);
	_uiRef++;
	if (_itrfunc) { _itrfunc(NULL, _val); }
	_uiRef--;
	assert(_uiRef == 0);
	this->~SQNativeItr();
	SQ_FREE(this, _memsize);
}
