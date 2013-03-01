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

#include "nit/script/NitBind.h"

#include <OCIdl.h>

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

template <typename TComClass>
inline void safeRelease(TComClass*& unk)
{
	if (unk)
	{
		unk->Release();
		unk = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////

template <typename T>
class ComRef
{
public:
	inline ComRef(T* obj = NULL) : _obj(obj)									{ inc(obj); }
	inline ComRef(const ComRef& other) : _obj(other._obj)						{ inc(_obj); }
	inline ~ComRef()															{ dec(_obj); }

	// auto casting
	inline operator T* () const													{ return _obj; }
	inline T& operator* () const												{ return *_obj; }
	inline T* operator-> () const												{ return _obj; }

	// assignment
	inline ComRef& operator= (const ComRef& other)								{ set(other._obj); return *this; }
	inline ComRef& operator= (T* obj)											{ set(obj); return *this; }

	// comparisons
	inline bool operator== (T* obj) const										{ return _obj == obj; }
	inline bool operator!= (T* obj) const										{ return _obj != obj; }
	inline bool operator== (const ComRef& other) const							{ return _obj == other._obj; }
	inline bool operator!= (const ComRef& other) const							{ return _obj != other._obj; }

	// utility
	inline void swap(ComRef& other)												{ T* t = _obj; _obj = other._obj; other._obj = t; }
	inline const T* get() const													{ return _obj; }
	inline T*& get()															{ return _obj; }
	inline void set(T* obj)														{ inc(obj); dec(_obj); _obj = obj; }

	// ogre-compatible only do not use!
protected:
	inline static void inc(T* obj)												{ if (obj) obj->AddRef(); }
	inline static void dec(T* obj)												{ if (obj) obj->Release(); }

	T* _obj;
};

////////////////////////////////////////////////////////////////////////////////

class NitBindCom : public RefCounted
{
public:
	typedef ::IUnknown					IUnknown;
	typedef ::IDispatch					IDispatch;
	typedef ::ITypeInfo					ITypeInfo;
	typedef ::ITypeLib					ITypeLib;
	typedef ::ITypeComp					ITypeComp;

	class TypeLib;
	class TypeInfo; 
	class Variant;
	class InvokeHandler;

public:
	NitBindCom(HSQUIRRELVM v);
	virtual ~NitBindCom();

	static NitBindCom*					get(HSQUIRRELVM v);

	static LCID							getLocaleID()							{ return LOCALE_SYSTEM_DEFAULT; }

	bool								isVerbose()								{ return _verbose; }
	void								setVerbose(bool flag)					{ _verbose = flag; }

public:
	ComRef<IDispatch>					create(const String& progid);
	ComRef<IDispatch>					open(const String& url);

public:
	SQInteger							push(HSQUIRRELVM v, IDispatch* disp);
	SQInteger							push(HSQUIRRELVM v, ITypeInfo* ti);
	SQInteger							push(HSQUIRRELVM v, ITypeLib* tl);

	bool								isCached(IDispatch* disp)				{ return _dispCache.find(disp) != _dispCache.end(); }
	bool								isCached(ITypeInfo* ti)					{ return _typeCache.find(ti) != _typeCache.end(); }
	bool								isCached(ITypeLib* tl)					{ return _typeLibCache.find(tl) != _typeLibCache.end(); }

	void								getInstancesOf(TypeLib* typelib, vector<IDispatch*>::type& outResults, HSQUIRRELVM v);

	HSQUIRRELVM							getVm()									{ return _vm; }

public:
	static SQInteger					push(HSQUIRRELVM v, IEnumVARIANT* en);
	static IEnumVARIANT*				getEnum(HSQUIRRELVM v, SQInteger idx);

public:
	static String						toString(BSTR bstr);
	static String						toString(const GUID& guid);
	static String						toString(const VARIANT& var);

	static BSTR							toBstr(const String& str)				{ return toBstr(str.c_str(), str.length()); }
	static BSTR							toBstr(const char* str, size_t len);

	static HRESULT						toClsId(const String& progID, CLSID& outClsID);

	static String						errorStr(HRESULT hr);
	static String						paramFlagsStr(ushort flags);
	static String						propGetRetStr(ITypeInfo* typeinfo, FUNCDESC* fd);
	static String						typeAttrStr(TYPEATTR* attr);
	static String						typeDescStr(ITypeInfo* typeinfo, const TYPEDESC& tdesc);
	static String						paramsDescStr(ITypeInfo* typeinfo, FUNCDESC* fd);
	static String						funcDescStr(ITypeInfo* typeinfo, FUNCDESC* fd);
	static String						varDescStr(ITypeInfo* typeinfo, VARDESC* vd);

private:
	typedef map<IDispatch*, SQInstance*>::type		DispCache;
	typedef map<ITypeInfo*, SQClass*>::type			TypeCache;
	typedef map<ITypeLib*, SQInstance*>::type		TypeLibCache;

	friend class						NitLibCom;

	DispCache							_dispCache;
	TypeCache							_typeCache;
	TypeLibCache						_typeLibCache;

	HSQUIRRELVM							_vm;

	bool								_verbose;

	void								uncache(ITypeLib* typelib);
	void								uncache(ITypeInfo* typeinfo);
	void								uncache(IDispatch* disp);

	static SQRESULT						dispRelease(HSQUIRRELVM v, SQUserPointer p, SQInteger size);
	static SQRESULT						enumRelease(HSQUIRRELVM v, SQUserPointer p, SQInteger size);
};

////////////////////////////////////////////////////////////////////////////////

class NitBindCom::TypeLib : public RefCounted
{
public:
	ITypeLib*							getPeer()								{ return _peer; }
	ITypeComp*							getComp()								{ return _comp; }
	NitBindCom*							getBindCom()							{ return _bindCom; }
	const String&						getName()								{ return _name; }

public:
	void								purge(HSQUIRRELVM v);

protected:
	virtual void						onDelete()								{ dispose(); }
	void								dispose();

private:
	friend class NitBindCom;
	friend class TypeInfo;
	friend class NitBindComTypeLib;

	TypeLib(NitBindCom* NitBindCom, ITypeLib* tl);
	void								remember(TypeInfo* ti, HSQOBJECT cls);

	ComRef<ITypeLib>					_peer;
	NitBindCom*							_bindCom;
	ComRef<ITypeComp>					_comp;
	String								_name;

	typedef map<String, HSQOBJECT>::type Types;
	Types								_types;
};

////////////////////////////////////////////////////////////////////////////////

class NitBindCom::TypeInfo : public RefCounted
{
public:
	ITypeInfo*							getPeer()								{ return _peer; }
	TypeLib*							getTypeLib()							{ return _typeLib; }
	NitBindCom*							getBindCom()							{ return _bindCom; }
	const String&						getName()								{ return _name; }
	const String&						getSrcName()							{ return _srcName; }

public:
	SQRESULT							createClass(HSQUIRRELVM v);
	SQRESULT							createInstance(HSQUIRRELVM v);
	SQRESULT							createMember(HSQUIRRELVM v);

protected:
	virtual void						onDelete()								{ dispose(); }
	void								dispose();

private:
	friend class NitBindCom;

	TypeInfo(ITypeInfo* peer, TypeLib* ti);

	ComRef<ITypeInfo>					_peer;
	TypeLib*							_typeLib;
	NitBindCom*							_bindCom;
	String								_name;
	String								_srcName;
	list<String>::type					_names;
	vector<FUNCDESC*>::type				_funcDescs;
	vector<VARDESC*>::type				_varDescs;

	static SQRESULT						classReleaseHook(HSQUIRRELVM v, SQUserPointer p, SQInteger size);
	static SQRESULT						instanceToString(HSQUIRRELVM v);

	class BindingContext;

	bool								isGoodGetter(FUNCDESC* fd);
	bool								isGoodSetter(FUNCDESC* fd);

	HRESULT								bindMetaCallFunc(HSQUIRRELVM v, FUNCDESC* fd, BindingContext* ctx);
	HRESULT								bindNewEnumFunc(HSQUIRRELVM v, FUNCDESC* fd, BindingContext* ctx);
	HRESULT								bindEvaluateFunc(HSQUIRRELVM v, FUNCDESC* fd, BindingContext* ctx);
	HRESULT								bindConstructor(HSQUIRRELVM v, FUNCDESC* fd, BindingContext* ctx);
	HRESULT								bindDestructor(HSQUIRRELVM v, FUNCDESC* fd, BindingContext* ctx);
	HRESULT								bindCollectFunc(HSQUIRRELVM v, FUNCDESC* fd, BindingContext* ctx);

	HRESULT								bindProp(HSQUIRRELVM v, FUNCDESC* fd, BindingContext* ctx);
	HRESULT								bindFunc(HSQUIRRELVM v, FUNCDESC* fd, BindingContext* ctx);

	HRESULT								bindVar(HSQUIRRELVM v, VARDESC* vd, BindingContext* ctx);
};

////////////////////////////////////////////////////////////////////////////////

class NitBindCom::InvokeHandler
{
public:
	InvokeHandler(IDispatch* instance, TypeInfo* typeinfo, FUNCDESC* fd)
		: _instance(instance), _typeInfo(typeinfo), _funcDesc(fd)
	{
	}

public:
	static SQRESULT						invokeCallback(HSQUIRRELVM v);
	static SQRESULT						invokeMetaCall(HSQUIRRELVM v);

	static SQRESULT						invokeMetaGet(HSQUIRRELVM v);
	static SQRESULT						invokeMetaSet(HSQUIRRELVM v);
	static SQRESULT						invokeMetaNexti(HSQUIRRELVM v);

public:
	typedef SQRESULT					(*HandlerFunc) (InvokeHandler* hdlr, HSQUIRRELVM v);

	static SQRESULT 					handleGet(InvokeHandler* hdlr, HSQUIRRELVM v);
	static SQRESULT 					handleGetByRef(InvokeHandler* hdlr, HSQUIRRELVM v);

	static SQRESULT 					handlePut(InvokeHandler* hdlr, HSQUIRRELVM v);
	static SQRESULT 					handlePutRef(InvokeHandler* hdlr, HSQUIRRELVM v);

	static SQRESULT						handleCall_noArg_noRet(InvokeHandler* hdlr, HSQUIRRELVM v);
	static SQRESULT						handleCall_oneArg_noRet(InvokeHandler* hdlr, HSQUIRRELVM v);

	static SQRESULT 					handleCall_noRet(InvokeHandler* hdlr, HSQUIRRELVM v);
	static SQRESULT 					handleCall_oneRet(InvokeHandler* hdlr, HSQUIRRELVM v);
	static SQRESULT						handleCall_multRet(InvokeHandler* hdlr, HSQUIRRELVM v);

	static SQRESULT						handleCall_default(InvokeHandler* hdlr, HSQUIRRELVM v);

	static SQRESULT						call_default(InvokeHandler* hdlr, HSQUIRRELVM v, int argStart, int argEnd);

protected:
	IDispatch*							_instance;
	TypeInfo*							_typeInfo;
	FUNCDESC*							_funcDesc;

	typedef vector<Variant>::type		ArgArray;
	typedef vector<DISPID>::type		NamedArgs;

	static SQRESULT						error(InvokeHandler* hdlr, const char* memname, HSQUIRRELVM v, HRESULT hr, EXCEPINFO& excep, int argNo);
};

////////////////////////////////////////////////////////////////////////////////

class NitBindCom::Variant
{
public:
	Variant()																	{ VariantInit(&_peer); }
	~Variant()																	{ clear(); }

	Variant(const Variant& other)												{ VariantInit(&_peer); VariantCopy(&_peer, &other._peer); }
	Variant(VARIANTARG& other)													{ VariantInit(&_peer); VariantCopy(&_peer, &other); }

	Variant&							operator = (const Variant& other)		{ clear(); VariantCopy(&_peer, &other._peer); return *this; }
	Variant&							operator = (VARIANTARG& other)			{ clear(); VariantCopy(&_peer, &other); return *this; }

public:
	VARIANTARG*							get()									{ return &_peer; }
	operator							VARIANTARG*()							{ return &_peer; }
	VARIANTARG*							operator-> ()							{ return &_peer; }

public:
	HRESULT								getFrom(HSQUIRRELVM v, SQInteger idx);
	SQRESULT							push(HSQUIRRELVM v);

public:
	HRESULT								changeType(VARTYPE vt);

	static long							getSize(VARTYPE vt);

	void								makeValue();
	void								makeRef();
	void								makeVariantRef();

	void								clear();

private:
	VARIANTARG							_peer;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
