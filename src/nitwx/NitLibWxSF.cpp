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

#include <wx/wxsf/wxShapeFramework.h>

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

#define NB_TYPE_WXSF_OBJ(API, TClass, TBaseClass)	NB_TYPE_WXOBJ_EXT(API, sf, wxSF, TClass, TBaseClass)

static SQInteger SFError(HSQUIRRELVM v, const char* msg, wxSF::ERRCODE err)
{
	const char* errmsg = "???";
	switch (err)
	{
	case wxSF::errOK:					errmsg = "ok???";
	case wxSF::errNOT_CREATED:			errmsg = "not created";
	case wxSF::errNOT_ACCEPTED:			errmsg = "not accepted";
	case wxSF::errINVALID_INPUT:		errmsg = "invalid input";
	}

	return sq_throwfmt(v, "%s: %s", msg, errmsg);
}

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ_EXT(NITWX_API, xs, xs, Property, wxObject);

class NB_XsProperty : TNitClass<xsProperty>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(name),
			PROP_ENTRY_R(type),
			PROP_ENTRY_R(defaultValue),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(set,			"(value)"),
			FUNC_ENTRY	(_call),
			FUNC_ENTRY	(_tostring),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(name)					{ return push(v, self(v)->m_sFieldName); }
	NB_PROP_GET(type)					{ return push(v, self(v)->m_sDataType); }
	NB_PROP_GET(defaultValue)			{ return push(v, self(v)->m_sDefaultValueStr); }

	NB_FUNC(set)
	{ 
		if (isString(v, 2))
			self(v)->FromString(getWxString(v, 2)); 
		else
		{
			sq_tostring(v, 2);
			self(v)->FromString(getWxString(v, -1));
			sq_poptop(v);
		}
		return 0; 
	}

	NB_FUNC(_call)						{ return PushWxVariant(v, self(v)->ToVariant()); }
	NB_FUNC(_tostring)					{ return push(v, self(v)->ToString()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ_EXT(NITWX_API, xs, xs, Serializable, wxObject);

class NB_XsSerializable : TNitClass<xsSerializable>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(parent),
			PROP_ENTRY_R(parentManager),
			PROP_ENTRY_R(firstChild),
			PROP_ENTRY_R(lastChild),
			PROP_ENTRY_R(sibling),
			PROP_ENTRY_R(children),
			PROP_ENTRY_R(properties),
			PROP_ENTRY	(id),
			PROP_ENTRY	(serialized),
			PROP_ENTRY	(cloningEnabled),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(getFirstChild,		"(type: wx.ClassInfo): wx.xs.Serializable"),
			FUNC_ENTRY_H(getLastChild,		"(type: wx.ClassInfo): wx.xs.Serializable"),
			FUNC_ENTRY_H(getSibling,		"(type: wx.ClassInfo): wx.xs.Serializable"),
			FUNC_ENTRY_H(getChild,			"(id: int, recursive=false): wx.xs.Serializable"),
			FUNC_ENTRY_H(getChildren,		"(type: wx.ClassInfo): wx.xs.Serializable[]"),
			FUNC_ENTRY_H(getChildrenRecursively, "(type: wx.ClassInfo, mode=SEARCH.BFS): wx.xs.Serializable[]"),
			FUNC_ENTRY_H(hasChildren,		"()"),
			FUNC_ENTRY_H(addChild,			"(child: wx.xs.Serializable): wx.xs.Serializable"),
			FUNC_ENTRY_H(insertChild,		"(pos: int, child: wx.xs.Serializable): wx.xs.Serializable"),
			FUNC_ENTRY_H(removeChild,		"(child: wx.xs.Serializable)"),
			FUNC_ENTRY_H(removeChildren,	"()"),
			FUNC_ENTRY_H(getProperty,		"(field: string): wx.xs.Property"),
			FUNC_ENTRY_H(addProperty,		"(property: wx.xs.Property)"),
			FUNC_ENTRY_H(removeProperty,	"(property: wx.xs.Property)"),
			FUNC_ENTRY_H(enablePropertySerialization, "(field: string, enable=true)"),
			FUNC_ENTRY_H(isPropertySerialized, "(field: string): bool"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "SEARCH");
		newSlot(v, -1, "DFS",			(int)type::searchDFS);
		newSlot(v, -1, "BFS",			(int)type::searchBFS);
		sq_poptop(v);
	}

	NB_PROP_GET(parent)					{ return push(v, self(v)->GetParent()); }
	NB_PROP_GET(parentManager)			{ return push(v, self(v)->GetParentManager()); }
	NB_PROP_GET(firstChild)				{ return push(v, self(v)->GetFirstChild()); }
	NB_PROP_GET(lastChild)				{ return push(v, self(v)->GetLastChild()); }
	NB_PROP_GET(sibling)				{ return push(v, self(v)->GetSibbling()); }
	NB_PROP_GET(hasChildren)			{ return push(v, self(v)->HasChildren()); }
	NB_PROP_GET(id)						{ return push(v, self(v)->GetId()); }
	NB_PROP_GET(children)				{ SerializableList& l = self(v)->GetChildrenList(); return nitNativeItr(v, l.begin(), l.end()); }
	NB_PROP_GET(properties)				{ PropertyList& l = self(v)->GetProperties(); return nitNativeItr(v, l.begin(), l.end()); }
	NB_PROP_GET(serialized)				{ return push(v, self(v)->IsSerialized()); }
	NB_PROP_GET(cloningEnabled)			{ return push(v, self(v)->IsCloningEnabled()); }

	NB_PROP_SET(id)						{ self(v)->SetId(getInt(v, 2)); return 0; }
	NB_PROP_SET(parent)					{ self(v)->Reparent(get<xsSerializable>(v, 2)); return 0; }
	NB_PROP_SET(serialized)				{ self(v)->EnableSerialization(getBool(v, 2)); return 0; }
	NB_PROP_SET(cloningEnabled)			{ self(v)->EnableCloning(getBool(v, 2)); return 0; }

	NB_FUNC(getFirstChild)				{ return push(v, self(v)->GetFirstChild(GetWxClassInfo(v, 2))); }
	NB_FUNC(getLastChild)				{ return push(v, self(v)->GetLastChild(GetWxClassInfo(v, 2))); }
	NB_FUNC(getSibling)					{ return push(v, self(v)->GetSibbling(GetWxClassInfo(v, 2))); }
	NB_FUNC(getChild)					{ return push(v, self(v)->GetChild(getInt(v, 2), optBool(v, 3, false))); }

	NB_FUNC(getChildren)
	{
		SerializableList list;
		self(v)->GetChildren(GetWxClassInfo(v, 2), list);
		sq_newarray(v, 0);
		for (SerializableList::iterator itr = list.begin(), end = list.end(); itr != end; ++itr)
			arrayAppend(v, -1, *itr);
		return 1;
	}

	NB_FUNC(getChildrenRecursively)
	{
		SerializableList list;
		self(v)->GetChildrenRecursively(GetWxClassInfo(v, 2), list, (type::SEARCHMODE)optInt(v, 3, type::searchBFS));
		sq_newarray(v, 0);
		for (SerializableList::iterator itr = list.begin(), end = list.end(); itr != end; ++itr)
			arrayAppend(v, -1, *itr);
		return 1;
	}

	NB_FUNC(hasChildren)				{ return push(v, self(v)->HasChildren()); }

	NB_FUNC(addChild)					{ return push(v, self(v)->AddChild(get<xsSerializable>(v, 2))); }
	NB_FUNC(insertChild)				{ return push(v, self(v)->InsertChild(getInt(v, 2), get<xsSerializable>(v, 3))); }
	NB_FUNC(removeChild)				{ self(v)->RemoveChild(get<xsSerializable>(v, 2)); return 0; }
	NB_FUNC(removeChildren)				{ self(v)->RemoveChildren(); return 0; }

	NB_FUNC(getProperty)				{ return push(v, self(v)->GetProperty(getWxString(v, 2))); }
	NB_FUNC(addProperty)				{ self(v)->AddProperty(get<xsProperty>(v, 2)); return 0; }
	NB_FUNC(removeProperty)				{ self(v)->RemoveProperty(get<xsProperty>(v, 2)); return 0; }

	NB_FUNC(enablePropertySerialization) { self(v)->EnablePropertySerialization(getWxString(v, 2), optBool(v, 3, true)); return 0; }
	NB_FUNC(isPropertySerialized)		{ return push(v, self(v)->IsPropertySerialized(getWxString(v, 2))); } 
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, XmlSerializer, wxObject);

