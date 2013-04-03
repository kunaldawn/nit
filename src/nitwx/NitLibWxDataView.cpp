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

#include "nitwx_pch.h"

#include "nitwx/NitLibWx.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

inline static int PushWxDataViewItem(HSQUIRRELVM v, const wxDataViewItem& item)
{
	sq_pushinteger(v, (int)item.GetID());
	return 1;
}

inline static wxDataViewItem GetWxDataViewItem(HSQUIRRELVM v, int idx)
{
	return wxDataViewItem((void*)NitBind::getInt(v, idx));
}

inline static int PushWxDataViewItemArray(HSQUIRRELVM v, wxDataViewItemArray& array)
{
	sq_newarray(v, 0);
	for (uint i=0; i<array.size(); ++i)
	{
		PushWxDataViewItem(v, array[i]);
		sq_arrayappend(v, -2);
	}
	return 1;
}

inline static int GetWxDataViewItemArray(HSQUIRRELVM v, int arrayIndex, wxDataViewItemArray& ret)
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
		ret[i] = wxDataViewItem((void*)NitBind::getInt(v, -1));
		sq_poptop(v);
	}

	return numItems;
}

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, DataViewEvent, wxNotifyEvent);

class NB_WxDataViewEvent : TNitClass<wxDataViewEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(item),
			PROP_ENTRY_R(column),
			PROP_ENTRY_R(dataViewColumn),
			PROP_ENTRY_R(model),
			PROP_ENTRY_R(position),
			PROP_ENTRY_R(value),
			PROP_ENTRY_R(editCancelled),
			PROP_ENTRY_R(dataFormat),
			PROP_ENTRY_R(dataSize),
			PROP_ENTRY_R(dropEffect),
			PROP_ENTRY_R(cacheFrom),
			PROP_ENTRY_R(cacheTo),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(dataBuffer,	"(): MemoryBuffer"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(item)					{ return PushWxDataViewItem(v, self(v)->GetItem()); }
	NB_PROP_GET(column)					{ return push(v, self(v)->GetColumn()); }
	NB_PROP_GET(model)					{ return push(v, self(v)->GetModel()); }
	NB_PROP_GET(position)				{ return push(v, self(v)->GetPosition()); }
	NB_PROP_GET(value)					{ return PushWxVariant(v, self(v)->GetValue()); }
	NB_PROP_GET(dataViewColumn)			{ return push(v, self(v)->GetDataViewColumn()); }
	NB_PROP_GET(editCancelled)			{ return push(v, self(v)->IsEditCancelled()); }
	NB_PROP_GET(dataFormat)				{ return push(v, self(v)->GetDataFormat().GetFormatId()); }
	NB_PROP_GET(dataSize)				{ return push(v, self(v)->GetDataSize()); }
	NB_PROP_GET(dropEffect)				{ return push(v, (int)self(v)->GetDropEffect()); }
	NB_PROP_GET(cacheFrom)				{ return push(v, self(v)->GetCacheFrom()); }
	NB_PROP_GET(cacheTo)				{ return push(v, self(v)->GetCacheTo()); }

	NB_FUNC(dataBuffer)				
	{ 
		void* buf = self(v)->GetDataBuffer();
		if (buf)
			return push(v, new MemoryBuffer(buf, self(v)->GetDataSize())); 
		else
			return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, DataViewModelNotifier, NULL);

class NB_WxDataViewModelNotifier : TNitClass<wxDataViewModelNotifier>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(owner),
			NULL
		};

		FuncEntry funcs[] = 
		{
			NULL
		};

		// This class is a derive point when you need to receive DataViewCtrl events
		// So we did not bind template methods.

		bind(v, props, funcs);
	}

	NB_PROP_GET(owner)					{ return push(v, self(v)->GetOwner()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXVALUE(NITWX_API, DataViewItemAttr, NULL);

class NB_WxDataViewItemAttr : TNitClass<wxDataViewItemAttr>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(bold),
			PROP_ENTRY	(color),
			PROP_ENTRY	(bgColor),
			PROP_ENTRY	(italic),
			NULL
		};

		FuncEntry funcs[] = 
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(bold)					{ return push(v, self(v)->GetBold()); }
	NB_PROP_GET(color)					{ return PushWxColor(v, self(v)->GetColour()); }
	NB_PROP_GET(bgColor)				{ return PushWxColor(v, self(v)->GetBackgroundColour()); }
	NB_PROP_GET(italic)					{ return push(v, self(v)->GetItalic()); }

	NB_PROP_SET(bold)					{ self(v)->SetBold(getBool(v, 2)); return 0; }
	NB_PROP_SET(color)					{ self(v)->SetColour(GetWxColor(v, 2)); return 0; }
	NB_PROP_SET(bgColor)				{ self(v)->SetBackgroundColour(GetWxColor(v, 2)); return 0; }
	NB_PROP_SET(italic)					{ self(v)->SetItalic(getBool(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

class wxScriptDataViewModel : public wxDataViewModel
{
public:
	wxScriptDataViewModel(HSQUIRRELVM v, int peerIdx);

public:									// wxDataViewModel impl
	virtual unsigned int				GetChildren(const wxDataViewItem& item, wxDataViewItemArray& children) const;
	virtual unsigned int				GetColumnCount() const;
	virtual wxString					GetColumnType(unsigned int col) const;
	virtual wxDataViewItem				GetParent(const wxDataViewItem& item) const;
	virtual void						GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const;
	virtual bool						IsContainer(const wxDataViewItem& item) const;
	virtual bool						SetValue(const wxVariant& variant, const wxDataViewItem& item, unsigned int col);

public:									// wxDataViewModel overrides
	virtual bool						HasContainerColumns(const wxDataViewItem& item) const;
	virtual bool						HasDefaultCompare() const;
	virtual int							Compare(const wxDataViewItem& item1, const wxDataViewItem& item2, unsigned int column, bool ascending);
	virtual bool						GetAttr(const wxDataViewItem& item, unsigned int col, wxDataViewItemAttr& attr);
	virtual bool						IsEnabled(const wxDataViewItem& item, unsigned int col) const;

public:									// wxDataViewModel notification overrides
	virtual bool						Cleared();
	virtual void						Resort();
	virtual bool						ValueChanged(const wxDataViewItem& item, unsigned int col);

public:
	ScriptPeer*							GetPeer()								{ return _peer; }

protected:
	typedef wxDataViewModel				inherited;

	Ref<ScriptPeer>						_peer;
	HSQOBJECT							_getChildren;
	HSQOBJECT							_getColumnCount;
	HSQOBJECT							_getColumnType;
	HSQOBJECT							_getParent;
	HSQOBJECT							_getValue;
	HSQOBJECT							_isContainer;
	HSQOBJECT							_setValue;

	HSQOBJECT							_hasContainerColumns;
	HSQOBJECT							_hasDefaultCompare;
	HSQOBJECT							_compare;
	HSQOBJECT							_getAttr;
	HSQOBJECT							_isEnabled;

	HSQOBJECT							_cleared;
	HSQOBJECT							_resort;
	HSQOBJECT							_valueChanged;

	bool								LinkMethod(HSQUIRRELVM v, int peerIdx, const char* mname, HSQOBJECT& holder);
};

////////////////////////////////////////////////////////////////////////////////

wxScriptDataViewModel::wxScriptDataViewModel(HSQUIRRELVM v, int peerIdx)
{
	_peer = new ScriptPeer(v, peerIdx);

	bool ok = true;

	_peer->makeWeak(v);

	_peer->pushObject(v);
	peerIdx = NitBind::toAbsIdx(v, -1);

	// abstract
	ok = ok && LinkMethod(v, peerIdx, "getChildren",	_getChildren);
	ok = ok && LinkMethod(v, peerIdx, "getColumnCount",	_getColumnCount);
	ok = ok && LinkMethod(v, peerIdx, "getColumnType",	_getColumnType);
	ok = ok && LinkMethod(v, peerIdx, "getParent",		_getParent);
	ok = ok && LinkMethod(v, peerIdx, "getValue",		_getValue);
	ok = ok && LinkMethod(v, peerIdx, "isContainer",	_isContainer);
	ok = ok && LinkMethod(v, peerIdx, "setValue",		_setValue);

	if (!ok) 
	{
		sq_poptop(v);
		NIT_THROW_FMT(EX_SCRIPT, "incomplete DataViewModel peer");
	}

	// overrides - Not needed
	LinkMethod(v, peerIdx, "hasContainerColumns",		_hasContainerColumns);
	LinkMethod(v, peerIdx, "hasDefaultCompare",			_hasDefaultCompare);
	LinkMethod(v, peerIdx, "compare",					_compare);
	LinkMethod(v, peerIdx, "getAttr",					_getAttr);
	LinkMethod(v, peerIdx, "isEnabled",					_isEnabled);

	LinkMethod(v, peerIdx, "cleared",					_cleared);
	LinkMethod(v, peerIdx, "resort",					_resort);
	LinkMethod(v, peerIdx, "valueChanged",				_valueChanged);

	// Connected closure object are referenced by the peer or the class.
	// So we don't need to addref / release again.

	sq_poptop(v);
}

bool wxScriptDataViewModel::LinkMethod(HSQUIRRELVM v, int peerIdx, const char* mname, HSQOBJECT& holder)
{
	sq_resetobject(&holder);

	sq_pushstring(v, mname, -1);

	if (SQ_FAILED(sq_get(v, peerIdx)))
		return false;

	sq_getstackobj(v, -1, &holder);
	sq_poptop(v);
	return true;
}

class StackCheck
{
public:
	StackCheck(HSQUIRRELVM v)
	{
		_vm = v;
		_top = sq_gettop(v);
	}

	~StackCheck()
	{
		if (sq_gettop(_vm) != _top)
		{
			ASSERT(false);
		}
	}

	HSQUIRRELVM _vm;
	int _top;
};

unsigned int wxScriptDataViewModel::GetChildren(const wxDataViewItem& item, wxDataViewItemArray& children) const
{
	children.clear();
	HSQUIRRELVM v = _peer->getWorker();
	if (v == NULL) return 0;

	StackCheck sc(v);

	PushWxDataViewItem(v, item);
	ScriptResult sr = _peer->callWith(_getChildren, 1, true);
	if (sr == SCRIPT_CALL_OK && sq_gettype(v, -1) == OT_ARRAY)
	{
		GetWxDataViewItemArray(v, -1, children);
		sq_poptop(v);
		return children.size();
	}
	return 0;
}

unsigned int wxScriptDataViewModel::GetColumnCount() const
{
	HSQUIRRELVM v = _peer->getWorker();
	if (v == NULL) return 0;

	StackCheck sc(v);

	int count = 0;

	ScriptResult sr = _peer->callWith(_getColumnCount, 0, true);
	if (sr == SCRIPT_CALL_OK)
	{
		count = NitBind::getInt(v, -1);
		sq_poptop(v);
	}
	return count;
}

wxString wxScriptDataViewModel::GetColumnType(unsigned int col) const
{
	wxString result;

	HSQUIRRELVM v = _peer->getWorker();
	if (v == NULL) return result;

	StackCheck sc(v);

	sq_pushinteger(v, col);
	ScriptResult sr = _peer->callWith(_getColumnType, 1, true);
	if (sr == SCRIPT_CALL_OK)
	{
		result = NitBind::getWxString(v, -1);
		sq_poptop(v);
	}
	return result;
}

wxDataViewItem wxScriptDataViewModel::GetParent(const wxDataViewItem& item) const
{
	wxDataViewItem parent;

	HSQUIRRELVM v = _peer->getWorker();
	if (v == NULL) return parent;

	StackCheck sc(v);

	PushWxDataViewItem(v, item);
	ScriptResult sr = _peer->callWith(_getParent, 1, true);
	if (sr == SCRIPT_CALL_OK)
	{
		parent = GetWxDataViewItem(v, -1);
		sq_poptop(v);
	}
	return parent;
}

void wxScriptDataViewModel::GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const
{
	HSQUIRRELVM v = _peer->getWorker();
	if (v == NULL) return;

	StackCheck sc(v);

	PushWxDataViewItem(v, item);
	sq_pushinteger(v, col);
	ScriptResult sr = _peer->callWith(_getValue, 2, true);
	if (sr == SCRIPT_CALL_OK)
	{
		variant = GetWxVariant(v, -1);
		sq_poptop(v);
	}
}

bool wxScriptDataViewModel::IsContainer(const wxDataViewItem& item) const
{
	bool ret = false;

	HSQUIRRELVM v = _peer->getWorker();
	if (v == NULL) return ret;

	StackCheck sc(v);

	PushWxDataViewItem(v, item);
	ScriptResult sr = _peer->callWith(_isContainer, 1, true);
	if (sr == SCRIPT_CALL_OK)
	{
		ret = NitBind::getBool(v, -1);
		sq_poptop(v);
	}

	return ret;
}

bool wxScriptDataViewModel::SetValue(const wxVariant& variant, const wxDataViewItem& item, unsigned int col)
{
	bool ret = false;

	HSQUIRRELVM v = _peer->getWorker();
	if (v == NULL) return ret;

	StackCheck sc(v);

	PushWxDataViewItem(v, item);
	sq_pushinteger(v, col);
	PushWxVariant(v, variant);
	ScriptResult sr = _peer->callWith(_setValue, 3, true);
	if (sr == SCRIPT_CALL_OK)
	{
		ret = NitBind::getBool(v, -1);
		sq_poptop(v);
	}

	return ret;
}

bool wxScriptDataViewModel::HasContainerColumns(const wxDataViewItem& item) const
{
	HSQUIRRELVM v = _peer->getWorker();
	if (v == NULL) return false;

	StackCheck sc(v);

	PushWxDataViewItem(v, item);
	ScriptResult sr = _peer->callWith(_hasContainerColumns, 1, true);

	bool ret;

	if (sr == SCRIPT_CALL_OK)
	{
		ret = NitBind::getBool(v, -1);
		sq_poptop(v);
	}
	else ret = inherited::HasContainerColumns(item);

	return ret;
}

bool wxScriptDataViewModel::HasDefaultCompare() const
{
	HSQUIRRELVM v = _peer->getWorker();
	if (v == NULL) return inherited::HasDefaultCompare();

	StackCheck sc(v);

	ScriptResult sr = _peer->callWith(_hasDefaultCompare, 0, true);

	bool ret;

	if (sr == SCRIPT_CALL_OK)
	{
		ret = NitBind::getBool(v, -1);
		sq_poptop(v);
	}
	else ret = inherited::HasDefaultCompare();

	return ret;
}

int	wxScriptDataViewModel::Compare(const wxDataViewItem& item1, const wxDataViewItem& item2, unsigned int column, bool ascending)
{
	HSQUIRRELVM v = _peer->getWorker();
	if (v == NULL) return inherited::Compare(item1, item2, column, ascending);

	StackCheck sc(v);

	PushWxDataViewItem(v, item1);
	PushWxDataViewItem(v, item2);
	sq_pushinteger(v, column);
	sq_pushbool(v, ascending);

	ScriptResult sr = _peer->callWith(_compare, 4, true);

	int ret;

	if (sr == SCRIPT_CALL_OK)
	{
		ret = NitBind::getInt(v, -1);
		sq_poptop(v);
	}
	else ret = inherited::Compare(item1, item2, column, ascending);

	return ret;
}

bool wxScriptDataViewModel::GetAttr(const wxDataViewItem& item, unsigned int col, wxDataViewItemAttr& attr)
{
	HSQUIRRELVM v = _peer->getWorker();
	if (v == NULL) return inherited::GetAttr(item, col, attr);

	StackCheck sc(v);

	PushWxDataViewItem(v, item);
	sq_pushinteger(v, col);

	ScriptResult sr = _peer->callWith(_getAttr, 2, true);

	bool ret;

	if (sr == SCRIPT_CALL_OK)
	{
		attr = *NitBind::get<wxDataViewItemAttr>(v, -1);
		ret = true;
		sq_poptop(v);
	}
	else ret = inherited::GetAttr(item, col, attr);

	return ret;
}

bool wxScriptDataViewModel::IsEnabled(const wxDataViewItem& item, unsigned int col) const
{
	HSQUIRRELVM v = _peer->getWorker();
	if (v == NULL) return inherited::IsEnabled(item, col);

	StackCheck sc(v);

	PushWxDataViewItem(v, item);
	sq_pushinteger(v, col);

	ScriptResult sr = _peer->callWith(_isEnabled, 2, true);

	bool ret;

	if (sr == SCRIPT_CALL_OK)
	{
		ret = NitBind::getBool(v, -1);
		sq_poptop(v);
	}
	else ret = inherited::IsEnabled(item, col);

	return ret;
}

// NOTE: Below implementations must call its inherited one unless returning false.
// (Inherited one notifies to notifiers)

bool wxScriptDataViewModel::Cleared()
{
	HSQUIRRELVM v = _peer->getWorker();
	if (v == NULL) return inherited::Cleared();

	StackCheck sc(v);

	ScriptResult sr = _peer->callWith(_cleared, 0, true);

	bool ret;

	if (sr == SCRIPT_CALL_OK)
	{
		ret = NitBind::getBool(v, -1);
		sq_poptop(v);

		if (!ret) return false;
	}

	ret = inherited::Cleared();

	return ret;
}

void wxScriptDataViewModel::Resort()
{
	HSQUIRRELVM v = _peer->getWorker();
	if (v == NULL) return inherited::Resort();;

	StackCheck sc(v);

	_peer->callWith(_resort, 0, false);

	inherited::Resort();
}

bool wxScriptDataViewModel::ValueChanged(const wxDataViewItem& item, unsigned int col)
{
	HSQUIRRELVM v = _peer->getWorker();
	if (v == NULL) return inherited::ValueChanged(item, col);

	StackCheck sc(v);

	PushWxDataViewItem(v, item);
	sq_pushinteger(v, col);

	ScriptResult sr = _peer->callWith(_valueChanged, 2, true);

	bool ret;

	if (sr == SCRIPT_CALL_OK)
	{
		ret = NitBind::getBool(v, -1);
		sq_poptop(v);

		if (!ret) return false;
	}

	ret = inherited::ValueChanged(item, col);

	return ret;
}

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXREF(NITWX_API, DataViewModel, NULL);

class NB_WxDataViewModel : TNitClass<wxDataViewModel>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(refCount),
			PROP_ENTRY_R(columnCount),
			PROP_ENTRY_R(peer),

			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(peer)"),

			FUNC_ENTRY_H(addNotifier,	"(notifier: wx.DataViewModelNotifier)"),
			FUNC_ENTRY_H(changeValue,	"(item: int, col: int, value): bool"),
			FUNC_ENTRY_H(cleared,		"(): bool"),
			FUNC_ENTRY_H(compare,		"(item1, item2: int, column: int, ascending: bool): int"),
			FUNC_ENTRY_H(getAttr,		"(item: int, col: int): wxDataViewItemAttr"),
			FUNC_ENTRY_H(isEnabled,		"(item: int, col: int): bool"),
			FUNC_ENTRY_H(getChildren,	"(item: int): int[]"),
			FUNC_ENTRY_H(getColumnType,	"(col: int): string"),
			FUNC_ENTRY_H(getParent,		"(item: int): int"),
			FUNC_ENTRY_H(getValue,		"(item: int, col: int): value"),
			FUNC_ENTRY_H(hasValue,		"(item: int, col: int): bool"),
			FUNC_ENTRY_H(isContainer,	"(item: int): bool"),
			FUNC_ENTRY_H(itemAdded,		"(parent: int, item: int): bool"),
			FUNC_ENTRY_H(itemChanged,	"(item: int): bool"),
			FUNC_ENTRY_H(itemDeleted,	"(parent: int, item: int): bool"),
			FUNC_ENTRY_H(itemsAdded,	"(parent: int, items: int[]): bool"),
			FUNC_ENTRY_H(itemsChanged,	"(items: int[]): bool"),
			FUNC_ENTRY_H(itemsDeleted,	"(parent: int, items: int[]): bool"),
			FUNC_ENTRY_H(removeNotifier,"(notifier: wx.DataViewModelNotifier)"),
			FUNC_ENTRY_H(resort,		"()"),
			FUNC_ENTRY_H(setValue,		"(item: int, col: int, value): bool"),
			FUNC_ENTRY_H(valueChanged,	"(item: int, col: int): bool"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(refCount)				{ return push(v, self(v)->GetRefCount()); }
	NB_PROP_GET(columnCount)			{ return push(v, self(v)->GetColumnCount()); }
	NB_PROP_GET(hasDefaultCompare)		{ return push(v, self(v)->HasDefaultCompare()); }

	NB_PROP_GET(peer)
	{
		wxScriptDataViewModel* m = dynamic_cast<wxScriptDataViewModel*>(self(v));
		if (m == NULL) return 0;

		return m->GetPeer()->pushObject(v);
	}

	NB_CONS()
	{
		setSelf(v, new wxScriptDataViewModel(v, 2));
		return SQ_OK;
	}

	NB_FUNC(addNotifier)				{ self(v)->AddNotifier(get<wxDataViewModelNotifier>(v, 2)); return 0; }
	NB_FUNC(changeValue)				{ return push(v, self(v)->ChangeValue(GetWxVariant(v, 4), GetWxDataViewItem(v, 3), getInt(v, 2))); }
	NB_FUNC(cleared)					{ return push(v, self(v)->Cleared()); }
	NB_FUNC(compare)					{ return push(v, self(v)->Compare(GetWxDataViewItem(v, 2), GetWxDataViewItem(v, 3), getInt(v, 4), getBool(v, 5))); }
	NB_FUNC(getAttr)					{ wxDataViewItemAttr attr; if (self(v)->GetAttr(GetWxDataViewItem(v, 2), getInt(v, 3), attr)) return push(v, attr); else return 0; }
	NB_FUNC(isEnabled)					{ return push(v, self(v)->IsEnabled(GetWxDataViewItem(v, 2), getInt(v, 3))); }

	NB_FUNC(getChildren)
	{
		wxDataViewItemArray children;
		int count = self(v)->GetChildren(GetWxDataViewItem(v, 2), children);
		return PushWxDataViewItemArray(v, children);
	}

	NB_FUNC(getColumnType)				{ return push(v, self(v)->GetColumnType(getInt(v, 2))); }
	NB_FUNC(getParent)					{ return PushWxDataViewItem(v, self(v)->GetParent(GetWxDataViewItem(v, 2))); }
	NB_FUNC(getValue)					{ wxVariant value; self(v)->GetValue(value, GetWxDataViewItem(v, 2), getInt(v, 3)); return PushWxVariant(v, value); }
	NB_FUNC(hasValue)					{ return push(v, self(v)->HasValue(GetWxDataViewItem(v, 2), getInt(v, 3))); }
	NB_FUNC(isContainer)				{ return push(v, self(v)->IsContainer(GetWxDataViewItem(v, 2))); }
	NB_FUNC(itemAdded)					{ return push(v, self(v)->ItemAdded(GetWxDataViewItem(v, 2), GetWxDataViewItem(v, 3))); }
	NB_FUNC(itemChanged)				{ return push(v, self(v)->ItemChanged(GetWxDataViewItem(v, 2))); }
	NB_FUNC(itemDeleted)				{ return push(v, self(v)->ItemDeleted(GetWxDataViewItem(v, 2), GetWxDataViewItem(v, 3))); }

	NB_FUNC(itemsAdded)
	{
		if (sq_gettype(v, 3) != OT_ARRAY) 
			return sq_throwerror(v, "array expected");

		wxDataViewItemArray array;
		GetWxDataViewItemArray(v, 3, array);

		return push(v, self(v)->ItemsAdded(GetWxDataViewItem(v, 2), array));
	}

	NB_FUNC(itemsChanged)
	{
		if (sq_gettype(v, 2) != OT_ARRAY) 
			return sq_throwerror(v, "array expected");

		wxDataViewItemArray array;
		GetWxDataViewItemArray(v, 3, array);

		return push(v, self(v)->ItemsChanged(array));
	}

	NB_FUNC(itemsDeleted)
	{
		if (sq_gettype(v, 3) != OT_ARRAY) 
			return sq_throwerror(v, "array expected");

		wxDataViewItemArray array;
		GetWxDataViewItemArray(v, 3, array);

		return push(v, self(v)->ItemsDeleted(GetWxDataViewItem(v, 2), array));
	}

	NB_FUNC(removeNotifier)				{ self(v)->RemoveNotifier(get<wxDataViewModelNotifier>(v, 2)); return 0; }
	NB_FUNC(resort)						{ self(v)->Resort(); return 0; }
	NB_FUNC(setValue)					{ return push(v, self(v)->SetValue(GetWxVariant(v, 4), GetWxDataViewItem(v, 2), getInt(v, 3))); }
	NB_FUNC(valueChanged)				{ return push(v, self(v)->ValueChanged(GetWxDataViewItem(v, 2), getInt(v, 3))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXREF(NITWX_API, DataViewListModel, wxDataViewModel);

class NB_WxDataViewListModel : TNitClass<wxDataViewListModel>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(count),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(getRow,		"(item: int): int"),
			FUNC_ENTRY_H(getValueByRow,	"(row, col: int): value"),
			FUNC_ENTRY_H(setValueByRow,	"(row, col: int, value)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(count)					{ return push(v, self(v)->GetCount()); }

	NB_FUNC(getRow)						{ return push(v, self(v)->GetRow(GetWxDataViewItem(v, 2))); }
	NB_FUNC(getValueByRow)				{ wxVariant var; self(v)->GetValueByRow(var, getInt(v, 2), getInt(v, 3)); return PushWxVariant(v, var); }
	NB_FUNC(setValueByRow)				{ self(v)->SetValueByRow(GetWxVariant(v, 4), getInt(v, 2), getInt(v, 3)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXREF(NITWX_API, DataViewIndexListModel, wxDataViewListModel);

class NB_WxDataViewIndexListModel : TNitClass<wxDataViewIndexListModel>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(getItem,		"(row: int): int"),
			FUNC_ENTRY_H(reset,			"(newSize: int)"),
			FUNC_ENTRY_H(rowAppended,	"()"),
			FUNC_ENTRY_H(rowChanged,	"(row: int)"),
			FUNC_ENTRY_H(rowDeleted,	"()"),
			FUNC_ENTRY_H(rowInserted,	"(before: int)"),
			FUNC_ENTRY_H(rowPrepended,	"()"),
			FUNC_ENTRY_H(rowValueChanged, "(row, col: int)"),
			FUNC_ENTRY_H(rowsDeleted,	"(rows: int[])"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_FUNC(getItem)					{ return PushWxDataViewItem(v, self(v)->GetItem(getInt(v, 2))); }
	NB_FUNC(reset)						{ self(v)->Reset(getInt(v, 2)); return 0; }
	NB_FUNC(rowAppended)				{ self(v)->RowAppended(); return 0; }
	NB_FUNC(rowChanged)					{ self(v)->RowChanged(getInt(v, 2)); return 0; }
	NB_FUNC(rowDeleted)					{ self(v)->RowDeleted(getInt(v, 2)); return 0; }
	NB_FUNC(rowInserted)				{ self(v)->RowInserted(getInt(v, 2)); return 0; }
	NB_FUNC(rowPrepended)				{ self(v)->RowPrepended(); return 0; }
	NB_FUNC(rowValueChanged)			{ self(v)->RowValueChanged(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(rowsDeleted)				{ wxArrayInt rows; GetWxArrayInt(v, 2, rows); self(v)->RowsDeleted(rows); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

//NB_TYPE_WXREF(NITWX_API, DataViewVirtualListModel, wxDataViewListModel);
//
//class NB_WxDataViewVirtualListModel : TNitClass<wxDataViewVirtualListModel>
//{
//public:
//	static void Register(HSQUIRRELVM v)
//	{
//		PropEntry props[] =
//		{
//			NULL
//		};
//
//		FuncEntry funcs[] =
//		{
//			FUNC_ENTRY_H(getItem,		"(row: int): int"),
//			FUNC_ENTRY_H(reset,			"(newSize: int)"),
//			FUNC_ENTRY_H(rowAppended,	"()"),
//			FUNC_ENTRY_H(rowChanged,	"(row: int)"),
//			FUNC_ENTRY_H(rowDeleted,	"()"),
//			FUNC_ENTRY_H(rowInserted,	"(before: int)"),
//			FUNC_ENTRY_H(rowPrepended,	"()"),
//			FUNC_ENTRY_H(rowValueChanged, "(row, col: int)"),
//			FUNC_ENTRY_H(rowsDeleted,	"(rows: int[])"),
//			NULL
//		};
//
//		Bind(v, props, funcs);
//	}
//
//	NB_FUNC(getItem)					{ return PushWxDataViewItem(v, Self(v)->GetItem(GetInt(v, 2))); }
//	NB_FUNC(reset)						{ Self(v)->Reset(GetInt(v, 2)); return 0; }
//	NB_FUNC(rowAppended)				{ Self(v)->RowAppended(); return 0; }
//	NB_FUNC(rowChanged)					{ Self(v)->RowChanged(GetInt(v, 2)); return 0; }
//	NB_FUNC(rowDeleted)					{ Self(v)->RowDeleted(GetInt(v, 2)); return 0; }
//	NB_FUNC(rowInserted)				{ Self(v)->RowInserted(GetInt(v, 2)); return 0; }
//	NB_FUNC(rowPrepended)				{ Self(v)->RowPrepended(); return 0; }
//	NB_FUNC(rowValueChanged)			{ Self(v)->RowValueChanged(GetInt(v, 2), GetInt(v, 3)); return 0; }
//	NB_FUNC(rowsDeleted)				{ wxArrayInt rows; GetWxArrayInt(v, 2, rows); Self(v)->RowsDeleted(rows); return 0; }
//};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXREF(NITWX_API, DataViewListStore, wxDataViewIndexListModel);

class NB_WxDataViewListStore : TNitClass<wxDataViewListStore>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(itemCount),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(					"()"),

			FUNC_ENTRY_H(prependColumn,		"(variantType: string)"),
			FUNC_ENTRY_H(insertColumn,		"(pos: int, variantType: string)"),
			FUNC_ENTRY_H(appendColumn,		"(variantType: string)"),

			FUNC_ENTRY_H(appendItem,		"(values: [], tag: int=0)"),
			FUNC_ENTRY_H(prependItem,		"(values: [], tag: int=0)"),
			FUNC_ENTRY_H(insertItem,		"(row: int, values: [], tag: int=0)"),
			FUNC_ENTRY_H(deleteItem,		"(pos: int)"),
			FUNC_ENTRY_H(deleteAllItems,	"()"),

			FUNC_ENTRY_H(setItemTag,		"(item: int, tag: int)"),
			FUNC_ENTRY_H(getItemTag,		"(item: int): int"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(itemCount)				{ return push(v, self(v)->GetItemCount()); }

	NB_CONS()							{ setSelf(v, new wxDataViewListStore()); return SQ_OK; }

	NB_FUNC(prependColumn)				{ self(v)->PrependColumn(getWxString(v, 2)); return 0; }
	NB_FUNC(insertColumn)				{ self(v)->InsertColumn(getInt(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(appendColumn)				{ self(v)->AppendColumn(getWxString(v, 2)); return 0; }
	
	NB_FUNC(appendItem)				
	{
		int tag = optInt(v, 3, 0);
		wxVector<wxVariant> values;
		GetWxVariantVector(v, 2, values);
		self(v)->AppendItem(values, wxUIntPtr(tag)); 
		return 0; 
	}

	NB_FUNC(prependItem)
	{
		int tag = optInt(v, 3, 0);
		wxVector<wxVariant> values;
		GetWxVariantVector(v, 2, values);
		self(v)->PrependItem(values, wxUIntPtr(tag)); 
		return 0; 
	}

	NB_FUNC(insertItem)
	{
		int row = getInt(v, 2);
		int tag = optInt(v, 4, 0);
		wxVector<wxVariant> values;
		GetWxVariantVector(v, 3, values);
		self(v)->InsertItem(row, values, wxUIntPtr(tag)); 
		return 0; 
	}

	NB_FUNC(deleteItem)					{ self(v)->DeleteItem(getInt(v, 2)); return 0; }
	NB_FUNC(deleteAllItems)				{ self(v)->DeleteAllItems(); return 0; }
	NB_FUNC(setItemTag)					{ self(v)->SetItemData(GetWxDataViewItem(v, 2), wxUIntPtr(getInt(v, 3))); return 0; }
	NB_FUNC(getItemTag)					{ return push(v, (int)self(v)->GetItemData(GetWxDataViewItem(v, 2))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXREF(NITWX_API, DataViewTreeStore, wxDataViewModel);

class NB_WxDataViewTreeStore : TNitClass<wxDataViewTreeStore>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(						"()"),

			FUNC_ENTRY_H(appendContainer,		"(parent: int, text: string, icon=-1, expanded=-1, data: object=null): int"),
			FUNC_ENTRY_H(appendItem,			"(parent: int, text: string, icon=-1, data: object=null): int"),
			FUNC_ENTRY_H(deleteAllItems,		"()"),
			FUNC_ENTRY_H(deleteChildren,		"(item: int)"),
			FUNC_ENTRY_H(deleteItem,			"(item: int)"),
			FUNC_ENTRY_H(getChildCount,			"(parent: int)"),
			FUNC_ENTRY_H(getItemData,			"(item: int): object"),
			FUNC_ENTRY_H(getItemExpandedIcon,	"(item: int): wx.Icon"),
			FUNC_ENTRY_H(getItemIcon,			"(item: int): wx.Icon"),
			FUNC_ENTRY_H(getItemText,			"(item: int): string"),
			FUNC_ENTRY_H(getNthChild,			"(parent: int, pos: int): int"),
			FUNC_ENTRY_H(insertContainer,		"(parent: int, previous: int, text: string, icon=-1, expanded=-1, data: object=null): int"),
			FUNC_ENTRY_H(insertItem,			"(parent: int, previous: int, text: string, icon=-1, data: object=null): int"),
			FUNC_ENTRY_H(prependContainer,		"(parent: int, text: string, icon=-1, expanded=-1, data: object=null): int"),
			FUNC_ENTRY_H(prependItem,			"(parent: int, text: string, icon=-1, data: object=null): int"),
			FUNC_ENTRY_H(setItemData,			"(item: int, data: object)"),
			FUNC_ENTRY_H(setItemExpandedIcon,	"(item: int, icon: wx.Icon)"),
			FUNC_ENTRY_H(setItemIcon,			"(item: int, icon: wx.Icon)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, new wxDataViewTreeStore()); return SQ_OK; }

	NB_FUNC(appendContainer)			{ self(v)->AppendContainer(GetWxDataViewItem(v, 2), getWxString(v, 3), *opt<wxIcon>(v, 4, wxNullIcon), *opt<wxIcon>(v, 5, wxNullIcon), GetWxScriptData(v, 6, NULL)); return 0; }
	NB_FUNC(appendItem)					{ self(v)->AppendItem(GetWxDataViewItem(v, 2), getWxString(v, 3), *opt<wxIcon>(v, 4, wxNullIcon), GetWxScriptData(v, 5, NULL)); return 0; }
	NB_FUNC(prependContainer)			{ self(v)->PrependContainer(GetWxDataViewItem(v, 2), getWxString(v, 3), *opt<wxIcon>(v, 4, wxNullIcon), *opt<wxIcon>(v, 5, wxNullIcon), GetWxScriptData(v, 6, NULL)); return 0; }
	NB_FUNC(prependItem)				{ self(v)->PrependItem(GetWxDataViewItem(v, 2), getWxString(v, 3), *opt<wxIcon>(v, 4, wxNullIcon), GetWxScriptData(v, 5, NULL)); return 0; }
	NB_FUNC(insertContainer)			{ self(v)->InsertContainer(GetWxDataViewItem(v, 2), GetWxDataViewItem(v, 3), getWxString(v, 4), *opt<wxIcon>(v, 5, wxNullIcon), *opt<wxIcon>(v, 6, wxNullIcon), GetWxScriptData(v, 7, NULL)); return 0; }
	NB_FUNC(insertItem)					{ self(v)->InsertItem(GetWxDataViewItem(v, 2), GetWxDataViewItem(v, 3), getWxString(v, 4), *opt<wxIcon>(v, 5, wxNullIcon), GetWxScriptData(v, 6, NULL)); return 0; }

	NB_FUNC(deleteAllItems)				{ self(v)->DeleteAllItems(); return 0; }
	NB_FUNC(deleteChildren)				{ self(v)->DeleteChildren(GetWxDataViewItem(v, 2)); return 0; }
	NB_FUNC(deleteItem)					{ self(v)->DeleteItem(GetWxDataViewItem(v, 2)); return 0; }
	NB_FUNC(getChildCount)				{ return push(v, self(v)->GetChildCount(GetWxDataViewItem(v, 2))); }
	NB_FUNC(getItemData)				{ return PushWxScriptData(v, self(v)->GetItemData(GetWxDataViewItem(v, 2))); }
	NB_FUNC(getItemExpandedIcon)		{ return push(v, self(v)->GetItemExpandedIcon(GetWxDataViewItem(v, 2))); }
	NB_FUNC(getItemIcon)				{ return push(v, self(v)->GetItemIcon(GetWxDataViewItem(v, 2))); }
	NB_FUNC(getItemText)				{ return push(v, self(v)->GetItemText(GetWxDataViewItem(v, 2))); }
	NB_FUNC(getNthChild)				{ return PushWxDataViewItem(v, self(v)->GetNthChild(GetWxDataViewItem(v, 2), getInt(v, 3))); }
	NB_FUNC(setItemData)				{ self(v)->SetItemData(GetWxDataViewItem(v, 2), GetWxScriptData(v, 3, NULL)); return 0; }
	NB_FUNC(setItemExpandedIcon)		{ self(v)->SetItemExpandedIcon(GetWxDataViewItem(v, 2), *get<wxIcon>(v, 3)); return 0; }
	NB_FUNC(setItemIcon)				{ self(v)->SetItemIcon(GetWxDataViewItem(v, 2), *get<wxIcon>(v, 3)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, HeaderColumn, NULL);

class NB_WxHeaderColumn : TNitClass<wxHeaderColumn>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(title),
			PROP_ENTRY_R(bitmap),
			PROP_ENTRY_R(width),
			PROP_ENTRY_R(minWidth),
			PROP_ENTRY_R(alignment),
			PROP_ENTRY_R(flags),
			PROP_ENTRY_R(resizable),
			PROP_ENTRY_R(sortable),
			PROP_ENTRY_R(reorderable),
			PROP_ENTRY_R(visible),
			PROP_ENTRY_R(sortKey),
			PROP_ENTRY_R(sortOrderAscending),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(title)					{ return push(v, self(v)->GetTitle()); }
	NB_PROP_GET(bitmap)					{ return push(v, self(v)->GetBitmap()); }
	NB_PROP_GET(width)					{ return push(v, self(v)->GetWidth()); }
	NB_PROP_GET(minWidth)				{ return push(v, self(v)->GetMinWidth()); }
	NB_PROP_GET(alignment)				{ return push(v, (int)self(v)->GetAlignment()); }
	NB_PROP_GET(flags)					{ return push(v, self(v)->GetFlags()); }
	NB_PROP_GET(resizable)				{ return push(v, self(v)->IsResizeable()); }
	NB_PROP_GET(sortable)				{ return push(v, self(v)->IsSortable()); }
	NB_PROP_GET(reorderable)			{ return push(v, self(v)->IsReorderable()); }
	NB_PROP_GET(visible)				{ return push(v, self(v)->IsShown()); }
	NB_PROP_GET(sortKey)				{ return push(v, self(v)->IsSortKey()); }
	NB_PROP_GET(sortOrderAscending)		{ return push(v, self(v)->IsSortOrderAscending()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, SettableHeaderColumn, wxHeaderColumn);

class NB_WxSettableHeaderColumn : TNitClass<wxSettableHeaderColumn>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(title),
			PROP_ENTRY	(bitmap),
			PROP_ENTRY	(width),
			PROP_ENTRY	(minWidth),
			PROP_ENTRY	(alignment),
			PROP_ENTRY	(flags),
			PROP_ENTRY	(resizable),
			PROP_ENTRY	(sortable),
			PROP_ENTRY	(reorderable),
			PROP_ENTRY	(visible),
			PROP_ENTRY	(sortOrderAscending),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(changeFlag,		"(flag: int, set: bool"),
			FUNC_ENTRY_H(toggleFlag,		"(flag: int)"),
			FUNC_ENTRY_H(unsetAsSortKey,	"()"),
			FUNC_ENTRY_H(toggleSortOrder,	"()"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(title)					{ return push(v, self(v)->GetTitle()); }
	NB_PROP_GET(bitmap)					{ return push(v, self(v)->GetBitmap()); }
	NB_PROP_GET(width)					{ return push(v, self(v)->GetWidth()); }
	NB_PROP_GET(minWidth)				{ return push(v, self(v)->GetMinWidth()); }
	NB_PROP_GET(alignment)				{ return push(v, (int)self(v)->GetAlignment()); }
	NB_PROP_GET(flags)					{ return push(v, self(v)->GetFlags()); }
	NB_PROP_GET(resizable)				{ return push(v, self(v)->IsResizeable()); }
	NB_PROP_GET(sortable)				{ return push(v, self(v)->IsSortable()); }
	NB_PROP_GET(reorderable)			{ return push(v, self(v)->IsReorderable()); }
	NB_PROP_GET(visible)				{ return push(v, self(v)->IsShown()); }
	NB_PROP_GET(sortOrderAscending)		{ return push(v, self(v)->IsSortOrderAscending()); }

	NB_PROP_SET(title)					{ self(v)->SetTitle(getWxString(v, 2)); return 0; }
	NB_PROP_SET(bitmap)					{ self(v)->SetBitmap(*opt<wxBitmap>(v, 2, wxNullBitmap)); return 0; }
	NB_PROP_SET(width)					{ self(v)->SetWidth(getInt(v, 2)); return 0; }
	NB_PROP_SET(minWidth)				{ self(v)->SetMinWidth(getInt(v, 2)); return 0; }
	NB_PROP_SET(alignment)				{ self(v)->SetAlignment((wxAlignment)getInt(v, 2)); return 0; }
	NB_PROP_SET(flags)					{ self(v)->SetFlags(getInt(v, 2)); return 0; }
	NB_PROP_SET(resizable)				{ self(v)->SetResizeable(getBool(v, 2)); return 0; }
	NB_PROP_SET(sortable)				{ self(v)->SetSortable(getBool(v, 2)); return 0; }
	NB_PROP_SET(reorderable)			{ self(v)->SetReorderable(getBool(v, 2)); return 0; }
	NB_PROP_SET(visible)				{ self(v)->SetHidden(!getBool(v, 2)); return 0; }
	NB_PROP_SET(sortOrderAscending)		{ self(v)->SetSortOrder(getBool(v, 2)); return 0; }

	NB_FUNC(changeFlag)					{ self(v)->ChangeFlag(getInt(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC(toggleFlag)					{ self(v)->ToggleFlag(getInt(v, 2)); return 0; }
	NB_FUNC(unsetAsSortKey)				{ self(v)->UnsetAsSortKey(); return 0; }
	NB_FUNC(toggleSortOrder)			{ self(v)->ToggleSortOrder(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, DataViewColumn, wxSettableHeaderColumn);

class NB_WxDataViewColumn : TNitClass<wxDataViewColumn>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(modelColumn),
			PROP_ENTRY_R(owner),
			PROP_ENTRY_R(renderer),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(title: string or wx.Bitmap, renderer: wx.DataViewRenderer, model_column: int, width=80, align=wx.ALIGN.CENTER, flags=wx.DataViewCtrl.COL.RESIZABLE)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		wxDataViewRenderer* rdr = get<wxDataViewRenderer>(v, 3);
		int col = getInt(v, 4);
		int width = optInt(v, 5, wxDVC_DEFAULT_WIDTH);
		wxAlignment align = (wxAlignment)optInt(v, 6, wxALIGN_CENTER);
		int flags = optInt(v, 7, wxDATAVIEW_COL_RESIZABLE);

		if (is<wxBitmap>(v, 2))
			setSelf(v, new wxDataViewColumn(*get<wxBitmap>(v, 2), rdr, col, width, align, flags));
		else
			setSelf(v, new wxDataViewColumn(getWxString(v, 2), rdr, col, width, align, flags));
		return SQ_OK;
	}

	NB_PROP_GET(modelColumn)			{ return push(v, self(v)->GetModelColumn()); }
	NB_PROP_GET(owner)					{ return push(v, self(v)->GetOwner()); }
	NB_PROP_GET(renderer)				{ return push(v, self(v)->GetRenderer()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, DataViewRenderer, wxObject);

class NB_WxDataViewRenderer : TNitClass<wxDataViewRenderer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(alignment),
			PROP_ENTRY_R(ellipsizeMode),
			PROP_ENTRY_R(mode),
			PROP_ENTRY_R(owner),
			PROP_ENTRY_R(variantType),
			PROP_ENTRY	(value),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(enableEllipsize,	"(mode=ELLIPSIZE.MIDDLE)"),
			FUNC_ENTRY_H(disableEllipsize,	"()"),
			FUNC_ENTRY_H(validate,			"(value): bool"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "ELLIPSIZE");
		newSlot(v, -1, "NONE",			(int)wxELLIPSIZE_NONE);
		newSlot(v, -1, "START",			(int)wxELLIPSIZE_START);
		newSlot(v, -1, "MIDDLE",		(int)wxELLIPSIZE_MIDDLE);
		newSlot(v, -1, "END",			(int)wxELLIPSIZE_END);
		sq_poptop(v);
	}

	NB_PROP_GET(alignment)				{ return push(v, self(v)->GetAlignment()); }
	NB_PROP_GET(ellipsizeMode)			{ return push(v, (int)self(v)->GetEllipsizeMode()); }
	NB_PROP_GET(mode)					{ return push(v, (int)self(v)->GetMode()); }
	NB_PROP_GET(owner)					{ return push(v, self(v)->GetOwner()); }
	NB_PROP_GET(variantType)			{ return push(v, self(v)->GetVariantType()); }
	NB_PROP_GET(value)					{ wxVariant value; if (self(v)->GetValue(value)) return PushWxVariant(v, value); return 0; }

	NB_PROP_SET(value)					{ self(v)->SetValue(GetWxVariant(v, 2)); return 0; }

	NB_FUNC(enableEllipsize)			{ self(v)->EnableEllipsize((wxEllipsizeMode)getInt(v, 2)); return 0; }
	NB_FUNC(disableEllipsize)			{ self(v)->DisableEllipsize(); return 0; }
	NB_FUNC(validate)					{ wxVariant var = GetWxVariant(v, 2); return push(v, self(v)->Validate(var)); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, DataViewBitmapRenderer, wxDataViewRenderer);

class NB_WxDataViewBitmapRenderer : TNitClass<wxDataViewBitmapRenderer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				""),
			NULL
		};

		bind(v, NULL, funcs);
	}

	NB_CONS()							{ setSelf(v, new wxDataViewBitmapRenderer()); return SQ_OK; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, DataViewChoiceRenderer, wxDataViewRenderer);

class NB_WxDataViewChoiceRenderer : TNitClass<wxDataViewChoiceRenderer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(choices),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(choices: string[], mode=wx.DataViewCtrl.CELL.EDITABLE, alignment=wx.ALIGN.DEFAULT)"),
			FUNC_ENTRY_H(getChoice,		"(index: int): string"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(choices)				{ return PushWxArray(v, self(v)->GetChoices()); }

	NB_CONS()
	{
		wxArrayString choices;
		GetWxArrayString(v, 2, choices);
		setSelf(v, new wxDataViewChoiceRenderer(
			choices, 
			(wxDataViewCellMode)optInt(v, 3, wxDATAVIEW_CELL_EDITABLE), 
			optInt(v, 4, wxDVR_DEFAULT_ALIGNMENT)
			));
		return SQ_OK;
	}

	NB_FUNC(getChoice)					{ return push(v, self(v)->GetChoice(getInt(v, 2))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, DataViewDateRenderer, wxDataViewRenderer);

class NB_WxDataViewDateRenderer : TNitClass<wxDataViewDateRenderer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"),
			NULL
		};

		bind(v, NULL, funcs);
	}

	NB_CONS()							{ setSelf(v, new wxDataViewDateRenderer()); return SQ_OK; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXVALUE(NITWX_API, DataViewIconText, NULL);

class NB_WxDataViewIconText : TNitClass<wxDataViewIconText>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(text),
			PROP_ENTRY	(icon),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(text=\"\", icon: wx.Icon=null)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(text)					{ return push(v, self(v)->GetText()); }
	NB_PROP_GET(icon)					{ return push(v, self(v)->GetIcon()); }

	NB_PROP_SET(text)					{ self(v)->SetText(getWxString(v, 2)); return 0; }
	NB_PROP_SET(icon)					{ self(v)->SetIcon(*get<wxIcon>(v, 2)); return 0; }

	NB_CONS()
	{
		new (self(v)) type(optWxString(v, 2, wxEmptyString), *opt<wxIcon>(v, 3, wxNullIcon));
		return SQ_OK;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, DataViewIconTextRenderer, wxDataViewRenderer);

class NB_WxDataViewIconTextRenderer : TNitClass<wxDataViewIconTextRenderer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"),
			NULL
		};

		bind(v, NULL, funcs);
	}

	NB_CONS()							{ setSelf(v, new wxDataViewIconTextRenderer()); return SQ_OK; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, DataViewProgressRenderer, wxDataViewRenderer);

class NB_WxDataViewProgressRenderer : TNitClass<wxDataViewProgressRenderer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"),
			NULL
		};

		bind(v, NULL, funcs);
	}

	NB_CONS()							{ setSelf(v, new wxDataViewProgressRenderer()); return SQ_OK; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, DataViewTextRenderer, wxDataViewRenderer);

class NB_WxDataViewTextRenderer : TNitClass<wxDataViewTextRenderer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"),
			NULL
		};

		bind(v, NULL, funcs);
	}

	NB_CONS()							{ setSelf(v, new wxDataViewTextRenderer()); return SQ_OK; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, DataViewToggleRenderer, wxDataViewRenderer);

class NB_WxDataViewToggleRenderer : TNitClass<wxDataViewToggleRenderer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"),
			NULL
		};

		bind(v, NULL, funcs);
	}

	NB_CONS()							{ setSelf(v, new wxDataViewToggleRenderer()); return SQ_OK; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, DataViewCustomRenderer, wxDataViewRenderer);

class NB_WxDataViewCustomRenderer : TNitClass<wxDataViewCustomRenderer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		// This class is a derive point when you need to override rendering.
		// So we did not bind template methods.

		bind(v, props, funcs);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, DataViewSpinRenderer, wxDataViewCustomRenderer);

class NB_WxDataViewSpinRenderer : TNitClass<wxDataViewSpinRenderer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(min, max: int, mode=wx.DataViewCtrl.CELL.EDITABLE, align=wx.ALIGN.DEFAULT)"),
			NULL
		};

		bind(v, NULL, funcs);
	}

	NB_CONS()							
	{ 
		setSelf(v, new wxDataViewSpinRenderer(
			getInt(v, 2), 
			getInt(v, 3),
			(wxDataViewCellMode)optInt(v, 4, wxDATAVIEW_CELL_EDITABLE),
			optInt(v, 5, wxDVR_DEFAULT_ALIGNMENT)
			)); 
		return SQ_OK; 
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, DataViewCtrl, wxControl);

class NB_WxDataViewCtrl : TNitClass<wxDataViewCtrl>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(columnCount),
			PROP_ENTRY	(indent),
			PROP_ENTRY_R(model),
			PROP_ENTRY_R(selectedItemsCount),
			PROP_ENTRY	(expanderColumn),
			PROP_ENTRY	(selection),
			PROP_ENTRY	(selections),
			PROP_ENTRY_R(sortingColumn),
			PROP_ENTRY	(currentItem),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(						"(parent: wx.Window=null, id=ID.ANY, pos=null, size=null, style=STYLE.DEFAULT, validator:wx.Validator=null, name=null)"),
			FUNC_ENTRY_H(hasSelection,			"(): bool"),
			FUNC_ENTRY_H(appendColumn,			"(col: wx.DataViewColumn): bool"),
			FUNC_ENTRY_H(prependColumn,			"(col: wx.DataViewColumn): bool"),
			FUNC_ENTRY_H(insertColumn,			"(pos: int, col: wx.DataViewColumn): bool"),
			FUNC_ENTRY_H(associateModel,		"(model: wx.DataViewModel): bool"),
			FUNC_ENTRY_H(clearColumns,			"(): bool"),
			FUNC_ENTRY_H(collapse,				"(item: wx.DataViewColumn)"),
			FUNC_ENTRY_H(deleteColumn,			"(column: wx.DataViewColumn): bool"),
			FUNC_ENTRY_H(enableDragSource,		"(format: wx.DF): bool"),
			FUNC_ENTRY_H(enableDropTarget,		"(format: wx.DF): bool"),
			FUNC_ENTRY_H(ensureVisible,			"(item: int, column: wxDataViewColumn=null)"),
			FUNC_ENTRY_H(expand,				"(item: int)"),
			FUNC_ENTRY_H(expandAncestors,		"(item: int)"),
			FUNC_ENTRY_H(getColumn,				"(pos: int): wx.DataViewColumn"),
			FUNC_ENTRY_H(getColumnPosition,		"(column: wx.DataViewColumn): int"),
			FUNC_ENTRY_H(getItemRect,			"(item: int, col: wx.DataViewColumn=null): wx.Rect"),
			FUNC_ENTRY_H(hitTest,				"(pt: wx.Point) : { item: int, col: wx.DataViewColumn }"),
			FUNC_ENTRY_H(isExpanded,			"(item: int): bool"),
			FUNC_ENTRY_H(isSelected,			"(item: int): bool"),
			FUNC_ENTRY_H(select,				"(item: int)"),
			FUNC_ENTRY_H(selectAll,				"()"),
			FUNC_ENTRY_H(startEditor,			"(item: int, column: int)"),
			FUNC_ENTRY_H(unselect,				"(item: int)"),
			FUNC_ENTRY_H(unselectAll,			"()"),
			FUNC_ENTRY_H(setRowHeight,			"(height: int)"),
			FUNC_ENTRY_H(appendBitmapColumn,	"(label: string or wx.Bitmap, model_column: int, mode=CELL.INERT, width=-1, align=wx.ALIGN.CENTER, flags=COL.RESIZABLE): wx.DataViewColumn"),
			FUNC_ENTRY_H(appendDateColumn,		"(label: string or wx.Bitmap, model_column: int, mode=CELL.ACTIVATABLE, width=-1, align=wx.ALIGN.NOT, flags=COL.RESIZABLE): wx.DataViewColumn"),
			FUNC_ENTRY_H(appendIconTextColumn,	"(label: string or wx.Bitmap, model_column: int, mode=CELL.INERT, width=-1, align=wx.ALIGN.NOT, flags=COL.RESIZABLE): wx.DataViewColumn"),
			FUNC_ENTRY_H(appendProgressColumn,	"(label: string or wx.Bitmap, model_column: int, mode=CELL.INERT, width=80, align=wx.ALIGN.CENTER, flags=COL.RESIZABLE): wx.DataViewColumn"),
			FUNC_ENTRY_H(appendTextColumn,		"(label: string or wx.Bitmap, model_column: int, mode=CELL.INERT, width=-1, align=wx.ALIGN.NOT, flags=COL.RESIZABLE): wx.DataViewColumn"),
			FUNC_ENTRY_H(appendToggleColumn,	"(label: string or wx.Bitmap, model_column: int, mode=CELL.INERT, width=30, align=wx.ALIGN.CENTER, flags=COL.RESIZABLE): wx.DataViewColumn"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "CELL");
		newSlot(v, -1, "INERT",			(int)wxDATAVIEW_CELL_INERT);
		newSlot(v, -1, "ACTIVATABLE",	(int)wxDATAVIEW_CELL_ACTIVATABLE);
		newSlot(v, -1, "EDITABLE",		(int)wxDATAVIEW_CELL_EDITABLE);
		sq_poptop(v);

		addStaticTable(v, "CELL_STATE");
		newSlot(v, -1, "SELECTED",		(int)wxDATAVIEW_CELL_SELECTED);
		newSlot(v, -1, "PRELIT",		(int)wxDATAVIEW_CELL_PRELIT);
		newSlot(v, -1, "INSENSITIVE",	(int)wxDATAVIEW_CELL_INSENSITIVE);
		newSlot(v, -1, "FOCUSED",		(int)wxDATAVIEW_CELL_FOCUSED);
		sq_poptop(v);

		addStaticTable(v, "COL");
		newSlot(v, -1, "RESIZABLE",		(int)wxDATAVIEW_COL_RESIZABLE);
		newSlot(v, -1, "SORTABLE",		(int)wxDATAVIEW_COL_SORTABLE);
		newSlot(v, -1, "REORDERABLE",	(int)wxDATAVIEW_COL_REORDERABLE);
		newSlot(v, -1, "HIDDEN",		(int)wxDATAVIEW_COL_HIDDEN);
		sq_poptop(v);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "SINGLE",		(int)wxDV_SINGLE);
		newSlot(v, -1, "MULTIPLE",		(int)wxDV_MULTIPLE);
		newSlot(v, -1, "ROW_LINES",		(int)wxDV_ROW_LINES);
		newSlot(v, -1, "HORIZ_RULES",	(int)wxDV_HORIZ_RULES);
		newSlot(v, -1, "VERT_RULES",	(int)wxDV_VERT_RULES);
		newSlot(v, -1, "VARIABLE_LINE_HEIGHT", (int)wxDV_VARIABLE_LINE_HEIGHT);
		newSlot(v, -1, "NO_HEADER",		(int)wxDV_NO_HEADER);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "SELECTION_CHANGED",		(int)wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED);
		newSlot(v, -1, "ITEM_ACTIVATED",		(int)wxEVT_COMMAND_DATAVIEW_ITEM_ACTIVATED);
		newSlot(v, -1, "ITEM_EDITING_STARTED",	(int)wxEVT_COMMAND_DATAVIEW_ITEM_EDITING_STARTED);
		newSlot(v, -1, "ITEM_EDITING_DONE",		(int)wxEVT_COMMAND_DATAVIEW_ITEM_EDITING_DONE);
		newSlot(v, -1, "ITEM_COLLAPSING",		(int)wxEVT_COMMAND_DATAVIEW_ITEM_COLLAPSING);
		newSlot(v, -1, "ITEM_COLLAPSED",		(int)wxEVT_COMMAND_DATAVIEW_ITEM_COLLAPSED);
		newSlot(v, -1, "ITEM_EXPANDING",		(int)wxEVT_COMMAND_DATAVIEW_ITEM_EXPANDING);
		newSlot(v, -1, "ITEM_EXPANDED",			(int)wxEVT_COMMAND_DATAVIEW_ITEM_EXPANDED);
		newSlot(v, -1, "ITEM_VALUE_CHANGED",	(int)wxEVT_COMMAND_DATAVIEW_ITEM_VALUE_CHANGED);
		newSlot(v, -1, "ITEM_CONTEXT_MENU",		(int)wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU);
		newSlot(v, -1, "COLUMN_HEADER_CLICK",	(int)wxEVT_COMMAND_DATAVIEW_COLUMN_HEADER_CLICK);
		newSlot(v, -1, "COLUMN_HEADER_RIGHT_CLICK", (int)wxEVT_COMMAND_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK);
		newSlot(v, -1, "COLUMN_SORTED",			(int)wxEVT_COMMAND_DATAVIEW_COLUMN_SORTED);
		newSlot(v, -1, "COLUMN_REORDERED",		(int)wxEVT_COMMAND_DATAVIEW_COLUMN_REORDERED);
		newSlot(v, -1, "ITEM_BEGIN_DRAG",		(int)wxEVT_COMMAND_DATAVIEW_ITEM_BEGIN_DRAG);
		newSlot(v, -1, "ITEM_DROP_POSSIBLE",	(int)wxEVT_COMMAND_DATAVIEW_ITEM_DROP_POSSIBLE);
		newSlot(v, -1, "ITEM_DROP",				(int)wxEVT_COMMAND_DATAVIEW_ITEM_DROP);
		newSlot(v, -1, "CACHE_HINT",			(int)wxEVT_COMMAND_DATAVIEW_CACHE_HINT);
		sq_poptop(v);
	}

	NB_PROP_GET(columnCount)			{ return push(v, self(v)->GetColumnCount()); }
	NB_PROP_GET(indent)					{ return push(v, self(v)->GetIndent()); }
	NB_PROP_GET(model)					{ return push(v, self(v)->GetModel()); }
	NB_PROP_GET(selectedItemsCount)		{ return push(v, self(v)->GetSelectedItemsCount()); }
	NB_PROP_GET(expanderColumn)			{ return push(v, self(v)->GetExpanderColumn()); }
	NB_PROP_GET(selection)				{ return PushWxDataViewItem(v, self(v)->GetSelection()); }
	NB_PROP_GET(sortingColumn)			{ return push(v, self(v)->GetSortingColumn()); }
	NB_PROP_GET(currentItem)			{ return PushWxDataViewItem(v, self(v)->GetCurrentItem()); }

	NB_PROP_SET(indent)					{ self(v)->SetIndent(getInt(v, 2)); return 0; }
	NB_PROP_SET(expanderColumn)			{ self(v)->SetExpanderColumn(get<wxDataViewColumn>(v, 2)); return 0; }
	NB_PROP_SET(currentItem)			{ self(v)->SetCurrentItem(GetWxDataViewItem(v, 2)); return 0; }
	NB_PROP_SET(selection)				{ self(v)->UnselectAll(); self(v)->Select(GetWxDataViewItem(v, 2)); return 0; }

	NB_PROP_GET(selections)
	{
		wxDataViewItemArray sel;
		self(v)->GetSelections(sel);
		return PushWxDataViewItemArray(v, sel);
	}

	NB_PROP_SET(selections)
	{
		if (sq_gettype(v, 2) != OT_ARRAY) 
			return sq_throwerror(v, "array expected");

		wxDataViewItemArray array;
		GetWxDataViewItemArray(v, 2, array);

		self(v)->SetSelections(array);
		return 0;
	}

	NB_CONS()
	{
		setSelf(v, new wxDataViewCtrl(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			*opt<wxPoint>(v, 4, wxDefaultPosition),
			*opt<wxSize>(v, 5, wxDefaultSize),
			optInt(v, 6, 0),
			*opt<wxValidator>(v, 7, (wxValidator*)&wxDefaultValidator),
			*optString(v, 8, wxDataViewCtrlNameStr)
			));

		return 0;
	}

	NB_FUNC(hasSelection)				{ return push(v, self(v)->HasSelection()); }

	NB_FUNC(appendColumn)				{ return push(v, self(v)->AppendColumn(get<wxDataViewColumn>(v, 2))); }
	NB_FUNC(prependColumn)				{ return push(v, self(v)->PrependColumn(get<wxDataViewColumn>(v, 2))); }
	NB_FUNC(insertColumn)				{ return push(v, self(v)->InsertColumn(getInt(v, 2), get<wxDataViewColumn>(v, 3))); }
	NB_FUNC(associateModel)				{ return push(v, self(v)->AssociateModel(get<wxDataViewModel>(v, 2))); }
	NB_FUNC(clearColumns)				{ return push(v, self(v)->ClearColumns()); }
	NB_FUNC(collapse)					{ self(v)->Collapse(GetWxDataViewItem(v, 2)); return 0; }
	NB_FUNC(deleteColumn)				{ return push(v, self(v)->DeleteColumn(get<wxDataViewColumn>(v, 2))); }
	NB_FUNC(enableDragSource)			{ return push(v, self(v)->EnableDragSource(wxDataFormat(getInt(v, 2)))); }
	NB_FUNC(enableDropTarget)			{ return push(v, self(v)->EnableDropTarget(wxDataFormat(getInt(v, 2)))); }
	NB_FUNC(ensureVisible)				{ self(v)->EnsureVisible(GetWxDataViewItem(v, 2), opt<wxDataViewColumn>(v, 3, NULL)); return 0; }
	NB_FUNC(expand)						{ self(v)->Expand(GetWxDataViewItem(v, 2)); return 0; }
	NB_FUNC(expandAncestors)			{ self(v)->ExpandAncestors(GetWxDataViewItem(v, 2)); return 0; }
	NB_FUNC(getColumn)					{ return push(v, self(v)->GetColumn(getInt(v, 2))); }
	NB_FUNC(getColumnPosition)			{ return push(v, self(v)->GetColumnPosition(get<wxDataViewColumn>(v, 2))); }
	NB_FUNC(getItemRect)				{ return push(v, self(v)->GetItemRect(GetWxDataViewItem(v, 2), opt<wxDataViewColumn>(v, 3, NULL))); }
	NB_FUNC(isExpanded)					{ return push(v, self(v)->IsExpanded(GetWxDataViewItem(v, 2))); }
	NB_FUNC(isSelected)					{ return push(v, self(v)->IsSelected(GetWxDataViewItem(v, 2))); }
	NB_FUNC(select)						{ self(v)->Select(GetWxDataViewItem(v, 2)); return 0; }
	NB_FUNC(selectAll)					{ self(v)->SelectAll(); return 0; }
	NB_FUNC(startEditor)				{ self(v)->StartEditor(GetWxDataViewItem(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(unselect)					{ self(v)->Unselect(GetWxDataViewItem(v, 2)); return 0; }
	NB_FUNC(unselectAll)				{ self(v)->UnselectAll(); return 0; }
	NB_FUNC(setRowHeight)				{ self(v)->SetRowHeight(getInt(v, 2)); return 0; }

	NB_FUNC(appendBitmapColumn)
	{
		int col = getInt(v, 3);
		wxDataViewCellMode mode = (wxDataViewCellMode)optInt(v, 4, wxDATAVIEW_CELL_INERT);
		int width = optInt(v, 5, -1);
		wxAlignment align = (wxAlignment)optInt(v, 6, wxALIGN_CENTER);
		int flags = optInt(v, 7, wxDATAVIEW_COL_RESIZABLE);

		if (is<wxBitmap>(v, 2))
			return push(v, self(v)->AppendBitmapColumn(*get<wxBitmap>(v, 2), col, mode, width, align, flags));
		else
			return push(v, self(v)->AppendBitmapColumn(getWxString(v, 2), col, mode, width, align, flags));
	}

	NB_FUNC(appendDateColumn)
	{
		int col = getInt(v, 3);
		wxDataViewCellMode mode = (wxDataViewCellMode)optInt(v, 4, wxDATAVIEW_CELL_ACTIVATABLE);
		int width = optInt(v, 5, -1);
		wxAlignment align = (wxAlignment)optInt(v, 6, wxALIGN_NOT);
		int flags = optInt(v, 7, wxDATAVIEW_COL_RESIZABLE);

		if (is<wxBitmap>(v, 2))
			return push(v, self(v)->AppendDateColumn(*get<wxBitmap>(v, 2), col, mode, width, align, flags));
		else
			return push(v, self(v)->AppendDateColumn(getWxString(v, 2), col, mode, width, align, flags));
	}

	NB_FUNC(appendIconTextColumn)
	{
		int col = getInt(v, 3);
		wxDataViewCellMode mode = (wxDataViewCellMode)optInt(v, 4, wxDATAVIEW_CELL_INERT);
		int width = optInt(v, 5, -1);
		wxAlignment align = (wxAlignment)optInt(v, 6, wxALIGN_NOT);
		int flags = optInt(v, 7, wxDATAVIEW_COL_RESIZABLE);

		if (is<wxBitmap>(v, 2))
			return push(v, self(v)->AppendIconTextColumn(*get<wxBitmap>(v, 2), col, mode, width, align, flags));
		else
			return push(v, self(v)->AppendIconTextColumn(getWxString(v, 2), col, mode, width, align, flags));
	}

	NB_FUNC(appendProgressColumn)
	{
		int col = getInt(v, 3);
		wxDataViewCellMode mode = (wxDataViewCellMode)optInt(v, 4, wxDATAVIEW_CELL_INERT);
		int width = optInt(v, 5, 80);
		wxAlignment align = (wxAlignment)optInt(v, 6, wxALIGN_CENTER);
		int flags = optInt(v, 7, wxDATAVIEW_COL_RESIZABLE);

		if (is<wxBitmap>(v, 2))
			return push(v, self(v)->AppendProgressColumn(*get<wxBitmap>(v, 2), col, mode, width, align, flags));
		else
			return push(v, self(v)->AppendProgressColumn(getWxString(v, 2), col, mode, width, align, flags));
	}

	NB_FUNC(appendTextColumn)
	{
		int col = getInt(v, 3);
		wxDataViewCellMode mode = (wxDataViewCellMode)optInt(v, 4, wxDATAVIEW_CELL_INERT);
		int width = optInt(v, 5, -1);
		wxAlignment align = (wxAlignment)optInt(v, 6, wxALIGN_NOT);
		int flags = optInt(v, 7, wxDATAVIEW_COL_RESIZABLE);

		if (is<wxBitmap>(v, 2))
			return push(v, self(v)->AppendTextColumn(*get<wxBitmap>(v, 2), col, mode, width, align, flags));
		else
			return push(v, self(v)->AppendTextColumn(getWxString(v, 2), col, mode, width, align, flags));
	}

	NB_FUNC(appendToggleColumn)
	{
		int col = getInt(v, 3);
		wxDataViewCellMode mode = (wxDataViewCellMode)optInt(v, 4, wxDATAVIEW_CELL_INERT);
		int width = optInt(v, 5, 30);
		wxAlignment align = (wxAlignment)optInt(v, 6, wxALIGN_CENTER);
		int flags = optInt(v, 7, wxDATAVIEW_COL_RESIZABLE);

		if (is<wxBitmap>(v, 2))
			return push(v, self(v)->AppendToggleColumn(*get<wxBitmap>(v, 2), col, mode, width, align, flags));
		else
			return push(v, self(v)->AppendToggleColumn(getWxString(v, 2), col, mode, width, align, flags));
	}

	NB_FUNC(hitTest)
	{ 
		wxDataViewItem item; 
		wxDataViewColumn* col;
		self(v)->HitTest(*get<wxPoint>(v, 2), item, col);
		sq_newtable(v);
		newSlot(v, -1, "item", (int)item.GetID());
		newSlot(v, -1, "col", col);
		return 1;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, DataViewListCtrl, wxDataViewCtrl);

class NB_WxDataViewListCtrl : TNitClass<wxDataViewListCtrl>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(selectedRow),
			PROP_ENTRY_R(store),
			PROP_ENTRY_R(itemCount),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: wx.Window=null, id=wx.ID.ANY, pos=wx.DEFAULT.POS, size=wx.DEFAULT.SIZE, style=STYLE.ROW_LINES, validator=wx.DEFAULT.VALIDATOR)"),

			FUNC_ENTRY_H(itemToRow,		"(item: int): int"),
			FUNC_ENTRY_H(rowToItem,		"(row: int): int)"),
			FUNC_ENTRY_H(selectRow,		"(row: int)"),
			FUNC_ENTRY_H(unselectRow,	"(row: int)"),
			FUNC_ENTRY_H(isRowSelected,	"(row: int): bool"),

			FUNC_ENTRY_H(appendColumn,	"(column: wx.DataViewColumn, variantType: string): bool"),
			FUNC_ENTRY_H(prependColumn,	"(column: wx.DataViewColumn, variantType: string): bool"),
			FUNC_ENTRY_H(insertColumn,	"(pos: int, column: wx.DataViewColumn, variantType: string): bool"),

			FUNC_ENTRY_H(appendItem,	"(values: [], tag: int=0)"),
			FUNC_ENTRY_H(prependItem,	"(values: [], tag: int=0)"),
			FUNC_ENTRY_H(insertItem,	"(row: int, values: [], tag: int=0)"),
			FUNC_ENTRY_H(deleteItem,	"(row: int)"),
			FUNC_ENTRY_H(deleteAllItems, "()"),

			FUNC_ENTRY_H(setValue,		"(row, col: int, value)"),
			FUNC_ENTRY_H(getValue,		"(row, col: int): value"),

			FUNC_ENTRY_H(setTextValue,	"(row, col: int, text: string)"),
			FUNC_ENTRY_H(getTextValue,	"(row, col: int): string"),

			FUNC_ENTRY_H(setToggleValue,"(row, col: int, value: bool)"),
			FUNC_ENTRY_H(getToggleValue,"(row, col: int): bool"),

			FUNC_ENTRY_H(setItemTag,	"(item: int, tag: int)"),
			FUNC_ENTRY_H(getItemTag,	"(item: int): int"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(selectedRow)			{ return push(v, self(v)->GetSelectedRow()); }
	NB_PROP_GET(store)					{ return push(v, self(v)->GetStore()); }
	NB_PROP_GET(itemCount)				{ return push(v, self(v)->GetItemCount()); }

	NB_CONS()
	{ 
		setSelf(v, new wxDataViewListCtrl(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			*opt<wxPoint>(v, 4, wxDefaultPosition),
			*opt<wxSize>(v, 5, wxDefaultSize),
			optInt(v, 6, wxDV_ROW_LINES),
			*opt<wxValidator>(v, 7, (wxValidator*)&wxDefaultValidator)
			));

		return SQ_OK;
	}

	NB_FUNC(itemToRow)					{ return push(v, self(v)->ItemToRow(GetWxDataViewItem(v, 2))); }
	NB_FUNC(rowToItem)					{ return PushWxDataViewItem(v, self(v)->RowToItem(getInt(v, 2))); }
	NB_FUNC(selectRow)					{ self(v)->SelectRow(getInt(v, 2)); return 0; }
	NB_FUNC(unselectRow)				{ self(v)->UnselectRow(getInt(v, 2)); return 0; }
	NB_FUNC(isRowSelected)				{ return push(v, self(v)->IsRowSelected(getInt(v, 2))); }

	NB_FUNC(appendColumn)				{ self(v)->AppendColumn(get<wxDataViewColumn>(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(prependColumn)				{ self(v)->PrependColumn(get<wxDataViewColumn>(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(insertColumn)				{ self(v)->InsertColumn(getInt(v, 2), get<wxDataViewColumn>(v, 3), getWxString(v, 4)); return 0; }

	NB_FUNC(appendItem)				
	{
		int tag = optInt(v, 3, 0);
		wxVector<wxVariant> values;
		GetWxVariantVector(v, 2, values);
		self(v)->AppendItem(values, wxUIntPtr(tag)); 
		return 0; 
	}

	NB_FUNC(prependItem)
	{
		int tag = optInt(v, 3, 0);
		wxVector<wxVariant> values;
		GetWxVariantVector(v, 2, values);
		self(v)->PrependItem(values, wxUIntPtr(tag)); 
		return 0; 
	}

	NB_FUNC(insertItem)
	{
		int row = getInt(v, 2);
		int tag = optInt(v, 4, 0);
		wxVector<wxVariant> values;
		GetWxVariantVector(v, 3, values);
		self(v)->InsertItem(row, values, wxUIntPtr(tag)); 
		return 0; 
	}

	NB_FUNC(deleteItem)					{ self(v)->DeleteItem(getInt(v, 2)); return 0; }
	NB_FUNC(deleteAllItems)				{ self(v)->DeleteAllItems(); return 0; }
	NB_FUNC(setItemTag)					{ self(v)->SetItemData(GetWxDataViewItem(v, 2), wxUIntPtr(getInt(v, 3))); return 0; }
	NB_FUNC(getItemTag)					{ return push(v, (int)self(v)->GetItemData(GetWxDataViewItem(v, 2))); }

	NB_FUNC(getValue)					{ wxVariant var; self(v)->GetValue(var, getInt(v, 2), getInt(v, 3)); return PushWxVariant(v, var); }
	NB_FUNC(setValue)					{ self(v)->SetValue(GetWxVariant(v, 4), getInt(v, 2), getInt(v, 3)); return 0; }

	NB_FUNC(getTextValue)				{ return push(v, self(v)->GetTextValue(getInt(v, 2), getInt(v, 3))); }
	NB_FUNC(setTextValue)				{ self(v)->SetTextValue(getWxString(v, 4), getInt(v, 2), getInt(v, 3)); return 0; }

	NB_FUNC(getToggleValue)				{ return push(v, self(v)->GetToggleValue(getInt(v, 2), getInt(v, 3))); }
	NB_FUNC(setToggleValue)				{ self(v)->SetToggleValue(getBool(v, 4), getInt(v, 2), getInt(v, 3)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, DataViewTreeCtrl, wxDataViewCtrl);

class NB_WxDataViewTreeCtrl : TNitClass<wxDataViewTreeCtrl>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(imageList),
			PROP_ENTRY_R(store),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(						"(parent: wx.Window=null, id=wx.ID.ANY, pos=wx.DEFAULT.POS, size=wx.DEFAULT.SIZE, style=STYLE.NO_HEADER|STYLE.ROW_LINES, validator=wx.DEFAULT.VALIDATOR)"),

			FUNC_ENTRY_H(appendContainer,		"(parent: int, text: string, icon: wx.Icon=null, expanded: wx.Icon=null, data: object=null): int"),
			FUNC_ENTRY_H(appendItem,			"(parent: int, text: string, icon: wx.Icon=null, data: object=null): int"),
			FUNC_ENTRY_H(deleteAllItems,		"()"),
			FUNC_ENTRY_H(deleteChildren,		"(item: int)"),
			FUNC_ENTRY_H(deleteItem,			"(item: int)"),
			FUNC_ENTRY_H(getChildCount,			"(parent: int)"),
			FUNC_ENTRY_H(getItemData,			"(item: int): object"),
			FUNC_ENTRY_H(getItemExpandedIcon,	"(item: int): wx.Icon"),
			FUNC_ENTRY_H(getItemIcon,			"(item: int): wx.Icon"),
			FUNC_ENTRY_H(getItemText,			"(item: int): string"),
			FUNC_ENTRY_H(getNthChild,			"(parent: int, pos: int): int"),
			FUNC_ENTRY_H(insertContainer,		"(parent: int, previous: int, text: string, icon: wx.Icon=null, expanded: wx.Icon=null, data: object=null): int"),
			FUNC_ENTRY_H(insertItem,			"(parent: int, previous: int, text: string, icon: wx.Icon=null, data: object=null): int"),
			FUNC_ENTRY_H(prependContainer,		"(parent: int, text: string, icon: wx.Icon=null, expanded: wx.Icon=null, data: object=null): int"),
			FUNC_ENTRY_H(prependItem,			"(parent: int, text: string, icon: wx.Icon=null, data: object=null): int"),
			FUNC_ENTRY_H(setItemData,			"(item: int, data: object)"),
			FUNC_ENTRY_H(setItemExpandedIcon,	"(item: int, icon: wx.Icon)"),
			FUNC_ENTRY_H(setItemIcon,			"(item: int, icon: wx.Icon)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(imageList)				{ return push(v, self(v)->GetImageList()); }
	NB_PROP_GET(store)					{ return push(v, self(v)->GetStore()); }
	
	NB_PROP_SET(imageList)				{ self(v)->SetImageList(get<wxImageList>(v, 2)); return 0; }

	NB_CONS()
	{ 
		setSelf(v, new wxDataViewTreeCtrl(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			*opt<wxPoint>(v, 4, wxDefaultPosition),
			*opt<wxSize>(v, 5, wxDefaultSize),
			optInt(v, 6, wxDV_NO_HEADER | wxDV_ROW_LINES),
			*opt<wxValidator>(v, 7, (wxValidator*)&wxDefaultValidator)
			));

		return SQ_OK;
	}

	NB_FUNC(appendContainer)			{ self(v)->AppendContainer(GetWxDataViewItem(v, 2), getWxString(v, 3), optInt(v, 4, -1), optInt(v, 5, -1), GetWxScriptData(v, 6, NULL)); return 0; }
	NB_FUNC(appendItem)					{ self(v)->AppendItem(GetWxDataViewItem(v, 2), getWxString(v, 3), optInt(v, 4, -1), GetWxScriptData(v, 5, NULL)); return 0; }
	NB_FUNC(prependContainer)			{ self(v)->PrependContainer(GetWxDataViewItem(v, 2), getWxString(v, 3), optInt(v, 4, -1), optInt(v, 5, -1), GetWxScriptData(v, 6, NULL)); return 0; }
	NB_FUNC(prependItem)				{ self(v)->PrependItem(GetWxDataViewItem(v, 2), getWxString(v, 3), optInt(v, 4, -1), GetWxScriptData(v, 5, NULL)); return 0; }
	NB_FUNC(insertContainer)			{ self(v)->InsertContainer(GetWxDataViewItem(v, 2), GetWxDataViewItem(v, 3), getWxString(v, 4), optInt(v, 5, -1), optInt(v, 6, -1), GetWxScriptData(v, 7, NULL)); return 0; }
	NB_FUNC(insertItem)					{ self(v)->InsertItem(GetWxDataViewItem(v, 2), GetWxDataViewItem(v, 3), getWxString(v, 4), optInt(v, 5, -1), GetWxScriptData(v, 6, NULL)); return 0; }

	NB_FUNC(deleteAllItems)				{ self(v)->DeleteAllItems(); return 0; }
	NB_FUNC(deleteChildren)				{ self(v)->DeleteChildren(GetWxDataViewItem(v, 2)); return 0; }
	NB_FUNC(deleteItem)					{ self(v)->DeleteItem(GetWxDataViewItem(v, 2)); return 0; }
	NB_FUNC(getChildCount)				{ return push(v, self(v)->GetChildCount(GetWxDataViewItem(v, 2))); }
	NB_FUNC(getItemData)				{ return PushWxScriptData(v, self(v)->GetItemData(GetWxDataViewItem(v, 2))); }
	NB_FUNC(getItemExpandedIcon)		{ return push(v, self(v)->GetItemExpandedIcon(GetWxDataViewItem(v, 2))); }
	NB_FUNC(getItemIcon)				{ return push(v, self(v)->GetItemIcon(GetWxDataViewItem(v, 2))); }
	NB_FUNC(getItemText)				{ return push(v, self(v)->GetItemText(GetWxDataViewItem(v, 2))); }
	NB_FUNC(getNthChild)				{ return PushWxDataViewItem(v, self(v)->GetNthChild(GetWxDataViewItem(v, 2), getInt(v, 3))); }
	NB_FUNC(setItemData)				{ self(v)->SetItemData(GetWxDataViewItem(v, 2), GetWxScriptData(v, 3, NULL)); return 0; }
	NB_FUNC(setItemExpandedIcon)		{ self(v)->SetItemExpandedIcon(GetWxDataViewItem(v, 2), *get<wxIcon>(v, 3)); return 0; }
	NB_FUNC(setItemIcon)				{ self(v)->SetItemIcon(GetWxDataViewItem(v, 2), *get<wxIcon>(v, 3)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

SQRESULT NitLibWxDataView(HSQUIRRELVM v)
{
	NB_WxDataViewEvent::Register(v);

	NB_WxDataViewItemAttr::Register(v);

	NB_WxDataViewModelNotifier::Register(v);

	NB_WxDataViewModel::Register(v);
	NB_WxDataViewListModel::Register(v);
	NB_WxDataViewIndexListModel::Register(v);
//	NB_WxDataViewVirtualListModel::Register(v);
	NB_WxDataViewListStore::Register(v);
	NB_WxDataViewTreeStore::Register(v);

	NB_WxHeaderColumn::Register(v);
	NB_WxSettableHeaderColumn::Register(v);
	NB_WxDataViewColumn::Register(v);

	NB_WxDataViewIconText::Register(v);

	NB_WxDataViewRenderer::Register(v);
	NB_WxDataViewBitmapRenderer::Register(v);
	NB_WxDataViewChoiceRenderer::Register(v);
	NB_WxDataViewDateRenderer::Register(v);
	NB_WxDataViewIconTextRenderer::Register(v);
	NB_WxDataViewProgressRenderer::Register(v);
	NB_WxDataViewTextRenderer::Register(v);
	NB_WxDataViewToggleRenderer::Register(v);
	NB_WxDataViewCustomRenderer::Register(v);
	NB_WxDataViewSpinRenderer::Register(v);

	NB_WxDataViewCtrl::Register(v);
	NB_WxDataViewListCtrl::Register(v);
	NB_WxDataViewTreeCtrl::Register(v);

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
