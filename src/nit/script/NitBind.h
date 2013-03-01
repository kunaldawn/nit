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

#pragma once

#include "nit/nit.h"
#include "nit/event/Event.h"

#include "squirrel/squirrel.h"
#include "squirrel/sqxapi.h"
#include <string>
#include <set>

////////////////////////////////////////////////////////////////////////////////

#define NIT_LIB_ENTRY(LIBNAME, REQUIRES) ::nit::NitBindLibRegistry(LIBNAME, #LIBNAME, REQUIRES)

////////////////////////////////////////////////////////////////////////////////

struct SQObjectPtr;

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

using std::type_info;
using ::HSQUIRRELVM;

#ifdef _WX_WX_H_
wxString NitWxGetString(HSQUIRRELVM v, SQInteger idx);
wxString NitWxOptString(HSQUIRRELVM v, SQInteger idx, const wxString& def);
SQInteger NitWxPushString(HSQUIRRELVM v, const wxString& value);
#endif

struct NIT_API SQObjectRef : public SQObject
{
	SQObjectRef()
	{
		_type = OT_NULL;
		_unVal.pUserPointer = NULL;
	}

	~SQObjectRef();

	SQObjectRef(const SQObjectRef& o);
	SQObjectRef(const SQObject& o);

	SQObjectRef& operator= (const SQObjectRef& o);
	SQObjectRef& operator= (const SQObject& o);

	void push(HSQUIRRELVM v);
	void top(HSQUIRRELVM v);
	void get(HSQUIRRELVM v, int idx);

	SQUserPointer getInstanceUp(SQUserPointer typeTag);
};

class NIT_API NitBindImpl
{
protected:
	struct PropEntry
	{
		const char*						name;
		SQFUNCTION						getter;
		SQFUNCTION						setter;
		const char*						file;
	};

	struct FuncEntry
	{
		const char*						name;
		SQFUNCTION						func;
		const char*						file;
		const char*						help;
	};

	typedef SQInteger (*CONS_HOOK) (HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p, SQInteger size);
	typedef void* (*INTF_CAST_FN) (SQUserPointer p);

	template <class TClass>
	struct TypeInfo
	{
	public:
		// TODO: Following functions have no chance to be inlined and called every PushDynamic() call.
		// Find some way to retrieve in batch.
		static const SQChar*			bindingName();			// The name string specified NB_TYPE_XXX macro
		static const SQChar*			typeCode();				// typeid(TClass).name()
		static const SQChar*			typeCode(TClass* ptr);	// typeid(*ptr).name()
		static const SQChar*			baseTypeCode();			// typeid.name(TBaseClass).name()
		static SQUserPointer			typeTag();				// &typeid(TClass)
		static SQUserPointer			typeTag(TClass* ptr);	// &typeid(*ptr)

		static bool						isPtr();
		static size_t					classUserDataSize();

		static SQUserPointer			toUserPtr(TClass* ptr);
		static TClass*					toClassPtr(SQUserPointer ptr);
		static SQInteger				constructHook(HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p, SQInteger size);
		static SQInteger				postConsHook(HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p, SQInteger size);
		static SQInteger				releaseHook(HSQUIRRELVM v, SQUserPointer p, SQInteger size);
		static void						defaultConstructor(TClass* ptr);
	};

	class BindEnv
	{
	public:
		BindEnv(void* userenv);
		~BindEnv();

		void							install(HSQUIRRELVM v);
		void							uninstall();

	public:
		void							addCachedPtr(HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p);
		SQRESULT						getCachedInstance(HSQUIRRELVM v, SQUserPointer p, SQUserPointer typeTag, SQObjectRef& outInstance);
		SQRESULT						getCachedInstance(HSQUIRRELVM v, SQUserPointer p, SQUserPointer typeTag);
		void							removeCachedPtr(SQUserPointer p);
		void							processCacheRemoval();

	public:
		HSQUIRRELVM						_rootVm;
		void*							_userEnv;

		HSQOBJECT						_instanceCache;
		vector<SQUserPointer>::type		_removalQueue;
	};


public:
	static bool							install(HSQUIRRELVM v, void* userenv);
	static void							uninstall(HSQUIRRELVM v);