class NB_WxXmlSerializer : TNitClass<wxXmlSerializer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(serializerOwner),
			PROP_ENTRY	(serializerRootName),
			PROP_ENTRY	(serializerVersion),
			PROP_ENTRY_R(libraryVersion),
			PROP_ENTRY_R(errMessage),
			PROP_ENTRY	(rootItem),
			PROP_ENTRY_R(cloned),
			PROP_ENTRY_R(usedIds),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(getItem,		"(id: int): wx.xs.Serializable"),
			FUNC_ENTRY_H(getItems,		"(type: wx.ClassInfo, mode=wx.xs.Serializable.SEARCH.BFS): wx.xs.Serializable[]"),
			FUNC_ENTRY_H(contains,		"(item: wx.xs.Serializable): bool"
			"\n"						"(type: wx.ClassInfo): bool"),
			FUNC_ENTRY_H(copyItems,		"(from: wx.XmlSerializer)"),
			FUNC_ENTRY_H(addItem,		"(parentId: int, item: wx.xs.Serializable): wx.xs.Serializable"
			"\n"						"(parent: wx.xs.Serializable, item: wx.xs.Serializable): wx.xs.Serializable"),
			FUNC_ENTRY_H(removeItem,	"(id: int)"
			"\n"						"(item: wx.xs.Serializable)"),
			FUNC_ENTRY_H(removeAll,		"()"),
			FUNC_ENTRY_H(enableCloning,	"(enable=true)"),
			FUNC_ENTRY_H(saveXml,		"(out: StreamWriter, withRoot=false): bool"
			"\n"						"(filename: string, withRoot=false): bool"),
			FUNC_ENTRY_H(loadXml,		"(in: StreamReader): bool"
			"\n"						"(filename: string): bool"),
			FUNC_ENTRY_H(newId,			"(): int"),
			FUNC_ENTRY_H(isIdUsed,		"(id: int): bool"),
			FUNC_ENTRY_H(getIdCount,	"(id: int): int"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(serializerOwner)		{ return push(v, self(v)->GetSerializerOwner()); }
	NB_PROP_GET(serializerRootName)		{ return push(v, self(v)->GetSerializerRootName()); }
	NB_PROP_GET(serializerVersion)		{ return push(v, self(v)->GetSerializerVersion()); }
	NB_PROP_GET(libraryVersion)			{ return push(v, self(v)->GetLibraryVersion()); }
	NB_PROP_GET(errMessage)				{ return push(v, self(v)->GetSerializerRootName()); }
	NB_PROP_GET(rootItem)				{ return push(v, self(v)->GetRootItem()); }
	NB_PROP_GET(cloned)					{ return push(v, self(v)->IsCloned()); }
	NB_PROP_GET(usedIds)				{ IDMap& idm = self(v)->GetUsedIDs(); return nitNativePairItr(v, idm.begin(), idm.end()); }

	NB_PROP_SET(serializerOwner)		{ self(v)->SetSerializerOwner(getWxString(v, 2)); return 0; }
	NB_PROP_SET(serializerRootName)		{ self(v)->SetSerializerRootName(getWxString(v, 2)); return 0; }
	NB_PROP_SET(serializerVersion)		{ self(v)->SetSerializerVersion(getWxString(v, 2)); return 0; }
	NB_PROP_SET(rootItem)				{ self(v)->SetRootItem(get<xsSerializable>(v, 2)); return 0; }

	NB_FUNC(getItem)					{ return push(v, self(v)->GetItem(getInt(v, 2))); }

	NB_FUNC(getItems)
	{
		SerializableList list;
		self(v)->GetItems(GetWxClassInfo(v, 2), list, (xsSerializable::SEARCHMODE)optInt(v, 3, xsSerializable::searchBFS));
		sq_newarray(v, 0);
		for (SerializableList::iterator itr = list.begin(), end = list.end(); itr != end; ++itr)
			arrayAppend(v, -1, *itr);
		return 1;
	}

	NB_FUNC(contains)
	{
		if (is<xsSerializable>(v, 2))
			return push(v, self(v)->Contains(get<xsSerializable>(v, 2)));
		else
			return push(v, self(v)->Contains(GetWxClassInfo(v, 2)));
	}

	NB_FUNC(copyItems)					{ self(v)->CopyItems(*get<wxXmlSerializer>(v, 2)); return 0; }

	NB_FUNC(addItem)
	{
		if (isInt(v, 2))
			return push(v, self(v)->AddItem(getInt(v, 2), get<xsSerializable>(v, 3)));
		else
			return push(v, self(v)->AddItem(get<xsSerializable>(v, 2), get<xsSerializable>(v, 3)));
	}

	NB_FUNC(removeItem)
	{
		if (isInt(v, 2))
			self(v)->RemoveItem(getInt(v, 2));
		else
			self(v)->RemoveItem(get<xsSerializable>(v, 2));
		return 0;
	}

	NB_FUNC(removeAll)					{ self(v)->RemoveAll(); return 0; }
	NB_FUNC(enableCloning)				{ self(v)->EnableCloning(optBool(v, 2, true)); return 0; }

	NB_FUNC(saveXml)
	{
		if (is<StreamWriter>(v, 2))
		{
			wxNitOutputStream out(get<StreamWriter>(v, 2));
			return push(v, self(v)->SerializeToXml(out, optBool(v, 3, false)));
		}
		return push(v, self(v)->SerializeToXml(getWxString(v, 2), optBool(v, 3, false)));
	}

	NB_FUNC(loadXml)
	{
		if (is<StreamReader>(v, 2))
		{
			wxNitInputStream in(get<StreamReader>(v, 2));
			return push(v, self(v)->DeserializeFromXml(in));
		}
		return push(v, self(v)->DeserializeFromXml(getWxString(v, 2)));
	}

	NB_FUNC(newId)						{ return push(v, self(v)->GetNewId()); }
	NB_FUNC(isIdUsed)					{ return push(v, self(v)->IsIdUsed(getInt(v, 2))); }
	NB_FUNC(getIdCount)					{ return push(v, self(v)->GetIDCount(getInt(v, 2))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, ShapeBase, xsSerializable);

class NB_WxSFShapeBase : TNitClass<wxSFShapeBase>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(absolutePosition),
			PROP_ENTRY	(position),
			PROP_ENTRY_R(center),
			PROP_ENTRY	(style),
			PROP_ENTRY	(valign),
			PROP_ENTRY	(halign),
			PROP_ENTRY	(vborder),
			PROP_ENTRY	(hborder),
			PROP_ENTRY	(selected),
			PROP_ENTRY	(customDockPoint),
			PROP_ENTRY_R(parent),
			PROP_ENTRY_R(grandParent),
			PROP_ENTRY_R(canvas),
			PROP_ENTRY_R(diagram),
			PROP_ENTRY_R(visible),
			PROP_ENTRY	(hoverColor),
			PROP_ENTRY	(active),
			PROP_ENTRY_R(acceptedChildren),
			PROP_ENTRY_R(acceptedConnections),
			PROP_ENTRY_R(acceptedSrcNeighbors),
			PROP_ENTRY_R(acceptedTrgNeighbors),
			PROP_ENTRY_R(handles),
			PROP_ENTRY_R(connectionPoints),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(refresh,					"(delayed=false)"),
			FUNC_ENTRY_H(draw,						"(dc: wx.DC, withChildren=true)"),
			FUNC_ENTRY_H(contains,					"(pos: wx.Point): bool"),
			FUNC_ENTRY_H(isInside,					"(rect: wx.Rect): bool"),
			FUNC_ENTRY_H(intersects,				"(rect: wx.Rect): bool"),
			FUNC_ENTRY_H(getBorderPoint,			"(start: Vector2, end: Vector2): Vector2)"),
			FUNC_ENTRY_H(showHandles,				"(show: bool)"),
			FUNC_ENTRY_H(addStyle,					"(style: STYLE)"),
			FUNC_ENTRY_H(removeStyle,				"(style: STYLE)"),
			FUNC_ENTRY_H(hasStyle,					"(style: STYLE): bool"),
			FUNC_ENTRY_H(getChildShapes,			"(type: wx.ClassInfo=NULL, recursive=false, mode=SEARCH.BFS): wx.sf.ShapeBase[]"),
			FUNC_ENTRY_H(getNeighbors,				"(type: wx.ClassInfo=NULL, connDir: CONNECT, direct=true): wx.sf.ShapeBase[]"),
			FUNC_ENTRY_H(getAssignedConnections,	"(type: wx.ClassInfo=NULL, mode: CONNECT): wx.sf.ShapeBase[]"),
			FUNC_ENTRY_H(getBoundingBox,			"(): wx.Rect"),
			FUNC_ENTRY_H(getCompleteBoundingBox,	"(mask=BB.ALL): wx.Rect"),
			FUNC_ENTRY_H(scale,						"(x, y: float, withChildren=true)"),
			FUNC_ENTRY_H(scaleChildren,				"(x, y: float)"),
			FUNC_ENTRY_H(moveTo,					"(x, y: float)"
			"\n"									"(pos: Vector2)"),
			FUNC_ENTRY_H(moveBy,					"(x, y: float)"
			"\n"									"(delta: Vector2)"),
			FUNC_ENTRY_H(doAlignment,				"()"),
			FUNC_ENTRY_H(update,					"()"),
			FUNC_ENTRY_H(fitToChildren,				"()"),
			FUNC_ENTRY_H(isAncestor,				"(child: wx.sf.ShapeBase): bool"),
			FUNC_ENTRY_H(isDescendant,				"(parent: wx.sf.ShapeBase): bool"),
			FUNC_ENTRY_H(isChildAccepted,			"(type: string): bool"),
			FUNC_ENTRY_H(acceptCurrentlyDraggedShapes, "(): bool"),
			FUNC_ENTRY_H(acceptChild,				"(type: string) // can use 'all' for type"),
			FUNC_ENTRY_H(isConnectionAccepted,		"(type: string): bool"),
			FUNC_ENTRY_H(acceptConnection,			"(type: string) // can use 'all' for type"),
			FUNC_ENTRY_H(isSrcNeighborAccepted,		"(type: string): bool"),
			FUNC_ENTRY_H(acceptSrcNeighbor,			"(type: string) // can use 'all' for type"),
			FUNC_ENTRY_H(isTrgNeighborAccepted,		"(type: string): bool"),
			FUNC_ENTRY_H(acceptTrgNeighbor,			"(type: string) // can use 'all' for type"),
			FUNC_ENTRY_H(getHandle,					"(type: wx.sf.ShapeHandle.TYPE, id=-1): wx.sf.ShapeHandle"),
			FUNC_ENTRY_H(addHandle,					"(type: wx.sf.ShapeHandle.TYPE, id=-1)"),
			FUNC_ENTRY_H(removeHandle,				"(type: wx.sf.ShapeHandle.TYPE, id=-1)"),
			FUNC_ENTRY_H(getConnectionPoint,		"(type: wx.sf.ConnectionPoint.TYPE, id=-1): wx.sf.ConnectionPoint"),
			FUNC_ENTRY_H(getNearestConnectionPoint,	"(pos: Vector2): wx.sf.ConnectionPoint"),
			FUNC_ENTRY_H(addConnectionPoint,		"(type: wx.sf.ConnectionPoint.TYPE, persistent=true)"
			"\n"									"(cp: wx.sf.ConnectionPoint, persistent=true)"
			"\n"									"(pos: Vector2, id=-1, persistent=true)"),
			FUNC_ENTRY_H(removeConnectionPoint,		"(type: wx.sf.ConnectionPoint.TYPE)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "BB");
		newSlot(v, -1, "SELF",			(int)type::bbSELF);
		newSlot(v, -1, "CHILDREN",		(int)type::bbCHILDREN);
		newSlot(v, -1, "CONNECTIONS",	(int)type::bbCONNECTIONS);
		newSlot(v, -1, "SHADOW",		(int)type::bbSHADOW);
		newSlot(v, -1, "ALL",			(int)type::bbALL);
		sq_poptop(v);

		addStaticTable(v, "CONNECT");
		newSlot(v, -1, "STARTING",		(int)type::lineSTARTING);
		newSlot(v, -1, "ENDING",		(int)type::lineENDING);
		newSlot(v, -1, "BOTH",			(int)type::lineBOTH);
		sq_poptop(v);

		addStaticTable(v, "VALIGN");
		newSlot(v, -1, "NONE",			(int)type::valignNONE);
		newSlot(v, -1, "TOP",			(int)type::valignTOP);
		newSlot(v, -1, "MIDDLE",		(int)type::valignMIDDLE);
		newSlot(v, -1, "BOTTOM",		(int)type::valignBOTTOM);
		newSlot(v, -1, "EXPAND",		(int)type::valignEXPAND);
		newSlot(v, -1, "LINE_START",	(int)type::valignLINE_START);
		newSlot(v, -1, "LINE_END",		(int)type::valignLINE_END);
		sq_poptop(v);

		addStaticTable(v, "HALIGN");
		newSlot(v, -1, "NONE",			(int)type::halignNONE);
		newSlot(v, -1, "LEFT",			(int)type::halignLEFT);
		newSlot(v, -1, "CENTER",		(int)type::halignCENTER);
		newSlot(v, -1, "RIGHT",			(int)type::halignRIGHT);
		newSlot(v, -1, "EXPAND",		(int)type::halignEXPAND);
		newSlot(v, -1, "LINE_START",	(int)type::halignLINE_START);
		newSlot(v, -1, "LINE_END",		(int)type::halignLINE_END);
		sq_poptop(v);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "PARENT_CHANGE",			(int)type::sfsPARENT_CHANGE);
		newSlot(v, -1, "POSITION_CHANGE",		(int)type::sfsPOSITION_CHANGE);
		newSlot(v, -1, "SIZE_CHANGE",			(int)type::sfsSIZE_CHANGE);
		newSlot(v, -1, "HOVERING",				(int)type::sfsHOVERING);
		newSlot(v, -1, "HIGHLIGHTING",			(int)type::sfsHIGHLIGHTING);
		newSlot(v, -1, "ALWAYS_INSIDE",			(int)type::sfsALWAYS_INSIDE);
		newSlot(v, -1, "DELETE_USER_DATA",		(int)type::sfsDELETE_USER_DATA);
		newSlot(v, -1, "PROCESS_DEL",			(int)type::sfsPROCESS_DEL);
		newSlot(v, -1, "SHOW_HANDLES",			(int)type::sfsSHOW_HANDLES);
		newSlot(v, -1, "SHOW_SHADOW",			(int)type::sfsSHOW_SHADOW);
		newSlot(v, -1, "LOCK_CHILDREN",			(int)type::sfsLOCK_CHILDREN);
		newSlot(v, -1, "EMIT_EVENTS",			(int)type::sfsEMIT_EVENTS);
		newSlot(v, -1, "PROPAGATE_DRAGGING",	(int)type::sfsPROPAGATE_DRAGGING);
		newSlot(v, -1, "PROPAGATE_SELECTION",	(int)type::sfsPROPAGATE_SELECTION);
		newSlot(v, -1, "DEFAULT",				(int)type::sfsDEFAULT_SHAPE_STYLE);
		sq_poptop(v);
	}

	NB_PROP_GET(absolutePosition)		{ return PushWxRealPoint(v, self(v)->GetAbsolutePosition()); }
	NB_PROP_GET(position)				{ return PushWxRealPoint(v, self(v)->GetRelativePosition()); }
	NB_PROP_GET(center)					{ return PushWxRealPoint(v, self(v)->GetCenter()); }
	NB_PROP_GET(style)					{ return push(v, self(v)->GetStyle()); }
	NB_PROP_GET(valign)					{ return push(v, (int)self(v)->GetVAlign()); }
	NB_PROP_GET(halign)					{ return push(v, (int)self(v)->GetHAlign()); }
	NB_PROP_GET(vborder)				{ return push(v, (float)self(v)->GetVBorder()); }
	NB_PROP_GET(hborder)				{ return push(v, (float)self(v)->GetHBorder()); }
	NB_PROP_GET(selected)				{ return push(v, self(v)->IsSelected()); }
	NB_PROP_GET(customDockPoint)		{ return push(v, self(v)->GetCustomDockPoint()); }
	NB_PROP_GET(parent)					{ return push(v, self(v)->GetParentShape()); }
	NB_PROP_GET(grandParent)			{ return push(v, self(v)->GetGrandParentShape()); }
	NB_PROP_GET(canvas)					{ return push(v, self(v)->GetParentCanvas()); }
	NB_PROP_GET(diagram)				{ return push(v, self(v)->GetShapeManager()); }
	NB_PROP_GET(visible)				{ return push(v, self(v)->IsVisible()); }
	NB_PROP_GET(hoverColor)				{ return PushWxColor(v, self(v)->GetHoverColour()); }
	NB_PROP_GET(active)					{ return push(v, self(v)->IsActive()); }

	NB_PROP_GET(acceptedChildren)		{ wxArrayString& arr = self(v)->GetAcceptedChildren(); return nitNativeItr(v, arr.begin(), arr.end()); }
	NB_PROP_GET(acceptedConnections)	{ wxArrayString& arr = self(v)->GetAcceptedConnections(); return nitNativeItr(v, arr.begin(), arr.end()); }
	NB_PROP_GET(acceptedSrcNeighbors)	{ wxArrayString& arr = self(v)->GetAcceptedSrcNeighbours(); return nitNativeItr(v, arr.begin(), arr.end()); }
	NB_PROP_GET(acceptedTrgNeighbors)	{ wxArrayString& arr = self(v)->GetAcceptedTrgNeighbours(); return nitNativeItr(v, arr.begin(), arr.end()); }
	NB_PROP_GET(handles)				{ HandleList& handles = self(v)->GetHandles(); return nitNativeItr(v, handles.begin(), handles.end()); }
	NB_PROP_GET(connectionPoints)		{ ConnectionPointList& conns = self(v)->GetConnectionPoints(); return nitNativeItr(v, conns.begin(), conns.end()); }

	NB_PROP_SET(position)
	{
		if (isNumber(v, 2))				
			self(v)->SetRelativePosition(getFloat(v, 2), getFloat(v, 3));
		else
			self(v)->SetRelativePosition(GetWxRealPoint(v, 2));
		return 0;
	}

	NB_PROP_SET(style)					{ self(v)->SetStyle(getInt(v, 2)); return 0; }
	NB_PROP_SET(valign)					{ self(v)->SetVAlign((type::VALIGN)getInt(v, 2)); return 0; }
	NB_PROP_SET(halign)					{ self(v)->SetHAlign((type::HALIGN)getInt(v, 2)); return 0; }
	NB_PROP_SET(vborder)				{ self(v)->SetVBorder(getFloat(v, 2)); return 0; }
	NB_PROP_SET(hborder)				{ self(v)->SetHBorder(getFloat(v, 2)); return 0; }
	NB_PROP_SET(selected)				{ self(v)->Select(getBool(v, 2)); return 0; }
	NB_PROP_SET(customDockPoint)		{ self(v)->SetCustomDockPoint(getInt(v, 2)); return 0; }
	NB_PROP_SET(hoverColor)				{ self(v)->SetHoverColour(GetWxColor(v, 2)); return 0; }
	NB_PROP_SET(active)					{ self(v)->Activate(getBool(v, 2)); return 0; }

	NB_FUNC(refresh)					{ self(v)->Refresh(optBool(v, 2, false)); return 0; }
	NB_FUNC(draw)						{ self(v)->Draw(*get<wxDC>(v, 2), optBool(v, 3, true)); return 0; }
	NB_FUNC(contains)					{ return push(v, self(v)->Contains(*get<wxPoint>(v, 2))); }
	NB_FUNC(isInside)					{ return push(v, self(v)->IsInside(*get<wxRect>(v, 2))); }
	NB_FUNC(intersects)					{ return push(v, self(v)->Intersects(*get<wxRect>(v, 2))); }
	NB_FUNC(getBorderPoint)				{ return PushWxRealPoint(v, self(v)->GetBorderPoint(GetWxRealPoint(v, 2), GetWxRealPoint(v, 3))); }
	NB_FUNC(showHandles)				{ self(v)->ShowHandles(getBool(v, 2)); return 0; }
	NB_FUNC(addStyle)					{ self(v)->AddStyle((type::STYLE)getInt(v, 2)); return 0; }
	NB_FUNC(removeStyle)				{ self(v)->RemoveStyle((type::STYLE)getInt(v, 2)); return 0; }
	NB_FUNC(hasStyle)					{ return push(v, self(v)->ContainsStyle((type::STYLE)getInt(v, 2))); }
	NB_FUNC(getBoundingBox)				{ return push(v, self(v)->GetBoundingBox()); }
	NB_FUNC(getCompleteBoundingBox)		{ wxRect rect; self(v)->GetCompleteBoundingBox(rect, getInt(v, 2)); return push(v, rect); }
	NB_FUNC(scale)						{ self(v)->Scale(getFloat(v, 2), getFloat(v, 3), optBool(v, 4, true)); return 0; }
	NB_FUNC(scaleChildren)				{ self(v)->ScaleChildren(getFloat(v, 2), getFloat(v, 3)); return 0; }
	NB_FUNC(moveTo)						{ if (isNumber(v, 2)) self(v)->MoveTo(getFloat(v, 2), getFloat(v, 3)); else self(v)->MoveTo(GetWxRealPoint(v, 2)); return 0; }
	NB_FUNC(moveBy)						{ if (isNumber(v, 2)) self(v)->MoveBy(getFloat(v, 2), getFloat(v, 3)); else self(v)->MoveBy(GetWxRealPoint(v, 2)); return 0; }
	NB_FUNC(doAlignment)				{ self(v)->DoAlignment(); return 0; }
	NB_FUNC(update)						{ self(v)->Update(); return 0; }
	NB_FUNC(fitToChildren)				{ self(v)->FitToChildren(); return 0; }

	NB_FUNC(getChildShapes)
	{
		ShapeList children;
		self(v)->GetChildShapes(OptWxClassInfo(v, 2, NULL), children, optBool(v, 3, false), (type::SEARCHMODE)optInt(v, 4, type::searchBFS));
		sq_newarray(v, 0);
		for (ShapeList::iterator itr = children.begin(), end = children.end(); itr != end; ++itr)
			arrayAppend(v, -1, *itr);
		return 1;
	}

	NB_FUNC(getNeighbors)
	{
		ShapeList neighbors;
		self(v)->GetNeighbours(neighbors, OptWxClassInfo(v, 2, NULL), (type::CONNECTMODE)getInt(v, 3), optBool(v, 4, true));
		sq_newarray(v, 0);
		for (ShapeList::iterator itr = neighbors.begin(), end = neighbors.end(); itr != end; ++itr)
			arrayAppend(v, -1, *itr);
		return 1;
	}

	NB_FUNC(getAssignedConnections)
	{
		ShapeList lines;
		self(v)->GetAssignedConnections(OptWxClassInfo(v, 2, NULL), (type::CONNECTMODE)getInt(v, 3), lines);
		sq_newarray(v, 0);
		for (ShapeList::iterator itr = lines.begin(), end = lines.end(); itr != end; ++itr)
			arrayAppend(v, -1, *itr);
		return 1;
	}

	NB_FUNC(isAncestor)					{ return push(v, self(v)->IsAncestor(get<wxSFShapeBase>(v, 2))); }
	NB_FUNC(isDescendant)				{ return push(v, self(v)->IsDescendant(get<wxSFShapeBase>(v, 2))); }

	NB_FUNC(isChildAccepted)			{ return push(v, self(v)->IsChildAccepted(getWxString(v, 2))); }
	NB_FUNC(acceptCurrentlyDraggedShapes) { return push(v, self(v)->AcceptCurrentlyDraggedShapes()); }
	NB_FUNC(acceptChild)				{ self(v)->AcceptChild(getWxString(v, 2)); return 0; }
	NB_FUNC(isConnectionAccepted)		{ return push(v, self(v)->IsConnectionAccepted(getWxString(v, 2))); }
	NB_FUNC(acceptConnection)			{ self(v)->AcceptConnection(getWxString(v, 2)); return 0; }
	NB_FUNC(isSrcNeighborAccepted)		{ return push(v, self(v)->IsSrcNeighbourAccepted(getWxString(v, 2))); }
	NB_FUNC(acceptSrcNeighbor)			{ self(v)->AcceptSrcNeighbour(getWxString(v, 2)); return 0; }
	NB_FUNC(isTrgNeighborAccepted)		{ return push(v, self(v)->IsTrgNeighbourAccepted(getWxString(v, 2))); }
	NB_FUNC(acceptTrgNeighbor)			{ self(v)->AcceptTrgNeighbour(getWxString(v, 2)); return 0; }

	NB_FUNC(getHandle)					{ return push(v, self(v)->GetHandle((wxSFShapeHandle::HANDLETYPE)getInt(v, 2), optInt(v, 3, -1))); }
	NB_FUNC(addHandle)					{ self(v)->AddHandle((wxSFShapeHandle::HANDLETYPE)getInt(v, 2), optInt(v, 3, -1)); return 0; }
	NB_FUNC(removeHandle)				{ self(v)->RemoveHandle((wxSFShapeHandle::HANDLETYPE)getInt(v, 2), optInt(v, 3, -1)); return 0; }
	NB_FUNC(getConnectionPoint)			{ return push(v, self(v)->GetConnectionPoint((wxSFConnectionPoint::CPTYPE)getInt(v, 2), optInt(v, 3, -1))); }
	NB_FUNC(getNearestConnectionPoint)	{ return push(v, self(v)->GetNearestConnectionPoint(GetWxRealPoint(v, 2))); }
	NB_FUNC(removeConnectionPoint)		{ self(v)->RemoveConnectionPoint((wxSFConnectionPoint::CPTYPE)getInt(v, 2)); return 0; }

	NB_FUNC(addConnectionPoint)
	{
		if (is<wxSFConnectionPoint>(v, 2))
			self(v)->AddConnectionPoint(get<wxSFConnectionPoint>(v, 2), optBool(v, 3, true));
		else if (isInt(v, 2))
			self(v)->AddConnectionPoint((wxSFConnectionPoint::CPTYPE)getInt(v, 2), optBool(v, 3, true));
		else
			self(v)->AddConnectionPoint(GetWxRealPoint(v, 2), optInt(v, 3, -1), optBool(v, 4, true));
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, ShapeHandle, wxObject);

class NB_WxSFShapeHandle : TNitClass<wxSFShapeHandle>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(position),
			PROP_ENTRY_R(delta),
			PROP_ENTRY_R(totalDelta),
			PROP_ENTRY_R(shape),
			PROP_ENTRY	(type),
			PROP_ENTRY	(visible),
			PROP_ENTRY	(id),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"
			"\n"						"(shape: wx.sf.ShapeBase, type: TYPE, id=-1)"
			"\n"						"(copyFrom: wx.sf.ShapeHandle)"),

			FUNC_ENTRY_H(refresh,		"()"),
			FUNC_ENTRY_H(contains,		"(pos: wx.Point): bool"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "TYPE");
		newSlot(v, -1, "LEFTTOP",		(int)type::hndLEFTTOP);
		newSlot(v, -1, "TOP",			(int)type::hndTOP);
		newSlot(v, -1, "RIGHT",			(int)type::hndRIGHT);
		newSlot(v, -1, "RIGHTBOTTOM",	(int)type::hndRIGHTBOTTOM);
		newSlot(v, -1, "BOTTOM",		(int)type::hndBOTTOM);
		newSlot(v, -1, "LEFTBOTTOM",	(int)type::hndLEFTBOTTOM);
		newSlot(v, -1, "LEFT",			(int)type::hndLEFT);
		newSlot(v, -1, "LINECTRL",		(int)type::hndLINECTRL);
		newSlot(v, -1, "LINESTART",		(int)type::hndLINESTART);
		newSlot(v, -1, "LINEEND",		(int)type::hndLINEEND);
		newSlot(v, -1, "UNDEF",			(int)type::hndUNDEF);
		sq_poptop(v);
	}

	NB_PROP_GET(position)				{ return push(v, self(v)->GetPosition()); }
	NB_PROP_GET(delta)					{ return push(v, self(v)->GetDelta()); }
	NB_PROP_GET(totalDelta)				{ return push(v, self(v)->GetTotalDelta()); }
	NB_PROP_GET(shape)					{ return push(v, self(v)->GetParentShape()); }
	NB_PROP_GET(type)					{ return push(v, (int)self(v)->GetType()); }
	NB_PROP_GET(visible)				{ return push(v, self(v)->IsVisible()); }
	NB_PROP_GET(id)						{ return push(v, self(v)->GetId()); }

	NB_PROP_SET(type)					{ self(v)->SetType((type::HANDLETYPE)getInt(v, 2)); return 0; }
	NB_PROP_SET(visible)				{ self(v)->Show(getBool(v, 2)); return 0; }
	NB_PROP_SET(id)						{ self(v)->SetId(getInt(v, 2)); return 0; }

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new type());
		else if (is<type>(v, 2))
			setSelf(v, new type(*get<type>(v, 2)));
		else
			setSelf(v, new type(get<wxSFShapeBase>(v, 2), (type::HANDLETYPE)getInt(v, 3), optInt(v, 4, -1)));
		return SQ_OK;
	}

	NB_FUNC(refresh)					{ self(v)->Refresh(); return 0; }
	NB_FUNC(contains)					{ return push(v, self(v)->Contains(*get<wxPoint>(v, 2))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, ConnectionPoint, xsSerializable);

class NB_WxSFConnectionPoint: TNitClass<wxSFConnectionPoint>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(type),
			PROP_ENTRY	(shape),
			PROP_ENTRY	(position),
			PROP_ENTRY_R(absolutePosition),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"
			"\n"						"(shape: wx.sf.ShapeBase, type: TYPE)"
			"\n"						"(shape: wx.sf.ShapeBase, relPos: Vector2, id=-1)"),

			FUNC_ENTRY_H(draw,			"(dc: wx.DC)"),
			FUNC_ENTRY_H(refresh,		"()"),
			FUNC_ENTRY_H(contains,		"(pos: wx.Point): bool"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "TYPE");
		newSlot(v, -1, "UNDEF",			(int)type::cpUNDEF);
		newSlot(v, -1, "TOPLEFT",		(int)type::cpTOPLEFT);
		newSlot(v, -1, "TOPMIDDLE",		(int)type::cpTOPMIDDLE);
		newSlot(v, -1, "TOPRIGHT",		(int)type::cpTOPRIGHT);
		newSlot(v, -1, "CENTERLEFT",	(int)type::cpCENTERLEFT);
		newSlot(v, -1, "CENTERMIDDLE",	(int)type::cpCENTERMIDDLE);
		newSlot(v, -1, "CENTERRIGHT",	(int)type::cpCENTERRIGHT);
		newSlot(v, -1, "BOTTOMLEFT",	(int)type::cpBOTTOMLEFT);
		newSlot(v, -1, "BOTTOMMIDDLE",	(int)type::cpBOTTOMMIDDLE);
		newSlot(v, -1, "BOTTOMRIGHT",	(int)type::cpBOTTOMRIGHT);
		newSlot(v, -1, "CUSTOM",		(int)type::cpCUSTOM);
		sq_poptop(v);
	}

	NB_PROP_GET(type)					{ return push(v, (int)self(v)->GetType()); }
	NB_PROP_GET(shape)					{ return push(v, self(v)->GetParentShape()); }
	NB_PROP_GET(position)				{ return PushWxRealPoint(v, self(v)->GetRelativePosition()); }
	NB_PROP_GET(absolutePosition)		{ return PushWxRealPoint(v, self(v)->GetConnectionPoint()); }
	
	NB_PROP_SET(shape)					{ self(v)->SetParentShape(get<wxSFShapeBase>(v, 2)); return 0; }
	NB_PROP_SET(position)				{ self(v)->SetRelativePosition(GetWxRealPoint(v, 2)); return 0; }

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new type());
		else if (isInt(v, 3))
			setSelf(v, new type(get<wxSFShapeBase>(v, 2), (type::CPTYPE)getInt(v, 3)));
		else
			setSelf(v, new type(get<wxSFShapeBase>(v, 2), GetWxRealPoint(v, 3), optInt(v, 4, -1)));
		return SQ_OK;
	}

	NB_FUNC(draw)						{ self(v)->Draw(*get<wxDC>(v, 2)); return 0; }
	NB_FUNC(refresh)					{ self(v)->Refresh(); return 0; }
	NB_FUNC(contains)					{ return push(v, self(v)->Contains(*get<wxPoint>(v, 2))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, ArrowBase, xsSerializable);

class NB_WxSFArrowBase : TNitClass<wxSFArrowBase>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(shape),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(draw,			"(from: Vector2, to: Vector2, dc: wx.DC)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(shape)					{ return push(v, self(v)->GetParentShape()); }

	NB_PROP_SET(shape)					{ self(v)->SetParentShape(get<wxSFShapeBase>(v, 2)); return 0; }

	NB_FUNC(draw)						{ self(v)->Draw(GetWxRealPoint(v, 2), GetWxRealPoint(v, 3), *get<wxDC>(v, 4)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, DiagramManager, xsSerializable);

class NB_WxSFDiagramManager : TNitClass<wxSFDiagramManager>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(version),
			PROP_ENTRY_R(acceptedShapes),
			PROP_ENTRY	(canvas),
			PROP_ENTRY_R(empty),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(					"()"),
			FUNC_ENTRY_H(connect,			"(srcId: int, trgId: int, saveState=true): wx.sf.ShapeBase"
			"\n"							"(srcId: int, trgId: int, lineType: wx.ClassInfo, saveState=true): wx.sf.ShapeBase"
			"\n"							"(srcId: int, trgId: int, line: wx.sf.LineShape, saveState=true): wx.sf.ShapeBase"),
			FUNC_ENTRY_H(addShape,			"(shapeType: wx.ClassInfo, saveState=true): wx.sf.ShapeBase"
			"\n"							"(shapeType: wx.ClassInfo, pos: wx.Point, saveState=true): wx.sf.ShapeBase"
			"\n"							"(shape: wx.sf.ShpaeBase, parent: wx.xs.Serializable, pos: wx.Point, initialize: bool, saveState=true): wx.sf.ShapeBase"),
			FUNC_ENTRY_H(removeShape,		"(shape: wx.sf.ShapeBase, refresh=true)"),
			FUNC_ENTRY_H(removeShapes,		"(selection: wx.sf.ShapeBase[])"),
			FUNC_ENTRY_H(clear,				"()"),
			FUNC_ENTRY_H(moveShapesFromNegatives, "()"),
			FUNC_ENTRY_H(updateAll,			"()"),
			FUNC_ENTRY_H(acceptShape,		"(type: string)"),
			FUNC_ENTRY_H(isShapeAccepted,	"(type: string): bool"),
			FUNC_ENTRY_H(clearAcceptedShapes, "()"),
			FUNC_ENTRY_H(findShape,			"(id: int): wx.sf.ShapeBase"),
			FUNC_ENTRY_H(getConnections,	"(parent: wx.sf.ShapeBase, shapeType: wx.ClassInfo=null, mode: wx.sf.ShapeBase.CONNECT): wx.sf.ShapeBase[]"),
			FUNC_ENTRY_H(getShapes,			"(shapeType: wx.ClassInfo=null, mode=wx.xs.Serializable.SEARCH.BFS): wx.sf.ShapeBase[]"),
			FUNC_ENTRY_H(getShapeAt,		"(pos: wx.Point, zorder=1, mode=SEARCH.BOTH): wx.sf.ShapeBase"),
			FUNC_ENTRY_H(getShapesAt,		"(pos: wx.Point): wx.sf.ShapeBase[]"),
			FUNC_ENTRY_H(getShapesInside,	"(rect: wx.Rect): wx.sf.ShapeBase[]"),
			FUNC_ENTRY_H(hasChildren,		"(parent: wx.sf.ShapeBase): bool"),
			FUNC_ENTRY_H(getNeighbors,		"(parent: wx.sf.ShapeBase, shapeType: wx.ClassInfo=null, connDir: wx.sf.ShapeBase.CONNECT, direct=true): wx.sf.ShapeBase[]"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "SEARCH");
		newSlot(v, -1, "SELECTED",		(int)type::searchSELECTED);
		newSlot(v, -1, "UNSELECTED",	(int)type::searchUNSELECTED);
		newSlot(v, -1, "BOTH",			(int)type::searchBOTH);
		sq_poptop(v);
	}

	NB_PROP_GET(version)				{ return push(v, self(v)->GetVersion()); }
	NB_PROP_GET(acceptedShapes)			{ wxArrayString& arr = self(v)->GetAcceptedShapes(); return nitNativeItr(v, arr.begin(), arr.end()); }
	NB_PROP_GET(canvas)					{ return push(v, self(v)->GetShapeCanvas()); }
	NB_PROP_GET(empty)					{ return push(v, self(v)->IsEmpty()); }

	NB_PROP_SET(canvas)					{ self(v)->SetShapeCanvas(get<wxSFShapeCanvas>(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, new wxSFDiagramManager()); return SQ_OK; }

	NB_FUNC(connect)
	{
		wxSF::ERRCODE err = (wxSF::ERRCODE)-1;
		wxSFShapeBase* ret = NULL;

		if (isNone(v, 4) || isBool(v, 4))
			ret = self(v)->CreateConnection(getInt(v, 2), getInt(v, 3), optBool(v, 4, true), &err);
		else if (is<wxSFLineShape>(v, 4))
			ret = self(v)->CreateConnection(getInt(v, 2), getInt(v, 3), get<wxSFLineShape>(v, 4), optBool(v, 5, true), &err);
		else
			ret = self(v)->CreateConnection(getInt(v, 2), getInt(v, 3), GetWxClassInfo(v, 4), optBool(v, 5, true), &err);

		if (err == wxSF::errOK && ret) return push(v, ret);

		return SFError(v, "can't AddShape", err);
	}

	NB_FUNC(addShape)
	{
		wxSF::ERRCODE err = (wxSF::ERRCODE)-1;
		wxSFShapeBase* ret = NULL;
		if (isNone(v, 3) || isBool(v, 3))
			ret = self(v)->AddShape(GetWxClassInfo(v, 2), optBool(v, 3, true), &err);
		else if (is<wxSFShapeBase>(v, 2))
			ret = self(v)->AddShape(get<wxSFShapeBase>(v, 2), get<xsSerializable>(v, 3), *get<wxPoint>(v, 4), getBool(v, 5), optBool(v, 6, true), &err);
		else
			ret = self(v)->AddShape(GetWxClassInfo(v, 2), *get<wxPoint>(v, 3), optBool(v, 4, true), &err);

		if (err == wxSF::errOK && ret) return push(v, ret);

		return SFError(v, "can't AddShape", err);
	}

	NB_FUNC(removeShape)				{ self(v)->RemoveShape(get<wxSFShapeBase>(v, 2), optBool(v, 3, true)); return 0; }

	NB_FUNC(removeShapes)
	{
		ShapeList sel;
		for (NitIterator itr(v, 2); itr.hasNext(); itr.next())
			sel.push_back(get<wxSFShapeBase>(v, itr.valueIndex()));
		self(v)->RemoveShapes(sel);
		return 0;
	}

	NB_FUNC(clear)						{ self(v)->Clear(); return 0; }
	NB_FUNC(moveShapesFromNegatives)	{ self(v)->MoveShapesFromNegatives(); return 0; }
	NB_FUNC(updateAll)					{ self(v)->UpdateAll(); return 0; }
	NB_FUNC(acceptShape)				{ self(v)->AcceptShape(getWxString(v, 2)); return 0; }
	NB_FUNC(isShapeAccepted)			{ return push(v, self(v)->IsShapeAccepted(getWxString(v, 2))); }
	NB_FUNC(clearAcceptedShapes)		{ self(v)->ClearAcceptedShapes(); return 0; }
	NB_FUNC(findShape)					{ return push(v, self(v)->FindShape(getInt(v, 2))); }

	NB_FUNC(getConnections)
	{
		ShapeList shapes;
		self(v)->GetAssignedConnections(get<wxSFShapeBase>(v, 2), OptWxClassInfo(v, 3, NULL), (wxSFShapeBase::CONNECTMODE)getInt(v, 4), shapes);
		sq_newarray(v, 0);
		for (ShapeList::iterator itr = shapes.begin(), end = shapes.end(); itr != end; ++itr)
			arrayAppend(v, -1, *itr);
		return 1;
	}

	NB_FUNC(getShapes)
	{
		ShapeList shapes;
		self(v)->GetShapes(OptWxClassInfo(v, 2, NULL), shapes, (xsSerializable::SEARCHMODE)optInt(v, 3, xsSerializable::searchBFS));
		sq_newarray(v, 0);
		for (ShapeList::iterator itr = shapes.begin(), end = shapes.end(); itr != end; ++itr)
			arrayAppend(v, -1, *itr);
		return 1;
	}

	NB_FUNC(getShapeAt)					{ return push(v, self(v)->GetShapeAtPosition(*get<wxPoint>(v, 2), optInt(v, 3, -1), (type::SEARCHMODE)optInt(v, 4, type::searchBOTH))); }

	NB_FUNC(getShapesAt)				
	{
		ShapeList shapes;
		self(v)->GetShapesAtPosition(*get<wxPoint>(v, 2), shapes);
		sq_newarray(v, 0);
		for (ShapeList::iterator itr = shapes.begin(), end = shapes.end(); itr != end; ++itr)
			arrayAppend(v, -1, *itr);
		return 1;
	}

	NB_FUNC(getShapesInside)
	{
		ShapeList shapes;
		self(v)->GetShapesInside(*get<wxRect>(v, 2), shapes);
		sq_newarray(v, 0);
		for (ShapeList::iterator itr = shapes.begin(), end = shapes.end(); itr != end; ++itr)
			arrayAppend(v, -1, *itr);
		return 1;
	}

	NB_FUNC(hasChildren)				{ return push(v, self(v)->HasChildren(get<wxSFShapeBase>(v, 2))); }

	NB_FUNC(getNeighbors)
	{
		ShapeList shapes;
		self(v)->GetNeighbours(get<wxSFShapeBase>(v, 2), shapes, OptWxClassInfo(v, 3, NULL), (wxSFShapeBase::CONNECTMODE)getInt(v, 4), optBool(v, 5, true));
		sq_newarray(v, 0);
		for (ShapeList::iterator itr = shapes.begin(), end = shapes.end(); itr != end; ++itr)
			arrayAppend(v, -1, *itr);
		return 1;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, ShapeCanvas, wxScrolledWindow);

class NB_WxSFShapeCanvas : TNitClass<wxSFShapeCanvas>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(diagram),
			PROP_ENTRY_R(totalBoundingBox),
			PROP_ENTRY_R(selectionBB),
			PROP_ENTRY	(style),
			PROP_ENTRY	(canvasColor),
			PROP_ENTRY	(gradientFrom),
			PROP_ENTRY	(gradientTo),
			PROP_ENTRY	(grid),
			PROP_ENTRY	(gridLineMult),
			PROP_ENTRY	(gridColor),
			PROP_ENTRY	(gridStyle),
			PROP_ENTRY	(shadowOffset),
			PROP_ENTRY	(shadowFill),
			PROP_ENTRY	(printHAlign),
			PROP_ENTRY	(printVAlign),
			PROP_ENTRY	(printMode),
			PROP_ENTRY	(minScale),
			PROP_ENTRY	(maxScale),
			PROP_ENTRY	(scale),
			PROP_ENTRY	(hoverColor),
			PROP_ENTRY_R(history),
			PROP_ENTRY_R(mode),
			PROP_ENTRY_R(multiSelBox),
			PROP_ENTRY_R(canCopy),
			PROP_ENTRY_R(canCut),
			PROP_ENTRY_R(canPaste),
			PROP_ENTRY_R(canUndo),
			PROP_ENTRY_R(canRedo),
			PROP_ENTRY_R(canAlignSelected),
			PROP_ENTRY_R(selectedShapes),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(manager: wx.sf.DiagramManager, parent: wx.Window=null, id=wx.ID.ANY, pos=null, size=null, style=wx.WS.HSCROLL|wx.WS.VSCROLL)"),

			FUNC_ENTRY_H(startInteractiveConnection,"(shapeType: wx.ClassInfo, pos: wx.Point)"
			"\n"									"(shape: wx.sf.LineShape, pos: wx.Point)"),
			FUNC_ENTRY_H(abortInteractiveConnection,"()"),

			FUNC_ENTRY_H(selectAll,					"()"),
			FUNC_ENTRY_H(deselectAll,				"()"),
			FUNC_ENTRY_H(hideAllHandles,			"()"),
			FUNC_ENTRY_H(refreshCanvas,				"(erase: bool, rect: wx.Rect)"),
			FUNC_ENTRY_H(invalidateRect,			"(rect: wx.Rect)"),
			FUNC_ENTRY_H(invalidateVisibleRect,		"()"),
			FUNC_ENTRY_H(refreshInvalidatedRect,	"()"),
			FUNC_ENTRY_H(showShadows,				"(show: bool, style: SHADOW)"),
			FUNC_ENTRY_H(doDragDrop,				"(shapes: wx.sf.ShapeBase[], start=wx.Point(-1,-1)): wx.DRAG_RESULT"),

			FUNC_ENTRY_H(copy,						"()"),
			FUNC_ENTRY_H(cut,						"()"),
			FUNC_ENTRY_H(paste,						"()"),
			FUNC_ENTRY_H(undo,						"()"),
			FUNC_ENTRY_H(redo,						"()"),

			FUNC_ENTRY_H(saveCanvasState,			"()"),
			FUNC_ENTRY_H(clearCanvasHistory,		"()"),

			FUNC_ENTRY_H(print,						"(prompt=true)"),
			FUNC_ENTRY_H(printPreview,				"()"),
			FUNC_ENTRY_H(pageSetup,					"()"),

			FUNC_ENTRY_H(dp2lp,						"(pos: wx.Point): wx.Point"
			"\n"									"(rect: wx.Rect): wx.Rect"),
			FUNC_ENTRY_H(lp2dp,						"(pos: wx.Point): wx.Point"
			"\n"									"(rect: wx.Rect): wx.Rect"),

			FUNC_ENTRY_H(getShapeUnderCursor,		"(mode=SEARCH.BOTH): wx.sf.ShapeBase"),
			FUNC_ENTRY_H(getShapeAt,				"(pos: wx.Point, zorder=1, mode=SEARCH.BOTH): wx.sf.ShapeBase"),
			FUNC_ENTRY_H(getTopmostHandleAt,		"(pos: wx.Point): wx.sf.ShapeHandle"),
			FUNC_ENTRY_H(getShapesAt,				"(pos: wx.Point): wx.sf.ShapeBase[]"),
			FUNC_ENTRY_H(getShapesInside,			"(rect: wx.Rect): wx.sf.ShapeBase[]"),
			FUNC_ENTRY_H(alignSelected,				"(halign: HALIGN, valign: VALIGN)"),

			FUNC_ENTRY_H(addStyle,					"(style: STYLE)"),
			FUNC_ENTRY_H(removeStyle,				"(style: STYLE)"),
			FUNC_ENTRY_H(hasStyle,					"(style: STYLE): bool"),

			FUNC_ENTRY_H(scaleToViewAll,			"()"),
			FUNC_ENTRY_H(scrollToShape,				"(shape: wx.sf.ShapeBase)"),
			FUNC_ENTRY_H(snapToGrid,				"(pos: wx.Point): wx.Point"),
			FUNC_ENTRY_H(updateMultiEditSize,		"()"),
			FUNC_ENTRY_H(updateVirtualSize,			"()"),
			FUNC_ENTRY_H(moveShapesFromNegatives,	"()"),
			FUNC_ENTRY_H(centerShapes,				"()"),
			FUNC_ENTRY_H(validateSelection,			"(sel: wx.sf.ShapeBase[])"),
			FUNC_ENTRY_H(drawContent,				"(dc: wx.DC, fromPaint: bool)"),
			FUNC_ENTRY_H(deleteAllTextCtrls,		"()"),

			FUNC_ENTRY_H(enableGC,		"[class] (enable=true)"),
			FUNC_ENTRY_H(isGCEnabled,	"[class] (): bool"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "MODE");
		newSlot(v, -1, "READY",				(int)type::modeREADY);
		newSlot(v, -1, "HANDLEMOVE",		(int)type::modeHANDLEMOVE);
		newSlot(v, -1, "MULTIHANDLEMOVE",	(int)type::modeMULTIHANDLEMOVE);
		newSlot(v, -1, "SHAPEMOVE",			(int)type::modeSHAPEMOVE);
		newSlot(v, -1, "MULTISELECTION",	(int)type::modeMULTISELECTION);
		newSlot(v, -1, "CREATECONNECTION",	(int)type::modeCREATECONNECTION);
		newSlot(v, -1, "DND",				(int)type::modeDND);
		sq_poptop(v);

		addStaticTable(v, "SEARCH");
		newSlot(v, -1, "SELECTED",			(int)type::searchSELECTED);
		newSlot(v, -1, "UNSELECTED",		(int)type::searchUNSELECTED);
		newSlot(v, -1, "BOTH",				(int)type::searchBOTH);
		sq_poptop(v);

		addStaticTable(v, "VALIGN");
		newSlot(v, -1, "NONE",				(int)type::valignNONE);
		newSlot(v, -1, "TOP",				(int)type::valignTOP);
		newSlot(v, -1, "MIDDLE",			(int)type::valignMIDDLE);
		newSlot(v, -1, "BOTTOM",			(int)type::valignBOTTOM);
		sq_poptop(v);

		addStaticTable(v, "HALIGN");
		newSlot(v, -1, "NONE",				(int)type::halignNONE);
		newSlot(v, -1, "LEFT",				(int)type::halignLEFT);
		newSlot(v, -1, "CENTER",			(int)type::halignCENTER);
		newSlot(v, -1, "RIGHT",				(int)type::halignRIGHT);
		sq_poptop(v);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "MULTI_SELECTION",	(int)type::sfsMULTI_SELECTION);
		newSlot(v, -1, "MULTI_SIZE_CHANGE",	(int)type::sfsMULTI_SIZE_CHANGE);
		newSlot(v, -1, "GRID_SHOW",			(int)type::sfsGRID_SHOW);
		newSlot(v, -1, "GRID_USE",			(int)type::sfsGRID_USE);
		newSlot(v, -1, "DND",				(int)type::sfsDND);
		newSlot(v, -1, "UNDOREDO",			(int)type::sfsUNDOREDO);
		newSlot(v, -1, "CLIPBOARD",			(int)type::sfsCLIPBOARD);
		newSlot(v, -1, "HOVERING",			(int)type::sfsHOVERING);
		newSlot(v, -1, "HIGHLIGHTING",		(int)type::sfsHIGHLIGHTING);
		newSlot(v, -1, "GRADIENT_BACKGROUND",(int)type::sfsGRADIENT_BACKGROUND);
		newSlot(v, -1, "PRINT_BACKGROUND",	(int)type::sfsPRINT_BACKGROUND);
		newSlot(v, -1, "PROCESS_MOUSEWHEEL",(int)type::sfsPROCESS_MOUSEWHEEL);
		newSlot(v, -1, "DEFAULT",			(int)type::sfsDEFAULT_CANVAS_STYLE);
		sq_poptop(v);

		addStaticTable(v, "SHADOW");
		newSlot(v, -1, "TOPMOST",			(int)type::shadowTOPMOST);
		newSlot(v, -1, "ALL",				(int)type::shadowALL);
		sq_poptop(v);

		addStaticTable(v, "PRINT");
		newSlot(v, -1, "FIT_TO_PAPER",		(int)type::prnFIT_TO_PAPER);
		newSlot(v, -1, "FIT_TO_PAGE",		(int)type::prnFIT_TO_PAGE);
		newSlot(v, -1, "FIT_TO_MARGINS",	(int)type::prnFIT_TO_MARGINS);
		newSlot(v, -1, "MAP_TO_PAPER",		(int)type::prnMAP_TO_PAPER);
		newSlot(v, -1, "MAP_TO_PAGE",		(int)type::prnMAP_TO_PAGE);
		newSlot(v, -1, "MAP_TO_MARGINS",	(int)type::prnMAP_TO_MARGINS);
		newSlot(v, -1, "MAP_TO_DEVICE",		(int)type::prnMAP_TO_DEVICE);
		sq_poptop(v);
	}

	NB_PROP_GET(diagram)				{ return push(v, self(v)->GetDiagramManager()); }
	NB_PROP_GET(totalBoundingBox)		{ return push(v, self(v)->GetTotalBoundingBox()); }
	NB_PROP_GET(selectionBB)			{ return push(v, self(v)->GetSelectionBB()); }
	NB_PROP_GET(style)					{ return push(v, (int)self(v)->GetStyle()); }
	NB_PROP_GET(canvasColor)			{ return PushWxColor(v, self(v)->GetCanvasColour()); }
	NB_PROP_GET(gradientFrom)			{ return PushWxColor(v, self(v)->GetGradientFrom()); }
	NB_PROP_GET(gradientTo)				{ return PushWxColor(v, self(v)->GetGradientTo()); }
	NB_PROP_GET(grid)					{ return push(v, self(v)->GetGrid()); }
	NB_PROP_GET(gridLineMult)			{ return push(v, self(v)->GetGrigLineMult()); }
	NB_PROP_GET(gridColor)				{ return PushWxColor(v, self(v)->GetGridColour()); }
	NB_PROP_GET(gridStyle)				{ return push(v, self(v)->GetGridStyle()); }
	NB_PROP_GET(shadowOffset)			{ return PushWxRealPoint(v, self(v)->GetShadowOffset()); }
	NB_PROP_GET(shadowFill)				{ return push(v, self(v)->GetShadowFill()); }
	NB_PROP_GET(printHAlign)			{ return push(v, (int)self(v)->GetPrintHAlign()); }
	NB_PROP_GET(printVAlign)			{ return push(v, (int)self(v)->GetPrintVAlign()); }
	NB_PROP_GET(printMode)				{ return push(v, (int)self(v)->GetPrintMode()); }
	NB_PROP_GET(minScale)				{ return push(v, (float)self(v)->GetMinScale()); }
	NB_PROP_GET(maxScale)				{ return push(v, (float)self(v)->GetMaxScale()); }
	NB_PROP_GET(scale)					{ return push(v, (float)self(v)->GetScale()); }
	NB_PROP_GET(hoverColor)				{ return PushWxColor(v, self(v)->GetHoverColour()); }
	NB_PROP_GET(history)				{ return push(v, &self(v)->GetHistoryManager()); }
	NB_PROP_GET(mode)					{ return push(v, (int)self(v)->GetMode()); }
	NB_PROP_GET(multiSelBox)			{ return push(v, &self(v)->GetMultiselectionBox()); }
	NB_PROP_GET(canCopy)				{ return push(v, self(v)->CanCopy()); }
	NB_PROP_GET(canCut)					{ return push(v, self(v)->CanCut()); }
	NB_PROP_GET(canPaste)				{ return push(v, self(v)->CanPaste()); }
	NB_PROP_GET(canUndo)				{ return push(v, self(v)->CanUndo()); }
	NB_PROP_GET(canRedo)				{ return push(v, self(v)->CanRedo()); }
	NB_PROP_GET(canAlignSelected)		{ return push(v, self(v)->CanAlignSelected()); }

	NB_PROP_GET(selectedShapes)
	{
		ShapeList shapes;
		self(v)->GetSelectedShapes(shapes);
		sq_newarray(v, 0);
		for (ShapeList::iterator itr = shapes.begin(), end = shapes.end(); itr != end; ++itr)
			arrayAppend(v, -1, *itr);
		return 1;
	}

	NB_PROP_SET(diagram)				{ self(v)->SetDiagramManager(get<wxSFDiagramManager>(v, 2)); return 0; }
	NB_PROP_SET(style)					{ self(v)->SetStyle((type::STYLE)getInt(v, 2)); return 0; }
	NB_PROP_SET(canvasColor)			{ self(v)->SetCanvasColour(GetWxColor(v, 2)); return 0; }
	NB_PROP_SET(gradientFrom)			{ self(v)->SetGradientFrom(GetWxColor(v, 2)); return 0; }
	NB_PROP_SET(gradientTo)				{ self(v)->SetGradientTo(GetWxColor(v, 2)); return 0; }
	NB_PROP_SET(grid)					{ self(v)->SetGrid(*get<wxSize>(v, 2)); return 0; }
	NB_PROP_SET(gridLineMult)			{ self(v)->SetGridLineMult(getInt(v, 2)); return 0; }
	NB_PROP_SET(gridColor)				{ self(v)->SetGridColour(GetWxColor(v, 2)); return 0; }
	NB_PROP_SET(gridStyle)				{ self(v)->SetGridStyle(getInt(v, 2)); return 0; }
	NB_PROP_SET(shadowOffset)			{ self(v)->SetShadowOffset(GetWxRealPoint(v, 2)); return 0; }
	NB_PROP_SET(shadowFill)				{ self(v)->SetShadowFill(*get<wxBrush>(v, 2)); return 0; }
	NB_PROP_SET(printHAlign)			{ self(v)->SetPrintHAlign((type::HALIGN)getInt(v, 2)); return 0; }
	NB_PROP_SET(printVAlign)			{ self(v)->SetPrintVAlign((type::VALIGN)getInt(v, 2)); return 0; }
	NB_PROP_SET(printMode)				{ self(v)->SetPrintMode((type::PRINTMODE)getInt(v, 2)); return 0; }
	NB_PROP_SET(minScale)				{ self(v)->SetMinScale(getFloat(v, 2)); return 0; }
	NB_PROP_SET(maxScale)				{ self(v)->SetMaxScale(getFloat(v, 2)); return 0; }
	NB_PROP_SET(scale)					{ self(v)->SetScale(getFloat(v, 2)); return 0; }
	NB_PROP_SET(hoverColor)				{ self(v)->SetHoverColour(GetWxColor(v, 2)); return 0; }

	NB_CONS()
	{
		type* self = setSelf(v, new type(
			get<wxSFDiagramManager>(v, 2),
			opt<wxWindow>(v, 3, NULL),
			optInt(v, 4, wxID_ANY),
			*opt<wxPoint>(v, 5, wxDefaultPosition),
			*opt<wxSize>(v, 6, wxDefaultSize),
			optInt(v, 7, wxHSCROLL | wxVSCROLL)));
		return SQ_OK;
	}

	NB_FUNC(enableGC)					{ type::EnableGC(optBool(v, 2, true)); return 0; }
	NB_FUNC(isGCEnabled)				{ return push(v, type::IsGCEnabled()); }

	NB_FUNC(startInteractiveConnection)
	{ 
		wxSF::ERRCODE err = (wxSF::ERRCODE)-1;
		if (is<wxSFLineShape>(v, 2))
			self(v)->StartInteractiveConnection(get<wxSFLineShape>(v, 2), *get<wxPoint>(v, 3), &err);
		else
			self(v)->StartInteractiveConnection(GetWxClassInfo(v, 2), *get<wxPoint>(v, 3), &err);

		if (err == wxSF::errOK) return 0;

		return SFError(v, "can't start interactive connection", err);
	}

	NB_FUNC(abortInteractiveConnection)	{ self(v)->AbortInteractiveConnection(); return 0; }

	NB_FUNC(selectAll)					{ self(v)->SelectAll(); return 0; }
	NB_FUNC(deselectAll)				{ self(v)->DeselectAll(); return 0; }
	NB_FUNC(hideAllHandles)				{ self(v)->HideAllHandles(); return 0; }
	NB_FUNC(refreshCanvas)				{ self(v)->RefreshCanvas(getBool(v, 2), *get<wxRect>(v, 3)); return 0; }
	NB_FUNC(invalidateRect)				{ self(v)->InvalidateRect(*get<wxRect>(v, 2)); return 0; }
	NB_FUNC(invalidateVisibleRect)		{ self(v)->InvalidateVisibleRect(); return 0; }
	NB_FUNC(refreshInvalidatedRect)		{ self(v)->RefreshInvalidatedRect(); return 0; }
	NB_FUNC(showShadows)				{ self(v)->ShowShadows(getBool(v, 2), (type::SHADOWMODE)getInt(v, 3)); return 0; }

	NB_FUNC(doDragDrop)				
	{ 
		ShapeList shapes;
		for (NitIterator itr(v, 2); itr.hasNext(); itr.next())
			shapes.push_back(get<wxSFShapeBase>(v, itr.valueIndex()));

		return push(v, (int)self(v)->DoDragDrop(shapes, *opt<wxPoint>(v, 3, wxPoint(-1, -1))));
	}

	NB_FUNC(copy)						{ self(v)->Copy(); return 0; }
	NB_FUNC(cut)						{ self(v)->Cut(); return 0; }
	NB_FUNC(paste)						{ self(v)->Paste(); return 0; }
	NB_FUNC(undo)						{ self(v)->Undo(); return 0; }
	NB_FUNC(redo)						{ self(v)->Redo(); return 0; }

	NB_FUNC(saveCanvasState)			{ self(v)->SaveCanvasState(); return 0; }
	NB_FUNC(clearCanvasHistory)			{ self(v)->ClearCanvasHistory(); return 0; }

	NB_FUNC(print)						{ self(v)->Print(optBool(v, 2, true)); return 0; }
	NB_FUNC(printPreview)				{ self(v)->PrintPreview(); return 0; }
	NB_FUNC(pageSetup)					{ self(v)->PageSetup(); return 0; }

	NB_FUNC(dp2lp)
	{
		if (is<wxRect>(v, 2))
			return push(v, self(v)->DP2LP(*get<wxRect>(v, 2)));
		else
			return push(v, self(v)->DP2LP(*get<wxPoint>(v, 2)));
	}

	NB_FUNC(lp2dp)
	{
		if (is<wxRect>(v, 2))
			return push(v, self(v)->LP2DP(*get<wxRect>(v, 2)));
		else
			return push(v, self(v)->LP2DP(*get<wxPoint>(v, 2)));
	}

	NB_FUNC(getShapeUnderCursor)		{ return push(v, self(v)->GetShapeUnderCursor((type::SEARCHMODE)optInt(v, 2, type::searchBOTH))); }

	NB_FUNC(getShapeAt)					{ return push(v, self(v)->GetShapeAtPosition(*get<wxPoint>(v, 2), optInt(v, 3, -1), (type::SEARCHMODE)optInt(v, 4, type::searchBOTH))); }
	NB_FUNC(getTopmostHandleAt)			{ return push(v, self(v)->GetTopmostHandleAtPosition(*get<wxPoint>(v, 2))); }

	NB_FUNC(getShapesAt)				
	{
		ShapeList shapes;
		self(v)->GetShapesAtPosition(*get<wxPoint>(v, 2), shapes);
		sq_newarray(v, 0);
		for (ShapeList::iterator itr = shapes.begin(), end = shapes.end(); itr != end; ++itr)
			arrayAppend(v, -1, *itr);
		return 1;
	}

	NB_FUNC(getShapesInside)
	{
		ShapeList shapes;
		self(v)->GetShapesInside(*get<wxRect>(v, 2), shapes);
		sq_newarray(v, 0);
		for (ShapeList::iterator itr = shapes.begin(), end = shapes.end(); itr != end; ++itr)
			arrayAppend(v, -1, *itr);
		return 1;
	}

	NB_FUNC(alignSelected)				{ self(v)->AlignSelected((type::HALIGN)getInt(v, 2), (type::VALIGN)getInt(v, 3)); return 0; }
	NB_FUNC(addStyle)					{ self(v)->AddStyle((type::STYLE)getInt(v, 2)); return 0; }
	NB_FUNC(removeStyle)				{ self(v)->RemoveStyle((type::STYLE)getInt(v, 2)); return 0; }
	NB_FUNC(hasStyle)					{ return push(v, self(v)->ContainsStyle((type::STYLE)getInt(v, 2))); }

	NB_FUNC(scaleToViewAll)				{ self(v)->SetScaleToViewAll(); return 0; }
	NB_FUNC(scrollToShape)				{ self(v)->ScrollToShape(get<wxSFShapeBase>(v, 2)); return 0; }
	NB_FUNC(snapToGrid)					{ return push(v, self(v)->FitPositionToGrid(*get<wxPoint>(v, 2))); }
	NB_FUNC(updateMultiEditSize)		{ self(v)->UpdateMultieditSize(); return 0; }
	NB_FUNC(updateVirtualSize)			{ self(v)->UpdateVirtualSize(); return 0; }
	NB_FUNC(moveShapesFromNegatives)	{ self(v)->MoveShapesFromNegatives(); return 0; }
	NB_FUNC(centerShapes)				{ self(v)->CenterShapes(); return 0; }

	NB_FUNC(validateSelection)			
	{ 
		ShapeList shapes;
		for (NitIterator itr(v, 2); itr.hasNext(); itr.next())
			shapes.push_back(get<wxSFShapeBase>(v, itr.valueIndex()));

		self(v)->ValidateSelection(shapes);
		return 0;
	}

	NB_FUNC(drawContent)				{ self(v)->DrawContent(*get<wxDC>(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC(deleteAllTextCtrls)			{ self(v)->DeleteAllTextCtrls(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, CanvasHistory, wxObject);

class NB_WxSFCanvasHistory : TNitClass<wxSFCanvasHistory>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(depth),
			PROP_ENTRY	(mode),
			PROP_ENTRY_R(canUndo),
			PROP_ENTRY_R(canRedo),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(saveCanvasState,	"()"),
			FUNC_ENTRY_H(restoreOlderState,	"()"),
			FUNC_ENTRY_H(restoreNewerState,	"()"),
			FUNC_ENTRY_H(clear,				"()"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "MODE");
		newSlot(v, -1, "USE_SERIALIZATION",	(int)type::histUSE_SERIALIZATION);
		newSlot(v, -1, "USE_CLONING",		(int)type::histUSE_CLONING);
		sq_poptop(v);
	}

	NB_PROP_GET(depth)					{ return push(v, self(v)->GetHistoryDepth()); }
	NB_PROP_GET(mode)					{ return push(v, (int)self(v)->GetMode()); }
	NB_PROP_GET(canUndo)				{ return push(v, self(v)->CanUndo()); }
	NB_PROP_GET(canRedo)				{ return push(v, self(v)->CanRedo()); }

	NB_PROP_SET(depth)					{ self(v)->SetHistoryDepth(getInt(v, 2)); return 0; }
	NB_PROP_SET(mode)					{ self(v)->SetMode((type::MODE)getInt(v, 2)); return 0; }

	NB_FUNC(saveCanvasState)			{ self(v)->SaveCanvasState(); return 0; }
	NB_FUNC(restoreOlderState)			{ self(v)->RestoreOlderState(); return 0; }
	NB_FUNC(restoreNewerState)			{ self(v)->RestoreNewerState(); return 0; }
	NB_FUNC(clear)						{ self(v)->Clear(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, Thumbnail, wxPanel);

class NB_WxSFThumbnail : TNitClass<wxSFThumbnail>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(thumbStyle),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: wx.Window=null)"),
			FUNC_ENTRY_H(setCanvas,		"(canvas: wx.sf.ShapeCanvas)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "ID");
		newSlot(v, -1, "UPDATETIMER",	(int)type::ID_UPDATETIMER);
		newSlot(v, -1, "SHOWELEMENTS",	(int)type::IDM_SHOWELEMENTS);
		newSlot(v, -1, "SHOWCONNECTIONS",	(int)type::IDM_SHOWCONNECTIONS);
		sq_poptop(v);

		addStaticTable(v, "THUMB");
		newSlot(v, -1, "SHOW_ELEMENTS",	(int)type::tsSHOW_ELEMENTS);
		newSlot(v, -1, "SHOW_CONNECTIONS",	(int)type::tsSHOW_CONNECTIONS);
		sq_poptop(v);
	}

	NB_PROP_GET(thumbStyle)				{ return push(v, self(v)->GetThumbStyle()); }

	NB_PROP_SET(thumbStyle)				{ self(v)->SetThumbStyle(getInt(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, new type(opt<wxWindow>(v, 2, NULL))); return SQ_OK; }

	NB_FUNC(setCanvas)					{ self(v)->SetCanvas(opt<wxSFShapeCanvas>(v, 2, NULL)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

namespace wx { namespace sf { typedef wxSFScaledDC ScaledDC; } }

NB_TYPE_AUTODELETE(NITWX_API, wx::sf::ScaledDC, wxDC, delete);

class NB_WxSFScaledDC : TNitClass<wxSFScaledDC>
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

		bind(v, props, funcs);
	}
};

////////////////////////////////////////////////////////////////////////////////

/// subclasses

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, LineShape, wxSFShapeBase);

class NB_WxSFLineShape: TNitClass<wxSFLineShape>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(srcShapeId),
			PROP_ENTRY	(trgShapeId),
			PROP_ENTRY	(srcPoint),
			PROP_ENTRY	(trgPoint),
			PROP_ENTRY	(srcArrow),
			PROP_ENTRY	(trgArrow),
			PROP_ENTRY	(linePen),
			PROP_ENTRY	(dockPoint),
			PROP_ENTRY_R(controlPoints),
			PROP_ENTRY	(standAlone),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(								"()"
			"\n"										"(src, trg: int, path: Vector2[], diagram: wx.sf.DiagramManager)"
			"\n"										"(src, trg: Vector2, path: Vector2[], diagram: wx.sf.DiagramManager)"
			"\n"										"(copyFrom: wx.sf.LineShape)"),
			FUNC_ENTRY_H(getDirectLine,					"(): [Vector2, Vector2]"),
			FUNC_ENTRY_H(setStartingConnectionPoint,	"(cp: wx.sf.ConnectionPoint)"),
			FUNC_ENTRY_H(setEndingConnectionPoint,		"(cp: wx.sf.ConnectionPoint)"),
			FUNC_ENTRY_H(getLineSegment,				"(index: int): [Vector2, Vector2]"),

			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(srcShapeId)				{ return push(v, self(v)->GetSrcShapeId()); }
	NB_PROP_GET(trgShapeId)				{ return push(v, self(v)->GetTrgShapeId()); }
	NB_PROP_GET(srcPoint)				{ return PushWxRealPoint(v, self(v)->GetSrcPoint()); }
	NB_PROP_GET(trgPoint)				{ return PushWxRealPoint(v, self(v)->GetTrgPoint()); }
	NB_PROP_GET(srcArrow)				{ return push(v, self(v)->GetSrcArrow()); }
	NB_PROP_GET(trgArrow)				{ return push(v, self(v)->GetTrgArrow()); }
	NB_PROP_GET(linePen)				{ return push(v, self(v)->GetLinePen()); }
	NB_PROP_GET(dockPoint)				{ return push(v, self(v)->GetDockPoint()); }
	NB_PROP_GET(standAlone)				{ return push(v, self(v)->IsStandAlone()); }

	NB_PROP_GET(controlPoints)			
	{ 
		wxXS::RealPointList& pts = self(v)->GetControlPoints(); 
		sq_newarray(v, 0);
		for (wxXS::RealPointList::iterator itr = pts.begin(), end = pts.end(); itr != end; ++itr)
		{
			PushWxRealPoint(v, **itr);
			sq_arrayappend(v, -2);
		}

		return 1;
	}

	NB_PROP_SET(srcShapeId)				{ self(v)->SetSrcShapeId(getInt(v, 2)); return 0; }
	NB_PROP_SET(trgShapeId)				{ self(v)->SetTrgShapeId(getInt(v, 2)); return 0; }
	NB_PROP_SET(srcPoint)				{ self(v)->SetSrcPoint(GetWxRealPoint(v, 2)); return 0; }
	NB_PROP_SET(trgPoint)				{ self(v)->SetTrgPoint(GetWxRealPoint(v, 2)); return 0; }
	NB_PROP_SET(srcArrow)				{ self(v)->SetSrcArrow(opt<wxSFArrowBase>(v, 2, NULL)); return 0; }
	NB_PROP_SET(trgArrow)				{ self(v)->SetTrgArrow(opt<wxSFArrowBase>(v, 2, NULL)); return 0; }
	NB_PROP_SET(linePen)				{ self(v)->SetLinePen(*get<wxPen>(v, 2)); return 0; }
	NB_PROP_SET(dockPoint)				{ self(v)->SetDockPoint(getInt(v, 2)); return 0; }
	NB_PROP_SET(standAlone)				{ self(v)->SetStandAlone(getBool(v, 2)); return 0; }

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new type());
		else if (is<type>(v, 2))
			setSelf(v, new type(*get<type>(v, 2)));
		else
		{
			wxXS::RealPointList path;
			for (NitIterator itr(v, 3); itr.hasNext(); itr.next())
				path.push_back(&GetWxRealPoint(v, itr.valueIndex()));

			if (isInt(v, 2))
				setSelf(v, new type(getInt(v, 2), getInt(v, 3), path, get<wxSFDiagramManager>(v, 5)));
			else
				setSelf(v, new type(GetWxRealPoint(v, 2), GetWxRealPoint(v, 3), path, get<wxSFDiagramManager>(v, 5)));
		}
		return SQ_OK;
	}

	NB_FUNC(getDirectLine)
	{
		wxRealPoint src, trg;
		self(v)->GetDirectLine(src, trg);
		sq_newarray(v, 0);
		PushWxRealPoint(v, src);
		sq_arrayappend(v, -2);
		PushWxRealPoint(v, trg);
		sq_arrayappend(v, -2);
		return 1;
	}

	NB_FUNC(setStartingConnectionPoint)	{ self(v)->SetStartingConnectionPoint(get<wxSFConnectionPoint>(v, 2)); return 0; }
	NB_FUNC(setEndingConnectionPoint)	{ self(v)->SetEndingConnectionPoint(get<wxSFConnectionPoint>(v, 2)); return 0; }

	NB_FUNC(getLineSegment)
	{
		wxRealPoint src, trg;
		self(v)->GetLineSegment(getInt(v, 2), src, trg);
		sq_newarray(v, 0);
		PushWxRealPoint(v, src);
		sq_arrayappend(v, -2);
		PushWxRealPoint(v, trg);
		sq_arrayappend(v, -2);
		return 1;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, CurveShape, wxSFLineShape);

class NB_WxSFCurveShape: TNitClass<wxSFCurveShape>
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
			CONS_ENTRY_H(				"()"
			"\n"						"(src, trg: int, path: Vector2[], diagram: wx.sf.DiagramManager)"
			"\n"						"(copyFrom: wx.sf.CurveShape)"),

			FUNC_ENTRY_H(getPoint,		"(segment: int, offset: float): Vector2"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new type());
		else if (is<type>(v, 2))
			setSelf(v, new type(*get<type>(v, 2)));
		else
		{
			wxXS::RealPointList path;
			for (NitIterator itr(v, 3); itr.hasNext(); itr.next())
				path.push_back(&GetWxRealPoint(v, itr.valueIndex()));

			setSelf(v, new type(getInt(v, 2), getInt(v, 3), path, get<wxSFDiagramManager>(v, 5)));
		}
		return SQ_OK;
	}

	NB_FUNC(getPoint)					{ return PushWxRealPoint(v, self(v)->GetPoint(getInt(v, 2), getFloat(v, 3))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, OrthoLineShape, wxSFLineShape);

class NB_WxSFOrthoLineShape: TNitClass<wxSFOrthoLineShape>
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
			CONS_ENTRY_H(				"()"
			"\n"						"(src, trg: int, path: Vector2[], diagram: wx.sf.DiagramManager)"
			"\n"						"(copyFrom: wx.sf.OrthoLineShape)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new type());
		else if (is<type>(v, 2))
			setSelf(v, new type(*get<type>(v, 2)));
		else
		{
			wxXS::RealPointList path;
			for (NitIterator itr(v, 3); itr.hasNext(); itr.next())
				path.push_back(&GetWxRealPoint(v, itr.valueIndex()));

			setSelf(v, new type(getInt(v, 2), getInt(v, 3), path, get<wxSFDiagramManager>(v, 5)));
		}
		return SQ_OK;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, RoundOrthoLineShape, wxSFOrthoLineShape);

class NB_WxSFRoundOrthoLineShape: TNitClass<wxSFRoundOrthoLineShape>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(maxRadius),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"
			"\n"						"(src, trg: int, path: Vector2[], diagram: wx.sf.DiagramManager)"
			"\n"						"(copyFrom: wx.sf.RoundOrthoLineShape)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(maxRadius)				{ return push(v, self(v)->GetMaxRadius()); }
	
	NB_PROP_SET(maxRadius)				{ self(v)->SetMaxRadius(getInt(v, 2)); return 0; }

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new type());
		else if (is<type>(v, 2))
			setSelf(v, new type(*get<type>(v, 2)));
		else
		{
			wxXS::RealPointList path;
			for (NitIterator itr(v, 3); itr.hasNext(); itr.next())
				path.push_back(&GetWxRealPoint(v, itr.valueIndex()));

			setSelf(v, new type(getInt(v, 2), getInt(v, 3), path, get<wxSFDiagramManager>(v, 5)));
		}
		return SQ_OK;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, RectShape, wxSFShapeBase);

