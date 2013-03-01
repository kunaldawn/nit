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

////////////////////////////////////////////////////////////////////////////////

// Support macros

#define PROP_ENTRY(PROP)				{ #PROP, _nb_get_##PROP, _nb_set_##PROP, __FILE__ }
#define PROP_ENTRY_R(PROP)				{ #PROP, _nb_get_##PROP, NULL, __FILE__ }
#define FUNC_ENTRY(FUNC)				{ #FUNC, _nb_##FUNC, __FILE__, NULL }
#define CONS_ENTRY()					{ "!", _nb_cons, __FILE__, NULL }

#define EXPROP_ENTRY(NAME, GETFN, SETFN){ NAME, GETFN, SETFN, __FILE__ }
#define EXFUNC_ENTRY(NAME, FUNC)		{ NAME, FUNC, __FILE__ }

#define CONS_ENTRY_H(HELP)				{ "!", _nb_cons, __FILE__, HELP }
#define FUNC_ENTRY_H(FUNC, HELP)		{ #FUNC, _nb_##FUNC, __FILE__, HELP }
#define EXFUNC_ENTRY_H(NAME, FUNC, HELP){ NAME, FUNC, __FILE__, HELP }

#define NB_PROP_GET(PROP)				static SQInteger _nb_get_##PROP(HSQUIRRELVM v)
#define NB_PROP_SET(PROP)				static SQInteger _nb_set_##PROP(HSQUIRRELVM v)
#define NB_FUNC(FUNC)					static SQInteger _nb_##FUNC(HSQUIRRELVM v)
#define NB_CONS()						static SQInteger _nb_cons(HSQUIRRELVM v)