	static inline BindEnv*				getBindEnv(HSQUIRRELVM v)								{ return (BindEnv*)sq_getforeignptr(v); }
	static inline HSQUIRRELVM			getRootThread(HSQUIRRELVM v)							{ return getBindEnv(v)->_rootVm; }
	static inline void*					getUserEnv(HSQUIRRELVM v)								{ return getBindEnv(v)->_userEnv; }
	static inline int					toAbsIdx(HSQUIRRELVM v, int idx)						{ return idx > 0 ? idx : sq_gettop(v) + idx + 1; }

	static SQInteger					getRegistryTable(HSQUIRRELVM v)							{ sq_pushregistrytable(v); return 1; }

	static void							addCachedPtr(HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p)	{ getBindEnv(v)->addCachedPtr(v, inst, p); }
	static void							removeCachedPtr(HSQUIRRELVM v, SQUserPointer p)					{ getBindEnv(v)->removeCachedPtr(p); }
	static SQRESULT						getCachedInstance(HSQUIRRELVM v, SQUserPointer p, SQUserPointer tt, SQObjectRef& o) { return getBindEnv(v)->getCachedInstance(v, p, tt, o); }
	static SQRESULT						getCachedInstance(HSQUIRRELVM v, SQUserPointer p, SQUserPointer tt)					{ return getBindEnv(v)->getCachedInstance(v, p, tt); }
	static SQInteger					getCacheTable(HSQUIRRELVM v)									{ sq_pushobject(v, getBindEnv(v)->_instanceCache); return 1; }

	static String						typeName(const SQChar* typecode);
	static void							createNamespace(HSQUIRRELVM v, const char* fullname);

protected:
	static void							_bind(HSQUIRRELVM v, PropEntry* props, FuncEntry* funcs, const SQChar* bindingName, const char* typeCode, SQUserPointer typeTag, const char* baseTypeCode, bool isPtr, size_t classudSize, CONS_HOOK consHook, CONS_HOOK postConsHook, SQRELEASEHOOK releaseHook);
	static void							_bindMore(HSQUIRRELVM v, const SQChar* bindingName, PropEntry* props, FuncEntry* funcs);
	static void							_addInterface(HSQUIRRELVM v, const char* typeCode, const SQChar* intfTypeCode, SQUserPointer intfTypeTag, INTF_CAST_FN castFn);
	static bool							_hasInterface(HSQUIRRELVM v, SQInteger clsIdx, const type_info& ti);
	static void*						_getInterfacePtr(HSQUIRRELVM v, SQInteger idx, const type_info& ti);

	static bool							newFunction(HSQUIRRELVM v, const char* classname, const char* name, SQFUNCTION fn, const char* file, const char* help, SQInteger idx, bool isstatic);

	static int							_bindPtrInstance(HSQUIRRELVM v, SQObjectRef& outInstance, SQUserPointer ptr, SQUserPointer actualTypetag, SQUserPointer typeTag, const SQChar* bindingName, const SQChar* typeCode, const SQChar* actualTypeCode, CONS_HOOK conshook, SQRELEASEHOOK relhook);
	static int							_bindValueInstance(HSQUIRRELVM v, SQObjectRef& outInstance, SQUserPointer typeTag, const char* bindingName, CONS_HOOK conshook, SQRELEASEHOOK relhook);

	static void							_newSlot(HSQUIRRELVM v, SQInteger tblIdx, bool isStatic);

	static SQInteger					_dispatchConstructor(HSQUIRRELVM v);
	static SQInteger					_nonConstructable(HSQUIRRELVM v);
	static SQInteger					_nonInheritable(HSQUIRRELVM v);

	static SQInteger					_doInstall(HSQUIRRELVM v);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API NitBindLibRegistry
{
	typedef int (*RegisterFn) (HSQUIRRELVM v);
public:
	NitBindLibRegistry(RegisterFn fn, const char* libName, const char* dependancy);
	~NitBindLibRegistry();

	static bool							install(HSQUIRRELVM v, NitBindLibRegistry* r);

private:
	friend class						NitBindImpl;

	RegisterFn							_register;
	const char*							_libName;
	const char*							_dependancy;
	NitBindLibRegistry*					_next;

	static NitBindLibRegistry*&			getHead();
	static NitBindLibRegistry*			add(NitBindLibRegistry* r);
	static void							remove(NitBindLibRegistry* r);
	static NitBindLibRegistry*			find(const char* libName, unsigned int nameLen);
};

////////////////////////////////////////////////////////////////////////////////

class NitBindImpl;

template <typename TImpl = NitBindImpl>
class TNitBind : public TImpl
{
public:
	template <class TClass>
	static SQBool						is(HSQUIRRELVM v, SQInteger idx)						{ return sqx_instanceof(v, idx, TImpl::template TypeInfo<TClass>::typeTag()); }
	
