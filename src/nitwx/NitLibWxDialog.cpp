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

NB_TYPE_WXOBJ(NITWX_API, MessageDialog, wxDialog);

class NB_WxMessageDialog : TNitClass<wxMessageDialog>
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
			CONS_ENTRY_H(						"(parent: Window=null, message: string, caption=null, style=STYLE.DEFAULT, pos=null)"),
			FUNC_ENTRY_H(setExtendedMessage,	"(extMsg: string)"),
			FUNC_ENTRY_H(setMessage,			"(msg: string)"),
			FUNC_ENTRY_H(setOKCancelLabels,		"(ok, cancel: string): bool"),
			FUNC_ENTRY_H(setOKLabel,			"(ok: string): bool"),
			FUNC_ENTRY_H(setYesNoCancelLabels,	"(yes, no, cancel: string): bool"),
			FUNC_ENTRY_H(setYesNoLabels,		"(yes, no: string): bool"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "DEFAULT",				(int)(wxOK | wxCENTER));
		newSlot(v, -1, "OK",					(int)wxOK);
		newSlot(v, -1, "CANCEL",				(int)wxCANCEL);
		newSlot(v, -1, "YES_NO",				(int)wxYES_NO);
		newSlot(v, -1, "NO_DEFAULT",			(int)wxNO_DEFAULT);
		newSlot(v, -1, "CANCEL_DEFAULT",		(int)wxCANCEL_DEFAULT);
		newSlot(v, -1, "YES_DEFAULT",			(int)wxYES_DEFAULT);
		newSlot(v, -1, "OK_DEFAULT",			(int)wxOK_DEFAULT);
		newSlot(v, -1, "STAY_ON_TOP",			(int)wxSTAY_ON_TOP);
		newSlot(v, -1, "CENTER",				(int)wxCENTER);
		sq_poptop(v);

		addStaticTable(v, "ICON");
		newSlot(v, -1, "NONE",					(int)wxICON_NONE);
		newSlot(v, -1, "EXCLAMATION",			(int)wxICON_EXCLAMATION);
		newSlot(v, -1, "ERROR",					(int)wxICON_ERROR);
		newSlot(v, -1, "HAND",					(int)wxICON_HAND);
		newSlot(v, -1, "QUESTION",				(int)wxICON_QUESTION);
		newSlot(v, -1, "INFORMATION",			(int)wxICON_INFORMATION);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxDialog* dlg = new wxMessageDialog(
			opt<wxWindow>	(v, 2, NULL),
			getWxString		(v, 3), 
			optWxString		(v, 4, wxMessageBoxCaptionStr),
			optInt			(v, 5, wxOK | wxCENTER),
			*opt<wxPoint>	(v, 6, wxDefaultPosition)
			);

		sq_setinstanceup(v, 1, dlg);
		return 0;
	}

	NB_FUNC(setExtendedMessage)			{ self(v)->SetExtendedMessage(getWxString(v, 2)); return 0; }
	NB_FUNC(setMessage)					{ self(v)->SetMessage(getWxString(v, 2)); return 0; }
	NB_FUNC(setOKCancelLabels)			{ return push(v, self(v)->SetOKCancelLabels(getWxString(v, 2), getWxString(v, 3))); }
	NB_FUNC(setOKLabel)					{ return push(v, self(v)->SetOKLabel(getWxString(v, 2))); }
	NB_FUNC(setYesNoCancelLabels)		{ return push(v, self(v)->SetYesNoCancelLabels(getWxString(v, 2), getWxString(v, 3), getWxString(v, 4))); }
	NB_FUNC(setYesNoLabels)				{ return push(v, self(v)->SetYesNoLabels(getWxString(v, 2), getWxString(v, 3))); }
	NB_FUNC(showModal)					{ return push(v, self(v)->ShowModal()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, FileDialog, wxDialog);

class NB_WxFileDialog : TNitClass<wxFileDialog>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(directory),
			PROP_ENTRY	(filename),
			PROP_ENTRY_R(filenames),
			PROP_ENTRY	(message),
			PROP_ENTRY	(path),
			PROP_ENTRY_R(paths),
			PROP_ENTRY	(wildcard),
			PROP_ENTRY	(filterIndex),
			PROP_ENTRY_R(extraControl),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: Window=null, message=null, defaultDir=null, defaultFile=null, wildcard=null, style=STYLE.DEFAULT, pos=null, size=null, name=null)"),

			// TODO: add ExtraControl methods
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "DEFAULT",			(int)wxFD_DEFAULT_STYLE);
		newSlot(v, -1, "OPEN",				(int)wxFD_OPEN);
		newSlot(v, -1, "SAVE",				(int)wxFD_SAVE);
		newSlot(v, -1, "OVERWRITE_PROMPT",	(int)wxFD_OVERWRITE_PROMPT);
		newSlot(v, -1, "FILE_MUST_EXIST",	(int)wxFD_FILE_MUST_EXIST);
		newSlot(v, -1, "MULTIPLE",			(int)wxFD_MULTIPLE);
		newSlot(v, -1, "CHANGE_DIR",		(int)wxFD_CHANGE_DIR);
		newSlot(v, -1, "PREVIEW",			(int)wxFD_PREVIEW);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxDialog* dlg = new wxFileDialog(
			opt<wxWindow>	(v, 2, NULL),
			optWxString		(v, 3, wxFileSelectorPromptStr), 
			optWxString		(v, 4, ""),
			optWxString		(v, 5, ""),
			optWxString		(v, 6, wxFileSelectorDefaultWildcardStr),
			optInt			(v, 7, wxFD_DEFAULT_STYLE),
			*opt<wxPoint>	(v, 8, wxDefaultPosition),
			*opt<wxSize>	(v, 9, wxDefaultSize),
			optWxString		(v, 10, wxFileDialogNameStr)
			);

		sq_setinstanceup(v, 1, dlg);
		return 0;
	}

	NB_PROP_GET(directory)				{ return push(v, self(v)->GetDirectory()); }
	NB_PROP_GET(filename)				{ return push(v, self(v)->GetFilename()); }
	NB_PROP_GET(filenames)				{ wxArrayString filenames; self(v)->GetFilenames(filenames); return PushWxArray(v, filenames); }
	NB_PROP_GET(message)				{ return push(v, self(v)->GetMessage()); }
	NB_PROP_GET(path)					{ return push(v, self(v)->GetPath()); }
	NB_PROP_GET(paths)					{ wxArrayString paths; self(v)->GetPaths(paths); return PushWxArray(v, paths); }
	NB_PROP_GET(wildcard)				{ return push(v, self(v)->GetWildcard()); }
	NB_PROP_GET(filterIndex)			{ return push(v, self(v)->GetFilterIndex()); }
	NB_PROP_GET(extraControl)			{ return push(v, self(v)->GetExtraControl()); }

	NB_PROP_SET(directory)				{ self(v)->SetDirectory(getWxString(v, 2)); return 0; }
	NB_PROP_SET(filename)				{ self(v)->SetFilename(getWxString(v, 2)); return 0; }
	NB_PROP_SET(message)				{ self(v)->SetMessage(getWxString(v, 2)); return 0; }
	NB_PROP_SET(path)					{ self(v)->SetPath(getWxString(v, 2)); return 0; }
	NB_PROP_SET(wildcard)				{ self(v)->SetWildcard(getWxString(v, 2)); return 0; }
	NB_PROP_SET(filterIndex)			{ self(v)->SetFilterIndex(getInt(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, DirDialog, wxDialog);

class NB_WxDirDialog : TNitClass<wxDirDialog>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(message),
			PROP_ENTRY	(path),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(					"(parent: Window=null, message=null, defaultPath=null, style=STYLE.DEFAULT, pos=null, size=null, name=null)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "DEFAULT",		(int)wxDD_DEFAULT_STYLE);
		newSlot(v, -1, "DIR_MUST_EXIST",(int)wxDD_DIR_MUST_EXIST);
		newSlot(v, -1, "CHANGE_DIR",	(int)wxDD_CHANGE_DIR);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxDialog* dlg = new wxDirDialog(
			opt<wxWindow>	(v, 2, NULL),
			optWxString		(v, 3, wxDirSelectorPromptStr), 
			optWxString		(v, 4, ""),
			optInt			(v, 5, wxDD_DEFAULT_STYLE),
			*opt<wxPoint>	(v, 6, wxDefaultPosition),
			*opt<wxSize>	(v, 7, wxDefaultSize),
			optWxString		(v, 8, wxDirDialogNameStr)
			);

		sq_setinstanceup(v, 1, dlg);
		return 0;
	}

	NB_PROP_GET(message)				{ return push(v, self(v)->GetMessage()); }
	NB_PROP_GET(path)					{ return push(v, self(v)->GetPath()); }

	NB_PROP_SET(message)				{ self(v)->SetMessage(getWxString(v, 2)); return 0; }
	NB_PROP_SET(path)					{ self(v)->SetPath(getWxString(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, RearrangeDialog, wxDialog);

class NB_WxRearrangeDialog : TNitClass<wxRearrangeDialog>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(order),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(					"(parent: Window=null, message, title: string, order: int[], items: string[], pos=null, name=null)"),
			FUNC_ENTRY_H(addExtraControls,	"(win: Window)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		wxArrayInt order;
		if (GetWxArrayInt(v, 5, order) < 0)
			return sq_throwerror(v, "invalid order array");

		wxArrayString items;
		if (GetWxArrayString(v, 6, items) < 0)
			return sq_throwerror(v, "inalid items array");

		wxDialog* dlg = new wxRearrangeDialog(
			opt<wxWindow>	(v, 2, NULL),
			getWxString		(v, 3),
			getWxString		(v, 4),
			order,
			items,
			*opt<wxPoint>	(v, 7, wxDefaultPosition),
			optWxString		(v, 8, wxRearrangeDialogNameStr)
			);

		sq_setinstanceup(v, 1, dlg);
		return 0;
	}

	NB_PROP_GET(order)					{ return PushWxArray(v, self(v)->GetOrder()); }

	NB_FUNC(addExtraControls)			{ self(v)->AddExtraControls(get<wxWindow>(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, ProgressDialog, wxDialog);

class NB_WxProgressDialog : TNitClass<wxProgressDialog>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(value),
			PROP_ENTRY	(range),
			PROP_ENTRY_R(message),
			PROP_ENTRY_R(cancelled),
			PROP_ENTRY_R(skipped),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(					"(title, message: string, maximum=100, parent: Window=NULL, style=STYLE.DEFAULT)"),
			FUNC_ENTRY_H(pulse,				"(newMsg=\"\"): bool"),
			FUNC_ENTRY_H(resume,			"()"),
			FUNC_ENTRY_H(update,			"(value: int, newMsg=\"\"): bool"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "DEFAULT",		(int)wxPD_AUTO_HIDE | wxPD_APP_MODAL);
		newSlot(v, -1, "APP_MODAL",		(int)wxPD_APP_MODAL);
		newSlot(v, -1, "AUTO_HIDE",		(int)wxPD_AUTO_HIDE);
		newSlot(v, -1, "SMOOTH",		(int)wxPD_SMOOTH);
		newSlot(v, -1, "CAN_ABORT",		(int)wxPD_CAN_ABORT);
		newSlot(v, -1, "CAN_SKIP",		(int)wxPD_CAN_SKIP);
		newSlot(v, -1, "ELAPSED_TIME",	(int)wxPD_ELAPSED_TIME);
		newSlot(v, -1, "ESTIMATED_TIME",(int)wxPD_ESTIMATED_TIME);
		newSlot(v, -1, "REMAINING_TIME",(int)wxPD_REMAINING_TIME);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxDialog* dlg = new wxProgressDialog(
			getWxString		(v, 2),
			getWxString		(v, 3),
			optInt			(v, 4, 100),
			opt<wxWindow>	(v, 5, NULL),
			optInt			(v, 6, wxPD_AUTO_HIDE | wxPD_APP_MODAL)
			);

		sq_setinstanceup(v, 1, dlg);
		return 0;
	}

	NB_PROP_GET(value)					{ return push(v, self(v)->GetValue()); }
	NB_PROP_GET(range)					{ return push(v, self(v)->GetRange()); }
	NB_PROP_GET(message)				{ return push(v, self(v)->GetMessage()); }
	NB_PROP_GET(cancelled)				{ return push(v, self(v)->WasCancelled()); }
	NB_PROP_GET(skipped)				{ return push(v, self(v)->WasSkipped()); }

	NB_PROP_SET(range)					{ self(v)->SetRange(getInt(v, 2)); return 0; }

	NB_FUNC(pulse)						{ return push(v, self(v)->Pulse(optWxString(v, 2, ""))); }
	NB_FUNC(update)						{ return push(v, self(v)->Update(getInt(v, 2), optWxString(v, 3, ""))); }
	NB_FUNC(resume)						{ self(v)->Resume(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

// special for AUTODELETE
NB_TYPE_WX_AUTODELETE(NITWX_API, BusyInfo, NULL, delete);

class NB_WxBusyInfo : TNitClass<wxBusyInfo>
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
			CONS_ENTRY_H(				"(msg: string, parent: Window=null)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		sq_setinstanceup(v, 1, new wxBusyInfo(getWxString(v, 2), opt<wxWindow>(v, 3, NULL)));
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

SQRESULT NitLibWxDialog(HSQUIRRELVM v)
{
	NB_WxMessageDialog::Register(v);
	NB_WxFileDialog::Register(v);
	NB_WxDirDialog::Register(v);
	NB_WxRearrangeDialog::Register(v);
	NB_WxProgressDialog::Register(v);
	NB_WxBusyInfo::Register(v);

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
