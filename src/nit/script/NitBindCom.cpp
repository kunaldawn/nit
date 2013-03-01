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

#include "nit/script/NitBindCom.h"

#include "squirrel/sqstate.h"
#include "squirrel/sqobject.h"
#include "squirrel/sqtable.h"
#include "squirrel/sqvm.h"
#include "squirrel/sqclass.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NitBindCom::NitBindCom(HSQUIRRELVM v)
: _vm(v)
{
//	_localeID = MAKELCID(LANG_ENGLISH, SORT_DEFAULT);
//	_localeID = LOCALE_NEUTRAL;

	// TODO: Check if needed CoInitialize()
//	CoInitialize(NULL);

	_verbose = false;

	// Link to static Get()
	sq_pushregistrytable(v);
	sq_pushuserpointer(v, get);
	sq_pushuserpointer(v, this);
	sq_newslot(v, -3, true);
	sq_poptop(v);
}

NitBindCom::~NitBindCom()
{
//	CoUninitialize(NULL);
}

String NitBindCom::errorStr(HRESULT hr)
{
	LPWSTR lpMsgBuf;

	DWORD result = FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		hr,
		getLocaleID(),
		(LPWSTR)&lpMsgBuf,
		0,
		NULL);

	if (result == 0)
		return StringUtil::format("unknown COM error(%08x)", hr);

	String msg = Unicode::toUtf8(lpMsgBuf);
	LocalFree(lpMsgBuf);

	StringUtil::trim(msg);
	return msg;
}

String NitBindCom::toString(BSTR bstr)
{
	if (bstr == NULL) return "";

	return Unicode::toUtf8(bstr);
}

String NitBindCom::toString(const GUID& guid)
{
	UniChar guidstr[MAX_PATH];
	StringFromGUID2(guid, guidstr, MAX_PATH);
	return Unicode::toUtf8(guidstr);
}

String NitBindCom::toString(const VARIANT& var)
{
	if (var.vt == VT_BSTR)
		return toString(var.bstrVal);

	VARIANT strvar;
	VariantInit(&strvar);
	VariantChangeType(&strvar, &var, 0, VT_BSTR);
	
	String ret = toString(strvar.bstrVal);
	VariantClear(&strvar);

	return ret;
}

BSTR NitBindCom::toBstr(const char* str, size_t len)
{
	BSTR bstr;
	if(len == 0)
	{
		bstr = SysAllocStringLen(NULL, 0);
	}
	else
	{
		int lenWide =
			MultiByteToWideChar(CP_UTF8, 0, str, static_cast<int>(len), NULL, 0);

		bstr = SysAllocStringLen(NULL, lenWide); // plus initializes '\0' terminator
		MultiByteToWideChar(CP_UTF8, 0, str, static_cast<int>(len), bstr, lenWide);
	}
	return bstr;
}

String NitBindCom::typeDescStr(ITypeInfo* typeinfo, const TYPEDESC& tdesc)
{
	char buffer[200];
	buffer[0] = '\0';

	switch(tdesc.vt & ~(VT_ARRAY | VT_BYREF))
	{
	case VT_EMPTY:					strcat(buffer, "empty"); break;
	case VT_VOID:					strcat(buffer, "void"); break;
	case VT_I1:						strcat(buffer, "int8"); break;
	case VT_I2:						strcat(buffer, "int16"); break;
	case VT_I4:						strcat(buffer, "int32"); break;
	case VT_I8:						strcat(buffer, "int64"); break;
	case VT_R4:						strcat(buffer, "float"); break;
	case VT_R8:						strcat(buffer, "double"); break;
	case VT_CY:						strcat(buffer, "CURRENCY"); break;
	case VT_DATE:					strcat(buffer, "DATE"); break;
	case VT_BSTR:					strcat(buffer, "BSTR"); break;
	case VT_DISPATCH:				strcat(buffer, "IDispatch"); break;
	case VT_BOOL:					strcat(buffer, "BOOL"); break;
	case VT_VARIANT:				strcat(buffer, "VARIANT"); break;
	case VT_UNKNOWN:				strcat(buffer, "IUnknown"); break;
	case VT_DECIMAL:				strcat(buffer, "DECIMAL"); break;
	case VT_UI1:					strcat(buffer, "uint8"); break;
	case VT_UI2:					strcat(buffer, "uint16"); break;
	case VT_UI4:					strcat(buffer, "uint32"); break;
	case VT_UI8:					strcat(buffer, "uint64"); break;
	case VT_INT:					strcat(buffer, "int"); break;
	case VT_UINT:					strcat(buffer, "uint"); break;
	case VT_INT_PTR:				strcat(buffer, "int_ptr"); break;
	case VT_UINT_PTR:				strcat(buffer, "uint_ptr"); break;
	case VT_HRESULT:				strcat(buffer, "void"); break;
	case VT_PTR:					strcat(buffer, "*"); break;

	case VT_USERDEFINED:		
		{
			ITypeInfo* userdef = NULL;
			HRESULT hr = typeinfo->GetRefTypeInfo(tdesc.hreftype, &userdef);
			if (FAILED(hr))
				strcat(buffer, "<unknown userdef>");
			else
			{
				TYPEATTR* typeattr = NULL;
				userdef->GetTypeAttr(&typeattr);
				String tname;
				BSTR _name = NULL;
				userdef->GetDocumentation(MEMBERID_NIL, &_name, NULL, NULL, NULL);
				if (_name)
				{
					tname.push_back(' ');
					tname += toString(_name);
					SysFreeString(_name);
				}
				switch (typeattr->typekind)
				{
				case TKIND_ENUM:
					sprintf(buffer, "enum%s", tname.c_str()); break;

				case TKIND_DISPATCH:
					sprintf(buffer, "dispatch%s", tname.c_str()); break;

				case TKIND_ALIAS:
					sprintf(buffer, "alias%s", tname.c_str()); break;

				case TKIND_INTERFACE:
					sprintf(buffer, "interface%s", tname.c_str()); break;

				case TKIND_UNION:
					sprintf(buffer, "union%s", tname.c_str()); break;

				case TKIND_COCLASS:
					sprintf(buffer, "coclass%s", tname.c_str()); break;

				case TKIND_RECORD:
					sprintf(buffer, "record%s", tname.c_str()); break;

				case TKIND_MODULE:
					sprintf(buffer, "module%s", tname.c_str()); break;
				}
				userdef->ReleaseTypeAttr(typeattr);
				userdef->Release();
			}
		}
		break;

	default:
		sprintf(buffer, "type(%d)", tdesc.vt);
	}

	if(tdesc.vt & VT_BYREF)
		strcat(buffer, " byref");

	if(tdesc.vt & VT_ARRAY)
		strcat(buffer, " array");

	if (tdesc.vt == VT_PTR)
		return typeDescStr(typeinfo, *tdesc.lptdesc) + buffer;

	return buffer;
}

String NitBindCom::paramFlagsStr(ushort flags)
{
	String ret;

	if (flags & PARAMFLAG_FIN)
		ret += "in ";
	if (flags & PARAMFLAG_FOUT)
		ret += "out ";
	if (flags & PARAMFLAG_FLCID)
		ret += "lcid ";
	if (flags & PARAMFLAG_FRETVAL)
		ret += "retval ";
	if (flags & PARAMFLAG_FOPT)
		ret += "opt ";
	if (flags & PARAMFLAG_FHASDEFAULT)
		ret += "hasdefault ";
	if (flags & PARAMFLAG_FHASCUSTDATA)
		ret += "hascustdata ";

	return ret;
}

ComRef<IDispatch> NitBindCom::create(const String& progid)
{
	CLSID clsid = IID_NULL;
	HRESULT hr = toClsId(progid, clsid);

	if (FAILED(hr))
		NIT_THROW_FMT(EX_SYSTEM, "can't create com object '%s': %s", progid.c_str(), errorStr(hr).c_str());

	ComRef<IDispatch> disp;
	DWORD context = CLSCTX_SERVER;

	hr = CoCreateInstance(clsid, NULL, context, IID_IDispatch, (void**)&disp.get());

	if (FAILED(hr))
		NIT_THROW_FMT(EX_SYSTEM, "can't create com object '%s': %s", progid.c_str(), errorStr(hr).c_str());

	ComRef<IPersistStreamInit> psi;
	hr = disp->QueryInterface(IID_IPersistStreamInit, (void**)&psi.get());

	if (SUCCEEDED(hr))
		psi->InitNew();

	return disp;
}