	static bool							isNone(HSQUIRRELVM v, SQInteger idx)					{ return sq_gettop(v) < idx; }
	static bool							isNull(HSQUIRRELVM v, SQInteger idx)					{ return sq_gettype(v, idx) == OT_NULL; }
	static bool							isInt(HSQUIRRELVM v, SQInteger idx)						{ return sq_gettype(v, idx) == OT_INTEGER; }
	static bool							isFloat(HSQUIRRELVM v, SQInteger idx)					{ return sq_gettype(v, idx) == OT_FLOAT; }
	static bool							isNumber(HSQUIRRELVM v, SQInteger idx)					{ return isInt(v, idx) || isFloat(v, idx); }
	static bool							isBool(HSQUIRRELVM v, SQInteger idx)					{ return sq_gettype(v, idx) == OT_BOOL; }
	static bool							isString(HSQUIRRELVM v, SQInteger idx)					{ return sq_gettype(v, idx) == OT_STRING; }
	
	template <class TClass>
	static TClass*						opt(HSQUIRRELVM v, SQInteger idx, TClass* def)			{ return isNone(v, idx) || isNull(v, idx) ? def : get<TClass>(v, idx); }
	template <class TClass>
	static const TClass*				opt(HSQUIRRELVM v, SQInteger idx, const TClass& def)	{ return isNone(v, idx) || isNull(v, idx) ? &def : get<TClass>(v, idx); }
	static bool							optBool(HSQUIRRELVM v, SQInteger idx, bool def)			{ return isNone(v, idx) || isNull(v, idx) ? def : (sqx_getbool(v, idx) != 0); }
	static int							optInt(HSQUIRRELVM v, SQInteger idx, int def)			{ return isNone(v, idx) || isNull(v, idx) ? def : sqx_getinteger(v, idx); }
	static float						optFloat(HSQUIRRELVM v, SQInteger idx, float def)		{ return isNone(v, idx) || isNull(v, idx) ? def : sqx_getfloat(v, idx); }
	static const char*					optString(HSQUIRRELVM v, SQInteger idx, const char* def){ return isNone(v, idx) || isNull(v, idx) ? def : sqx_getstring(v, idx); }
	
	template <class TClass>
	static TClass*						get(HSQUIRRELVM v, SQInteger idx);
	
	static int							getInt(HSQUIRRELVM v, SQInteger idx)					{ return sqx_getinteger(v, idx); }
	static float						getFloat(HSQUIRRELVM v, SQInteger idx)					{ return sqx_getfloat(v, idx); }
	static bool							getBool(HSQUIRRELVM v, SQInteger idx)					{ return sqx_getbool(v, idx) != 0; }
	static const char*					getString(HSQUIRRELVM v, SQInteger idx)					{ return sqx_getstring(v, idx); }
	
	static int							onlyInt(HSQUIRRELVM v, SQInteger idx)					{ return sqx_getonlyinteger(v, idx); }
	static float						onlyFloat(HSQUIRRELVM v, SQInteger idx)					{ return sqx_getonlyfloat(v, idx); }

	template <class TInterface>
	static bool							hasInterface(HSQUIRRELVM v, SQInteger clsIdx);
	
	template <class TInterface>
	static TInterface*					getInterface(HSQUIRRELVM v, SQInteger idx, bool allowNull = false);

	template <class TClass>
	static SQRESULT						bindInstance(HSQUIRRELVM v, TClass* ptr, SQObjectRef& outInstance);

	template <class TClass>
	static SQRESULT						bindInstance(HSQUIRRELVM v, const TClass& value, SQObjectRef& outInstance);
	
	template <class TClass>
	static int							push(HSQUIRRELVM v, TClass* ptr);
	
