/*	see copyright notice in squirrel.h */
#ifndef _SQOBJECT_H_
#define _SQOBJECT_H_

#include "squtils.h"

#ifdef _SQ64
#define UINT_MINUS_ONE (0xFFFFFFFFFFFFFFFF)
#else
#define UINT_MINUS_ONE (0xFFFFFFFF)
#endif

#define SQ_CLOSURESTREAM_HEAD (('S'<<24)|('Q'<<16)|('I'<<8)|('R'))
#define SQ_CLOSURESTREAM_PART (('P'<<24)|('A'<<16)|('R'<<8)|('T'))
#define SQ_CLOSURESTREAM_TAIL (('T'<<24)|('A'<<16)|('I'<<8)|('L'))

struct SQSharedState;

enum SQMetaMethod{
	MT_ADD=0,
	MT_SUB=1,
	MT_MUL=2,
	MT_DIV=3,
	MT_UNM=4,
	MT_MODULO=5,
	MT_SET=6,
	MT_GET=7,
	MT_TYPEOF=8,
	MT_NEXTI=9,
	MT_CMP=10,
	MT_CALL=11,
	MT_CLONED=12,
	MT_NEWSLOT=13,
	MT_DELSLOT=14,
	MT_TOSTRING=15,
	MT_NEWMEMBER=16,
	MT_INHERITED=17,
	MT_ASSIGN=18,
	MT_WEAKREF=19,
	MT_LAST = 20
};

#define MM_ADD		_SC("_add")
#define MM_SUB		_SC("_sub")
#define MM_MUL		_SC("_mul")
#define MM_DIV		_SC("_div")
#define MM_UNM		_SC("_unm")
#define MM_MODULO	_SC("_modulo")
#define MM_SET		_SC("_set")
#define MM_GET		_SC("_get")
#define MM_TYPEOF	_SC("_typeof")
#define MM_NEXTI	_SC("_nexti")
#define MM_CMP		_SC("_cmp")
#define MM_CALL		_SC("_call")
#define MM_CLONED	_SC("_cloned")
#define MM_NEWSLOT	_SC("_newslot")
#define MM_DELSLOT	_SC("_delslot")
#define MM_TOSTRING	_SC("_tostring")
#define MM_NEWMEMBER _SC("_newmember")
#define MM_INHERITED _SC("_inherited")
#define MM_ASSIGN	_SC("_assign")
#define MM_WEAKREF	_SC("_weakref")

#define MINPOWER2 4

struct SQRefCounted
{
	SQUnsignedInteger _uiRef;
	struct SQWeakRef *_weakref;
	SQRefCounted() { _uiRef = 0; _weakref = NULL; }
	virtual ~SQRefCounted();
	SQWeakRef *GetWeakRef(SQObjectType type);
	virtual void Release()=0;
};

struct SQWeakRef : SQRefCounted
{
	void Release();
	SQObject _obj;
};

struct SQObjectPtr;

struct SQNativeWeakRef : SQRefCounted
{
public:
	typedef SQRESULT (*GetRefCallback) (HSQUIRRELVM v, SQUserPointer, SQObjectPtr& outValue);

	static SQNativeWeakRef* Create(SQUserPointer up, GetRefCallback callback, SQSharedState* ss)
	{
		SQNativeWeakRef* wr = (SQNativeWeakRef*)SQ_MALLOC(sizeof(SQNativeWeakRef));
		new (wr) SQNativeWeakRef(up, callback, ss);
		return wr;
	}

	SQRESULT GetRef(SQVM* v, SQObjectPtr& outValue) 
	{ 
		if (v == NULL) 
			v = GetWorkerThread();

		return _callback(v, _userpointer, outValue); 
	}

	void Release();

	SQUserPointer _userpointer;
	SQSharedState*	_sharedstate;
	GetRefCallback _callback;
	SQRELEASEHOOK _hook;

private:
	SQVM* GetWorkerThread();
	SQNativeWeakRef(SQUserPointer up, GetRefCallback callback, SQSharedState* ss)
		: _userpointer(up), _sharedstate(ss), _callback(callback), _hook(NULL) { }
};

#define _getrealval(o, v, out) \
	if (!SQ_ISWEAKREF((o)._type)) out = (o); \
	else if ((o)._type == OT_WEAKREF) out = sqi_weakref(o)->_obj; \
	else sqi_nativeweakref(o)->GetRef(v, out); 

#define __AddRef(sqi_type,unval) if(SQ_ISREFCOUNTED(sqi_type))	\
		{ \
			unval.pRefCounted->_uiRef++; \
		}  

