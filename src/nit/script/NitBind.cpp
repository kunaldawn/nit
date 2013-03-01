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

#include "nit_pch.h"

#include "nit/script/NitBind.h"

#include <vector>
#include <set>

#define NB_LOG(...) 

#include "squirrel/sqobject.h"
#include "squirrel/sqstate.h"
#include "squirrel/sqtable.h"
#include "squirrel/sqvm.h"
#include "squirrel/sqclass.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

SQObjectRef::SQObjectRef(const SQObjectRef& o)
{
	_type = o._type;
	_unVal = o._unVal;

	if (SQ_ISREFCOUNTED(_type))
		++_unVal.pRefCounted->_uiRef;
}

SQObjectRef::SQObjectRef(const SQObject& o)
{
	_type = o._type;
	_unVal = o._unVal;

	if (SQ_ISREFCOUNTED(_type))
		++_unVal.pRefCounted->_uiRef;
}

SQObjectRef::~SQObjectRef()
{
	if (SQ_ISREFCOUNTED(_type))
	{
		SQRefCounted* o = _unVal.pRefCounted;
		if (--o->_uiRef == 0)
			o->Release();
	}
}

SQObjectRef& SQObjectRef::operator=(const SQObjectRef& o)
{
	SQObjectType oldType = _type;
	SQObjectValue oldVal = _unVal;

	_type = o._type;
	_unVal = o._unVal;

	if (SQ_ISREFCOUNTED(_type))
		++_unVal.pRefCounted->_uiRef;

	if (SQ_ISREFCOUNTED(oldType))
	{
		SQRefCounted* old = oldVal.pRefCounted;
		if (--old->_uiRef == 0)
			old->Release();
	}

	return *this;
}

SQObjectRef& SQObjectRef::operator=(const SQObject& o)
{
	SQObjectType oldType = _type;
	SQObjectValue oldVal = _unVal;

	_type = o._type;
	_unVal = o._unVal;

	if (SQ_ISREFCOUNTED(_type))
		++_unVal.pRefCounted->_uiRef;

	if (SQ_ISREFCOUNTED(oldType))
	{
		SQRefCounted* old = oldVal.pRefCounted;
		if (--old->_uiRef == 0)
			old->Release();
	}

	return *this;
}

void SQObjectRef::push(HSQUIRRELVM v)
{
	v->Push(*this);
}

void SQObjectRef::top(HSQUIRRELVM v)
{
	*this = v->Top();
}

void SQObjectRef::get(HSQUIRRELVM v, int idx)
{
	*this = stack_get(v, idx);
}