	template <class TClass>
	static int							push(HSQUIRRELVM v, const TClass& value);
	
	static int							pushNull(HSQUIRRELVM v)									{ sq_pushnull(v); return 1; }
	static int							push(HSQUIRRELVM v, int value)							{ sq_pushinteger(v, value); return 1; }
	static int							push(HSQUIRRELVM v, uint value)							{ sq_pushinteger(v, (int)value); return 1; }
	static int							push(HSQUIRRELVM v, long value)							{ sq_pushinteger(v, (int)value); return 1; }
	static int							push(HSQUIRRELVM v, unsigned long value)				{ sq_pushinteger(v, (int)value); return 1; }
	static int							push(HSQUIRRELVM v, short value)						{ sq_pushinteger(v, (int)value); return 1; }
	static int							push(HSQUIRRELVM v, ushort value)						{ sq_pushinteger(v, (int)value); return 1; }
	static int							push(HSQUIRRELVM v, float value)						{ sq_pushfloat(v, value); return 1; }
	static int							push(HSQUIRRELVM v, double value)						{ sq_pushfloat(v, (float)value); return 1; }
	static int							push(HSQUIRRELVM v, bool value)							{ sq_pushbool(v, value); return 1; }
	static int							push(HSQUIRRELVM v, const char* value, int len = -1)	{ sq_pushstring(v, value, len); return 1; }
	static int							push(HSQUIRRELVM v, const String& value)				{ sq_pushstring(v, value.c_str(), value.size()); return 1; }
	static int 							pushFmt(HSQUIRRELVM v, const char* fmt, ...);

	template <typename EvtClass>
	static int							push(HSQUIRRELVM v, const EventType<EvtClass>& e)		{ return push(v, const_cast<EventInfo*>(e.getInfo())); }
	
#ifdef _WX_WX_H_
	static inline wxString				getWxString(HSQUIRRELVM v, SQInteger idx)				{ return NitWxGetString(v, idx); }
	static inline wxString				optWxString(HSQUIRRELVM v, SQInteger idx, const wxString& def) { return NitWxOptString(v, idx, def); }
	static inline int					push(HSQUIRRELVM v, const wxString& value)				{ return NitWxPushString(v, value); }
#endif
	
	template <class TKeyClass, class TValueClass>
	static void newSlot(HSQUIRRELVM v, int idx, const TKeyClass& key, const TValueClass& value, bool isstatic = false)	{ push(v, key); push(v, value); TImpl::_newSlot(v, idx >= 0 ? idx : idx - 2, isstatic); }
	
	template <class TClass>
	static void arrayAppend(HSQUIRRELVM v, int idx, const TClass& value)						{ push(v, value); sq_arrayappend(v, idx >= 0 ? idx : idx - 1); }
	
	template <class TClass>
	static int							pushClass(HSQUIRRELVM v);
	
	template <class TClass>
	static bool							purge(HSQUIRRELVM v, TClass* ptr);
};

typedef TNitBind<NitBindImpl> NitBind;

////////////////////////////////////////////////////////////////////////////////

template <typename TClass, typename TNitBind = NitBind >
class TNitClass : public TNitBind
{
public:
	typedef TClass type;
	typedef TClass* ptrtype;
	typedef typename TNitBind::template TypeInfo<type> typeinfo;
	typedef typename TNitBind::PropEntry PropEntry;
	typedef typename TNitBind::FuncEntry FuncEntry;
	typedef TNitBind inherited;

	static void bind(HSQUIRRELVM v, PropEntry* props, FuncEntry* funcs)
	{
		TNitBind::_bind(
			v, 
			props, 
			funcs, 
			typeinfo::bindingName(),
			typeinfo::typeCode(),
			typeinfo::typeTag(), 
			typeinfo::baseTypeCode(), 
			typeinfo::isPtr(),
			typeinfo::classUserDataSize(),
			typeinfo::constructHook,
			typeinfo::postConsHook,
			typeinfo::releaseHook);
	}

	static void bindMore(HSQUIRRELVM v, PropEntry* props, FuncEntry* funcs)
	{
		const SQChar* bindingName = typeinfo::bindingName();
		TNitBind::template pushClass<TClass>(v);
		TNitBind::_bindMore(v, bindingName, props, funcs);
		sq_poptop(v);
	}