#define _NB_TYPE_PTR_BASE(API, TClass, TBaseClass) \
	template<> API const SQChar*		NitBindImpl::TypeInfo<TClass>::bindingName() { return #TClass; } \
	template<> API const SQChar*		NitBindImpl::TypeInfo<TClass>::typeCode() { return typeid(TClass).name(); } \
	template<> API const SQChar*		NitBindImpl::TypeInfo<TClass>::typeCode(TClass* ptr) { return typeid(*ptr).name(); } \
	template<> API const SQChar*		NitBindImpl::TypeInfo<TClass>::baseTypeCode() { return typeid(TBaseClass).name(); } \
	template<> API SQUserPointer		NitBindImpl::TypeInfo<TClass>::typeTag() { return (SQUserPointer) &typeid(TClass); } \
	template<> API SQUserPointer		NitBindImpl::TypeInfo<TClass>::typeTag(TClass* ptr) { return (SQUserPointer) &typeid(*ptr); } \
	template<> API bool					NitBindImpl::TypeInfo<TClass>::isPtr() { return true; } \
	template<> API size_t				NitBindImpl::TypeInfo<TClass>::classUserDataSize() { return 0; }

#define NB_TYPE_RAW_PTR(API, TClass, TBaseClass) \
	_NB_TYPE_PTR_BASE(API, TClass, TBaseClass) \
	template<> API SQInteger			NitBindImpl::TypeInfo<TClass>::constructHook(HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p, SQInteger size) { NitBindImpl::addCachedPtr(v, inst, p); return 0; } \
	template<> API SQInteger			NitBindImpl::TypeInfo<TClass>::postConsHook(HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p, SQInteger size) { return 0; } \
	template<> API SQInteger			NitBindImpl::TypeInfo<TClass>::releaseHook(HSQUIRRELVM v, SQUserPointer p, SQInteger size) { NitBindImpl::removeCachedPtr(v, p); return 0; } \
	template<> API SQUserPointer		NitBindImpl::TypeInfo<TClass>::toUserPtr(TClass* ptr) { return (SQUserPointer*)ptr; } \
	template<> API TClass*				NitBindImpl::TypeInfo<TClass>::toClassPtr(SQUserPointer ptr) { return (TClass*)ptr; } 

#define NB_TYPE_REF(API, TClass, TBaseClass, IncRefFn, DecRefFn) \
	_NB_TYPE_PTR_BASE(API, TClass, TBaseClass) \
	template<> API SQInteger			NitBindImpl::TypeInfo<TClass>::constructHook(HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p, SQInteger size) { NitBindImpl::addCachedPtr(v, inst, p); IncRefFn((TClass*)p); return 0; } \
	template<> API SQInteger			NitBindImpl::TypeInfo<TClass>::postConsHook(HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p, SQInteger size) { return 0; } \
	template<> API SQInteger			NitBindImpl::TypeInfo<TClass>::releaseHook(HSQUIRRELVM v, SQUserPointer p, SQInteger size) { NitBindImpl::removeCachedPtr(v, p); DecRefFn((TClass*)p); return 0; } \
	template<> API SQUserPointer		NitBindImpl::TypeInfo<TClass>::toUserPtr(TClass* ptr) { return (SQUserPointer*)ptr; } \
	template<> API TClass*				NitBindImpl::TypeInfo<TClass>::toClassPtr(SQUserPointer ptr) { return (TClass*)ptr; } 

#define NB_TYPE_AUTODELETE(API, TClass, TBaseClass, DeleteFn) \
	_NB_TYPE_PTR_BASE(API, TClass, TBaseClass) \
	template<> API SQInteger			NitBindImpl::TypeInfo<TClass>::constructHook(HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p, SQInteger size) { NitBindImpl::addCachedPtr(v, inst, p); return 0; } \
	template<> API SQInteger			NitBindImpl::TypeInfo<TClass>::postConsHook(HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p, SQInteger size) { return 0; } \
	template<> API SQInteger			NitBindImpl::TypeInfo<TClass>::releaseHook(HSQUIRRELVM v, SQUserPointer p, SQInteger size) { NitBindImpl::removeCachedPtr(v, p); DeleteFn((TClass*)p); return 0; } \
	template<> API SQUserPointer		NitBindImpl::TypeInfo<TClass>::toUserPtr(TClass* ptr) { return (SQUserPointer*)ptr; } \
	template<> API TClass*				NitBindImpl::TypeInfo<TClass>::toClassPtr(SQUserPointer ptr) { return (TClass*)ptr; } 

#define NB_TYPE_VALUE_BASE(API, TClass, TBaseClass) \
	template<> API const SQChar*		NitBindImpl::TypeInfo<TClass>::bindingName() { return #TClass; } \
	template<> API const char*			NitBindImpl::TypeInfo<TClass>::typeCode() { return typeid(TClass).name(); } \
	template<> API const char*			NitBindImpl::TypeInfo<TClass>::baseTypeCode() { return typeid(TBaseClass).name(); } \
	template<> API SQUserPointer		NitBindImpl::TypeInfo<TClass>::typeTag() { return (SQUserPointer) &typeid(TClass); } \
	template<> API SQUserPointer		NitBindImpl::TypeInfo<TClass>::typeTag(TClass* ptr) { return (SQUserPointer) &typeid(*ptr); } \
	template<> API bool					NitBindImpl::TypeInfo<TClass>::isPtr() { return false; } \
	template<> API size_t				NitBindImpl::TypeInfo<TClass>::classUserDataSize() { return sizeof(TClass); } \
	template<> API SQUserPointer		NitBindImpl::TypeInfo<TClass>::toUserPtr(TClass* ptr) { return (SQUserPointer*)ptr; } \
	template<> API TClass*				NitBindImpl::TypeInfo<TClass>::toClassPtr(SQUserPointer ptr) { return (TClass*)ptr; } \
	template<> API void					NitBindImpl::TypeInfo<TClass>::defaultConstructor(TClass* ptr) { new (ptr) TClass(); } \

#define NB_TYPE_VALUE(API, TClass, TBaseClass) \
	NB_TYPE_VALUE_BASE(API, TClass, TBaseClass) \
	template<> API SQInteger			NitBindImpl::TypeInfo<TClass>::constructHook(HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p, SQInteger size) { return 0; } \
	template<> API SQInteger			NitBindImpl::TypeInfo<TClass>::postConsHook(HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p, SQInteger size) { return 0; } \
	template<> API SQInteger			NitBindImpl::TypeInfo<TClass>::releaseHook(HSQUIRRELVM v, SQUserPointer p, SQInteger size) { ::nit::callDestructor(*(TClass*)p); return 0; }	

#define NB_TYPE_VALUE_CUSTOM(API, TClass, TBaseClass, CONS_STMT, DEST_STMT) \
	NB_TYPE_VALUE_BASE(API, TClass, TBaseClass) \
	template<> API SQInteger			NitBindImpl::TypeInfo<TClass>::constructHook(HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p, SQInteger size) { {CONS_STMT}; return 0; } \
	template<> API SQInteger			NitBindImpl::TypeInfo<TClass>::postConsHook(HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p, SQInteger size) { return 0; } \
	template<> API SQInteger			NitBindImpl::TypeInfo<TClass>::releaseHook(HSQUIRRELVM v, SQUserPointer p, SQInteger size) { {DEST_STMT}; return 0; }	

#define NB_TYPE_WEAK(API, TClass, TBaseClass) \
	_NB_TYPE_PTR_BASE(API, TClass, TBaseClass) \
	template<> API SQInteger			NitBindImpl::TypeInfo<TClass>::constructHook(HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p, SQInteger size) { NitBindImpl::addCachedPtr(v, inst, p); ScriptRuntime::getRuntime(v)->weakAdd((TClass*)p); return 0; } \
	template<> API SQInteger			NitBindImpl::TypeInfo<TClass>::postConsHook(HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p, SQInteger size) { return 0; } \
	template<> API SQInteger			NitBindImpl::TypeInfo<TClass>::releaseHook(HSQUIRRELVM v, SQUserPointer p, SQInteger size) { NitBindImpl::removeCachedPtr(v, p); ScriptRuntime::getRuntime(v)->weakRelease((TClass*)p); return 0; } \
	template<> API SQUserPointer		NitBindImpl::TypeInfo<TClass>::toUserPtr(TClass* ptr) { return (SQUserPointer*)ptr; } \
	template<> API TClass*				NitBindImpl::TypeInfo<TClass>::toClassPtr(SQUserPointer ptr) { return (TClass*)ptr; } 

////////////////////////////////////////////////////////////////////////////////