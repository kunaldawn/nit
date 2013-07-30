/*	see copyright notice in squirrel.h */
#ifndef _SQCLASS_H_
#define _SQCLASS_H_

struct SQInstance;

struct SQClassMember {
	SQObjectPtr val;
	SQObjectPtr attrs;
};

typedef sqvector<SQClassMember> SQClassMemberVec;

#define MEMBER_TYPE_METHOD 0x01000000
#define MEMBER_TYPE_FIELD  0x02000000
#define MEMBER_TYPE_PROPERTY 0x04000000

#define _ismethod(o) (sqi_integer(o)&MEMBER_TYPE_METHOD)
#define _isfield(o) (sqi_integer(o)&MEMBER_TYPE_FIELD)
#define _isproperty(o) (sqi_integer(o)&MEMBER_TYPE_PROPERTY)
#define _make_method_idx(i) ((SQInteger)(MEMBER_TYPE_METHOD|i))
#define _make_field_idx(i) ((SQInteger)(MEMBER_TYPE_FIELD|i))
#define _make_property_idx(i) ((SQInteger)(MEMBER_TYPE_PROPERTY|i))
#define _member_type(o) (sqi_integer(o)&0xFF000000)
#define _member_idx(o) (sqi_integer(o)&0x00FFFFFF)

struct SQClass : public CHAINABLE_OBJ
{
	SQClass(SQSharedState *ss,SQClass *base);
public:
	static SQClass* Create(SQSharedState *ss,SQClass *base) {
		SQClass *newclass = (SQClass *)SQ_MALLOC(sizeof(SQClass));
		new (newclass) SQClass(ss, base);
		ss->_classes->push_back(SQObjectPtr(newclass->GetWeakRef(OT_CLASS)));
		return newclass;
	}
	~SQClass();
	bool NewSlot(SQSharedState *ss, const SQObjectPtr &key,const SQObjectPtr &val,bool bstatic);

	inline SQInteger GetIndex(const SQObjectPtr& key)
	{
		SQObjectPtr idx;
		return _members->Get(key, idx) ? sqi_integer(idx) : SQ_ERROR;
	}

	bool Get(const SQObjectPtr &key,SQObjectPtr &val) {
		if(_members->Get(key,val)) {
			if(_isfield(val)) {
				SQObjectPtr &o = _defaultvalues[_member_idx(val)].val;
				_getrealval(o, NULL, val);
			}
			else if (_isproperty(val)) {
				val = _null_;
			}
			else {
				val = _methods[_member_idx(val)].val;
			}
			return true;
		}
		return false;
	}
	bool GetInitializer(SQObjectPtr &init)
	{
		if(_initializeridx != -1) {
			init = _methods[_initializeridx].val;
			return true;
		}
		return false;
	}
	bool GetConstructor(SQObjectPtr &ctor)
	{
		if(_constructoridx != -1) {
			ctor = _methods[_constructoridx].val;
			return true;
		}
		return false;
	}
	bool GetDestructor(SQObjectPtr &dtor)
	{
		if(_destructoridx != -1) {
			dtor = _methods[_destructoridx].val;
			return true;
		}
		return false;
	}
	bool NewProperty(SQSharedState *ss, const SQObjectPtr &name, const SQObjectPtr &getter, const SQObjectPtr &setter);
	bool GetProperty(const SQObjectPtr& name, SQObjectPtr& getter, SQObjectPtr& setter)
	{
		SQObjectPtr val;
		if (_members->Get(name, val) && _isproperty(val)) {
			getter = _getters[_member_idx(val)].val;
			setter = _setters[_member_idx(val)].val;
			return true;
		}
		return false;
	}
	bool SetAttributes(const SQObjectPtr &key,const SQObjectPtr &val);
	bool GetAttributes(const SQObjectPtr &key,SQObjectPtr &outval);
	void Lock() { if (!_locked) { _locked = true; if(_base) _base->Lock(); } }
	void Release() { 
		if (_hook) { _sharedstate->CallReleaseHook(_hook, _typetag, 0);}
		sq_delete(this, SQClass);	
	}
	void Finalize();
#ifndef NO_GARBAGE_COLLECTOR
	void Mark(SQGC* chain);
#endif
	SQInteger Next(const SQObjectPtr &refpos, SQObjectPtr &outkey, SQObjectPtr &outval);
	SQInstance *CreateInstance();
	SQTable *_members;
	SQClass *_base;
	SQClassMemberVec _defaultvalues;
	SQClassMemberVec _methods;
	SQClassMemberVec _getters;
	SQClassMemberVec _setters;
	SQObjectPtrVec _metamethods;
	SQObjectPtr _attributes;
	SQUserPointer _typetag;
	SQRELEASEHOOK _hook;
	bool _locked;
	SQInteger _initializeridx;
	SQInteger _constructoridx;
	SQInteger _destructoridx;
	SQBool _needinitializer;
	SQBool _needdestructor;
	SQInteger _udsize;
};

#define calcinstancesize(_theclass_) \
	(_theclass_->_udsize + sizeof(SQInstance) + (sizeof(SQObjectPtr)*(_theclass_->_defaultvalues.size()>0?_theclass_->_defaultvalues.size()-1:0)))