	template <class TInterface>
	static void addInterface(HSQUIRRELVM v)
	{
		TClass* p = NULL; TInterface* i = p; // NOTE: this is a compiler assert. If you get an error here, then TClass doesn't support TInterface.

		SQUserPointer intfTypeTag = (SQUserPointer)&typeid(TInterface);
		const SQChar* intfTypeCode = typeid(TInterface).name();
		typename TNitBind::INTF_CAST_FN castFn = _castInterface<TInterface>;

		TNitBind::_addInterface(v, typeinfo::typeCode(), intfTypeCode, intfTypeTag, castFn);
	}

	static void addStaticTable(HSQUIRRELVM v, const char* tblName, bool cloneIfExists = false, bool useDelegate = true)
	{
		TNitBind::template pushClass<TClass>(v);
		sq_pushstring(v, tblName, -1);
		if (SQ_FAILED(sq_get(v, -2)))
		{
			// Create one if not exists
			sq_newtable(v);
			sq_pushstring(v, tblName, -1);
			sq_push(v, -2);
			sq_newslot(v, -4, true);
			return;
		}

		if (sq_gettype(v, -1) != OT_TABLE)
			sqx_throwfmt(v, "%s.%s: not a table", typeinfo::bindingName(), tblName);

		bool created = false;
		if (cloneIfExists)
		{
			sq_clone(v, -1);
			created = true;
		}
		else if (useDelegate)
		{
			sq_newtable(v);
			sq_push(v, -2);
			sq_setdelegate(v, -2);
			created = true;
		}

		if (created)
		{
			sq_replace(v, -2);

			sq_pushstring(v, tblName, -1);
			sq_push(v, -2);
			sq_newslot(v, -4, true);
		}

		sq_replace(v, -2);
	}

	template <typename TValueClass>
	static void addStatic(HSQUIRRELVM v, const char* name, const TValueClass& value)
	{
		TNitBind::template pushClass<TClass>(v);
		TNitBind::template newSlot(v, -1, name, value, true);
		sq_poptop(v);
	}

	static ptrtype self(HSQUIRRELVM v)
	{
		SQUserPointer ptr = sqx_getinstanceup(v, 1, typeinfo::typeTag());
		ptrtype ret = ptr ? typeinfo::toClassPtr(ptr) : NULL;

		if (ret == NULL)
		{
			sq_purgeinstance(v, 1);
			sqx_throwfmt(v, "%s.Self(v): null native instance", typeinfo::bindingName());
		}
		return ret;
	}

	static ptrtype setSelf(HSQUIRRELVM v, ptrtype obj)
	{
		sq_setinstanceup(v, 1, obj);
		return obj;
	}

	static ptrtype self_noThrow(HSQUIRRELVM v)
	{
		SQUserPointer ptr = sqx_getinstanceup(v, 1, typeinfo::typeTag());
		ptrtype ret = ptr ? typeinfo::toClassPtr(ptr) : NULL;

		return ret;
	}

private:
	template <class TInterface>
	static void* _castInterface(SQUserPointer p)
	{
		TClass* concrete = typeinfo::toClassPtr(p);
		TInterface* intf = concrete;
		return intf;
	}
};

////////////////////////////////////////////////////////////////////////////////

template <typename TInterface, typename TNitBind = NitBind >
class TNitInterface : public TNitBind
{
public:
	typedef TInterface type;
	typedef TInterface* ptrtype;
	typedef typename TNitBind::PropEntry PropEntry;
	typedef typename TNitBind::FuncEntry FuncEntry;

	template <typename TClass>
	static void bind(HSQUIRRELVM v, PropEntry* props, FuncEntry* funcs)
	{
		TNitClass<TClass, TNitBind>::template addInterface<type>(v);
		TNitClass<TClass, TNitBind>::bindMore(v, props, funcs);
	}

