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
#include "nitwx/NitLibWxTextCtrl.h"

#include <wx/clrpicker.h>
#include <wx/fontpicker.h>
#include <wx/html/m_templ.h>
#include <wx/dirctrl.h>
#include <wx/hyperlink.h>
#include <wx/commandlinkbutton.h>
#include <wx/bannerwindow.h>
#include <wx/infobar.h>
#include <wx/richtooltip.h>
#include <wx/editlbox.h>
#include <wx/treectrl.h>
#include <wx/listctrl.h>
#include <wx/webview.h>
#include <wx/htmllbox.h>

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, Button, wxControl);

class NB_WxButton : TNitClass<wxButton>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] = 
		{
			PROP_ENTRY	(bitmap),
			PROP_ENTRY	(bitmapCurrent),
			PROP_ENTRY	(bitmapDisabled),
			PROP_ENTRY	(bitmapFocus),
			PROP_ENTRY	(bitmapLabel),
			PROP_ENTRY	(bitmapPressed),
			PROP_ENTRY	(bitmapMargins),
			PROP_ENTRY	(authNeeded),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, label=\"\", pos=null, size=null, style=0, validator=null, name=null)"),
			FUNC_ENTRY_H(setDefault,	"(): Window // returns old default"),
			FUNC_ENTRY_H(setBitmap,		"(bitmap: Bitmap, dir=DIR.LEFT)"),
			NULL
		};

		bind(v, props, funcs);

		addStatic(v, "DEFAULT_SIZE",	wxButton::GetDefaultSize());

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "LEFT",			(int)wxBU_LEFT);
		newSlot(v, -1, "TOP",			(int)wxBU_TOP);
		newSlot(v, -1, "RIGHT",			(int)wxBU_RIGHT);
		newSlot(v, -1, "BOTTOM",		(int)wxBU_BOTTOM);
		newSlot(v, -1, "EXACTFIT",		(int)wxBU_EXACTFIT);
		newSlot(v, -1, "NOTEXT",		(int)wxBU_NOTEXT);
		newSlot(v, -1, "BORDER_NONE",	(int)wxBORDER_NONE);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "BUTTON",		(int)wxEVT_COMMAND_BUTTON_CLICKED);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxControl* control = new wxButton(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			optWxString(v, 4, ""),
			*opt<wxPoint>(v, 5, wxDefaultPosition),
			*opt<wxSize>(v, 6, wxDefaultSize),
			optInt(v, 7, 0),
			*opt<wxValidator>(v, 8, (wxValidator*)&wxDefaultValidator),
			optWxString(v, 9, wxButtonNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}

	NB_PROP_GET(bitmap)					{ return push(v, self(v)->GetBitmap()); }
	NB_PROP_GET(bitmapCurrent)			{ return push(v, self(v)->GetBitmapCurrent()); }
	NB_PROP_GET(bitmapDisabled)			{ return push(v, self(v)->GetBitmapDisabled()); }
	NB_PROP_GET(bitmapFocus)			{ return push(v, self(v)->GetBitmapFocus()); }
	NB_PROP_GET(bitmapLabel)			{ return push(v, self(v)->GetBitmapLabel()); }
	NB_PROP_GET(bitmapPressed)			{ return push(v, self(v)->GetBitmapPressed()); }
	NB_PROP_GET(bitmapMargins)			{ return push(v, self(v)->GetBitmapMargins()); }
	NB_PROP_GET(authNeeded)				{ return push(v, self(v)->GetAuthNeeded()); }

	NB_PROP_SET(bitmap)					{ self(v)->SetBitmap(*get<wxBitmap>(v, 2)); return 0; }
	NB_PROP_SET(bitmapCurrent)			{ self(v)->SetBitmapCurrent(*get<wxBitmap>(v, 2)); return 0; }
	NB_PROP_SET(bitmapDisabled)			{ self(v)->SetBitmapDisabled(*get<wxBitmap>(v, 2)); return 0; }
	NB_PROP_SET(bitmapFocus)			{ self(v)->SetBitmapFocus(*get<wxBitmap>(v, 2)); return 0; }
	NB_PROP_SET(bitmapLabel)			{ self(v)->SetBitmapLabel(*get<wxBitmap>(v, 2)); return 0; }
	NB_PROP_SET(bitmapPressed)			{ self(v)->SetBitmapPressed(*get<wxBitmap>(v, 2)); return 0; }
	NB_PROP_SET(bitmapMargins)			{ self(v)->SetBitmapMargins(*get<wxSize>(v, 2)); return 0; }
	NB_PROP_SET(authNeeded)				{ self(v)->SetAuthNeeded(getBool(v, 2)); return 0; }

	NB_FUNC(setDefault)					{ return push(v, self(v)->SetDefault()); }
	NB_FUNC(setBitmap)					{ self(v)->SetBitmap(*get<wxBitmap>(v, 2), (wxDirection)optInt(v, 3, wxLEFT)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, StaticBox, wxControl);

class NB_WxStaticBox : TNitClass<wxStaticBox>
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
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, label: string, pos=null, size=null, style=0, name=null)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		wxControl* control = new wxStaticBox(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			getWxString(v, 4),
			*opt<wxPoint>(v, 5, wxDefaultPosition),
			*opt<wxSize>(v, 6, wxDefaultSize),
			optInt(v, 7, 0),
			optWxString(v, 8, wxStaticBoxNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, StaticText, wxControl);

class NB_WxStaticText : TNitClass<wxStaticText>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(ellipsized),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, label=\"\", pos=null, size=null, style=0, name=null)"),
			FUNC_ENTRY_H(wrap,			"(width: int)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "ALIGN_LEFT",		(int)wxALIGN_LEFT);
		newSlot(v, -1, "ALIGN_RIGHT",		(int)wxALIGN_RIGHT);
		newSlot(v, -1, "ALIGN_CENTER",		(int)wxALIGN_CENTER);
		newSlot(v, -1, "NO_AUTORESIZE",		(int)wxST_NO_AUTORESIZE);
		newSlot(v, -1, "ELLIPSIZE_START",	(int)wxST_ELLIPSIZE_START);
		newSlot(v, -1, "ELLIPSIZE_MIDDLE",	(int)wxST_ELLIPSIZE_MIDDLE);
		newSlot(v, -1, "ELLIPSIZE_END",		(int)wxST_ELLIPSIZE_END);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxControl* control = new wxStaticText(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			optWxString(v, 4, ""),
			*opt<wxPoint>(v, 5, wxDefaultPosition),
			*opt<wxSize>(v, 6, wxDefaultSize),
			optInt(v, 7, 0),
			optWxString(v, 8, wxStaticTextNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}

	NB_PROP_GET(ellipsized)				{ return push(v, self(v)->IsEllipsized()); }

	NB_FUNC(wrap)						{ self(v)->Wrap(getInt(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, StaticLine, wxControl);

class NB_WxStaticLine : TNitClass<wxStaticLine>
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
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, pos=null, size=null, style=STYLE.HORIZONTAL, name=null)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "HORIZONTAL",	(int)wxLI_HORIZONTAL);
		newSlot(v, -1, "VERTICAL",		(int)wxLI_VERTICAL);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxControl* control = new wxStaticLine(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			*opt<wxPoint>(v, 4, wxDefaultPosition),
			*opt<wxSize>(v, 5, wxDefaultSize),
			optInt(v, 6, wxLI_HORIZONTAL),
			optWxString(v, 7, wxStaticLineNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, StaticBitmap, wxControl);

class NB_WxStaticBitmap : TNitClass<wxStaticBitmap>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(bitmap),
			PROP_ENTRY	(icon),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, label: Bitmap, pos=null, size=null, style=0, name=null)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		wxControl* control = new wxStaticBitmap(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			*get<wxBitmap>(v, 4),
			*opt<wxPoint>(v, 5, wxDefaultPosition),
			*opt<wxSize>(v, 6, wxDefaultSize),
			optInt(v, 7, 0),
			optWxString(v, 8, wxStaticBitmapNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}

	NB_PROP_GET(bitmap)					{ return push(v, self(v)->GetBitmap()); }
	NB_PROP_GET(icon)					{ return push(v, self(v)->GetIcon()); }

	NB_PROP_SET(bitmap)					{ self(v)->SetBitmap(*get<wxBitmap>(v, 2)); return 0; }
	NB_PROP_SET(icon)					{ self(v)->SetBitmap(*get<wxIcon>(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, CheckBox, wxControl);

class NB_WxCheckBox : TNitClass<wxCheckBox>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(value),
			PROP_ENTRY_R(thirdState),
			PROP_ENTRY_R(thirdStateAllowedForUser),
			PROP_ENTRY_R(checked),
			PROP_ENTRY	(undetermined),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, label: string, pos=null, size=null, style=0, validator=null, name=null)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "TWO_STATE",		(int)wxCHK_2STATE);
		newSlot(v, -1, "THREE_STATE",	(int)wxCHK_3STATE);
		newSlot(v, -1, "ALLOW_3RD_STATE_FOR_USER",	(int)wxCHK_ALLOW_3RD_STATE_FOR_USER);
		newSlot(v, -1, "ALIGN_RIGHT",	(int)wxALIGN_RIGHT);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "CHECKBOX",		(int)wxEVT_COMMAND_CHECKBOX_CLICKED);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxControl* control = new wxButton(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			getWxString(v, 4),
			*opt<wxPoint>(v, 5, wxDefaultPosition),
			*opt<wxSize>(v, 6, wxDefaultSize),
			optInt(v, 7, 0),
			*opt<wxValidator>(v, 8, (wxValidator*)&wxDefaultValidator),
			optWxString(v, 9, wxCheckBoxNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}

	NB_PROP_GET(value)					{ return push(v, self(v)->GetValue()); }
	NB_PROP_GET(undetermined)			{ return push(v, self(v)->Get3StateValue() == wxCHK_UNDETERMINED); }
	NB_PROP_GET(thirdState)				{ return push(v, self(v)->Is3State()); }
	NB_PROP_GET(thirdStateAllowedForUser)	{ return push(v, self(v)->Is3rdStateAllowedForUser()); }
	NB_PROP_GET(checked)				{ return push(v, self(v)->IsChecked()); }

	NB_PROP_SET(value)					{ self(v)->SetValue(getBool(v, 2)); return 0; }
	NB_PROP_SET(undetermined)			{ if (getBool(v, 2)) self(v)->Set3StateValue(wxCHK_UNDETERMINED); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, RadioButton, wxControl);

class NB_WxRadioButton : TNitClass<wxRadioButton>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(value),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, label: string, pos=null, size=null, style=0, validator=null, name=null)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "GROUP",			(int)wxRB_GROUP);
		newSlot(v, -1, "SINGLE",		(int)wxRB_SINGLE);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "RADIOBUTTON",	(int)wxEVT_COMMAND_RADIOBUTTON_SELECTED);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxControl* control = new wxRadioButton(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			getWxString(v, 4),
			*opt<wxPoint>(v, 5, wxDefaultPosition),
			*opt<wxSize>(v, 6, wxDefaultSize),
			optInt(v, 7, 0),
			*opt<wxValidator>(v, 8, (wxValidator*)&wxDefaultValidator),
			optWxString(v, 9, wxRadioButtonNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}

	NB_PROP_GET(value)					{ return push(v, self(v)->GetValue()); }

	NB_PROP_SET(value)					{ self(v)->SetValue(getBool(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, Gauge, wxControl);

class NB_WxGauge : TNitClass<wxGauge>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(bezelFace),
			PROP_ENTRY	(range),
			PROP_ENTRY	(shadowWidth),
			PROP_ENTRY	(value),
			PROP_ENTRY_R(vertical),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, range: int, pos=null, size=null, style=null, validator=null, name=null)"),
			FUNC_ENTRY_H(pulse,			"()"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "HORIZONTAL",	(int)wxGA_HORIZONTAL);
		newSlot(v, -1, "VERTICAL",		(int)wxGA_VERTICAL);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxControl* control = new wxGauge(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			getInt(v, 4),
			*opt<wxPoint>(v, 5, wxDefaultPosition),
			*opt<wxSize>(v, 6, wxDefaultSize),
			optInt(v, 7, wxGA_HORIZONTAL),
			*opt<wxValidator>(v, 8, (wxValidator*)&wxDefaultValidator),
			optWxString(v, 9, wxGaugeNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}

	NB_PROP_GET(bezelFace)				{ return push(v, self(v)->GetBezelFace()); }
	NB_PROP_GET(range)					{ return push(v, self(v)->GetRange()); }
	NB_PROP_GET(shadowWidth)			{ return push(v, self(v)->GetShadowWidth()); }
	NB_PROP_GET(value)					{ return push(v, self(v)->GetValue()); }
	NB_PROP_GET(vertical)				{ return push(v, self(v)->IsVertical()); }

	NB_PROP_SET(bezelFace)				{ self(v)->SetBezelFace(getInt(v, 2)); return 0; }
	NB_PROP_SET(range)					{ self(v)->SetRange(getInt(v, 2)); return 0; }
	NB_PROP_SET(shadowWidth)			{ self(v)->SetShadowWidth(getInt(v, 2)); return 0; }
	NB_PROP_SET(value)					{ self(v)->SetValue(getInt(v, 2)); return 0; }

	NB_FUNC(pulse)						{ self(v)->Pulse(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, Slider, wxControl);

class NB_WxSlider : TNitClass<wxSlider>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(lineSize),
			PROP_ENTRY_R(max),
			PROP_ENTRY_R(min),
			PROP_ENTRY	(pageSize),
			PROP_ENTRY_R(selEnd),
			PROP_ENTRY_R(selStart),
			PROP_ENTRY	(thumbLength),
			PROP_ENTRY  (tickFreq),
			PROP_ENTRY	(value),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, value, minValue, maxValue: int, pos=null, size=null, style=STYLE.HORIZONTAL, validator=null, name=null)"),
			FUNC_ENTRY_H(clearSel,		"()"),
			FUNC_ENTRY_H(clearTicks,	"()"),
			FUNC_ENTRY_H(setRange,		"(min, max: int)"),
			FUNC_ENTRY_H(setSelection,	"(start, end: int)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "HORIZONTAL",		(int)wxSL_HORIZONTAL);
		newSlot(v, -1, "VERTICAL",			(int)wxSL_VERTICAL);
		newSlot(v, -1, "AUTOTICKS",			(int)wxSL_AUTOTICKS);
		newSlot(v, -1, "MIN_MAX_LABELS",	(int)wxSL_MIN_MAX_LABELS);
		newSlot(v, -1, "VALUE_LABEL",		(int)wxSL_VALUE_LABEL);
		newSlot(v, -1, "LABELS",			(int)wxSL_LABELS);
		newSlot(v, -1, "LEFT",				(int)wxSL_LEFT);
		newSlot(v, -1, "RIGHT",				(int)wxSL_RIGHT);
		newSlot(v, -1, "TOP",				(int)wxSL_TOP);
		newSlot(v, -1, "BOTTOM",			(int)wxSL_BOTTOM);
		newSlot(v, -1, "SELRANGE",			(int)wxSL_SELRANGE);
		newSlot(v, -1, "INVERSE",			(int)wxSL_INVERSE);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "SCROLL_TOP",		(int)wxEVT_SCROLL_TOP);
		newSlot(v, -1, "SCROLL_BOTTOM",		(int)wxEVT_SCROLL_BOTTOM);
		newSlot(v, -1, "SCROLL_LINEUP",		(int)wxEVT_SCROLL_LINEUP);
		newSlot(v, -1, "SCROLL_LINEDOWN",	(int)wxEVT_SCROLL_LINEDOWN);
		newSlot(v, -1, "SCROLL_PAGEUP",		(int)wxEVT_SCROLL_PAGEUP);
		newSlot(v, -1, "SCROLL_PAGEDOWN",	(int)wxEVT_SCROLL_PAGEDOWN);
		newSlot(v, -1, "SCROLL_THUMBTRACK",	(int)wxEVT_SCROLL_THUMBTRACK);
		newSlot(v, -1, "SCROLL_THUMBRELEASE",	(int)wxEVT_SCROLL_THUMBRELEASE);
		newSlot(v, -1, "SCROLL_CHANGED",		(int)wxEVT_SCROLL_CHANGED);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxControl* control = new wxSlider(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			getInt(v, 4), // value
			getInt(v, 5), // min-value
			getInt(v, 6), // max-value
			*opt<wxPoint>(v, 7, wxDefaultPosition),
			*opt<wxSize>(v, 8, wxDefaultSize),
			optInt(v, 9, wxSL_HORIZONTAL),
			*opt<wxValidator>(v, 10, (wxValidator*)&wxDefaultValidator),
			optWxString(v, 11, wxGaugeNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}

	NB_PROP_GET(lineSize)				{ return push(v, self(v)->GetLineSize()); }
	NB_PROP_GET(max)					{ return push(v, self(v)->GetMax()); }
	NB_PROP_GET(min)					{ return push(v, self(v)->GetMin()); }
	NB_PROP_GET(pageSize)				{ return push(v, self(v)->GetPageSize()); }
	NB_PROP_GET(selEnd)					{ return push(v, self(v)->GetSelEnd()); }
	NB_PROP_GET(selStart)				{ return push(v, self(v)->GetSelStart()); }
	NB_PROP_GET(thumbLength)			{ return push(v, self(v)->GetThumbLength()); }
	NB_PROP_GET(tickFreq)				{ return push(v, self(v)->GetTickFreq()); }
	NB_PROP_GET(value)					{ return push(v, self(v)->GetValue()); }

	NB_PROP_SET(lineSize)				{ self(v)->SetLineSize(getInt(v, 2)); return 0; }
	NB_PROP_SET(pageSize)				{ self(v)->SetPageSize(getInt(v, 2)); return 0; }
	NB_PROP_SET(thumbLength)			{ self(v)->SetThumbLength(getInt(v, 2)); return 0; }
    NB_PROP_SET(tickFreq)				{ self(v)->SetTickFreq(getInt(v, 2)); return 0; }
	NB_PROP_SET(value)					{ self(v)->SetValue(getInt(v, 2)); return 0; }

	NB_FUNC(clearSel)					{ self(v)->ClearSel(); return 0; }
	NB_FUNC(clearTicks)					{ self(v)->ClearTicks(); return 0; }
	NB_FUNC(setRange)					{ self(v)->SetRange(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(setSelection)				{ self(v)->SetSelection(getInt(v, 2), getInt(v, 3)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, ScrollBar, wxControl);

class NB_WxScrollBar : TNitClass<wxScrollBar>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(pageSize),
			PROP_ENTRY_R(thumbSize),
			PROP_ENTRY	(thumbPosition),
			PROP_ENTRY_R(range),
			PROP_ENTRY_R(vertical),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, pos=null, size=null, style=STYLE.HORIZONTAL, validator=null, name=null)"),
			FUNC_ENTRY_H(setScrollbar,	"(position: int, thumbSize: int, range: int, pageSize: int, refresh=true)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "HORIZONTAL",		(int)wxSB_HORIZONTAL);
		newSlot(v, -1, "VERTICAL",			(int)wxSB_VERTICAL);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "SCROLL_TOP",			(int)wxEVT_SCROLL_TOP);
		newSlot(v, -1, "SCROLL_BOTTOM",			(int)wxEVT_SCROLL_BOTTOM);
		newSlot(v, -1, "SCROLL_LINEUP",			(int)wxEVT_SCROLL_LINEUP);
		newSlot(v, -1, "SCROLL_LINEDOWN",		(int)wxEVT_SCROLL_LINEDOWN);
		newSlot(v, -1, "SCROLL_PAGEUP",			(int)wxEVT_SCROLL_PAGEUP);
		newSlot(v, -1, "SCROLL_PAGEDOWN",		(int)wxEVT_SCROLL_PAGEDOWN);
		newSlot(v, -1, "SCROLL_THUMBTRACK",		(int)wxEVT_SCROLL_THUMBTRACK);
		newSlot(v, -1, "SCROLL_THUMBRELEASE",	(int)wxEVT_SCROLL_THUMBRELEASE);
		newSlot(v, -1, "SCROLL_CHANGED",		(int)wxEVT_SCROLL_CHANGED);
		sq_poptop(v);
	}

	NB_PROP_GET(pageSize)				{ return push(v, self(v)->GetPageSize()); }
	NB_PROP_GET(thumbSize)				{ return push(v, self(v)->GetThumbSize()); }
	NB_PROP_GET(thumbPosition)			{ return push(v, self(v)->GetThumbPosition()); }
	NB_PROP_GET(range)					{ return push(v, self(v)->GetRange()); }
	NB_PROP_GET(vertical)				{ return push(v, self(v)->IsVertical()); }
	
	NB_PROP_SET(thumbPosition)			{ self(v)->SetThumbPosition(getInt(v, 2)); return 0; }

	NB_CONS()
	{
		type* self = new type(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			*opt<wxPoint>(v, 4, wxDefaultPosition),
			*opt<wxSize>(v, 5, wxDefaultSize),
			optInt(v, 6, wxSB_HORIZONTAL),
			*opt<wxValidator>(v, 7, (wxValidator*)&wxDefaultValidator),
			optWxString(v, 8, wxScrollBarNameStr)
			);

		setSelf(v, self);
		return SQ_OK;
	}

	NB_FUNC(setScrollbar)				{ self(v)->SetScrollbar(getInt(v, 2), getInt(v, 3), getInt(v, 4), getInt(v, 5), optBool(v, 6, true)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, SearchCtrl, wxControl);

class NB_WxSearchCtrl : TNitClass<wxSearchCtrl>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(menu),
			PROP_ENTRY	(searchButtonVisible),
			PROP_ENTRY	(cancelButtonVisible),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, value=\"\", pos=null, size=null, style=0, validator=null, name=null)"),
			NULL
		};

		bind(v, props, funcs);

		NB_WxTextEntryBase::Register<type>(v);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "PROCESS_ENTER",	(int)wxTE_PROCESS_ENTER);
		newSlot(v, -1, "PROCESS_TAB",	(int)wxTE_PROCESS_TAB);
		newSlot(v, -1, "NOHIDESEL",		(int)wxTE_NOHIDESEL);
		newSlot(v, -1, "LEFT",			(int)wxTE_LEFT);
		newSlot(v, -1, "CENTER",		(int)wxTE_CENTER);
		newSlot(v, -1, "RIGHT",			(int)wxTE_RIGHT);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "SEARCH_BTN",	(int)wxEVT_COMMAND_SEARCHCTRL_SEARCH_BTN);
		newSlot(v, -1, "CANCEL_BTN",	(int)wxEVT_COMMAND_SEARCHCTRL_CANCEL_BTN);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxControl* control = new wxSearchCtrl(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			optWxString(v, 4, ""),
			*opt<wxPoint>(v, 5, wxDefaultPosition),
			*opt<wxSize>(v, 6, wxDefaultSize),
			optInt(v, 7, 0),
			*opt<wxValidator>(v, 8, (wxValidator*)&wxDefaultValidator),
			optWxString(v, 9, wxSearchCtrlNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}

	NB_PROP_GET(menu)					{ return push(v, self(v)->GetMenu()); }
	NB_PROP_GET(searchButtonVisible)	{ return push(v, self(v)->IsSearchButtonVisible()); }
	NB_PROP_GET(cancelButtonVisible)	{ return push(v, self(v)->IsSearchButtonVisible()); }

	NB_PROP_SET(menu)					{ self(v)->SetMenu(opt<wxMenu>(v, 2, NULL)); return 0; }
	NB_PROP_SET(searchButtonVisible)	{ self(v)->ShowSearchButton(getBool(v, 2)); return 0; }
	NB_PROP_SET(cancelButtonVisible)	{ self(v)->ShowCancelButton(getBool(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

typedef wxColourPickerCtrl wxColorPickerCtrl;

NB_TYPE_WXOBJ(NITWX_API, ColorPickerCtrl, wxControl);

class NB_WxColorPickerCtrl : TNitClass<wxColorPickerCtrl>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(color),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, color = Color.BLACK, pos=null, size=null, style=STYLE.DEFAULT, validator=null, name=null)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "DEFAULT",			(int)wxCLRP_DEFAULT_STYLE);
		newSlot(v, -1, "USE_TEXTCTRL",		(int)wxCLRP_USE_TEXTCTRL);
		newSlot(v, -1, "SHOW_LABEL",		(int)wxCLRP_SHOW_LABEL);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "CHANGED",			(int)wxEVT_COMMAND_COLOURPICKER_CHANGED);
		sq_poptop(v);
	}

	NB_PROP_GET(color)					{ return PushWxColor(v, self(v)->GetColour()); }

	NB_PROP_SET(color)					{ self(v)->SetColour(GetWxColor(v, 2)); return 0; }

	NB_CONS()
	{
		wxControl* control = new wxColourPickerCtrl(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			OptWxColor(v, 4, *wxBLACK),
			*opt<wxPoint>(v, 5, wxDefaultPosition),
			*opt<wxSize>(v, 6, wxDefaultSize),
			optInt(v, 7, 0),
			*opt<wxValidator>(v, 8, (wxValidator*)&wxDefaultValidator),
			optWxString(v, 9, wxColourPickerCtrlNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, FontPickerCtrl, wxControl);

class NB_WxFontPickerCtrl : TNitClass<wxFontPickerCtrl>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(maxPointSize),
			PROP_ENTRY	(selectedFont),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, font: wxFont = null, pos=null, size=null, style=STYLE.DEFAULT, validator=null, name=null)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "DEFAULT",			(int)wxFNTP_DEFAULT_STYLE);
		newSlot(v, -1, "USE_TEXTCTRL",		(int)wxFNTP_USE_TEXTCTRL);
		newSlot(v, -1, "FONTDESC_AS_LABEL",	(int)wxFNTP_FONTDESC_AS_LABEL);
		newSlot(v, -1, "USEFONT_FOR_LABEL",	(int)wxFNTP_USEFONT_FOR_LABEL);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "CHANGED",			(int)wxEVT_COMMAND_COLOURPICKER_CHANGED);
		sq_poptop(v);
	}

	NB_PROP_GET(maxPointSize)			{ return push(v, self(v)->GetMaxPointSize()); }
	NB_PROP_GET(selectedFont)			{ return push(v, self(v)->GetSelectedFont()); }

	NB_PROP_SET(maxPointSize)			{ self(v)->SetMaxPointSize(getInt(v, 2)); return 0; }
	NB_PROP_SET(selectedFont)			{ self(v)->SetSelectedFont(*get<wxFont>(v, 2)); return 0; }

	NB_CONS()
	{
		wxControl* control = new wxFontPickerCtrl(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			*opt<wxFont>(v, 4, wxNullFont),
			*opt<wxPoint>(v, 5, wxDefaultPosition),
			*opt<wxSize>(v, 6, wxDefaultSize),
			optInt(v, 7, 0),
			*opt<wxValidator>(v, 8, (wxValidator*)&wxDefaultValidator),
			optWxString(v, 9, wxColourPickerCtrlNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, FilePickerCtrl, wxControl);

class NB_WxFilePickerCtrl : TNitClass<wxFilePickerCtrl>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(fileName),
			PROP_ENTRY	(path),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, path=\"\", msg=null, wildcard=null, pos=null, size=null, style=STYLE.DEFAULT, validator=null, name=null)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "DEFAULT",			(int)wxFLP_DEFAULT_STYLE);
		newSlot(v, -1, "USE_TEXTCTRL",		(int)wxFLP_USE_TEXTCTRL);
		newSlot(v, -1, "OPEN",				(int)wxFLP_OPEN);
		newSlot(v, -1, "SAVE",				(int)wxFLP_SAVE);
		newSlot(v, -1, "OVERWRITE_PROMPT",	(int)wxFLP_OVERWRITE_PROMPT);
		newSlot(v, -1, "FILE_MUST_EXIST",	(int)wxFLP_FILE_MUST_EXIST);
		newSlot(v, -1, "CHANGE_DIR",		(int)wxFLP_CHANGE_DIR);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "CHANGED",			(int)wxEVT_COMMAND_FILEPICKER_CHANGED);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxControl* control = new wxFilePickerCtrl(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			optWxString(v, 4, ""),
			optWxString(v, 5, wxFileSelectorPromptStr),
			optWxString(v, 6, wxFileSelectorDefaultWildcardStr),
			*opt<wxPoint>(v, 7, wxDefaultPosition),
			*opt<wxSize>(v, 8, wxDefaultSize),
			optInt(v, 9, 0),
			*opt<wxValidator>(v, 10, (wxValidator*)&wxDefaultValidator),
			optWxString(v, 11, wxFilePickerCtrlNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}

	// TODO: wxFileName binding

	NB_PROP_GET(fileName)				{ return push(v, self(v)->GetFileName().GetFullPath()); }
	NB_PROP_GET(path)					{ return push(v, self(v)->GetPath()); }

	NB_PROP_SET(fileName)				{ self(v)->SetFileName(getWxString(v, 2)); return 0; }
	NB_PROP_SET(path)					{ self(v)->SetPath(getWxString(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, DirPickerCtrl, wxControl);

class NB_WxDirPickerCtrl : TNitClass<wxDirPickerCtrl>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(dirName),
			PROP_ENTRY	(path),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, path=\"\", msg=null, pos=null, size=null, style=STYLE.DEFAULT, validator=null, name=null)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "DEFAULT",		(int)wxDIRP_DEFAULT_STYLE);
		newSlot(v, -1, "USE_TEXTCTRL",	(int)wxDIRP_USE_TEXTCTRL);
		newSlot(v, -1, "DIR_MUST_EXIST",(int)wxDIRP_DIR_MUST_EXIST);
		newSlot(v, -1, "CHANGE_DIR",	(int)wxDIRP_CHANGE_DIR);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "CHANGED",		(int)wxEVT_COMMAND_DIRPICKER_CHANGED);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxControl* control = new wxDirPickerCtrl(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			optWxString(v, 4, ""),
			optWxString(v, 5, wxDirSelectorPromptStr),
			*opt<wxPoint>(v, 6, wxDefaultPosition),
			*opt<wxSize>(v, 7, wxDefaultSize),
			optInt(v, 8, 0),
			*opt<wxValidator>(v, 9, (wxValidator*)&wxDefaultValidator),
			optWxString(v, 10, wxFilePickerCtrlNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}

	NB_PROP_GET(dirName)				{ return push(v, self(v)->GetDirName().GetFullPath()); }
	NB_PROP_GET(path)					{ return push(v, self(v)->GetPath()); }

	NB_PROP_SET(dirName)				{ self(v)->SetDirName(getWxString(v, 2)); return 0; }
	NB_PROP_SET(path)					{ self(v)->SetPath(getWxString(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, GenericDirCtrl, wxControl);

class NB_WxGenericDirCtrl : TNitClass<wxGenericDirCtrl>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(defaultPath),
			PROP_ENTRY_R(filePath),
			PROP_ENTRY_R(filePaths),
			PROP_ENTRY	(path),
			PROP_ENTRY_R(paths),
			PROP_ENTRY	(filter),
			PROP_ENTRY	(filterIndex),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, dir=null, pos=null, size=null, style=STYLE.DEFAULT, filter=\"\", defaultFilter=0, name=null)"),
			FUNC_ENTRY_H(collapsePath,	"(path: string): bool"),
			FUNC_ENTRY_H(collapseTree,	"()"),
			FUNC_ENTRY_H(expandPath,	"(path: string): bool"),
			FUNC_ENTRY_H(reCreateTree,	"()"),
			FUNC_ENTRY_H(showHidden,	"(show: bool)"),
			FUNC_ENTRY_H(selectPath,	"(path: string, select=true)"),
			FUNC_ENTRY_H(selectPaths,	"(paths: string[])"),
			FUNC_ENTRY_H(unselectAll,	"()"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "DEFAULT",		(int)wxDIRCTRL_3D_INTERNAL);
		newSlot(v, -1, "DIR_ONLY",		(int)wxDIRCTRL_DIR_ONLY);
		newSlot(v, -1, "SELECT_FIRST",	(int)wxDIRCTRL_SELECT_FIRST);
		newSlot(v, -1, "EDIT_LABELS",	(int)wxDIRCTRL_EDIT_LABELS);
		newSlot(v, -1, "MULTIPLE",		(int)wxDIRCTRL_MULTIPLE);
		sq_poptop(v);
	}

	NB_PROP_GET(defaultPath)			{ return push(v, self(v)->GetDefaultPath()); }
	NB_PROP_GET(filePath)				{ return push(v, self(v)->GetFilePath()); }
	NB_PROP_GET(filePaths)				
	{ 
		sq_newarray(v, 0); 
		wxArrayString paths; 
		self(v)->GetFilePaths(paths); 
		for (uint i=0; i < paths.size(); ++i)
			arrayAppend(v, -1, paths[i]);
		return 1;
	}
	NB_PROP_GET(path)					{ return push(v, self(v)->GetPath()); }
	NB_PROP_GET(paths)
	{
		sq_newarray(v, 0);
		wxArrayString paths; 
		self(v)->GetPaths(paths); 
		for (uint i=0; i < paths.size(); ++i)
			arrayAppend(v, -1, paths[i]);
		return 1;
	}
	NB_PROP_GET(filter)					{ return push(v, self(v)->GetFilter()); }
	NB_PROP_GET(filterIndex)			{ return push(v, self(v)->GetFilterIndex()); }
	
	NB_PROP_SET(defaultPath)			{ self(v)->SetDefaultPath(getWxString(v, 2)); return 0; }
	NB_PROP_SET(path)					{ self(v)->SetPath(getWxString(v, 2)); return 0; }
	NB_PROP_SET(filter)					{ self(v)->SetFilter(getWxString(v, 2)); return 0; }
	NB_PROP_SET(filterIndex)			{ self(v)->SetFilterIndex(getInt(v, 2)); return 0; }

	NB_CONS()
	{
		wxControl* control = new wxGenericDirCtrl(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			optWxString(v, 4, wxDirDialogDefaultFolderStr),
			*opt<wxPoint>(v, 5, wxDefaultPosition),
			*opt<wxSize>(v, 6, wxDefaultSize),
			optInt(v, 7, wxDIRCTRL_3D_INTERNAL),
			optWxString(v, 8, wxEmptyString),
			optInt(v, 9, 0),
			optWxString(v, 10, wxTreeCtrlNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}

	NB_FUNC(collapsePath)				{ return push(v, self(v)->CollapsePath(getWxString(v, 2))); }
	NB_FUNC(collapseTree)				{ self(v)->CollapseTree(); return 0; }
	NB_FUNC(expandPath)					{ return push(v, self(v)->ExpandPath(getWxString(v, 2))); }
	NB_FUNC(reCreateTree)				{ self(v)->ReCreateTree(); return 0; }
	NB_FUNC(showHidden)					{ self(v)->ShowHidden(getBool(v, 2)); return 0; }
	NB_FUNC(selectPath)					{ self(v)->SelectPath(getWxString(v, 2), optBool(v, 3, true)); return 0; }
	NB_FUNC(selectPaths)
	{
		if (sq_gettype(v, 2) != OT_ARRAY)
			return sq_throwerror(v, "array expected");
		wxArrayString paths;
		for (NitIterator itr(v, 2); itr.hasNext(); itr.next())
			paths.push_back(getWxString(v, 2));
		self(v)->SelectPaths(paths);
		return 0;
	}
	NB_FUNC(unselectAll)				{ self(v)->UnselectAll(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, HyperlinkCtrl, wxControl);

class NB_WxHyperlinkCtrl : TNitClass<wxHyperlinkCtrl>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(hoverColor),
			PROP_ENTRY	(normalColor),
			PROP_ENTRY	(url),
			PROP_ENTRY	(visited),
			PROP_ENTRY	(visitedColor),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, label: string, url: string, pos=null, size=null, style=STYLE.DEFAULT, filter=\"\", defaultFilter=0, name=null)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "DEFAULT",		(int)wxHL_DEFAULT_STYLE);
		newSlot(v, -1, "ALIGN_LEFT",	(int)wxHL_ALIGN_LEFT);
		newSlot(v, -1, "ALIGN_RIGHT",	(int)wxHL_ALIGN_RIGHT);
		newSlot(v, -1, "ALIGN_CENTER",	(int)wxHL_ALIGN_CENTRE);
		newSlot(v, -1, "CONTEXTMENU",	(int)wxHL_CONTEXTMENU);
		sq_poptop(v);
	}

	NB_PROP_GET(hoverColor)				{ return PushWxColor(v, self(v)->GetHoverColour()); }
	NB_PROP_GET(normalColor)			{ return PushWxColor(v, self(v)->GetNormalColour()); }
	NB_PROP_GET(url)					{ return push(v, self(v)->GetURL()); }
	NB_PROP_GET(visited)				{ return push(v, self(v)->GetVisited()); }
	NB_PROP_GET(visitedColor)			{ return PushWxColor(v, self(v)->GetVisitedColour()); }

	NB_PROP_SET(hoverColor)				{ self(v)->SetHoverColour(GetWxColor(v, 2)); return 0; }
	NB_PROP_SET(normalColor)			{ self(v)->SetNormalColour(GetWxColor(v, 2)); return 0; }
	NB_PROP_SET(url)					{ self(v)->SetURL(getWxString(v, 2)); return 0; }
	NB_PROP_SET(visited)				{ self(v)->SetVisited(getBool(v, 2)); return 0; }
	NB_PROP_SET(visitedColor)			{ self(v)->SetVisitedColour(GetWxColor(v, 2)); return 0; }

	NB_CONS()
	{
		wxControl* control = new wxHyperlinkCtrl(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			getWxString(v, 4),
			getWxString(v, 5),
			*opt<wxPoint>(v, 6, wxDefaultPosition),
			*opt<wxSize>(v, 7, wxDefaultSize),
			optInt(v, 8, wxHL_DEFAULT_STYLE),
			optWxString(v, 9, wxHyperlinkCtrlNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, CommandLinkButton, wxButton);

class NB_WxCommandLinkButton : TNitClass<wxCommandLinkButton>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(label),
			PROP_ENTRY	(note),
			PROP_ENTRY	(mainLabel),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, mainLabel=\"\", note=\"\", pos=null, size=null, style=0, validator=null, name=null)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(label)					{ return push(v, self(v)->GetLabel()); }
	NB_PROP_GET(note)					{ return push(v, self(v)->GetNote()); }
	NB_PROP_GET(mainLabel)				{ return push(v, self(v)->GetMainLabel()); }

	NB_PROP_SET(label)					{ self(v)->SetLabel(getWxString(v, 2)); return 0; }
	NB_PROP_SET(note)					{ self(v)->SetNote(getWxString(v, 2)); return 0; }
	NB_PROP_SET(mainLabel)				{ self(v)->SetMainLabel(getWxString(v, 2)); return 0; }

	NB_CONS()
	{
		wxControl* control = new wxCommandLinkButton(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			optWxString(v, 4, wxEmptyString),
			optWxString(v, 5, wxEmptyString),
			*opt<wxPoint>(v, 6, wxDefaultPosition),
			*opt<wxSize>(v, 7, wxDefaultSize),
			optInt(v, 8, 0),
			*opt<wxValidator>(v, 9, wxDefaultValidator),
			optWxString(v, 10, wxButtonNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, BannerWindow, wxWindow);

class NB_WxBannerWindow : TNitClass<wxBannerWindow>
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
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, dir=wx.DIR.LEFT, pos=null, size=null, style=0, name=null)"),
			FUNC_ENTRY_H(setBitmap,		"(bitmap: wx.Bitmap)"),
			FUNC_ENTRY_H(setText,		"(title: string, message: string)"),
			FUNC_ENTRY_H(setGradient,	"(start: Color, end: Color)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		type* self = new wxBannerWindow(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			(wxDirection)optInt(v, 4, wxLEFT),
			*opt<wxPoint>(v, 5, wxDefaultPosition),
			*opt<wxSize>(v, 6, wxDefaultSize),
			optInt(v, 7, 0),
			optWxString(v, 8, wxBannerWindowNameStr)
			);

		sq_setinstanceup(v, 1, self);
		return 0;
	}

	NB_FUNC(setBitmap)					{ self(v)->SetBitmap(*opt<wxBitmap>(v, 2, wxNullBitmap)); return 0; }
	NB_FUNC(setText)					{ self(v)->SetText(getWxString(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(setGradient)				{ self(v)->SetGradient(GetWxColor(v, 2), GetWxColor(v, 3)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, InfoBar, wxControl);

class NB_WxInfoBar : TNitClass<wxInfoBar>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(showEffect),
			PROP_ENTRY_R(hideEffect),
			PROP_ENTRY	(effectDuration),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: wx.Window=null, id=wx.ID.ANY)"),
			FUNC_ENTRY_H(addButton,		"(id: int, label=\"\")"),
			FUNC_ENTRY_H(dismiss,		"()"),
			FUNC_ENTRY_H(removeButton,	"(id: int)"),
			FUNC_ENTRY_H(showMessage,	"(msg: string, flags=wx.ICON.NONE)"),
			FUNC_ENTRY_H(setShowHideEffects,	"(show: wx.SHOW_EFFECT, hide: wx.SHOW_EFFECT)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(showEffect)				{ return push(v, (int)self(v)->GetShowEffect()); }
	NB_PROP_GET(hideEffect)				{ return push(v, (int)self(v)->GetHideEffect()); }
	NB_PROP_GET(effectDuration)			{ return push(v, self(v)->GetEffectDuration()); }

	NB_PROP_SET(effectDuration)			{ self(v)->SetEffectDuration(getInt(v, 2)); return 0; }

	NB_CONS()
	{
		type* self = new wxInfoBar(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY)
			);

		setSelf(v, self);
		return SQ_OK;
	}

	NB_FUNC(addButton)					{ self(v)->AddButton(getInt(v, 2), optWxString(v, 3, wxEmptyString)); return 0; }
	NB_FUNC(dismiss)					{ self(v)->Dismiss(); return 0; }
	NB_FUNC(removeButton)				{ self(v)->RemoveButton(getInt(v, 2)); return 0; }
	NB_FUNC(showMessage)				{ self(v)->ShowMessage(getWxString(v, 2), optInt(v, 3, wxICON_NONE)); return 0; }
	NB_FUNC(setShowHideEffects)			{ self(v)->SetShowHideEffects((wxShowEffect)getInt(v, 2), (wxShowEffect)getInt(v, 3)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WX_AUTODELETE(NITWX_API, RichToolTip, NULL, delete);

class NB_WxRichToolTip : TNitClass<wxRichToolTip>
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
			CONS_ENTRY_H(				"(title: string, message: string)"),
			FUNC_ENTRY_H(showFor,		"(win: wx.Window)"),
			FUNC_ENTRY_H(setBackColor,	"(col: Color, end: Color=null)"),
			FUNC_ENTRY_H(setTimeout,	"(milliseconds: int)"),
			FUNC_ENTRY_H(setTipKind,	"(tipKind: TIP_KIND)"),
			FUNC_ENTRY_H(setTitleFont,	"(font: wx.Font)"),
			FUNC_ENTRY_H(setIcon,		"(icon=wx.ICON.INFORMATION)"
			"\n"						"(icon: wx.Icon)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "TIP_KIND");
		newSlot(v, -1, "NONE",			(int)wxTipKind_None);
		newSlot(v, -1, "TOP_LEFT",		(int)wxTipKind_TopLeft);
		newSlot(v, -1, "TOP",			(int)wxTipKind_Top);
		newSlot(v, -1, "TOP_RIGHT",		(int)wxTipKind_TopRight);
		newSlot(v, -1, "BOTTOM_LEFT",	(int)wxTipKind_BottomLeft);
		newSlot(v, -1, "BOTTOM",		(int)wxTipKind_Bottom);
		newSlot(v, -1, "BOTTOM_RIGHT",	(int)wxTipKind_BottomRight);
		newSlot(v, -1, "AUTO",			(int)wxTipKind_Auto);
		sq_poptop(v);
	}

	NB_CONS()							{ setSelf(v, new wxRichToolTip(getWxString(v, 2), getWxString(v, 3))); return SQ_OK; }
	NB_FUNC(showFor)					{ self(v)->ShowFor(get<wxWindow>(v, 2)); return 0; }
	NB_FUNC(setBackColor)				{ self(v)->SetBackgroundColour(GetWxColor(v, 2), OptWxColor(v, 3, wxColour())); return 0; }
	NB_FUNC(setTimeout)					{ self(v)->SetTimeout(getInt(v, 2)); return 0; }
	NB_FUNC(setTipKind)					{ self(v)->SetTipKind((wxTipKind)getInt(v, 2)); return 0; }
	NB_FUNC(setTitleFont)				{ self(v)->SetTitleFont(*get<wxFont>(v, 2)); return 0; }
	NB_FUNC(setIcon)					
	{ 
		if (is<wxIcon>(v, 2))
			self(v)->SetIcon(*get<wxIcon>(v, 2));
		else
			self(v)->SetIcon(getInt(v, 2));
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, EditableListBox, wxPanel);

class NB_WxEditableListBox : TNitClass<wxEditableListBox>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(strings),
			PROP_ENTRY_R(listCtrl),
			PROP_ENTRY_R(delButton),
			PROP_ENTRY_R(newButton),
			PROP_ENTRY_R(upButton),
			PROP_ENTRY_R(downButton),
			PROP_ENTRY_R(editButton),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, label: string, pos=null, size=null, style=STYLE.DEFAULT, name=null)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "DEFAULT",		(int)wxEL_DEFAULT_STYLE);
		newSlot(v, -1, "ALLOW_NEW",		(int)wxEL_ALLOW_NEW);
		newSlot(v, -1, "ALLOW_EDIT",	(int)wxEL_ALLOW_EDIT);
		newSlot(v, -1, "ALLOW_DELETE",	(int)wxEL_ALLOW_DELETE);
		newSlot(v, -1, "NO_REORDER",	(int)wxEL_NO_REORDER);
		sq_poptop(v);
	}

	NB_PROP_GET(strings)
	{
		wxArrayString strings;
		self(v)->GetStrings(strings);
		return PushWxArray(v, strings);
	}

	NB_PROP_SET(strings)
	{
		wxArrayString strings;
		GetWxArrayString(v, 2, strings);
		self(v)->SetStrings(strings);
		return 0;
	}

	NB_PROP_GET(listCtrl)				{ return push(v, (wxButton*)self(v)->GetListCtrl()); }
	NB_PROP_GET(delButton)				{ return push(v, (wxButton*)self(v)->GetDelButton()); }
	NB_PROP_GET(newButton)				{ return push(v, (wxButton*)self(v)->GetNewButton()); }
	NB_PROP_GET(upButton)				{ return push(v, (wxButton*)self(v)->GetUpButton()); }
	NB_PROP_GET(downButton)				{ return push(v, (wxButton*)self(v)->GetDownButton()); }
	NB_PROP_GET(editButton)				{ return push(v, (wxButton*)self(v)->GetEditButton()); }

	NB_CONS()
	{
		type* self = new type(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			getWxString(v, 3),
			*opt<wxPoint>(v, 4, wxDefaultPosition),
			*opt<wxSize>(v, 5, wxDefaultSize),
			optInt(v, 6, wxEL_DEFAULT_STYLE),
			optWxString(v, 7, wxEditableListBoxNameStr)
			);

		setSelf(v, self);
		return SQ_OK;
	}
};

////////////////////////////////////////////////////////////////////////////////

class NB_WxItemContainerImmutable : TNitInterface<wxItemContainerImmutable>
{
public:
	template <typename TClass>
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(count),
			PROP_ENTRY_R(empty),
			PROP_ENTRY_R(strings),
			PROP_ENTRY	(selection),
			PROP_ENTRY	(stringSelection),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(getString,			"(n: int): string"),
			FUNC_ENTRY_H(setString,			"(n: int, str: string)"),
			FUNC_ENTRY_H(findString,		"(str: string, caseSensitive=false): int"),
			FUNC_ENTRY_H(select,			"(n: int)"),
			FUNC_ENTRY_H(setStringSelection,"(str: string): bool // true if string selected"),
			NULL
		};

		bind<TClass>(v, props, funcs);
	}

	NB_PROP_GET(count)					{ return push(v, self(v)->GetCount()); }
	NB_PROP_GET(empty)					{ return push(v, self(v)->IsEmpty()); }
	NB_PROP_GET(selection)				{ return push(v, self(v)->GetSelection()); }
	NB_PROP_GET(stringSelection)		{ return push(v, self(v)->GetStringSelection()); }
	NB_PROP_GET(strings)				{ return PushWxArray(v, self(v)->GetStrings()); }

	NB_PROP_SET(selection)				{ self(v)->SetSelection(getInt(v, 2)); return 0; }
	NB_PROP_SET(stringSelection)		{ self(v)->SetStringSelection(getWxString(v, 2)); return 0; }

	NB_FUNC(getString)					{ return push(v, self(v)->GetString(getInt(v, 2))); }
	NB_FUNC(setString)					{ self(v)->SetString(getInt(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(findString)					{ return push(v, self(v)->FindString(getWxString(v, 2), optBool(v, 3, false))); }
	NB_FUNC(select)						{ self(v)->Select(getInt(v, 2)); return 0; }
	NB_FUNC(setStringSelection)			{ return push(v, self(v)->SetStringSelection(getWxString(v, 2))); }
};

////////////////////////////////////////////////////////////////////////////////

class NB_WxItemContainer : TNitInterface<wxItemContainer>
{
public:
	template <typename TClass>
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(clear,				"()"),
			FUNC_ENTRY_H(delete,			"(index: int)"),
			FUNC_ENTRY_H(append,			"(item: string, clientObject=null): int\n"
											"(items: string[], clientObjects: object[]=null): int"),
			FUNC_ENTRY_H(insert,			"(item: string, pos: int, clientObject=null): int\n"
											"(items: string[], pos: int, clientObjects: object[]=null): int"),
			FUNC_ENTRY_H(set,				"(items: string[], clientObjects: object[]=null): int"),
			FUNC_ENTRY_H(getClientObject,	"(index: int)"),
			FUNC_ENTRY_H(setClientObject,	"(index: int, clientObject: object)"),
			NULL
		};

		NB_WxItemContainerImmutable::Register<TClass>(v);

		bind<TClass>(v, props, funcs);
	}

	NB_FUNC(clear)						{ self(v)->Clear(); return 0; }
	NB_FUNC(delete)						{ self(v)->Delete(getInt(v, 2)); return 0; }

	NB_FUNC(getClientObject)			{ return PushWxScriptData(v, self(v)->GetClientObject(getInt(v, 2))); }

	NB_FUNC(setClientObject)
	{
		type* o = self(v);
		int idx = getInt(v, 2);
		wxClientData* obj = o->GetClientObject(idx);
		wxClientData* nobj = GetWxScriptData(v, 3, obj);

		if (obj != nobj)
			o->SetClientObject(idx, nobj);
		return 0;
	}

	// TODO: Handle when items.size() != objects.size()

	NB_FUNC(append)
	{
		if (sq_gettype(v, 2) == OT_ARRAY)
		{
			wxArrayString items;
			GetWxArrayString(v, 2, items);

			if (isNone(v, 3))
				return push(v, self(v)->Append(items));
			else if (sq_gettype(v, 3) != OT_ARRAY)
				return sq_throwerror(v, "invalid array of client objects");
			vector<wxClientData*>::type objects;
			GetWxArrayScriptData(v, 3, objects);
			return push(v, self(v)->Append(items, &*objects.begin()));
		}
		else
		{
			if (isNone(v, 3))
				return push(v, self(v)->Append(getWxString(v, 2)));
			else
				return push(v, self(v)->Append(getWxString(v, 2), GetWxScriptData(v, 3, NULL)));
		}
	}

	NB_FUNC(insert)
	{
		if (sq_gettype(v, 2) == OT_ARRAY)
		{
			wxArrayString items;
			GetWxArrayString(v, 2, items);

			if (isNone(v, 4))
				return push(v, self(v)->Insert(items, getInt(v, 3)));
			else if (sq_gettype(v, 4) != OT_ARRAY)
				return sq_throwerror(v, "invalid array of client objects");
			vector<wxClientData*>::type objects;
			GetWxArrayScriptData(v, 4, objects);
			return push(v, self(v)->Insert(items, getInt(v, 3), &*objects.begin()));
		}
		else
		{
			if (isNone(v, 4))
				return push(v, self(v)->Insert(getWxString(v, 2), getInt(v, 3)));
			else
				return push(v, self(v)->Insert(getWxString(v, 2), getInt(v, 3), GetWxScriptData(v, 4, NULL)));
		}
	}

	NB_FUNC(set)
	{
		if (sq_gettype(v, 2) != OT_ARRAY)
			return sq_throwerror(v, "invalid args");

		wxArrayString items;
		GetWxArrayString(v, 2, items);

		if (isNone(v, 3))
		{
			self(v)->Set(items);
			return 0;
		}
		else if (sq_gettype(v, 3) != OT_ARRAY)
			return sq_throwerror(v, "invalid array of client objects");

		if (items.size() == 0)
		{
			self(v)->Set(0, NULL);
			return 0;
		}

		if (sq_getsize(v, 3) != (int)items.size())
			return sq_throwerror(v, "item count mismatch");

		vector<wxClientData*>::type objects;
		GetWxArrayScriptData(v, 3, objects);
		self(v)->Set(items, &*objects.begin());
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, Choice, wxControl);

class NB_WxChoice : TNitClass<wxChoice>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(columns),
			PROP_ENTRY_R(currentSelection),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, pos=null, size=null, choices: string[]=null, style=0, validator=null, name=null)"),
			NULL
		};

		bind(v, props, funcs);

		NB_WxItemContainer::Register<type>(v);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "SORT",			(int)wxCB_SORT);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "CHOICE",		(int)wxEVT_COMMAND_CHOICE_SELECTED);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxArrayString choices;

		if (!isNone(v, 6))
		{
			if (sq_gettype(v, 6) != OT_ARRAY)
				return sq_throwerror(v, "invalid choice array");

			GetWxArrayString(v, 6, choices);
		}

		wxControl* control = new wxChoice(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			*opt<wxPoint>(v, 4, wxDefaultPosition),
			*opt<wxSize>(v, 5, wxDefaultSize),
			choices,
			optInt(v, 7, 0),
			*opt<wxValidator>(v, 8, (wxValidator*)&wxDefaultValidator),
			optWxString(v, 9, wxChoiceNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}

	NB_PROP_GET(columns)				{ return push(v, self(v)->GetColumns()); }
	NB_PROP_GET(currentSelection)		{ return push(v, self(v)->GetCurrentSelection()); }

	NB_PROP_SET(columns)				{ self(v)->SetColumns(getInt(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, ComboBox, wxChoice);

class NB_WxComboBox : TNitClass<wxComboBox>
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
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, value=\"\", pos=null, size=null, choices: string[]=null, style=0, validator=null, name=null)"),
			FUNC_ENTRY_H(popup,			"()"),
			FUNC_ENTRY_H(dismiss,		"()"),
			NULL
		};

		bind(v, props, funcs);

		NB_WxTextEntryBase::Register<type>(v);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "SIMPLE",		(int)wxCB_SIMPLE);
		newSlot(v, -1, "DROPDOWN",		(int)wxCB_DROPDOWN);
		newSlot(v, -1, "READONLY",		(int)wxCB_READONLY);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "COMBOBOX",			(int)wxEVT_COMMAND_COMBOBOX_SELECTED);
		newSlot(v, -1, "COMBOBOX_DROPDOWN",	(int)wxEVT_COMMAND_COMBOBOX_DROPDOWN);
		newSlot(v, -1, "COMBOBOX_CLOSEUP",	(int)wxEVT_COMMAND_COMBOBOX_CLOSEUP);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxArrayString choices;

		if (!isNone(v, 7))
		{
			if (sq_gettype(v, 7) != OT_ARRAY)
				return sq_throwerror(v, "invalid choice array");

			GetWxArrayString(v, 7, choices);
		}

		wxControl* control = new wxComboBox(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			optWxString(v, 4, ""),
			*opt<wxPoint>(v, 5, wxDefaultPosition),
			*opt<wxSize>(v, 6, wxDefaultSize),
			choices,
			optInt(v, 8, 0),
			*opt<wxValidator>(v, 9, (wxValidator*)&wxDefaultValidator),
			optWxString(v, 10, wxComboBoxNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}

	NB_FUNC(popup)						{ self(v)->Popup(); return 0; }
	NB_FUNC(dismiss)					{ self(v)->Dismiss(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, BitmapComboBox, wxComboBox);

class NB_WxBitmapComboBox : TNitClass<wxBitmapComboBox>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(bitmapSize),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, value=\"\", pos=null, size=null, choices: string[]=null, style=0, validator=null, name=null)"),
			FUNC_ENTRY_H(append,		"(item: string, clientObject=null): int\n"
										"(items: string[], clientObjects: object[]=null): int\n"
										"(item: string, bitmap: Bitmap): int\n"
										"(item: string, bitmap: Bitmap, data): int\n"),
			FUNC_ENTRY_H(getItemBitmap,	"(n: int): Bitmap"),
			FUNC_ENTRY_H(setItemBitmap,	"(n: int, bitmap: Bitmap)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		wxArrayString choices;

		if (!isNone(v, 7))
		{
			if (sq_gettype(v, 7) != OT_ARRAY)
				return sq_throwerror(v, "invalid choice array");

			GetWxArrayString(v, 7, choices);
		}

		wxControl* control = new wxBitmapComboBox(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			optWxString(v, 4, ""),
			*opt<wxPoint>(v, 5, wxDefaultPosition),
			*opt<wxSize>(v, 6, wxDefaultSize),
			choices,
			optInt(v, 8, 0),
			*opt<wxValidator>(v, 9, (wxValidator*)&wxDefaultValidator),
			optWxString(v, 10, wxBitmapComboBoxNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}

	NB_PROP_GET(bitmapSize)				{ return push(v, self(v)->GetBitmapSize()); }

	NB_FUNC(getItemBitmap)				{ return push(v, self(v)->GetItemBitmap(getInt(v, 2))); }
	NB_FUNC(setItemBitmap)				{ self(v)->SetItemBitmap(getInt(v, 2), *get<wxBitmap>(v, 3)); return 0; }

	NB_FUNC(append)
	{
		if (!isNone(v, 3) && is<wxBitmap>(v, 3))
		{
			if (!isNone(v, 4))
				return push(v, self(v)->Append(getWxString(v, 2), *get<wxBitmap>(v, 3), GetWxScriptData(v, 4, NULL)));
			else
				return push(v, self(v)->Append(getWxString(v, 2), *get<wxBitmap>(v, 3)));
		}
		else
			return NB_WxItemContainer::_nb_append(v);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, RadioBox, wxStaticBox);

class NB_WxRadioBox : TNitClass<wxRadioBox>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(columnCount),
			PROP_ENTRY_R(rowCount),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(					"(parent: Window=null, id=wx.ID.ANY, label: string=\"\", pos=null, size=null, choices: string[]=null, majorDimension=1, style=STYLE.SPECIFY_COLS, validator=null, name=null)"),
			FUNC_ENTRY_H(enable,			"(itemIndex: int, enable=true): bool"),
			FUNC_ENTRY_H(show,				"(itemIndex: int, show=true): bool"),
			FUNC_ENTRY_H(isItemEnabled,		"(itemIndex: int): bool"),
			FUNC_ENTRY_H(isItemShown,		"(itemIndex: int): bool"),
			FUNC_ENTRY_H(getItemHelpText,	"(itemIndex: int): string"),
			FUNC_ENTRY_H(setItemHelpText,	"(itemIndex: int, text: string)"),
			NULL
		};

		bind(v, props, funcs);

		NB_WxItemContainerImmutable::Register<type>(v);
	
		addStaticTable(v, "STYLE");
		newSlot(v, -1, "SPECIFY_ROWS",	(int)wxRA_SPECIFY_ROWS);
		newSlot(v, -1, "SPECIFY_COLS",	(int)wxRA_SPECIFY_COLS);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "RADIOBOX",		(int)wxEVT_COMMAND_RADIOBOX_SELECTED);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxArrayString choices;

		if (!isNone(v, 7))
		{
			if (sq_gettype(v, 7) != OT_ARRAY)
				return sq_throwerror(v, "invalid choice array");

			GetWxArrayString(v, 7, choices);
		}

		wxControl* control = new wxRadioBox(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			optWxString(v, 4, ""),
			*opt<wxPoint>(v, 5, wxDefaultPosition),
			*opt<wxSize>(v, 6, wxDefaultSize),
			choices,
			optInt(v, 8, 1),
			optInt(v, 9, wxRA_SPECIFY_COLS),
			*opt<wxValidator>(v, 10, (wxValidator*)&wxDefaultValidator),
			optWxString(v, 11, wxRadioBoxNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}

	NB_PROP_GET(columnCount)			{ return push(v, self(v)->GetColumnCount()); }
	NB_PROP_GET(rowCount)				{ return push(v, self(v)->GetRowCount()); }

	NB_FUNC(enable)						{ return push(v, self(v)->Enable(getInt(v, 2), optBool(v, 3, true))); }
	NB_FUNC(show)						{ return push(v, self(v)->Show(getInt(v, 2), optBool(v, 3, true))); }
	NB_FUNC(isItemEnabled)				{ return push(v, self(v)->IsItemEnabled(getInt(v, 2))); }
	NB_FUNC(isItemShown)				{ return push(v, self(v)->IsItemShown(getInt(v, 2))); }
	NB_FUNC(getItemHelpText)			{ return push(v, self(v)->GetItemHelpText(getInt(v, 2))); }
	NB_FUNC(setItemHelpText)			{ self(v)->SetItemHelpText(getInt(v, 2), getWxString(v, 3)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, ListBox, wxControl);

class NB_WxListBox : TNitClass<wxListBox>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(selections),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, pos=null, size=null, choices: string[]=null, style=0, validator=null, name=null)"),
			FUNC_ENTRY_H(deselect,		"(itemIndex: int)"),
			FUNC_ENTRY_H(isSelected,	"(itemIndex: int): bool"),
			FUNC_ENTRY_H(setFirstItem,	"(itemIndex: int)\n"
										"(string)\n"),
			FUNC_ENTRY_H(hitTest,		"(point: Point): int"),
			NULL
		};

		bind(v, props, funcs);

		NB_WxItemContainer::Register<type>(v);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "SINGLE",	(int)wxLB_SINGLE);
		newSlot(v, -1, "MULTIPLE",	(int)wxLB_MULTIPLE);
		newSlot(v, -1, "EXTENDED",	(int)wxLB_EXTENDED);
		newSlot(v, -1, "HSCROLL",	(int)wxLB_HSCROLL);
		newSlot(v, -1, "ALWAYS_SB",	(int)wxLB_ALWAYS_SB);
		newSlot(v, -1, "NEEDED_SB",	(int)wxLB_NEEDED_SB);
		newSlot(v, -1, "NO_SB",		(int)wxLB_NO_SB);
		newSlot(v, -1, "SORT",		(int)wxLB_SORT);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "LISTBOX",			(int)wxEVT_COMMAND_LISTBOX_SELECTED);
		newSlot(v, -1, "LISTBOX_DCLICK",	(int)wxEVT_COMMAND_LISTBOX_DOUBLECLICKED);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxArrayString choices;

		if (!isNone(v, 6))
		{
			if (sq_gettype(v, 6) != OT_ARRAY)
				return sq_throwerror(v, "invalid choice array");

			GetWxArrayString(v, 6, choices);
		}

		wxControl* control = new wxListBox(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			*opt<wxPoint>(v, 4, wxDefaultPosition),
			*opt<wxSize>(v, 5, wxDefaultSize),
			choices,
			optInt(v, 7, 0),
			*opt<wxValidator>(v, 8, (wxValidator*)&wxDefaultValidator),
			optWxString(v, 9, wxListBoxNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}

	NB_PROP_GET(selections)				{ wxArrayInt sel; self(v)->GetSelections(sel); return PushWxArray(v, sel); }
	
	NB_FUNC(deselect)					{ self(v)->Deselect(getInt(v, 2)); return 0; }
	NB_FUNC(isSelected)					{ return push(v, self(v)->IsSelected(getInt(v, 2))); }

	NB_FUNC(setFirstItem)
	{
		if (isInt(v, 2))
			self(v)->SetFirstItem(getInt(v, 2));
		else
			self(v)->SetFirstItem(getWxString(v, 2));
		return 0;
	}

	NB_FUNC(hitTest)					{ return push(v, self(v)->HitTest(*get<wxPoint>(v, 2))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, CheckListBox, wxListBox);

class NB_WxCheckListBox : TNitClass<wxCheckListBox>
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
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, pos=null, size=null, choices: string[]=null, style=0, validator=null, name=null)"),
			FUNC_ENTRY_H(check,			"(itemIndex: int, check=true)"),
			FUNC_ENTRY_H(isChecked,		"(itemIndex: int): bool"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "CHECKLISTBOX",	(int)wxEVT_COMMAND_CHECKLISTBOX_TOGGLED);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxArrayString choices;

		if (!isNone(v, 6))
		{
			if (sq_gettype(v, 6) != OT_ARRAY)
				return sq_throwerror(v, "invalid choice array");

			GetWxArrayString(v, 6, choices);
		}

		wxControl* control = new wxCheckListBox(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			*opt<wxPoint>(v, 4, wxDefaultPosition),
			*opt<wxSize>(v, 5, wxDefaultSize),
			choices,
			optInt(v, 7, 0),
			*opt<wxValidator>(v, 8, (wxValidator*)&wxDefaultValidator),
			optWxString(v, 9, wxListBoxNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}

	NB_FUNC(check)						{ self(v)->Check(getInt(v, 2), optBool(v, 3, true)); return 0; }
	NB_FUNC(isChecked)					{ return push(v, self(v)->IsChecked(getInt(v, 2))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, CollapsiblePane, wxControl);

class NB_WxCollapsiblePane : TNitClass<wxCollapsiblePane>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(pane),
			PROP_ENTRY	(collapsed),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(					"(parent: Window=null, id=wx.ID.ANY, label: string, pos=null, size=null, style=STYLE.DEFAULT, validator=null, name=null)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "DEFAULT",		(int)wxCP_DEFAULT_STYLE);
		newSlot(v, -1, "NO_TLW_RESIZE",	(int)wxCP_NO_TLW_RESIZE);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "PANE_CHANGED",	(int)wxEVT_COMMAND_COLLPANE_CHANGED);
		newSlot(v, -1, "NAVIGATION_KEY",(int)wxEVT_NAVIGATION_KEY);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxControl* control = new wxCollapsiblePane(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			getWxString(v, 4),
			*opt<wxPoint>(v, 5, wxDefaultPosition),
			*opt<wxSize>(v, 6, wxDefaultSize),
			optInt(v, 7, wxCP_DEFAULT_STYLE),
			*opt<wxValidator>(v, 8, (wxValidator*)&wxDefaultValidator),
			optWxString(v, 9, wxCollapsiblePaneNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}

	NB_PROP_GET(pane)					{ return push(v, self(v)->GetPane()); }
	NB_PROP_GET(collapsed)				{ return push(v, self(v)->IsCollapsed()); }
	
	NB_PROP_SET(collapsed)				{ self(v)->Collapse(getBool(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, BookCtrlBase, wxControl);

class NB_WxBookCtrlBase : TNitClass<wxBookCtrlBase>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(pageCount),
			PROP_ENTRY_R(currentPage),
			PROP_ENTRY	(selection),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(addPage,			"(page: Window, text: string, select=false, imageid=null): bool"),
			FUNC_ENTRY_H(deletePage,		"(pageIndex: int): bool"),
			FUNC_ENTRY_H(removePage,		"(pageIndex: int): bool // doesn't delete the associated window"),
			FUNC_ENTRY_H(deleteAllPages,	"(): bool"),
			FUNC_ENTRY_H(getPage,			"(pageIndex: int): Window"),

			FUNC_ENTRY_H(getPageText,		"(pageIndex: int): string"),
			FUNC_ENTRY_H(setPageText,		"(pageIndex: int, text: string): bool"),

			FUNC_ENTRY_H(advanceSelection,	"(forward=true)"),
			FUNC_ENTRY_H(changeSelection,	"(pageIndex: int): int // doesn't generate page event"),

			// TODO: Image related
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(pageCount)				{ return push(v, self(v)->GetPageCount()); }
	NB_PROP_GET(currentPage)			{ return push(v, self(v)->GetCurrentPage()); }
	NB_PROP_GET(selection)				{ return push(v, self(v)->GetSelection()); }

	NB_PROP_SET(selection)				{ self(v)->SetSelection(getInt(v, 2)); return 0; }

	NB_FUNC(addPage)					{ return push(v, self(v)->AddPage(get<wxWindow>(v, 2), getWxString(v, 3), optBool(v, 4, false), optInt(v, 5, wxNOT_FOUND))); }
	NB_FUNC(deletePage)					{ return push(v, self(v)->DeletePage(getInt(v, 2))); }
	NB_FUNC(removePage)					{ return push(v, self(v)->RemovePage(getInt(v, 2))); }
	NB_FUNC(deleteAllPages)				{ return push(v, self(v)->DeleteAllPages()); }
	NB_FUNC(getPage)					{ return push(v, self(v)->GetPage(getInt(v, 2))); }

	NB_FUNC(getPageText)				{ return push(v, self(v)->GetPageText(getInt(v, 2))); }
	NB_FUNC(setPageText)				{ return push(v, self(v)->SetPageText(getInt(v, 2), getWxString(v, 3))); }

	NB_FUNC(advanceSelection)			{ self(v)->AdvanceSelection(optBool(v, 2, true)); return 0; }
	NB_FUNC(changeSelection)			{ return push(v, self(v)->GetPageText(getInt(v, 2))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, Notebook, wxBookCtrlBase);

class NB_WxNotebook : TNitClass<wxNotebook>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(rowCount),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(					"(parent: Window=null, id=wx.ID.ANY, pos=null, size=null, style=STYLE.DEFAULT, name=null)"),
			FUNC_ENTRY_H(setPadding,		"(padding: Size)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "DEFAULT",		(int)wxNB_DEFAULT);
		newSlot(v, -1, "TOP",			(int)wxNB_TOP);
		newSlot(v, -1, "LEFT",			(int)wxNB_LEFT);
		newSlot(v, -1, "RIGHT",			(int)wxNB_RIGHT);
		newSlot(v, -1, "BOTTOM",		(int)wxNB_BOTTOM);
		newSlot(v, -1, "FIXEDWIDTH",	(int)wxNB_FIXEDWIDTH);
		newSlot(v, -1, "MULTILINE",		(int)wxNB_MULTILINE);
		newSlot(v, -1, "NOPAGETHEME",	(int)wxNB_NOPAGETHEME);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "CHANGED",		(int)wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED);
		newSlot(v, -1, "CHANGING",		(int)wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGING);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxControl* control = new wxNotebook(
			opt<wxWindow>	(v, 2, NULL),
			optInt			(v, 3, wxID_ANY),
			*opt<wxPoint>	(v, 4, wxDefaultPosition),
			*opt<wxSize>	(v, 5, wxDefaultSize),
			optInt			(v, 6, wxNB_DEFAULT),
			optWxString		(v, 7, wxNotebookNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}

	NB_PROP_GET(rowCount)				{ return push(v, self(v)->GetRowCount()); }

	NB_FUNC(setPadding)					{ self(v)->SetPadding(*get<wxSize>(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, Choicebook, wxBookCtrlBase);

class NB_WxChoicebook : TNitClass<wxChoicebook>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(choiceCtrl),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, pos=null, size=null, style=STYLE.DEFAULT, name=null)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "DEFAULT",		(int)wxCHB_DEFAULT);
		newSlot(v, -1, "TOP",			(int)wxCHB_TOP);
		newSlot(v, -1, "LEFT",			(int)wxCHB_LEFT);
		newSlot(v, -1, "RIGHT",			(int)wxCHB_RIGHT);
		newSlot(v, -1, "BOTTOM",		(int)wxCHB_BOTTOM);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "CHANGED",		(int)wxEVT_COMMAND_CHOICEBOOK_PAGE_CHANGED);
		newSlot(v, -1, "CHANGING",		(int)wxEVT_COMMAND_CHOICEBOOK_PAGE_CHANGING);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxControl* control = new wxChoicebook(
			opt<wxWindow>	(v, 2, NULL),
			optInt			(v, 3, wxID_ANY),
			*opt<wxPoint>	(v, 4, wxDefaultPosition),
			*opt<wxSize>	(v, 5, wxDefaultSize),
			optInt			(v, 6, wxCHB_DEFAULT),
			optWxString		(v, 7, "choicebook")
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}

	NB_PROP_GET(choiceCtrl)				{ return push(v, self(v)->GetChoiceCtrl()); }
};

////////////////////////////////////////////////////////////////////////////////

// TAG_HANDLER sample : <nit> /* nit script code */ </nit>

TAG_HANDLER_BEGIN(NIT, "NIT")

TAG_HANDLER_PROC(tag)
{
	// TODO: How about a StreamReader which reads a wxString?
	wxString text(tag.GetBeginIter(), tag.GetEndIter1());

	// TODO: expose context or setup print function
	// TODO: handle '<', '>'
	g_Session->getScript()->doString(text.utf8_str(), true);

	return true;
}

TAG_HANDLER_END(NIT)

TAGS_MODULE_BEGIN(NitWx)
	TAGS_MODULE_ADD(NIT)
TAGS_MODULE_END(NitWx)

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, HtmlWindow, wxScrolledWindow);

class NB_WxHtmlWindow : TNitClass<wxHtmlWindow>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(openedAnchor),
			PROP_ENTRY_R(openedPage),
			PROP_ENTRY_R(openedPageTitle),
			PROP_ENTRY_R(relatedFrame),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(						"(parent: Window, id=ID.ANY, pos=null, size=null, style=STYLE.DEFAULT, name=null)"),

			FUNC_ENTRY_H(setPage,				"(source: string): bool"),
			FUNC_ENTRY_H(appendToPage,			"(source: string): bool"),
			FUNC_ENTRY_H(toText,				"(): string // convert to plain text"),

			FUNC_ENTRY_H(loadFile,				"(filename: string): bool"),
			FUNC_ENTRY_H(loadPage,				"(location: string): bool"),

			FUNC_ENTRY_H(historyCanBack,		"(): bool"),
			FUNC_ENTRY_H(historyCanForward,		"(): bool"),
			FUNC_ENTRY_H(historyBack,			"(): bool"),
			FUNC_ENTRY_H(historyForward,		"(): bool"),
			FUNC_ENTRY_H(historyClear,			"()"),

			FUNC_ENTRY_H(setRelatedFrame,		"(frame: Frame, format: string) // format e.g. \"HtmlHelp: %%s\""),
			FUNC_ENTRY_H(setRelatedStatusBar,	"(bar: StatusBar, index=0)"),
			FUNC_ENTRY_H(setBorders,			"(space: int)"),

			FUNC_ENTRY_H(selectAll,				"()"),
			FUNC_ENTRY_H(selectLine,			"(pos: Point)"),
			FUNC_ENTRY_H(selectWord,			"(pos: Point)"),
			FUNC_ENTRY_H(selectionToText,		"(): string // convert to plain text"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "DEFAULT",				(int)wxHW_DEFAULT_STYLE);
		newSlot(v, -1, "SCROLLBAR_NEVER",		(int)wxHW_SCROLLBAR_NEVER);
		newSlot(v, -1, "SCROLLBAR_AUTO",		(int)wxHW_SCROLLBAR_AUTO);
		newSlot(v, -1, "NO_SELECTION",			(int)wxHW_NO_SELECTION);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "CELL_CLICKED",			(int)wxEVT_COMMAND_HTML_CELL_CLICKED);
		newSlot(v, -1, "CELL_HOVER",			(int)wxEVT_COMMAND_HTML_CELL_HOVER);
		newSlot(v, -1, "LINK_CLICKED",			(int)wxEVT_COMMAND_HTML_LINK_CLICKED);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxHtmlWindow* wnd = new wxHtmlWindow(
			get<wxWindow>	(v, 2),
			optInt			(v, 3, wxID_ANY),
			*opt<wxPoint>	(v, 4, wxDefaultPosition),
			*opt<wxSize>	(v, 5, wxDefaultSize),
			optInt			(v, 6, wxHW_DEFAULT_STYLE),
			optWxString		(v, 7, "htmlWindow")
			);

		// NOTE: Use <meta> tag when need to specify encoding explicitly.

		sq_setinstanceup(v, 1, wnd);
		return 0;
	}

	NB_PROP_GET(openedAnchor)			{ return push(v, self(v)->GetOpenedAnchor()); }
	NB_PROP_GET(openedPage)				{ return push(v, self(v)->GetOpenedPage()); }
	NB_PROP_GET(openedPageTitle)		{ return push(v, self(v)->GetOpenedPageTitle()); }
	NB_PROP_GET(relatedFrame)			{ return push(v, self(v)->GetRelatedFrame()); }

	NB_FUNC(setPage)					{ return push(v, self(v)->SetPage(getWxString(v, 2))); return 0; }
	NB_FUNC(appendToPage)				{ return push(v, self(v)->AppendToPage(getWxString(v, 2))); return 0; }
	NB_FUNC(toText)						{ return push(v, self(v)->ToText()); }
	
	NB_FUNC(loadFile)					{ return push(v, self(v)->LoadFile(getWxString(v, 2))); return 0; }
	NB_FUNC(loadPage)					{ return push(v, self(v)->LoadPage(getWxString(v, 2))); return 0; }

	NB_FUNC(historyCanBack)				{ return push(v, self(v)->HistoryCanBack()); }
	NB_FUNC(historyCanForward)			{ return push(v, self(v)->HistoryCanForward()); }
	NB_FUNC(historyBack)				{ return push(v, self(v)->HistoryBack()); }
	NB_FUNC(historyForward)				{ return push(v, self(v)->HistoryForward()); }
	NB_FUNC(historyClear)				{ self(v)->HistoryClear(); return 0; }

	NB_FUNC(setRelatedFrame)			{ self(v)->SetRelatedFrame(opt<wxFrame>(v, 2, NULL), getWxString(v, 3)); return 0; }
	NB_FUNC(setRelatedStatusBar)		{ self(v)->SetRelatedStatusBar(opt<wxStatusBar>(v, 2, NULL), optInt(v, 3, 0)); return 0; }
	NB_FUNC(setBorders)					{ self(v)->SetBorders(getInt(v, 2)); return 0; }

	NB_FUNC(selectAll)					{ self(v)->SelectAll(); return 0; }
	NB_FUNC(selectLine)					{ self(v)->SelectLine(*get<wxPoint>(v, 2)); return 0; }
	NB_FUNC(selectWord)					{ self(v)->SelectWord(*get<wxPoint>(v, 2)); return 0; }
	NB_FUNC(selectionToText)			{ return push(v, self(v)->SelectionToText()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, VListBox, wxPanel);

class NB_WxVListBox : TNitClass<wxVListBox>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(itemCount),
			PROP_ENTRY	(selection),
			PROP_ENTRY_R(selected),
			PROP_ENTRY	(margins),
			PROP_ENTRY_R(selectedCount),
			PROP_ENTRY	(selectionBackColor),
			PROP_ENTRY_R(hasMultipleSelection),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(clear,			"()"),
			FUNC_ENTRY_H(deselectAll,	"()"),
			FUNC_ENTRY_H(isCurrent,		"(item: int): bool"),
			FUNC_ENTRY_H(isSelected,	"(item: int): bool"),
			FUNC_ENTRY_H(select,		"(item: int, select=true): bool"),
			FUNC_ENTRY_H(selectAll,		"(): bool"),
			FUNC_ENTRY_H(selectRange,	"(from, to: int): bool"),
			FUNC_ENTRY_H(toggle,		"(item: int)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(itemCount)				{ return push(v, self(v)->GetItemCount()); }
	NB_PROP_GET(selection)				{ return push(v, self(v)->GetSelection()); }
	NB_PROP_GET(margins)				{ return push(v, self(v)->GetMargins()); }
	NB_PROP_GET(selectedCount)			{ return push(v, self(v)->GetSelectedCount()); }
	NB_PROP_GET(selectionBackColor)		{ return PushWxColor(v, self(v)->GetSelectionBackground()); }
	NB_PROP_GET(hasMultipleSelection)	{ return push(v, self(v)->HasMultipleSelection()); }

	NB_PROP_SET(itemCount)				{ self(v)->SetItemCount(getInt(v, 2)); return 0; }
	NB_PROP_SET(selection)				{ self(v)->SetSelection(getInt(v, 2)); return 0; }
	NB_PROP_SET(margins)				{ self(v)->SetMargins(*get<wxPoint>(v, 2)); return 0; }
	NB_PROP_SET(selectionBackColor)		{ self(v)->SetSelectionBackground(GetWxColor(v, 2)); return 0; }

	NB_FUNC(clear)						{ self(v)->Clear(); return 0; }
	NB_FUNC(deselectAll)				{ self(v)->DeselectAll(); return 0; }
	NB_FUNC(isCurrent)					{ return push(v, self(v)->IsCurrent(getInt(v, 2))); }
	NB_FUNC(isSelected)					{ return push(v, self(v)->IsSelected(getInt(v, 2))); }
	NB_FUNC(select)						{ return push(v, self(v)->Select(getInt(v, 2), optBool(v, 3, true))); }
	NB_FUNC(selectAll)					{ return push(v, self(v)->SelectAll()); }
	NB_FUNC(selectRange)				{ return push(v, self(v)->SelectRange(getInt(v, 2), getInt(v, 3))); }
	NB_FUNC(toggle)						{ self(v)->Toggle(getInt(v, 2)); return 0; }

	struct SelectedItr
	{
		wxVListBox* lb;
		int nextIndex;
		ulong cookie;

		SelectedItr(wxVListBox* lb): lb(lb) 
		{ 
			nextIndex = lb->GetFirstSelected(cookie);
		}

		static SQInteger itrfunc(HSQUIRRELVM v, SQUserPointer pItr)
		{
			SelectedItr* itr = (SelectedItr*)pItr;
			if (v == NULL)
			{
				itr->~SelectedItr();
				return 0;
			}

			int index = itr->nextIndex;

			if (index == wxNOT_FOUND) return 0;

			itr->nextIndex = itr->lb->GetNextSelected(itr->cookie);
			return NitBind::push(v, index);
		}
	};

	NB_PROP_GET(selected)
	{
		SelectedItr* itr = (SelectedItr*)sq_newnativeitr(v, SelectedItr::itrfunc, sizeof(SelectedItr));
		new (itr) SelectedItr(self(v));
		return 1;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, HtmlListBox, wxVListBox);

class NB_WxHtmlListBox : TNitClass<wxHtmlListBox>
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

		addStaticTable(v, "EVT");
		newSlot(v, -1, "CELL_CLICKED",	(int)wxEVT_COMMAND_HTML_CELL_CLICKED);
		newSlot(v, -1, "CELL_HOVER",	(int)wxEVT_COMMAND_HTML_CELL_HOVER);
		newSlot(v, -1, "LINK_CLICKED",	(int)wxEVT_COMMAND_HTML_LINK_CLICKED);
		sq_poptop(v);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, SimpleHtmlListBox, wxHtmlListBox);

class NB_WxSimpleHtmlListBox : TNitClass<wxSimpleHtmlListBox>
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
			CONS_ENTRY_H(				"(parent: wx.Window, id=wx.ID.ANY, pos=null, size=null, choices: string[], style=STYLE.DEFAULT, validator=null, name=null)"),
			NULL
		};

		bind(v, props, funcs);

		NB_WxItemContainer::Register<type>(v);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "DEFAULT",			(int)wxHLB_DEFAULT_STYLE);
		newSlot(v, -1, "MULTIPLE",			(int)wxHLB_MULTIPLE);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "LISTBOX",			(int)wxEVT_COMMAND_LISTBOX_SELECTED);
		newSlot(v, -1, "LISTBOX_DCLICK",	(int)wxEVT_COMMAND_LISTBOX_DOUBLECLICKED);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxArrayString choices;
		GetWxArrayString(v, 6, choices);

		type* self = setSelf(v, new type(
			get<wxWindow>(v, 2),
			optInt(v, 3, wxID_ANY),
			*opt<wxPoint>(v, 4, wxDefaultPosition),
			*opt<wxSize>(v, 5, wxDefaultSize),
			choices,
			optInt(v, 7, wxHLB_DEFAULT_STYLE),
			*opt<wxValidator>(v, 8, wxDefaultValidator),
			optWxString(v, 9, wxSimpleHtmlListBoxNameStr)
			));
		return SQ_OK;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, WebViewEvent, wxNotifyEvent);

class NB_WxWebViewEvent : TNitClass<wxWebViewEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(url),
			PROP_ENTRY_R(target),
			NULL
		};
		
		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(url)					{ return push(v, self(v)->GetURL()); }
	NB_PROP_GET(target)				{ return push(v, self(v)->GetTarget()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, WebView, wxControl);

class NB_WxWebView : TNitClass<wxWebView>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] = 
		{
			PROP_ENTRY_R(currentTitle),
			PROP_ENTRY_R(currentURL),
			PROP_ENTRY_R(busy),
			PROP_ENTRY	(editable),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(					"(parent: wx.Parent, id=wx.ID.ANY, url=null, pos=null, size=null, backend=null, style=0, name=null): wx.WebView"),
			FUNC_ENTRY_H(canGoBack,			"(): bool"),
			FUNC_ENTRY_H(canGoForward,		"(): bool"),
			FUNC_ENTRY_H(canCut,			"(): bool"),
			FUNC_ENTRY_H(canCopy,			"(): bool"),
			FUNC_ENTRY_H(canPaste,			"(): bool"),
			FUNC_ENTRY_H(canUndo,			"(): bool"),
			FUNC_ENTRY_H(canRedo,			"(): bool"),
			FUNC_ENTRY_H(hasSelection,		"(): bool"),

			FUNC_ENTRY_H(getPageSource,		"(): string"),
			FUNC_ENTRY_H(getPageText,		"(): string"),
			FUNC_ENTRY_H(loadUrl,			"(url: string)"),
			FUNC_ENTRY_H(print,				"()"),
			FUNC_ENTRY_H(reload,			"(flags=RELOAD.DEFAULT)"),
			FUNC_ENTRY_H(runScript,			"(javascript: string)"),
			FUNC_ENTRY_H(setPage,			"(html: string, baseUrl: string)"
			"\n"							"(reader: StreamReader, baseUrl: string)"),
			FUNC_ENTRY_H(stop,				"()"),
			FUNC_ENTRY_H(goBack,			"()"),
			FUNC_ENTRY_H(goForward,			"()"),
			FUNC_ENTRY_H(clearHistory,		"()"),
			FUNC_ENTRY_H(enableHistory,		"(enable=true)"),
			FUNC_ENTRY_H(getBackwardHistory,"(): [ { url: string, title: string } ]"),
			FUNC_ENTRY_H(getForwardHistory,	"(): [ { url: string, title: string } ]"),
			FUNC_ENTRY_H(canSetZoomType,	"(type: ZOOM_TYPE): bool"),
			FUNC_ENTRY_H(selectAll,			"()"),
			FUNC_ENTRY_H(deleteSelection,	"()"),
			FUNC_ENTRY_H(clearSelection,	"()"),
			FUNC_ENTRY_H(getSelectedText,	"(): string"),
			FUNC_ENTRY_H(getSelectedSource,	"(): string"),
			FUNC_ENTRY_H(cut,				"()"),
			FUNC_ENTRY_H(copy,				"()"),
			FUNC_ENTRY_H(paste,				"()"),
			FUNC_ENTRY_H(undo,				"()"),
			FUNC_ENTRY_H(redo,				"()"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "ZOOM");
		newSlot(v, -1, "TINY",			(int)wxWEB_VIEW_ZOOM_TINY);
		newSlot(v, -1, "SMALL",			(int)wxWEB_VIEW_ZOOM_SMALL);
		newSlot(v, -1, "MEDIUM",		(int)wxWEB_VIEW_ZOOM_MEDIUM);
		newSlot(v, -1, "LARGE",			(int)wxWEB_VIEW_ZOOM_LARGE);
		newSlot(v, -1, "LARGEST",		(int)wxWEB_VIEW_ZOOM_LARGEST);
		sq_poptop(v);

		addStaticTable(v, "ZOOM_TYPE");
		newSlot(v, -1, "LAYOUT",		(int)wxWEB_VIEW_ZOOM_TYPE_LAYOUT);
		newSlot(v, -1, "TEXT",			(int)wxWEB_VIEW_ZOOM_TYPE_TEXT);
		sq_poptop(v);

		addStaticTable(v, "RELOAD");
		newSlot(v, -1, "DEFAULT",		(int)wxWEB_VIEW_RELOAD_DEFAULT);
		newSlot(v, -1, "NO_CACHE",		(int)wxWEB_VIEW_RELOAD_NO_CACHE);
		sq_poptop(v);

		addStaticTable(v, "NAV_ERR");
		newSlot(v, -1, "CONNECTION",	(int)wxWEB_NAV_ERR_CONNECTION);
		newSlot(v, -1, "CERTIFICATE",	(int)wxWEB_NAV_ERR_CERTIFICATE);
		newSlot(v, -1, "AUTH",			(int)wxWEB_NAV_ERR_AUTH);
		newSlot(v, -1, "SECURITY",		(int)wxWEB_NAV_ERR_SECURITY);
		newSlot(v, -1, "NOT_FOUND",		(int)wxWEB_NAV_ERR_NOT_FOUND);
		newSlot(v, -1, "REQUEST",		(int)wxWEB_NAV_ERR_REQUEST);
		newSlot(v, -1, "USER_CANCELLED",(int)wxWEB_NAV_ERR_USER_CANCELLED);
		newSlot(v, -1, "OTHER",			(int)wxWEB_NAV_ERR_OTHER);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "NAVIGATING",	(int)wxEVT_COMMAND_WEB_VIEW_NAVIGATING);
		newSlot(v, -1, "NAVIGATED",		(int)wxEVT_COMMAND_WEB_VIEW_NAVIGATED);
		newSlot(v, -1, "LOADED",		(int)wxEVT_COMMAND_WEB_VIEW_LOADED);
		newSlot(v, -1, "ERROR",			(int)wxEVT_COMMAND_WEB_VIEW_ERROR);
		newSlot(v, -1, "NEWWINDOW",		(int)wxEVT_COMMAND_WEB_VIEW_NEWWINDOW);
		newSlot(v, -1, "TITLE_CHANGED",	(int)wxEVT_COMMAND_WEB_VIEW_TITLE_CHANGED);
		sq_poptop(v);
	}

	NB_PROP_GET(currentTitle)			{ return push(v, self(v)->GetCurrentTitle()); }
	NB_PROP_GET(currentURL)				{ return push(v, self(v)->GetCurrentURL()); }
	NB_PROP_GET(busy)					{ return push(v, self(v)->IsBusy()); }
	NB_PROP_GET(editable)				{ return push(v, self(v)->IsEditable()); }

	NB_PROP_SET(editable)				{ self(v)->SetEditable(getBool(v, 2)); return 0; }

	NB_CONS()
	{
		setSelf(v, type::New(
			get<wxWindow>(v, 2),
			optInt(v, 3, wxID_ANY),
			optWxString(v, 4, wxWebViewDefaultURLStr),
			*opt<wxPoint>(v, 5, wxDefaultPosition),
			*opt<wxSize>(v, 6, wxDefaultSize),
			(wxWebViewBackend)optInt(v, 7, wxWEB_VIEW_BACKEND_DEFAULT),
			optInt(v, 8, 0),
			optWxString(v, 9, wxWebViewNameStr)
			));
		return SQ_OK;
	}

	NB_FUNC(canGoBack)					{ return push(v, self(v)->CanGoBack()); }
	NB_FUNC(canGoForward)				{ return push(v, self(v)->CanGoForward()); }
	NB_FUNC(canCut)						{ return push(v, self(v)->CanCut()); }
	NB_FUNC(canCopy)					{ return push(v, self(v)->CanCopy()); }
	NB_FUNC(canPaste)					{ return push(v, self(v)->CanPaste()); }
	NB_FUNC(canUndo)					{ return push(v, self(v)->CanUndo()); }
	NB_FUNC(canRedo)					{ return push(v, self(v)->CanRedo()); }
	NB_FUNC(hasSelection)				{ return push(v, self(v)->HasSelection()); }

	NB_FUNC(getPageSource)				{ return push(v, self(v)->GetPageSource()); }
	NB_FUNC(getPageText)				{ return push(v, self(v)->GetPageText()); }
	NB_FUNC(loadUrl)					{ self(v)->LoadURL(getWxString(v, 2)); return 0; }
	NB_FUNC(print)						{ self(v)->Print(); return 0; }
	NB_FUNC(reload)						{ self(v)->Reload((wxWebViewReloadFlags)optInt(v, 2, wxWEB_VIEW_RELOAD_DEFAULT)); return 0; }
	NB_FUNC(runScript)					{ self(v)->RunScript(getWxString(v, 2)); return 0; }
	NB_FUNC(setPage)
	{ 
		if (is<StreamReader>(v, 2))
		{
			wxNitInputStream stream(get<StreamReader>(v, 2));
			self(v)->SetPage(stream, getWxString(v, 3));
		}
		else self(v)->SetPage(getWxString(v, 2), getWxString(v, 3)); 
		return 0;
	}
	NB_FUNC(stop)						{ self(v)->Stop(); return 0; }
	NB_FUNC(goBack)						{ self(v)->GoBack(); return 0; }
	NB_FUNC(goForward)					{ self(v)->GoForward(); return 0; }
	NB_FUNC(clearHistory)				{ self(v)->ClearHistory(); return 0; }
	NB_FUNC(enableHistory)				{ self(v)->EnableHistory(optBool(v, 2, true)); return 0; }

	NB_FUNC(getBackwardHistory)
	{
		sq_newarray(v, 0);
		wxVector<wxSharedPtr<wxWebViewHistoryItem> > items = self(v)->GetBackwardHistory();

		for (uint i=0; i < items.size(); ++i)
		{
			sq_newtable(v);
			newSlot(v, -1, items[i]->GetUrl(), items[i]->GetTitle());
			sq_arrayappend(v, -2);
		}
		return 1;
	}

	NB_FUNC(getForwardHistory)
	{
		sq_newarray(v, 0);
		wxVector<wxSharedPtr<wxWebViewHistoryItem> > items = self(v)->GetForwardHistory();

		for (uint i=0; i < items.size(); ++i)
		{
			sq_newtable(v);
			newSlot(v, -1, items[i]->GetUrl(), items[i]->GetTitle());
			sq_arrayappend(v, -2);
		}
		return 1;
	}

	NB_FUNC(canSetZoomType)				{ return push(v, self(v)->CanSetZoomType((wxWebViewZoomType)getInt(v, 2))); }
	NB_FUNC(selectAll)					{ self(v)->SelectAll(); return 0; }
	NB_FUNC(deleteSelection)			{ self(v)->DeleteSelection(); return 0; }
	NB_FUNC(getSelectedText)			{ return push(v, self(v)->GetSelectedText()); }
	NB_FUNC(getSelectedSource)			{ return push(v, self(v)->GetSelectedSource()); }
	NB_FUNC(clearSelection)				{ self(v)->ClearSelection(); return 0; }
	NB_FUNC(cut)						{ self(v)->Cut(); return 0; }
	NB_FUNC(copy)						{ self(v)->Copy(); return 0; }
	NB_FUNC(paste)						{ self(v)->Paste(); return 0; }
	NB_FUNC(undo)						{ self(v)->Undo(); return 0; }
	NB_FUNC(redo)						{ self(v)->Redo(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, ToolBar, wxControl);

class NB_WxToolBar : TNitClass<wxToolBar>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(toolCount),
			PROP_ENTRY	(margins),
			PROP_ENTRY	(toolBitmapSize),
			PROP_ENTRY_R(toolSize),
			PROP_ENTRY	(toolPacking),
			PROP_ENTRY	(toolSeparation),
			PROP_ENTRY_R(vertical),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(						"(parent: Window=null, id=wx.ID.ANY, pos=null, size=null, style=STYLE.HORIZONTAL, name=null): ToolBarTool"),
			FUNC_ENTRY_H(addTool,				"(id: int, label: string, bitmap: Bitmap, shortHelp=\"\", kind=ITEM_NORMAL): ToolBarTool\n"
												"(id: int, label: string, bitmap, bmpDisabled: Bitmap, itemKind=ITEM_NORMAL, shortHelp=\"\", longHelp=\"\")"),
			FUNC_ENTRY_H(addCheckTool,			"(id: int, label: string, bitmap: Bitmap, bmpDisabled=null, shortHelp=\"\", longHelp=\"\"): ToolBarTool"),
			FUNC_ENTRY_H(addControl,			"(control: Control, label=\"\"): ToolBarTool"),
			FUNC_ENTRY_H(addRadioTool,			"(id: int, label: string, bitmap: Bitmap, bmpDisabled=null, shortHelp=\"\", longHelp=\"\"): ToolBarTool"),
			FUNC_ENTRY_H(addSeparator,			"(): ToolBarTool"),
			FUNC_ENTRY_H(addStretchableSpace,	"(): ToolBarTool"),
			FUNC_ENTRY_H(clearTools,			"()"),
			FUNC_ENTRY_H(deleteTool,			"(id): bool"),
			FUNC_ENTRY_H(findById,				"(id): ToolBarTool"),
			FUNC_ENTRY_H(findToolForPosition,	"(pos: Point): ToolBarTool"),
			FUNC_ENTRY_H(getToolByPos,			"(pos: int): ToolBarTool"),
			FUNC_ENTRY_H(realize,				"()"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "FLAT",					(int)wxTB_FLAT);
		newSlot(v, -1, "DOCKABLE",				(int)wxTB_DOCKABLE);
		newSlot(v, -1, "HORIZONTAL",			(int)wxTB_HORIZONTAL);
		newSlot(v, -1, "VERTICAL",				(int)wxTB_VERTICAL);
		newSlot(v, -1, "TEXT",					(int)wxTB_TEXT);
		newSlot(v, -1, "NOICONS",				(int)wxTB_NOICONS);
		newSlot(v, -1, "NODIVIDER",				(int)wxTB_NODIVIDER);
		newSlot(v, -1, "NOALIGN",				(int)wxTB_NOALIGN);
		newSlot(v, -1, "HORZ_LAYOUT",			(int)wxTB_HORZ_LAYOUT);
		newSlot(v, -1, "HORZ_TEXT",				(int)wxTB_HORZ_TEXT);
		newSlot(v, -1, "NO_TOOLTIPS",			(int)wxTB_NO_TOOLTIPS);
		newSlot(v, -1, "BOTTOM",				(int)wxTB_BOTTOM);
		newSlot(v, -1, "RIGHT",					(int)wxTB_RIGHT);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "TOOL",					(int)wxEVT_COMMAND_TOOL_CLICKED);
		newSlot(v, -1, "MENU",					(int)wxEVT_COMMAND_MENU_SELECTED);
		newSlot(v, -1, "TOOL_RCLICKED",			(int)wxEVT_COMMAND_TOOL_RCLICKED);
		newSlot(v, -1, "TOOL_ENTER",			(int)wxEVT_COMMAND_TOOL_ENTER);
		newSlot(v, -1, "TOOL_DROPDOWN",			(int)wxEVT_COMMAND_TOOL_DROPDOWN_CLICKED);
		sq_poptop(v);

		addStaticTable(v, "ITEM");
		newSlot(v, -1, "SEPARATOR",				(int)wxITEM_SEPARATOR);
		newSlot(v, -1, "NORMAL",				(int)wxITEM_NORMAL);
		newSlot(v, -1, "CHECK",					(int)wxITEM_CHECK);
		newSlot(v, -1, "RADIO",					(int)wxITEM_RADIO);
		newSlot(v, -1, "DROPDOWN",				(int)wxITEM_DROPDOWN);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxControl* control = new wxToolBar(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			*opt<wxPoint>(v, 4, wxDefaultPosition),
			*opt<wxSize>(v, 5, wxDefaultSize),
			optInt(v, 6, wxTB_HORIZONTAL),
			optWxString(v, 7, wxToolBarNameStr)
			);

		sq_setinstanceup(v, 1, control);
		return 0;
	}

	NB_PROP_GET(toolCount)				{ return push(v, self(v)->GetToolsCount()); }
	NB_PROP_GET(margins)				{ return push(v, self(v)->GetMargins()); }
	NB_PROP_GET(toolBitmapSize)			{ return push(v, self(v)->GetToolBitmapSize()); }
	NB_PROP_GET(toolSize)				{ return push(v, self(v)->GetToolSize()); }
	NB_PROP_GET(toolPacking)			{ return push(v, self(v)->GetToolPacking()); }
	NB_PROP_GET(toolSeparation)			{ return push(v, self(v)->GetToolSeparation()); }
	NB_PROP_GET(vertical)				{ return push(v, self(v)->IsVertical()); }

	NB_PROP_SET(margins)				{ self(v)->SetMargins(*get<wxSize>(v, 2)); return 0; }
	NB_PROP_SET(toolBitmapSize)			{ self(v)->SetToolBitmapSize(*get<wxSize>(v, 2)); return 0; }
	NB_PROP_SET(toolPacking)			{ self(v)->SetToolPacking(getInt(v, 2)); return 0; }
	NB_PROP_SET(toolSeparation)			{ self(v)->SetToolSeparation(getInt(v, 2)); return 0; }

	NB_FUNC(addTool)
	{
		if (!isNone(v, 5) && is<wxBitmap>(v, 5))
			return push(v, self(v)->AddTool(getInt(v, 2), getWxString(v, 3), *opt<wxBitmap>(v, 4, wxNullBitmap), *opt<wxBitmap>(v, 5, wxNullBitmap), (wxItemKind)optInt(v, 6, wxITEM_NORMAL), optWxString(v, 7, ""), optWxString(v, 8, "")));
		else
			return push(v, self(v)->AddTool(getInt(v, 2), getWxString(v, 3), *opt<wxBitmap>(v, 4, wxNullBitmap), optWxString(v, 5, ""), (wxItemKind)optInt(v, 6, wxITEM_NORMAL)));
	}

	NB_FUNC(addCheckTool)
	{
		return push(v, self(v)->AddCheckTool(getInt(v, 2), getWxString(v, 3), *get<wxBitmap>(v, 4), *opt<wxBitmap>(v, 5, wxNullBitmap), optWxString(v, 6, ""), optWxString(v, 7, "")));
	}

	NB_FUNC(addRadioTool)
	{
		return push(v, self(v)->AddRadioTool(getInt(v, 2), getWxString(v, 3), *get<wxBitmap>(v, 4), *opt<wxBitmap>(v, 5, wxNullBitmap), optWxString(v, 6, ""), optWxString(v, 7, "")));
	}

	NB_FUNC(addControl)					{ return push(v, self(v)->AddControl(get<wxControl>(v, 2), optWxString(v, 3, ""))); }
	NB_FUNC(addSeparator)				{ return push(v, self(v)->AddSeparator()); }
	NB_FUNC(addStretchableSpace)		{ return push(v, self(v)->AddStretchableSpace()); }
	NB_FUNC(clearTools)					{ self(v)->ClearTools(); return 0; }
	NB_FUNC(deleteTool)					{ return push(v, self(v)->DeleteTool(getInt(v, 2))); }
	NB_FUNC(findById)					{ return push(v, self(v)->FindById(getInt(v, 2))); }
	NB_FUNC(findToolForPosition)		{ wxPoint pt = *get<wxPoint>(v, 2); return push(v, self(v)->FindToolForPosition(pt.x, pt.y)); }
	NB_FUNC(getToolByPos)				{ return push(v, (wxToolBarToolBase*)self(v)->GetToolByPos(getInt(v, 2))); }
	NB_FUNC(realize)					{ return push(v, self(v)->Realize()); }
};