ComRef<IDispatch> NitBindCom::open(const String& url)
{
	ComRef<IDispatch> disp;
	HRESULT hr = CoGetObject(Unicode::toUtf16(url).c_str(), NULL, IID_IDispatch, (void**)&disp.get());

	if (FAILED(hr))
		NIT_THROW_FMT(EX_SYSTEM, "can't open com object from '%s': %s", url.c_str(), errorStr(hr).c_str());

	ComRef<IPersistStreamInit> psi;
	hr = disp->QueryInterface(IID_IPersistStreamInit, (void**)&psi.get());

	if (SUCCEEDED(hr))
		psi->InitNew();

	return disp;
}

SQInteger NitBindCom::push(HSQUIRRELVM v, IDispatch* disp)
{
	if (disp == NULL)
	{
		sq_pushnull(v);
		return 1;
	}

	DispCache::iterator itr = _dispCache.find(disp);
	if (itr != _dispCache.end())
	{
		sq_pushobject(v, SQObjectPtr(itr->second));
		return 1;
	}

	ComRef<ITypeInfo> ti;
	HRESULT hr = disp->GetTypeInfo(0, getLocaleID(), &ti.get());
	if (FAILED(hr))
		NIT_THROW_FMT(EX_SYSTEM, "can't obtain typeinfo");

	ComRef<ITypeLib> tl;
	UINT tiIndex = 0;
	hr = ti->GetContainingTypeLib(&tl.get(), &tiIndex);
	if (FAILED(hr))
		NIT_THROW_FMT(EX_SYSTEM, "can't obtain type lib");

	push(v, tl);

	push(v, ti);
	if (SQ_FAILED(sq_createinstance(v, -1)))
		NIT_THROW_FMT(EX_SYSTEM, "can't instantiate");

	sq_setinstanceup(v, -1, disp);
	sq_setreleasehook(v, -1, dispRelease);
	disp->AddRef();

	HSQOBJECT o;
	sq_getstackobj(v, -1, &o);
	assert(sqi_type(o) == OT_INSTANCE);

	_dispCache.insert(std::make_pair(disp, sqi_instance(o)));

	TypeInfo* typeinfo = NULL;
	sq_gettypetag(v, -1, (SQUserPointer*)&typeinfo);

	sq_pushstring(v, "TypeLib", -1);
	sq_push(v, -4);
	sq_set(v, -3);
	sq_replace(v, -3);
	sq_poptop(v);

	if (isVerbose())
		LOG(0, ".. [NitBindCom] Cached: IDispatch %08x -> '%s.%s'\n",
			disp, typeinfo->getTypeLib()->getName().c_str(), typeinfo->getName().c_str());

	return 1;
}

SQInteger NitBindCom::push(HSQUIRRELVM v, ITypeInfo* ti)
{
	TypeCache::iterator itr = _typeCache.find(ti);
	if (itr != _typeCache.end())
	{
		sq_pushobject(v, SQObjectPtr(itr->second));
		return 1;
	}

	ComRef<ITypeLib> tl;
	UINT tiIndex = 0;
	HRESULT hr = ti->GetContainingTypeLib(&tl.get(), &tiIndex);
	if (FAILED(hr))
		NIT_THROW_FMT(EX_SYSTEM, "can't obtain type lib");

	push(v, tl);

	TypeLib* typelib = NULL;
	sq_getinstanceup(v, -1, (SQUserPointer*)&typelib, NULL);
	assert(typelib);

	sq_poptop(v);

	TypeInfo* typeinfo = new TypeInfo(ti, typelib);
	SQRESULT sr = typeinfo->createClass(v);

	if (SQ_FAILED(sr))
		return sr;

	HSQOBJECT o;
	sq_getstackobj(v, -1, &o);
	assert(sqi_type(o) == OT_CLASS);

	_typeCache.insert(std::make_pair(ti, sqi_class(o)));

	typelib->remember(typeinfo, o);

	if (isVerbose())
 		LOG(0, ".. [NitBindCom] Cached: ITypeInfo %08x -> '%s.%s'\n",
 			ti, typeinfo->getTypeLib()->getName().c_str(), typeinfo->getName().c_str());

	return 1;
}

SQInteger NitBindCom::push(HSQUIRRELVM v, ITypeLib* tl)
{
	TypeLibCache::iterator itr = _typeLibCache.find(tl);
	if (itr != _typeLibCache.end())
	{
		sq_pushobject(v, SQObjectPtr(itr->second));
		return 1;
	}

	TypeLib* typelib = new TypeLib(this, tl);

	NitBind::push(v, typelib);

	HSQOBJECT o;
	sq_getstackobj(v, -1, &o);
	assert(sqi_type(o) == OT_INSTANCE);

	_typeLibCache.insert(std::make_pair(tl, sqi_instance(o)));

	if (isVerbose())
		LOG(0, ".. [NitBindCom] Cached: ITypeLib %08x -> '%s'\n",
			tl, typelib->getName().c_str());

	return 1;
}

SQInteger NitBindCom::push(HSQUIRRELVM v, IEnumVARIANT* en)
{
	en->AddRef();
	IEnumVARIANT*& p = *(IEnumVARIANT**)sq_newuserdata(v, sizeof(en));
	p = en;
	sq_settypetag(v, -1, (SQUserPointer)&IID_IEnumVARIANT);
	sq_setreleasehook(v, -1, enumRelease);

	return 1;
}

IEnumVARIANT* NitBindCom::getEnum(HSQUIRRELVM v, SQInteger idx)
{
	SQUserPointer p = NULL;
	SQUserPointer typetag = NULL;
	sq_getuserdata(v, idx, (SQUserPointer*)&p, &typetag);

	if (typetag == (SQUserPointer)&IID_IEnumVARIANT)
	{
		IEnumVARIANT* en = *(IEnumVARIANT**)p;
		return en;
	}

	return NULL;
}

SQRESULT NitBindCom::enumRelease(HSQUIRRELVM v, SQUserPointer p, SQInteger size)
{
	IEnumVARIANT* en = *(IEnumVARIANT**)p;
	if (en) en->Release();

	return SQ_OK;
}

SQRESULT NitBindCom::dispRelease(HSQUIRRELVM v, SQUserPointer p, SQInteger size)
{
	IDispatch* disp = (IDispatch*)p;
	NitBindCom* NitBindCom = NitBindCom::get(v);

	if (NitBindCom)
	{
		NitBindCom->uncache(disp);

		if (NitBindCom->isVerbose())
			LOG(0, ".. [NitBindCom] Uncached: IDispatch %08x\n", disp);
	}

	disp->Release();
	return SQ_OK;
}

void NitBindCom::uncache(ITypeLib* typelib)
{
	_typeLibCache.erase(typelib);
}

void NitBindCom::uncache(ITypeInfo* typeinfo)
{
	_typeCache.erase(typeinfo);
}

void NitBindCom::uncache(IDispatch* disp)
{
	_dispCache.erase(disp);
}

HRESULT NitBindCom::toClsId(const String& progID, CLSID& outClsID)
{
	if (!progID.empty() && progID[0] == '{')
	{
		return CLSIDFromString( (LPOLESTR)Unicode::toUtf16(progID).c_str(), &outClsID);
	}
	else
	{
		return CLSIDFromProgID( (LPOLESTR)Unicode::toUtf16(progID).c_str(), &outClsID);
	}
}

String NitBindCom::typeAttrStr(TYPEATTR* attr)
{
	String tkind;

	switch (attr->typekind)
	{
	case TKIND_ENUM:		tkind = "enum"; break;			// enum
	case TKIND_RECORD:		tkind = "record"; break;		// struct
	case TKIND_MODULE:		tkind = "module"; break;		// module
	case TKIND_INTERFACE:	tkind = "interface"; break;		// interface
	case TKIND_DISPATCH:	tkind = "dispatch"; break;		// dispinterface
	case TKIND_COCLASS:		tkind = "coclass"; break;		// coclass
	case TKIND_ALIAS:		tkind = "alias"; break;			// typedef
	case TKIND_UNION:		tkind = "union"; break;			// union
	default:				tkind = StringUtil::format("typekind(%d)", attr->typekind);
	}

	return tkind;
}