	template <typename TClass>
	static void addStaticTable(HSQUIRRELVM v, const char* tblName, bool cloneIfExists = false, bool useDelegate = true)
	{
		typedef typename TNitBind::template TypeInfo<TClass> typeinfo;

		TNitBind::template pushClass<TClass>(v);

		sq_pushstring(v, tblName, -1);
		if (SQ_FAILED(sq_get(v, -2)))
		{
			// Create one if not exists
			sq_newtable(v);
			sq_pushstring(v, tblName, -1);
			sq_push(v, -2);
			sq_newslot(v, -4, true);
			return;
		}

		if (sq_gettype(v, -1) != OT_TABLE)
			sqx_throwfmt(v, "%s.%s: not a table", typeinfo::bindingName(), tblName);

		bool created = false;
		if (cloneIfExists)
		{
			sq_clone(v, -1);
			created = true;
		}
		else if (useDelegate)
		{
			sq_newtable(v);
			sq_push(v, -2);
			sq_setdelegate(v, -2);
			created = true;
		}

		if (created)
		{
			sq_replace(v, -2);

			sq_pushstring(v, tblName, -1);
			sq_push(v, -2);
			sq_newslot(v, -4, true);
		}

		sq_replace(v, -2);
	}

	template <typename TClass, typename TValueClass>
	static void addStatic(HSQUIRRELVM v, const char* name, const TValueClass& value)
	{
		TNitBind::template pushClass<TClass>(v);
		TNitBind::template newSlot(v, -1, name, value, true);
		sq_poptop(v);
	}

	static ptrtype self(HSQUIRRELVM v)
	{
		return TNitBind::template getInterface<TInterface>(v, 1, false);
	}

	static ptrtype self_noThrow(HSQUIRRELVM v)
	{
		return TNitBind::template getInterface<TInterface>(v, 1, true);
	}
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API NitIterator
{
public:
	NitIterator(HSQUIRRELVM v, int objIdx)
	{
		this->v = v;
		_outTop = sq_gettop(v);
		_objIdx = NitBindImpl::toAbsIdx(v, objIdx);
		_keyIndex = _objIdx;
		_valueIndex = _objIdx;
		sq_pushnull(v);
		_loopTop = sq_gettop(v);
	}

	~NitIterator()
	{
		if (sq_gettop(v) > _outTop)
			sq_settop(v, _outTop);
	}

	bool hasNext()
	{
		bool hasNext = SQ_SUCCEEDED(sq_next(v, _objIdx));
		_keyIndex = sq_gettop(v) - 1;
		_valueIndex = sq_gettop(v);
		return hasNext;
	}

	int keyIndex()
	{
		return _keyIndex;
	}

	int valueIndex()
	{
		return _valueIndex;
	}

	void next()
	{
		if (sq_gettop(v) > _loopTop)
			sq_settop(v, _loopTop);
	}

private:
	HSQUIRRELVM v;
	int _objIdx;
	int _outTop;
	int _loopTop;
	int _keyIndex;
	int _valueIndex;
};

////////////////////////////////////////////////////////////////////////////////

// TODO: Check safety - Crash may occur if source object deleted during iteration

template <typename TPeerIterator>
struct TNitNativeItrAdapter
{
	typedef TNitNativeItrAdapter<TPeerIterator> ThisClass;

	TPeerIterator curr;
	TPeerIterator end;

	TNitNativeItrAdapter(const TPeerIterator& begin, const TPeerIterator& end) : curr(begin), end(end) { }

	static SQInteger _nexti(HSQUIRRELVM v, const TPeerIterator& begin, const TPeerIterator& end)
	{
		ThisClass* itr = (ThisClass*)sq_newnativeitr(v, itrfunc, sizeof(ThisClass));
		new (itr) ThisClass(begin, end);
		return 1;
	}

	static SQInteger itrfunc(HSQUIRRELVM v, SQUserPointer pItr)
	{
		ThisClass* itr = (ThisClass*)pItr;
		if (v == NULL)
		{
			itr->~ThisClass();
			return 0;
		}

		if (itr->curr == itr->end)
			return 0;

		return NitBind::push(v, *itr->curr++);
	}
};

template <typename TPeerIterator>
inline static SQInteger nitNativeItr(HSQUIRRELVM v, const TPeerIterator& begin, const TPeerIterator& end)
{
	return TNitNativeItrAdapter<TPeerIterator>::_nexti(v, begin, end);
}

////////////////////////////////////////////////////////////////////////////////

// TODO: Check safety - Crash may occur if source object deleted during iteration

template <typename TPeerIterator>
struct TNitNativePairItrAdapter
{
	typedef TNitNativePairItrAdapter<TPeerIterator> ThisClass;