#define __Release(sqi_type,unval) if(SQ_ISREFCOUNTED(sqi_type) && ((--unval.pRefCounted->_uiRef)==0))	\
		{	\
			unval.pRefCounted->Release();	\
		}

#define __ObjRelease(obj) { \
	if ((obj)) {	\
		if ( --((obj)->_uiRef) == 0) \
			(obj)->Release(); \
		(obj) = NULL;	\
	} \
}

#define __ObjAddRef(obj) { \
	(obj)->_uiRef++; \
}

#define sqi_type(obj) ((obj)._type)
#define sqi_isdelegable(t) (sqi_type(t)&SQOBJECT_DELEGABLE)
#define sqi_raw_type(obj) _RAW_TYPE((obj)._type)

#define sqi_integer(obj) ((obj)._unVal.nInteger)
#define sqi_float(obj) ((obj)._unVal.fFloat)
#define sqi_string(obj) ((obj)._unVal.pString)
#define sqi_table(obj) ((obj)._unVal.pTable)
#define sqi_array(obj) ((obj)._unVal.pArray)
#define sqi_closure(obj) ((obj)._unVal.pClosure)
#define sqi_generator(obj) ((obj)._unVal.pGenerator)
#define sqi_nativeclosure(obj) ((obj)._unVal.pNativeClosure)
#define sqi_userdata(obj) ((obj)._unVal.pUserData)
#define sqi_userpointer(obj) ((obj)._unVal.pUserPointer)
#define sqi_thread(obj) ((obj)._unVal.pThread)
#define sqi_funcproto(obj) ((obj)._unVal.pFunctionProto)
#define sqi_class(obj) ((obj)._unVal.pClass)
#define sqi_instance(obj) ((obj)._unVal.pInstance)
#define sqi_delegable(obj) ((SQDelegable *)(obj)._unVal.pDelegable)
#define sqi_weakref(obj) ((obj)._unVal.pWeakRef)
#define sqi_nativeweakref(obj) ((obj)._unVal.pNativeWeakRef)
#define sqi_outer(obj) ((obj)._unVal.pOuter)
#define sqi_nativeitr(obj) ((obj)._unVal.pNativeItr)
#define sqi_refcounted(obj) ((obj)._unVal.pRefCounted)
#define sqi_rawval(obj) ((obj)._unVal.raw)

#define sqi_stringval(obj) (obj)._unVal.pString->_val
#define sqi_userdataval(obj) (obj)._unVal.pUserData->_val

#define sqi_tofloat(num) ((sqi_type(num)==OT_INTEGER)?(SQFloat)sqi_integer(num):sqi_float(num))
#define sqi_tointeger(num) ((sqi_type(num)==OT_FLOAT)?(SQInteger)sqi_float(num):sqi_integer(num))
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
#define _REF_TYPE_DECL(sqi_type,sqi_class,sym) \
	SQObjectPtr(sqi_class * x) \
	{ \
		SQ_OBJECT_RAWINIT() \
		_type=sqi_type; \
		_unVal.sym = x; \
		assert(_unVal.pTable); \
		_unVal.pRefCounted->_uiRef++; \
	} \
	inline SQObjectPtr& operator=(sqi_class *x) \
	{  \
		SQObjectType tOldType = _type; \
		SQObjectValue unOldVal = _unVal; \
		_type = sqi_type; \
		_unVal.sym = x; \
		_unVal.pRefCounted->_uiRef++; \
		__Release(tOldType,unOldVal); \
		return *this; \
	}

#define _SCALAR_TYPE_DECL(sqi_type,sqi_class,sym) \
	SQObjectPtr(sqi_class x) \
	{ \
		SQ_OBJECT_RAWINIT() \
		_type=sqi_type; \
		_unVal.sym = x; \
	} \
	inline SQObjectPtr& operator=(sqi_class x) \
	{  \
		SQObjectType tOldType = _type; \
		SQObjectValue unOldVal = _unVal; \
		_type = sqi_type; \
		_unVal.sym = x; \
		__Release(tOldType,unOldVal); \
		return *this; \
	}