class NB_WxSFRectShape: TNitClass<wxSFRectShape>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(fill),
			PROP_ENTRY	(border),
			PROP_ENTRY	(rectSize),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"
			"\n"						"(pos: Vector2, size: Vector2, diagram: wx.sf.DiagramManager)"
			"\n"						"(copyFrom: wx.sf.RectShape)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(fill)					{ return push(v, self(v)->GetFill()); }
	NB_PROP_GET(border)					{ return push(v, self(v)->GetBorder()); }
	NB_PROP_GET(rectSize)				{ return PushWxRealPoint(v, self(v)->GetRectSize()); }
	
	NB_PROP_SET(fill)					{ self(v)->SetFill(*get<wxBrush>(v, 2)); return 0; }
	NB_PROP_SET(border)					{ self(v)->SetBorder(*get<wxPen>(v, 2)); return 0; }
	NB_PROP_SET(rectSize)				{ self(v)->SetRectSize(GetWxRealPoint(v, 2)); return 0; }

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new type());
		else if (is<type>(v, 2))
			setSelf(v, new type(*get<type>(v, 2)));
		else
			setSelf(v, new type(GetWxRealPoint(v, 2), GetWxRealPoint(v, 3), get<wxSFDiagramManager>(v, 4)));
		return SQ_OK;
	}
};