String NitBindCom::paramsDescStr(ITypeInfo* typeinfo, FUNCDESC* fd)
{
	UINT found = 0;
	vector<BSTR>::type names(fd->cParams+1);

	typeinfo->GetNames(fd->memid, &names[0], fd->cParams+1, &found);

	String line = "(";

	for (int p = 0; p < fd->cParams; ++p)
	{
		if (p != 0)
			line += ", ";

		const PARAMDESC& pd = fd->lprgelemdescParam[p].paramdesc;
		const TYPEDESC& td = fd->lprgelemdescParam[p].tdesc;

		line += toString(names[p+1]);
		line += ": ";
		line += paramFlagsStr(pd.wParamFlags);
		line += typeDescStr(typeinfo, td);
	}

	line += "): ";
	line += paramFlagsStr(fd->elemdescFunc.paramdesc.wParamFlags);
	line += typeDescStr(typeinfo, fd->elemdescFunc.tdesc);

	for (int n=0; n < fd->cParams+1; ++n)
	{
		SysFreeString(names[n]);
	}

	return line;
}

String NitBindCom::propGetRetStr(ITypeInfo* typeinfo, FUNCDESC* fd)
{
	String line = ": ";
	if (fd->cParams == 1)
	{
		line += paramFlagsStr(fd->lprgelemdescParam[0].paramdesc.wParamFlags);
		line += typeDescStr(typeinfo, fd->lprgelemdescParam[0].tdesc);
	}
	else
	{
		assert(fd->cParams == 0);
		line += paramFlagsStr(fd->elemdescFunc.paramdesc.wParamFlags);
		line += typeDescStr(typeinfo, fd->elemdescFunc.tdesc);
	}

	return line;
}

String NitBindCom::funcDescStr(ITypeInfo* typeinfo, FUNCDESC* fd)
{
	UINT found = 0;
	BSTR _name;

	typeinfo->GetNames(fd->memid, &_name, 1, &found);

	String line;

	switch (fd->invkind)
	{
	case INVOKE_PROPERTYGET:	line = "property get "; break;
	case INVOKE_PROPERTYPUT:	line = "property put "; break;
	case INVOKE_PROPERTYPUTREF:	line = "property putref "; break;
	case INVOKE_FUNC:			line = "function "; break;
	default:					line = "??? ";
	}

	line += StringUtil::format("[%d] ", fd->memid);
	line += toString(_name);
	line += paramsDescStr(typeinfo, fd);

	SysFreeString(_name);

	return line;
}

String NitBindCom::varDescStr(ITypeInfo* typeinfo, VARDESC* vd)
{
	UINT found = 0;
	BSTR name;
	typeinfo->GetNames(vd->memid, &name, 1, &found);

	String line;

	switch (vd->varkind)
	{
	case VAR_PERINSTANCE:		line = "var perinstance "; break;
	case VAR_STATIC:			line = "var static "; break;
	case VAR_CONST:				line = "var const "; break;
	case VAR_DISPATCH:			line = "var dispatch "; break;
	default:					line = "??? ";
	}

	line += StringUtil::format("[%d] ", vd->memid);
	line += toString(name);
	line += ": ";
	line += paramFlagsStr(vd->elemdescVar.paramdesc.wParamFlags);
	line += typeDescStr(typeinfo, vd->elemdescVar.tdesc);

	SysFreeString(name);

	return line;
}

NitBindCom* NitBindCom::get(HSQUIRRELVM v)
{
	// TODO: Hardwire to env or runtime
	sq_pushregistrytable(v);
	sq_pushuserpointer(v, NitBindCom::get);
	if (SQ_SUCCEEDED(sq_get(v, -2)))
	{
		NitBindCom* NitBindCom = NULL;
		sq_getuserpointer(v, -1, (SQUserPointer*)&NitBindCom);
		sq_pop(v, 2);
		return NitBindCom;
	}
	else
	{
		sq_poptop(v);
		return NULL;
	}
}