////////////////////////////////////////////////////////////////////////////////

// rename
typedef wxToolBarToolBase wxToolBarTool;

NB_TYPE_WXOBJ(NITWX_API, ToolBarTool, wxObject);

class NB_WxToolBarTool: TNitClass<wxToolBarToolBase>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(id),
			PROP_ENTRY_R(control),
			PROP_ENTRY_R(toolBar),
			PROP_ENTRY	(enabled),
			PROP_ENTRY	(toggled),
			PROP_ENTRY_R(bitmap),
			PROP_ENTRY	(normalBitmap),
			PROP_ENTRY	(disabledBitmap),
			PROP_ENTRY	(label),
			PROP_ENTRY	(shortHelp),
			PROP_ENTRY	(longHelp),
			PROP_ENTRY	(dropdownMenu),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(id)						{ return push(v, self(v)->GetId()); }
	NB_PROP_GET(control)				{ return push(v, self(v)->IsControl() ? self(v)->GetControl() : NULL); }
	NB_PROP_GET(toolBar)				{ return push(v, dynamic_cast<wxToolBar*>(self(v)->GetToolBar())); }

	NB_PROP_GET(enabled)				{ return push(v, self(v)->IsEnabled()); }
	NB_PROP_GET(toggled)				{ return push(v, self(v)->IsToggled()); }
	NB_PROP_GET(bitmap)					{ return push(v, self(v)->GetBitmap()); }
	NB_PROP_GET(normalBitmap)			{ return push(v, self(v)->GetNormalBitmap()); }
	NB_PROP_GET(disabledBitmap)			{ return push(v, self(v)->GetDisabledBitmap()); }
	NB_PROP_GET(label)					{ return push(v, self(v)->GetLabel()); }
	NB_PROP_GET(shortHelp)				{ return push(v, self(v)->GetShortHelp()); }
	NB_PROP_GET(longHelp)				{ return push(v, self(v)->GetLongHelp()); }
	NB_PROP_GET(dropdownMenu)			{ return push(v, self(v)->GetDropdownMenu()); }

	NB_PROP_SET(enabled)				{ self(v)->Enable(getBool(v, 2)); return 0; }
	NB_PROP_SET(toggled)				{ self(v)->Toggle(getBool(v, 2)); return 0; }
	NB_PROP_SET(normalBitmap)			{ self(v)->SetNormalBitmap(*get<wxBitmap>(v, 2)); return 0; }
	NB_PROP_SET(disabledBitmap)			{ self(v)->SetDisabledBitmap(*get<wxBitmap>(v, 2)); return 0; }
	NB_PROP_SET(label)					{ self(v)->SetLabel(getWxString(v, 2)); return 0; }
	NB_PROP_SET(shortHelp)				{ self(v)->SetShortHelp(getWxString(v, 2)); return 0; }
	NB_PROP_SET(longHelp)				{ self(v)->SetLongHelp(getWxString(v, 2)); return 0; }
	NB_PROP_SET(dropdownMenu)			{ self(v)->SetDropdownMenu(opt<wxMenu>(v, 2, NULL)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, ListCtrl, wxControl);

class NB_WxListCtrl : TNitClass<wxListCtrl>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(columnCount),
			PROP_ENTRY	(normalImageList),
			PROP_ENTRY	(smallImageList),
			PROP_ENTRY_R(countPerPage),
			PROP_ENTRY_R(editControl),
			PROP_ENTRY	(itemCount),
			PROP_ENTRY_R(selectedItemCount),
			PROP_ENTRY_R(itemSpacing),
			PROP_ENTRY	(textColor),
			PROP_ENTRY_R(topItem),
			PROP_ENTRY_R(viewRect),
			PROP_ENTRY_R(inReportView),
			PROP_ENTRY_R(virtual),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(						"(parent: wx.Window, id=wx.ID.ANY, pos=null, size=null, style=STYLE.ICON, validator=null, name=null)"),
			FUNC_ENTRY_H(appendColumn,			"(heading: string, format=FORMAT.LEFT, width=-1): int"),
			FUNC_ENTRY_H(arrange,				"(flag=ALIGN.DEFAULT): bool"),
			FUNC_ENTRY_H(clearAll,				"()"),
			FUNC_ENTRY_H(deleteAllItems,		"(): bool"),
			FUNC_ENTRY_H(deleteColumn,			"(col: int): bool"),
			FUNC_ENTRY_H(deleteItem,			"(item: int): bool"),
			FUNC_ENTRY_H(editLabel,				"(item: int): wx.TextCtrl"),
			FUNC_ENTRY_H(ensureVisible,			"(item: int): bool"),
			FUNC_ENTRY_H(findItem,				"(start: int, str: string, partial=false): int"
			"\n"								"(start: int, data: int): int"
			"\n"								"(start: int, pt: wx.Point, direction: FIND): int"),
			FUNC_ENTRY_H(getColumnWidth,		"(col: int): int"),
			FUNC_ENTRY_H(getItemBackColor,		"(item: int): Color"),
			FUNC_ENTRY_H(getItemData,			"(item: int): int"),
			FUNC_ENTRY_H(getItemFont,			"(item: int): wx.Font"),
			FUNC_ENTRY_H(getItemPosition,		"(item: int): wx.Point"),
			FUNC_ENTRY_H(getItemRect,			"(item: int, code=RECT.BOUNDS): wx.Rect"),
			FUNC_ENTRY_H(getItemState,			"(item: int, state: STATE): bool"),
			FUNC_ENTRY_H(getItemText,			"(item: int, col=0): string"),
			FUNC_ENTRY_H(getItemTextColor,		"(item: int): Color"),
			FUNC_ENTRY_H(getNextItem,			"(item: int, next=NEXT.ALL, state=STATE.DONTCARE): int"),
			FUNC_ENTRY_H(getSubItemRect,		"(item: int, subitem: int, code=RECT.BOUNDS): wx.Rect"),
			FUNC_ENTRY_H(hitTest,				"(pt: wx.Point): [ item: int, flags: HITTEST, subItem: int ]"),
			FUNC_ENTRY_H(insertColumn,			"(col: int, heading: string, format=FORMAT.LEFT, width=-1): int"),
			FUNC_ENTRY_H(insertItem,			"(index: int, label: string): int"
			"\n"								"(index: int, imageIndex: int): int"
			"\n"								"(index: int, label: string, imageIndex: int): int"),
			FUNC_ENTRY_H(refreshItem,			"(item: int)"),
			FUNC_ENTRY_H(refreshItems,			"(fromItem, toItem: int)"),
			FUNC_ENTRY_H(scrollList,			"(dx, dy: int): bool"),
			FUNC_ENTRY_H(setColumnWidth,		"(col: int, width: int): bool"),
			FUNC_ENTRY_H(setItem,				"(item: int, col: int, label: string, imageId=-1): int"),
			FUNC_ENTRY_H(setItemBackColor,		"(item: int, col: Color)"),
			FUNC_ENTRY_H(setItemColumnImage,	"(item: int, column: int, imageId: int): bool"),
			FUNC_ENTRY_H(setItemData,			"(item: int, data: int): bool"),
			FUNC_ENTRY_H(setItemFont,			"(item: int, font: wx.Font)"),
			FUNC_ENTRY_H(setItemImage,			"(item: int, imageId: int, selImageId=-1): bool"),
			FUNC_ENTRY_H(setItemPosition,		"(item: int, pos: wx.Point): bool"),
			FUNC_ENTRY_H(setItemState,			"(item: int, state: STATE, on: bool): bool"),
			FUNC_ENTRY_H(setItemText,			"(item: int, text: string)"),
			FUNC_ENTRY_H(setItemTextColor,		"(item: int, color: Color)"),
			FUNC_ENTRY_H(setSingleStyle,		"(style, add=true)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "LIST",				(int)wxLC_LIST);
		newSlot(v, -1, "REPORT",			(int)wxLC_REPORT);
		newSlot(v, -1, "VIRTUAL",			(int)wxLC_VIRTUAL);
		newSlot(v, -1, "ICON",				(int)wxLC_ICON);
		newSlot(v, -1, "SMALL_ICON",		(int)wxLC_SMALL_ICON);
		newSlot(v, -1, "ALIGN_TOP",			(int)wxLC_ALIGN_TOP);
		newSlot(v, -1, "ALIGN_LEFT",		(int)wxLC_ALIGN_LEFT);
		newSlot(v, -1, "AUTOARRANGE",		(int)wxLC_AUTOARRANGE);
		newSlot(v, -1, "EDIT_LABELS",		(int)wxLC_EDIT_LABELS);
		newSlot(v, -1, "NO_HEADER",			(int)wxLC_NO_HEADER);
		newSlot(v, -1, "SINGLE_SEL",		(int)wxLC_SINGLE_SEL);
		newSlot(v, -1, "SORT_ASCENDING",	(int)wxLC_SORT_ASCENDING);
		newSlot(v, -1, "SORT_DESCENDING",	(int)wxLC_SORT_DESCENDING);
		newSlot(v, -1, "HRULES",			(int)wxLC_HRULES);
		newSlot(v, -1, "VRULES",			(int)wxLC_VRULES);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "BEGIN_DRAG",		(int)wxEVT_COMMAND_LIST_BEGIN_DRAG);
		newSlot(v, -1, "BEGIN_RDRAG",		(int)wxEVT_COMMAND_LIST_BEGIN_RDRAG);
		newSlot(v, -1, "BEGIN_LABEL_EDIT",	(int)wxEVT_COMMAND_LIST_BEGIN_LABEL_EDIT);
		newSlot(v, -1, "END_LABEL_EDIT",	(int)wxEVT_COMMAND_LIST_END_LABEL_EDIT);
		newSlot(v, -1, "DELETE_ITEM",		(int)wxEVT_COMMAND_LIST_DELETE_ITEM);
		newSlot(v, -1, "DELETE_ALL_ITEMS",	(int)wxEVT_COMMAND_LIST_DELETE_ALL_ITEMS);
		newSlot(v, -1, "ITEM_SELECTED",		(int)wxEVT_COMMAND_LIST_ITEM_SELECTED);
		newSlot(v, -1, "ITEM_DESELECTED",	(int)wxEVT_COMMAND_LIST_ITEM_DESELECTED);
		newSlot(v, -1, "ITEM_ACTIVATED",	(int)wxEVT_COMMAND_LIST_ITEM_ACTIVATED);
		newSlot(v, -1, "ITEM_FOCUSED",		(int)wxEVT_COMMAND_LIST_ITEM_FOCUSED);
		newSlot(v, -1, "ITEM_MIDDLE_CLICK",	(int)wxEVT_COMMAND_LIST_ITEM_MIDDLE_CLICK);
		newSlot(v, -1, "ITEM_RIGHT_CLICK",	(int)wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK);
		newSlot(v, -1, "KEY_DOWN",			(int)wxEVT_COMMAND_LIST_KEY_DOWN);
		newSlot(v, -1, "INSERT_ITEM",		(int)wxEVT_COMMAND_LIST_INSERT_ITEM);
		newSlot(v, -1, "COL_CLICK",			(int)wxEVT_COMMAND_LIST_COL_CLICK);
		newSlot(v, -1, "COL_RIGHT_CLICK",	(int)wxEVT_COMMAND_LIST_COL_RIGHT_CLICK);
		newSlot(v, -1, "COL_BEGIN_DRAG",	(int)wxEVT_COMMAND_LIST_COL_BEGIN_DRAG);
		newSlot(v, -1, "COL_DRAGGING",		(int)wxEVT_COMMAND_LIST_COL_DRAGGING);
		newSlot(v, -1, "COL_END_DRAG",		(int)wxEVT_COMMAND_LIST_COL_END_DRAG);
		newSlot(v, -1, "CACHE_HINT",		(int)wxEVT_COMMAND_LIST_CACHE_HINT);
		sq_poptop(v);

		addStaticTable(v, "ALIGN");
		newSlot(v, -1, "DEFAULT",			(int)wxLIST_ALIGN_DEFAULT);
		newSlot(v, -1, "LEFT",				(int)wxLIST_ALIGN_LEFT);
		newSlot(v, -1, "TOP",				(int)wxLIST_ALIGN_TOP);
		newSlot(v, -1, "SNAP_TO_GRID",		(int)wxLIST_ALIGN_SNAP_TO_GRID);
		sq_poptop(v);

		addStaticTable(v, "FORMAT");
		newSlot(v, -1, "LEFT",				(int)wxLIST_FORMAT_LEFT);
		newSlot(v, -1, "RIGHT",				(int)wxLIST_FORMAT_RIGHT);
		newSlot(v, -1, "CENTER",			(int)wxLIST_FORMAT_CENTER);
		sq_poptop(v);

		addStaticTable(v, "RECT");
		newSlot(v, -1, "BOUNDS",			(int)wxLIST_RECT_BOUNDS);
		newSlot(v, -1, "ICON",				(int)wxLIST_RECT_ICON);
		newSlot(v, -1, "LABEL",				(int)wxLIST_RECT_LABEL);
		sq_poptop(v);

		addStaticTable(v, "NEXT");
		newSlot(v, -1, "ABOVE",				(int)wxLIST_NEXT_ABOVE);
		newSlot(v, -1, "ALL",				(int)wxLIST_NEXT_ALL);
		newSlot(v, -1, "BELOW",				(int)wxLIST_NEXT_BELOW);
		newSlot(v, -1, "LEFT",				(int)wxLIST_NEXT_LEFT);
		newSlot(v, -1, "RIGHT",				(int)wxLIST_NEXT_RIGHT);
		sq_poptop(v);

		addStaticTable(v, "HITTEST");
		newSlot(v, -1, "ABOVE",				(int)wxLIST_HITTEST_ABOVE);
		newSlot(v, -1, "BELOW",				(int)wxLIST_HITTEST_BELOW);
		newSlot(v, -1, "NOWHERE",			(int)wxLIST_HITTEST_NOWHERE);
		newSlot(v, -1, "ONITEMICON",		(int)wxLIST_HITTEST_ONITEMICON);
		newSlot(v, -1, "ONITEMLABEL",		(int)wxLIST_HITTEST_ONITEMLABEL);
		newSlot(v, -1, "ONITEMRIGHT",		(int)wxLIST_HITTEST_ONITEMRIGHT);
		newSlot(v, -1, "ONITEMSTATEICON",	(int)wxLIST_HITTEST_ONITEMSTATEICON);
		newSlot(v, -1, "ONITEM",			(int)wxLIST_HITTEST_ONITEM);
		newSlot(v, -1, "TOLEFT",			(int)wxLIST_HITTEST_TOLEFT);
		newSlot(v, -1, "TORIGHT",			(int)wxLIST_HITTEST_TORIGHT);
		sq_poptop(v);

		addStaticTable(v, "STATE");
		newSlot(v, -1, "DONTCARE",			(int)wxLIST_STATE_DONTCARE);
		newSlot(v, -1, "DROPHILITED",		(int)wxLIST_STATE_DROPHILITED);
		newSlot(v, -1, "FOCUSED",			(int)wxLIST_STATE_FOCUSED);
		newSlot(v, -1, "SELECTED",			(int)wxLIST_STATE_SELECTED);
		newSlot(v, -1, "CUT",				(int)wxLIST_STATE_CUT);
		sq_poptop(v);

		addStaticTable(v, "FIND");
		newSlot(v, -1, "UP",				(int)wxLIST_FIND_UP);
		newSlot(v, -1, "DOWN",				(int)wxLIST_FIND_DOWN);
		newSlot(v, -1, "LEFT",				(int)wxLIST_FIND_LEFT);
		newSlot(v, -1, "RIGHT",				(int)wxLIST_FIND_RIGHT);
		sq_poptop(v);
	}

	NB_PROP_GET(columnCount)			{ return push(v, self(v)->GetColumnCount()); }
	NB_PROP_GET(normalImageList)		{ return push(v, self(v)->GetImageList(wxIMAGE_LIST_NORMAL)); }
	NB_PROP_GET(smallImageList)			{ return push(v, self(v)->GetImageList(wxIMAGE_LIST_SMALL)); }
	NB_PROP_GET(countPerPage)			{ return push(v, self(v)->GetCountPerPage()); }
	NB_PROP_GET(editControl)			{ return push(v, self(v)->GetEditControl()); }
	NB_PROP_GET(itemCount)				{ return push(v, self(v)->GetItemCount()); }
	NB_PROP_GET(selectedItemCount)		{ return push(v, self(v)->GetSelectedItemCount()); }
	NB_PROP_GET(itemSpacing)			{ return push(v, self(v)->GetItemSpacing()); }
	NB_PROP_GET(textColor)				{ return PushWxColor(v, self(v)->GetTextColour()); }
	NB_PROP_GET(topItem)				{ return push(v, self(v)->GetTopItem()); }
	NB_PROP_GET(viewRect)				{ return push(v, self(v)->GetViewRect()); }
	NB_PROP_GET(inReportView)			{ return push(v, self(v)->InReportView()); }
	NB_PROP_GET(virtual)				{ return push(v, self(v)->IsVirtual()); }

	NB_PROP_SET(normalImageList)		{ self(v)->SetImageList(get<wxImageList>(v, 2), wxIMAGE_LIST_NORMAL); return 0; }
	NB_PROP_SET(smallImageList)			{ self(v)->SetImageList(get<wxImageList>(v, 2), wxIMAGE_LIST_SMALL); return 0; }
	NB_PROP_SET(itemCount)				{ self(v)->SetItemCount(getInt(v, 2)); return 0; }
	NB_PROP_SET(textColor)				{ self(v)->SetTextColour(GetWxColor(v, 2)); return 0; }

	NB_CONS()
	{
		type* self = new type(
			get<wxWindow>(v, 2), 
			optInt(v, 3, wxID_ANY),
			*opt<wxPoint>(v, 4, wxDefaultPosition),
			*opt<wxSize>(v, 5, wxDefaultSize),
			optInt(v, 6, wxLC_ICON),
			*opt<wxValidator>(v, 7, wxDefaultValidator),
			optWxString(v, 8, wxListCtrlNameStr)
			);

		setSelf(v, self);
		return SQ_OK;
	}

	NB_FUNC(appendColumn)				{ return push(v, self(v)->AppendColumn(getWxString(v, 2), optInt(v, 3, wxLIST_FORMAT_LEFT), optInt(v, 4, -1))); }
	NB_FUNC(arrange)					{ return push(v, self(v)->Arrange(optInt(v, 2, wxLIST_ALIGN_DEFAULT))); }
	NB_FUNC(clearAll)					{ self(v)->ClearAll(); return 0; }
	NB_FUNC(deleteAllItems)				{ return push(v, self(v)->DeleteAllItems()); }
	NB_FUNC(deleteColumn)				{ return push(v, self(v)->DeleteColumn(getInt(v, 2))); }
	NB_FUNC(deleteItem)					{ return push(v, self(v)->DeleteItem(getInt(v, 2))); }
	NB_FUNC(editLabel)					{ return push(v, self(v)->EditLabel(getInt(v, 2))); }
	NB_FUNC(ensureVisible)				{ return push(v, self(v)->EnsureVisible(getInt(v, 2))); }

	NB_FUNC(findItem)
	{
		if (isString(v, 3))
			return push(v, self(v)->FindItem(getInt(v, 2), getWxString(v, 3), optBool(v, 4, false)));
		else if (isInt(v, 3))
			return push(v, self(v)->FindItem(getInt(v, 2), getInt(v, 3)));
		else
			return push(v, self(v)->FindItem(getInt(v, 2), *get<wxPoint>(v, 3), getInt(v, 4)));
	}

	NB_FUNC(getColumnWidth)				{ return push(v, self(v)->GetColumnWidth(getInt(v, 2))); }
	NB_FUNC(getItemBackColor)			{ return PushWxColor(v, self(v)->GetItemBackgroundColour(getInt(v, 2))); }
	NB_FUNC(getItemData)				{ return push(v, (long)self(v)->GetItemData(getInt(v, 2))); }
	NB_FUNC(getItemFont)				{ return push(v, self(v)->GetItemFont(getInt(v, 2))); }
	NB_FUNC(getItemPosition)			{ wxPoint pos; bool ok = self(v)->GetItemPosition(getInt(v, 2), pos); return ok ? push(v, pos) : 0; }
	NB_FUNC(getItemRect)				{ wxRect rect; bool ok = self(v)->GetItemRect(getInt(v, 2), rect, optInt(v, 3, wxLIST_RECT_BOUNDS)); return ok ? push(v, rect) : 0; }
	NB_FUNC(getItemState)				{ int mask = getInt(v, 3); return push(v, self(v)->GetItemState(getInt(v, 2), mask) == mask); }
	NB_FUNC(getItemText)				{ return push(v, self(v)->GetItemText(getInt(v, 2), optInt(v, 3, 0))); }
	NB_FUNC(getItemTextColor)			{ return PushWxColor(v, self(v)->GetItemTextColour(getInt(v, 2))); }
	NB_FUNC(getNextItem)				{ return push(v, self(v)->GetNextItem(getInt(v, 2), optInt(v, 3, wxLIST_NEXT_ALL), optInt(v, 4, wxLIST_STATE_DONTCARE))); }
	NB_FUNC(getSubItemRect)				{ wxRect rect; bool ok = self(v)->GetSubItemRect(getInt(v, 2), getInt(v, 3), rect, optInt(v, 4, wxLIST_RECT_BOUNDS)); return ok ? push(v, rect) : 0; }

	NB_FUNC(hitTest)					
	{ 
		int flags = 0;
		long subItem = 0;
		int item = self(v)->HitTest(*get<wxPoint>(v, 2), flags, &subItem);
		if (item == -1)
			return 0;
		sq_newarray(v, 0);
		arrayAppend(v, -1, item);
		arrayAppend(v, -1, flags);
		arrayAppend(v, -1, subItem);
		return 1;
	}

	NB_FUNC(insertColumn)				{ return push(v, self(v)->InsertColumn(getInt(v, 2), getWxString(v, 3), optInt(v, 4, wxLIST_FORMAT_LEFT), optInt(v, 5, -1))); }

	NB_FUNC(insertItem)
	{ 
		if (isInt(v, 3))
			return push(v, self(v)->InsertItem(getInt(v, 2), getInt(v, 3)));
		else if (isNone(v, 4))
			return push(v, self(v)->InsertItem(getInt(v, 2), getWxString(v, 3)));
		else
			return push(v, self(v)->InsertItem(getInt(v, 2), getWxString(v, 3), getInt(v, 4)));
	}

	NB_FUNC(refreshItem)				{ self(v)->RefreshItem(getInt(v, 2)); return 0; }
	NB_FUNC(refreshItems)				{ self(v)->RefreshItems(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(scrollList)					{ return push(v, self(v)->ScrollList(getInt(v, 2), getInt(v, 3))); }
	NB_FUNC(setColumnWidth)				{ return push(v, self(v)->SetColumnWidth(getInt(v, 2), getInt(v, 3))); }
	NB_FUNC(setItem)					{ return push(v, self(v)->SetItem(getInt(v, 2), getInt(v, 3), getWxString(v, 4), optInt(v, 5, -1))); }
	NB_FUNC(setItemBackColor)			{ self(v)->SetItemBackgroundColour(getInt(v, 2), GetWxColor(v, 3)); return 0; }
	NB_FUNC(setItemColumnImage)			{ return push(v, self(v)->SetItemColumnImage(getInt(v, 2), getInt(v, 3), getInt(v, 4))); }
	NB_FUNC(setItemData)				{ return push(v, self(v)->SetItemData(getInt(v, 2), getInt(v, 3))); }
	NB_FUNC(setItemFont)				{ self(v)->SetItemFont(getInt(v, 2), *get<wxFont>(v, 3)); return 0; }
	NB_FUNC(setItemImage)				{ return push(v, self(v)->SetItemImage(getInt(v, 2), getInt(v, 3), optInt(v, 4, -1))); }
	NB_FUNC(setItemPosition)			{ return push(v, self(v)->SetItemPosition(getInt(v, 2), *get<wxPoint>(v, 3))); }
	NB_FUNC(setItemState)				{ bool on = getBool(v, 4); int state = getInt(v, 3); return push(v, self(v)->SetItemState(getInt(v, 2), on ? state : 0, state)); }
	NB_FUNC(setItemText)				{ self(v)->SetItemText(getInt(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(setItemTextColor)			{ self(v)->SetItemTextColour(getInt(v, 2), GetWxColor(v, 3)); return 0; }
	NB_FUNC(setSingleStyle)				{ self(v)->SetSingleStyle(getInt(v, 2), optBool(v, 3, true)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, ListView, wxListCtrl);

class NB_WxListView : TNitClass<wxListView>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(firstSelected),
			PROP_ENTRY_R(focusedItem),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(					"(parent: wx.Window, id=wx.ID.ANY, pos=null, size=null, style=STYLE.ICON, validator=null, name=null)"),
			FUNC_ENTRY_H(clearColumnImage,	"(col: int)"),
			FUNC_ENTRY_H(focus,				"(item: int)"),
			FUNC_ENTRY_H(getNextSelected,	"(item: int): int"),
			FUNC_ENTRY_H(isSelected,		"(item: int): bool"),
			FUNC_ENTRY_H(select,			"(item: int, on=true)"),
			FUNC_ENTRY_H(setColumnImage,	"(col: int, imageId: int)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(firstSelected)			{ return push(v, self(v)->GetFirstSelected()); }
	NB_PROP_GET(focusedItem)			{ return push(v, self(v)->GetFocusedItem()); }

	NB_CONS()
	{
		type* self = new type(
			get<wxWindow>(v, 2), 
			optInt(v, 3, wxID_ANY),
			*opt<wxPoint>(v, 4, wxDefaultPosition),
			*opt<wxSize>(v, 5, wxDefaultSize),
			optInt(v, 6, wxLC_ICON),
			*opt<wxValidator>(v, 7, wxDefaultValidator),
			optWxString(v, 8, wxListCtrlNameStr)
			);

		setSelf(v, self);
		return SQ_OK;
	}

	NB_FUNC(clearColumnImage)			{ self(v)->ClearColumnImage(getInt(v, 2)); return 0; }
	NB_FUNC(focus)						{ self(v)->Focus(getInt(v, 2)); return 0; }
	NB_FUNC(getNextSelected)			{ return push(v, self(v)->GetNextSelected(getInt(v, 2))); }
	NB_FUNC(isSelected)					{ return push(v, self(v)->IsSelected(getInt(v, 2))); }
	NB_FUNC(select)						{ self(v)->Select(getInt(v, 2), optBool(v, 3, true)); return 0; }
	NB_FUNC(setColumnImage)				{ self(v)->SetColumnImage(getInt(v, 2), getInt(v, 3)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

SQRESULT NitLibWxControl(HSQUIRRELVM v)
{
	NB_WxButton::Register(v);
	NB_WxStaticBox::Register(v);
	NB_WxStaticText::Register(v);
	NB_WxStaticLine::Register(v);
	NB_WxStaticBitmap::Register(v);
	NB_WxCheckBox::Register(v);
	NB_WxRadioButton::Register(v);
	NB_WxGauge::Register(v);
	NB_WxSlider::Register(v);
	NB_WxScrollBar::Register(v);
	NB_WxSearchCtrl::Register(v);

	NB_WxColorPickerCtrl::Register(v);
	NB_WxFontPickerCtrl::Register(v);
	NB_WxFilePickerCtrl::Register(v);
	NB_WxDirPickerCtrl::Register(v);
	NB_WxGenericDirCtrl::Register(v);

	NB_WxHyperlinkCtrl::Register(v);
	NB_WxCommandLinkButton::Register(v);
	NB_WxBannerWindow::Register(v);
	NB_WxInfoBar::Register(v);
	NB_WxRichToolTip::Register(v);
	NB_WxEditableListBox::Register(v);

	NB_WxChoice::Register(v);
	NB_WxComboBox::Register(v);
	NB_WxBitmapComboBox::Register(v);
	NB_WxRadioBox::Register(v);
	NB_WxListBox::Register(v);
	NB_WxCheckListBox::Register(v);

	NB_WxCollapsiblePane::Register(v);

	NB_WxBookCtrlBase::Register(v);
	NB_WxNotebook::Register(v);
	NB_WxChoicebook::Register(v);

	NB_WxHtmlWindow::Register(v);

	NB_WxVListBox::Register(v);
	NB_WxHtmlListBox::Register(v);
	NB_WxSimpleHtmlListBox::Register(v);
	
	NB_WxWebViewEvent::Register(v);
	NB_WxWebView::Register(v);

	NB_WxToolBar::Register(v);
	NB_WxToolBarTool::Register(v);

	NB_WxListCtrl::Register(v);
	NB_WxListView::Register(v);

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