////////////////////////////////////////////////////////////////////////////////

// TODO: Load bitmap from a nit pack

NB_TYPE_WXSF_OBJ(NITWX_API, BitmapShape, wxSFRectShape);

class NB_WxSFBitmapShape: TNitClass<wxSFBitmapShape>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(bitmapPath),
			PROP_ENTRY	(canScale),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"
			"\n"						"(pos: Vector2, bitmapPath: string, diagram: wx.sf.DiagramManager)"
			"\n"						"(copyFrom: wx.sf.BitmapShape)"),
			FUNC_ENTRY_H(loadFile,		"(filePath: string, type=wx.Bitmap.TYPE.BMP)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(bitmapPath)				{ return push(v, self(v)->GetBitmapPath()); }
	NB_PROP_GET(canScale)				{ return push(v, self(v)->CanScale()); }

	NB_PROP_SET(canScale)				{ self(v)->EnableScale(getBool(v, 2)); return 0; }

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new type());
		else if (is<type>(v, 2))
			setSelf(v, new type(*get<type>(v, 2)));
		else
			setSelf(v, new type(GetWxRealPoint(v, 2), getWxString(v, 3), get<wxSFDiagramManager>(v, 4)));
		return SQ_OK;
	}

	NB_FUNC(loadFile)					{ return push(v, self(v)->CreateFromFile(getWxString(v, 2), (wxBitmapType)optInt(v, 3, wxBITMAP_TYPE_BMP))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, ControlShape, wxSFRectShape);

class NB_WxSFControlShape: TNitClass<wxSFControlShape>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(control),
			PROP_ENTRY	(eventProcessing),
			PROP_ENTRY	(modFill),
			PROP_ENTRY	(modBorder),
			PROP_ENTRY	(controlOffset),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"
			"\n"						"(ctrl: wx.Window, pos: Vector2, size: Vector2, diagram: wx.sf.DiagramManager)"
			"\n"						"(copyFrom: wx.sf.ControlShape)"),

			FUNC_ENTRY_H(setControl,	"(ctrl: wx.Window, fitShapeToControl: bool)"),
			FUNC_ENTRY_H(updateControl,	"()"),
			FUNC_ENTRY_H(updateShape,	"()"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "EVENT_PROCESSING");
		newSlot(v, -1, "NONE",			(int)type::evtNONE);
		newSlot(v, -1, "KEY2GUI",		(int)type::evtKEY2GUI);
		newSlot(v, -1, "KEY2CANVAS",	(int)type::evtKEY2CANVAS);
		newSlot(v, -1, "MOUSE2GUI",		(int)type::evtMOUSE2GUI);
		newSlot(v, -1, "MOUSE2CANVAS",	(int)type::evtMOUSE2CANVAS);
		sq_poptop(v);
	}

	NB_PROP_GET(control)				{ return push(v, self(v)->GetControl()); }
	NB_PROP_GET(eventProcessing)		{ return push(v, self(v)->GetEventProcessing()); }
	NB_PROP_GET(modFill)				{ return push(v, self(v)->GetModFill()); }
	NB_PROP_GET(modBorder)				{ return push(v, self(v)->GetModBorder()); }
	NB_PROP_GET(controlOffset)			{ return push(v, self(v)->GetControlOffset()); }

	NB_PROP_SET(control)				{ self(v)->SetControl(get<wxWindow>(v, 2)); return 0; }
	NB_PROP_SET(eventProcessing)		{ self(v)->SetEventProcessing(getInt(v, 2)); return 0; }
	NB_PROP_SET(modFill)				{ self(v)->SetModFill(*get<wxBrush>(v, 2)); return 0; }
	NB_PROP_SET(modBorder)				{ self(v)->SetModBorder(*get<wxPen>(v, 2)); return 0; }
	NB_PROP_SET(controlOffset)			{ self(v)->SetControlOffset(getInt(v, 2)); return 0; }

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new type());
		else if (is<type>(v, 2))
			setSelf(v, new type(*get<type>(v, 2)));
		else
			setSelf(v, new type(get<wxWindow>(v, 2), GetWxRealPoint(v, 3), GetWxRealPoint(v, 4), get<wxSFDiagramManager>(v, 5)));
		return SQ_OK;
	}

	NB_FUNC(setControl)					{ self(v)->SetControl(get<wxWindow>(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC(updateControl)				{ self(v)->UpdateControl(); return 0; }
	NB_FUNC(updateShape)				{ self(v)->UpdateShape(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, EllipseShape, wxSFRectShape);

class NB_WxSFEllipseShape: TNitClass<wxSFEllipseShape>
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
			CONS_ENTRY_H(				"()"
			"\n"						"(pos: Vector2, size: Vector2, diagram: wx.sf.DiagramManager)"
			"\n"						"(copyFrom: wx.sf.EllipseShape)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new type());
		else if (is<type>(v, 2))
			setSelf(v, new type(*get<type>(v, 2)));
		else
			setSelf(v, new type(GetWxRealPoint(v, 2), GetWxRealPoint(v, 3), get<wxSFDiagramManager>(v, 4)));
		return SQ_OK;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, GridShape, wxSFRectShape);

class NB_WxSFGridShape: TNitClass<wxSFGridShape>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(dimensions),
			PROP_ENTRY	(cellSpace),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(					"()"
			"\n"							"(pos: Vector2, size: Vector2, rows: int, cols: int, cellspace: int, diagram: wx.sf.DiagramManager)"
			"\n"							"(copyFrom: wx.sf.GridShape)"),

			FUNC_ENTRY_H(getManagedShape,	"(index: int): wx.sf.ShapeBase"
			"\n"							"(row, col: int): wx.sf.ShapeBase"),
			FUNC_ENTRY_H(clearGrid,			"()"),
			FUNC_ENTRY_H(appendToGrid,		"(shape: wx.sf.ShapeBase): bool"),
			FUNC_ENTRY_H(insertToGrid,		"(row, col, shape: wx.sf.ShapeBase): bool"
			"\n"							"(index: int, shape: wx.sf.ShapeBase): bool"),
			FUNC_ENTRY_H(removeFromGrid,	"(id: int)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(dimensions)				{ wxSize dim; self(v)->GetDimensions(&dim.x, &dim.y); return push(v, dim); }
	NB_PROP_GET(cellSpace)				{ return push(v, self(v)->GetCellSpace()); }

	NB_PROP_SET(dimensions)				{ wxSize& dim = *get<wxSize>(v, 2); self(v)->SetDimensions(dim.x, dim.y); return 0; }
	NB_PROP_SET(cellSpace)				{ self(v)->SetCellSpace(getInt(v, 2)); return 0; }

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new type());
		else if (is<type>(v, 2))
			setSelf(v, new type(*get<type>(v, 2)));
		else
			setSelf(v, new type(GetWxRealPoint(v, 2), GetWxRealPoint(v, 3), getInt(v, 4), getInt(v, 5), getInt(v, 6), get<wxSFDiagramManager>(v, 7)));
		return SQ_OK;
	}

	NB_FUNC(getManagedShape)
	{
		if (isNone(v, 3))
			return push(v, self(v)->GetManagedShape(getInt(v, 2)));
		else
			return push(v, self(v)->GetManagedShape(getInt(v, 2), getInt(v, 3)));
	}

	NB_FUNC(clearGrid)					{ self(v)->ClearGrid(); return 0; }
	NB_FUNC(appendToGrid)				{ return push(v, self(v)->AppendToGrid(get<wxSFShapeBase>(v, 2))); }

	NB_FUNC(insertToGrid)
	{
		if (is<wxSFShapeBase>(v, 3))
			return push(v, self(v)->InsertToGrid(getInt(v, 2), get<wxSFShapeBase>(v, 3)));
		else
			return push(v, self(v)->InsertToGrid(getInt(v, 2), getInt(v, 3), get<wxSFShapeBase>(v, 4)));
	}

	NB_FUNC(removeFromGrid)				{ self(v)->RemoveFromGrid(getInt(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, FlexGridShape, wxSFGridShape);

class NB_WxSFFlexGridShape: TNitClass<wxSFFlexGridShape>
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
			CONS_ENTRY_H(					"()"
			"\n"							"(pos: Vector2, size: Vector2, rows: int, cols: int, cellspace: int, diagram: wx.sf.DiagramManager)"
			"\n"							"(copyFrom: wx.sf.FlexGridShape)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new type());
		else if (is<type>(v, 2))
			setSelf(v, new type(*get<type>(v, 2)));
		else
			setSelf(v, new type(GetWxRealPoint(v, 2), GetWxRealPoint(v, 3), getInt(v, 4), getInt(v, 5), getInt(v, 6), get<wxSFDiagramManager>(v, 7)));
		return SQ_OK;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, MultiSelRect, wxSFRectShape);

class NB_WxSFMultiSelRect: TNitClass<wxSFMultiSelRect>
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

		// TODO: I can't find a nit way to implement a constructor yet.

		bind(v, props, funcs);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, PolygonShape, wxSFRectShape);

class NB_WxSFPolygonShape: TNitClass<wxSFPolygonShape>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(connectToVertex),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"
			"\n"						"(points: Vector2[], pos: Vector2, diagram: wx.sf.DiagramManager)"
			"\n"						"(copyFrom: wx.sf.PolygonShape)"),

			FUNC_ENTRY_H(setVertices,	"(points: Vectror2[])"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(connectToVertex)		{ return push(v, self(v)->IsConnectedToVertex()); }

	NB_PROP_SET(connectToVertex)		{ self(v)->SetConnectToVertex(getBool(v, 2)); return 0; }

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new type());
		else if (is<type>(v, 2))
			setSelf(v, new type(*get<type>(v, 2)));
		else
		{
			vector<wxRealPoint>::type points;
			for (NitIterator itr(v, 2); itr.hasNext(); itr.next())
				points.push_back(GetWxRealPoint(v, itr.valueIndex()));

			if (points.empty())
				return sq_throwerror(v, "invalid points array");

			setSelf(v, new type(points.size(), &points[0], GetWxRealPoint(v, 3), get<wxSFDiagramManager>(v, 5)));
		}
		return SQ_OK;
	}

	NB_FUNC(setVertices)
	{
		vector<wxRealPoint>::type points;
		for (NitIterator itr(v, 2); itr.hasNext(); itr.next())
			points.push_back(GetWxRealPoint(v, itr.valueIndex()));

		if (points.empty())
			return sq_throwerror(v, "invalid points array");

		self(v)->SetVertices(points.size(), &points[0]);
		return 0; 
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, DiamondShape, wxSFPolygonShape);

class NB_WxSFDiamondShape: TNitClass<wxSFDiamondShape>
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
			CONS_ENTRY_H(					"()"
			"\n"							"(pos: Vector2, diagram: wx.sf.DiagramManager)"
			"\n"							"(copyFrom: wx.sf.DiamondShape)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new type());
		else if (is<type>(v, 2))
			setSelf(v, new type(*get<type>(v, 2)));
		else
			setSelf(v, new type(GetWxRealPoint(v, 2), get<wxSFDiagramManager>(v, 3)));
		return SQ_OK;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, RoundRectShape, wxSFRectShape);

class NB_WxSFRoundRectShape: TNitClass<wxSFRoundRectShape>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(radius),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"
			"\n"						"(pos: Vector2, size: Vector2, radius: float, diagram: wx.sf.DiagramManager)"
			"\n"						"(copyFrom: wx.sf.RoundRectShape)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(radius)					{ return push(v, (float)self(v)->GetRadius()); }

	NB_PROP_SET(radius)					{ self(v)->SetRadius(getFloat(v, 2)); return 0; }

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new type());
		else if (is<type>(v, 2))
			setSelf(v, new type(*get<type>(v, 2)));
		else
			setSelf(v, new type(GetWxRealPoint(v, 2), GetWxRealPoint(v, 3), getFloat(v, 4), get<wxSFDiagramManager>(v, 5)));
		return SQ_OK;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, SquareShape, wxSFRectShape);