void NitBindCom::getInstancesOf(TypeLib* typelib, vector<IDispatch*>::type& outResults, HSQUIRRELVM v)
{
	for (DispCache::iterator itr = _dispCache.begin(), end = _dispCache.end(); itr != end; ++itr)
	{
		IDispatch* disp = itr->first;

		if (v)
		{
			push(v, itr->first);
			TypeInfo* ti = NULL;
			sq_gettypetag(v, -1, (SQUserPointer*)&ti);
			if (ti && ti->getTypeLib() == typelib)
				outResults.push_back(itr->first);
			sq_poptop(v);
		}
		else
		{
			ComRef<ITypeInfo> ti;
			ComRef<ITypeLib> tl;
			UINT tiIndex;

			disp->GetTypeInfo(0, getLocaleID(), &ti.get());
			if (ti)
				ti->GetContainingTypeLib(&tl.get(), &tiIndex);

			if (tl == typelib->getPeer())
				outResults.push_back(itr->first);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

NitBindCom::TypeLib::TypeLib(NitBindCom* bindCom, ITypeLib* tl)
: _peer(tl), _bindCom(bindCom)
{
	BSTR bname = NULL;

	tl->GetDocumentation(MEMBERID_NIL, &bname, NULL, NULL, NULL);
	if (bname)
	{
		_name = toString(bname);
		SysFreeString(bname);
	}

	tl->GetTypeComp(&_comp.get());
}

void NitBindCom::TypeLib::dispose()
{
	if (_peer == NULL) return;

	HSQUIRRELVM v = _bindCom->getVm();

	for (Types::iterator itr = _types.begin(), end = _types.end(); itr != end; ++itr)
	{
		sq_release(v, &itr->second);
	}
	_types.clear();
	_comp = NULL;

	_bindCom->uncache(_peer);

	if (_bindCom->isVerbose())
		LOG(0, ".. [NitBindCom] Uncached: ITypeLib %08x -> '%s'\n",
			_peer, _name.c_str());

	_peer = NULL;
}

void NitBindCom::TypeLib::remember(TypeInfo* ti, HSQOBJECT cls)
{
	HSQUIRRELVM v = _bindCom->getVm();

	const String& name = ti->getName();
	Types::iterator itr = _types.find(name);

	assert(itr == _types.end());

	if (itr == _types.end())
	{
		_types.insert(std::make_pair(name, cls));
		sq_addref(v, &cls);
	}
}

void NitBindCom::TypeLib::purge(HSQUIRRELVM v)
{
	Ref<TypeLib> safe = this;

	vector<IDispatch*>::type insts;

	_bindCom->getInstancesOf(this, insts, v);

	for (uint i=0; i<insts.size(); ++i)
	{
		_bindCom->push(v, insts[i]);
		sq_purgeinstance(v, -1);
	}
}

////////////////////////////////////////////////////////////////////////////////

class NitBindCom::TypeInfo::BindingContext
{
public:
	BindingContext(TypeInfo* typeinfo, int clsIdx)
		: _typeInfo(typeinfo), _clsIdx(clsIdx)
	{
		// Obtain a TypeComp which is needed for property access binding
		typeinfo->getPeer()->GetTypeComp(&_comp.get());
	}

	~BindingContext()
	{
		Flush();
	}

	ITypeComp*							GetComp()								{ return _comp; }
	int									GetClsIdx()								{ return _clsIdx; }

	void Flush()
	{
		ITypeInfo* ti = _typeInfo->getPeer();

		for (Funcs::iterator itr = _unusedFuncs.begin(), end = _unusedFuncs.end(); itr != end; ++itr)
			ti->ReleaseFuncDesc(*itr);

		for (Vars::iterator itr = _unusedVars.begin(), end = _unusedVars.end(); itr != end; ++itr)
			ti->ReleaseVarDesc(*itr);

		for (Comps::iterator itr = _unusedComps.begin(), end = _unusedComps.end(); itr != end; ++itr)
			(*itr)->Release();

		_unusedFuncs.clear();
		_unusedVars.clear();
		_unusedComps.clear();
	}

	void Tag(FUNCDESC* fd)				
	{ 
		if (fd) _unusedFuncs.insert(fd); 
	}

	void Tag(VARDESC* vd)
	{ 
		if (vd) _unusedVars.insert(vd); 
	}

	void Tag(DESCKIND kind, BINDPTR ptr)
	{
		if (kind == DESCKIND_FUNCDESC)
			_unusedFuncs.insert(ptr.lpfuncdesc);
		if (kind == DESCKIND_VARDESC)
			_unusedVars.insert(ptr.lpvardesc);
		if (kind == DESCKIND_TYPECOMP)
			_unusedComps.insert(ptr.lptcomp);
	}

	void Use(FUNCDESC* fd)				
	{ 
		Funcs::iterator itr = _unusedFuncs.find(fd);
		if (itr != _unusedFuncs.end())
		{
			_unusedFuncs.erase(itr); 
			_typeInfo->_funcDescs.push_back(fd);
		}
	}

	void Use(VARDESC* vd)
	{ 
		Vars::iterator itr = _unusedVars.find(vd);
		if (itr != _unusedVars.end())
		{
			_unusedVars.erase(itr); 
			_typeInfo->_varDescs.push_back(vd);
		}
	}

	void Use(ITypeComp* comp)
	{
		Comps::iterator itr = _unusedComps.find(comp);
		if (itr != _unusedComps.end())
			_unusedComps.erase(itr);
	}

private:
	TypeInfo*							_typeInfo;
	int									_clsIdx;
	ComRef<ITypeComp>					_comp;

	typedef set<FUNCDESC*>::type		Funcs;
	typedef set<VARDESC*>::type			Vars;
	typedef set<ITypeComp*>::type		Comps;

	Funcs								_unusedFuncs;
	Vars								_unusedVars;
	Comps								_unusedComps;
};

////////////////////////////////////////////////////////////////////////////////

NitBindCom::TypeInfo::TypeInfo(ITypeInfo* peer, TypeLib* tl)
: _peer(peer), _typeLib(tl), _bindCom(tl->getBindCom())
{
	BSTR bname = NULL;
	peer->GetDocumentation(MEMBERID_NIL, &bname, NULL, NULL, NULL);
	if (bname)
	{
		_name = toString(bname);
		SysFreeString(bname);
	}

	_srcName = tl->getName() + "." + _name;
}

void NitBindCom::TypeInfo::dispose()
{
	if (_peer == NULL) return;

	for (uint i=0; i<_funcDescs.size(); ++i)
	{
		_peer->ReleaseFuncDesc(_funcDescs[i]);
	}
	_funcDescs.clear();

	_bindCom->uncache(_peer);

	if (_bindCom->isVerbose())
 		LOG(0, ".. [NitBindCom] Uncached: ITypeInfo %08x -> '%s'\n",
 			_peer, _srcName.c_str());

	_peer = NULL;
	_names.clear();
}

////////////////////////////////////////////////////////////////////////////////

SQRESULT NitBindCom::TypeInfo::createClass(HSQUIRRELVM v)
{
	const char* libName = _typeLib->getName().c_str();
	const char* name = _name.c_str();

	TYPEATTR* attr = NULL;
	HRESULT hr = _peer->GetTypeAttr(&attr);
	if (FAILED(hr))
		return sq_throwfmt(v, "%s.%s: can't obtain typeattr: %s", libName, name, errorStr(hr).c_str());

	sq_newclass(v, false, _name.c_str());
	int clsIdx = NitBind::toAbsIdx(v, -1);

	sq_pushstring(v, "_namespace", -1);
	NitBind::push(v, _typeLib->getName());
	sq_newslot(v, clsIdx, true);

	// bind type tag
	this->incRefCount();
	sq_settypetag(v, clsIdx, this);
	sq_setreleasehook(v, clsIdx, classReleaseHook);

	set<MEMBERID>::type boundMembers;

	BindingContext ctx(this, clsIdx);

	// populate members
	for (int fi=0; fi < attr->cFuncs; ++fi, ctx.Flush())
	{
		FUNCDESC* fd = NULL;
		hr = _peer->GetFuncDesc(fi, &fd);
		ctx.Tag(fd);

		if (FAILED(hr))
		{
			LOG(0, "*** %s.%s: can't bind func #%d: %s", libName, name, fi, errorStr(hr).c_str());
			continue;
		}

		if (boundMembers.find(fd->memid) != boundMembers.end())
			continue;

		switch (fd->memid)
		{
		case DISPID_VALUE:				hr = bindMetaCallFunc(v, fd, &ctx); break;
		case DISPID_NEWENUM:			hr = bindNewEnumFunc(v, fd, &ctx); break;
		case DISPID_EVALUATE:			hr = bindEvaluateFunc(v, fd, &ctx); break;
		case DISPID_CONSTRUCTOR:		hr = bindConstructor(v, fd, &ctx); break;
		case DISPID_DESTRUCTOR:			hr = bindDestructor(v, fd, &ctx); break;
		case DISPID_COLLECT:			hr = bindCollectFunc(v, fd, &ctx); break;
		default:
			if (fd->invkind != INVOKE_FUNC)
				hr = bindProp(v, fd, &ctx);
			else
				hr = bindFunc(v, fd, &ctx);
		}

		if (SUCCEEDED(hr))
			boundMembers.insert(fd->memid);
	}

	// populate vars
	for (int vi=0; vi < attr->cVars; ++vi, ctx.Flush())
	{
		VARDESC* vd = NULL;
		hr = _peer->GetVarDesc(vi, &vd);
		ctx.Tag(vd);

		if (FAILED(hr))
		{
			LOG(0, "*** %s.%s: can't bind var #%d: %s", libName, name, vi, errorStr(hr).c_str());
			continue;
		}

		hr = bindVar(v, vd, &ctx);
	}

	_peer->ReleaseTypeAttr(attr);

	sq_settop(v, clsIdx);

	// failsafe '_get' metamethod
	sq_pushstring(v, "_get", -1);
	sq_newclosure(v, InvokeHandler::invokeMetaGet, 0);
	sq_newslot(v, clsIdx, true);

	// failsafe '_set' metamethod
	sq_pushstring(v, "_set", -1);
	sq_newclosure(v, InvokeHandler::invokeMetaSet, 0);
	sq_newslot(v, clsIdx, true);

	// placeholder for TypeLib
	sq_pushstring(v, "TypeLib", -1);
	sq_pushnull(v);
	sq_newslot(v, clsIdx, false);

	return 1;
}

bool NitBindCom::TypeInfo::isGoodGetter(FUNCDESC* fd)
{
	if (fd->invkind != INVOKE_PROPERTYGET)
		return false;

	if (fd->cParams > 0)
	{
		// Handle get by ref
		if (fd->cParams == 1 && fd->cParamsOpt == 0 && fd->cScodes == 0)
		{
			if (fd->lprgelemdescParam[0].paramdesc.wParamFlags & PARAMFLAG_FOUT)
				return true;
		}

		return false;
	}

	return true;
}

bool NitBindCom::TypeInfo::isGoodSetter(FUNCDESC* fd)
{
	if (fd->invkind != INVOKE_PROPERTYPUT && fd->invkind != INVOKE_PROPERTYPUTREF)
		return false;

 	if (fd->cScodes != 0 || fd->cParams != 1)
 		return false;
 
 	return true;
}

HRESULT NitBindCom::TypeInfo::bindProp(HSQUIRRELVM v, FUNCDESC* fd, BindingContext* ctx)
{
	// Here we'll handle the three - get, put, putref together as property
	UINT found = 0;
	BSTR _name = NULL;
	HRESULT hr = _peer->GetNames(fd->memid, &_name, 1, &found);
	if (FAILED(hr)) return hr; // Skip member which doesn't have proper name

	FUNCDESC* getFunc = fd->invkind == INVOKE_PROPERTYGET ? fd : NULL;
	FUNCDESC* putFunc = fd->invkind == INVOKE_PROPERTYPUT ? fd : NULL;
	FUNCDESC* putrefFunc = fd->invkind == INVOKE_PROPERTYPUTREF ? fd : NULL;

	ULONG hashVal = LHashValOfName(getLocaleID(), _name);
	ComRef<ITypeInfo> ti2;
	DESCKIND desckind = DESCKIND_NONE;
	BINDPTR bindptr = { 0 };
	
	// Look for a get
	if (getFunc == NULL)
	{
		hr = ctx->GetComp()->Bind(_name, hashVal, INVOKE_PROPERTYGET, &ti2.get(), &desckind, &bindptr);
		if (SUCCEEDED(hr) && desckind == DESCKIND_FUNCDESC)
		{
			assert(ti2 == _peer);
			getFunc = bindptr.lpfuncdesc;
		}
		else
		{
			assert(FAILED(hr) || desckind == DESCKIND_NONE);
		}
	}
	ti2 = NULL;

	// Look for a put
	if (putFunc == NULL)
	{
		hr = ctx->GetComp()->Bind(_name, hashVal, INVOKE_PROPERTYPUT, &ti2.get(), &desckind, &bindptr);
		if (SUCCEEDED(hr) && desckind == DESCKIND_FUNCDESC)
		{
			assert(ti2 == _peer);
			putFunc = bindptr.lpfuncdesc;
		}
		else
		{
			assert(FAILED(hr) || desckind == DESCKIND_NONE);
		}
	}
	ti2 = NULL;

	// Look for a putref
	if (putrefFunc == NULL)
	{
		hr = ctx->GetComp()->Bind(_name, hashVal, INVOKE_PROPERTYPUTREF, &ti2.get(), &desckind, &bindptr);
		if (SUCCEEDED(hr) && desckind == DESCKIND_FUNCDESC)
		{
			assert(ti2 == _peer);
			putrefFunc = bindptr.lpfuncdesc;
		}
	}
	ti2 = NULL;

	// Replace obscure get, put, putref to a func
	if (getFunc && !isGoodGetter(getFunc))
	{
		bindFunc(v, getFunc, ctx);
		getFunc = NULL;
	}

	if (putFunc && putrefFunc)
	{
		// If there's a put and also a putref : put will become property, putref as func.
		bindFunc(v, putrefFunc, ctx);
		putrefFunc = NULL;
	}

	// if no get, all become funcs.

	if (putFunc && (getFunc == NULL || !isGoodSetter(putFunc)) )
	{
		bindFunc(v, putFunc, ctx);
		putFunc = NULL;
	}

	if (putrefFunc && (getFunc == NULL || !isGoodSetter(putrefFunc)) )
	{
		assert(putFunc == NULL);
		bindFunc(v, putrefFunc, ctx);
		putrefFunc = NULL;
	}

	// If a put or a putref left over, register it as a put
	if (putFunc == NULL && putrefFunc)
	{
		putFunc = putrefFunc;
		putrefFunc = NULL;
	}

	assert(putrefFunc == NULL);

	// If both getter and setter not present : just skip as handled.

	_names.push_back(toString(_name));
	String& propname = _names.back();
	SysFreeString(_name);

	if (propname.find("Dummy") == 0)
		propname = String("_") + propname;

	// Start property generation
	sq_pushstring(v, propname.c_str(), propname.length());

	if (getFunc)
	{
		ctx->Use(getFunc);
		sq_pushuserpointer(v, getFunc);
		if (getFunc->cParams == 0)
			sq_pushuserpointer(v, InvokeHandler::handleGet);
		else
		{
			if (_bindCom->isVerbose())
				LOG(0, ".. prop getbyref %s [%d] : %s\n", propname.c_str(), getFunc->memid, paramsDescStr(_peer, getFunc).c_str());
			sq_pushuserpointer(v, InvokeHandler::handleGetByRef);
		}
		sq_newclosure(v, InvokeHandler::invokeCallback, 2);
		sq_setnativeclosureinfo(v, -1, propname.c_str(), _srcName.c_str());
		sq_pushstring(v, propGetRetStr(_peer, getFunc).c_str(), -1);
		sq_sethelp(v, -2);
	}
	else
	{
		sq_pushnull(v);
	}

	if (putFunc)
	{
		ctx->Use(putFunc);
		sq_pushuserpointer(v, putFunc);
		if (putFunc->invkind == INVOKE_PROPERTYPUT)
			sq_pushuserpointer(v, InvokeHandler::handlePut);
		else
			sq_pushuserpointer(v, InvokeHandler::handlePutRef);
		sq_newclosure(v, InvokeHandler::invokeCallback, 2);
		sq_setnativeclosureinfo(v, -1, propname.c_str(), _srcName.c_str());
	}
	else
	{
		sq_pushnull(v);
	}

	sq_newproperty(v, ctx->GetClsIdx());
	return S_OK;
}

static bool IsIgnoredName(BSTR name)
{
	static std::set<std::wstring> names; // HACK: this is a local static so do not use nit version which managed by Memory Manager

	if (names.empty())
	{
		// IUnknown
		names.insert(L"QueryInterface");
		names.insert(L"AddRef");
		names.insert(L"Release");

		// IDispatch
		names.insert(L"Invoke");
		names.insert(L"GetTypeInfo");
		names.insert(L"GetTypeInfoCount");
		names.insert(L"GetIDsOfNames");
	}

	return names.find(name) != names.end();
}

HRESULT NitBindCom::TypeInfo::bindFunc(HSQUIRRELVM v, FUNCDESC* fd, BindingContext* ctx)
{
	UINT found = 0;
	BSTR _name = NULL;
	HRESULT hr = _peer->GetNames(fd->memid, &_name, 1, &found);
	if (FAILED(hr)) return hr; // Skip member which doesn't have a proper name

	// Ignore too verbose names
	if (IsIgnoredName(_name))
	{
		if (_bindCom->isVerbose())
			LOG(0, ".. func %s.%s [%d] ignored\n", _srcName.c_str(), toString(_name).c_str(), fd->memid);
		return S_OK;
	}

	_names.push_back(toString(_name));
	String& fnname = _names.back();
	SysFreeString(_name);

	if (fnname.find("Dummy") == 0)
		fnname = String("_") + fnname;

	// Handle props left over
	if (fd->invkind != INVOKE_FUNC)
	{
		String newname;
		const char* kind = NULL;

		switch (fd->invkind)
		{
		case INVOKE_PROPERTYGET:		kind = "get"; newname = fnname; break; // Preserve the property name for a getter
		case INVOKE_PROPERTYPUT:		kind = "put"; newname = String("Set") + fnname; break; 
		case INVOKE_PROPERTYPUTREF:		kind = "putref"; newname = String("Set") + fnname + "Ref"; break;
		}

		if (_bindCom->isVerbose())
	 		LOG(0, ".. prop %s %s.%s [%d] forced as func %s%s\n",
 				kind, _srcName.c_str(), fnname.c_str(), fd->memid, newname.c_str(), paramsDescStr(_peer, fd).c_str());

		fnname = newname;
	}

	sq_pushstring(v, fnname.c_str(), fnname.length());

	InvokeHandler::HandlerFunc handlerFunc = InvokeHandler::handleCall_default;

	ctx->Use(fd);
	sq_pushuserpointer(v, fd);
	sq_pushuserpointer(v, handlerFunc);
	sq_newclosure(v, InvokeHandler::invokeCallback, 2);
	sq_setnativeclosureinfo(v, -1, fnname.c_str(), _srcName.c_str());
	sq_pushstring(v, paramsDescStr(_peer, fd).c_str(), -1);
	sq_sethelp(v, -2);

	sq_newslot(v, ctx->GetClsIdx(), true);

	return S_OK;
}

HRESULT NitBindCom::TypeInfo::bindMetaCallFunc(HSQUIRRELVM v, FUNCDESC* fd, BindingContext* ctx)
{
	// Register original method
	HRESULT hr = bindFunc(v, fd, ctx);
	if (FAILED(hr)) return hr;

	// Register one more - meta _nexti
	sq_pushstring(v, "_call", -1);
	sq_pushuserpointer(v, fd);
	sq_newclosure(v, InvokeHandler::invokeMetaCall, 1);
	sq_setnativeclosureinfo(v, -1, "_call", _srcName.c_str());
	sq_newslot(v, ctx->GetClsIdx(), true);

	return S_OK;
}

HRESULT NitBindCom::TypeInfo::bindNewEnumFunc(HSQUIRRELVM v, FUNCDESC* fd, BindingContext* ctx)
{
	// Register original method
	HRESULT hr = bindFunc(v, fd, ctx);
	if (FAILED(hr)) return hr;

	// Register one more - meta _nexti
	sq_pushstring(v, "_nexti", -1);
	sq_pushuserpointer(v, fd);
	sq_newclosure(v, InvokeHandler::invokeMetaNexti, 1);
	sq_setnativeclosureinfo(v, -1, "_nexti", _srcName.c_str());
	sq_newslot(v, ctx->GetClsIdx(), true);

	return S_OK;
}

HRESULT NitBindCom::TypeInfo::bindEvaluateFunc(HSQUIRRELVM v, FUNCDESC* fd, BindingContext* ctx)
{
	return bindFunc(v, fd, ctx);
}

HRESULT NitBindCom::TypeInfo::bindConstructor(HSQUIRRELVM v, FUNCDESC* fd, BindingContext* ctx)
{
	return bindFunc(v, fd, ctx);
}

HRESULT NitBindCom::TypeInfo::bindDestructor(HSQUIRRELVM v, FUNCDESC* fd, BindingContext* ctx)
{
	return bindFunc(v, fd, ctx);
}

HRESULT NitBindCom::TypeInfo::bindCollectFunc(HSQUIRRELVM v, FUNCDESC* fd, BindingContext* ctx)
{
	return bindFunc(v, fd, ctx);
}

HRESULT NitBindCom::TypeInfo::bindVar(HSQUIRRELVM v, VARDESC* vd, BindingContext* ctx)
{
	if (vd->varkind != VAR_CONST)
	{
		LOG(0, "*** var %s.%s [%d]: not supported yet\n", _srcName.c_str(), varDescStr(_peer, vd).c_str());
		return S_FALSE;
	}

	UINT found = 0;
	BSTR _name = NULL;
	HRESULT hr = _peer->GetNames(vd->memid, &_name, 1, &found);
	if (FAILED(hr)) return hr; // Skip member which doesn't have a proper name

	String varname = toString(_name);
	SysFreeString(_name);

	sq_pushstring(v, varname.c_str(), varname.length());

	if (vd->lpvarValue)
	{
		Variant var = *vd->lpvarValue;
		var.push(v);
	}
	else
	{
		sq_pushnull(v);
	}

	sq_newslot(v, ctx->GetClsIdx(), true);

	return S_OK;
}

SQRESULT NitBindCom::TypeInfo::classReleaseHook(HSQUIRRELVM v, SQUserPointer p, SQInteger size)
{
	TypeInfo* self = (TypeInfo*)p;
	self->decRefCount();

	return SQ_OK;
}

SQRESULT NitBindCom::TypeInfo::instanceToString(HSQUIRRELVM v)
{
	TypeInfo* typeinfo = NULL;
	sq_gettypetag(v, 1, (SQUserPointer*)&typeinfo);

	assert(typeinfo);

	IDispatch* instance = NULL;
	sq_getinstanceup(v, 1, (SQUserPointer*)&instance, typeinfo);

	return NitBind::pushFmt(v, "(%s -> %08x)", typeinfo->getSrcName().c_str(), instance);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT NitBindCom::Variant::changeType(VARTYPE vt)
{
	if (vt == _peer.vt) return S_OK;

	VARIANTARG newarg;
	VariantInit(&newarg);

	HRESULT hr = VariantChangeType(&newarg, &_peer, 0, vt);
	if (SUCCEEDED(hr))
	{
		clear();
		_peer = newarg;
	}

	return hr;
}

HRESULT NitBindCom::Variant::getFrom(HSQUIRRELVM v, SQInteger idx)
{
	clear();

	switch (sq_gettype(v, idx))
	{
	case OT_STRING:			{ const char* str = NULL; sq_getstring(v, idx, &str); _peer.vt = VT_BSTR; _peer.bstrVal = NitBindCom::toBstr(str, strlen(str)); break; }
	case OT_BOOL:			{ SQBool value = 0; sq_getbool(v, idx, &value); _peer.vt = VT_BOOL; _peer.boolVal = value; break; }
	case OT_INTEGER:		{ SQInteger value = 0; sq_getinteger(v, idx, &value); _peer.vt = VT_INT; _peer.intVal = value; break; }
	case OT_FLOAT:			{ SQFloat value = 0; sq_getfloat(v, idx, &value); _peer.vt = VT_R4; _peer.fltVal = value; break; }
	case OT_NULL:			{ _peer.vt = VT_EMPTY; break; }
	default:				{ _peer.vt = VT_EMPTY; return S_FALSE; }
	}

	return S_OK;
}

SQRESULT NitBindCom::Variant::push(HSQUIRRELVM v)
{
	HRESULT hr;

	makeValue();

	if (_peer.vt & VT_BYREF)
	{
		// TODO: Get byref other than VT_VARIANT (see oaidl.h line 360)
		sqx_pushstringf(v, "<variant_%d byref>", _peer.vt & ~VT_BYREF);
		return 1;
	}

	if (_peer.vt == (VT_ARRAY | VT_VARIANT) )
	{
		// Get the array of a VT_VARIANT
		SAFEARRAY* a = _peer.parray;

//		SafeArrayGetElement
	}

	switch (_peer.vt)
	{
	case VT_EMPTY:
	case VT_NULL:	
	case VT_VOID:
		sq_pushnull(v); 
		return 1;

	case VT_INT: case VT_UINT: 
	case VT_I1: case VT_I2: case VT_I4: case VT_I8: 
	case VT_UI1: case VT_UI2: case VT_UI4: case VT_UI8:
		hr = changeType(VT_I4);
		sq_pushinteger(v, _peer.intVal);
		return 1;

	case VT_R4: case VT_R8:
		hr = changeType(VT_R4);
		sq_pushfloat(v, _peer.fltVal);
		return 1;

	case VT_BOOL:
		sq_pushbool(v, _peer.boolVal); 
		return 1;

	case VT_BSTR:
		return NitBind::push(v, toString(_peer.bstrVal)); 

	case VT_UNKNOWN:
		{
			ComRef<IDispatch> disp;
			hr = _peer.punkVal->QueryInterface(IID_IDispatch, (void**)&disp.get());
			if (SUCCEEDED(hr))
				return NitBindCom::get(v)->push(v, disp);

			ComRef<IEnumVARIANT> en;
			hr = _peer.punkVal->QueryInterface(IID_IEnumVARIANT, (void**)&en.get());
			if (SUCCEEDED(hr))
				return NitBindCom::get(v)->push(v, en);

			sqx_pushstringf(v, "IUnknown(%08x)", _peer.punkVal);
			return 1;
		}

	case VT_DISPATCH:
		return NitBindCom::get(v)->push(v, _peer.pdispVal); 
		break;

	case VT_ERROR:						
		return NitBind::push(v, errorStr(_peer.scode)); 

	case VT_CY: 
		hr = changeType(VT_I4);
		sq_pushinteger(v, _peer.intVal);
		return 1;

	case VT_ARRAY:
		sq_pushstring(v, "VT_ARRAY", -1);
		return 1;

	case VT_DATE:						
		sq_pushstring(v, "VT_DATE", -1); 
		return 1;

	case VT_PTR:						
	case VT_INT_PTR:
	case VT_UINT_PTR:					
	case VT_BYREF:
		sq_pushuserpointer(v, _peer.puintVal); 
		return 1;

	default:
		sqx_pushstringf(v, "<variant_%d>", _peer.vt);
		return 1;
	}
}

long NitBindCom::Variant::getSize(VARTYPE vt)
{
	if (vt & VT_ARRAY)					return sizeof(SAFEARRAY*);

	switch (vt)
	{
	case VT_I2:							return 2;
	case VT_I4:							return 4;
	case VT_R4:							return 4;
	case VT_R8:							return 8;
	case VT_CY:							return sizeof(CURRENCY);
	case VT_DATE:						return sizeof(DATE);
	case VT_BSTR:						return sizeof(BSTR);
	case VT_DISPATCH:					return sizeof(IDispatch*);
	case VT_ERROR:						return sizeof(SCODE);
	case VT_BOOL:						return sizeof(VARIANT_BOOL);
	case VT_VARIANT:					return sizeof(VARIANT);
	case VT_UNKNOWN:					return sizeof(IUnknown);
	case VT_DECIMAL:					return 16;
	case VT_UI1:
	case VT_I1:							return 1;
	case VT_UI2:						return 2;
	case VT_UI4:						return 4;
	case VT_INT:						return sizeof(int);
	case VT_UINT:						return sizeof(unsigned int);
	default:							NIT_THROW(EX_INVALID_PARAMS);
	}
}

void NitBindCom::Variant::makeVariantRef()
{
	if (_peer.vt & VT_BYREF) return;

	VARIANTARG npeer;
	VariantInit(&npeer);

	npeer.vt = VT_VARIANT | VT_BYREF;
	npeer.pvarVal = (VARIANTARG*)CoTaskMemAlloc(sizeof(VARIANT));
	*npeer.pvarVal = _peer;

	_peer = npeer;
}

void NitBindCom::Variant::makeRef()
{
	if (_peer.vt & VT_BYREF) return;

	VARIANTARG npeer;
	VariantInit(&npeer);

	long size = getSize(_peer.vt);
	npeer.vt = _peer.vt | VT_BYREF;
	npeer.byref = (void*)CoTaskMemAlloc(size);
	memcpy(npeer.byref, &_peer.byref, size);

	_peer = npeer;
}

void NitBindCom::Variant::makeValue()
{
	if (!(_peer.vt & VT_BYREF)) return;

	VARIANTARG npeer;
	VariantInit(&npeer);

	VARTYPE nvt = _peer.vt & ~VT_BYREF;

	if (nvt == VT_VARIANT)
	{
		npeer = *_peer.pvarVal;
	}
	else
	{
		long size = getSize(npeer.vt);
		memcpy(&npeer.byref, _peer.byref, size);
	}

	CoTaskMemFree(_peer.byref);

	npeer.vt = nvt;
	_peer = npeer;
}

void NitBindCom::Variant::clear()
{
	// We need to release the associated internal data of a byref
	if ((_peer.vt & VT_BYREF) && _peer.byref)
	{
		switch (_peer.vt & ~VT_BYREF)
		{
		case VT_BSTR:					SysFreeString(*_peer.pbstrVal); break;
		case VT_DISPATCH:				(*_peer.ppdispVal)->Release(); break;
		case VT_UNKNOWN:				(*_peer.ppunkVal)->Release(); break;

		case VT_VARIANT:
			// No other byref can be contained in a byref variant so treat as a normal case.
			VariantClear(_peer.pvarVal); break; 
		}

		if ((_peer.vt & VT_ARRAY) && *_peer.pparray)
			SafeArrayDestroy(*_peer.pparray);

		CoTaskMemFree(_peer.byref);
		VariantClear(&_peer);
	}
	else
	{
		// Normal case : VariantClear automatically cleans up the internal data.
		VariantClear(&_peer);
	}
}

////////////////////////////////////////////////////////////////////////////////

SQRESULT NitBindCom::InvokeHandler::invokeCallback(HSQUIRRELVM v)
{
	TypeInfo* typeinfo = NULL;
	sq_gettypetag(v, 1, (SQUserPointer*)&typeinfo);

	assert(typeinfo);

	IDispatch* instance = NULL;
	sq_getinstanceup(v, 1, (SQUserPointer*)&instance, typeinfo);

	if (instance == NULL)
		return sq_throwfmt(v, "this as '%s' expected", typeinfo->getName().c_str());

	FUNCDESC* fd = NULL;
	sq_getuserpointer(v, -1, (SQUserPointer*)&fd);
	sq_poptop(v);

	InvokeHandler::HandlerFunc handlerFunc = NULL;
	sq_getuserpointer(v, -1, (SQUserPointer*)&handlerFunc);
	sq_poptop(v);

	assert(fd && handlerFunc);

	InvokeHandler handler(instance, typeinfo, fd);
	return handlerFunc(&handler, v);
}

SQRESULT NitBindCom::InvokeHandler::invokeMetaCall(HSQUIRRELVM v)
{
	TypeInfo* typeinfo = NULL;
	sq_gettypetag(v, 1, (SQUserPointer*)&typeinfo);

	assert(typeinfo);

	IDispatch* instance = NULL;
	sq_getinstanceup(v, 1, (SQUserPointer*)&instance, typeinfo);

	if (instance == NULL)
		return sq_throwfmt(v, "this as '%s' expected", typeinfo->getName().c_str());

	FUNCDESC* fd = NULL;
	sq_getuserpointer(v, -1, (SQUserPointer*)&fd);
	sq_poptop(v);

	assert(fd);

	InvokeHandler handler(instance, typeinfo, fd);
	return call_default(&handler, v, 3, sq_gettop(v));
}

SQRESULT NitBindCom::InvokeHandler::invokeMetaNexti(HSQUIRRELVM v)
{
	// If we got an IEnumVARIANT (an iteration keeps going)
	if (sq_gettype(v, 2) == OT_USERDATA)
	{
		// Reuse the IEnumVARIANT (itr was already advanced during the previous _get call)
		sq_push(v, 2);
		return 1;
	}

	// New iteration
	if (sq_gettype(v, 2) != OT_NULL)
		return sq_throwerror(v, "invalid foreach status");

	TypeInfo* typeinfo = NULL;
	sq_gettypetag(v, 1, (SQUserPointer*)&typeinfo);

	assert(typeinfo);

	IDispatch* instance = NULL;
	sq_getinstanceup(v, 1, (SQUserPointer*)&instance, typeinfo);

	if (instance == NULL)
		return sq_throwfmt(v, "this as '%s' expected", typeinfo->getName().c_str());

	FUNCDESC* fd = NULL;
	sq_getuserpointer(v, -1, (SQUserPointer*)&fd);
	sq_poptop(v);

	assert(fd);

	InvokeHandler handler(instance, typeinfo, fd);
	return call_default(&handler, v, 3, sq_gettop(v));
}

SQRESULT NitBindCom::InvokeHandler::invokeMetaGet(HSQUIRRELVM v)
{
	// If we got an IEnumVARIANT as key, handle as a for-each iteration
	if (sq_gettype(v, 2) == OT_USERDATA)
	{
		IEnumVARIANT* en = getEnum(v, 2);
		// return the next value and advance the iteration
		Variant var;
		ULONG numElems = 0;
		HRESULT hr = en->Next(1, var, &numElems);
		if (numElems)
		{
			var.push(v);
			return 1;
		}

		// Terminal condition of the iteration
		sq_reseterror(v);
		return SQ_ERROR;
	}

	// Common handling
	IDispatch* instance = NULL;
	sq_getinstanceup(v, 1, (SQUserPointer*)&instance, NULL);

	if (instance == NULL)
		return SQ_ERROR;

	DISPID dispid;
	UniString propname = Unicode::toUtf16(NitBind::getString(v, 2));
	UniChar* propnamestr = const_cast<UniChar*>(propname.c_str());
	HRESULT hr = instance->GetIDsOfNames(IID_NULL, (LPOLESTR*)&propnamestr, 1, getLocaleID(), &dispid);

	if (hr == DISP_E_UNKNOWNNAME)
	{
		sq_reseterror(v);
		return SQ_ERROR; // clean failure
	}

	if (FAILED(hr))
		return sq_throwfmt(v, "'%s': %s", NitBind::getString(v, 2), errorStr(hr).c_str());

	DISPPARAMS	dispParams = { 0 };
	EXCEPINFO	excep;
	UINT		errArgNo = 0;
	Variant		result;

	hr = instance->Invoke(
		dispid,
		IID_NULL,
		getLocaleID(),
		INVOKE_PROPERTYGET,
		&dispParams,
		result,
		&excep,
		&errArgNo);

	if (FAILED(hr))
		return error(NULL, NitBind::getString(v, 2), v, hr, excep, errArgNo);

	return result.push(v);
}

SQRESULT NitBindCom::InvokeHandler::invokeMetaSet(HSQUIRRELVM v)
{
	IDispatch* instance = NULL;
	sq_getinstanceup(v, 1, (SQUserPointer*)&instance, NULL);

	if (instance == NULL)
		return SQ_ERROR;

	DISPID dispid;
	UniString propname = Unicode::toUtf16(NitBind::getString(v, 2));
	UniChar* propnamestr = const_cast<UniChar*>(propname.c_str());
	HRESULT hr = instance->GetIDsOfNames(IID_NULL, (LPOLESTR*)&propnamestr, 1, getLocaleID(), &dispid);

	if (hr == DISP_E_UNKNOWNNAME)
	{
		sq_reseterror(v);
		return SQ_ERROR; // clean failure
	}

	if (FAILED(hr))
		return sq_throwfmt(v, "'%s': %s", NitBind::getString(v, 2), errorStr(hr).c_str());

	DISPPARAMS	dispParams = { 0 };
	EXCEPINFO	excep;
	UINT		errArgNo = 0;

	DISPID dispIdNamed = DISPID_PROPERTYPUT;
	dispParams.cNamedArgs = 1;
	dispParams.rgdispidNamedArgs = &dispIdNamed;

	Variant value;
	value.getFrom(v, 3);

	dispParams.cArgs = 1;
	dispParams.rgvarg = value;

	hr = instance->Invoke(
		dispid,
		IID_NULL,
		getLocaleID(),
		INVOKE_PROPERTYPUT,
		&dispParams,
		NULL,
		&excep,
		&errArgNo);

	if (FAILED(hr))
		return error(NULL, NitBind::getString(v, 2), v, hr, excep, errArgNo);

	return 0;
}

SQRESULT NitBindCom::InvokeHandler::handleGet(InvokeHandler* hdlr, HSQUIRRELVM v)
{
	DISPPARAMS	dispParams = { 0 };
	EXCEPINFO	excep;
	UINT		errArgNo = 0;
	Variant		result;

	HRESULT hr = hdlr->_instance->Invoke(
 		hdlr->_funcDesc->memid, 
		IID_NULL,
		getLocaleID(),
		hdlr->_funcDesc->invkind,
 		&dispParams, 
 		result, 
 		&excep, 
 		&errArgNo);

	if (FAILED(hr))
		return error(hdlr, NULL, v, hr, excep, dispParams.cArgs - errArgNo);

	return result.push(v);
}

SQRESULT NitBindCom::InvokeHandler::handleGetByRef(InvokeHandler* hdlr, HSQUIRRELVM v)
{
	DISPPARAMS	dispParams = { 0 };
	EXCEPINFO	excep;
	UINT		errArgNo = 0;
	Variant		result;

	dispParams.cArgs = 1;
	dispParams.rgvarg = result;

	HRESULT hr = hdlr->_instance->Invoke(
		hdlr->_funcDesc->memid, 
		IID_NULL,
		getLocaleID(),
		hdlr->_funcDesc->invkind,
		&dispParams, 
		NULL, 
		&excep, 
		&errArgNo);

	if (FAILED(hr))
		return error(hdlr, NULL, v, hr, excep, dispParams.cArgs - errArgNo);

	return result.push(v);
}

SQRESULT NitBindCom::InvokeHandler::handlePut(InvokeHandler* hdlr, HSQUIRRELVM v)
{
	if (sq_gettop(v) != 2)
		return sq_throwerror(v, "invalid args");

	DISPPARAMS	dispParams = { 0 };
	EXCEPINFO	excep;
	UINT		errArgNo = 0;

	DISPID dispIdNamed = DISPID_PROPERTYPUT;
	dispParams.cNamedArgs = 1;
	dispParams.rgdispidNamedArgs = &dispIdNamed;

	Variant var;
	var.getFrom(v, 2);

	dispParams.cArgs = 1;
	dispParams.rgvarg = var;

	HRESULT hr = hdlr->_instance->Invoke(
		hdlr->_funcDesc->memid, 
		IID_NULL,
		getLocaleID(),
		hdlr->_funcDesc->invkind,
		&dispParams, 
		NULL, 
		&excep, 
		&errArgNo);

	if (FAILED(hr))
		return error(hdlr, NULL, v, hr, excep, dispParams.cArgs - errArgNo);

	return 0;
}

SQRESULT NitBindCom::InvokeHandler::handlePutRef(InvokeHandler* hdlr, HSQUIRRELVM v)
{
	DISPPARAMS dispParams = { 0 };

	DISPID dispIdNamed = DISPID_PROPERTYPUT;
	dispParams.cNamedArgs = 1;
	dispParams.rgdispidNamedArgs = &dispIdNamed;

	return sq_throwerror(v, "not supported yet");

//	return hdlr->Invoke(v, dispParams, INVOKE_PROPERTYPUTREF);
}

SQRESULT NitBindCom::InvokeHandler::handleCall_default(InvokeHandler* hdlr, HSQUIRRELVM v)
{
	return call_default(hdlr, v, 2, sq_gettop(v));
}

SQRESULT NitBindCom::InvokeHandler::call_default(InvokeHandler* hdlr, HSQUIRRELVM v, int argStart, int argEnd)
{
	// Pushes all parameters haphazardly.
	// Returns if one value returned from underlying COM method.
	// We don't handle out or ref.

	DISPPARAMS	dispParams = { 0 };
	EXCEPINFO	excep;
	UINT		errArgNo = 0;
	Variant		result;

	vector<Variant>::type vargs;

	if (argStart <= argEnd)
	{
		// Make args list in reversed order following to COM protocol.
		for (int i=argEnd; i >= argStart; --i)
		{
			int sqidx = i;

			vargs.push_back(Variant());
			Variant& var = vargs.back();
			var.getFrom(v, sqidx);
		}
	}

	if (!vargs.empty())
	{
		dispParams.rgvarg = (VARIANTARG*)&vargs[0];
		dispParams.cArgs = vargs.size();
	}

	INVOKEKIND invkind = hdlr->_funcDesc->invkind;

	if (invkind == INVOKE_PROPERTYPUT || invkind == INVOKE_PROPERTYPUTREF)
	{
		DISPID dispIdNamed = DISPID_PROPERTYPUT;
		dispParams.cNamedArgs = 1;
		dispParams.rgdispidNamedArgs = &dispIdNamed;
	}

	HRESULT hr = hdlr->_instance->Invoke(
		hdlr->_funcDesc->memid, 
		IID_NULL,
		getLocaleID(),
		invkind,
		&dispParams,
		result,
		&excep,
		&errArgNo);

	if (FAILED(hr))
		return error(hdlr, NULL, v, hr, excep, dispParams.cArgs - errArgNo);

	return result.push(v);
}

SQRESULT NitBindCom::InvokeHandler::error(InvokeHandler* hdlr, const char* memname, HSQUIRRELVM v, HRESULT hr, EXCEPINFO& excep, int argNo)
{
	String memberName;

	if (memname)
	{
		memberName = memname;
	}
	else
	{
		UINT found = 0;
		BSTR _name = NULL;

		if (hdlr)
		{
			hdlr->_typeInfo->getPeer()->GetNames(hdlr->_funcDesc->memid, &_name, 1, &found);
		}

		if (_name)
		{
			memberName = hdlr->_typeInfo->getSrcName() + "." + toString(_name);
			SysFreeString(_name);
		}
	}

	switch (hr)
	{
	case DISP_E_EXCEPTION:
		if (excep.bstrDescription != NULL)
		{
			// Error string may become extremely long so take extra care
			String err = StringUtil::format("%s: ", memberName.c_str()) + toString(excep.bstrDescription); 
			SysFreeString(excep.bstrDescription);
			return sq_throwerror(v, err.c_str());
		}
		else if (excep.wCode != 0)
			return sq_throwfmt(v, "%s: %s", memberName.c_str(), errorStr(excep.wCode).c_str());
		else if (excep.scode != 0)
			return sq_throwfmt(v, "%s: %s", memberName.c_str(), errorStr(excep.scode).c_str());
		else
			return sq_throwfmt(v, "%s: unknown COM exception", memberName.c_str());

	case DISP_E_TYPEMISMATCH:
	case DISP_E_PARAMNOTFOUND:
		return sq_throwfmt(v, "%s(param %d): %s", memberName.c_str(), argNo, errorStr(hr).c_str());

	default:
		return sq_throwfmt(v, "%s : %s", memberName.c_str(), errorStr(hr).c_str());
	}
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;