SQUserPointer SQObjectRef::getInstanceUp(SQUserPointer typeTag)
{
	if (sqi_type(*this) != OT_INSTANCE)
		return NULL;

	SQInstance* inst = sqi_instance(*this);
	SQUserPointer up = inst->_userpointer;

	if (typeTag == NULL || up == NULL)
		return up;

	SQClass* cl = inst->_class;
	if (cl == NULL) 
		return up;

	do 
	{
		if (cl->_typetag == typeTag) 
			return up;
		cl = cl->_base;
	} while (cl != NULL);

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////

bool NitBindImpl::install(HSQUIRRELVM v, void* ext)
{
	if (sq_getforeignptr(v) != NULL)
	{
		sqx_prterrf(v, "*** foreign ptr already exists!\n");
	}

	// Install BindEnv
	NitBindImpl::BindEnv* env = new NitBindImpl::BindEnv(ext);
	sq_setforeignptr(v, env);

	env->install(v);

	SQInteger top = sq_gettop(v);
	
	// Make _solbind table
	sq_pushroottable(v);
	NitBind::push(v, "_nitbind");
	sq_newtable(v);

	// Make libs table
	NitBind::push(v, "libs");
	sq_newtable(v);
	sq_newslot(v, -3, false); // libs
	sq_newslot(v, -3, false); // _nitbind

	sq_settop(v, top);

	// Call _DoInstall in squirrel vm to handle exceptions during the process
	sq_newclosure(v, _doInstall, 0);
	sq_pushnull(v);
	SQRESULT r = sq_call(v, 1, false, true);

	if (SQ_FAILED(r))
	{
		sq_getlasterror(v);
		sq_tostring(v, -1);
		const char* lasterr = "???";
		sq_getstring(v, -1, &lasterr);
		NitRuntime::getSingleton()->alert("NitBind", StringUtil::format("Install failed : %s", lasterr), true);
	}

	sq_settop(v, top);

	return SQ_SUCCEEDED(r);
}

SQInteger NitBindImpl::_doInstall(HSQUIRRELVM v)
{
	// register all libraries by dependancy order
	for (NitBindLibRegistry* r = NitBindLibRegistry::getHead(); r; r = r->_next)
	{
		bool ok = NitBindLibRegistry::install(v, r);
		if (!ok) return SQ_ERROR;
	}
	return 0;
}

extern void GCDump(HSQUIRRELVM v);

void NitBindImpl::uninstall(HSQUIRRELVM v)
{
	// delete bind env
	BindEnv* env = (BindEnv*)sq_getforeignptr(v);

	env->uninstall();

	if (false)
	{
		// clear root table
		sq_pushroottable(v);
		sq_clear(v, -1);
		sq_poptop(v);

		// clear registry table
		sq_pushregistrytable(v);
		sq_clear(v, -1);
		sq_poptop(v);

		// clear main thread
		sq_settop(v, 0);
	}

	// collect remaing garbages
	SQInteger gccount = sq_collectgarbage(v);

	if (gccount > 0)
	{
		LOG(0, ".. GC sweeps %d before closing\n", gccount);
	}

	// close the VM
	SQGCFinalizeReport report = { 0 };
	sq_close(v, &report);

	if (report.sweepCount > 0)
	{
		LOG(0, ".. GC sweeps %d during closing\n", report.sweepCount);
	}

	if (report.stickyCount > 0)
	{
		LOG(0, "*** GC Finalize sweeps %d sticky objects\n", report.sweepCount);
	}

	if (report.leakCount > 0)
	{
		LOG(0, "*** sq_vm leaks %d garbage object\n", report.leakCount);
	}

	if (env) delete env;
}

SQInteger NitBindImpl::_dispatchConstructor(HSQUIRRELVM v)
{
	// stack[-1] : user-constructor
	// stack[-2] : construct hook
	// stack[-3] : post-cons hook
	// stack[-4] : release hook
	// param[1]  : this

	// get hooks
	CONS_HOOK consHook = (CONS_HOOK) sqx_getuserpointer(v, -2);
	CONS_HOOK postConsHook = (CONS_HOOK) sqx_getuserpointer(v, -3);
	SQRELEASEHOOK releaseHook = (SQRELEASEHOOK) sqx_getuserpointer(v, -4);

	// execute user-constructor
	int nargs = sq_gettop(v) - 4;
	sq_push(v, -1);
	for (int i=1; i<=nargs; ++i)
		sq_push(v, i);
	if (SQ_FAILED(sq_call(v, nargs, SQFalse, SQTrue)))
		return SQ_ERROR;
	sq_poptop(v); // closure

	SQUserPointer obj = sqx_getinstanceup(v, 1, 0);
	if (obj == NULL)
		sqx_throwfmt(v, "cons failed: null native instance");

	SQObjectRef inst = stack_get(v, 1);

	// execute cons hook
	if (consHook) consHook(v, inst, obj, 0);

	// execute post cons hook
	if (postConsHook) postConsHook(v, inst, obj, 0);

	// register release hook
	if (releaseHook) sq_setreleasehook(v, 1, releaseHook);

	return 1;
}

SQInteger NitBindImpl::_nonConstructable(HSQUIRRELVM v)
{
	return sq_throwerror(v, "non-creatable native class");
}

SQInteger NitBindImpl::_nonInheritable(HSQUIRRELVM v)
{
	return sq_throwerror(v, "non-inheritable native class");
}


bool NitBindImpl::newFunction(HSQUIRRELVM v, const char* classname, const char* name, SQFUNCTION fn, const char* file, const char* help, SQInteger idx, bool isstatic)
{
	int absidx = toAbsIdx(v, idx);

	sq_pushstring(v, name, -1);
	sq_newclosure(v, fn, 0);
	sq_setnativeclosureinfo(v, -1, name, file);
	if (help)
	{
		sq_pushstring(v, help, -1);
		sq_sethelp(v, -2);
	}
	if (SQ_SUCCEEDED(sq_newslot(v, absidx, isstatic))) 
		return true;

	sq_pop(v, 2);
	sq_getlasterror(v);
	sqx_printf(v, "*** cannot register function '%s' for '%s' : %s\n",
		name, classname, sqx_getstring(v, -1));
	sq_poptop(v);

	return false;
}

void NitBindImpl::_bind(
					HSQUIRRELVM v, 
					PropEntry* props, 
					FuncEntry* funcs, 
					const SQChar* fullClassname, 
					const char* rawname,
					SQUserPointer typeTag, 
					const char* baseRawName,
					bool isPtr, 
					size_t classudSize,
					CONS_HOOK consHook, 
					CONS_HOOK postConsHook,
					SQRELEASEHOOK releaseHook)
{
	NB_LOG(0, v, "Binding %s for typetag %08x, raw: %s, baseraw: %s\n", fullClassname, typeTag, rawname, baseRawName ? baseRawName : "(null)");

	// exec: <baseclass> = <registry>[baseTypeTag]
	// exec: <class> = newclass(<baseclass>)

	if (strcmp(baseRawName, typeid(NULL).name()) == 0)
	{
		// This is root class - create class without base class
		sq_pushnull(v);								// stack: [base]    (base = null)
		sq_newclass(v, false, 0);					// stack: [base] [class]

	}
	else
	{
		// This is derived class
		sq_pushregistrytable(v);					// stack: [reg]
		sq_pushstring(v, baseRawName, -1);			// stack: [reg] "base"
		if (SQ_FAILED(sq_get(v, -2)))				// stack: [reg] [base]
		{
			sq_poptop(v); // registry table
			sqx_prterrf(v, "*** '%s': base class '%s' is not yet registered\n", fullClassname, baseRawName);
			return;
		}

		// create class derived from base class
		sq_push(v, -1);								// stack: [reg] [base] [base]
		sq_newclass(v, true, 0);					// stack: [reg] [base] [class]

		sq_remove(v, -3); // registry table			// stack: [base] [class]
	}

	// stack now:
	// [-1] : class
	// [-2] : base class (null if non inherited)

	// link type tag
	sq_settypetag(v, -1, typeTag);

	// setup automatic mem allocation for value types
	if (!isPtr)
	{
		sq_setclassudsize(v, -1, classudSize);
	}

	// register default functions ///////////////

	newFunction(v, fullClassname, "_inherited", _nonInheritable, __FILE__, NULL, -1, false);

	// register user functions //////////////////

	SQFUNCTION userCons = NULL;
	const char* userConsFile = NULL;
	const char* userConsHelp = NULL;

	if (funcs)
	{
		// exec: <class>.<f.Name> = <f.Func>
		for (FuncEntry* f = funcs; f->name; ++f)
		{
			if (f->name[0] == '!')
			{
				if (userCons == NULL)
				{
					userCons = f->func;
					userConsFile = f->file;
					userConsHelp = f->help;
				}
				else
				{
					sqx_prterrf(v, "*** constructor for '%s' already exists\n", fullClassname);
					continue;
				}
			}
			else
			{
				newFunction(v, fullClassname, f->name, f->func, f->file, f->help, -1, false);
			}
		}
	}

	// register constructor /////////////////
	if (userCons)
	{
		sq_pushstring(v, "constructor", -1);
		sq_newclosure(v, userCons, 0);
		sq_setnativeclosureinfo(v, -1, "constructor", userConsFile);
		sq_pushuserpointer(v, (SQUserPointer)consHook);
		sq_pushuserpointer(v, (SQUserPointer)postConsHook);
		sq_pushuserpointer(v, (SQUserPointer)releaseHook);
		sq_newclosure(v, _dispatchConstructor, 4);
		sq_setnativeclosureinfo(v, -1, "constructor", userConsFile);
		if (userConsHelp)
		{
			sq_pushstring(v, userConsHelp, -1);
			sq_sethelp(v, -2);
		}
		sq_newslot(v, -3, false);
	}
	else
	{
		// Non-constructable
		newFunction(v, fullClassname, "constructor", _nonConstructable, __FILE__, "<prohibited>", -1, false);
	}

	// register properties //////////////////

	if (props)
	{
		for (PropEntry* p = props; p->name; ++p)
		{
			sq_pushstring(v, p->name, -1);

			if (p->getter) 
			{
				sq_newclosure(v, p->getter, 0);
				sq_setnativeclosureinfo(v, -1, p->name, p->file);
			}
			else
				sq_pushnull(v);

			if (p->setter)
			{
				sq_newclosure(v, p->setter, 0);
				sq_setnativeclosureinfo(v, -1, p->name, p->file);
			}
			else
				sq_pushnull(v);

			if (SQ_FAILED(sq_newproperty(v, -4)))
			{
				sq_pop(v, 3);
				sq_getlasterror(v);
				sqx_prterrf(v, "*** cannot register property '%s' for '%s' : %s\n",
					p->name, fullClassname, sqx_getstring(v, -1));
				sq_poptop(v);
			}
		}
	}

	// stack now:
	// [-1] : class
	// [-2] : base class

	// create namespace and assign to it
	sq_pushroottable(v);
	sq_push(v, -2);
	createNamespace(v, fullClassname);
	sq_pop(v, 2);

	// exec: <registry>[<typeTag>] = <class>
	// exec: <registey>[<rawname>] = <class>
	sq_pushregistrytable(v);
	sq_pushuserpointer(v, typeTag);
	sq_push(v, -3);
	sq_newslot(v, -3, false);
	sq_pushstring(v, rawname, -1);
	sq_push(v, -3);
	sq_newslot(v, -3, false);
	sq_poptop(v);

	sq_poptop(v); // <class>
	sq_poptop(v); // <baseclass>

	// stack now:
	// empty
}

void NitBindImpl::_bindMore(HSQUIRRELVM v, const SQChar* fullClassname, PropEntry* props, FuncEntry* funcs)
{
	if (funcs)
	{
		for (FuncEntry* f = funcs; f->name; ++f)
		{
			newFunction(v, fullClassname, f->name, f->func, f->file, f->help, -1, false);
		}
	}

	if (props)
	{
		for (PropEntry* p = props; p->name; ++p)
		{
			sq_pushstring(v, p->name, -1);

			if (p->getter) 
			{
				sq_newclosure(v, p->getter, 0);
				sq_setnativeclosureinfo(v, -1, p->name, p->file);
			}
			else
				sq_pushnull(v);

			if (p->setter)
			{
				sq_newclosure(v, p->setter, 0);
				sq_setnativeclosureinfo(v, -1, p->name, p->file);
			}
			else
				sq_pushnull(v);

			if (SQ_FAILED(sq_newproperty(v, -4)))
			{
				sq_pop(v, 3);
				sq_getlasterror(v);
				sqx_prterrf(v, "*** cannot register property '%s' for '%s' : %s\n",
					p->name, fullClassname, sqx_getstring(v, -1));
				sq_poptop(v);
			}
		}
	}
}

void NitBindImpl::_addInterface(HSQUIRRELVM v, const char* raw, const SQChar* intfRaw, SQUserPointer intfTypeTag, INTF_CAST_FN castFn)
{
	SQRESULT r;
	SQInteger base = sq_gettop(v);

	// Get the concrete class by raw name
	sq_pushregistrytable(v);				// stack: [reg]
	sq_pushstring(v, raw, -1);				// stack: [reg] "raw"
	r = sq_get(v, -2);						// stack: [reg] [class]
	if (SQ_FAILED(r)) 
	{ 
		sq_settop(v, base);
		sqx_prterrf(v, "*** class '%s' is not registered yet\n", raw);
		return;
	}

	// Add interface cast function to interface table (by tag)
	sq_pushuserpointer(v, intfTypeTag);		// stack: [reg] [class] [tag]
	sq_pushuserpointer(v, (SQUserPointer)castFn);			// stack: [reg] [class] [tag] [fn]
	sq_newslot(v, -3, false);				// stack: [reg] [class]

	// Add interface cast function to interface table (by raw name)
	sq_pushstring(v, intfRaw, -1);			// stack: [reg] [class] "raw"
	sq_pushuserpointer(v, (SQUserPointer)castFn);			// stack: [reg] [class] "raw" [fn]
	sq_newslot(v, -3, false);				// stack: [reg] [class] 

	sq_settop(v, base);						// stack: empty
}

bool NitBindImpl::_hasInterface(HSQUIRRELVM v, SQInteger clsIdx, const type_info& ti)
{
	SQRESULT r;
	SQInteger base = sq_gettop(v);

	// find matching intf tag
	SQUserPointer intfTag = (SQUserPointer)&ti;
	sq_pushuserpointer(v, intfTag);					// [class] [tag]
	r = sq_get(v, -2);								// [class] [fn]
	if (SQ_FAILED(r))
	{
		sq_pushstring(v, ti.name(), -1);			// [class] [typecode]
		if (SQ_FAILED(sq_get(v, -2)))
		{
			sq_settop(v, base);
			return false;
		}

		sq_pushuserpointer(v, intfTag);
		sq_push(v, -2);
		sq_newslot(v, -4, true);
		sqx_printf(v, "++ compatible intf typetag %08x for %s is registered and used now on\n", &ti, ti.name());
	}

	sq_settop(v, base);							// stack cleanup

	return true;
}

void* NitBindImpl::_getInterfacePtr(HSQUIRRELVM v, SQInteger idx, const type_info& ti)
{
	SQRESULT r;
	SQInteger base = sq_gettop(v);

	SQUserPointer up; 
	if (SQ_FAILED(sq_getinstanceup(v, idx, &up, NULL))) sqx_throw(v, "invalid object type");

	// get the class
	sq_getclass(v, idx);							// [class]

	// find matching intf tag
	SQUserPointer intfTag = (SQUserPointer)&ti;
	sq_pushuserpointer(v, intfTag);					// [class] [tag]
	r = sq_get(v, -2);								// [class] [fn]
	if (SQ_FAILED(r))
	{
		sq_pushstring(v, ti.name(), -1);			// [class] [typecode]
		if (SQ_FAILED(sq_get(v, -2)))
		{
			sq_settop(v, base);
			return NULL;
		}

		sq_pushuserpointer(v, intfTag);
		sq_push(v, -2);
		sq_newslot(v, -4, true);
		sqx_printf(v, "++ compatible intf typetag %08x for %s is registered and used now on\n", &ti, ti.name());
	}

	INTF_CAST_FN castFn;
	r = sq_getuserpointer(v, -1, (SQUserPointer*)&castFn);
	if (SQ_FAILED(r)) 
	{ 
		sq_settop(v, base); 
		sqx_throwfmt(v, "invalid interface cast: %s", ti.name());
	}

	void* intf = castFn(up);
	sq_settop(v, base);							// stack cleanup

	if (intf == NULL)
		sqx_throwfmt(v, "native interface lost: %s", ti.name());

	return intf;
}

#ifndef iscsym
#define iscsym(c) \
	( ('0' <= (c) && (c) <= '9') || \
	('A' <= (c) && (c) <= 'Z') || \
	('a' <= (c) && (c) <= 'z') || \
    ('_' == (c) ) )
#endif

void NitBindImpl::createNamespace(HSQUIRRELVM v, const char* fullname)
{
	// stack begin condition:
	// [-1] : class to add
	// [-2] : given root table

	sq_push(v, -2);
	size_t fulllen = strlen(fullname);

	if ((fulllen >= 2 && fullname[0] == ':' && fullname[1] == ':') || fullname[0] == '/' )
	{
		sq_poptop(v);
		sq_pushroottable(v);
	}

	const char* c = fullname;
	String ns = "";

	// skip first delimiter
	while (*c && !iscsym(*c)) ++c;

	while (*c)
	{
		// find end position
		const char *s = c, *e = c;
		while (*e && iscsym(*e)) ++e;

		// skip delimiter
		c = e;
		while (*c && !iscsym(*c)) ++c;

		bool last = *c == 0;

		int idlen = e - s;
		if (idlen > 0)
		{
			sq_pushstring(v, s, idlen);
			sq_push(v, -1);

			if (last)
			{
				sq_pushstring(v, "_classname", -1);
				sq_push(v, -2);
				sq_newslot(v, -6, true);
				
				sq_pushstring(v, "_namespace", -1);
				sq_pushstring(v, ns.c_str(), -1);
				sq_newslot(v, -6, true);

				sq_push(v, -4);
				sq_rawset(v, -4);
				sq_pop(v, 2);
				break;
			}
			
			if (!ns.empty())
				ns.push_back('.');
			ns.append(s, idlen);
			
			if (SQ_FAILED(sq_rawget(v, -3)))
			{
				sq_newtable(v);
				sq_push(v, -2);
				sq_push(v, -2);
				sq_rawset(v, -5);
				sq_replace(v, -3);
				sq_poptop(v);
			}
			else
			{
				sq_replace(v, -3);
				sq_poptop(v);
			}
		}
	}

	// stack end condition: (same with begin)
	// [-1] : value to add
	// [-2] : given root table
}

int NitBindImpl::_bindPtrInstance(
	HSQUIRRELVM v, 
	SQObjectRef& outInstance, 
	SQUserPointer ptr, 
	SQUserPointer actualTypeTag,
	SQUserPointer typeTag,
	const SQChar* bindingName,
	const SQChar* typeCode,
	const SQChar* actualTypeCode,
	CONS_HOOK conshook,
	SQRELEASEHOOK relhook)
{
	SQTable* registry = sqi_table(_ss(v)->_registry);

	SQObjectPtr classVal;

	// Acquire already bound class by type tag from type-info
	if (!registry->Get(actualTypeTag, classVal))
	{
		// Try with class name from type-info
		SQObjectPtr codeStr(SQString::Create(_ss(v), actualTypeCode, -1));
		if (!registry->Get(codeStr, classVal))
		{
			// No such a sub-class registered yet.
			// Try with a class which recognized during compile time
			if (!registry->Get(typeTag, classVal))
			{
				// Maybe they omit Bind() by mistake on its Register() impl
				sqx_throwfmt(v, "type '%s' not registered - forgot to call Bind()?", bindingName);
			}
			else
			{
				// Unregistered sub-class of registered base-class
				registry->NewSlot(actualTypeTag, classVal);
				LOG(0, "++ subclass %s (typetag %08x) not registered, using %s instead now on\n", typeName(actualTypeCode).c_str(), actualTypeTag, bindingName);
			}
		}
		else
		{
			// Same class from different DLL
			registry->NewSlot(actualTypeTag, classVal);
			LOG(0, "++ compatible typetag %08x for %s is registered and used now on\n", actualTypeTag, typeName(actualTypeCode).c_str());
		}
	}

	SQObjectPtr instVal;
	
	if (sqi_type(classVal) == OT_CLASS)
	{
		instVal = sqi_class(classVal)->CreateInstance();
	}

	if (sqi_type(instVal) == OT_INSTANCE)
	{
		SQInstance* inst = sqi_instance(instVal);
		inst->_userpointer = ptr;

		outInstance = instVal;
		conshook(v, outInstance, ptr, 0);
		inst->_hook = relhook;

		return 1;
	}

	sqx_throwfmt(v, "cannot instantiate");
	return SQ_ERROR;
}

int NitBindImpl::_bindValueInstance(HSQUIRRELVM v, SQObjectRef& outInstance, SQUserPointer typeTag, const char* bindingName, CONS_HOOK conshook, SQRELEASEHOOK relhook)
{
	SQTable* registry = sqi_table(_ss(v)->_registry);

	SQObjectPtr classVal;

	if (!registry->Get(typeTag, classVal))
		sqx_throwfmt(v, "type not registered: %s", bindingName);

	SQObjectPtr instVal;

	if (sqi_type(classVal) == OT_CLASS)
	{
		instVal = sqi_class(classVal)->CreateInstance();
	}

	if (sqi_type(instVal) == OT_INSTANCE)
	{
		SQInstance* inst = sqi_instance(instVal);

		outInstance = instVal;
		conshook(v, outInstance, inst->_userpointer, 0);
		inst->_hook = relhook;

		return 1;
	}

	sqx_throwfmt(v, "cannot instantiate");
	return SQ_ERROR;
}

void NitBindImpl::_newSlot(HSQUIRRELVM v, SQInteger tblIdx, bool isStatic)
{
	SQInteger keyIdx = toAbsIdx(v, -2);
	SQInteger valueIdx = toAbsIdx(v, -1);

	if (!isStatic || sq_gettype(v, keyIdx) != OT_STRING)
	{
		sq_newslot(v, tblIdx, isStatic);
		return;
	}

	const char* keyStr = NULL;
	sq_getstring(v, keyIdx, &keyStr);

	const char* c = keyStr;
	while (*c && *c != '.') ++c;

	if (*c == 0)
	{
		// no '.'
		sq_newslot(v, tblIdx, isStatic);
		return;
	}

	const char* s = keyStr;
	const char* e = c;

	sq_push(v, tblIdx);						// [tbl] [key] [val] [par=tbl]

	while (true)
	{
		sq_pushstring(v, s, e-s);			// [tbl] [key] [val] [par] "tok"

		if (*e == 0)
		{
			sq_push(v, valueIdx);			// [tbl] [key] [val] [par] "tok" [val]
			sq_newslot(v, -3, true);		// [tbl] [key] [val] [par]                // TODO: Always static, heh?
			sq_pop(v, 3);					// [tbl]
			return;
		}

		if (SQ_SUCCEEDED(sq_get(v, -2)))
		{
			sq_replace(v, -2);				// [tbl] [key] [val] [tok]
		}
		else
		{
			sq_newtable(v);					// [tbl] [key] [val] [par] [tok]
			sq_pushstring(v, s, e-s);		// [tbl] [key] [val] [par] [tok] "tok"
			sq_push(v, -2);					// [tbl] [key] [val] [par] [tok] "tok" [tok]
			sq_newslot(v, -4, true);		// [tbl] [key] [val] [par] [tok]
			sq_replace(v, -2);				// [tbl] [key] [val] [tok]
		}

		s = ++e;
		while (*e && *e != '.') ++e;
	}
}

String NitBindImpl::typeName(const SQChar* typecode)
{
#ifdef NIT_FAMILY_WIN32
	return typecode + 6; // "class "
#else
	return typecode;
#endif
}

////////////////////////////////////////////////////////////////////////////////

NitBindLibRegistry::NitBindLibRegistry(RegisterFn fn, const char* libName, const char* dependancy) : _register(fn), _libName(libName), _dependancy(dependancy), _next(NULL)
{
	_next = add(this);
}

NitBindLibRegistry::~NitBindLibRegistry()
{
	remove(this);
}

NitBindLibRegistry*& NitBindLibRegistry::getHead()
{
	static NitBindLibRegistry* head = NULL;
	return head;
}

NitBindLibRegistry* NitBindLibRegistry::add(NitBindLibRegistry* reg)
{
	NitBindLibRegistry*& head = getHead();
	NitBindLibRegistry* next = head;
	head = reg;

	return next;
}

void NitBindLibRegistry::remove(NitBindLibRegistry* reg)
{
	NitBindLibRegistry*& head = getHead();

	if (head == reg)
	{
		head = reg->_next;
		return;
	}

	NitBindLibRegistry* p = head;
	NitBindLibRegistry* r = head->_next;

	while (r)
	{
		if (reg == r)
		{
			p->_next = r->_next;
			return;
		}

		p = r;
		r = r->_next;
	}

	assert(false);
}

NitBindLibRegistry* NitBindLibRegistry::find(const char* libName, unsigned int nameLen)
{
	for (NitBindLibRegistry* r = getHead(); r; r = r->_next)
	{
		if (strlen(r->_libName) == nameLen && strncmp(r->_libName, libName, nameLen) == 0)
			return r;
	}

	return NULL;
}

bool NitBindLibRegistry::install(HSQUIRRELVM v, NitBindLibRegistry* r)
{
	SQInteger top = sq_gettop(v);

	// check installed
	sq_pushroottable(v);
	NitBind::push(v, "_nitbind");
	sq_get(v, -2);
	NitBind::push(v, "libs");
	sq_get(v, -2);
	NitBind::push(v, r->_libName);
	bool installed = SQ_SUCCEEDED(sq_get(v, -2));
	sq_settop(v, top);

	if (installed)
		return true;

	const char* token = r->_dependancy;

	while (true)
	{
		const char* te = strchr(token, ' ');
		if (te == NULL)
			te = token + strlen(token);

		int len = te - token;
		if (len <= 0)
			break;

		NitBindLibRegistry* dep = NitBindLibRegistry::find(token, len);
		if (dep == NULL)
		{
			// We cannot sure if we can use string because system may not be intialized yet. 
			// Make manual copy of string
			char errtoken[64];
			for (int i=0; i<len; ++i) errtoken[i] = token[i];
			errtoken[len] = 0;

			sqx_prterrf(v, "*** Error: '%s' requires '%s'\n", r->_libName, errtoken);
			return false;
		}

		bool ok = install(v, dep);
		if (!ok)
			return false;

		token += len;

		if (*token == ' ') ++token;
	}

	sqx_printf(v, ".. NitBind: Install '%s'\n", r->_libName);
	r->_register(v);

	// mark to _nitbind.libs
	sq_pushroottable(v);
	NitBind::push(v, "_nitbind");
	sq_get(v, -2);
	NitBind::push(v, "libs");
	sq_get(v, -2);
	NitBind::push(v, r->_libName);
	NitBind::push(v, r->_dependancy);
	sq_newslot(v, -3, false);

	sq_settop(v, top);

	return true;
}

//////////////////////////////////////////////////

NitBindImpl::BindEnv::BindEnv(void* userenv)
: _userEnv(userenv)
{
}

NitBindImpl::BindEnv::~BindEnv()
{
}

void NitBindImpl::BindEnv::install(HSQUIRRELVM v)
{
	_rootVm = v;

	sq_resetobject(&_instanceCache);
	sq_newtable(v);
	sq_getstackobj(v, -1, &_instanceCache);
	sq_addref(v, &_instanceCache);
	sq_poptop(v);
}

void NitBindImpl::BindEnv::uninstall()
{
	if (_rootVm)
	{
		sq_release(_rootVm, &_instanceCache);
		_rootVm = NULL;
	}
}

void NitBindImpl::BindEnv::addCachedPtr(HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p)
{
	if (_rootVm == NULL) return;

	processCacheRemoval();

	SQTable* t = sqi_table(_instanceCache);

	assert(SQ_ISREFCOUNTED(sqi_type(inst)));
	t->NewSlot(p, sqi_refcounted(inst)->GetWeakRef(sqi_type(inst)));
}

SQRESULT NitBindImpl::BindEnv::getCachedInstance(HSQUIRRELVM v, SQUserPointer p, SQUserPointer typeTag, SQObjectRef& outInstance)
{
	if (_rootVm == NULL) return SQ_ERROR;

	typeTag = NULL; // HACK: Disables INVALID_TAG type checking (comment out if checking needed)

	processCacheRemoval();

	SQTable* t = sqi_table(_instanceCache);

	SQObjectPtr key = p;
	SQObjectPtr val;

	if (!t->Get(key, val))
	{
		outInstance._type = OT_NULL;
		return SQ_ERROR;
	}

	outInstance = val;

	SQUserPointer up = outInstance.getInstanceUp(typeTag);

	if (up == p)
		return SQ_OK;

	// type mismatch - out-of-date cache entry (perhaps raw pointer destroyed and reallocated somewhere)
	SQObjectPtr str;
	v->ToString(val, str);

	LOG(0, "*** NitBind: GetCachedPtr(): invalid cached obj purged: %s\n", sqi_stringval(str));

	// purge now
	sqi_instance(val)->_hook = NULL;
	sqi_instance(val)->Purge();

	// and remove from cache
	_removalQueue.push_back(p);

	return sq_throwerror(v, _SC("invalid type tag"));
}

SQRESULT NitBindImpl::BindEnv::getCachedInstance(HSQUIRRELVM v, SQUserPointer p, SQUserPointer typeTag)
{
	SQObjectRef inst;
	SQRESULT sr = getCachedInstance(v, p, typeTag, inst);
	if (SQ_FAILED(sr)) return sr;

	v->Push(inst);
	return 1;
}

void NitBindImpl::BindEnv::removeCachedPtr(SQUserPointer p)
{
	if (_rootVm == NULL) return;

	_removalQueue.push_back(p);
	NB_LOG(0, v, "queue remove: %08x\n", p);
}

void NitBindImpl::BindEnv::processCacheRemoval()
{
	// assume that _instanceCache on top of stack
	SQTable* t = sqi_table(_instanceCache);

	while (!_removalQueue.empty())
	{
		SQUserPointer p = _removalQueue.back();
		_removalQueue.pop_back();

		NB_LOG(0, v, "cache remove: %08x\n", p);

		SQObjectPtr key = p;
		t->Remove(p);
	}
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
