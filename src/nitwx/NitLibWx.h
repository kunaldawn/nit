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

#include "nit/script/ScriptRuntime.h"
#include "nit/script/NitBind.h"

#include "nit/script/NitLibData.h"

#include "nit/script/NitBindMacro.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

// Implementation omitted in NitBind.cpp

inline wxString NitWxGetString(HSQUIRRELVM v, SQInteger idx)
{
	return wxString::FromUTF8Unchecked(sqx_getstring(v, idx));
}

inline wxString NitWxOptString(HSQUIRRELVM v, SQInteger idx, const wxString& def)
{
	return NitBind::isNone(v, idx) || NitBind::isNull(v, idx) ? def : wxString::FromUTF8Unchecked(sqx_getstring(v, idx));
}

inline SQInteger NitWxPushString(HSQUIRRELVM v, const wxString& value)
{
	const wxScopedCharBuffer utf8 = value.utf8_str(); 
	sq_pushstring(v, utf8.data(), utf8.length()); 
	return 1; 
}

////////////////////////////////////////////////////////////////////////////////

#define NB_TYPE_WXOBJ(API, TClass, TBaseClass) \
	namespace wx { typedef wx##TClass TClass; } \
	_NB_TYPE_PTR_BASE(API, wx::TClass, TBaseClass) \
	template<> API SQInteger			NitBindImpl::TypeInfo<wx::TClass>::constructHook(HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p, SQInteger size) { NitBindImpl::addCachedPtr(v, inst, (wx::TClass*)p); static_cast<ScriptWxWeakTracker*>(ScriptRuntime::getRuntime(v)->getWxWeakTracker())->add((wxTrackable*)(wx::TClass*)p, (wx::TClass*)p); return 0; } \
	template<> API SQInteger			NitBindImpl::TypeInfo<wx::TClass>::postConsHook(HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p, SQInteger size) { return 0; } \
	template<> API SQInteger			NitBindImpl::TypeInfo<wx::TClass>::releaseHook(HSQUIRRELVM v, SQUserPointer p, SQInteger size) { NitBindImpl::removeCachedPtr(v, (wx::TClass*)p); static_cast<ScriptWxWeakTracker*>(ScriptRuntime::getRuntime(v)->getWxWeakTracker())->release((wxTrackable*)(wx::TClass*)p, (wx::TClass*)p); return 0; } \
	template<> API SQUserPointer		NitBindImpl::TypeInfo<wx::TClass>::toUserPtr(wx::TClass* ptr) { return (SQUserPointer*)ptr; } \
	template<> API wx::TClass*			NitBindImpl::TypeInfo<wx::TClass>::toClassPtr(SQUserPointer ptr) { return (wx::TClass*)ptr; } 

#define NB_TYPE_WXOBJ_EXT(API, NIT_NS, PREFIX, TClass, TBaseClass) \
	namespace wx { namespace NIT_NS { typedef PREFIX##TClass TClass; } } \
	_NB_TYPE_PTR_BASE(API, wx::NIT_NS::TClass, TBaseClass) \
	template<> API SQInteger			NitBindImpl::TypeInfo<wx::NIT_NS::TClass>::constructHook(HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p, SQInteger size) { NitBindImpl::addCachedPtr(v, inst, (wx::NIT_NS::TClass*)p); static_cast<ScriptWxWeakTracker*>(ScriptRuntime::getRuntime(v)->getWxWeakTracker())->add((wxTrackable*)(wx::NIT_NS::TClass*)p, (wx::NIT_NS::TClass*)p); return 0; } \
	template<> API SQInteger			NitBindImpl::TypeInfo<wx::NIT_NS::TClass>::postConsHook(HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p, SQInteger size) { return 0; } \
	template<> API SQInteger			NitBindImpl::TypeInfo<wx::NIT_NS::TClass>::releaseHook(HSQUIRRELVM v, SQUserPointer p, SQInteger size) { NitBindImpl::removeCachedPtr(v, (wx::NIT_NS::TClass*)p); static_cast<ScriptWxWeakTracker*>(ScriptRuntime::getRuntime(v)->getWxWeakTracker())->release((wxTrackable*)(wx::NIT_NS::TClass*)p, (wx::NIT_NS::TClass*)p); return 0; } \
	template<> API SQUserPointer		NitBindImpl::TypeInfo<wx::NIT_NS::TClass>::toUserPtr(wx::NIT_NS::TClass* ptr) { return (SQUserPointer*)ptr; } \
	template<> API wx::NIT_NS::TClass*		NitBindImpl::TypeInfo<wx::NIT_NS::TClass>::toClassPtr(SQUserPointer ptr) { return (wx::NIT_NS::TClass*)ptr; } 

#define NB_TYPE_WXREF(API, TClass, TBaseClass) \
	namespace wx { typedef wx##TClass TClass; } \
	_NB_TYPE_PTR_BASE(API, wx::TClass, TBaseClass) \
	template<> API SQInteger			NitBindImpl::TypeInfo<wx::TClass>::constructHook(HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p, SQInteger size) { NitBindImpl::addCachedPtr(v, inst, p); ((wx::TClass*)p)->IncRef(); return 0; } \
	template<> API SQInteger			NitBindImpl::TypeInfo<wx::TClass>::postConsHook(HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p, SQInteger size) { ((wx::TClass*)p)->DecRef(); return 0; } \
	template<> API SQInteger			NitBindImpl::TypeInfo<wx::TClass>::releaseHook(HSQUIRRELVM v, SQUserPointer p, SQInteger size) { NitBindImpl::removeCachedPtr(v, p); ((wx::TClass*)p)->DecRef(); return 0; } \
	template<> API SQUserPointer		NitBindImpl::TypeInfo<wx::TClass>::toUserPtr(wx::TClass* ptr) { return (SQUserPointer*)ptr; } \
	template<> API wx::TClass*			NitBindImpl::TypeInfo<wx::TClass>::toClassPtr(SQUserPointer ptr) { return (wx::TClass*)ptr; } 

#define NB_TYPE_WX_AUTODELETE(API, TClass, TBaseClass, DeleteFn) \
	namespace wx { typedef wx##TClass TClass; } \
	NB_TYPE_AUTODELETE(API, wx::TClass, TBaseClass, DeleteFn)

#define NB_TYPE_WXVALUE(API, TClass, TBaseClass) \
	namespace wx { typedef wx##TClass TClass; } \
	NB_TYPE_VALUE(API, wx::TClass, TBaseClass)

////////////////////////////////////////////////////////////////////////////////

class NB_WxScrollHelper : TNitInterface<wxScrollHelper>
{
public:
	template <typename TClass>
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(viewStart),
			PROP_ENTRY	(targetWindow),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(calcScrolledPosition,		"(pt: Point): Point"),
			FUNC_ENTRY_H(calcUnscrolledPosition,	"(pt: Point): Point"),
			FUNC_ENTRY_H(disableKeyboardScrolling,	"()"),
			FUNC_ENTRY_H(enableScrolling,			"(xscroll, yscroll: bool)"),
			FUNC_ENTRY_H(showScrollbars,			"(horz=SHOW_SB_DEFAULT, vert=SHOW_SB_DEFAULT)"),
			FUNC_ENTRY_H(getScrollPixelsPerUnit,	"(): Point"),
			FUNC_ENTRY_H(scroll,					"(pt: Point)"),
			FUNC_ENTRY_H(setScrollRate,				"(xstep, ystep)"),
			FUNC_ENTRY_H(setScrollbars,				"(pixelsPerUnitX, pixelsPerUnitY, numUnitsX, numUnitsY, xpos=0, ypos=0, noRefresh=false)"),
			NULL
		};

		bind<TClass>(v, props, funcs);

		addStaticTable<TClass>(v, "SHOW_SB");
		newSlot(v, -1, "DEFAULT",		(int)wxSHOW_SB_DEFAULT);
		newSlot(v, -1, "ALWAYS",		(int)wxSHOW_SB_ALWAYS);
		newSlot(v, -1, "NEVER",			(int)wxSHOW_SB_NEVER);
		sq_poptop(v);
	}

	NB_PROP_GET(viewStart)				{ return push(v, self(v)->GetViewStart()); }
	NB_PROP_GET(targetWindow)			{ return push(v, self(v)->GetTargetWindow()); }

	NB_PROP_SET(targetWindow)			{ self(v)->SetTargetWindow(opt<wxWindow>(v, 2, NULL)); return 0; }

	NB_FUNC(calcScrolledPosition)		{ wxPoint p = *get<wxPoint>(v, 2); wxPoint r; self(v)->CalcScrolledPosition(p.x, p.y, &r.x, &r.y); return push(v, r); }
	NB_FUNC(calcUnscrolledPosition)		{ wxPoint p = *get<wxPoint>(v, 2); wxPoint r; self(v)->CalcUnscrolledPosition(p.x, p.y, &r.x, &r.y); return push(v, r); }
	NB_FUNC(disableKeyboardScrolling)	{ self(v)->DisableKeyboardScrolling(); return 0; }
	NB_FUNC(enableScrolling)			{ self(v)->EnableScrolling(getBool(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC(showScrollbars)				{ self(v)->ShowScrollbars((wxScrollbarVisibility)optInt(v, 2, wxSHOW_SB_DEFAULT), (wxScrollbarVisibility)optInt(v, 3, wxSHOW_SB_DEFAULT)); return 0; }
	NB_FUNC(getScrollPixelsPerUnit)		{ wxPoint p; self(v)->GetScrollPixelsPerUnit(&p.x, &p.y); return push(v, p); }
	NB_FUNC(scroll)						{ wxPoint p = *get<wxPoint>(v, 2); self(v)->Scroll(p); return 0; }
	NB_FUNC(setScrollRate)				{ self(v)->SetScrollRate(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(setScrollbars)				{ self(v)->SetScrollbars(getInt(v, 2), getInt(v, 3), getInt(v, 4), getInt(v, 5), optInt(v, 6, 0), optInt(v, 7, 0), optBool(v, 8, false)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

// TODO: grid?
// TODO: htlbox (html list box), combo box etc
// TODO: html window
// TODO: sash ? splitwindow ? (splitter is newer?)
// TODO: scroll bar?
// TODO: splash
// TODO: spinbtn, spinctrl, spinctrldbl
// TODO: color picker
// TODO: editable list box
// TODO: hyper link
// TODO: wrap sizer
// TODO: popup, transient window

////////////////////////////////////////////////////////////////////////////////

class NITWX_API wxScriptFrame : public wxFrame
{
public:
	wxScriptFrame(ScriptPeer* peer);
	virtual ~wxScriptFrame();

	nit::Ref<ScriptPeer>				_peer;
};

////////////////////////////////////////////////////////////////////////////////

class NITWX_API wxScriptDialog : public wxDialog
{
public:
	wxScriptDialog(ScriptPeer* peer);
	virtual ~wxScriptDialog();

	nit::Ref<ScriptPeer>				_peer;
};

////////////////////////////////////////////////////////////////////////////////

class NITWX_API wxScriptPanel : public wxPanel
{
public:
	wxScriptPanel(ScriptPeer* peer);
	virtual ~wxScriptPanel();

	nit::Ref<ScriptPeer>				_peer;
};

////////////////////////////////////////////////////////////////////////////////

class NITWX_API wxScriptWizard : public wxWizard
{
public:
	wxScriptWizard(ScriptPeer* peer);
	virtual ~wxScriptWizard();

	nit::Ref<ScriptPeer>				_peer;
};

////////////////////////////////////////////////////////////////////////////////

class NITWX_API wxScriptWizardPage : public wxWizardPage
{
public:
	wxScriptWizardPage(ScriptPeer* peer);
	virtual ~wxScriptWizardPage();

	nit::Ref<ScriptPeer>				_peer;

	virtual wxBitmap GetBitmap() const;
	virtual wxWizardPage* GetNext() const;
	virtual wxWizardPage* GetPrev() const;
};

////////////////////////////////////////////////////////////////////////////////

class NITWX_API wxScriptData : public wxClientData
{
public:
	wxScriptData(HSQUIRRELVM v, int idx)
	{
		_peer = new ScriptPeer(v, idx);
	}

	~wxScriptData()
	{
	}

	int PushInto(HSQUIRRELVM v)
	{
		_peer->pushObject(v);
		return 1;
	}

	void Replace(HSQUIRRELVM v, int idx)
	{
		_peer->replace(v, idx);
	}

private:
	Ref<ScriptPeer>	_peer;
};

////////////////////////////////////////////////////////////////////////////////

class NITWX_API ScriptWxWeakTracker : public ScriptRuntime::Extension
{
public:
	class TrackerNode;

	ScriptWxWeakTracker(ScriptRuntime* env);
	virtual ~ScriptWxWeakTracker();
public:
	void 								add(wxTrackable* trackable, void* object);
	void 								release(wxTrackable* trackable, void* object);
	void 								clear();

protected:
	void								onDestroy(wxTrackable* object, void* basePtr);

	Weak<ScriptRuntime> _env;
	typedef unordered_map<wxTrackable*, TrackerNode*>::type Targets;
	Targets _targets;
};

////////////////////////////////////////////////////////////////////////////////

template <typename wxArrayType>
inline int PushWxArray(HSQUIRRELVM v, const wxArrayType& arr)
{
	sq_newarray(v, arr.size());
	for (uint i=0; i<arr.size(); ++i)
	{
		sq_pushinteger(v, i);
		NitBind::push(v, arr[i]);
		sq_set(v, -3);
	}
	return 1;
}

////////////////////////////////////////////////////////////////////////////////

inline wxClassInfo* GetWxClassInfo(HSQUIRRELVM v, int idx)
{
	if (NitBind::isString(v, idx))
	{
		wxClassInfo* info = wxClassInfo::FindClass(NitBind::getWxString(v, idx));
		if (info == NULL)
		{
			sqx_throwfmt(v, "invalid wx classname: '%s'", sqx_getstring(v, idx));
		}
		return info;
	}

	return NitBind::get<wxClassInfo>(v, idx);
}

inline wxClassInfo* OptWxClassInfo(HSQUIRRELVM v, int idx, wxClassInfo* def)
{
	if (NitBind::isString(v, idx))
	{
		wxClassInfo* info = wxClassInfo::FindClass(NitBind::getWxString(v, idx));
		if (info == NULL)
		{
			sqx_throwfmt(v, "invalid wx classname: '%s'", sqx_getstring(v, idx));
		}
		return info;
	}

	return NitBind::opt<wxClassInfo>(v, idx, def);
}

////////////////////////////////////////////////////////////////////////////////

inline wxColour GetWxColor(HSQUIRRELVM v, int idx)
{
	// Actually an ABGR format (little-endian)
	if (NitBind::isInt(v, idx))
	{
		int bgr = NitBind::getInt(v, idx);
		bgr = ((bgr & 0xFF0000) >> 16) | (bgr & 0x00FF00) | ((bgr & 0x0000FF) << 16);
		return wxColour(bgr);
	}
	else if (NitBind::isString(v, idx))
		return wxColour(NitBind::getString(v, idx));

	Color c = *NitBind::get<Color>(v, idx);
	ABGR abgr = c.getAsABGR();
	wxColour wxc;
	wxc.SetRGBA(abgr);
	return wxc;
}

inline wxColour OptWxColor(HSQUIRRELVM v, int idx, const wxColour& def)
{
	if (NitBind::isNone(v, idx) || NitBind::isNull(v, idx))
		return def;

	return GetWxColor(v, idx);
}

inline int PushWxColor(HSQUIRRELVM v, const wxColour& c)
{
	Color oc;
	oc.setAsABGR(ABGR(c.GetRGBA()));
	return NitBind::push(v, oc);
}

////////////////////////////////////////////////////////////////////////////////

inline wxRealPoint GetWxRealPoint(HSQUIRRELVM v, int idx)
{
	if (NitBind::is<wxPoint>(v, idx))
	{
		wxPoint& p = *NitBind::get<wxPoint>(v, idx);
		return wxRealPoint(p);
	}
	else
	{
		Vector2& p = *NitBind::get<Vector2>(v, idx);
		return wxRealPoint(p.x, p.y);
	}
}

inline int PushWxRealPoint(HSQUIRRELVM v, const wxRealPoint& p)
{
	return NitBind::push(v, Vector2((float)p.x, (float)p.y));
}

////////////////////////////////////////////////////////////////////////////////

inline SQInteger PushWxScriptData(HSQUIRRELVM v, wxClientData* obj)
{
	if (obj == NULL) return 0;

	if (dynamic_cast<wxScriptData*>(obj))
		return ((wxScriptData*)obj)->PushInto(v);
	else if (dynamic_cast<wxStringClientData*>(obj))
		return NitBind::push(v, ((wxStringClientData*)obj)->GetData());

	sq_pushuserpointer(v, obj);
	return 1;
}

inline wxClientData* GetWxScriptData(HSQUIRRELVM v, int idx, wxClientData* obj)
{
	if (NitBind::isNone(v, idx) || NitBind::isNull(v, idx))
		return NULL;

	if (NitBind::isString(v, idx))
	{
		if (dynamic_cast<wxStringClientData*>(obj))
		{
			((wxStringClientData*)obj)->SetData(NitBind::getWxString(v, idx));
			return obj;
		}
		else
		{
			return new wxStringClientData(NitBind::getWxString(v, idx));
		}
	}
	else
	{
		if (dynamic_cast<wxScriptData*>(obj))
		{
			((wxScriptData*)obj)->Replace(v, idx);
			return obj;
		}
		else
		{
			return new wxScriptData(v, idx);
		}
	}
	return obj;
}

////////////////////////////////////////////////////////////////////////////////

inline int GetWxArrayString(HSQUIRRELVM v, int arrayIndex, wxArrayString& ret)
{
	arrayIndex = NitBind::toAbsIdx(v, arrayIndex);
	if (NitBind::isNone(v, arrayIndex)) return 0;
	if (sq_gettype(v, arrayIndex) != OT_ARRAY) return -1;

	int numItems = sq_getsize(v, arrayIndex);

	if (numItems == 0) return 0;

	ret.resize(numItems);
	for (int i=0; i<numItems; ++i)
	{
		sq_pushinteger(v, i);
		sq_get(v, arrayIndex);
		sq_tostring(v, -1);
		ret[i] = NitBind::getWxString(v, -1);
		sq_pop(v, 2);
	}

	return numItems;
}

inline int GetWxArrayInt(HSQUIRRELVM v, int arrayIndex, wxArrayInt& ret)
{
	arrayIndex = NitBind::toAbsIdx(v, arrayIndex);
	if (NitBind::isNone(v, arrayIndex)) return 0;
	if (sq_gettype(v, arrayIndex) != OT_ARRAY) return -1;

	int numItems = sq_getsize(v, arrayIndex);

	if (numItems == 0) return 0;

	ret.resize(numItems);
	for (int i=0; i<numItems; ++i)
	{
		sq_pushinteger(v, i);
		sq_get(v, arrayIndex);
		ret[i] = NitBind::getInt(v, -1);
		sq_pop(v, 1);
	}

	return numItems;
}

template <typename TArray>
inline int GetWxArray(HSQUIRRELVM v, int arrayIndex, TArray& ret)
{
	arrayIndex = NitBind::toAbsIdx(v, arrayIndex);
	if (NitBind::isNone(v, arrayIndex)) return 0;
	if (sq_gettype(v, arrayIndex) != OT_ARRAY) return -1;

	int numItems = sq_getsize(v, arrayIndex);

	if (numItems == 0) return 0;

	ret.resize(numItems);
	for (int i=0; i<numItems; ++i)
	{
		sq_pushinteger(v, i);
		sq_get(v, arrayIndex);
		ret[i] = *NitBind::get<TArray::value_type>(v, -1);
		sq_pop(v, 1);
	}

	return numItems;
}

inline int GetWxArrayScriptData(HSQUIRRELVM v, int arrayIndex, vector<wxClientData*>::type& ret)
{
	arrayIndex = NitBind::toAbsIdx(v, arrayIndex);
	if (NitBind::isNone(v, arrayIndex)) return 0;
	if (sq_gettype(v, arrayIndex) != OT_ARRAY) return -1;

	int numItems = sq_getsize(v, arrayIndex);

	if (numItems == 0) return 0;

	ret.resize(numItems);
	for (int i=0; i<numItems; ++i)
	{
		sq_pushinteger(v, i);
		sq_get(v, arrayIndex);
		ret[i] = GetWxScriptData(v, -1, NULL);
		sq_pop(v, 1);
	}

	return numItems;
}

////////////////////////////////////////////////////////////////////////////////

class NITWX_API wxVariantDataValue : public wxVariantData
{
public:
	wxVariantDataValue(const DataValue& value) : _value(value) { }

public:
	inline const DataValue&				GetValue()								{ return _value; }

public:									// wxVariantData impl
	virtual wxVariantData*				Clone()									{ return new wxVariantDataValue(_value); }
	virtual bool						Eq(wxVariantData& data) const			{ return false; }
	virtual wxString					GetType() const							{ return "DataValue"; }

	virtual bool						GetAsAny(wxAny* any) const
	{ 
		switch (_value.getType())
		{
		case DataValue::TYPE_BOOL:		*any = _value.getData<uint8>() != 0; return true;
		case DataValue::TYPE_INT:		*any = _value.getData<int>(); return true;
		case DataValue::TYPE_INT64:		*any = _value.getData<int64>(); return true;
		case DataValue::TYPE_DOUBLE:	*any = _value.getData<double>(); return true;
		case DataValue::TYPE_FLOAT:		*any = _value.getData<float>(); return true;
		case DataValue::TYPE_STRING:	*any = wxString(_value.getStringPtr(), _value.getStringPtr() + _value.getStringSize()); return true;
		case DataValue::TYPE_TIMESTAMP:	*any = wxDateTime((time_t)_value.getData<Timestamp>().getUnixTime64()); return true;
		default:						*any = _value; return true; 
		}
	}

private:
	DataValue _value;
};

inline wxVariant GetWxVariant(HSQUIRRELVM v, int index)
{
	switch (sq_gettype(v, index))
	{
	case OT_NULL:						return (void*)NULL;
	case OT_INTEGER:					return NitBind::getInt(v, index);
	case OT_FLOAT:						return NitBind::getFloat(v, index);
	case OT_STRING:						return NitBind::getWxString(v, index);
	case OT_BOOL:						return NitBind::getBool(v, index);

	case OT_INSTANCE:
		// TODO: Optimize to table lookup instead of expensive Is<XXX>() call
		if (NitBind::is<Color>(v, index))				return wxVariant(GetWxColor(v, index));
		if (NitBind::is<wxPoint>(v, index))				return wxVariant(*NitBind::get<wxPoint>(v, index));
		if (NitBind::is<wxSize>(v, index))				return wxVariant(*NitBind::get<wxSize>(v, index));
		if (NitBind::is<wxRect>(v, index))				return wxVariant(*NitBind::get<wxRect>(v, index));
		if (NitBind::is<wxImage>(v, index))				return wxVariant(*NitBind::get<wxImage>(v, index));
		if (NitBind::is<wxBitmap>(v, index))			return wxVariant(*NitBind::get<wxBitmap>(v, index));
		if (NitBind::is<wxIcon>(v, index))				return wxVariant(*NitBind::get<wxIcon>(v, index));
		if (NitBind::is<wxFont>(v, index))				return wxVariant(*NitBind::get<wxFont>(v, index));
		if (NitBind::is<wxDataViewIconText>(v, index))	return wxVariant(*NitBind::get<wxDataViewIconText>(v, index));
		if (NitBind::is<wxObject>(v, index))			return NitBind::get<wxObject>(v, index);
	}

	DataValue value;
	SQRESULT sr = ScriptDataValue::toValue(v, index, value);

	if (SQ_FAILED(sr))
		sqx_throw(v, "not supported wxVariant");

	return wxVariant(new wxVariantDataValue(value));
}

inline int GetWxVariantVector(HSQUIRRELVM v, int arrayIndex, wxVector<wxVariant>& ret)
{
	if (NitBind::isNone(v, arrayIndex)) return 0;
	if (sq_gettype(v, arrayIndex) != OT_ARRAY) return -1;

	int numItems = sq_getsize(v, arrayIndex);

	if (numItems == 0) return 0;

	ret.resize(numItems);
	for (int i=0; i<numItems; ++i)
	{
		sq_pushinteger(v, i);
		sq_get(v, arrayIndex);
		ret[i] = GetWxVariant(v, -1);
		sq_pop(v, 1);
	}

	return numItems;
}

inline SQInteger PushWxVariant(HSQUIRRELVM v, const wxVariant& value)
{
	wxAny any = value.GetAny();

	// TODO: Optimize to table lookup instead of if statement?

	if (any.IsNull())							return NitBind::pushNull(v);

	if (any.CheckType<int>())					return NitBind::push(v, any.As<int>());
	if (any.CheckType<uint>())					return NitBind::push(v, (int)any.As<uint>());
	if (any.CheckType<int16>())					return NitBind::push(v, (int)any.As<int16>());
	if (any.CheckType<uint16>())				return NitBind::push(v, (int)any.As<uint16>());
	if (any.CheckType<int8>())					return NitBind::push(v, (int)any.As<int8>());
	if (any.CheckType<uint8>())					return NitBind::push(v, (int)any.As<uint8>());
	if (any.CheckType<int64>())					return NitBind::push(v, (int)any.As<int64>());
	if (any.CheckType<uint64>())				return NitBind::push(v, (int)any.As<uint64>());

	if (any.CheckType<float>())					return NitBind::push(v, any.As<float>());
	if (any.CheckType<double>())				return NitBind::push(v, (float)any.As<double>());

	if (any.CheckType<bool>())					return NitBind::push(v, any.As<bool>());
	if (any.CheckType<wxString>())				return NitBind::push(v, any.As<wxString>());
	if (any.CheckType<wxColour>())				return PushWxColor(v, any.As<wxColour>());
	if (any.CheckType<wxPoint>())				return NitBind::push(v, any.As<wxPoint>());
	if (any.CheckType<wxSize>())				return NitBind::push(v, any.As<wxSize>());
	if (any.CheckType<wxRect>())				return NitBind::push(v, any.As<wxRect>());
	if (any.CheckType<wxImage>())				return NitBind::push(v, any.As<wxImage>());
	if (any.CheckType<wxBitmap>())				return NitBind::push(v, any.As<wxBitmap>());
	if (any.CheckType<wxIcon>())				return NitBind::push(v, any.As<wxIcon>());
	if (any.CheckType<wxFont>())				return NitBind::push(v, any.As<wxFont>());
	if (any.CheckType<wxDataViewIconText>())	return NitBind::push(v, any.As<wxDataViewIconText>());
	if (any.CheckType<DataValue>())				return ScriptDataValue::push(v, any.As<DataValue>());
	if (any.CheckType<wxObject*>())				return NitBind::push(v, any.As<wxObject*>());

	if (any.CheckType<wxArrayString>())			return PushWxArray(v, any.As<wxArrayString>());
	if (any.CheckType<wxArrayInt>())			return PushWxArray(v, any.As<wxArrayInt>());

	sqx_throw(v, "not supported wxVariant");
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

class wxNitInputStream : public wxInputStream
{
public:
	wxNitInputStream(nit::Ref<StreamReader> r) : _reader(r) { }

public:
	StreamReader*						GetReader()								{ return _reader; }

public:
	virtual bool						IsSeekable() const						{ return _reader->isSeekable(); }
	virtual size_t						GetSize() const							{ return _reader->isSized() ? _reader->getSize() : 0; }
	virtual wxFileOffset				GetLength() const						{ return _reader->isSized() ? _reader->getSize() : wxInvalidOffset; }

protected:
	virtual size_t						OnSysRead(void *buffer, size_t size)	{ return _reader->readRaw(buffer, size); }
	virtual wxFileOffset				OnSysSeek(wxFileOffset seek, wxSeekMode mode);
	virtual wxFileOffset				OnSysTell() const;

	nit::Ref<StreamReader>				_reader;
};

////////////////////////////////////////////////////////////////////////////////

class wxNitOutputStream : public wxOutputStream
{
public:
	wxNitOutputStream(nit::Ref<StreamWriter> w) : _writer(w) { }

public:
	StreamWriter*						GetWriter()								{ return _writer; }

public:
	virtual bool						IsSeekable() const						{ return _writer->isSeekable(); }
	virtual size_t						GetSize() const							{ return _writer->isSized() ? _writer->getSize() : 0; }
	virtual wxFileOffset				GetLength() const						{ return _writer->isSized() ? _writer->getSize() : wxInvalidOffset; }

protected:
	virtual size_t						OnSysWrite(const void *buffer, size_t bufsize)				{ return _writer->writeRaw(buffer, bufsize); }
	virtual wxFileOffset				OnSysSeek(wxFileOffset seek, wxSeekMode mode);
	virtual wxFileOffset				OnSysTell() const;

	nit::Ref<StreamWriter>				_writer;
};

////////////////////////////////////////////////////////////////////////////////

wxImage wxLoadNitImage(Ref<StreamSource> source, wxBitmapType type = wxBITMAP_TYPE_ANY, int index = -1);

////////////////////////////////////////////////////////////////////////////////

class NITWX_API NB_WxKeyboardState : TNitInterface<wxKeyboardState>
{
public:
	template <typename TClass>
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(modifiers),
			PROP_ENTRY_R(controlDown),
			PROP_ENTRY_R(shiftDown),
			PROP_ENTRY_R(altDown),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind<TClass>(v, props, funcs);
	}

	NB_PROP_GET(modifiers)							{ return push(v, self(v)->GetModifiers()); }
	NB_PROP_GET(controlDown)						{ return push(v, self(v)->ControlDown()); }
	NB_PROP_GET(shiftDown)							{ return push(v, self(v)->ShiftDown()); }
	NB_PROP_GET(altDown)							{ return push(v, self(v)->AltDown()); }
};

////////////////////////////////////////////////////////////////////////////////

class NITWX_API NB_WxMouseState : TNitInterface<wxMouseState>
{
public:
	template <typename TClass>
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(x),
			PROP_ENTRY_R(y),
			PROP_ENTRY_R(position),
			PROP_ENTRY_R(leftIsDown),
			PROP_ENTRY_R(middleIsDown),
			PROP_ENTRY_R(rightIsDown),
			PROP_ENTRY_R(aux1IsDown),
			PROP_ENTRY_R(aux2IsDown),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(buttonIsDown,	"(button: BTN): bool"),
			NULL
		};

		bind<TClass>(v, props, funcs);

		NB_WxKeyboardState::Register<TClass>(v);

		TNitClass<TClass>::addStaticTable(v, "BTN");
		newSlot(v, -1, "ANY",			(int)wxMOUSE_BTN_ANY);
		newSlot(v, -1, "NONE",			(int)wxMOUSE_BTN_NONE);
		newSlot(v, -1, "LEFT",			(int)wxMOUSE_BTN_LEFT);
		newSlot(v, -1, "MIDDLE",		(int)wxMOUSE_BTN_MIDDLE);
		newSlot(v, -1, "RIGHT",			(int)wxMOUSE_BTN_RIGHT);
		newSlot(v, -1, "AUX1",			(int)wxMOUSE_BTN_AUX1);
		newSlot(v, -1, "AUX2",			(int)wxMOUSE_BTN_AUX2);
		sq_poptop(v);

		TNitClass<TClass>::addStaticTable(v, "WHEEL");
		newSlot(v, -1, "VERTICAL",		(int)wxMOUSE_WHEEL_VERTICAL);
		newSlot(v, -1, "HORIZONTAL",	(int)wxMOUSE_WHEEL_HORIZONTAL);
		sq_poptop(v);
	}

	NB_PROP_GET(x)						{ return push(v, self(v)->GetX()); }
	NB_PROP_GET(y)						{ return push(v, self(v)->GetY()); }
	NB_PROP_GET(position)				{ return push(v, self(v)->GetPosition()); }
	NB_PROP_GET(leftIsDown)			{ return push(v, self(v)->LeftIsDown()); }
	NB_PROP_GET(middleIsDown)			{ return push(v, self(v)->MiddleIsDown()); }
	NB_PROP_GET(rightIsDown)			{ return push(v, self(v)->RightIsDown()); }
	NB_PROP_GET(aux1IsDown)			{ return push(v, self(v)->Aux1IsDown()); }
	NB_PROP_GET(aux2IsDown)			{ return push(v, self(v)->Aux2IsDown()); }

	NB_FUNC(buttonIsDown)				{ return push(v, self(v)->ButtonIsDown((wxMouseButton)getInt(v, 2))); }
	
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
