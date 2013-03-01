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

#include <wx/ribbon/gallery.h>

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, AuiManagerEvent, wxEvent);

class NB_WxAuiManagerEvent : TNitClass<wxAuiManagerEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(dc),
			PROP_ENTRY_R(manager),
			PROP_ENTRY_R(pane),
			PROP_ENTRY_R(canVeto),
			PROP_ENTRY_R(button),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(veto,			"(veto=true)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(dc)						{ return push(v, self(v)->GetDC()); }
	NB_PROP_GET(manager)				{ return push(v, self(v)->GetManager()); }
	NB_PROP_GET(pane)					{ return push(v, *self(v)->GetPane()); }
	NB_PROP_GET(canVeto)				{ return push(v, self(v)->CanVeto()); }
	NB_PROP_GET(button)					{ return push(v, self(v)->GetButton()); }

	NB_FUNC(veto)						{ self(v)->Veto(optBool(v, 2, true)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, AuiNotebookEvent, wxBookCtrlEvent);

class NB_WxAuiNotebookEvent : TNitClass<wxAuiNotebookEvent>
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

NB_TYPE_WXOBJ(NITWX_API, AuiToolBarEvent, wxNotifyEvent);

class NB_WxAuiToolBarEvent : TNitClass<wxAuiToolBarEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(dropDownClicked),
			PROP_ENTRY_R(clickPoint),
			PROP_ENTRY_R(itemRect),
			PROP_ENTRY_R(toolId),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(dropDownClicked)		{ return push(v, self(v)->IsDropDownClicked()); }
	NB_PROP_GET(clickPoint)				{ return push(v, self(v)->GetClickPoint()); }
	NB_PROP_GET(itemRect)				{ return push(v, self(v)->GetItemRect()); }
	NB_PROP_GET(toolId)					{ return push(v, self(v)->GetToolId()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXVALUE(NITWX_API, AuiPaneInfo, NULL);

class NB_WxAuiPaneInfo : TNitClass<wxAuiPaneInfo>
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
			CONS_ENTRY_H(					"()"),
			FUNC_ENTRY_H(safeSet,			"(source: wx.AuiPaneInfo)"),

			FUNC_ENTRY_H(bottom,			"(): this"),
			FUNC_ENTRY_H(bottomDockable,	"(dockable=true): this"),
			FUNC_ENTRY_H(caption,			"(cap: string): this"),
			FUNC_ENTRY_H(captionVisible,	"(visible=true): this"),
			FUNC_ENTRY_H(closeButton,		"(visible=true): this"),
			FUNC_ENTRY_H(defaultPane,		"(): this"),
			FUNC_ENTRY_H(destroyOnClose,	"(flag=true): this"),
			FUNC_ENTRY_H(direction,			"(dir: wx.DIR): this"),
			FUNC_ENTRY_H(dock,				"(): this"),
			FUNC_ENTRY_H(dockFixed,			"(flag=true): this"),
			FUNC_ENTRY_H(dockable,			"(flag=true): this"),
			FUNC_ENTRY_H(fixed,				"(): this"),
			FUNC_ENTRY_H(float,				"(): this"),
			FUNC_ENTRY_H(floatable,			"(flag=true): this"),
			FUNC_ENTRY_H(gripper,			"(visible=true): this"),
			FUNC_ENTRY_H(gripperTop,		"(atTop=true): this"),
			FUNC_ENTRY_H(hide,				"(): this"),
			FUNC_ENTRY_H(icon,				"(icon: wx.Bitmap): this"),
			FUNC_ENTRY_H(layer,				"(layer: int): this"),
			FUNC_ENTRY_H(left,				"(): this"),
			FUNC_ENTRY_H(leftDockable,		"(flag=true): this"),
			FUNC_ENTRY_H(maximizeButton,	"(visible=true): this"),
			FUNC_ENTRY_H(minimizeButton,	"(visible=true): this"),
			FUNC_ENTRY_H(movable,			"(flag=true): this"),
			FUNC_ENTRY_H(name,				"(name: string): this"),
			FUNC_ENTRY_H(paneBorder,		"(visible=true): this"),
			FUNC_ENTRY_H(pinButton,			"(visible=true): this"),
			FUNC_ENTRY_H(position,			"(pos: int): this"),
			FUNC_ENTRY_H(resizable,			"(flag=true): this"),
			FUNC_ENTRY_H(right,				"(): this"),
			FUNC_ENTRY_H(rightDockable,		"(flag=true): this"),
			FUNC_ENTRY_H(row,				"(row: int): this"),
			FUNC_ENTRY_H(show,				"(show=true): this"),
			FUNC_ENTRY_H(toolbarPane,		"(): this"),
			FUNC_ENTRY_H(top,				"(): this"),
			FUNC_ENTRY_H(topDockable,		"(flag=true): this"),
			FUNC_ENTRY_H(window,			"(w: wx.Window): this"),
			FUNC_ENTRY_H(bestSize,			"(size: wx.Size): this"),
			FUNC_ENTRY_H(center,			"(): this"),
			FUNC_ENTRY_H(centerPane,		"(): this"),
			FUNC_ENTRY_H(floatingPosition,	"(pos: wx.Point): this"),
			FUNC_ENTRY_H(floatingSize,		"(size: wx.Size): this"),
			FUNC_ENTRY_H(maxSize,			"(size: wx.Size): this"),
			FUNC_ENTRY_H(minSize,			"(size: wx.Size): this"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ new (self(v)) wxAuiPaneInfo(); return SQ_OK; }

	inline static SQRESULT PushSelf(HSQUIRRELVM v) 
	{ 
		sq_push(v, 1); 
		return 1; 
	}

	NB_FUNC(safeSet)					{ self(v)->SafeSet(*get<wxAuiPaneInfo>(v, 2)); return 0; }

	NB_FUNC(bottom)						{ self(v)->Bottom(); return PushSelf(v); }
	NB_FUNC(bottomDockable)				{ self(v)->BottomDockable(optBool(v, 2, true)); return PushSelf(v); }
	NB_FUNC(caption)					{ self(v)->Caption(getWxString(v, 2)); return PushSelf(v); }
	NB_FUNC(captionVisible)				{ self(v)->CaptionVisible(optBool(v, 2, true)); return PushSelf(v); }
	NB_FUNC(closeButton)				{ self(v)->CloseButton(optBool(v, 2, true)); return PushSelf(v); }
	NB_FUNC(defaultPane)				{ self(v)->DefaultPane(); return PushSelf(v); }
	NB_FUNC(destroyOnClose)				{ self(v)->DestroyOnClose(optBool(v, 2, true)); return PushSelf(v); }
	NB_FUNC(direction)					{ self(v)->Direction(getInt(v, 2)); return PushSelf(v); }
	NB_FUNC(dock)						{ self(v)->Dock(); return PushSelf(v); }
	NB_FUNC(dockFixed)					{ self(v)->DockFixed(optBool(v, 2, true)); return PushSelf(v); }
	NB_FUNC(dockable)					{ self(v)->Dockable(optBool(v, 2, true)); return PushSelf(v); }
	NB_FUNC(fixed)						{ self(v)->Fixed(); return PushSelf(v); }
	NB_FUNC(float)						{ self(v)->Float(); return PushSelf(v); }
	NB_FUNC(floatable)					{ self(v)->Floatable(optBool(v, 2, true)); return PushSelf(v); }
	NB_FUNC(gripper)					{ self(v)->Gripper(optBool(v, 2, true)); return PushSelf(v); }
	NB_FUNC(gripperTop)					{ self(v)->GripperTop(optBool(v, 2, true)); return PushSelf(v); }
	NB_FUNC(hide)						{ self(v)->Hide(); return PushSelf(v); }
	NB_FUNC(icon)						{ self(v)->Icon(*get<wxBitmap>(v, 2)); return PushSelf(v); }
	NB_FUNC(layer)						{ self(v)->Layer(getInt(v, 2)); return PushSelf(v); }
	NB_FUNC(left)						{ self(v)->Left(); return PushSelf(v); }
	NB_FUNC(leftDockable)				{ self(v)->LeftDockable(optBool(v, 2, true)); return PushSelf(v); }
	NB_FUNC(maximizeButton)				{ self(v)->MaximizeButton(optBool(v, 2, true)); return PushSelf(v); }
	NB_FUNC(minimizeButton)				{ self(v)->MinimizeButton(optBool(v, 2, true)); return PushSelf(v); }
	NB_FUNC(movable)					{ self(v)->Movable(optBool(v, 2, true)); return PushSelf(v); }
	NB_FUNC(name)						{ self(v)->Name(getWxString(v, 2)); return PushSelf(v); }
	NB_FUNC(paneBorder)					{ self(v)->PaneBorder(optBool(v, 2, true)); return PushSelf(v); }
	NB_FUNC(pinButton)					{ self(v)->PinButton(optBool(v, 2, true)); return PushSelf(v); }
	NB_FUNC(position)					{ self(v)->Position(getInt(v, 2)); return PushSelf(v); }
	NB_FUNC(resizable)					{ self(v)->Resizable(optBool(v, 2, true)); return PushSelf(v); }
	NB_FUNC(right)						{ self(v)->Right(); return PushSelf(v); }
	NB_FUNC(rightDockable)				{ self(v)->RightDockable(optBool(v, 2, true)); return PushSelf(v); }
	NB_FUNC(row)						{ self(v)->Row(getInt(v, 2)); return PushSelf(v); }
	NB_FUNC(show)						{ self(v)->Show(optBool(v, 2, true)); return PushSelf(v); }
	NB_FUNC(toolbarPane)				{ self(v)->ToolbarPane(); return PushSelf(v); }
	NB_FUNC(top)						{ self(v)->Top(); return PushSelf(v); }
	NB_FUNC(topDockable)				{ self(v)->TopDockable(optBool(v, 2, true)); return PushSelf(v); }
	NB_FUNC(window)						{ self(v)->Window(get<wxWindow>(v, 2)); return PushSelf(v); }
	NB_FUNC(bestSize)					{ self(v)->BestSize(*get<wxSize>(v, 2)); return PushSelf(v); }
	NB_FUNC(center)						{ self(v)->Center(); return PushSelf(v); }
	NB_FUNC(centerPane)					{ self(v)->CenterPane(); return PushSelf(v); }
	NB_FUNC(floatingPosition)			{ self(v)->FloatingPosition(*get<wxPoint>(v, 2)); return PushSelf(v); }
	NB_FUNC(floatingSize)				{ self(v)->FloatingSize(*get<wxSize>(v, 2)); return PushSelf(v); }
	NB_FUNC(maxSize)					{ self(v)->MaxSize(*get<wxSize>(v, 2)); return PushSelf(v); }
	NB_FUNC(minSize)					{ self(v)->MinSize(*get<wxSize>(v, 2)); return PushSelf(v); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, AuiManager, wxEvtHandler);

class NB_WxAuiManager : TNitClass<wxAuiManager>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(managedWindow),
			PROP_ENTRY	(flags),
			PROP_ENTRY	(dockSizeConstraint),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(					"(managed: wx.Window=null, flags=STYLE.DEFAULT)"),
			FUNC_ENTRY_H(getManager,		"[class] (window: wx.Window): wx.AuiManager"),
			FUNC_ENTRY_H(addPane,			"(window: wx.Window, paneInfo: wx.AuiPaneInfo): bool"
			"\n"							"(window: wx.Window, direction=wx.DIR.LEFT, caption=\"\"): bool"
			"\n"							"(window: wx.Window, paneInfo: wx.AuiPaneInfo, dropPos: wx.Point): bool"),
			FUNC_ENTRY_H(insertPane,		"(window: wx.Window, insertLocation: wx.AuiPaneInfo, level=INSERT.PANE): bool"),
			FUNC_ENTRY_H(detachPane,		"(window: wx.Window): bool"),
			FUNC_ENTRY_H(getPane,			"(window: wx.Window): wx.AuiPaneInfo"
			"\n"							"(name: string): wx.AuiPaneInfo"),
			FUNC_ENTRY_H(getAllPanes,		"(): wxAuiPaneInfo[]"),
			FUNC_ENTRY_H(update,			"()"),
			FUNC_ENTRY_H(loadPerspective,	"(perspective: string, update=true): bool"),
			FUNC_ENTRY_H(loadPaneInfo,		"(panePart: string): wx.AuiPaneInfo"),
			FUNC_ENTRY_H(savePerspective,	"(): string"),
			FUNC_ENTRY_H(savePaneInfo,		"(pane: wx.AuiPaneInfo): string"),
			FUNC_ENTRY_H(showHint,			"(rect: wx.Rect)"),
			FUNC_ENTRY_H(hideHint,			"()"),
			FUNC_ENTRY_H(unInit,			"()"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "ALLOW_FLOATING",			(int)wxAUI_MGR_ALLOW_FLOATING);
		newSlot(v, -1, "ALLOW_ACTIVE_PANE",			(int)wxAUI_MGR_ALLOW_ACTIVE_PANE);
		newSlot(v, -1, "TRANSPARENT_DRAG",			(int)wxAUI_MGR_TRANSPARENT_DRAG);
		newSlot(v, -1, "TRANSPARENT_HINT",			(int)wxAUI_MGR_TRANSPARENT_HINT);
		newSlot(v, -1, "VENETIAN_BLINDS_HINT",		(int)wxAUI_MGR_VENETIAN_BLINDS_HINT);
		newSlot(v, -1, "RECTANGLE_HINT",			(int)wxAUI_MGR_RECTANGLE_HINT);
		newSlot(v, -1, "HINT_FADE",					(int)wxAUI_MGR_HINT_FADE);
		newSlot(v, -1, "NO_VENETIAN_BLINDS_FADE",	(int)wxAUI_MGR_NO_VENETIAN_BLINDS_FADE);
		newSlot(v, -1, "LIVE_RESIZE",				(int)wxAUI_MGR_LIVE_RESIZE);
		newSlot(v, -1, "DEFAULT",					(int)wxAUI_MGR_DEFAULT);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "PANE_BUTTON",				(int)wxEVT_AUI_PANE_BUTTON);
		newSlot(v, -1, "PANE_CLOSE",				(int)wxEVT_AUI_PANE_CLOSE);
		newSlot(v, -1, "PANE_MAXIMIZE",				(int)wxEVT_AUI_PANE_MAXIMIZE);
		newSlot(v, -1, "PANE_RESTORE",				(int)wxEVT_AUI_PANE_RESTORE);
		newSlot(v, -1, "PANE_ACTIVATED",			(int)wxEVT_AUI_PANE_ACTIVATED);
		newSlot(v, -1, "RENDER",					(int)wxEVT_AUI_RENDER);
		newSlot(v, -1, "FIND_MANAGER",				(int)wxEVT_AUI_FIND_MANAGER);
		sq_poptop(v);

		addStaticTable(v, "DOCK");
		newSlot(v, -1, "NONE",						(int)wxAUI_DOCK_NONE);
		newSlot(v, -1, "TOP",						(int)wxAUI_DOCK_TOP);
		newSlot(v, -1, "RIGHT",						(int)wxAUI_DOCK_RIGHT);
		newSlot(v, -1, "BOTTOM",					(int)wxAUI_DOCK_BOTTOM);
		newSlot(v, -1, "LEFT",						(int)wxAUI_DOCK_LEFT);
		newSlot(v, -1, "CENTER",					(int)wxAUI_DOCK_CENTER);
		sq_poptop(v);

		addStaticTable(v, "INSERT");
		newSlot(v, -1, "PANE",						(int)wxAUI_INSERT_PANE);
		newSlot(v, -1, "ROW",						(int)wxAUI_INSERT_ROW);
		newSlot(v, -1, "DOCK",						(int)wxAUI_INSERT_DOCK);
		sq_poptop(v);

		addStaticTable(v, "GRADIENT");
		newSlot(v, -1, "NONE",						(int)wxAUI_GRADIENT_NONE);
		newSlot(v, -1, "VERTICAL",					(int)wxAUI_GRADIENT_VERTICAL);
		newSlot(v, -1, "HORIZONTAL",				(int)wxAUI_GRADIENT_HORIZONTAL);
		sq_poptop(v);

		addStaticTable(v, "BUTTON");
		newSlot(v, -1, "CLOSE",						(int)wxAUI_BUTTON_CLOSE);
		newSlot(v, -1, "MAXIMIZE_RESTORE",			(int)wxAUI_BUTTON_MAXIMIZE_RESTORE);
		newSlot(v, -1, "MINIMIZE",					(int)wxAUI_BUTTON_MINIMIZE);
		newSlot(v, -1, "PIN",						(int)wxAUI_BUTTON_PIN);
		newSlot(v, -1, "OPTIONS",					(int)wxAUI_BUTTON_OPTIONS);
		newSlot(v, -1, "WINDOWLIST",				(int)wxAUI_BUTTON_WINDOWLIST);
		newSlot(v, -1, "LEFT",						(int)wxAUI_BUTTON_LEFT);
		newSlot(v, -1, "RIGHT",						(int)wxAUI_BUTTON_RIGHT);
		newSlot(v, -1, "UP",						(int)wxAUI_BUTTON_UP);
		newSlot(v, -1, "DOWN",						(int)wxAUI_BUTTON_DOWN);
		newSlot(v, -1, "CUSTOM1",					(int)wxAUI_BUTTON_CUSTOM1);
		newSlot(v, -1, "CUSTOM2",					(int)wxAUI_BUTTON_CUSTOM2);
		newSlot(v, -1, "CUSTOM3",					(int)wxAUI_BUTTON_CUSTOM3);
		sq_poptop(v);

		addStaticTable(v, "BUTTON_STATE");
		newSlot(v, -1, "NORMAL",					(int)wxAUI_BUTTON_STATE_NORMAL);
		newSlot(v, -1, "HOVER",						(int)wxAUI_BUTTON_STATE_HOVER);
		newSlot(v, -1, "PRESSED",					(int)wxAUI_BUTTON_STATE_PRESSED);
		newSlot(v, -1, "DISABLED",					(int)wxAUI_BUTTON_STATE_DISABLED);
		newSlot(v, -1, "HIDDEN",					(int)wxAUI_BUTTON_STATE_HIDDEN);
		newSlot(v, -1, "CHECKED",					(int)wxAUI_BUTTON_STATE_CHECKED);
		sq_poptop(v);
	}

	NB_PROP_GET(managedWindow)			{ return push(v, self(v)->GetManagedWindow()); }
	NB_PROP_GET(flags)					{ return push(v, self(v)->GetFlags()); }
	NB_PROP_GET(dockSizeConstraint)		{ double w, h; self(v)->GetDockSizeConstraint(&w, &h); return push(v, Vector2((float)w, (float)h)); }
	
	NB_PROP_SET(managedWindow)			{ self(v)->SetManagedWindow(get<wxWindow>(v, 2)); return 0; }
	NB_PROP_SET(flags)					{ self(v)->SetFlags(getInt(v, 2)); return 0; }
	NB_PROP_SET(dockSizeConstraint)		{ Vector2 c = *get<Vector2>(v, 2); self(v)->SetDockSizeConstraint(c.x, c.y); return 0; }

	NB_CONS()							{ setSelf(v, new wxAuiManager(opt<wxWindow>(v, 2, NULL), optInt(v, 3, wxAUI_MGR_DEFAULT))); return SQ_OK; }	

	NB_FUNC(getManager)					{ return push(v, type::GetManager(get<wxWindow>(v, 2))); }

	NB_FUNC(addPane)
	{
		if (isNone(v, 3))
			return push(v, self(v)->AddPane(get<wxWindow>(v, 2)));
		else if (isInt(v, 3))
			return push(v, self(v)->AddPane(get<wxWindow>(v, 2), optInt(v, 3, wxLEFT), optWxString(v, 4, "")));
		else if (isNone(v, 4))
			return push(v, self(v)->AddPane(get<wxWindow>(v, 2), *get<wxAuiPaneInfo>(v, 3)));
		else
			return push(v, self(v)->AddPane(get<wxWindow>(v, 2), *get<wxAuiPaneInfo>(v, 3), *get<wxPoint>(v, 4)));
	}

	NB_FUNC(insertPane)					{ return push(v, self(v)->InsertPane(get<wxWindow>(v, 2), *get<wxAuiPaneInfo>(v, 3), optInt(v, 4, wxAUI_INSERT_PANE))); }
	NB_FUNC(detachPane)					{ return push(v, self(v)->DetachPane(get<wxWindow>(v, 2))); }

	NB_FUNC(getPane)
	{
		if (isString(v, 2))
			return push(v, self(v)->GetPane(getWxString(v, 2)));
		else
			return push(v, self(v)->GetPane(get<wxWindow>(v, 2)));
	}

	NB_FUNC(update)						{ self(v)->Update(); return 0; }
	NB_FUNC(loadPerspective)			{ return push(v, self(v)->LoadPerspective(getWxString(v, 2), optBool(v, 3, true))); }
	NB_FUNC(loadPaneInfo)				{ wxAuiPaneInfo i; self(v)->LoadPaneInfo(getWxString(v, 2), i); return push(v, i); }
	NB_FUNC(savePerspective)			{ return push(v, self(v)->SavePerspective()); }
	NB_FUNC(savePaneInfo)				{ return push(v, self(v)->SavePaneInfo(*get<wxAuiPaneInfo>(v, 2))); }
	NB_FUNC(showHint)					{ self(v)->ShowHint(*get<wxRect>(v, 2)); return 0; }
	NB_FUNC(hideHint)					{ self(v)->HideHint(); return 0; }
	NB_FUNC(unInit)						{ self(v)->UnInit(); return 0; }

	NB_FUNC(getAllPanes)
	{
		sq_newarray(v, 0);
		wxAuiPaneInfoArray& panes = self(v)->GetAllPanes();
		for (uint i=0; i<panes.size(); ++i)
			arrayAppend(v, -1, panes[i]);
		return 1;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, AuiNotebook, wxBookCtrlBase);

class NB_WxAuiNotebook : TNitClass<wxAuiNotebook>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(tabCtrlHeight),
			PROP_ENTRY_R(auiManager),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(					"(parent: Window=null, id=wx.ID.ANY, pos=null, size=null, style=STYLE.DEFAULT)"),
			FUNC_ENTRY_H(addPage,			"(page: wx.Window, caption: string, select=false, bitmap=null): bool"
			"\n"							"(page: wx.Window, text: string, select: bool, imageid: int): bool"),
			FUNC_ENTRY_H(insertPage,		"(page_idx: int, page: wx.Window, caption: string, select=false, bitmap=null): bool"),
			FUNC_ENTRY_H(getPageBitmap,		"(page: int): wx.Bitmap"),
			FUNC_ENTRY_H(getPageIndex,		"(page: wx.Window): int"),
			FUNC_ENTRY_H(getPageText,		"(page: int): string"),
			FUNC_ENTRY_H(getPageToolTip,	"(page: int): string"),
			FUNC_ENTRY_H(setPageBitmap,		"(page: int, bitmap: wx.Bitmap): bool"),
			FUNC_ENTRY_H(setPageImage,		"(page: int, imageid: int): bool"),
			FUNC_ENTRY_H(setPageText,		"(page: int, text: string): bool"),
			FUNC_ENTRY_H(setPageToolTip,	"(page: int, text: string): bool"),
			FUNC_ENTRY_H(setUniformBitmapSize, "(size: wx.Size)"),
			FUNC_ENTRY_H(showWindowMenu,	"(): bool"),
			FUNC_ENTRY_H(split,				"(page: int, dir: wx.DIR)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "DEFAULT",				(int)wxAUI_NB_DEFAULT_STYLE);
		newSlot(v, -1, "TAB_SPLIT",				(int)wxAUI_NB_TAB_SPLIT);
		newSlot(v, -1, "TAB_MOVE",				(int)wxAUI_NB_TAB_MOVE);
		newSlot(v, -1, "TAB_EXTERNAL_MOVE",		(int)wxAUI_NB_TAB_EXTERNAL_MOVE);
		newSlot(v, -1, "TAB_FIXED_WIDTH",		(int)wxAUI_NB_TAB_FIXED_WIDTH);
		newSlot(v, -1, "SCROLL_BUTTONS",		(int)wxAUI_NB_SCROLL_BUTTONS);
		newSlot(v, -1, "WINDOWLIST_BUTTON",		(int)wxAUI_NB_WINDOWLIST_BUTTON);
		newSlot(v, -1, "CLOSE_BUTTON",			(int)wxAUI_NB_CLOSE_BUTTON);
		newSlot(v, -1, "CLOSE_ON_ACTIVE_TAB",	(int)wxAUI_NB_CLOSE_ON_ACTIVE_TAB);
		newSlot(v, -1, "CLOSE_ON_ALL_TABS",		(int)wxAUI_NB_CLOSE_ON_ALL_TABS);
		newSlot(v, -1, "MIDDLE_CLICK_CLOSE",	(int)wxAUI_NB_MIDDLE_CLICK_CLOSE);
		newSlot(v, -1, "TOP",					(int)wxAUI_NB_TOP);
		newSlot(v, -1, "BOTTOM",				(int)wxAUI_NB_BOTTOM);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "PAGE_CLOSE",			(int)wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSE);
		newSlot(v, -1, "PAGE_CLOSED",			(int)wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSED);
		newSlot(v, -1, "PAGE_CHANGED",			(int)wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED);
		newSlot(v, -1, "PAGE_CHANGING",			(int)wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGING);
		newSlot(v, -1, "BUTTON",				(int)wxEVT_COMMAND_AUINOTEBOOK_BUTTON);
		newSlot(v, -1, "BEGIN_DRAG",			(int)wxEVT_COMMAND_AUINOTEBOOK_BEGIN_DRAG);
		newSlot(v, -1, "END_DRAG",				(int)wxEVT_COMMAND_AUINOTEBOOK_END_DRAG);
		newSlot(v, -1, "DRAG_MOTION",			(int)wxEVT_COMMAND_AUINOTEBOOK_DRAG_MOTION);
		newSlot(v, -1, "ALLOW_DND",				(int)wxEVT_COMMAND_AUINOTEBOOK_ALLOW_DND);
		newSlot(v, -1, "DRAG_DONE",				(int)wxEVT_COMMAND_AUINOTEBOOK_DRAG_DONE);
		newSlot(v, -1, "TAB_MIDDLE_DOWN",		(int)wxEVT_COMMAND_AUINOTEBOOK_TAB_MIDDLE_DOWN);
		newSlot(v, -1, "TAB_MIDDLE_UP",			(int)wxEVT_COMMAND_AUINOTEBOOK_TAB_MIDDLE_UP);
		newSlot(v, -1, "TAB_RIGHT_DOWN",		(int)wxEVT_COMMAND_AUINOTEBOOK_TAB_RIGHT_DOWN);
		newSlot(v, -1, "TAB_RIGHT_UP",			(int)wxEVT_COMMAND_AUINOTEBOOK_TAB_RIGHT_UP);
		newSlot(v, -1, "BG_DCLICK",				(int)wxEVT_COMMAND_AUINOTEBOOK_BG_DCLICK);
		sq_poptop(v);
	}

	NB_PROP_GET(tabCtrlHeight)			{ return push(v, self(v)->GetTabCtrlHeight()); }
	NB_PROP_GET(auiManager)				{ return push(v, const_cast<wxAuiManager*>(&self(v)->GetAuiManager())); }

	NB_PROP_SET(tabCtrlHeight)			{ self(v)->SetTabCtrlHeight(getInt(v, 2)); return 0; }

	NB_CONS()
	{
		wxControl* control = new wxAuiNotebook(
			opt<wxWindow>	(v, 2, NULL),
			optInt			(v, 3, wxID_ANY),
			*opt<wxPoint>	(v, 4, wxDefaultPosition),
			*opt<wxSize>	(v, 5, wxDefaultSize),
			optInt			(v, 6, wxAUI_NB_DEFAULT_STYLE)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}

	NB_FUNC(addPage)
	{
		if (isNone(v, 5) || isNull(v, 5) || is<wxBitmap>(v, 5))
			return push(v, self(v)->AddPage(get<wxWindow>(v, 2), getWxString(v, 3), optBool(v, 4, false), *opt<wxBitmap>(v, 5, wxNullBitmap)));
		else
			return push(v, self(v)->AddPage(get<wxWindow>(v, 2), getWxString(v, 3), getBool(v, 4), getInt(v, 5)));
	}

	NB_FUNC(insertPage)					{ return push(v, self(v)->InsertPage(getInt(v, 2), get<wxWindow>(v, 3), getWxString(v, 4), optBool(v, 5, false), *opt<wxBitmap>(v, 6, wxNullBitmap))); }
	NB_FUNC(getPageBitmap)				{ return push(v, self(v)->GetPageBitmap(getInt(v, 2))); }
	NB_FUNC(getPageIndex)				{ return push(v, self(v)->GetPageIndex(get<wxWindow>(v, 2))); }
	NB_FUNC(getPageText)				{ return push(v, self(v)->GetPageText(getInt(v, 2))); }
	NB_FUNC(getPageToolTip)				{ return push(v, self(v)->GetPageToolTip(getInt(v, 2))); }
	NB_FUNC(setPageBitmap)				{ return push(v, self(v)->SetPageBitmap(getInt(v, 2), *get<wxBitmap>(v, 3))); }
	NB_FUNC(setPageImage)				{ return push(v, self(v)->SetPageImage(getInt(v, 2), getInt(v, 3))); }
	NB_FUNC(setPageText)				{ return push(v, self(v)->SetPageText(getInt(v, 2), getWxString(v, 3))); }
	NB_FUNC(setPageToolTip)				{ return push(v, self(v)->SetPageToolTip(getInt(v, 2), getWxString(v, 3))); }
	NB_FUNC(setUniformBitmapSize)		{ self(v)->SetUniformBitmapSize(*get<wxSize>(v, 2)); return 0; }
	NB_FUNC(showWindowMenu)				{ return push(v, self(v)->ShowWindowMenu()); }
	NB_FUNC(split)						{ self(v)->Split(getInt(v, 2), getInt(v, 3)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, AuiToolBar, wxControl);

class NB_WxAuiToolBar : TNitClass<wxAuiToolBar>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(windowStyleFlag),
			PROP_ENTRY_R(toolCount),
			PROP_ENTRY	(toolSeparation),
			PROP_ENTRY_R(toolBarFits),
			PROP_ENTRY	(toolBitmapSize),
			PROP_ENTRY	(overflowVisible),
			PROP_ENTRY	(gripperVisible),
			PROP_ENTRY	(toolBorderPadding),
			PROP_ENTRY	(toolTextOrientation),
			PROP_ENTRY	(toolPacking),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(					"(parent: Window=null, id=wx.ID.ANY, pos=null, size=null, style=STYLE.DEFAULT)"),
			FUNC_ENTRY_H(addTool,			"(tool_id: int, label: string, bitmap: wx.Bitmap, shortHelp=\"\", kind=wx.ITEM.NORMAL): wx.AuiToolBarItem"
			"\n"							"(tool_id: int, label: string, bitmap, disabled: wx.Bitmap, kind: wx.ITEM, shortHelp: string, longHelp: string): wx.AuiToolBarItem"),
			FUNC_ENTRY_H(addLabel,			"(tool_id: int, label=\"\", width=-1): wx.AuiToolBarItem"),
			FUNC_ENTRY_H(addControl,		"(control: wx.Control, label=\"\"): wx.AuiToolBarItem"),
			FUNC_ENTRY_H(addSeparator,		"(): wx.AuiToolBarItem"),
			FUNC_ENTRY_H(addSpacer,			"(pixels: int): wx.AuiToolBarItem"),
			FUNC_ENTRY_H(addStretchSpacer,	"(proportion=1): wx.AuiToolBarItem"),
			FUNC_ENTRY_H(realize,			"(): bool"),
			FUNC_ENTRY_H(findControl,		"(window_id: int): wx.Control"),
			FUNC_ENTRY_H(findTool,			"(tool_id: int): wx.AuiToolBarItem"),
			FUNC_ENTRY_H(findToolByIndex,	"(idx: int): wx.AuiToolBarItem"),
			FUNC_ENTRY_H(clearTools,		"()"),
			FUNC_ENTRY_H(clear,				"()"),
			FUNC_ENTRY_H(deleteTool,		"(tool_id: int): bool"),
			FUNC_ENTRY_H(deleteByIndex,		"(idx: int): bool"),
			FUNC_ENTRY_H(getToolIndex,		"(tool_id: int): int"),
			FUNC_ENTRY_H(isToolFits,		"(tool_id: int): bool"),
			FUNC_ENTRY_H(getToolRect,		"(tool_id: int): wx.Rect"),
			FUNC_ENTRY_H(getToolFitsByIndex,"(idx: int): bool"),
			FUNC_ENTRY_H(setMargins,		"(x, y: int)"
			"\n"							"(left, right, top, bottom: int)"),
			FUNC_ENTRY_H(toggleTool,		"(tool_id: int, state: bool)"),
			FUNC_ENTRY_H(isToolToggled,		"(tool_id: int): bool"),
			FUNC_ENTRY_H(enableTool,		"(tool_id, state: bool)"),
			FUNC_ENTRY_H(isToolEnabled,		"(tool_id: int): bool"),
			FUNC_ENTRY_H(setToolDropDown,	"(tool_id, dropdown: bool)"),
			FUNC_ENTRY_H(hasToolDropDown,	"(tool_id): bool"),
			FUNC_ENTRY_H(setToolProportion,	"(tool_id, proportion: int)"),
			FUNC_ENTRY_H(getToolProportion,	"(tool_id: int): int"),
			FUNC_ENTRY_H(setToolSticky,		"(tool_id: int, sticky: bool)"),
			FUNC_ENTRY_H(isToolSticky,		"(tool_id: int): bool"),
			FUNC_ENTRY_H(setToolLabel,		"(tool_id: int, label: string)"),
			FUNC_ENTRY_H(getToolLabel,		"(tool_id: int): string"),
			FUNC_ENTRY_H(setToolBitmap,		"(tool_id: int, bitmap: wx.Bitmap)"),
			FUNC_ENTRY_H(getToolBitmap,		"(tool_id: int): wx.Bitmap"),
			FUNC_ENTRY_H(setToolShortHelp,	"(tool_id: int, help: string)"),
			FUNC_ENTRY_H(getToolShortHelp,	"(tool_id: int): string"),
			FUNC_ENTRY_H(setToolLongHelp,	"(tool_id: int, help: string)"),
			FUNC_ENTRY_H(getToolLongHelp,	"(tool_id: int): string"),
			FUNC_ENTRY_H(setCustomOverflowItems, "(prepend: wx.AuiToolBarItem[], append: wx.AuiToolBarItem[])"),
			FUNC_ENTRY_H(getHintSize,		"(dock_direction: wx.AuiManager.DOCK)"),
			FUNC_ENTRY_H(isPaneValid,		"(pane: wx.AuiPaneInfo): bool"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "DEFAULT",				(int)wxAUI_TB_DEFAULT_STYLE);
		newSlot(v, -1, "TEXT",					(int)wxAUI_TB_TEXT);
		newSlot(v, -1, "NO_TOOLTIPS",			(int)wxAUI_TB_NO_TOOLTIPS);
		newSlot(v, -1, "NO_AUTORESIZE",			(int)wxAUI_TB_NO_AUTORESIZE);
		newSlot(v, -1, "GRIPPER",				(int)wxAUI_TB_GRIPPER);
		newSlot(v, -1, "OVERFLOW",				(int)wxAUI_TB_OVERFLOW);
		newSlot(v, -1, "VERTICAL",				(int)wxAUI_TB_VERTICAL);
		newSlot(v, -1, "HORZ_LAYOUT",			(int)wxAUI_TB_HORZ_LAYOUT);
		newSlot(v, -1, "HORIZONTAL",			(int)wxAUI_TB_HORIZONTAL);
		newSlot(v, -1, "HORZ_TEXT",				(int)wxAUI_TB_HORZ_TEXT);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "TOOL",					(int)wxEVT_COMMAND_TOOL_CLICKED);
		newSlot(v, -1, "MENU",					(int)wxEVT_COMMAND_MENU_SELECTED);
		newSlot(v, -1, "TOOL_RCLICKED",			(int)wxEVT_COMMAND_TOOL_RCLICKED);
		newSlot(v, -1, "TOOL_ENTER",			(int)wxEVT_COMMAND_TOOL_ENTER);
		newSlot(v, -1, "TOOL_DROPDOWN",			(int)wxEVT_COMMAND_AUITOOLBAR_TOOL_DROPDOWN);
		newSlot(v, -1, "OVERFLOW_CLICK",		(int)wxEVT_COMMAND_AUITOOLBAR_OVERFLOW_CLICK);
		newSlot(v, -1, "RIGHT_CLICK",			(int)wxEVT_COMMAND_AUITOOLBAR_RIGHT_CLICK);
		newSlot(v, -1, "MIDDLE_CLICK",			(int)wxEVT_COMMAND_AUITOOLBAR_MIDDLE_CLICK);
		newSlot(v, -1, "BEGIN_DRAG",			(int)wxEVT_COMMAND_AUITOOLBAR_BEGIN_DRAG);
		sq_poptop(v);
	}

	NB_PROP_GET(windowStyleFlag)		{ return push(v, self(v)->GetWindowStyleFlag()); }
	NB_PROP_GET(toolCount)				{ return push(v, self(v)->GetToolCount()); }
	NB_PROP_GET(toolSeparation)			{ return push(v, self(v)->GetToolSeparation()); }
	NB_PROP_GET(toolBarFits)			{ return push(v, self(v)->GetToolBarFits()); }
	NB_PROP_GET(toolBitmapSize)			{ return push(v, self(v)->GetToolBitmapSize()); }
	NB_PROP_GET(overflowVisible)		{ return push(v, self(v)->GetOverflowVisible()); }
	NB_PROP_GET(gripperVisible)			{ return push(v, self(v)->GetGripperVisible()); }
	NB_PROP_GET(toolBorderPadding)		{ return push(v, self(v)->GetToolBorderPadding()); }
	NB_PROP_GET(toolTextOrientation)	{ return push(v, self(v)->GetToolTextOrientation()); }
	NB_PROP_GET(toolPacking)			{ return push(v, self(v)->GetToolPacking()); }

	NB_PROP_SET(windowStyleFlag)		{ self(v)->SetWindowStyleFlag(getInt(v, 2)); return 0; }
	NB_PROP_SET(toolSeparation)			{ self(v)->SetToolSeparation(getInt(v, 2)); return 0; }
	NB_PROP_SET(toolBitmapSize)			{ self(v)->SetToolBitmapSize(*get<wxSize>(v, 2)); return 0; }
	NB_PROP_SET(overflowVisible)		{ self(v)->SetOverflowVisible(getBool(v, 2)); return 0; }
	NB_PROP_SET(gripperVisible)			{ self(v)->SetGripperVisible(getBool(v, 2)); return 0; }
	NB_PROP_SET(toolBorderPadding)		{ self(v)->SetToolBorderPadding(getInt(v, 2)); return 0; }
	NB_PROP_SET(toolTextOrientation)	{ self(v)->SetToolTextOrientation(getInt(v, 2)); return 0; }
	NB_PROP_SET(toolPacking)			{ self(v)->SetToolPacking(getInt(v, 2)); return 0; }

	NB_CONS()
	{
		wxControl* control = new wxAuiToolBar(
			opt<wxWindow>	(v, 2, NULL),
			optInt			(v, 3, wxID_ANY),
			*opt<wxPoint>	(v, 4, wxDefaultPosition),
			*opt<wxSize>	(v, 5, wxDefaultSize),
			optInt			(v, 6, wxAUI_TB_DEFAULT_STYLE)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}

	NB_FUNC(addTool)
	{
		if (!isNone(v, 5) && (is<wxBitmap>(v, 5) || isNull(v, 5)))
			return push(v, *self(v)->AddTool(getInt(v, 2), getWxString(v, 3), *get<wxBitmap>(v, 4), *opt<wxBitmap>(v, 5, wxNullBitmap), (wxItemKind)getInt(v, 6), getWxString(v, 7), getWxString(v, 8), NULL));
		else
			return push(v, *self(v)->AddTool(getInt(v, 2), getWxString(v, 3), *get<wxBitmap>(v, 4), getWxString(v, 5), (wxItemKind)getInt(v, 6)));
	}

	NB_FUNC(addLabel)					{ return push(v, *self(v)->AddLabel(getInt(v, 2), optWxString(v, 3, ""), optInt(v, 4, -1))); }
	NB_FUNC(addControl)					{ return push(v, *self(v)->AddControl(get<wxControl>(v, 2), optWxString(v, 3, ""))); }
	NB_FUNC(addSeparator)				{ return push(v, *self(v)->AddSeparator()); }
	NB_FUNC(addSpacer)					{ return push(v, *self(v)->AddSpacer(getInt(v, 2))); }
	NB_FUNC(addStretchSpacer)			{ return push(v, *self(v)->AddStretchSpacer(optInt(v, 2, 1))); }
	NB_FUNC(realize)					{ return push(v, self(v)->Realize()); }
	NB_FUNC(findControl)				{ return push(v, self(v)->FindControl(getInt(v, 2))); }
	NB_FUNC(findTool)					{ return push(v, *self(v)->FindTool(getInt(v, 2))); }
	NB_FUNC(findToolByPosition)			{ wxPoint& pt = *get<wxPoint>(v, 2); return push(v, *self(v)->FindToolByPosition(pt.x, pt.y)); }
	NB_FUNC(findToolByIndex)			{ return push(v, *self(v)->FindToolByIndex(getInt(v, 2))); }
	NB_FUNC(clearTools)					{ self(v)->ClearTools(); return 0; }
	NB_FUNC(clear)						{ self(v)->Clear(); return 0; }
	NB_FUNC(deleteTool)					{ return push(v, self(v)->DeleteTool(getInt(v, 2))); }
	NB_FUNC(deleteByIndex)				{ return push(v, self(v)->DeleteByIndex(getInt(v, 2))); }
	NB_FUNC(getToolIndex)				{ return push(v, self(v)->GetToolIndex(getInt(v, 2))); }
	NB_FUNC(isToolFits)					{ return push(v, self(v)->GetToolFits(getInt(v, 2))); }
	NB_FUNC(getToolRect)				{ return push(v, self(v)->GetToolRect(getInt(v, 2))); }
	NB_FUNC(getToolFitsByIndex)			{ return push(v, self(v)->GetToolFitsByIndex(getInt(v, 2))); }

	NB_FUNC(setMargins)
	{
		if (!isNone(v, 4))
			self(v)->SetMargins(getInt(v, 2), getInt(v, 3), getInt(v, 4), getInt(v, 5));
		else
			self(v)->SetMargins(getInt(v, 2), getInt(v, 3));
		return 0;
	}

	NB_FUNC(isToolToggled)				{ return push(v, self(v)->GetToolToggled(getInt(v, 2))); }
	NB_FUNC(isToolEnabled)				{ return push(v, self(v)->GetToolEnabled(getInt(v, 2))); }
	NB_FUNC(hasToolDropDown)			{ return push(v, self(v)->GetToolDropDown(getInt(v, 2))); }
	NB_FUNC(getToolProportion)			{ return push(v, self(v)->GetToolProportion(getInt(v, 2))); }
	NB_FUNC(isToolSticky)				{ return push(v, self(v)->GetToolSticky(getInt(v, 2))); }
	NB_FUNC(getToolLabel)				{ return push(v, self(v)->GetToolLabel(getInt(v, 2))); }
	NB_FUNC(getToolBitmap)				{ return push(v, self(v)->GetToolBitmap(getInt(v, 2))); }
	NB_FUNC(getToolShortHelp)			{ return push(v, self(v)->GetToolShortHelp(getInt(v, 2))); }
	NB_FUNC(getToolLongHelp)			{ return push(v, self(v)->GetToolLongHelp(getInt(v, 2))); }
	NB_FUNC(getHintSize)				{ return push(v, self(v)->GetHintSize(getInt(v, 2))); }
	NB_FUNC(isPaneValid)				{ return push(v, self(v)->IsPaneValid(*get<wxAuiPaneInfo>(v, 2))); }

	NB_FUNC(toggleTool)					{ self(v)->ToggleTool(getInt(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC(enableTool)					{ self(v)->EnableTool(getInt(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC(setToolDropDown)			{ self(v)->SetToolDropDown(getInt(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC(setToolProportion)			{ self(v)->SetToolProportion(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(setToolSticky)				{ self(v)->SetToolSticky(getInt(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC(setToolLabel)				{ self(v)->SetToolLabel(getInt(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(setToolBitmap)				{ self(v)->SetToolBitmap(getInt(v, 2), *opt<wxBitmap>(v, 3, wxNullBitmap)); return 0; }
	NB_FUNC(setToolShortHelp)			{ self(v)->SetToolShortHelp(getInt(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(setToolLongHelp)			{ self(v)->SetToolLongHelp(getInt(v, 2), getWxString(v, 3)); return 0; }

	NB_FUNC(setCustomOverflowItems)
	{
		if (sq_gettype(v, 2) != OT_ARRAY || sq_gettype(v, 3) != OT_ARRAY)
			return sq_throwerror(v, "invalid array");

		wxAuiToolBarItemArray prepend, append;

		for (NitIterator itr(v, 2); itr.hasNext(); itr.next())
			prepend.push_back(*get<wxAuiToolBarItem>(v, itr.valueIndex()));

		for (NitIterator itr(v, 3); itr.hasNext(); itr.next())
			append.push_back(*get<wxAuiToolBarItem>(v, itr.valueIndex()));

		self(v)->SetCustomOverflowItems(prepend, append);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXVALUE(NITWX_API, AuiToolBarItem, NULL);

class NB_WxAuiToolBarItem : TNitClass<wxAuiToolBarItem>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(id),
			PROP_ENTRY	(kind),
			PROP_ENTRY	(state),
			PROP_ENTRY	(sizerItem),
			PROP_ENTRY	(label),
			PROP_ENTRY	(bitmap),
			PROP_ENTRY	(disabledBitmap),
			PROP_ENTRY	(hoverBitmap),
			PROP_ENTRY	(shortHelp),
			PROP_ENTRY	(longHelp),
			PROP_ENTRY	(minSize),
			PROP_ENTRY	(spacerPixels),
			PROP_ENTRY	(proportion),
			PROP_ENTRY	(active),
			PROP_ENTRY	(dropDown),
			PROP_ENTRY	(sticky),
			PROP_ENTRY	(userData),
			PROP_ENTRY	(alignment),
			PROP_ENTRY	(window),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"()"),
			NULL
		};

		bind(v, props, funcs);
	}
	
	NB_PROP_GET(id)						{ return push(v, self(v)->GetId()); }
	NB_PROP_GET(kind)					{ return push(v, self(v)->GetKind()); }
	NB_PROP_GET(state)					{ return push(v, self(v)->GetState()); }
	NB_PROP_GET(sizerItem)				{ return push(v, self(v)->GetSizerItem()); }
	NB_PROP_GET(label)					{ return push(v, self(v)->GetLabel()); }
	NB_PROP_GET(bitmap)					{ return push(v, self(v)->GetBitmap()); }
	NB_PROP_GET(disabledBitmap)			{ return push(v, self(v)->GetDisabledBitmap()); }
	NB_PROP_GET(hoverBitmap)			{ return push(v, self(v)->GetHoverBitmap()); }
	NB_PROP_GET(shortHelp)				{ return push(v, self(v)->GetShortHelp()); }
	NB_PROP_GET(longHelp)				{ return push(v, self(v)->GetLongHelp()); }
	NB_PROP_GET(minSize)				{ return push(v, self(v)->GetMinSize()); }
	NB_PROP_GET(spacerPixels)			{ return push(v, self(v)->GetSpacerPixels()); }
	NB_PROP_GET(proportion)				{ return push(v, self(v)->GetProportion()); }
	NB_PROP_GET(active)					{ return push(v, self(v)->IsActive()); }
	NB_PROP_GET(dropDown)				{ return push(v, self(v)->HasDropDown()); }
	NB_PROP_GET(sticky)					{ return push(v, self(v)->IsSticky()); }
	NB_PROP_GET(userData)				{ return push(v, self(v)->GetUserData()); }
	NB_PROP_GET(alignment)				{ return push(v, self(v)->GetAlignment()); }
	NB_PROP_GET(window)					{ return push(v, self(v)->GetWindow()); }

	NB_PROP_SET(id)						{ self(v)->SetId(getInt(v, 2)); return 0; }
	NB_PROP_SET(kind)					{ self(v)->SetKind(getInt(v, 2)); return 0; }
	NB_PROP_SET(state)					{ self(v)->SetState(getInt(v, 2)); return 0; }
	NB_PROP_SET(sizerItem)				{ self(v)->SetSizerItem(get<wxSizerItem>(v, 2)); return 0; }
	NB_PROP_SET(label)					{ self(v)->SetLabel(getWxString(v, 2)); return 0; }
	NB_PROP_SET(bitmap)					{ self(v)->SetBitmap(*opt<wxBitmap>(v, 2, wxNullBitmap)); return 0; }
	NB_PROP_SET(disabledBitmap)			{ self(v)->SetDisabledBitmap(*opt<wxBitmap>(v, 2, wxNullBitmap)); return 0; }
	NB_PROP_SET(hoverBitmap)			{ self(v)->SetHoverBitmap(*opt<wxBitmap>(v, 2, wxNullBitmap)); return 0; }
	NB_PROP_SET(shortHelp)				{ self(v)->SetShortHelp(getWxString(v, 2)); return 0; }
	NB_PROP_SET(longHelp)				{ self(v)->SetLongHelp(getWxString(v, 2)); return 0; }
	NB_PROP_SET(minSize)				{ self(v)->SetMinSize(*get<wxSize>(v, 2)); return 0; }
	NB_PROP_SET(spacerPixels)			{ self(v)->SetSpacerPixels(getInt(v, 2)); return 0; }
	NB_PROP_SET(proportion)				{ self(v)->SetProportion(getInt(v, 2)); return 0; }
	NB_PROP_SET(active)					{ self(v)->SetActive(getBool(v, 2)); return 0; }
	NB_PROP_SET(dropDown)				{ self(v)->SetHasDropDown(getBool(v, 2)); return 0; }
	NB_PROP_SET(sticky)					{ self(v)->SetSticky(getBool(v, 2)); return 0; }
	NB_PROP_SET(userData)				{ self(v)->SetUserData(getInt(v, 2)); return 0; }
	NB_PROP_SET(alignment)				{ self(v)->SetAlignment(getInt(v, 2)); return 0; }
	NB_PROP_SET(window)					{ self(v)->SetWindow(opt<wxWindow>(v, 2, NULL)); return 0; }

	NB_CONS()							{ new (self(v)) wxAuiToolBarItem(); return SQ_OK; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, RibbonBarEvent, wxNotifyEvent);

class NB_WxRibbonBarEvent : TNitClass<wxRibbonBarEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(page),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(page)					{ return push(v, self(v)->GetPage()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, RibbonToolBarEvent, wxCommandEvent);

class NB_WxRibbonToolBarEvent : TNitClass<wxRibbonToolBarEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(bar),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(popupMenu,		"(menu: wx.Menu): bool"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(bar)					{ return push(v, self(v)->GetBar()); }

	NB_FUNC(popupMenu)					{ return push(v, self(v)->PopupMenu(get<wxMenu>(v, 2))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, RibbonButtonBarEvent, wxCommandEvent);

class NB_WxRibbonButtonBarEvent: TNitClass<wxRibbonButtonBarEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(bar),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(popupMenu,		"(menu: wx.Menu): bool"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(bar)					{ return push(v, self(v)->GetBar()); }

	NB_FUNC(popupMenu)					{ return push(v, self(v)->PopupMenu(get<wxMenu>(v, 2))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, RibbonPanelEvent, wxCommandEvent);

class NB_WxRibbonPanelEvent: TNitClass<wxRibbonPanelEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(panel),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(panel)					{ return push(v, self(v)->GetPanel()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, RibbonGalleryEvent, wxCommandEvent);

class NB_WxRibbonGalleryEvent: TNitClass<wxRibbonGalleryEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(gallery),
			PROP_ENTRY_R(galleryItem),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(gallery)				{ return push(v, self(v)->GetGallery()); }
	NB_PROP_GET(galleryItem)			{ return push(v, self(v)->GetGalleryItem()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, RibbonArtProvider, NULL);

class NB_WxRibbonArtProvider : TNitClass<wxRibbonArtProvider>
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

NB_TYPE_WXOBJ(NITWX_API, RibbonControl, wxControl);

class NB_WxRibbonControl : TNitClass<wxRibbonControl>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(artProvider),
			PROP_ENTRY_R(ancestorRibbonBar),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(realize,		"(): bool"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(artProvider)			{ return push(v, self(v)->GetArtProvider()); }
	NB_PROP_GET(ancestorRibbonBar)		{ return push(v, self(v)->GetAncestorRibbonBar()); }

	NB_PROP_SET(artProvider)			{ self(v)->SetArtProvider(get<wxRibbonArtProvider>(v, 2)); return 0; }

	NB_FUNC(realize)					{ return push(v, self(v)->Realize()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, RibbonBar, wxRibbonControl);

class NB_WxRibbonBar : TNitClass<wxRibbonBar>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(activePage),
			PROP_ENTRY_R(pageCount),
			PROP_ENTRY	(panelsVisible),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(						"(parent: wx.Window=null, id=wx.ID.ANY, pos=null, size=null, style=STYLE.DEFAULT)"),
			FUNC_ENTRY_H(setTabCtrlMargins,		"(left, right: int)"),
			FUNC_ENTRY_H(getPage,				"(index: int): wx.RibbonPage"),
			FUNC_ENTRY_H(dismissExpandedPanel,	"(): bool"),
			FUNC_ENTRY_H(deletePage,			"(index: int)"),
			FUNC_ENTRY_H(clearPages,			"()"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "DEFAULT",						(int)wxRIBBON_BAR_DEFAULT_STYLE);
		newSlot(v, -1, "FOLDBAR",						(int)wxRIBBON_BAR_FOLDBAR_STYLE);
		newSlot(v, -1, "SHOW_PAGE_LABELS",				(int)wxRIBBON_BAR_SHOW_PAGE_LABELS);
		newSlot(v, -1, "SHOW_PAGE_ICONS",				(int)wxRIBBON_BAR_SHOW_PAGE_ICONS);
		newSlot(v, -1, "FLOW_HORIZONTAL",				(int)wxRIBBON_BAR_FLOW_HORIZONTAL);
		newSlot(v, -1, "FLOW_VERTICAL",					(int)wxRIBBON_BAR_FLOW_VERTICAL);
		newSlot(v, -1, "SHOW_PANEL_EXT_BUTTONS",		(int)wxRIBBON_BAR_SHOW_PANEL_EXT_BUTTONS);
		newSlot(v, -1, "SHOW_PANEL_MINIMISE_BUTTONS",	(int)wxRIBBON_BAR_SHOW_PANEL_MINIMISE_BUTTONS);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "PAGE_CHANGED",		(int)wxEVT_COMMAND_RIBBONBAR_PAGE_CHANGED);
		newSlot(v, -1, "PAGE_CHANGING",		(int)wxEVT_COMMAND_RIBBONBAR_PAGE_CHANGING);
		newSlot(v, -1, "TAB_MIDDLE_DOWN",	(int)wxEVT_COMMAND_RIBBONBAR_TAB_MIDDLE_DOWN);
		newSlot(v, -1, "TAB_MIDDLE_UP",		(int)wxEVT_COMMAND_RIBBONBAR_TAB_MIDDLE_UP);
		newSlot(v, -1, "TAB_RIGHT_DOWN",	(int)wxEVT_COMMAND_RIBBONBAR_TAB_RIGHT_DOWN);
		newSlot(v, -1, "TAB_RIGHT_UP",		(int)wxEVT_COMMAND_RIBBONBAR_TAB_RIGHT_UP);
		newSlot(v, -1, "TAB_LEFT_DCLICK",	(int)wxEVT_COMMAND_RIBBONBAR_TAB_LEFT_DCLICK);
		sq_poptop(v);
	}

	NB_PROP_GET(activePage)				{ return push(v, self(v)->GetActivePage()); }
	NB_PROP_GET(pageCount)				{ return push(v, self(v)->GetPageCount()); }
	NB_PROP_GET(panelsVisible)			{ return push(v, self(v)->ArePanelsShown()); }

	NB_PROP_SET(activePage)				{ if (isInt(v, 2)) self(v)->SetActivePage(getInt(v, 2)); else self(v)->SetActivePage(get<wxRibbonPage>(v, 2)); return 0; }
	NB_PROP_SET(panelsVisible)			{ self(v)->ShowPanels(getBool(v, 2)); return 0; }

	NB_CONS()
	{
		type* self = setSelf(v, new type(
			opt<wxWindow>(v, 2, NULL), 
			optInt(v, 3, wxID_ANY),
			*opt<wxPoint>(v, 4, wxDefaultPosition),
			*opt<wxSize>(v, 5, wxDefaultSize),
			optInt(v, 6, wxRIBBON_BAR_DEFAULT_STYLE)
			));

		return SQ_OK;
	}

	NB_FUNC(setTabCtrlMargins)			{ self(v)->SetTabCtrlMargins(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(getPage)					{ return push(v, self(v)->GetPage(getInt(v, 2))); }
	NB_FUNC(dismissExpandedPanel)		{ return push(v, self(v)->DismissExpandedPanel()); }
	NB_FUNC(deletePage)					{ self(v)->DeletePage(getInt(v, 2)); return 0; }
	NB_FUNC(clearPages)					{ self(v)->ClearPages(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, RibbonPage, wxRibbonControl);

class NB_WxRibbonPage : TNitClass<wxRibbonPage>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(icon),
			PROP_ENTRY_R(majorAxis),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: wx.RibbonBar, id=wx.ID.ANY, label=\"\", icon: wx.Bitmap=null, style=0)"),
			FUNC_ENTRY_H(setSizeWithScrollButtonAdjustment, "(x, y, width, height: int)"),
			FUNC_ENTRY_H(adjustRectToIncludeScrollButtons,	"(rect: wx.Rect) : wx.Rect"),
			FUNC_ENTRY_H(dismissExpandedPanel,	"(): bool"),
			FUNC_ENTRY_H(scrollLines,	"(lines: int): bool"),
			FUNC_ENTRY_H(scrollPixels,	"(pixels: int): bool"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(icon)					{ return push(v, self(v)->GetIcon()); }
	NB_PROP_GET(majorAxis)				{ return push(v, (int)self(v)->GetMajorAxis()); }

	NB_CONS()
	{
		type* self = setSelf(v, new type(
			get<wxRibbonBar>(v, 2),
			optInt(v, 3, wxID_ANY),
			optWxString(v, 4, wxEmptyString),
			*opt<wxBitmap>(v, 5, wxNullBitmap),
			optInt(v, 6, 0)
			));
		return SQ_OK;
	}

	NB_FUNC(setSizeWithScrollButtonAdjustment)	{ self(v)->SetSizeWithScrollButtonAdjustment(getInt(v, 2), getInt(v, 3), getInt(v, 4), getInt(v, 5)); return 0; }
	NB_FUNC(adjustRectToIncludeScrollButtons)	{ wxRect rect = *get<wxRect>(v, 2); self(v)->AdjustRectToIncludeScrollButtons(&rect); return push(v, rect); }
	NB_FUNC(dismissExpandedPanel)		{ return push(v, self(v)->DismissExpandedPanel()); }
	NB_FUNC(scrollLines)				{ return push(v, self(v)->ScrollLines(getInt(v, 2))); }
	NB_FUNC(scrollPixels)				{ return push(v, self(v)->ScrollPixels(getInt(v, 2))); }
};

////////////////////////////////////////////////////////////////////////////////

typedef wxRibbonToolBarToolBase wxRibbonToolBarItem;

NB_TYPE_WXOBJ(NITWX_API, RibbonToolBar, wxRibbonControl);

class NB_WxRibbonToolBar : TNitClass<wxRibbonToolBar>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(toolCount),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(					"(parent: wx.Window=null, id=wx.ID.ANY, pos=null, size=null, style=0)"),
			FUNC_ENTRY_H(addTool,			"(tool_id: int, bitmap: wx.Bitmap, help=\"\", kind=BUTTON.NORMAL, clientObject=null): wx.RibbonToolBarItem"
			"\n"							"(tool_id: int, bitmap: wx.Bitmap, disabled: wx.Bitmap, help: string, kind: BUTTON, clientObject=null): wx.RibbonToolBarItem"),
			FUNC_ENTRY_H(addDropdownTool,	"(tool_id: int, bitmap: wx.Bitmap, help=\"\"): wx.RibbonToolBarItem"),
			FUNC_ENTRY_H(addHybridTool,		"(tool_id: int, bitmap: wx.Bitmap, help=\"\"): wx.RibbonToolBarItem"),
			FUNC_ENTRY_H(addToggleTool,		"(tool_id: int, bitmap: wx.Bitmap, help=\"\"): wx.RibbonToolBarItem"),
			FUNC_ENTRY_H(addSeparator,		"(): wx.RibbonToolBarItem"),
			FUNC_ENTRY_H(insertTool,		"(pos: int, tool_id: int, bitmap: wx.Bitmap, help=\"\", kind=BUTTON.NORMAL, clientObject=null): wx.RibbonToolBarItem"
			"\n"							"(pos: int, tool_id: int, bitmap: wx.Bitmap, disabled: wx.Bitmap, help: string, kind: BUTTON, clientObject=null): wx.RibbonToolBarItem"),
			FUNC_ENTRY_H(insertDropdownTool,"(pos: int, tool_id: int, bitmap: wx.Bitmap, help=\"\"): wx.RibbonToolBarItem"),
			FUNC_ENTRY_H(insertHybridTool,	"(pos: int, tool_id: int, bitmap: wx.Bitmap, help=\"\"): wx.RibbonToolBarItem"),
			FUNC_ENTRY_H(addToggleTool,		"(pos: int, tool_id: int, bitmap: wx.Bitmap, help=\"\"): wx.RibbonToolBarItem"),
			FUNC_ENTRY_H(insertSeparator,	"(): wx.RibbonToolBarItem"),
			FUNC_ENTRY_H(clearTools,		"()"),
			FUNC_ENTRY_H(deleteTool,		"(tool_id: int): bool"),
			FUNC_ENTRY_H(deleteToolByPos,	"(pos: int): bool"),
			FUNC_ENTRY_H(findById,			"(tool_id: int): wx.RibbonToolBarItem"),
			FUNC_ENTRY_H(getToolByPos,		"(tool_id: int): wx.RibbonToolBarItem"),
			FUNC_ENTRY_H(getToolEnabled,	"(tool_id: int): bool"),
			FUNC_ENTRY_H(getToolPos,		"(tool_id: int): int"),
			FUNC_ENTRY_H(getToolState,		"(tool_id: int): bool"),
			FUNC_ENTRY_H(setRows,			"(min: int, max=-1)"),
			FUNC_ENTRY_H(enableTool,		"(tool_id: int, enable=true)"),
			FUNC_ENTRY_H(toggleTool,		"(tool_id: int, checked: bool"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "CLICKED",			(int)wxEVT_COMMAND_RIBBONTOOL_CLICKED);
		newSlot(v, -1, "DROPDOWN_CLICKED",	(int)wxEVT_COMMAND_RIBBONTOOL_DROPDOWN_CLICKED);
		sq_poptop(v);

		addStaticTable(v, "BUTTON");
		newSlot(v, -1, "NORMAL",		(int)wxRIBBON_BUTTON_NORMAL);
		newSlot(v, -1, "DROPDOWN",		(int)wxRIBBON_BUTTON_DROPDOWN);
		newSlot(v, -1, "HYBRID",		(int)wxRIBBON_BUTTON_HYBRID);
		newSlot(v, -1, "TOGGLE",		(int)wxRIBBON_BUTTON_TOGGLE);
		sq_poptop(v);
	}

	NB_PROP_GET(toolCount)				{ return push(v, self(v)->GetToolCount()); }

	NB_CONS()
	{
		type* self = setSelf(v, new type(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			*opt<wxPoint>(v, 4, wxDefaultPosition),
			*opt<wxSize>(v, 5, wxDefaultSize),
			optInt(v, 6, 0)
			));

		return SQ_OK;
	}

	NB_FUNC(addTool)
	{ 
		if (isNull(v, 4) || is<wxBitmap>(v, 4))
			return push(v, self(v)->AddTool(
				getInt(v, 2), *get<wxBitmap>(v, 3), *opt<wxBitmap>(v, 4, wxNullBitmap), 
				getWxString(v, 5), (wxRibbonButtonKind)getInt(v, 6), GetWxScriptData(v, 7, NULL)));
		else
			return push(v, self(v)->AddTool(getInt(v, 2), *get<wxBitmap>(v, 3), wxNullBitmap, optWxString(v, 4, wxEmptyString), (wxRibbonButtonKind)optInt(v, 5, wxRIBBON_BUTTON_NORMAL), GetWxScriptData(v, 6, NULL))); 
	}

	NB_FUNC(addDropdownTool)			{ return push(v, self(v)->AddDropdownTool(getInt(v, 2), *get<wxBitmap>(v, 3), optWxString(v, 4, wxEmptyString))); }
	NB_FUNC(addHybridTool)				{ return push(v, self(v)->AddHybridTool(getInt(v, 2), *get<wxBitmap>(v, 3), optWxString(v, 4, wxEmptyString))); }
	NB_FUNC(addToggleTool)				{ return push(v, self(v)->AddToggleTool(getInt(v, 2), *get<wxBitmap>(v, 3), optWxString(v, 4, wxEmptyString))); }
	NB_FUNC(addSeparator)				{ return push(v, self(v)->AddSeparator()); }

	NB_FUNC(insertTool)				
	{ 
		if (isNull(v, 5) || is<wxBitmap>(v, 5))
			return push(v, self(v)->InsertTool(
				getInt(v, 2), getInt(v, 3), 
				*get<wxBitmap>(v, 4), *opt<wxBitmap>(v, 5, wxNullBitmap), 
				getWxString(v, 6), (wxRibbonButtonKind)getInt(v, 7), GetWxScriptData(v, 8, NULL)));
		else
			return push(v, self(v)->InsertTool(getInt(v, 2), getInt(v, 3), *get<wxBitmap>(v, 4), wxNullBitmap, optWxString(v, 5, wxEmptyString), (wxRibbonButtonKind)optInt(v, 6, wxRIBBON_BUTTON_NORMAL), GetWxScriptData(v, 7, NULL)));
	}

	NB_FUNC(insertDropdownTool)			{ return push(v, self(v)->InsertDropdownTool(getInt(v, 2), getInt(v, 3), *get<wxBitmap>(v, 4), optWxString(v, 5, wxEmptyString))); }
	NB_FUNC(insertHybridTool)			{ return push(v, self(v)->InsertHybridTool(getInt(v, 2), getInt(v, 3), *get<wxBitmap>(v, 4), optWxString(v, 5, wxEmptyString))); }
	NB_FUNC(insertToggleTool)			{ return push(v, self(v)->InsertToggleTool(getInt(v, 2), getInt(v, 3), *get<wxBitmap>(v, 4), optWxString(v, 5, wxEmptyString))); }
	NB_FUNC(insertSeparator)			{ return push(v, self(v)->InsertSeparator(getInt(v, 2))); }

	NB_FUNC(clearTools)					{ self(v)->ClearTools(); return 0; }
	NB_FUNC(deleteTool)					{ return push(v, self(v)->DeleteTool(getInt(v, 2))); }
	NB_FUNC(deleteToolByPos)			{ return push(v, self(v)->DeleteToolByPos(getInt(v, 2))); }
	NB_FUNC(findById)					{ return push(v, self(v)->FindById(getInt(v, 2))); }
	NB_FUNC(getToolByPos)				{ return push(v, self(v)->GetToolByPos(getInt(v, 2))); }
	NB_FUNC(getToolEnabled)				{ return push(v, self(v)->GetToolEnabled(getInt(v, 2))); }
	NB_FUNC(getToolPos)					{ int pos = self(v)->GetToolPos(getInt(v, 2)); return pos != wxNOT_FOUND ? push(v, pos) : 0; }
	NB_FUNC(getToolState)				{ return push(v, self(v)->GetToolState(getInt(v, 2))); }
	NB_FUNC(setRows)					{ self(v)->SetRows(getInt(v, 2), optInt(v, 3, -1)); return 0; }
	NB_FUNC(enableTool)					{ self(v)->EnableTool(getInt(v, 2), optBool(v, 3, true)); return 0; }
	NB_FUNC(toggleTool)					{ self(v)->ToggleTool(getInt(v, 2), getBool(v, 3)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, RibbonToolBarItem, NULL);

class NB_WxRibbonToolBarItem : TNitClass<wxRibbonToolBarItem>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(helpString),
			PROP_ENTRY	(bitmap),
			PROP_ENTRY	(bitmapDisabled),
			PROP_ENTRY	(clientObject),
			PROP_ENTRY_R(id),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(helpString)				{ return push(v, self(v)->help_string); }
	NB_PROP_GET(bitmap)					{ return push(v, self(v)->bitmap); }
	NB_PROP_GET(bitmapDisabled)			{ return push(v, self(v)->bitmap_disabled); }
	NB_PROP_GET(clientObject)			{ return PushWxScriptData(v, self(v)->client_data.GetClientObject()); }
	NB_PROP_GET(id)						{ return push(v, self(v)->id); }
	
	NB_PROP_SET(helpString)				{ self(v)->help_string = getWxString(v, 2); return 0; }
	NB_PROP_SET(bitmap)					{ self(v)->bitmap = *get<wxBitmap>(v, 2); return 0; }
	NB_PROP_SET(bitmapDisabled)			{ self(v)->bitmap_disabled = *get<wxBitmap>(v, 2); return 0; }

	NB_PROP_SET(clientObject)
	{
		type* o = self(v);
		wxClientData* obj = o->client_data.GetClientObject();
		wxClientData* nobj = GetWxScriptData(v, 2, obj);
		if (obj != nobj)
			o->client_data.SetClientObject(nobj);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, RibbonButtonBar, wxRibbonControl);

class NB_WxRibbonButtonBar : TNitClass<wxRibbonButtonBar>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(buttonCount),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(						"(parent: wx.Window=null, id=wx.ID.ANY, pos=null, size=null, style=0)"),
			FUNC_ENTRY_H(addButton,				"(btn_id: int, label: string, bitmap: wx.Bitmap, small: wx.Bitmap, disabled: wx.Bitmap, smallDisabled: wx.Bitmap, kind: BUTTON, help: string)"
			"\n"								"(btn_id: int, label: string, bitmap: wx.Bitmap, help=\"\", kind=BUTTON.NORMAL)"),
			FUNC_ENTRY_H(addDropdownButton, 	"(btn_id: int, label: string, bitmap: wx.Bitmap, help=\"\")"),
			FUNC_ENTRY_H(addHybridButton,		"(btn_id: int, label: string, bitmap: wx.Bitmap, help=\"\")"),
			FUNC_ENTRY_H(addToggleButton,		"(btn_id: int, label: string, bitmap: wx.Bitmap, help=\"\")"),
			FUNC_ENTRY_H(insertButton,			"(pos: int, btn_id: int, label: string, bitmap: wx.Bitmap, small: wx.Bitmap, disabled: wx.Bitmap, smallDisabled: wx.Bitmap, kind: BUTTON, help: string)"
			"\n"								"(pos: int, btn_id: int, label: string, bitmap: wx.Bitmap, help=\"\", kind=BUTTON.NORMAL)"),
			FUNC_ENTRY_H(insertDropdownButton,	"(pos: int, btn_id: int, label: string, bitmap: wx.Bitmap, help=\"\")"),
			FUNC_ENTRY_H(insertHybridButton,	"(pos: int, btn_id: int, label: string, bitmap: wx.Bitmap, help=\"\")"),
			FUNC_ENTRY_H(insertToggleButton,	"(pos: int, btn_id: int, label: string, bitmap: wx.Bitmap, help=\"\")"),
			FUNC_ENTRY_H(clearButtons,			"()"),
			FUNC_ENTRY_H(deleteButton,			"(btn_id: int): bool"),
			FUNC_ENTRY_H(enableButton,			"(btn_id: int, enable=true)"),
			FUNC_ENTRY_H(toggleButton,			"(btn_id: int, checked: bool)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "CLICKED",			(int)wxEVT_COMMAND_RIBBONBUTTON_CLICKED);
		newSlot(v, -1, "DROPDOWN_CLICKED",	(int)wxEVT_COMMAND_RIBBONBUTTON_DROPDOWN_CLICKED);
		sq_poptop(v);

		addStaticTable(v, "BUTTON");
		newSlot(v, -1, "NORMAL",		(int)wxRIBBON_BUTTON_NORMAL);
		newSlot(v, -1, "DROPDOWN",		(int)wxRIBBON_BUTTON_DROPDOWN);
		newSlot(v, -1, "HYBRID",		(int)wxRIBBON_BUTTON_HYBRID);
		newSlot(v, -1, "TOGGLE",		(int)wxRIBBON_BUTTON_TOGGLE);
		sq_poptop(v);
	}

	NB_PROP_GET(buttonCount)			{ return push(v, self(v)->GetButtonCount()); }

	NB_CONS()
	{
		type* self = setSelf(v, new type(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			*opt<wxPoint>(v, 4, wxDefaultPosition),
			*opt<wxSize>(v, 5, wxDefaultSize),
			optInt(v, 6, 0)
			));

		return SQ_OK;
	}

	NB_FUNC(addButton)
	{
		if (isNull(v, 5) || is<wxBitmap>(v, 5))
			self(v)->AddButton(
				getInt(v, 2), getWxString(v, 3),
				*get<wxBitmap>(v, 4), *opt<wxBitmap>(v, 5, wxNullBitmap), *opt<wxBitmap>(v, 6, wxNullBitmap), *opt<wxBitmap>(v, 7, wxNullBitmap),
				(wxRibbonButtonKind)getInt(v, 8), getWxString(v, 9));
		else
			self(v)->AddButton(getInt(v, 2), getWxString(v, 3), *get<wxBitmap>(v, 4), optWxString(v, 5, wxEmptyString), (wxRibbonButtonKind)optInt(v, 6, wxRIBBON_BUTTON_NORMAL));
		return 0;
	}

	NB_FUNC(addDropdownButton)			{ self(v)->AddDropdownButton(getInt(v, 2), getWxString(v, 3), *get<wxBitmap>(v, 4), optWxString(v, 5, wxEmptyString)); return 0; }
	NB_FUNC(addHybridButton)			{ self(v)->AddHybridButton(getInt(v, 2), getWxString(v, 3), *get<wxBitmap>(v, 4), optWxString(v, 5, wxEmptyString)); return 0; }
	NB_FUNC(addToggleButton)			{ self(v)->AddToggleButton(getInt(v, 2), getWxString(v, 3), *get<wxBitmap>(v, 4), optWxString(v, 5, wxEmptyString)); return 0; }

	NB_FUNC(insertButton)
	{
		if (isNull(v, 6) || is<wxBitmap>(v, 6))
			self(v)->InsertButton(
				getInt(v, 2), getInt(v, 3), getWxString(v, 4),
				*get<wxBitmap>(v, 5), *opt<wxBitmap>(v, 6, wxNullBitmap), *opt<wxBitmap>(v, 7, wxNullBitmap), *opt<wxBitmap>(v, 8, wxNullBitmap),
				(wxRibbonButtonKind)getInt(v, 9), getWxString(v, 10));
		else
			self(v)->InsertButton(getInt(v, 2), getInt(v, 3), getWxString(v, 4), *get<wxBitmap>(v, 5), optWxString(v, 6, wxEmptyString), (wxRibbonButtonKind)optInt(v, 6, wxRIBBON_BUTTON_NORMAL));
		return 0;
	}

	NB_FUNC(insertDropdownButton)		{ self(v)->InsertDropdownButton(getInt(v, 2), getInt(v, 3), getWxString(v, 4), *get<wxBitmap>(v, 5), optWxString(v, 6, wxEmptyString)); return 0; }
	NB_FUNC(insertHybridButton)			{ self(v)->InsertHybridButton(getInt(v, 2), getInt(v, 3), getWxString(v, 4), *get<wxBitmap>(v, 5), optWxString(v, 6, wxEmptyString)); return 0; }
	NB_FUNC(insertToggleButton)			{ self(v)->InsertToggleButton(getInt(v, 2), getInt(v, 3), getWxString(v, 4), *get<wxBitmap>(v, 5), optWxString(v, 6, wxEmptyString)); return 0; }

	NB_FUNC(clearButtons)				{ self(v)->ClearButtons(); return 0; }
	NB_FUNC(deleteButton)				{ return push(v, self(v)->DeleteButton(getInt(v, 2))); return 0; }
	NB_FUNC(enableButton)				{ self(v)->EnableButton(getInt(v, 2), optBool(v, 3, true)); return 0; }
	NB_FUNC(toggleButton)				{ self(v)->ToggleButton(getInt(v, 2), getBool(v, 3)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, RibbonPanel, wxRibbonControl);

class NB_WxRibbonPanel : TNitClass<wxRibbonPanel>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(minimizedIcon),
			PROP_ENTRY_R(hasExtButton),
			PROP_ENTRY_R(minimized),
			PROP_ENTRY_R(hovered),
			PROP_ENTRY_R(extButtonHovered),
			PROP_ENTRY_R(canAutoMinimise),
			PROP_ENTRY_R(expandedDummy),
			PROP_ENTRY_R(expandedPanel),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: wx.Window=null, id=wx.ID.ANY, label=\"\", minimizedIcon: wxBitmap=null, pos=null, size=null, style=STYLE.DEFAULT)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "DEFAULT",				(int)wxRIBBON_PANEL_DEFAULT_STYLE);
		newSlot(v, -1, "NO_AUTO_MINIMIZE",		(int)wxRIBBON_PANEL_NO_AUTO_MINIMISE);
		newSlot(v, -1, "EXT_BUTTON",			(int)wxRIBBON_PANEL_EXT_BUTTON);
		newSlot(v, -1, "MINIMIZE_BUTTON",		(int)wxRIBBON_PANEL_MINIMISE_BUTTON);
		newSlot(v, -1, "STRETCH",				(int)wxRIBBON_PANEL_STRETCH);
		newSlot(v, -1, "FLEXIBLE",				(int)wxRIBBON_PANEL_FLEXIBLE);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "EXTBUTTON_ACTIVATED",	(int)wxEVT_COMMAND_RIBBONPANEL_EXTBUTTON_ACTIVATED);
		sq_poptop(v);
	}

	NB_PROP_GET(minimizedIcon)			{ return push(v, self(v)->GetMinimisedIcon()); }
	NB_PROP_GET(hasExtButton)			{ return push(v, self(v)->HasExtButton()); }
	NB_PROP_GET(minimized)				{ return push(v, self(v)->IsMinimised()); }
	NB_PROP_GET(hovered)				{ return push(v, self(v)->IsHovered()); }
	NB_PROP_GET(extButtonHovered)		{ return push(v, self(v)->IsExtButtonHovered()); }
	NB_PROP_GET(canAutoMinimise)		{ return push(v, self(v)->CanAutoMinimise()); }
	NB_PROP_GET(expandedDummy)			{ return push(v, self(v)->GetExpandedDummy()); }
	NB_PROP_GET(expandedPanel)			{ return push(v, self(v)->GetExpandedPanel()); }

	NB_CONS()
	{
		type* self = setSelf(v, new type(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			optWxString(v, 4, wxEmptyString),
			*opt<wxBitmap>(v, 5, wxNullBitmap),
			*opt<wxPoint>(v, 6, wxDefaultPosition),
			*opt<wxSize>(v, 7, wxDefaultSize),
			optInt(v, 8, wxRIBBON_PANEL_DEFAULT_STYLE)
			));
		return SQ_OK;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, RibbonGallery, wxRibbonControl);

class NB_WxRibbonGallery : TNitClass<wxRibbonGallery>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(empty),
			PROP_ENTRY_R(count),
			PROP_ENTRY	(selection),
			PROP_ENTRY_R(hoveredItem),
			PROP_ENTRY_R(activeItem),
			PROP_ENTRY_R(hovered),
			PROP_ENTRY_R(upButtonState),
			PROP_ENTRY_R(downButtonState),
			PROP_ENTRY_R(extensionButtonState),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: wx.Window=null, id=wx.ID.ANY, pos=null, size=null, style=0)"),
			FUNC_ENTRY_H(getItem,		"(index: int): wx.RibbonGalleryItem"),
			FUNC_ENTRY_H(append,		"(bitmap: wx.Bitmap, id: int, object=null): wx.RibbonGalleryItem"),
			FUNC_ENTRY_H(scrollLines,	"(lines: int): bool"),
			FUNC_ENTRY_H(scrollPixels,	"(pixels: int): bool"),
			FUNC_ENTRY_H(ensureVisible,	"(item: wx.RibbonGalleryItem)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "BUTTON_STATE");
		newSlot(v, -1, "NORMAL",		(int)wxRIBBON_GALLERY_BUTTON_NORMAL);
		newSlot(v, -1, "HOVERED",		(int)wxRIBBON_GALLERY_BUTTON_HOVERED);
		newSlot(v, -1, "ACTIVE",		(int)wxRIBBON_GALLERY_BUTTON_ACTIVE);
		newSlot(v, -1, "DISABLED",		(int)wxRIBBON_GALLERY_BUTTON_DISABLED);
		sq_poptop(v);
	}

	NB_PROP_GET(empty)					{ return push(v, self(v)->IsEmpty()); }
	NB_PROP_GET(count)					{ return push(v, self(v)->GetCount()); }
	NB_PROP_GET(selection)				{ return push(v, self(v)->GetSelection()); }
	NB_PROP_GET(hoveredItem)			{ return push(v, self(v)->GetHoveredItem()); }
	NB_PROP_GET(activeItem)				{ return push(v, self(v)->GetActiveItem()); }
	NB_PROP_GET(hovered)				{ return push(v, self(v)->IsHovered()); }
	NB_PROP_GET(upButtonState)			{ return push(v, (int)self(v)->GetUpButtonState()); }
	NB_PROP_GET(downButtonState)		{ return push(v, (int)self(v)->GetDownButtonState()); }
	NB_PROP_GET(extensionButtonState)	{ return push(v, (int)self(v)->GetExtensionButtonState()); }

	NB_PROP_SET(selection)				{ self(v)->SetSelection(opt<wxRibbonGalleryItem>(v, 2, NULL)); return 0; }

	NB_CONS()
	{ 
		type* self = setSelf(v, new type(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			*opt<wxPoint>(v, 4, wxDefaultPosition),
			*opt<wxSize>(v, 5, wxDefaultSize),
			optInt(v, 6, 0)
			));
		return SQ_OK;
	}

	NB_FUNC(getItem)					{ return push(v, self(v)->GetItem(getInt(v, 2))); }

	NB_FUNC(append)
	{
		if (isNone(v, 4))
			return push(v, self(v)->Append(*get<wxBitmap>(v, 2), getInt(v, 3)));
		else
			return push(v, self(v)->Append(*get<wxBitmap>(v, 2), getInt(v, 3), GetWxScriptData(v, 4, NULL)));
	}

	NB_FUNC(scrollLines)				{ return push(v, self(v)->ScrollLines(getInt(v, 2))); }
	NB_FUNC(scrollPixels)				{ return push(v, self(v)->ScrollPixels(getInt(v, 2))); }
	NB_FUNC(ensureVisible)				{ self(v)->EnsureVisible(get<wxRibbonGalleryItem>(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, RibbonGalleryItem, NULL);

class NB_WxRibbonGalleryItem : TNitClass<wxRibbonGalleryItem>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(id),
			PROP_ENTRY_R(bitmap),
			PROP_ENTRY_R(visible),
			PROP_ENTRY_R(position),
			PROP_ENTRY	(clientObject),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(id)						{ return push(v, self(v)->GetId()); }
	NB_PROP_GET(bitmap)					{ return push(v, self(v)->GetBitmap()); }
	NB_PROP_GET(visible)				{ return push(v, self(v)->IsVisible()); }
	NB_PROP_GET(position)				{ return push(v, self(v)->GetPosition()); }
	NB_PROP_GET(clientObject)			{ return PushWxScriptData(v, self(v)->GetClientObject()); }

	NB_PROP_SET(clientObject)
	{
		type* o = self(v);
		wxClientData* obj = o->GetClientObject();
		wxClientData* nobj = GetWxScriptData(v, 2, obj);
		if (obj != nobj)
			o->SetClientObject(nobj);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

SQRESULT NitLibWxAui(HSQUIRRELVM v)
{
	NB_WxAuiManagerEvent::Register(v);
	NB_WxAuiNotebookEvent::Register(v);
	NB_WxAuiToolBarEvent::Register(v);

	NB_WxAuiPaneInfo::Register(v);
	NB_WxAuiManager::Register(v);
	NB_WxAuiNotebook::Register(v);
	NB_WxAuiToolBar::Register(v);
	NB_WxAuiToolBarItem::Register(v);

	NB_WxRibbonBarEvent::Register(v);
	NB_WxRibbonToolBarEvent::Register(v);
	NB_WxRibbonButtonBarEvent::Register(v);
	NB_WxRibbonPanelEvent::Register(v);
	NB_WxRibbonGalleryEvent::Register(v);

	NB_WxRibbonArtProvider::Register(v);
	NB_WxRibbonControl::Register(v);
	NB_WxRibbonBar::Register(v);
	NB_WxRibbonPage::Register(v);
	NB_WxRibbonToolBar::Register(v);
	NB_WxRibbonToolBarItem::Register(v);
	NB_WxRibbonButtonBar::Register(v);
	NB_WxRibbonPanel::Register(v);
	NB_WxRibbonGallery::Register(v);
	NB_WxRibbonGalleryItem::Register(v);

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