struct SQObjectPtr : public SQObject
{
	SQObjectPtr()
	{
		SQ_OBJECT_RAWINIT()
		_type=OT_NULL;
		_unVal.pUserPointer=NULL;
	}
	SQObjectPtr(const SQObjectPtr &o)
	{
		//SQ_OBJECT_RAWINIT()
		_type=o._type;
		_unVal=o._unVal;
		__AddRef(_type,_unVal);
	}
	SQObjectPtr(const SQObject &o)
	{
		//SQ_OBJECT_RAWINIT()
		_type=o._type;
		_unVal=o._unVal;
		__AddRef(_type,_unVal);
	}
	_REF_TYPE_DECL(OT_TABLE,SQTable,pTable)
	_REF_TYPE_DECL(OT_CLASS,SQClass,pClass)
	_REF_TYPE_DECL(OT_INSTANCE,SQInstance,pInstance)
	_REF_TYPE_DECL(OT_ARRAY,SQArray,pArray)
	_REF_TYPE_DECL(OT_CLOSURE,SQClosure,pClosure)
	_REF_TYPE_DECL(OT_NATIVECLOSURE,SQNativeClosure,pNativeClosure)
	_REF_TYPE_DECL(OT_OUTER,SQOuter,pOuter)
	_REF_TYPE_DECL(OT_GENERATOR,SQGenerator,pGenerator)
	_REF_TYPE_DECL(OT_STRING,SQString,pString)
	_REF_TYPE_DECL(OT_USERDATA,SQUserData,pUserData)
	_REF_TYPE_DECL(OT_WEAKREF,SQWeakRef,pWeakRef)
	_REF_TYPE_DECL(OT_NATIVEWEAKREF,SQNativeWeakRef,pNativeWeakRef);
	_REF_TYPE_DECL(OT_THREAD,SQVM,pThread)
	_REF_TYPE_DECL(OT_FUNCPROTO,SQFunctionProto,pFunctionProto)
	_REF_TYPE_DECL(OT_NATIVE_ITR,SQNativeItr,pNativeItr)
	
	_SCALAR_TYPE_DECL(OT_INTEGER,SQInteger,nInteger)
	_SCALAR_TYPE_DECL(OT_FLOAT,SQFloat,fFloat)
	_SCALAR_TYPE_DECL(OT_USERPOINTER,SQUserPointer,pUserPointer)

	SQObjectPtr(bool bBool)
	{
		SQ_OBJECT_RAWINIT()
		_type = OT_BOOL;
		_unVal.nInteger = bBool?1:0;
	}
	inline SQObjectPtr& operator=(bool b)
	{ 
		SQObjectType tOldType = _type;
		SQObjectValue unOldVal = _unVal;
		_type = OT_BOOL;
		_unVal.nInteger = b?1:0;
		__Release(tOldType,unOldVal);
		return *this;
	}

	~SQObjectPtr()
	{
		SQObjectType tOldType = _type;
		SQObjectValue unOldVal = _unVal;
		_type = OT_NULL;
		_unVal.pUserPointer = NULL;
		__Release(tOldType,unOldVal);
	}
			
	inline SQObjectPtr& operator=(const SQObjectPtr& obj)
	{ 
		SQObjectType tOldType = _type;
		SQObjectValue unOldVal = _unVal;
		_type = obj._type;
		_unVal = obj._unVal;
		__AddRef(_type,_unVal);
		__Release(tOldType,unOldVal);
		return *this;
	}
	inline SQObjectPtr& operator=(const SQObject& obj)
	{ 
		SQObjectType tOldType = _type;
		SQObjectValue unOldVal = _unVal;
		_type = obj._type;
		_unVal = obj._unVal;
		__AddRef(_type,_unVal);
		__Release(tOldType,unOldVal);
		return *this;
	}
	inline void Null()
	{
		SQObjectType tOldType = _type;
		SQObjectValue unOldVal = _unVal;
		_type = OT_NULL;
		_unVal.pUserPointer = NULL;
		__Release(tOldType,unOldVal);
	}
	private:
		SQObjectPtr(const SQChar *){} //safety
};

/////////////////////////////////////////////////////////////////////////////////////

#include "sqgc.h"

struct SQDelegable : public CHAINABLE_OBJ {
	bool SetDelegate(SQTable *m);
	virtual bool GetMetaMethod(SQVM *v,SQMetaMethod mm,SQObjectPtr &res);
	SQTable *_delegate;
};

SQUnsignedInteger TranslateIndex(const SQObjectPtr &idx);
typedef sqvector<SQObjectPtr> SQObjectPtrVec;
typedef sqvector<SQInteger> SQIntVec;
const SQChar *GetTypeName(const SQObjectPtr &obj1);
const SQChar *IdType2Name(SQObjectType type);



#endif //_SQOBJECT_H_
