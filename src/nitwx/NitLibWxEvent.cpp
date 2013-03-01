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

#include <wx/process.h>
#include <wx/event.h>

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, CommandEvent, wxEvent);

class NB_WxCommandEvent : TNitClass<wxCommandEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(clientObject),
			PROP_ENTRY_R(extraLong),
			PROP_ENTRY_R(int),
			PROP_ENTRY_R(itemIndex),
			PROP_ENTRY_R(string),
			PROP_ENTRY_R(checked),
			PROP_ENTRY_R(selection),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(clientObject)			{ return PushWxScriptData(v, self(v)->GetClientObject()); }
	NB_PROP_GET(extraLong)				{ return push(v, (int)self(v)->GetExtraLong()); }
	NB_PROP_GET(int)					{ return push(v, self(v)->GetInt()); }
	NB_PROP_GET(itemIndex)				{ return push(v, self(v)->GetSelection()); }
	NB_PROP_GET(string)					{ return push(v, self(v)->GetString()); }
	NB_PROP_GET(checked)				{ return push(v, self(v)->IsChecked()); }
	NB_PROP_GET(selection)				{ return push(v, self(v)->IsSelection()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, ActivateEvent, wxEvent);

class NB_WxActivateEvent : TNitClass<wxActivateEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(active),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(active)					{ return push(v, self(v)->GetActive()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, MenuEvent, wxCommandEvent);

class NB_WxMenuEvent : TNitClass<wxMenuEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(menuId),
			PROP_ENTRY_R(popup),
			PROP_ENTRY_R(menu),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(menuId)					{ return push(v, self(v)->GetMenuId()); }
	NB_PROP_GET(popup)					{ return push(v, self(v)->IsPopup()); }
	NB_PROP_GET(menu)					{ return push(v, self(v)->GetMenu()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, CloseEvent, wxEvent);

class NB_WxCloseEvent : TNitClass<wxCloseEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(canVeto),
			PROP_ENTRY_R(loggingOff),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(veto,			"(veto=true)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(canVeto)				{ return push(v, self(v)->CanVeto()); }
	NB_PROP_GET(loggingOff)				{ return push(v, self(v)->GetLoggingOff()); }

	NB_FUNC(veto)						{ self(v)->Veto(optBool(v, 2, true)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, NotifyEvent, wxCommandEvent);

class NB_WxNotifyEvent : TNitClass<wxNotifyEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(allowed),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(veto,			"(veto=true)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(allowed)				{ return push(v, self(v)->IsAllowed()); }

	NB_FUNC(veto)						{ if (optBool(v, 2, true)) self(v)->Veto(); else self(v)->Allow(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, ScrollEvent, wxCommandEvent);

class NB_WxScrollEvent : TNitClass<wxScrollEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(orientation),
			PROP_ENTRY_R(position),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);

		addStatic(v, "HORIZONTAL",		(int)wxHORIZONTAL);
		addStatic(v, "VERTICAL",		(int)wxVERTICAL);
	}

	NB_PROP_GET(orientation)			{ return push(v, (int)self(v)->GetOrientation()); }
	NB_PROP_GET(position)				{ return push(v, self(v)->GetPosition()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, ScrollWinEvent, wxCommandEvent);

class NB_WxScrollWinEvent : TNitClass<wxScrollWinEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(orientation),
			PROP_ENTRY_R(position),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);

		addStatic(v, "HORIZONTAL",		(int)wxHORIZONTAL);
		addStatic(v, "VERTICAL",		(int)wxVERTICAL);
	}

	NB_PROP_GET(orientation)			{ return push(v, (int)self(v)->GetOrientation()); }
	NB_PROP_GET(position)				{ return push(v, self(v)->GetPosition()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, MouseEvent, wxEvent);

class NB_WxMouseEvent : TNitClass<wxMouseEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(entering),
			PROP_ENTRY_R(moving),
			PROP_ENTRY_R(dragging),
			PROP_ENTRY_R(leaving),

			PROP_ENTRY_R(leftDClick),
			PROP_ENTRY_R(leftDown),
			PROP_ENTRY_R(leftUp),
			PROP_ENTRY_R(middleDClick),
			PROP_ENTRY_R(middleDown),
			PROP_ENTRY_R(middleUp),
			PROP_ENTRY_R(rightDClick),
			PROP_ENTRY_R(rightDown),
			PROP_ENTRY_R(rightUp),

			PROP_ENTRY_R(wheelDelta),
			PROP_ENTRY_R(wheelRotation),
			PROP_ENTRY_R(wheelAxis),

			PROP_ENTRY_R(linesPerAction),
			PROP_ENTRY_R(clickCount),
			PROP_ENTRY_R(buttons),

			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(button,			"(btn: BTN): bool"),
			FUNC_ENTRY_H(buttonDClick,		"(btn: BTN.ANY): bool"),
			FUNC_ENTRY_H(buttonDown,		"(btn: BTN.ANY): bool"),
			FUNC_ENTRY_H(buttonUp,			"(btn: BTN.ANY): bool"),
// TODO:	FUNC_ENTRY_H(getLogicalPosition,"(dc: DC): Point"),
			NULL
		};

		bind(v, props, funcs);

		NB_WxMouseState::Register<type>(v);
	}

	NB_PROP_GET(entering)				{ return push(v, self(v)->Entering()); }
	NB_PROP_GET(moving)					{ return push(v, self(v)->Moving()); }
	NB_PROP_GET(dragging)				{ return push(v, self(v)->Dragging()); }
	NB_PROP_GET(leaving)				{ return push(v, self(v)->Leaving()); }
	NB_PROP_GET(leftDClick)				{ return push(v, self(v)->LeftDClick()); }
	NB_PROP_GET(leftDown)				{ return push(v, self(v)->LeftDown()); }
	NB_PROP_GET(leftUp)					{ return push(v, self(v)->LeftUp()); }
	NB_PROP_GET(middleDClick)			{ return push(v, self(v)->MiddleDClick()); }
	NB_PROP_GET(middleDown)				{ return push(v, self(v)->MiddleDown()); }
	NB_PROP_GET(middleUp)				{ return push(v, self(v)->MiddleUp()); }
	NB_PROP_GET(rightDClick)			{ return push(v, self(v)->RightDClick()); }
	NB_PROP_GET(rightDown)				{ return push(v, self(v)->RightDown()); }
	NB_PROP_GET(rightUp)				{ return push(v, self(v)->RightUp()); }
	NB_PROP_GET(wheelDelta)				{ return push(v, self(v)->GetWheelDelta()); }
	NB_PROP_GET(wheelRotation)			{ return push(v, self(v)->GetWheelRotation()); }
	NB_PROP_GET(wheelAxis)				{ return push(v, (int)self(v)->GetWheelAxis()); }
	NB_PROP_GET(linesPerAction)			{ return push(v, self(v)->GetLinesPerAction()); }
	NB_PROP_GET(clickCount)				{ return push(v, self(v)->GetClickCount()); }
	NB_PROP_GET(buttons)				{ return push(v, self(v)->GetButton()); }

	NB_FUNC(button)						{ return push(v, self(v)->Button((wxMouseButton)getInt(v, 2))); }
	NB_FUNC(buttonDClick)				{ return push(v, self(v)->ButtonDClick((wxMouseButton)getInt(v, 2))); }
	NB_FUNC(buttonDown)					{ return push(v, self(v)->ButtonDown((wxMouseButton)getInt(v, 2))); }
	NB_FUNC(buttonUp)					{ return push(v, self(v)->ButtonUp((wxMouseButton)getInt(v, 2))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, KeyEvent, wxEvent);

class NB_WxKeyEvent : TNitClass<wxKeyEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(keyCode),
			PROP_ENTRY_R(rawKeyCode),
			PROP_ENTRY_R(rawKeyFlags),
			PROP_ENTRY_R(unicodeKey),
			PROP_ENTRY_R(position),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(inCategory,	"(category: CATEGORY): bool"),
			NULL
		};

		bind(v, props, funcs);

		NB_WxKeyboardState::Register<type>(v);

		addStaticTable(v, "CATEGORY");
		newSlot(v, -1, "ARROW",			(int)WXK_CATEGORY_ARROW);
		newSlot(v, -1, "PAGING",		(int)WXK_CATEGORY_PAGING);
		newSlot(v, -1, "JUMP",			(int)WXK_CATEGORY_JUMP);
		newSlot(v, -1, "TAB",			(int)WXK_CATEGORY_TAB) ;
		newSlot(v, -1, "CUT",			(int)WXK_CATEGORY_CUT);
		newSlot(v, -1, "NAVIGATION",	(int)WXK_CATEGORY_NAVIGATION);
		sq_poptop(v);
	}

	NB_PROP_GET(keyCode)				{ return push(v, self(v)->GetKeyCode()); }
	NB_PROP_GET(rawKeyCode)				{ return push(v, self(v)->GetRawKeyCode()); }
	NB_PROP_GET(rawKeyFlags)			{ return push(v, self(v)->GetRawKeyFlags()); }
	NB_PROP_GET(unicodeKey)				{ return push(v, (int)self(v)->GetUnicodeKey()); }
	NB_PROP_GET(position)				{ wxPoint p; self(v)->GetPosition(&p.x, &p.y); return push(v, p); }

	NB_FUNC(inCategory)					{ return push(v, self(v)->IsKeyInCategory((int)getInt(v, 2))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, SizeEvent, wxEvent);

class NB_WxSizeEvent : TNitClass<wxSizeEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(size),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(size)					{ return push(v, self(v)->GetSize()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, MoveEvent, wxEvent);

class NB_WxMoveEvent : TNitClass<wxMoveEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(position),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(position)				{ return push(v, self(v)->GetPosition()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, FocusEvent, wxEvent);

class NB_WxFocusEvent : TNitClass<wxFocusEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(window),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(window)					{ return push(v, self(v)->GetWindow()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, ChildFocusEvent, wxCommandEvent);

class NB_WxChildFocusEvent : TNitClass<wxChildFocusEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(window),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(window)					{ return push(v, self(v)->GetWindow()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, ShowEvent, wxEvent);

class NB_WxShowEvent : TNitClass<wxShowEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(shown),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(shown)					{ return push(v, self(v)->IsShown()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, DropFilesEvent, wxEvent);

class NB_WxDropFilesEvent : TNitClass<wxDropFilesEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(position),
			PROP_ENTRY_R(numberOfFiles),
			PROP_ENTRY_R(files),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(position)				{ return push(v, self(v)->GetPosition()); }
	NB_PROP_GET(numberOfFiles)			{ return push(v, self(v)->GetNumberOfFiles()); }

	NB_PROP_GET(files)
	{
		sq_newarray(v, 0);
		type* o = self(v);
		wxString* files = o->GetFiles();
		for (int i=0; i<o->GetNumberOfFiles(); ++i)
			arrayAppend(v, -1, files[i]);

		return 1;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, UpdateUIEvent, wxEvent);

class NB_WxUpdateUIEvent : TNitClass<wxUpdateUIEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(checked),
			PROP_ENTRY	(enabled),
			PROP_ENTRY	(visible),
			PROP_ENTRY	(text),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(wasCheckedSet,	"(): bool // return if setter of 'Checked' called"),
			FUNC_ENTRY_H(wasEnabledSet,	"(): bool // return if setter of 'Enabled' called"),
			FUNC_ENTRY_H(wasVisibleSet,	"(): bool // return if setter of 'Visible' called"),
			FUNC_ENTRY_H(wasTextSet,	"(): bool // return if setter of 'Text' called"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(checked)				{ return push(v, self(v)->GetChecked()); }
	NB_PROP_GET(enabled)				{ return push(v, self(v)->GetEnabled()); }
	NB_PROP_GET(visible)				{ return push(v, self(v)->GetShown()); }
	NB_PROP_GET(text)					{ return push(v, self(v)->GetText()); }

	NB_PROP_SET(checked)				{ self(v)->Check(getBool(v, 2)); return 0; }
	NB_PROP_SET(enabled)				{ self(v)->Enable(getBool(v, 2)); return 0; }
	NB_PROP_SET(visible)				{ self(v)->Show(getBool(v, 2)); return 0; }
	NB_PROP_SET(text)					{ self(v)->SetText(getWxString(v, 2)); return 0; }

	NB_FUNC(wasCheckedSet)				{ return push(v, self(v)->GetSetChecked()); }
	NB_FUNC(wasEnabledSet)				{ return push(v, self(v)->GetSetEnabled()); }
	NB_FUNC(wasVisibleSet)				{ return push(v, self(v)->GetSetShown()); }
	NB_FUNC(wasTextSet)					{ return push(v, self(v)->GetSetText()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, MouseCaptureChangedEvent, wxEvent);

class NB_WxMouseCaptureChangedEvent : TNitClass<wxMouseCaptureChangedEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(capturedWindow),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(capturedWindow)			{ return push(v, self(v)->GetCapturedWindow()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, NavigationKeyEvent, wxEvent);

class NB_WxNavigationKeyEvent : TNitClass<wxNavigationKeyEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(direction),
			PROP_ENTRY_R(fromTab),
			PROP_ENTRY_R(windowChange),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(direction)				{ return push(v, self(v)->GetDirection()); }
	NB_PROP_GET(fromTab)				{ return push(v, self(v)->IsFromTab()); }
	NB_PROP_GET(windowChange)			{ return push(v, self(v)->IsWindowChange()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, HelpEvent, wxCommandEvent);

class NB_WxHelpEvent : TNitClass<wxHelpEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(origin),
			PROP_ENTRY_R(position),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "ORIGIN");
		newSlot(v, -1, "UNKNOWN",		(int)wxHelpEvent::Origin_Unknown);
		newSlot(v, -1, "KEYBOARD",		(int)wxHelpEvent::Origin_Keyboard);
		newSlot(v, -1, "HELPBUTTON",	(int)wxHelpEvent::Origin_HelpButton);
		sq_poptop(v);
	}

	NB_PROP_GET(origin)					{ return push(v, (int)self(v)->GetOrigin()); }
	NB_PROP_GET(position)				{ return push(v, self(v)->GetPosition()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, ContextMenuEvent, wxCommandEvent);

class NB_WxContextMenuEvent : TNitClass<wxContextMenuEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(position),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(position)				{ return push(v, self(v)->GetPosition()); }
	NB_PROP_SET(position)				{ self(v)->SetPosition(*get<wxPoint>(v, 2)); return 0; } // TODO: Check if works as expected
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, WindowCreateEvent, wxCommandEvent);

class NB_WxWindowCreateEvent : TNitClass<wxWindowCreateEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(window),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(window)					{ return push(v, self(v)->GetWindow()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, WindowDestroyEvent, wxCommandEvent);

class NB_WxWindowDestroyEvent : TNitClass<wxWindowDestroyEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(window),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(window)					{ return push(v, self(v)->GetWindow()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, IdleEvent, wxEvent);

class NB_WxIdleEvent : TNitClass<wxIdleEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(moreRequested),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(requestMore,	"(needMore=true)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(moreRequested)			{ return push(v, self(v)->MoreRequested()); }

	NB_FUNC(requestMore)				{ self(v)->RequestMore(optBool(v, 2, true)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, BookCtrlEvent, wxNotifyEvent);

class NB_WxBookCtrlEvent : TNitClass<wxBookCtrlEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(oldSelection),
			PROP_ENTRY_R(selection),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(oldSelection)			{ return push(v, self(v)->GetOldSelection()); }
	NB_PROP_GET(selection)				{ return push(v, self(v)->GetSelection()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, CollapsiblePaneEvent, wxCommandEvent);

class NB_WxCollapsiblePaneEvent : TNitClass<wxCollapsiblePaneEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(collapsed),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(collapsed)				{ return push(v, self(v)->GetCollapsed()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, FileSystemWatcherEvent, wxEvent);

// TODO: bind wxFileName

class NB_WxFileSystemWatcherEvent : TNitClass<wxFileSystemWatcherEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(path),
			PROP_ENTRY_R(newPath),
			PROP_ENTRY_R(changeType),
			PROP_ENTRY_R(error),
			PROP_ENTRY_R(errorDescription),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "CHANGE");
		newSlot(v, -1, "CREATE",		(int)wxFSW_EVENT_CREATE);
		newSlot(v, -1, "DELETE",		(int)wxFSW_EVENT_DELETE);
		newSlot(v, -1, "RENAME",		(int)wxFSW_EVENT_RENAME);
		newSlot(v, -1, "MODIFY",		(int)wxFSW_EVENT_MODIFY);
		newSlot(v, -1, "ACCESS",		(int)wxFSW_EVENT_ACCESS);
		newSlot(v, -1, "WARNING",		(int)wxFSW_EVENT_WARNING);
		newSlot(v, -1, "ERROR",			(int)wxFSW_EVENT_ERROR);
		sq_poptop(v);
	}

	NB_PROP_GET(path)					{ return push(v, self(v)->GetPath().GetFullPath()); }
	NB_PROP_GET(newPath)				{ return push(v, self(v)->GetNewPath().GetFullPath()); }
	NB_PROP_GET(changeType)				{ return push(v, self(v)->GetChangeType()); }
	NB_PROP_GET(error)					{ return push(v, self(v)->IsError()); }
	NB_PROP_GET(errorDescription)		{ return push(v, self(v)->GetErrorDescription()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, ProcessEvent, wxEvent);

class NB_WxProcessEvent: TNitClass<wxProcessEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(exitCode),
			PROP_ENTRY_R(pid),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(exitCode)				{ return push(v, self(v)->GetExitCode()); }
	NB_PROP_GET(pid)					{ return push(v, self(v)->GetPid()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, TimerEvent, wxEvent);

class NB_WxTimerEvent : TNitClass<wxTimerEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(interval),
			PROP_ENTRY_R(timer),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(interval)				{ return push(v, self(v)->GetInterval()); }
	NB_PROP_GET(timer)					{ return push(v, &self(v)->GetTimer()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WX_AUTODELETE(NITWX_API, EventBlocker, wxEvtHandler, delete);

class NB_WxEventBlocker : TNitClass<wxEventBlocker>
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
			CONS_ENTRY_H(				"(win: Window, type=-1: EVT)"),
			FUNC_ENTRY_H(block,			"(type: EVT)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		sq_setinstanceup(v, 1, new wxEventBlocker(get<wxWindow>(v, 2), optInt(v, 3, -1)));
		return 0;
	}

	NB_FUNC(block)						{ self(v)->Block(getInt(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, Timer, wxEvtHandler);

class NB_WxTimer : TNitClass<wxTimer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(id),
			PROP_ENTRY_R(interval),
			PROP_ENTRY	(owner),
			PROP_ENTRY_R(oneShot),
			PROP_ENTRY_R(running),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(owner: EvtHandler=null, id=-1)"),
			FUNC_ENTRY_H(start,			"(millisec=-1, oneShot=false): bool"),
			FUNC_ENTRY_H(stop,			"()"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "TIMER",			(int)wxEVT_TIMER);
		sq_poptop(v);
	}

	NB_CONS()
	{
		if (isNone(v, 2))
			sq_setinstanceup(v, 1, new wxTimer());
		else
			sq_setinstanceup(v, 1, new wxTimer(get<wxEvtHandler>(v, 2), optInt(v, 3, -1)));
		return 0;
	}

	NB_PROP_GET(id)						{ return push(v, self(v)->GetId()); }
	NB_PROP_GET(interval)				{ return push(v, self(v)->GetInterval()); }
	NB_PROP_GET(owner)					{ return push(v, self(v)->GetOwner()); }
	NB_PROP_GET(oneShot)				{ return push(v, self(v)->IsOneShot()); }
	NB_PROP_GET(running)				{ return push(v, self(v)->IsRunning()); }

	NB_PROP_SET(owner)					{ self(v)->SetOwner(get<wxEvtHandler>(v, 2)); return 0; }

	NB_FUNC(start)						{ return push(v, self(v)->Start(optInt(v, 2, -1), optBool(v, 3, false))); }
	NB_FUNC(stop)						{ self(v)->Stop(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, StreamBase, wxObject);

class NB_WxStreamBase : TNitClass<wxStreamBase>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(lastError),
			PROP_ENTRY_R(size),
			PROP_ENTRY_R(ok),
			PROP_ENTRY_R(seekable),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "ERROR");
		newSlot(v, -1, "NONE",			(int)wxSTREAM_NO_ERROR);
		newSlot(v, -1, "EOF",			(int)wxSTREAM_EOF);
		newSlot(v, -1, "WRITE_ERROR",	(int)wxSTREAM_WRITE_ERROR);
		newSlot(v, -1, "READ_ERROR",	(int)wxSTREAM_READ_ERROR);
		sq_poptop(v);

		addStaticTable(v, "PROTOCOL");
		newSlot(v, -1, "DEFAULT",		(int)wxSTREAM_PROTOCOL);
		newSlot(v, -1, "MIMETYPE",		(int)wxSTREAM_MIMETYPE);
		newSlot(v, -1, "ENCODING",		(int)wxSTREAM_ENCODING);
		newSlot(v, -1, "FILEEXT",		(int)wxSTREAM_FILEEXT);
		sq_poptop(v);
	};

	NB_PROP_GET(lastError)				{ return push(v, (int)self(v)->GetLastError()); }
	NB_PROP_GET(size)					{ return push(v, self(v)->GetSize()); }
	NB_PROP_GET(ok)						{ return push(v, self(v)->IsOk()); }
	NB_PROP_GET(seekable)				{ return push(v, self(v)->IsSeekable()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, InputStream, wxStreamBase);

// TODO: integrate with nit.StreamReader

class NB_WxInputStream : TNitClass<wxInputStream>
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
	};
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, OutputStream, wxStreamBase);

// TODO: integrate with nit.StreamWriter

class NB_WxOutputStream : TNitClass<wxOutputStream>
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
	};
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, Process, wxEvtHandler);

class NB_WxProcess : TNitClass<wxProcess>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(inputStream),
			PROP_ENTRY_R(outputStream),
			PROP_ENTRY_R(errorStream),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: EvtHandler=null, id=-1)\n"
										"(flags: PROCESS)"),
			FUNC_ENTRY_H(closeOutput,	"()"),
			FUNC_ENTRY_H(detach,		"()"),
			FUNC_ENTRY_H(redirect,		"()"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "PROCESS");
		newSlot(v, -1, "DEFAULT",		(int)wxPROCESS_DEFAULT);
		newSlot(v, -1, "REDIRECT",		(int)wxPROCESS_REDIRECT);
		sq_poptop(v);

		addStaticTable(v, "EXEC");
		newSlot(v, -1, "ASYNC",			(int)wxEXEC_ASYNC);
		newSlot(v, -1, "SYNC",			(int)wxEXEC_SYNC);
		newSlot(v, -1, "NOHIDE",		(int)wxEXEC_NOHIDE);
		newSlot(v, -1, "NODISABLE",		(int)wxEXEC_NODISABLE);
		newSlot(v, -1, "NOEVENTS",		(int)wxEXEC_NOEVENTS);
		newSlot(v, -1, "BLOCK",			(int)wxEXEC_BLOCK);
		sq_poptop(v);
	};

	NB_CONS()
	{
		if (isInt(v, 2))
			sq_setinstanceup(v, 1, new wxProcess(getInt(v, 2)));
		else
			sq_setinstanceup(v, 1, new wxProcess(opt<wxEvtHandler>(v, 2, NULL), optInt(v, 3, -1)));

		return 0;
	}

	NB_PROP_GET(inputStream)
	{
		type* o = self(v);
		return push(v, o->IsInputAvailable() && o->IsInputOpened() ? o->GetInputStream() : NULL);
	}

	NB_PROP_GET(outputStream)
	{
		type* o = self(v);
		return push(v, o->GetOutputStream());
	}

	NB_PROP_GET(errorStream)
	{
		type* o = self(v);
		return push(v, o->IsErrorAvailable() ? o->GetErrorStream() : NULL);
	}

	NB_FUNC(closeOutput)				{ self(v)->CloseOutput(); return 0; }
	NB_FUNC(detach)						{ self(v)->Detach(); return 0; }
	NB_FUNC(redirect)					{ self(v)->Redirect(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

SQRESULT NitLibWxEvent(HSQUIRRELVM v)
{
	NB_WxCommandEvent::Register(v);
	NB_WxActivateEvent::Register(v);
	NB_WxMenuEvent::Register(v);
	NB_WxCloseEvent::Register(v);
	NB_WxNotifyEvent::Register(v);
	NB_WxScrollEvent::Register(v);
	NB_WxScrollWinEvent::Register(v);
	NB_WxMouseEvent::Register(v);
	NB_WxKeyEvent::Register(v);
	NB_WxSizeEvent::Register(v);
	NB_WxMoveEvent::Register(v);
	NB_WxFocusEvent::Register(v);
	NB_WxChildFocusEvent::Register(v);
	NB_WxShowEvent::Register(v);
	NB_WxDropFilesEvent::Register(v);
	NB_WxUpdateUIEvent::Register(v);
	NB_WxMouseCaptureChangedEvent::Register(v);
	NB_WxNavigationKeyEvent::Register(v);
	NB_WxHelpEvent::Register(v);
	NB_WxContextMenuEvent::Register(v);
	NB_WxWindowCreateEvent::Register(v);
	NB_WxWindowDestroyEvent::Register(v);
	NB_WxIdleEvent::Register(v);
	NB_WxBookCtrlEvent::Register(v);
	NB_WxCollapsiblePaneEvent::Register(v);
	NB_WxFileSystemWatcherEvent::Register(v);
	NB_WxProcessEvent::Register(v);
	NB_WxTimerEvent::Register(v);

	NB_WxEventBlocker::Register(v);

	// TODO: Refactor to somewhere
	NB_WxTimer::Register(v); 

	NB_WxStreamBase::Register(v);
	NB_WxInputStream::Register(v);
	NB_WxOutputStream::Register(v);
	NB_WxProcess::Register(v);
	
	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
