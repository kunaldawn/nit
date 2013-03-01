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

#include "nit/script/NitLibCore.h"
#include "nit/script/NitBindCom.h"

#include "nit/script/NitBind.h"
#include "nit/script/NitBindMacro.h"

#include "squirrel/sqstate.h"
#include "squirrel/sqobject.h"
#include "squirrel/sqtable.h"
#include "squirrel/sqvm.h"
#include "squirrel/sqclass.h"

#include <OCIdl.h>

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

typedef NitBindCom com;

NB_TYPE_REF(NIT_API, com, NULL, incRefCount, decRefCount);

class NitLibCom : public TNitClass<NitBindCom>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(typeLibs),
			PROP_ENTRY_R(types),
			PROP_ENTRY_R(instances),
			PROP_ENTRY	(verbose),
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY_H(create,		"(progid: string): com.Dispatch"),
			FUNC_ENTRY_H(open,			"(url: string): com.Dispatch"),

			FUNC_ENTRY_H(toArray,		"(enum: IEnumVARIANT): array"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(verbose)				{ return push(v, self(v)->isVerbose()); }
	NB_PROP_SET(verbose)				{ self(v)->setVerbose(getBool(v, 2)); return 0; }

	NB_FUNC(toArray)
	{
		IEnumVARIANT* en = NitBindCom::getEnum(v, 2);
		if (en == NULL)
			return sq_throwerror(v, "IEnumVARIANT expected");

		HRESULT hr = en->Reset();
		if (FAILED(hr))
			return sq_throwerror(v, "can't reset enum");

		sq_newarray(v, 0);
		while (true)
		{
			NitBindCom::Variant var;
			ULONG numElem = 0;
			HRESULT hr = en->Next(1, var, &numElem);
			if (numElem == 0)
				break;

			var.push(v);
			sq_arrayappend(v, -2);
		}

		return 1;
	}

	NB_PROP_GET(typeLibs)
	{
		NitBindCom* o = self(v);

		sq_newarray(v, 0);
		for (NitBindCom::TypeLibCache::iterator itr = o->_typeLibCache.begin(), end = o->_typeLibCache.end(); itr != end; ++itr)
		{
			o->push(v, itr->first);
			sq_arrayappend(v, -2);
		}

		return 1;
	}

	NB_PROP_GET(types)
	{
		NitBindCom* o = self(v);

		sq_newarray(v, 0);
		for (NitBindCom::TypeCache::iterator itr = o->_typeCache.begin(), end = o->_typeCache.end(); itr != end; ++itr)
		{
			o->push(v, itr->first);
			sq_arrayappend(v, -2);
		}

		return 1;
	}

	NB_PROP_GET(instances)
	{
		NitBindCom* o = self(v);

		sq_newarray(v, 0);
		for (NitBindCom::DispCache::iterator itr = o->_dispCache.begin(), end = o->_dispCache.end(); itr != end; ++itr)
		{
			o->push(v, itr->first);
			sq_arrayappend(v, -2);
		}

		return 1;
	}

	NB_FUNC(create)						{ NitBindCom* o = self(v); return o->push(v, o->create(getString(v, 2))); }
	NB_FUNC(open)						{ NitBindCom* o = self(v); return o->push(v, o->open(getString(v, 2))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, com::TypeLib, NULL, incRefCount, decRefCount);

class NitBindComTypeLib : public TNitClass<NitBindCom::TypeLib>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(name),
			PROP_ENTRY_R(guid),
			PROP_ENTRY_R(version),
			PROP_ENTRY_R(consts),
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY_H(find,			"(pattern: string): string[]"),
			FUNC_ENTRY_H(typeof,		"(name: string): TypeInfo"),
			FUNC_ENTRY_H(allTypes,		"(): { name = TypeInfo, ... }"),

			FUNC_ENTRY_H(purge,			"() // purges this and all related classes/instances"),

			FUNC_ENTRY	(_get),
			FUNC_ENTRY	(_tostring),
			NULL
		};

		bind(v, props, funcs);

		pushClass<type>(v);

		sq_pushstring(v, "_types", -1);
		sq_pushnull(v);
		sq_newslot(v, -3, false);

		sq_pushstring(v, "_consts", -1);
		sq_pushnull(v);
		sq_newslot(v, -3, false);

		sq_poptop(v);
	}

	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }

	NB_PROP_GET(guid)					
	{ 
		type* o = self(v);
		ITypeLib* tlib = o->getPeer();

		TLIBATTR* attr = NULL;
		tlib->GetLibAttr(&attr);
		if (attr == NULL) 
			return 0;

		push(v, NitBindCom::toString(attr->guid));
		tlib->ReleaseTLibAttr(attr);
		return 1;
	}

	NB_PROP_GET(version)
	{
		type* o = self(v);
		ITypeLib* tlib = o->getPeer();

		TLIBATTR* attr = NULL;
		tlib->GetLibAttr(&attr);
		if (attr == NULL)
			return 0;

		pushFmt(v, "%d.%d", attr->wMajorVerNum, attr->wMinorVerNum);
		tlib->ReleaseTLibAttr(attr);
		return 1;
	}

	NB_FUNC(_tostring)
	{
		type* o = self(v);
		ITypeLib* tlib = o->getPeer();

		TLIBATTR* attr = NULL;
		tlib->GetLibAttr(&attr);
		if (attr == NULL)
			return pushFmt(v, "(TypeLib '%s' %s -> %08x)", o->getName().c_str(), tlib);

		pushFmt(v, "(TypeLib '%s' v%d.%d -> %08x)", o->getName().c_str(), attr->wMajorVerNum, attr->wMinorVerNum, tlib);
		tlib->ReleaseTLibAttr(attr);
		return 1;
	}

	NB_FUNC(purge)						
	{ 
		self(v)->purge(v);
		sq_purgeinstance(v, 1);
		return 0;
	}

	NB_FUNC(allTypes)
	{
		sq_pushstring(v, "_types", -1);
		sq_get(v, 1);
		if (sq_gettype(v, -1) != OT_NULL)
			return 1;

		sq_newtable(v);

		ITypeLib* tl = self(v)->getPeer();
		NitBindCom* com = self(v)->getBindCom();

		UINT tc = tl->GetTypeInfoCount();

		for (UINT i=0; i<tc; ++i)
		{
			ComRef<ITypeInfo> ti;
			tl->GetTypeInfo(i, &ti.get());

			if (ti == NULL) continue;

			int top = sq_gettop(v);
			SQRESULT sr;

			sq_pushnull(v);
			sr = com->push(v, ti.get());

			if (SQ_FAILED(sr))
			{
				sq_settop(v, top);
				continue;
			}

			NitBindCom::TypeInfo* typeinfo = NULL;
			sq_gettypetag(v, -1, (SQUserPointer*)&typeinfo);
			assert(typeinfo);

			push(v, typeinfo->getName());
			sq_replace(v, -3);

			sq_newslot(v, -3, false);
		}

		sq_pushstring(v, "_types", -1);
		sq_push(v, -2);
		sq_set(v, 1);

		return 1;
	}

	NB_PROP_GET(consts)
	{
		sq_pushstring(v, "_consts", -1);
		sq_get(v, 1);
		if (sq_gettype(v, -1) != OT_NULL)
			return 1;

		sq_newtable(v);

		ITypeLib* tl = self(v)->getPeer();
		NitBindCom* com = self(v)->getBindCom();

		UINT tc = tl->GetTypeInfoCount();

		HRESULT hr;

		for (UINT i=0; i<tc; ++i)
		{
			ComRef<ITypeInfo> ti;
			tl->GetTypeInfo(i, &ti.get());

			if (ti == NULL) continue;

			TYPEATTR* attr = NULL;
			ti->GetTypeAttr(&attr);

			if (attr == NULL) continue;

			for (UINT vi = 0; vi < attr->cVars; ++vi)
			{
				VARDESC* vd = NULL;
				hr = ti->GetVarDesc(vi, &vd);
				if (FAILED(hr))
					continue;

				if (vd->varkind != VAR_CONST)
				{
					ti->ReleaseVarDesc(vd);
					continue;
				}

				BSTR _name = NULL;
				UINT found = 0;
				ti->GetNames(vd->memid, &_name, 1, &found);
				if (SUCCEEDED(hr))
				{
					String varname = NitBindCom::toString(_name);
					SysFreeString(_name);

					NitBindCom::Variant var = *vd->lpvarValue;

					NitBind::push(v, varname);
					var.push(v);
					sq_newslot(v, -3, true);
				}

				ti->ReleaseVarDesc(vd);
			}

			ti->ReleaseTypeAttr(attr);
		}

		sq_pushstring(v, "_consts", -1);
		sq_push(v, -2);
		sq_set(v, 1);

		return 1;
	}

	NB_FUNC(find)
	{
		const String& pattern = getString(v, 2);

		ITypeLib* tl = self(v)->getPeer();
		NitBindCom* com = self(v)->getBindCom();

		UINT tc = tl->GetTypeInfoCount();

		sq_newarray(v, 0);
		for (UINT i=0; i<tc; ++i)
		{
			ComRef<ITypeInfo> ti;
			tl->GetTypeInfo(i, &ti.get());

			if (ti == NULL) continue;

			BSTR _name = NULL;
			ti->GetDocumentation(MEMBERID_NIL, &_name, NULL, NULL, NULL);

			if (_name == NULL)
				continue;

			TYPEATTR* attr = NULL;
			ti->GetTypeAttr(&attr);

			String name = NitBindCom::typeAttrStr(attr);
			name.push_back(' ');
			name.append(NitBindCom::toString(_name));
			SysFreeString(_name);

			if (Wildcard::match(pattern, name, true))
			{
				NitBind::push(v, name);
				sq_arrayappend(v, -2);
			}

			ti->ReleaseTypeAttr(attr);
		}

		return 1;
	}

	NB_FUNC(typeof)
	{
		type* o = self(v);
		ITypeComp* comp = o->getComp();

		UniString bname = Unicode::toUtf16(getString(v, 2));
		ULONG hashVal = LHashValOfName(NitBindCom::getLocaleID(), (LPOLESTR)bname.c_str());

		ComRef<ITypeInfo> ti;
		HRESULT hr = comp->BindType((LPOLESTR)bname.c_str(), hashVal, &ti.get(), NULL);
		if (FAILED(hr))
			return sq_throwerror(v, NitBindCom::errorStr(hr).c_str());

		if (ti)
			return o->getBindCom()->push(v, ti);

		ti = NULL;
		DESCKIND desckind = DESCKIND_NONE;
		BINDPTR bindptr = { 0 };
		hr = comp->Bind((LPOLESTR)bname.c_str(), hashVal, INVOKE_PROPERTYGET, &ti.get(), &desckind, &bindptr);

		switch (desckind)
		{
		case DESCKIND_NONE:				
			return 0;

		case DESCKIND_TYPECOMP:
			NitBindCom::get(v)->push(v, ti);
			bindptr.lptcomp->Release();
			return 1;

		case DESCKIND_FUNCDESC:
			NitBindCom::get(v)->push(v, ti);
			ti->ReleaseFuncDesc(bindptr.lpfuncdesc);
			return 1;

		case DESCKIND_VARDESC:
			NitBindCom::get(v)->push(v, ti);
			ti->ReleaseVarDesc(bindptr.lpvardesc);
			return 1;

		default:
			return sq_throwerror(v, "not supported binding");
		}
	}

	NB_FUNC(_get)
	{
		type* o = self_noThrow(v);
		if (o == NULL)
		{
			sq_reseterror(v);
			return SQ_ERROR;
		}

		ITypeComp* comp = o->getComp();

		UniString bname = Unicode::toUtf16(getString(v, 2));
		ULONG hashVal = LHashValOfName(NitBindCom::getLocaleID(), (LPOLESTR)bname.c_str());

		ComRef<ITypeInfo> ti;
		HRESULT hr = comp->BindType((LPOLESTR)bname.c_str(), hashVal, &ti.get(), NULL);
		if (FAILED(hr))
		{
			sq_reseterror(v);
			return SQ_ERROR;
		}

		if (ti)
			return o->getBindCom()->push(v, ti);

		ti = NULL;
		DESCKIND desckind = DESCKIND_NONE;
		BINDPTR bindptr = { 0 };
		hr = comp->Bind((LPOLESTR)bname.c_str(), hashVal, INVOKE_PROPERTYGET, &ti.get(), &desckind, &bindptr);
		
		switch (desckind)
		{
		case DESCKIND_NONE:
			break;

		case DESCKIND_TYPECOMP:
			NitBindCom::get(v)->push(v, ti);
			bindptr.lptcomp->Release();
			return 1;

		case DESCKIND_FUNCDESC:
			ti->ReleaseFuncDesc(bindptr.lpfuncdesc);
			break;

		case DESCKIND_VARDESC:
			{
				VARDESC* vd = bindptr.lpvardesc;
				if (vd->varkind != VAR_CONST)
				{
					ti->ReleaseVarDesc(vd);
					return sq_throwerror(v, "not supported var");
				}

				NitBindCom::Variant var = *vd->lpvarValue;
				var.push(v);

				ti->ReleaseVarDesc(vd);
				return 1;
			}

		default:
			return sq_throwerror(v, "not supported binding");
		}

		sq_reseterror(v);
		return SQ_ERROR;
	}
};

////////////////////////////////////////////////////////////////////////////////

SQRESULT NitLibCom(HSQUIRRELVM v)
{
	NitLibCom::Register(v);
	NitBindComTypeLib::Register(v);

	NitBindCom* com = new NitBindCom(v);
	sq_pushroottable(v);
	sq_pushstring(v, "com", -1);
	NitBind::push(v, com);
	sq_newslot(v, -3, true);
	sq_poptop(v);

	// Prevent deletion from registry
	sq_pushregistrytable(v);
	sq_pushuserpointer(v, com);
	NitBind::push(v, com);
	sq_newslot(v, -3, true);
	sq_poptop(v);

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