	TPeerIterator curr;
	TPeerIterator end;

	TNitNativePairItrAdapter(const TPeerIterator& begin, const TPeerIterator& end) : curr(begin), end(end) { }

	static SQInteger _nexti(HSQUIRRELVM v, const TPeerIterator& begin, const TPeerIterator& end)
	{
		ThisClass* itr = (ThisClass*)sq_newnativeitr(v, itrfunc, sizeof(ThisClass));
		new (itr) ThisClass(begin, end);
		return 1;
	}

	static SQInteger itrfunc(HSQUIRRELVM v, SQUserPointer pItr)
	{
		ThisClass* itr = (ThisClass*)pItr;
		if (v == NULL)
		{
			itr->~ThisClass();
			return 0;
		}

		TPeerIterator& curr = itr->curr;

		if (curr == itr->end)
			return 0;

		NitBind::push(v, curr->first);
		NitBind::push(v, curr->second);
		++curr;
		return 2;
	}
};

template <typename TPeerIterator>
inline static SQInteger nitNativePairItr(HSQUIRRELVM v, const TPeerIterator& begin, const TPeerIterator& end)
{
	return TNitNativePairItrAdapter<TPeerIterator>::_nexti(v, begin, end);
}

////////////////////////////////////////////////////////////////////////////////

// template implementations

template <typename TImpl>
int TNitBind<TImpl>::pushFmt(HSQUIRRELVM v, const char* fmt, ...)
{
	static char buf[512];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, COUNT_OF(buf), fmt, args);
	va_end(args);