class NB_WxSFSquareShape: TNitClass<wxSFSquareShape>
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
			CONS_ENTRY_H(				"()"
			"\n"						"(pos: Vector2, size: float, diagram: wx.sf.DiagramManager)"
			"\n"						"(copyFrom: wx.sf.SquareShape)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new type());
		else if (is<type>(v, 2))
			setSelf(v, new type(*get<type>(v, 2)));
		else
			setSelf(v, new type(GetWxRealPoint(v, 2), getFloat(v, 3), get<wxSFDiagramManager>(v, 4)));
		return SQ_OK;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, CircleShape, wxSFSquareShape);

class NB_WxSFCircleShape: TNitClass<wxSFCircleShape>
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
			CONS_ENTRY_H(				"()"
			"\n"						"(pos: Vector2, radius: float, diagram: wx.sf.DiagramManager)"
			"\n"						"(copyFrom: wx.sf.CircleShape)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new type());
		else if (is<type>(v, 2))
			setSelf(v, new type(*get<type>(v, 2)));
		else
			setSelf(v, new type(GetWxRealPoint(v, 2), getFloat(v, 3), get<wxSFDiagramManager>(v, 4)));
		return SQ_OK;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, TextShape, wxSFRectShape);

class NB_WxSFTextShape: TNitClass<wxSFTextShape>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(font),
			PROP_ENTRY	(text),
			PROP_ENTRY	(textColor),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"
			"\n"						"(pos: Vector2, text: string, diagram: wx.sf.DiagramManager)"
			"\n"						"(copyFrom: wx.sf.TextShape)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(font)					{ return push(v, self(v)->GetFont()); }
	NB_PROP_GET(text)					{ return push(v, self(v)->GetText()); }
	NB_PROP_GET(textColor)				{ return PushWxColor(v, self(v)->GetTextColour()); }

	NB_PROP_SET(font)					{ self(v)->SetFont(*get<wxFont>(v, 2)); return 0; }
	NB_PROP_SET(text)					{ self(v)->SetText(getWxString(v, 2)); return 0; }
	NB_PROP_SET(textColor)				{ self(v)->SetTextColour(GetWxColor(v, 2)); return 0; }

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new type());
		else if (is<type>(v, 2))
			setSelf(v, new type(*get<type>(v, 2)));
		else
			setSelf(v, new type(GetWxRealPoint(v, 2), getWxString(v, 3), get<wxSFDiagramManager>(v, 4)));
		return SQ_OK;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, EditTextShape, wxSFTextShape);

