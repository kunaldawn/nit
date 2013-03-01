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

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NITWX_API NB_WxTextEntryBase : TNitInterface<wxTextEntryBase>
{
public:
	template <typename TClass>
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(editable),
			PROP_ENTRY_R(empty),
			PROP_ENTRY	(hint),
			PROP_ENTRY	(value),
			PROP_ENTRY_R(stringSelection),			// TODO: Hides StringSelection.Setter of ItemContainerImmutable
			PROP_ENTRY	(insertionPoint),
			PROP_ENTRY_R(lastPosition),
			PROP_ENTRY	(margins),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(canCopy,					"(): bool"),
			FUNC_ENTRY_H(canCut,					"(): bool"),
			FUNC_ENTRY_H(canPaste,					"(): bool"),
			FUNC_ENTRY_H(canUndo,					"(): bool"),
			FUNC_ENTRY_H(canRedo,					"(): bool"),

			FUNC_ENTRY_H(clear,						"()"),
			FUNC_ENTRY_H(copy,						"()"),
			FUNC_ENTRY_H(paste,						"()"),
			FUNC_ENTRY_H(cut,						"()"),
			FUNC_ENTRY_H(undo,						"()"),
			FUNC_ENTRY_H(redo,						"()"),
			FUNC_ENTRY_H(selectAll,					"()"),

			FUNC_ENTRY_H(appendText,				"(text: string) // append text at the end"),
			FUNC_ENTRY_H(writeText,					"(text: string) // write text at insertion point"),
			FUNC_ENTRY_H(changeValue,				"(value: string) // does not generate update event"),

			FUNC_ENTRY_H(setMaxLength,				"(len: int)"),
			FUNC_ENTRY_H(getSelection,				"(): [from, to: int]"),
			FUNC_ENTRY_H(setSelection,				"(from, to: int)"),
			FUNC_ENTRY_H(getStringSelection,		"(): wxString // same as 'TextCtrl.Selection'"),
			FUNC_ENTRY_H(getRange,					"(from, to: int): wxString"),
			FUNC_ENTRY_H(setInsertionPointEnd,		"()"),

			FUNC_ENTRY_H(autoComplete,				"(choices: string[]): bool"),
			NULL
		};

		bind<TClass>(v, props, funcs);

		addStaticTable<TClass>(v, "EVT");
		newSlot(v, -1, "UPDATED",		(int)wxEVT_COMMAND_TEXT_UPDATED);
		newSlot(v, -1, "ENTER",			(int)wxEVT_COMMAND_TEXT_ENTER);
		newSlot(v, -1, "URL",			(int)wxEVT_COMMAND_TEXT_URL);
		newSlot(v, -1, "MAXLEN",		(int)wxEVT_COMMAND_TEXT_MAXLEN);
		newSlot(v, -1, "CUT",			(int)wxEVT_COMMAND_TEXT_CUT);
		newSlot(v, -1, "COPY",			(int)wxEVT_COMMAND_TEXT_COPY);
		newSlot(v, -1, "PASTE",			(int)wxEVT_COMMAND_TEXT_PASTE);
		sq_poptop(v);
	}

	NB_PROP_GET(editable)				{ return push(v, self(v)->IsEditable()); }
	NB_PROP_GET(empty)					{ return push(v, self(v)->IsEmpty()); }
	NB_PROP_GET(hint)					{ return push(v, self(v)->GetHint()); }
	NB_PROP_GET(value)					{ return push(v, self(v)->GetValue()); }
	NB_PROP_GET(stringSelection)		{ return push(v, self(v)->GetStringSelection()); }
	NB_PROP_GET(insertionPoint)			{ return push(v, self(v)->GetInsertionPoint()); }
	NB_PROP_GET(lastPosition)			{ return push(v, self(v)->GetLastPosition()); }
	NB_PROP_GET(margins)				{ return push(v, self(v)->GetMargins()); }

	NB_PROP_SET(editable)				{ self(v)->SetEditable(getBool(v, 2)); return 0; }
	NB_PROP_SET(hint)					{ self(v)->SetHint(getWxString(v, 2)); return 0; }
	NB_PROP_SET(value)					{ self(v)->SetValue(getWxString(v, 2)); return 0; }
	NB_PROP_SET(insertionPoint)			{ self(v)->SetInsertionPoint(getInt(v, 2)); return 0; }
	NB_PROP_SET(margins)				{ self(v)->SetMargins(*get<wxPoint>(v, 2)); return 0; }

	NB_FUNC(canCopy)					{ return push(v, self(v)->CanCopy()); }
	NB_FUNC(canCut)						{ return push(v, self(v)->CanCut()); }
	NB_FUNC(canPaste)					{ return push(v, self(v)->CanPaste()); }
	NB_FUNC(canUndo)					{ return push(v, self(v)->CanUndo()); }
	NB_FUNC(canRedo)					{ return push(v, self(v)->CanRedo()); }

	NB_FUNC(clear)						{ self(v)->Clear(); return 0; }
	NB_FUNC(copy)						{ self(v)->Copy(); return 0; }
	NB_FUNC(paste)						{ self(v)->Paste(); return 0; }
	NB_FUNC(cut)						{ self(v)->Cut(); return 0; }
	NB_FUNC(undo)						{ self(v)->Undo(); return 0; }
	NB_FUNC(redo)						{ self(v)->Redo(); return 0; }
	NB_FUNC(selectAll)					{ self(v)->SelectAll(); return 0; }

	NB_FUNC(appendText)					{ self(v)->AppendText(getWxString(v, 2)); return 0; }
	NB_FUNC(writeText)					{ self(v)->WriteText(getWxString(v, 2)); return 0; }
	NB_FUNC(changeValue)				{ self(v)->ChangeValue(getWxString(v, 2)); return 0; }

	NB_FUNC(setMaxLength)				{ self(v)->SetMaxLength(getInt(v, 2)); return 0; }
	NB_FUNC(setSelection)				{ self(v)->SetSelection(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(setInsertionPointEnd)		{ self(v)->SetInsertionPointEnd(); return 0; }
	NB_FUNC(getStringSelection)			{ return push(v, self(v)->GetStringSelection()); }
	NB_FUNC(getRange)					{ return push(v, self(v)->GetRange(getInt(v, 2), getInt(v, 3))); }

	NB_FUNC(getSelection)
	{
		sq_newarray(v, 0);
		long from, to;
		self(v)->GetSelection(&from , &to);
		arrayAppend(v, -1, from);
		arrayAppend(v, -1, to);
		return 1;
	}

	NB_FUNC(autoComplete)
	{
		wxArrayString choices;
		if (GetWxArrayString(v, 2, choices) <= 0)
			return sq_throwerror(v, "invalid choice array");

		return push(v, self(v)->AutoComplete(choices));
	}
};

////////////////////////////////////////////////////////////////////////////////

class NITWX_API NB_WxTextAreaBase : TNitInterface<wxTextAreaBase>
{
public:
	template <typename TClass>
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(numberOfLines),
			PROP_ENTRY	(modified),
			PROP_ENTRY	(value),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(getLineLength,	"(lineNo: int): int"),
			FUNC_ENTRY_H(getLineText,	"(lineNo: int): string"),
			FUNC_ENTRY_H(positionToxy,	"(pos: int): Point)"),
			FUNC_ENTRY_H(xyToPosition,	"(xy: Point): int"),
			FUNC_ENTRY_H(showPosition,	"(pos: int)"),

			FUNC_ENTRY_H(loadFile,		"(filename: string, type=TEXT_TYPE_ANY): bool"),
			FUNC_ENTRY_H(saveFile,		"(filename=\"\", type=TEXT_TYPE_ANY): bool"),
			NULL
		};

		bind<TClass>(v, props, funcs);
	}

	NB_PROP_GET(numberOfLines)			{ return push(v, self(v)->GetNumberOfLines()); }
	NB_PROP_GET(modified)				{ return push(v, self(v)->IsModified()); }
	NB_PROP_GET(value)					{ return push(v, self(v)->GetValue()); }

	NB_PROP_SET(modified)				{ self(v)->SetModified(getBool(v, 2)); return 0; }
	NB_PROP_SET(value)					{ self(v)->SetValue(getWxString(v, 2)); return 0; }

	NB_FUNC(getLineLength)				{ return push(v, self(v)->GetLineLength(getInt(v, 2))); }
	NB_FUNC(getLineText)				{ return push(v, self(v)->GetLineText(getInt(v, 2))); }
	NB_FUNC(positionToxy)				{ long x, y; self(v)->PositionToXY(getInt(v, 2), &x, &y); return push(v, wxPoint(x, y)); }
	NB_FUNC(xyToPosition)				{ wxPoint xy = *get<wxPoint>(v, 2); return push(v, self(v)->XYToPosition(xy.x, xy.y)); }
	NB_FUNC(showPosition)				{ self(v)->ShowPosition(getInt(v, 2)); return 0; }
	NB_FUNC(loadFile)					{ return push(v, self(v)->LoadFile(getWxString(v, 2), optInt(v, 3, wxTEXT_TYPE_ANY))); }
	NB_FUNC(saveFile)					{ return push(v, self(v)->SaveFile(optWxString(v, 2, ""), optInt(v, 3, wxTEXT_TYPE_ANY))); }
};

////////////////////////////////////////////////////////////////////////////////

class NITWX_API NB_WxTextCtrlBase : TNitInterface<wxTextCtrlBase>
{
public:
	template <typename TClass>
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(multiLine),
			PROP_ENTRY_R(singleLine),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind<TClass>(v, props, funcs);
	}

	NB_PROP_GET(multiLine)				{ return push(v, self(v)->IsMultiLine()); }
	NB_PROP_GET(singleLine)				{ return push(v, self(v)->IsSingleLine()); }
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