	sq_pushstring(v, buf, -1);
	return 1;

}

template <typename TImpl>
template <class TClass>
TClass* TNitBind<TImpl>::get(HSQUIRRELVM v, SQInteger idx)
{
	void* up = NULL;

	if (SQ_FAILED(sq_getinstanceup(v, idx, &up, TImpl::template TypeInfo<TClass>::typeTag())))
	{
		sq_tostring(v, idx);
		String value = sqx_getstring(v, -1);
		sq_poptop(v);

		sqx_throwfmt(v, "%s expected, got %s", TImpl::template TypeInfo<TClass>::bindingName(), value.c_str());
	}

	if (up)
	{
		return TImpl::template TypeInfo<TClass>::toClassPtr(up);
	}
	else
	{
		sqx_throwfmt(v, "native instance lost: %s", TImpl::template TypeInfo<TClass>::bindingName());
		return NULL;
	}
}

template <typename TImpl>
template <class TInterface>
bool TNitBind<TImpl>::hasInterface(HSQUIRRELVM v, SQInteger clsIdx)
{
	return TImpl::_hasInterface(v, clsIdx, typeid(TInterface));
}

template <typename TImpl>
template <class TInterface>
TInterface* TNitBind<TImpl>::getInterface(HSQUIRRELVM v, SQInteger idx, bool allowNull)
{
	if (allowNull && sq_gettype(v, idx) == OT_NULL) return NULL;

	void* intf = TImpl::_getInterfacePtr(v, idx, typeid(TInterface));

	if (intf == NULL)
	{
		if (allowNull) return NULL;
		sqx_throwfmt(v, "instance has no %s interface", typeid(TInterface).name());
	}

	return (TInterface*)intf;
}

template <typename TImpl>
template <class TClass>
SQRESULT TNitBind<TImpl>::bindInstance(HSQUIRRELVM v, TClass* ptr, SQObjectRef& outInstance)
{
	if (!TImpl::template TypeInfo<TClass>::isPtr())
		sqx_throwfmt(v, "pointer of %s expected", TImpl::template TypeInfo<TClass>::bindingName());

	if (ptr == NULL)
	{
		sq_pushnull(v);
		return 1;
	}

	SQUserPointer up = TImpl::template TypeInfo<TClass>::toUserPtr(ptr);

	if (SQ_SUCCEEDED(TImpl::getCachedInstance(v, up, TImpl::template TypeInfo<TClass>::typeTag(), outInstance)))
		return SQ_OK;

	return TImpl::_bindPtrInstance(
		v, 
		outInstance,
		up,
		TImpl::template TypeInfo<TClass>::typeTag(ptr),
		TImpl::template TypeInfo<TClass>::typeTag(), 
		TImpl::template TypeInfo<TClass>::bindingName(), 
		TImpl::template TypeInfo<TClass>::typeCode(), 
		TImpl::template TypeInfo<TClass>::typeCode(ptr),
		TImpl::template TypeInfo<TClass>::constructHook, 
		TImpl::template TypeInfo<TClass>::releaseHook);
}


template <typename TImpl>
template <class TClass>
int TNitBind<TImpl>::push(HSQUIRRELVM v, TClass* ptr)
{
	SQObjectRef inst;
	SQRESULT sr = bindInstance(v, ptr, inst);

	if (SQ_SUCCEEDED(sr))
	{
		inst.push(v);
		return 1;
	}

	return sr;
}

template <typename TImpl>
template <class TClass>
SQRESULT TNitBind<TImpl>::bindInstance(HSQUIRRELVM v, const TClass& value, SQObjectRef& outInstance)
{
	// value version - doesn't need cache lookup
	if (TImpl::template TypeInfo<TClass>::isPtr())
		sqx_throwfmt(v, "value of %s expected", TImpl::template TypeInfo<TClass>::bindingName());

	SQRESULT sr = TImpl::_bindValueInstance(
		v,
		outInstance,
		TImpl::template TypeInfo<TClass>::typeTag(),
		TImpl::template TypeInfo<TClass>::bindingName(), 
		TImpl::template TypeInfo<TClass>::constructHook, 
		TImpl::template TypeInfo<TClass>::releaseHook);

	if (SQ_SUCCEEDED(sr))
	{
		TClass* lvalue = (TClass*)outInstance.getInstanceUp(0);
		TImpl::template TypeInfo<TClass>::defaultConstructor(lvalue);
		*lvalue = value;
		return SQ_OK;
	}

	return sr;
}

template <typename TImpl>
template <class TClass>
int TNitBind<TImpl>::push(HSQUIRRELVM v, const TClass& value)
{
	SQObjectRef inst;
	SQRESULT sr = bindInstance(v, value, inst);

	if (SQ_SUCCEEDED(sr))
	{
		inst.push(v);
		return 1;
	}

	return sr;
}

template <typename TImpl>
template <class TClass>
int TNitBind<TImpl>::pushClass(HSQUIRRELVM v)
{
	sq_pushregistrytable(v);							// [reg]
	sq_pushstring(v, TImpl::template TypeInfo<TClass>::typeCode(), -1);	// [reg] "typecode"
	if (SQ_FAILED(sq_get(v, -2)))						// [reg] [class]
	{
		sq_poptop(v);
		sqx_prterrf(v, "*** class '%s' is not yet registered\n", TImpl::template TypeInfo<TClass>::bindingName());
		return 0;
	}

	sq_replace(v, -2);									// [class]
	return 1;
}

template <typename TImpl>
template <class TClass>
bool TNitBind<TImpl>::purge(HSQUIRRELVM v, TClass* ptr)
{
	if (!TImpl::template TypeInfo<TClass>::isPtr())
		sqx_throwfmt(v, "ptr expected");

	if (ptr == NULL) return false;

	SQUserPointer up = TImpl::template TypeInfo<TClass>::toUserPtr(ptr);
	if (SQ_FAILED(getCachedPtr(v, up, TImpl::template TypeInfo<TClass>::typeTag()))) return false;

	sq_purgeinstance(v, -1);
	sq_poptop(v);
	return true;
}

////////////////////////////////////////////////////////////////////////////////

#define OPT_STATIC_DEF(type, value)	const_cast<type*>(&value)

#define sq_register(v, n, f) (sq_pushstring(v, n, -1), sq_newclosure(v, f, 0), sq_setnativeclosureinfo(v, -1, n, __FILE__), sq_createslot(v, -3))
#define sq_register_h(v, n, f, h) (sq_pushstring(v, n, -1), sq_newclosure(v, f, 0), sq_setnativeclosureinfo(v, -1, n, __FILE__), sq_pushstring(v, h, -1), sq_sethelp(v, -2), sq_createslot(v, -3))
#define sq_dostring(v, s) (sq_compilebuffer(v, s, strlen(s), "", true), sq_pushroottable(v), sq_call(v, 1, false, true), sq_poptop(v))

NS_NIT_END;