class NB_WxSFEditTextShape: TNitClass<wxSFEditTextShape>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(editType),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"
			"\n"						"(pos: Vector2, text: string, diagram: wx.sf.DiagramManager)"
			"\n"						"(copyFrom: wx.sf.EditTextShape)"),
			FUNC_ENTRY_H(editLabel,		"()"),
			FUNC_ENTRY_H(forceMultiline,"(multiline: bool)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "EDIT");
		newSlot(v, -1, "INPLACE",		(int)type::editINPLACE);
		newSlot(v, -1, "DIALOG",		(int)type::editDIALOG);
		newSlot(v, -1, "DISABLED",		(int)type::editDISABLED);
		sq_poptop(v);
	}

	NB_PROP_GET(editType)				{ return push(v, (int)self(v)->GetEditType()); }

	NB_PROP_SET(editType)				{ self(v)->SetEditType((type::EDITTYPE)getInt(v, 2)); return 0; }

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new type());
		else if (is<type>(v, 2))
			setSelf(v, new type(*get<type>(v, 2)));
		else
			setSelf(v, new type(GetWxRealPoint(v, 2), getWxString(v, 3), get<wxSFDiagramManager>(v, 4)));
		return SQ_OK;
	}

	NB_FUNC(editLabel)					{ self(v)->EditLabel(); return 0; }
	NB_FUNC(forceMultiline)				{ self(v)->ForceMultiline(getBool(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, OpenArrow, wxSFArrowBase);

class NB_WxSFOpenArrow: TNitClass<wxSFOpenArrow>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(arrowPen),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"
			"\n"						"(shape: wx.sf.ShapeBase)"
			"\n"						"(copyFrom: wx.sf.OpenArrow)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(arrowPen)				{ return push(v, self(v)->GetArrowPen()); }

	NB_PROP_SET(arrowPen)				{ self(v)->SetArrowPen(*get<wxPen>(v, 2)); return 0; }

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new type());
		else if (is<type>(v, 2))
			setSelf(v, new type(*get<type>(v, 2)));
		else
			setSelf(v, new type(get<wxSFShapeBase>(v, 2)));
		return SQ_OK;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, SolidArrow, wxSFArrowBase);