struct SQInstance : public SQDelegable 
{
	void Init(SQSharedState *ss);
	SQInstance(SQSharedState *ss, SQClass *c, SQInteger memsize);
	SQInstance(SQSharedState *ss, SQInstance *c, SQInteger memsize);
public:
	static SQInstance* Create(SQSharedState *ss,SQClass *theclass) {
		
		SQInteger size = calcinstancesize(theclass);
		SQInstance *newinst = (SQInstance *)SQ_MALLOC(size);
		new (newinst) SQInstance(ss, theclass,size);
		if(theclass->_udsize) {
			newinst->_userpointer = ((unsigned char *)newinst) + (size - theclass->_udsize);
		}
		return newinst;
	}
	SQInstance *Clone(SQSharedState *ss)
	{
		SQInteger size = calcinstancesize(_class);
		SQInstance *newinst = (SQInstance *)SQ_MALLOC(size);
		new (newinst) SQInstance(ss, this,size);
		if(_class->_udsize) {
			newinst->_userpointer = ((unsigned char *)newinst) + (size - _class->_udsize);
		}
		return newinst;
	}
	~SQInstance();
	enum AccessResult { IA_OK, IA_NO_INDEX, IA_ERROR };

	enum Flags 
	{ 
		IS_INITIALIZING = 0x0001, 
		IS_CONSTRUCTING = 0x0002, 
		IS_DESTRUCTING = 0x0004
	};

	bool isValid() {
		if (_class == NULL) return false;
		if (_class->_typetag && _userpointer == NULL) return false;
		return true;
	}

	AccessResult GetByIndex(SQVM* vm, SQInteger index, SQObjectPtr& val)
	{
		if (index & MEMBER_TYPE_FIELD) 
		{
			SQObjectPtr &o = _values[index & 0x00FFFFFF];
			_getrealval(o, vm, val);
			return IA_OK;
		}
		else if (index & MEMBER_TYPE_PROPERTY) 
		{
			SQObjectPtr getter = _class->_getters[index & 0x00FFFFFF].val;
			if (sqi_type(getter) == OT_NULL) { vm->Raise_Error("non-readable property"); return IA_ERROR; }
			vm->Push(SQObjectPtr(this));
			if (vm->Call(getter, 1, vm->_top-1, val, SQTrue)) {
				vm->Pop();
				return IA_OK;
			}
			else return IA_ERROR;
		}
		else if (index & MEMBER_TYPE_METHOD) 
		{
			val = _class->_methods[index & 0x00FFFFFF].val;
			return IA_OK;
		}
		return IA_NO_INDEX;
	}

	AccessResult SetByIndex(SQVM* vm, SQInteger index, const SQObjectPtr& val)
	{
		if (index & MEMBER_TYPE_FIELD) 
		{
			_values[index & 0x00FFFFFF] = val;
			GC_MUTATED(this);
			return IA_OK;
		}
		else if (index & MEMBER_TYPE_PROPERTY) 
		{
			SQObjectPtr setter = _class->_setters[index & 0x00FFFFFF].val;
			if (sqi_type(setter) == OT_NULL) { vm->Raise_Error("non-writable property"); return IA_ERROR; }
			vm->Push(SQObjectPtr(this)); vm->Push(val);
			SQObjectPtr t;
			if (vm->Call(setter, 2, vm->_top-2, t, SQTrue)) 
			{
				GC_MUTATED(this);
				vm->Pop(2);
				return IA_OK;
			}
			else return IA_ERROR;
		}
		return IA_NO_INDEX;
	}

	inline AccessResult Get(SQVM* vm, const SQObjectPtr &key,SQObjectPtr &val)  {
		if (IsPurged()) { vm->Raise_Error("can't access purged instance"); return IA_ERROR; }

		return _class->_members->Get(key,val) ? GetByIndex(vm, sqi_integer(val), val) : IA_NO_INDEX;
	}

	inline AccessResult Set(SQVM* vm, const SQObjectPtr &key,const SQObjectPtr &val) {
		if (IsPurged()) { vm->Raise_Error("can't access purged instance"); return IA_ERROR; }

		SQObjectPtr idx;
		return _class->_members->Get(key,idx) ? SetByIndex(vm, sqi_integer(idx), val) : IA_NO_INDEX;
	}

	inline bool IsPurged() { return _class == NULL; }

	void Purge();
	void Release();
	void Finalize();
#ifndef NO_GARBAGE_COLLECTOR 
	void Mark(SQGC*);
#endif
	bool InstanceOf(SQClass *trg);
	bool GetMetaMethod(SQVM *v,SQMetaMethod mm,SQObjectPtr &res);

	SQClass *_class;
	SQUserPointer _userpointer;
	SQRELEASEHOOK _hook;
	SQInteger _memsize;
	SQInteger _flags;
	SQObjectPtr _values[1];
};

struct SQNativeItr : public SQRefCounted
{
public:
	static SQNativeItr* Create(SQUnsignedInteger memsize, SQ_NATIVEITR_FUNC itrfunc)
	{
		memsize = sizeof(SQNativeItr) + (memsize-1);
		SQNativeItr* itr = (SQNativeItr*)SQ_MALLOC(memsize);
		new (itr) SQNativeItr(itrfunc, memsize);
		itr->_memsize = memsize;
		return itr;
	}

	SQ_NATIVEITR_FUNC					_itrfunc;
	SQUnsignedInteger					_memsize;
	SQChar								_val[1];

	void Release();

private:
	SQNativeItr(SQ_NATIVEITR_FUNC func, SQUnsignedInteger memsize): _itrfunc(func), _memsize(memsize) { }
};

#endif //_SQCLASS_H_