class NB_WxSFSolidArrow: TNitClass<wxSFSolidArrow>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(arrowPen),
			PROP_ENTRY	(arrowFill),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"
			"\n"						"(shape: wx.sf.ShapeBase)"
			"\n"						"(copyFrom: wx.sf.SolidArrow)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(arrowPen)				{ return push(v, self(v)->GetArrowPen()); }
	NB_PROP_GET(arrowFill)				{ return push(v, self(v)->GetArrowFill()); }

	NB_PROP_SET(arrowPen)				{ self(v)->SetArrowPen(*get<wxPen>(v, 2)); return 0; }
	NB_PROP_SET(arrowFill)				{ self(v)->SetArrowFill(*get<wxBrush>(v, 2)); return 0; }

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new type());
		else if (is<type>(v, 2))
			setSelf(v, new type(*get<type>(v, 2)));
		else
			setSelf(v, new type(get<wxSFShapeBase>(v, 2)));
		return SQ_OK;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, CircleArrow, wxSFSolidArrow);

class NB_WxSFCircleArrow: TNitClass<wxSFCircleArrow>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(radius),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"
			"\n"						"(shape: wx.sf.ShapeBase)"
			"\n"						"(copyFrom: wx.sf.CircleArrow)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(radius)					{ return push(v, self(v)->GetRadius()); }

	NB_PROP_SET(radius)					{ self(v)->SetRadius(getInt(v, 2)); return 0; }

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new type());
		else if (is<type>(v, 2))
			setSelf(v, new type(*get<type>(v, 2)));
		else
			setSelf(v, new type(get<wxSFShapeBase>(v, 2)));
		return SQ_OK;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, DiamondArrow, wxSFSolidArrow);

class NB_WxSFDiamondArrow: TNitClass<wxSFDiamondArrow>
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
			CONS_ENTRY_H(				"()"
			"\n"						"(shape: wx.sf.ShapeBase)"
			"\n"						"(copyFrom: wx.sf.DiamondArrow)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new type());
		else if (is<type>(v, 2))
			setSelf(v, new type(*get<type>(v, 2)));
		else
			setSelf(v, new type(get<wxSFShapeBase>(v, 2)));
		return SQ_OK;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, LayoutAlgorithm, wxObject);

class NB_WxSFLayoutAlgorithm : TNitClass<wxSFLayoutAlgorithm>
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

		bind(v, props, funcs);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, LayoutCircle, wxSFLayoutAlgorithm);

class NB_WxSFLayoutCircle : TNitClass<wxSFLayoutCircle>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(distanceRatio),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(distanceRatio)			{ return push(v, (float)self(v)->GetDistanceRatio()); }

	NB_PROP_SET(distanceRatio)			{ self(v)->SetDistanceRatio(getFloat(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, LayoutHorizontalTree, wxSFLayoutAlgorithm);

class NB_WxSFLayoutHorizontalTree : TNitClass<wxSFLayoutHorizontalTree>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(hspace),
			PROP_ENTRY	(vspace),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(hspace)					{ return push(v, (float)self(v)->GetHSpace()); }
	NB_PROP_GET(vspace)					{ return push(v, (float)self(v)->GetVSpace()); }

	NB_PROP_SET(hspace)					{ self(v)->SetHSpace(getFloat(v, 2)); return 0; }
	NB_PROP_SET(vspace)					{ self(v)->SetVSpace(getFloat(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, LayoutMesh, wxSFLayoutAlgorithm);

class NB_WxSFLayoutMesh : TNitClass<wxSFLayoutMesh>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(hspace),
			PROP_ENTRY	(vspace),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(hspace)					{ return push(v, (float)self(v)->GetHSpace()); }
	NB_PROP_GET(vspace)					{ return push(v, (float)self(v)->GetVSpace()); }

	NB_PROP_SET(hspace)					{ self(v)->SetHSpace(getFloat(v, 2)); return 0; }
	NB_PROP_SET(vspace)					{ self(v)->SetVSpace(getFloat(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, LayoutVerticalTree, wxSFLayoutAlgorithm);

class NB_WxSFLayoutVerticalTree : TNitClass<wxSFLayoutVerticalTree>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(hspace),
			PROP_ENTRY	(vspace),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(hspace)					{ return push(v, (float)self(v)->GetHSpace()); }
	NB_PROP_GET(vspace)					{ return push(v, (float)self(v)->GetVSpace()); }

	NB_PROP_SET(hspace)					{ self(v)->SetHSpace(getFloat(v, 2)); return 0; }
	NB_PROP_SET(vspace)					{ self(v)->SetVSpace(getFloat(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

namespace wx { namespace sf { typedef wxSFAutoLayout AutoLayout; } }
NB_TYPE_AUTODELETE(NITWX_API, wx::sf::AutoLayout, NULL, delete);

class NB_WxSFAutoLayout: TNitClass<wxSFAutoLayout>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(algorithms),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"),

			FUNC_ENTRY_H(layout,		"(diagram: wx.sf.DiagramManager, algName: string)"
			"\n"						"(canvas: wx.sf.ShapeCanvas, algName: string)"
			"\n"						"(shapes: wx.sf.ShapeBase[], algName: string)"),
			FUNC_ENTRY_H(getAlgorithm,	"(algName: string): wx.sf.LayoutAlgorithm"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(algorithms)				{ wxArrayString arr = self(v)->GetRegisteredAlgorithms(); return nitNativeItr(v, arr.begin(), arr.end()); }

	NB_CONS()							{ setSelf(v, new type()); return SQ_OK; }

	NB_FUNC(layout)
	{
		if (is<wxSFDiagramManager>(v, 2))
			self(v)->Layout(*get<wxSFDiagramManager>(v, 2), getWxString(v, 3));
		else if (is<wxSFShapeCanvas>(v, 2))
			self(v)->Layout(get<wxSFShapeCanvas>(v, 2), getWxString(v, 3));
		else
		{
			ShapeList shapes;
			for (NitIterator itr(v, 2); itr.hasNext(); itr.next())
				shapes.push_back(get<wxSFShapeBase>(v, itr.valueIndex()));

			self(v)->Layout(shapes, getWxString(v, 3));
		}
		return 0;
	}

	NB_FUNC(getAlgorithm)				{ return push(v, self(v)->GetAlgorithm(getWxString(v, 2))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, ShapeEvent, wxEvent);

class NB_WxSFShapeEvent : TNitClass<wxSFShapeEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(shape),
			PROP_ENTRY_R(allowed),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(veto,			"()"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(shape)					{ return push(v, self(v)->GetShape()); }

	NB_PROP_GET(allowed)				{ return push(v, !self(v)->IsVetoed()); }

	NB_FUNC(veto)						{ self(v)->Veto(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, ShapeHandleEvent, wxEvent);

class NB_WxSFShapeHandleEvent : TNitClass<wxSFShapeHandleEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(shape),
			PROP_ENTRY_R(handle),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(shape)					{ return push(v, self(v)->GetShape()); }
	NB_PROP_GET(handle)					{ return push(v, &self(v)->GetHandle()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, ShapeMouseEvent, wxEvent);

class NB_WxSFShapeMouseEvent : TNitClass<wxSFShapeMouseEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(shape),
			PROP_ENTRY_R(mousePosition),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(shape)					{ return push(v, self(v)->GetShape()); }
	NB_PROP_GET(mousePosition)			{ return push(v, self(v)->GetMousePosition()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, ShapeKeyEvent, wxEvent);

class NB_WxSFShapeKeyEvent : TNitClass<wxSFShapeKeyEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(shape),
			PROP_ENTRY_R(keyCode),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(shape)					{ return push(v, self(v)->GetShape()); }
	NB_PROP_GET(keyCode)				{ return push(v, self(v)->GetKeyCode()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, ShapeTextEvent, wxEvent);

class NB_WxSFShapeTextEvent : TNitClass<wxSFShapeTextEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(shape),
			PROP_ENTRY_R(text),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(shape)					{ return push(v, self(v)->GetShape()); }
	NB_PROP_GET(text)					{ return push(v, self(v)->GetText()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, ShapePasteEvent, wxEvent);

class NB_WxSFShapePasteEvent : TNitClass<wxSFShapePasteEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(pastedShapes),
			PROP_ENTRY_R(dropTarget),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(pastedShapes)			{ ShapeList& list = self(v)->GetPastedShapes(); return nitNativeItr(v, list.begin(), list.end()); }
	NB_PROP_GET(dropTarget)				{ return push(v, self(v)->GetDropTarget()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, ShapeDropEvent, wxEvent);

class NB_WxSFShapeDropEvent : TNitClass<wxSFShapeDropEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(droppedShapes),
			PROP_ENTRY_R(dropPosition),
			PROP_ENTRY_R(dropTarget),
			PROP_ENTRY	(dragResult),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(droppedShapes)			{ ShapeList& list = self(v)->GetDroppedShapes(); return nitNativeItr(v, list.begin(), list.end()); }
	NB_PROP_GET(dropPosition)			{ return push(v, self(v)->GetDropPosition()); }
	NB_PROP_GET(dropTarget)				{ return push(v, self(v)->GetDropTarget()); }
	NB_PROP_GET(dragResult)				{ return push(v, (int)self(v)->GetDragResult()); }

	NB_PROP_SET(dragResult)				{ self(v)->SetDragResult((wxDragResult)getInt(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXSF_OBJ(NITWX_API, ShapeChildDropEvent, wxEvent);

class NB_WxSFShapeChildDropEvent : TNitClass<wxSFShapeChildDropEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(shape),
			PROP_ENTRY_R(childShape),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(shape)					{ return push(v, self(v)->GetShape()); }
	NB_PROP_GET(childShape)				{ return push(v, self(v)->GetChildShape()); }
};

////////////////////////////////////////////////////////////////////////////////

SQRESULT NitLibWxSF(HSQUIRRELVM v)
{
	NB_XsProperty::Register(v);
	NB_XsSerializable::Register(v);
	NB_WxXmlSerializer::Register(v);

	NB_WxSFShapeBase::Register(v);
	NB_WxSFShapeHandle::Register(v);
	NB_WxSFConnectionPoint::Register(v);
	NB_WxSFArrowBase::Register(v);

	NB_WxSFDiagramManager::Register(v);

	NB_WxSFShapeCanvas::Register(v);
	NB_WxSFCanvasHistory::Register(v);

	NB_WxSFThumbnail::Register(v);

	NB_WxSFScaledDC::Register(v);

	NB_WxSFLineShape::Register(v);
	NB_WxSFCurveShape::Register(v);
	NB_WxSFOrthoLineShape::Register(v);
	NB_WxSFRoundOrthoLineShape::Register(v);

	NB_WxSFRectShape::Register(v);
	NB_WxSFBitmapShape::Register(v);
	NB_WxSFControlShape::Register(v);
	NB_WxSFEllipseShape::Register(v);
	NB_WxSFGridShape::Register(v);
	NB_WxSFFlexGridShape::Register(v);
	NB_WxSFMultiSelRect::Register(v);
	NB_WxSFPolygonShape::Register(v);
	NB_WxSFDiamondShape::Register(v);
	NB_WxSFRoundRectShape::Register(v);
	NB_WxSFSquareShape::Register(v);
	NB_WxSFCircleShape::Register(v);
	NB_WxSFTextShape::Register(v);
	NB_WxSFEditTextShape::Register(v);

	NB_WxSFOpenArrow::Register(v);
	NB_WxSFSolidArrow::Register(v);
	NB_WxSFCircleArrow::Register(v);
	NB_WxSFDiamondArrow::Register(v);

	NB_WxSFLayoutAlgorithm::Register(v);
	NB_WxSFLayoutCircle::Register(v);
	NB_WxSFLayoutHorizontalTree::Register(v);
	NB_WxSFLayoutMesh::Register(v);
	NB_WxSFLayoutVerticalTree::Register(v);
	NB_WxSFAutoLayout::Register(v);

	NB_WxSFShapeEvent::Register(v);
	NB_WxSFShapeHandleEvent::Register(v);
	NB_WxSFShapeMouseEvent::Register(v);
	NB_WxSFShapeKeyEvent::Register(v);
	NB_WxSFShapeTextEvent::Register(v);
	NB_WxSFShapePasteEvent::Register(v);
	NB_WxSFShapeDropEvent::Register(v);
	NB_WxSFShapeChildDropEvent::Register(v);

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
