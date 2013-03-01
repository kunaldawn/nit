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

#define NEWSLOT(key, val) newSlot(v, -1, key, val)

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, TextCtrl, wxControl);

class NB_WxTextCtrl : TNitClass<wxTextCtrl>
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
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, value=\"\", pos=null, size=null, style=0, validator=null, name=null)"),
			NULL
		};

		bind(v, props, funcs);

		NB_WxTextEntryBase::Register<type>(v);
		NB_WxTextCtrlBase::Register<type>(v);

		addStaticTable(v, "TEXT_TYPE");
		NEWSLOT("ANY",					(int)wxTEXT_TYPE_ANY);
		sq_poptop(v);

		addStaticTable(v, "STYLE");
		NEWSLOT("PROCESS_ENTER",		(int)wxTE_PROCESS_ENTER);
		NEWSLOT("PROCESS_TAB",			(int)wxTE_PROCESS_TAB);
		NEWSLOT("MULTILINE",			(int)wxTE_MULTILINE);
		NEWSLOT("PASSWORD",				(int)wxTE_PASSWORD);
		NEWSLOT("READONLY",				(int)wxTE_READONLY);
		NEWSLOT("RICH",					(int)wxTE_RICH);
		NEWSLOT("RICH2",				(int)wxTE_RICH2);
		NEWSLOT("AUTO_URL",				(int)wxTE_AUTO_URL);
		NEWSLOT("NOHIDESEL",			(int)wxTE_NOHIDESEL);
		NEWSLOT("HSCROLL",				(int)wxHSCROLL);
		NEWSLOT("NO_VSCROLL",			(int)wxTE_NO_VSCROLL);
		NEWSLOT("LEFT",					(int)wxTE_LEFT);
		NEWSLOT("CENTER",				(int)wxTE_CENTER);
		NEWSLOT("RIGHT",				(int)wxTE_RIGHT);
		NEWSLOT("DONTWRAP",				(int)wxTE_DONTWRAP);
		NEWSLOT("CHARWRAP",				(int)wxTE_CHARWRAP);
		NEWSLOT("WORDWRAP",				(int)wxTE_WORDWRAP);
		NEWSLOT("BESTWRAP",				(int)wxTE_BESTWRAP);
		NEWSLOT("CAPITALIZE",			(int)wxTE_CAPITALIZE);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxControl* ctrl = new wxTextCtrl(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			optWxString(v, 4, ""),
			*opt<wxPoint>(v, 5, wxDefaultPosition),
			*opt<wxSize>(v, 6, wxDefaultSize),
			optInt(v, 7, 0),
			*opt<wxValidator>(v, 8, (wxValidator*)&wxDefaultValidator), // TODO: import this
			optWxString(v, 9, wxTextCtrlNameStr));

		sq_setinstanceup(v, 1, ctrl);
		return 0; 
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, RichTextCtrl, wxControl);

class NB_WxRichTextCtrl : TNitClass<wxRichTextCtrl>
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
			CONS_ENTRY_H(				"(parent: Window=null, id=wx.ID.ANY, value=\"\", pos=null, size=null, style=STYLE.MULTILINE, validator=null, name=null)"),

			FUNC_ENTRY_H(addImage,		"(image: Image)"),
			FUNC_ENTRY_H(addParagraph,	"(text: string)"),

			// TODO: wxRichTextRange
			// TODO: wxTextAttr
			// TODO: Style attribute and other features

			NULL
		};

		bind(v, props, funcs);

		NB_WxTextEntryBase::Register<type>(v);
		NB_WxTextAreaBase::Register<type>(v);

		addStaticTable(v, "STYLE");
		NEWSLOT("CENTER_CARET",			(int)wxRE_CENTER_CARET);
		NEWSLOT("MULTILINE",			(int)wxRE_MULTILINE);
		NEWSLOT("READONLY",				(int)wxRE_READONLY);
		sq_poptop(v);

		addStaticTable(v, "TEXT_TYPE");
		NEWSLOT("ANY",					(int)wxRICHTEXT_TYPE_ANY);
		NEWSLOT("HTML",					(int)wxRICHTEXT_TYPE_HTML);
		NEWSLOT("PDF",					(int)wxRICHTEXT_TYPE_PDF);
		NEWSLOT("RTF",					(int)wxRICHTEXT_TYPE_RTF);
		NEWSLOT("TEXT",					(int)wxRICHTEXT_TYPE_TEXT);
		NEWSLOT("XML",					(int)wxRICHTEXT_TYPE_XML);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxRichTextCtrl* ctrl = new wxRichTextCtrl(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			optWxString(v, 4, ""),
			*opt<wxPoint>(v, 5, wxDefaultPosition),
			*opt<wxSize>(v, 6, wxDefaultSize),
			optInt(v, 7, wxRE_MULTILINE),
			*opt<wxValidator>(v, 8, (wxValidator*)&wxDefaultValidator), // TODO: import this
			optWxString(v, 9, wxTextCtrlNameStr));

		sq_setinstanceup(v, 1, ctrl);
		return 0; 
	}

	NB_FUNC(addImage)					{ self(v)->AddImage(*get<wxImage>(v, 2)); return 0; }
	NB_FUNC(addParagraph)				{ self(v)->AddParagraph(getWxString(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, StyledTextEvent, wxCommandEvent);

class NB_WxStyledTextEvent : TNitClass<wxStyledTextEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(alt),
			PROP_ENTRY_R(control),
			PROP_ENTRY	(dragAllowMove),
			PROP_ENTRY	(dragResult),
			PROP_ENTRY	(dragText),
			PROP_ENTRY_R(foldLevelNow),
			PROP_ENTRY_R(foldLevelPrev),
			PROP_ENTRY_R(key),
			PROP_ENTRY_R(lParam),
			PROP_ENTRY_R(length),
			PROP_ENTRY_R(line),
			PROP_ENTRY_R(linesAdded),
			PROP_ENTRY_R(listType),
			PROP_ENTRY_R(margin),
			PROP_ENTRY_R(message),
			PROP_ENTRY_R(modificationType),
			PROP_ENTRY_R(modifiers),
			PROP_ENTRY_R(position),
			PROP_ENTRY_R(shift),
			PROP_ENTRY_R(text),
			PROP_ENTRY_R(wparam),
			PROP_ENTRY_R(x),
			PROP_ENTRY_R(y),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(alt)					{ return push(v, self(v)->GetAlt()); }
	NB_PROP_GET(control)				{ return push(v, self(v)->GetControl()); }
	NB_PROP_GET(dragAllowMove)			{ return push(v, self(v)->GetDragAllowMove()); }
	NB_PROP_GET(dragResult)				{ return push(v, (int)self(v)->GetDragResult()); }
	NB_PROP_GET(dragText)				{ return push(v, self(v)->GetDragText()); }
	NB_PROP_GET(foldLevelNow)			{ return push(v, self(v)->GetFoldLevelNow()); }
	NB_PROP_GET(foldLevelPrev)			{ return push(v, self(v)->GetFoldLevelPrev()); }
	NB_PROP_GET(key)					{ return push(v, self(v)->GetKey()); }
	NB_PROP_GET(lParam)					{ return push(v, self(v)->GetLParam()); }
	NB_PROP_GET(length)					{ return push(v, self(v)->GetLength()); }
	NB_PROP_GET(line)					{ return push(v, self(v)->GetLine()); }
	NB_PROP_GET(linesAdded)				{ return push(v, self(v)->GetLinesAdded()); }
	NB_PROP_GET(listType)				{ return push(v, self(v)->GetListType()); }
	NB_PROP_GET(margin)					{ return push(v, self(v)->GetMargin()); }
	NB_PROP_GET(message)				{ return push(v, self(v)->GetMessage()); }
	NB_PROP_GET(modificationType)		{ return push(v, self(v)->GetModificationType()); }
	NB_PROP_GET(modifiers)				{ return push(v, self(v)->GetModifiers()); }
	NB_PROP_GET(position)				{ return push(v, self(v)->GetPosition()); }
	NB_PROP_GET(shift)					{ return push(v, self(v)->GetShift()); }
	NB_PROP_GET(text)					{ return push(v, self(v)->GetText()); }
	NB_PROP_GET(wparam)					{ return push(v, self(v)->GetWParam()); }
	NB_PROP_GET(x)						{ return push(v, self(v)->GetX()); }
	NB_PROP_GET(y)						{ return push(v, self(v)->GetY()); }

	NB_PROP_SET(dragAllowMove)			{ self(v)->SetDragAllowMove(getBool(v, 2)); return 0; }
	NB_PROP_SET(dragResult)				{ self(v)->SetDragResult((wxDragResult)getInt(v, 2)); return 0; }
	NB_PROP_SET(dragText)				{ self(v)->SetDragText(getWxString(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, StyledTextCtrl, wxControl);

class NB_WxStyledTextCtrl : TNitClass<wxStyledTextCtrl>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(autoCompActive),			// : bool
			PROP_ENTRY	(autoCompAutoHide),			// : bool
			PROP_ENTRY	(autoCompCancelAtStart),	// : bool
			PROP_ENTRY	(autoCompChooseSingle),		// : bool
			PROP_ENTRY_R(autoCompCurrent),			// : int as item_index
			PROP_ENTRY	(autoCompDropRestOfWord),	// : bool
			PROP_ENTRY	(autoCompIgnoreCase),		// : bool
			PROP_ENTRY	(autoCompMaxHeight),		// : int as num_rows
			PROP_ENTRY	(autoCompMaxWidth),			// : int as num_chars
			PROP_ENTRY	(autoCompSeparator),		// : int as char
			PROP_ENTRY	(autoCompTypeSeparator),	// : int as char
			PROP_ENTRY_R(autoCompPosStart),			// : int as pos
			PROP_ENTRY_R(callTipActive),			// : bool
			PROP_ENTRY_R(callTipPosAtStart),		// : int as pos
//			PROP_ENTRY_R(canPaste),					// : bool          // TODO: already in wxTextEntryBase?
//			PROP_ENTRY_R(canRedo),					// : bool
//			PROP_ENTRY_R(canUndo),					// : bool
			PROP_ENTRY_R(anchor),					// : int as pos
			PROP_ENTRY	(backSpaceUnIndents),		// : bool 
			PROP_ENTRY	(bufferedDraw),				// : bool
			PROP_ENTRY	(caretForeColor),			// : Color
			PROP_ENTRY	(caretLineBackAlpha),		// : int as (0~255) ?
			PROP_ENTRY	(caretLineBackColor),		// : Color
			PROP_ENTRY	(caretLineVisible),			// : bool
			PROP_ENTRY	(caretPeriod),				// : int as millis
			PROP_ENTRY	(caretSticky),				// : bool
			PROP_ENTRY	(caretWidth),				// : int as pixel
			PROP_ENTRY_R(codePage),					// : CP
			PROP_ENTRY_R(lineText),					// : string
			PROP_ENTRY_R(currentLine),				// : int as line
			PROP_ENTRY	(currentPos),				// : int as pos
			PROP_ENTRY	(docPointer),				// : user_pointer
			PROP_ENTRY	(eolMode),					// : EOL
			PROP_ENTRY	(edgeColor),				// : Color
			PROP_ENTRY	(edgeColumn),				// : int as column
			PROP_ENTRY	(edgeMode),					// : EDGE
			PROP_ENTRY	(endAtLastLine),			// : bool
			PROP_ENTRY_R(endStyled),				// : int as pos
			PROP_ENTRY	(firstVisibleLine),			// : int as line
			PROP_ENTRY	(highlightGuide),			// : int as column
			PROP_ENTRY	(keysUnicode),				// : bool
			PROP_ENTRY	(extraAscent),				// : int
			PROP_ENTRY	(extraDescent),				// : int
			PROP_ENTRY	(indent),					// : int as text_count
			PROP_ENTRY	(indentationGuides),		// : INDENT_GUIDE
			PROP_ENTRY	(lastKeydownProcessed),		// : bool
			PROP_ENTRY	(layoutCache),				// : CACHE
			PROP_ENTRY_R(length),					// : int as text_count
			PROP_ENTRY	(lexer),					// : LEXER
			PROP_ENTRY_R(lineCount),				// : int as line
			PROP_ENTRY	(marginLeft),				// : int as pixel
			PROP_ENTRY	(marginRight),				// : int as pixel
			PROP_ENTRY_R(maxLineState),				// : int as line
			PROP_ENTRY	(modEventMask),				// : EVT_MASK
			PROP_ENTRY_R(modified),					// : bool
			PROP_ENTRY	(mouseDownCaptures),		// : bool
			PROP_ENTRY	(mouseDwellTime),			// : int as millis
			PROP_ENTRY	(overtype),					// : bool
			PROP_ENTRY	(pasteConvertEndings),		// : bool
			PROP_ENTRY	(printColorMode),			// : PRINT
			PROP_ENTRY	(printMagnification),		// : int as percentage?
			PROP_ENTRY	(printWrapMode),			// : WRAP
			PROP_ENTRY	(readOnly),					// : bool
			PROP_ENTRY	(stcCursor),				// : CURSOR
			PROP_ENTRY	(stcFocus),					// : bool
			PROP_ENTRY	(scrollWidth),				// : int as pixels
			PROP_ENTRY	(searchFlags),				// : FIND
			PROP_ENTRY	(selAlpha),					// : int as (0~255) ?
			PROP_ENTRY_R(selectedText),				// : string
			PROP_ENTRY	(selectionEnd),				// : int as pos
			PROP_ENTRY	(selectionMode),			// : SEL
			PROP_ENTRY	(selectionStart),			// : int as pos
			PROP_ENTRY	(status),					// : STATUS
			PROP_ENTRY	(styleBits),				// : int
			PROP_ENTRY_R(styleBitsNeeded),			// : bool
			PROP_ENTRY	(tabIndents),				// : bool
			PROP_ENTRY	(tabWidth),					// : int as text_count
			PROP_ENTRY	(targetEnd),				// : int as pos
			PROP_ENTRY	(targetStart),				// : int as pos
			PROP_ENTRY_R(textLength),				// : int as text_count
			PROP_ENTRY	(twoPhaseDraw),				// : bool
			PROP_ENTRY	(undoCollection),			// : bool
			PROP_ENTRY	(useAntiAliasing),			// : bool
			PROP_ENTRY	(useHorizontalScrollBar),	// : bool
			PROP_ENTRY	(useTabs),					// : bool
			PROP_ENTRY	(useVerticalScrollBar),		// : bool
			PROP_ENTRY	(viewEol),					// : bool
			PROP_ENTRY	(viewWhiteSpace),			// : WHITESPACE
			PROP_ENTRY	(wrapIndentMode),			// : WRAPINDENT
			PROP_ENTRY	(wrapMode),					// : WRAP
			PROP_ENTRY	(wrapStartIndent),			// : int as text_count ?
			PROP_ENTRY	(wrapVisualFlags),			// : WRAPVISUALFLAG
			PROP_ENTRY	(wrapVisualFlagsLocation),	// : WRAPVISUALFLAGLOC
			PROP_ENTRY	(xoffset),					// : int as pixels (scroll pos)
			PROP_ENTRY	(zoom),						// : int as percentage ?
			PROP_ENTRY_R(linesOnScreen),			// : int as line_count
			PROP_ENTRY_R(inRectSel),				// : bool
			PROP_ENTRY	(whitespaceSize),			// : int as pixels
			PROP_ENTRY	(annotationVisible),		// : bool
			PROP_ENTRY	(annotationStyleOffset),	// : ?
			PROP_ENTRY	(rectSelCaret),				// : int in pos
			PROP_ENTRY	(rectSelAnchor),			// : int in pos
			PROP_ENTRY	(rectSelCaretVirtualSpace),	// ?
			PROP_ENTRY	(rectSelAnchorVirtualSpace),// ?
			PROP_ENTRY	(virtualSpaceOptions),		// : ?
			PROP_ENTRY	(rectSelModifier),			// : SCMOD
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(						"(parent: wx.Window=null, id=wx.ID.ANY, pos=null, size=null, style=0, name=null)"),
			FUNC_ENTRY_H(addRefDocument,		"(doc: doc_ptr)"),
			FUNC_ENTRY_H(addText,				"(text: string)"),
			FUNC_ENTRY_H(allocate,				"(bytes: int)"),
			FUNC_ENTRY_H(autoCompCancel,		"()"),
			FUNC_ENTRY_H(autoCompComplete,		"()"),
			FUNC_ENTRY_H(autoCompSetFillUps,	"(chars: string)"),
			FUNC_ENTRY_H(autoCompShow,			"(lenEntered: int, itemList: string)"),
			FUNC_ENTRY_H(autoCompSetStops,		"(chars: string)"),
			FUNC_ENTRY_H(beginUndoAction,		"()"),
			FUNC_ENTRY_H(braceBadLight,			"(pos: int)"),
			FUNC_ENTRY_H(braceHighlight,		"(pos1, pos2: int)"),
			FUNC_ENTRY_H(braceMatch,			"(pos: int)"),
			FUNC_ENTRY_H(callTipCancel,			"()"),
			FUNC_ENTRY_H(callTipSetBackColor,	"(color: Color)"),
			FUNC_ENTRY_H(callTipSetForeColor,	"(color: Color)"),
			FUNC_ENTRY_H(callTipSetHighlightColor, "(color: Color)"),
			FUNC_ENTRY_H(callTipHighlight,		"(start, end: int)"),
			FUNC_ENTRY_H(callTipShow,			"(pos: int, definition: string)"),
			FUNC_ENTRY_H(callTipUseStyle,		"(tabSize: int)"),
			FUNC_ENTRY_H(cancel,				"()"),
			FUNC_ENTRY_H(chooseCaretX,			"()"),
			FUNC_ENTRY_H(clear,					"()"),
			FUNC_ENTRY_H(clearAll,				"()"),
			FUNC_ENTRY_H(clearDocumentStyle,	"()"),
			FUNC_ENTRY_H(clearRegisteredImages,	"()"),
			FUNC_ENTRY_H(cmdKeyAssign,			"(key: int, modifiers: SCMOD, cmd: CMD)"),
			FUNC_ENTRY_H(cmdKeyClear,			"(key: int, modifiers: SCMOD)"),
			FUNC_ENTRY_H(cmdKeyClearAll,		"()"),
			FUNC_ENTRY_H(exec,					"(cmd: CMD)"),
			FUNC_ENTRY_H(colorize,				"(start, end)"),
			FUNC_ENTRY_H(convertEOLs,			"(eolMode: EOL)"),
			FUNC_ENTRY_H(copy,					"()"),
			FUNC_ENTRY_H(copyRange,				"(start, end: int)"),
			FUNC_ENTRY_H(copyText,				"(length: int, text: string)"),
			FUNC_ENTRY_H(createDocument,		"(): doc_ptr"),
			FUNC_ENTRY_H(cut,					"()"),
			FUNC_ENTRY_H(docLineFromVisible,	"(lineDisplay: int): int"),
			FUNC_ENTRY_H(editToggleOvertype,	"()"),
			FUNC_ENTRY_H(emptyUndoBuffer,		"()"),
			FUNC_ENTRY_H(endUndoAction,			"()"),
			FUNC_ENTRY_H(ensureCaretVisible,	"()"),
			FUNC_ENTRY_H(ensureVisible,			"(line: int)"),
			FUNC_ENTRY_H(ensureVisibleEnforcePolicy, "(line: int)"),
			FUNC_ENTRY_H(findColumn,			"(line: int, column: int): int"),
			FUNC_ENTRY_H(findText,				"(minPos: int, maxPos: int, text: string, flags: FIND=0): int"),
			FUNC_ENTRY_H(formatRange,			"(doDraw: bool, startPos, endPos: int, dc: wx.DC, target: wx.DC, renderRect: wx.Rect, pageRect: wx.Rect): int"),
			FUNC_ENTRY_H(getCharAt,				"(pos: int): int"),
			FUNC_ENTRY_H(getColumn,				"(pos): int"),
			FUNC_ENTRY_H(getFoldExpanded,		"(line: int): bool"),
			FUNC_ENTRY_H(getFoldLevel,			"(line: int): FOLDLEVEL"),
			FUNC_ENTRY_H(getFoldParent,			"(line: int): int"),
			FUNC_ENTRY_H(getLastChild,			"(line: int, level: FOLDLEVEL): int)"),
			FUNC_ENTRY_H(getLine,				"(line: int): string"),
			FUNC_ENTRY_H(getLineEndPosition,	"(line: int): int"),
			FUNC_ENTRY_H(getLineIndentPosition,	"(line: int): int"),
			FUNC_ENTRY_H(getLineIndentation,	"(line: int): int"),
			FUNC_ENTRY_H(getLineSelEndPosition,	"(line: int): int"),
			FUNC_ENTRY_H(getLineSelStartPosition,	"(line: int): int"),
			FUNC_ENTRY_H(getLineState,			"(line: int): int"), // ??? FOLDFLAG?
			FUNC_ENTRY_H(getLineVisible,		"(line: int): bool"),
			FUNC_ENTRY_H(getMarginMask,			"(margin: int): int"),
			FUNC_ENTRY_H(getMarginSensitive,	"(margin: int): int"),
			FUNC_ENTRY_H(getMarginType,			"(margin: int): int"),
			FUNC_ENTRY_H(getMarginWidth,		"(margin: int): int"),
			FUNC_ENTRY_H(getProperty,			"(key: string): string"),
			FUNC_ENTRY_H(getPropertyExpanded,	"(key: string): string"),
			FUNC_ENTRY_H(getPropertyInt,		"(key: string): int"),
			FUNC_ENTRY_H(getStyleAt,			"(pos: int): int"),
//			FUNC_ENTRY_H(getStyledText,			"(start, end: int): wx.MemoryBuffer"),
			FUNC_ENTRY_H(getText,				"(): string"),
			FUNC_ENTRY_H(getTextRange,			"(start, end): string"),
			FUNC_ENTRY_H(gotoLine,				"(line: int)"),
			FUNC_ENTRY_H(gotoPos,				"(pos: int)"),
			FUNC_ENTRY_H(hideLines,				"(lineStart, lineEnd: int)"),
			FUNC_ENTRY_H(hideSelection,			"(normal: bool)"),
			FUNC_ENTRY_H(indicatorGetAlpha,		"(indicator: INDIC): int"),
			FUNC_ENTRY_H(indicatorSetAlpha,		"(indicator: INDIC, alpha: int)"),
			FUNC_ENTRY_H(indicatorGetForeColor,	"(indicator: INDIC): Color"),
			FUNC_ENTRY_H(indicatorGetStyle,		"(indicator: INDIC): int"),
			FUNC_ENTRY_H(indicatorSetForeColor,	"(indicator: INDIC, fore: Color"),
			FUNC_ENTRY_H(indicatorSetStyle,		"(indicator: INDIC, style: int"),
			FUNC_ENTRY_H(insertText,			"(pos: int, text: string)"),
			FUNC_ENTRY_H(lineFromPosition,		"(pos: int): int"),
			FUNC_ENTRY_H(lineLength,			"(line: int): int"),
			FUNC_ENTRY_H(lineScroll,			"(columns: int, lines: int)"),
			FUNC_ENTRY_H(linesJoin,				"()"),
			FUNC_ENTRY_H(linesSplit,			"(pixelWidth: int)"),
			FUNC_ENTRY_H(loadFile,				"(file: string): bool"),
			FUNC_ENTRY_H(markerAdd,				"(line: int, markerNumber: int): int // returns handle"),
			FUNC_ENTRY_H(markerDefine,			"(markerNumber: int, markerSymbol: MARKER, fore: Color=null, back: Color=null)"),
			FUNC_ENTRY_H(markerDefineBitmap,	"(markerNumber: int, bitmap: wx.Bitmap)"),
			FUNC_ENTRY_H(markerDelete,			"(line: int, markerNumber: int)"),
			FUNC_ENTRY_H(markerDeleteAll,		"(markerNumber: int)"),
			FUNC_ENTRY_H(markerDeleteHandle,	"(handle: int)"),
			FUNC_ENTRY_H(markerGet,				"(line: int): int"),
			FUNC_ENTRY_H(markerLineFromHandle,	"(handle: int): int"),
			FUNC_ENTRY_H(markerNext,			"(lineStart: int, markerMask: int): int"),
			FUNC_ENTRY_H(markerPrevious,		"(lineStart: int, markerMask: int): int"),
			FUNC_ENTRY_H(markerSetAlpha,		"(markerNumber: int, alpha: int)"),
			FUNC_ENTRY_H(markerSetBackColor,	"(markerNumber: int, back: Color)"),
			FUNC_ENTRY_H(markerSetForeColor,	"(markerNumber: int, fore: Color)"),
			FUNC_ENTRY_H(moveCaretInsideView,	"()"),
			FUNC_ENTRY_H(newLine,				"()"),
			FUNC_ENTRY_H(paste,					"()"),
			FUNC_ENTRY_H(pointFromPosition,		"(pos: int): wx.Point"),
			FUNC_ENTRY_H(after,					"(pos: int): int"),
			FUNC_ENTRY_H(before,				"(pos: int): int"),
			FUNC_ENTRY_H(positionFromLine,		"(line: int): int"),
			FUNC_ENTRY_H(positionFromPoint,		"(pt: wx.Point): int"),
			FUNC_ENTRY_H(positionFromPointClose,"(pt: wx.Point): int"),
			FUNC_ENTRY_H(redo,					"()"),
			FUNC_ENTRY_H(registerImage,			"(type: int, bitmap: wx.Bitmap)"),
			FUNC_ENTRY_H(releaseDocument,		"(doc: doc_ptr)"),
			FUNC_ENTRY_H(replaceSelection,		"(text: string)"),
			FUNC_ENTRY_H(replaceTarget,			"(text: string)"),
			FUNC_ENTRY_H(replaceTargetRE,		"(text: string)"),
			FUNC_ENTRY_H(saveFile,				"(file=\"\"): bool"),
			FUNC_ENTRY_H(scrollToColumn,		"(column: int)"),
			FUNC_ENTRY_H(scrollToLine,			"(line: int)"),
			FUNC_ENTRY_H(searchAnchor,			"()"),
			FUNC_ENTRY_H(searchInTarget,		"(text: string): int"),
			FUNC_ENTRY_H(searchNext,			"(flags: FIND, text: string): int"),
			FUNC_ENTRY_H(searchPrev,			"(flags: FIND, text: string): int"),
			FUNC_ENTRY_H(selectionDuplicate,	"()"),
			FUNC_ENTRY_H(sendMsg,				"(msg: int, wp: int, lp: int): int"),
			FUNC_ENTRY_H(setAnchor,				"(posAnchor: int)"),
			FUNC_ENTRY_H(setCharsDefault,		"()"),
			FUNC_ENTRY_H(setControlCharSymbol,	"(symbol: int)"),
			FUNC_ENTRY_H(setFoldExpanded,		"(line: int, expanded: bool)"),
			FUNC_ENTRY_H(setFoldFlags,			"(flags: FOLDFLAG)"),
			FUNC_ENTRY_H(setFoldLevel,			"(line: int, level: FOLDLEVEL)"),
			FUNC_ENTRY_H(setFoldMarginColor,	"(back: Color=null)"),
			FUNC_ENTRY_H(setFoldMarginHiColor,	"(fore: Color=null)"),
			FUNC_ENTRY_H(setHScrollBar,			"(bar: wx.ScrollBar)"),
			FUNC_ENTRY_H(setHotspotActiveBackColor,	"(use: bool, back: Color=null)"),
			FUNC_ENTRY_H(setHotspotActiveForeColor,	"(use: bool, fore: Color=null)"),
			FUNC_ENTRY_H(setHotspotActiveUnderline,	"(flag: bool)"),
			FUNC_ENTRY_H(setHotspotSingleLine,		"(flag: bool)"),
			FUNC_ENTRY_H(setKeywords,			"(keywordSet: int, keywords: string)"),
			FUNC_ENTRY_H(setLexerLanguage,		"(language: string)"),
			FUNC_ENTRY_H(setLineIndentation,	"(line: int, indentSize: int)"),
			FUNC_ENTRY_H(setLineState,			"(line: int, state: int)"),
			FUNC_ENTRY_H(setMarginSensitive,	"(margin: int, sensitive: bool)"),
			FUNC_ENTRY_H(setMarginType,			"(margin: int, type: int)"),
			FUNC_ENTRY_H(setMarginWidth,		"(margin: int, pixelWidth: int)"),
			FUNC_ENTRY_H(setMarginMask,			"(margin: int, mask: int)"),
			FUNC_ENTRY_H(setMargins,			"(left: int, right: int)"),
			FUNC_ENTRY_H(setProperty,			"(key: string, value: string)"),
			FUNC_ENTRY_H(setSavePoint,			"()"),
			FUNC_ENTRY_H(setSelBackColor,		"(back: Color=null)"),
			FUNC_ENTRY_H(setSelForeColor,		"(fore: Color=null)"),
			FUNC_ENTRY_H(setSelection,			"(from, to: int)"),
			FUNC_ENTRY_H(setStyling,			"(length: int, style: int)"),
			FUNC_ENTRY_H(setText,				"(text: string)"),
			FUNC_ENTRY_H(setVScrollBar,			"(bar: wx.ScrollBar)"),
			FUNC_ENTRY_H(setWhitespaceBackColor,"(back: Color=null)"),
			FUNC_ENTRY_H(setWhitespaceChars,	"(chars: string)"),
			FUNC_ENTRY_H(setWhitespaceForeColor,"(fore: Color=null)"),
			FUNC_ENTRY_H(setWordChars,			"(chars: string)"),
			FUNC_ENTRY_H(setXCaretPolicy,		"(policy: CARET_POLICY, slopLines: int)"),
			FUNC_ENTRY_H(setYCaretPolicy,		"(policy: CARET_POLICY, slopLines: int)"),
			FUNC_ENTRY_H(setVisiblePolicy,		"(policy: CARET_POLICY, slopLines: int)"),
			FUNC_ENTRY_H(showLines,				"(lineStart, lineEnd: int)"),
			FUNC_ENTRY_H(startRecord,			"()"),
			FUNC_ENTRY_H(startStyling,			"(pos: int, mask: int)"),
			FUNC_ENTRY_H(stopRecord,			"()"),
			FUNC_ENTRY_H(styleClearAll,			"()"),
			FUNC_ENTRY_H(styleResetDefault,		"()"),
			FUNC_ENTRY_H(styleSetBackColor,		"(style: int, back: Color)"),
			FUNC_ENTRY_H(styleSetBold,			"(style: int, bold: bool)"),
			FUNC_ENTRY_H(styleSetCase,			"(style: int, caseForce: int)"),
			FUNC_ENTRY_H(styleSetChangeable,	"(style: int, changeable: bool)"),
			FUNC_ENTRY_H(styleSetCharacterSet,	"(style: int, characterSet: int)"),
			FUNC_ENTRY_H(styleSetEOLFilled,		"(style: int, filled: bool)"),
			FUNC_ENTRY_H(styleSetFaceName,		"(style: int, faceName: string)"),
			FUNC_ENTRY_H(styleSetFont,			"(style: int, font: wx.Font)"),
			FUNC_ENTRY_H(styleSetForeColor,		"(style: int, fore: Color)"),
			FUNC_ENTRY_H(styleSetHotSpot,		"(style: int, hotspot: bool)"),
			FUNC_ENTRY_H(styleSetItalic,		"(style: int, italic: bool)"),
			FUNC_ENTRY_H(styleSetSize,			"(style: int, sizeInPoints: int)"),
			FUNC_ENTRY_H(styleSetSpec,			"(style: int, spec: string)"),
			FUNC_ENTRY_H(styleSetUnderline,		"(style: int, underline: bool)"),
			FUNC_ENTRY_H(styleSetVisible,		"(style: int, visible: bool)"),
			FUNC_ENTRY_H(targetFromSelection,	"()"),
			FUNC_ENTRY_H(textHeight,			"(line: int): int"),
			FUNC_ENTRY_H(textWidth,				"(style: int, text: string): int"),
			FUNC_ENTRY_H(toggleCaretSticky,		"()"),
			FUNC_ENTRY_H(toggleFold,			"(line: int)"),
			FUNC_ENTRY_H(undo,					"()"),
			FUNC_ENTRY_H(usePopUp,				"(allowPopUp: bool)"),
			FUNC_ENTRY_H(userListShow,			"(listType: int, itemList: string)"),
			FUNC_ENTRY_H(visibleFromDocLine,	"(line: int): int"),
			FUNC_ENTRY_H(wordEndPosition,		"(pos: int, onlyWordChars: bool): int"),
			FUNC_ENTRY_H(wordStartPosition,		"(pos: int, onlyWordChars: bool): int"),
			FUNC_ENTRY_H(wrapCount,				"(line: int): int"),
			FUNC_ENTRY_H(zoomIn,				"()"),
			FUNC_ENTRY_H(zoomOut,				"()"),
			FUNC_ENTRY_H(copyAllowLine,			"()"),
			FUNC_ENTRY_H(getMarkerSymbolDefined,"(markerNumber: int): int"),
			FUNC_ENTRY_H(marginSetText,			"(line: int, text: string)"),
			FUNC_ENTRY_H(marginGetText,			"(line: int): string"),
			FUNC_ENTRY_H(marginSetStyle,		"(line: int, style: int)"),
			FUNC_ENTRY_H(marginGetStyle,		"(line: int): int"),
			FUNC_ENTRY_H(marginSetStyles,		"(line: int, styles: string)"),
			FUNC_ENTRY_H(marginGetStyles,		"(line: int): string"),
			FUNC_ENTRY_H(marginTextClearAll,	"()"),
			FUNC_ENTRY_H(marginSetStyleOffset,	"(style: int)"),
			FUNC_ENTRY_H(annotationSetText,		"(line: int, text: string)"),
			FUNC_ENTRY_H(annotationGetText,		"(line: int): string"),
			FUNC_ENTRY_H(annotationGetStyle,	"(line: int, style: int)"),
			FUNC_ENTRY_H(annotationSetStyle,	"(line: int): int"),
			FUNC_ENTRY_H(annotationSetStyles,	"(line: int, styles: string)"),
			FUNC_ENTRY_H(annotationGetStyles,	"(line: int): string"),
			FUNC_ENTRY_H(annotationClearAll,	"()"),
			FUNC_ENTRY_H(addUndoAction,			"(token: int, flags: int)"),
			FUNC_ENTRY_H(charPositionFromPoint,	"(wx.Point): int"),
			FUNC_ENTRY_H(charPositionFromPointClose, "(wx.Point): int"),
			NULL
		};

		bind(v, props, funcs);

		NB_WxTextEntryBase::Register<type>(v);

		FuncEntry ovr_funcs[] =
		{
			FUNC_ENTRY_H(appendText,			"(text: string)"),
			NULL
		};

		bindMore(v, NULL, ovr_funcs);

		addStatic(v, "LIB_VERSION",		type::GetLibraryVersionInfo().ToString());

		addStatic(v, "INVALID_POS",		(int)wxSTC_INVALID_POSITION);

		addStaticTable(v, "CMD");
		NEWSLOT("BackTab",					(int)wxSTC_CMD_BACKTAB);	
		NEWSLOT("Cancel",					(int)wxSTC_CMD_CANCEL);	
		NEWSLOT("CharLeft",					(int)wxSTC_CMD_CHARLEFT);	
		NEWSLOT("CharLeftExtend",			(int)wxSTC_CMD_CHARLEFTEXTEND);			
		NEWSLOT("CharLeftRectExtend",		(int)wxSTC_CMD_CHARLEFTRECTEXTEND);				
		NEWSLOT("CharRight",				(int)wxSTC_CMD_CHARRIGHT);	
		NEWSLOT("CharRightExtend",			(int)wxSTC_CMD_CHARRIGHTEXTEND);			
		NEWSLOT("CharRightRectExtend",		(int)wxSTC_CMD_CHARRIGHTRECTEXTEND);				
		NEWSLOT("Clear",					(int)wxSTC_CMD_CLEAR);
		NEWSLOT("Copy",						(int)wxSTC_CMD_COPY);
		NEWSLOT("Cut",						(int)wxSTC_CMD_CUT);
		NEWSLOT("DeleteBack",				(int)wxSTC_CMD_DELETEBACK);		
		NEWSLOT("DeleteBackNotLine",		(int)wxSTC_CMD_DELETEBACKNOTLINE);			
		NEWSLOT("DelLineLeft",				(int)wxSTC_CMD_DELLINELEFT);		
		NEWSLOT("DelLineRight",				(int)wxSTC_CMD_DELLINERIGHT);		
		NEWSLOT("DelWordLeft",				(int)wxSTC_CMD_DELWORDLEFT);		
		NEWSLOT("DelWordRight",				(int)wxSTC_CMD_DELWORDRIGHT);		
		NEWSLOT("DelWordRightEnd",			(int)wxSTC_CMD_DELWORDRIGHTEND);			
		NEWSLOT("DocumentEnd",				(int)wxSTC_CMD_DOCUMENTEND);		
		NEWSLOT("DocumentEndExtend",		(int)wxSTC_CMD_DOCUMENTENDEXTEND);			
		NEWSLOT("DocumentStart",			(int)wxSTC_CMD_DOCUMENTSTART);		
		NEWSLOT("DocumentStartExtend",		(int)wxSTC_CMD_DOCUMENTSTARTEXTEND);				
		NEWSLOT("EditToggleOvertype",		(int)wxSTC_CMD_EDITTOGGLEOVERTYPE);				
		NEWSLOT("FormFeed",					(int)wxSTC_CMD_FORMFEED);	
		NEWSLOT("Home",						(int)wxSTC_CMD_HOME);
		NEWSLOT("HomeDisplay",				(int)wxSTC_CMD_HOMEDISPLAY);		
		NEWSLOT("HomeExtend",				(int)wxSTC_CMD_HOMEEXTEND);		
		NEWSLOT("HomeRectExtend",			(int)wxSTC_CMD_HOMERECTEXTEND);			
		NEWSLOT("HomeWrap",					(int)wxSTC_CMD_HOMEWRAP);	
		NEWSLOT("HomeWrapExtend",			(int)wxSTC_CMD_HOMEWRAPEXTEND);			
		NEWSLOT("LineCopy",					(int)wxSTC_CMD_LINECOPY);	
		NEWSLOT("LineCut",					(int)wxSTC_CMD_LINECUT);	
		NEWSLOT("LineDelete",				(int)wxSTC_CMD_LINEDELETE);		
		NEWSLOT("LineDown",					(int)wxSTC_CMD_LINEDOWN);	
		NEWSLOT("LineDownExtend",			(int)wxSTC_CMD_LINEDOWNEXTEND);			
		NEWSLOT("LineDownRectExtend",		(int)wxSTC_CMD_LINEDOWNRECTEXTEND);				
		NEWSLOT("LineDuplicate",			(int)wxSTC_CMD_LINEDUPLICATE);		
		NEWSLOT("LineEnd",					(int)wxSTC_CMD_LINEEND);	
		NEWSLOT("LineEndDisplay",			(int)wxSTC_CMD_LINEENDDISPLAY);			
		NEWSLOT("LineEndDisplayExtend",		(int)wxSTC_CMD_LINEENDDISPLAYEXTEND);				
		NEWSLOT("LineEndExtend",			(int)wxSTC_CMD_LINEENDEXTEND);		
		NEWSLOT("LineEndRectExtend",		(int)wxSTC_CMD_LINEENDRECTEXTEND);			
		NEWSLOT("LineEndWrap",				(int)wxSTC_CMD_LINEENDWRAP);		
		NEWSLOT("LineEndWrapExtend",		(int)wxSTC_CMD_LINEENDWRAPEXTEND);			
		NEWSLOT("LineScrollDown",			(int)wxSTC_CMD_LINESCROLLDOWN);			
		NEWSLOT("LineScrollUp",				(int)wxSTC_CMD_LINESCROLLUP);		
		NEWSLOT("LineTranspose",			(int)wxSTC_CMD_LINETRANSPOSE);		
		NEWSLOT("LineUp",					(int)wxSTC_CMD_LINEUP);	
		NEWSLOT("LineUpExtend",				(int)wxSTC_CMD_LINEUPEXTEND);		
		NEWSLOT("LineUpRectExtend",			(int)wxSTC_CMD_LINEUPRECTEXTEND);			
		NEWSLOT("LowerCase",				(int)wxSTC_CMD_LOWERCASE);	
		NEWSLOT("NewLine",					(int)wxSTC_CMD_NEWLINE);	
		NEWSLOT("PageDown",					(int)wxSTC_CMD_PAGEDOWN);	
		NEWSLOT("PageDownExtend",			(int)wxSTC_CMD_PAGEDOWNEXTEND);			
		NEWSLOT("PageDownRectExtend",		(int)wxSTC_CMD_PAGEDOWNRECTEXTEND);				
		NEWSLOT("PageUp",					(int)wxSTC_CMD_PAGEUP);	
		NEWSLOT("PageUpExtend",				(int)wxSTC_CMD_PAGEUPEXTEND);		
		NEWSLOT("PageUpRectExtend",			(int)wxSTC_CMD_PAGEUPRECTEXTEND);			
		NEWSLOT("ParaDown",					(int)wxSTC_CMD_PARADOWN);	
		NEWSLOT("ParaDownExtend",			(int)wxSTC_CMD_PARADOWNEXTEND);			
		NEWSLOT("ParaUp",					(int)wxSTC_CMD_PARAUP);	
		NEWSLOT("ParaUpExtend",				(int)wxSTC_CMD_PARAUPEXTEND);		
		NEWSLOT("Paste",					(int)wxSTC_CMD_PASTE);
		NEWSLOT("Redo",						(int)wxSTC_CMD_REDO);
		NEWSLOT("SelectAll",				(int)wxSTC_CMD_SELECTALL);	
		NEWSLOT("StutteredPageDown",		(int)wxSTC_CMD_STUTTEREDPAGEDOWN);			
		NEWSLOT("StutteredPageDownExtend",	(int)wxSTC_CMD_STUTTEREDPAGEDOWNEXTEND);					
		NEWSLOT("StutteredPageUp",			(int)wxSTC_CMD_STUTTEREDPAGEUP);			
		NEWSLOT("StutteredPageUpExtend",	(int)wxSTC_CMD_STUTTEREDPAGEUPEXTEND);				
		NEWSLOT("Tab",						(int)wxSTC_CMD_TAB);
		NEWSLOT("Undo",						(int)wxSTC_CMD_UNDO);
		NEWSLOT("UpperCase",				(int)wxSTC_CMD_UPPERCASE);	
		NEWSLOT("VCHome",					(int)wxSTC_CMD_VCHOME);	
		NEWSLOT("VCHomeExtend",				(int)wxSTC_CMD_VCHOMEEXTEND);		
		NEWSLOT("VCHomeRectExtend",			(int)wxSTC_CMD_VCHOMERECTEXTEND);			
		NEWSLOT("VCHomeWrap",				(int)wxSTC_CMD_VCHOMEWRAP);		
		NEWSLOT("VCHomeWrapExtend",			(int)wxSTC_CMD_VCHOMEWRAPEXTEND);			
		NEWSLOT("WordLeft",					(int)wxSTC_CMD_WORDLEFT);	
		NEWSLOT("WordLeftEnd",				(int)wxSTC_CMD_WORDLEFTEND);		
		NEWSLOT("WordLeftEndExtend",		(int)wxSTC_CMD_WORDLEFTENDEXTEND);			
		NEWSLOT("WordLeftExtend",			(int)wxSTC_CMD_WORDLEFTEXTEND);			
		NEWSLOT("WordPartLeft",				(int)wxSTC_CMD_WORDPARTLEFT);		
		NEWSLOT("WordPartLeftExtend",		(int)wxSTC_CMD_WORDPARTLEFTEXTEND);				
		NEWSLOT("WordPartRightExtend",		(int)wxSTC_CMD_WORDPARTRIGHTEXTEND);				
		NEWSLOT("WordRight",				(int)wxSTC_CMD_WORDRIGHT);	
		NEWSLOT("WordRightEnd",				(int)wxSTC_CMD_WORDRIGHTEND);		
		NEWSLOT("WordRightEndExtend",		(int)wxSTC_CMD_WORDRIGHTENDEXTEND);				
		NEWSLOT("WordRightExtend",			(int)wxSTC_CMD_WORDRIGHTEXTEND);			
		NEWSLOT("ZoomIn",					(int)wxSTC_CMD_ZOOMIN);	
		NEWSLOT("ZoomOut",					(int)wxSTC_CMD_ZOOMOUT);	
		sq_poptop(v);

		addStaticTable(v, "MARKER");
		NEWSLOT("CIRCLE",					(int)wxSTC_MARK_CIRCLE);
		NEWSLOT("ROUNDRECT",				(int)wxSTC_MARK_ROUNDRECT);
		NEWSLOT("ARROW",					(int)wxSTC_MARK_ARROW);
		NEWSLOT("SMALLRECT",				(int)wxSTC_MARK_SMALLRECT);
		NEWSLOT("SHORTARROW",				(int)wxSTC_MARK_SHORTARROW);
		NEWSLOT("EMPTY",					(int)wxSTC_MARK_EMPTY);
		NEWSLOT("ARROWDOWN",				(int)wxSTC_MARK_ARROWDOWN);
		NEWSLOT("MINUS",					(int)wxSTC_MARK_MINUS);
		NEWSLOT("PLUS",						(int)wxSTC_MARK_PLUS);
		NEWSLOT("VLINE",					(int)wxSTC_MARK_VLINE);
		NEWSLOT("LCORNER",					(int)wxSTC_MARK_LCORNER);
		NEWSLOT("TCORNER",					(int)wxSTC_MARK_TCORNER);
		NEWSLOT("BOXPLUS",					(int)wxSTC_MARK_BOXPLUS);
		NEWSLOT("BOXPLUSCONNECTED",			(int)wxSTC_MARK_BOXPLUSCONNECTED);
		NEWSLOT("BOXMINUS",					(int)wxSTC_MARK_BOXMINUS);
		NEWSLOT("BOXMINUSCONNECTED",		(int)wxSTC_MARK_BOXMINUSCONNECTED);
		NEWSLOT("LCORNERCURVE",				(int)wxSTC_MARK_LCORNERCURVE);
		NEWSLOT("TCORNERCURVE",				(int)wxSTC_MARK_TCORNERCURVE);
		NEWSLOT("CIRCLEPLUS",				(int)wxSTC_MARK_CIRCLEPLUS);
		NEWSLOT("CIRCLEPLUSCONNECTED",		(int)wxSTC_MARK_CIRCLEPLUSCONNECTED);
		NEWSLOT("CIRCLEMINUS",				(int)wxSTC_MARK_CIRCLEMINUS);
		NEWSLOT("CIRCLEMINUSCONNECTED",		(int)wxSTC_MARK_CIRCLEMINUSCONNECTED);
		NEWSLOT("BACKGROUND",				(int)wxSTC_MARK_BACKGROUND);
		NEWSLOT("DOTDOTDOT",				(int)wxSTC_MARK_DOTDOTDOT);
		NEWSLOT("ARROWS",					(int)wxSTC_MARK_ARROWS);
		NEWSLOT("PIXMAP",					(int)wxSTC_MARK_PIXMAP);
		NEWSLOT("FULLRECT",					(int)wxSTC_MARK_FULLRECT);
		NEWSLOT("LEFTRECT",					(int)wxSTC_MARK_LEFTRECT);
		NEWSLOT("AVAILABLE",				(int)wxSTC_MARK_AVAILABLE);
		NEWSLOT("UNDERLINE",				(int)wxSTC_MARK_UNDERLINE);
		NEWSLOT("CHARACTER",				(int)wxSTC_MARK_CHARACTER);
		sq_poptop(v);

		addStaticTable(v, "MARKNUM");
		NEWSLOT("FOLDEREND",				(int)wxSTC_MARKNUM_FOLDEREND);
		NEWSLOT("FOLDEROPENMID",			(int)wxSTC_MARKNUM_FOLDEROPENMID);
		NEWSLOT("FOLDERMIDTAIL",			(int)wxSTC_MARKNUM_FOLDERMIDTAIL);
		NEWSLOT("FOLDERTAIL",				(int)wxSTC_MARKNUM_FOLDERTAIL);
		NEWSLOT("FOLDERSUB",				(int)wxSTC_MARKNUM_FOLDERSUB);
		NEWSLOT("FOLDER",					(int)wxSTC_MARKNUM_FOLDER);
		NEWSLOT("FOLDEROPEN",				(int)wxSTC_MARKNUM_FOLDEROPEN);
		NEWSLOT("MASK_FOLDERS",				(int)wxSTC_MASK_FOLDERS);
		sq_poptop(v);

		addStaticTable(v, "MARGIN");
		NEWSLOT("SYMBOL",					(int)wxSTC_MARGIN_SYMBOL);
		NEWSLOT("NUMBER",					(int)wxSTC_MARGIN_NUMBER);
		NEWSLOT("BACK",						(int)wxSTC_MARGIN_BACK);
		NEWSLOT("FORE",						(int)wxSTC_MARGIN_FORE);
		NEWSLOT("TEXT",						(int)wxSTC_MARGIN_TEXT);
		NEWSLOT("RTEXT",					(int)wxSTC_MARGIN_RTEXT);
		sq_poptop(v);

		addStaticTable(v, "CASE");
		NEWSLOT("MIXED",					(int)wxSTC_CASE_MIXED);
		NEWSLOT("UPPER",					(int)wxSTC_CASE_UPPER);
		NEWSLOT("LOWER",					(int)wxSTC_CASE_LOWER);
		sq_poptop(v);

		addStaticTable(v, "INDIC");
		NEWSLOT("PLAIN",					(int)wxSTC_INDIC_PLAIN);
		NEWSLOT("SQUIGGLE",					(int)wxSTC_INDIC_SQUIGGLE);
		NEWSLOT("TT",						(int)wxSTC_INDIC_TT);
		NEWSLOT("DIAGONAL",					(int)wxSTC_INDIC_DIAGONAL);
		NEWSLOT("STRIKE",					(int)wxSTC_INDIC_STRIKE);
		NEWSLOT("HIDDEN",					(int)wxSTC_INDIC_HIDDEN);
		NEWSLOT("BOX",						(int)wxSTC_INDIC_BOX);
		NEWSLOT("ROUNDBOX",					(int)wxSTC_INDIC_ROUNDBOX);
		NEWSLOT("CONTAINER",				(int)wxSTC_INDIC_CONTAINER);
		sq_poptop(v);

		addStaticTable(v, "INDENT_GUIDE");
		NEWSLOT("NONE",						(int)wxSTC_IV_NONE);
		NEWSLOT("REAL",						(int)wxSTC_IV_REAL);
		NEWSLOT("LOOKFORWARD",				(int)wxSTC_IV_LOOKFORWARD);
		NEWSLOT("LOOKBOTH",					(int)wxSTC_IV_LOOKBOTH);
		sq_poptop(v);

		addStaticTable(v, "PRINT");
		NEWSLOT("NORMAL",					(int)wxSTC_PRINT_NORMAL);
		NEWSLOT("INVERTLIGHT",				(int)wxSTC_PRINT_INVERTLIGHT);
		NEWSLOT("BLACKONWHITE",				(int)wxSTC_PRINT_BLACKONWHITE);
		NEWSLOT("COLORONWHITE",				(int)wxSTC_PRINT_COLOURONWHITE);
		NEWSLOT("COLORONWHITEDEFAULTBG",	(int)wxSTC_PRINT_COLOURONWHITEDEFAULTBG);
		sq_poptop(v);

		addStaticTable(v, "FIND");
		NEWSLOT("WHOLEWORD",				(int)wxSTC_FIND_WHOLEWORD);
		NEWSLOT("MATCHCASE",				(int)wxSTC_FIND_MATCHCASE);
		NEWSLOT("WORDSTART",				(int)wxSTC_FIND_WORDSTART);
		NEWSLOT("REGEXP",					(int)wxSTC_FIND_REGEXP);
		NEWSLOT("POSIX",					(int)wxSTC_FIND_POSIX);
		sq_poptop(v);

		addStaticTable(v, "FOLDLEVEL");
		NEWSLOT("BASE",						(int)wxSTC_FOLDLEVELBASE);
		NEWSLOT("WHITEFLAG",				(int)wxSTC_FOLDLEVELWHITEFLAG);
		NEWSLOT("HEADERFLAG",				(int)wxSTC_FOLDLEVELHEADERFLAG);
		NEWSLOT("NUMBERMASK",				(int)wxSTC_FOLDLEVELNUMBERMASK);
		sq_poptop(v);

		addStaticTable(v, "FOLDFLAG");
		NEWSLOT("LINEBEFORE_EXPANDED",		(int)wxSTC_FOLDFLAG_LINEBEFORE_EXPANDED);
		NEWSLOT("LINEBEFORE_CONTRACTED",	(int)wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED);
		NEWSLOT("LINEAFTER_EXPANDED",		(int)wxSTC_FOLDFLAG_LINEAFTER_EXPANDED);
		NEWSLOT("LINEAFTER_CONTRACTED",		(int)wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);
		sq_poptop(v);

		addStaticTable(v, "WRAP");
		NEWSLOT("NONE",						(int)wxSTC_WRAP_NONE);
		NEWSLOT("WORD",						(int)wxSTC_WRAP_WORD);
		NEWSLOT("CHAR",						(int)wxSTC_WRAP_CHAR);
		sq_poptop(v);

		addStaticTable(v, "WRAPVISUALFLAG");
		NEWSLOT("NONE",						(int)wxSTC_WRAPVISUALFLAG_NONE);
		NEWSLOT("END",						(int)wxSTC_WRAPVISUALFLAG_END);
		NEWSLOT("START",					(int)wxSTC_WRAPVISUALFLAG_START);
		sq_poptop(v);

		addStaticTable(v, "WRAPVISUALFLAGLOC");
		NEWSLOT("DEFAULT",					(int)wxSTC_WRAPVISUALFLAGLOC_DEFAULT);
		NEWSLOT("END_BY_TEXT",				(int)wxSTC_WRAPVISUALFLAGLOC_END_BY_TEXT);
		NEWSLOT("START_BY_TEXT",			(int)wxSTC_WRAPVISUALFLAGLOC_START_BY_TEXT);
		sq_poptop(v);

		addStaticTable(v, "WRAPINDENT");
		NEWSLOT("FIXED",					(int)wxSTC_WRAPINDENT_FIXED);
		NEWSLOT("SAME",						(int)wxSTC_WRAPINDENT_SAME);
		NEWSLOT("INDENT",					(int)wxSTC_WRAPINDENT_INDENT);
		sq_poptop(v);

		addStaticTable(v, "CACHE");
		NEWSLOT("NONE",						(int)wxSTC_CACHE_NONE);
		NEWSLOT("CARET",					(int)wxSTC_CACHE_CARET);
		NEWSLOT("PAGE",						(int)wxSTC_CACHE_PAGE);
		NEWSLOT("DOCUMENT",					(int)wxSTC_CACHE_DOCUMENT);
		sq_poptop(v);

		addStaticTable(v, "EFF_QUALITY");
		NEWSLOT("MASK",						(int)wxSTC_EFF_QUALITY_MASK);
		NEWSLOT("DEFAULT",					(int)wxSTC_EFF_QUALITY_DEFAULT);
		NEWSLOT("NON_ANTIALIASED",			(int)wxSTC_EFF_QUALITY_NON_ANTIALIASED);
		NEWSLOT("ANTIALIASED",				(int)wxSTC_EFF_QUALITY_ANTIALIASED);
		NEWSLOT("LCD_OPTIMIZED",			(int)wxSTC_EFF_QUALITY_LCD_OPTIMIZED);
		sq_poptop(v);

		addStaticTable(v, "EDGE");
		NEWSLOT("NONE",						(int)wxSTC_EDGE_NONE);
		NEWSLOT("LINE",						(int)wxSTC_EDGE_LINE);
		NEWSLOT("BACKGROUND",				(int)wxSTC_EDGE_BACKGROUND);
		sq_poptop(v);

		addStaticTable(v, "STATUS");
		NEWSLOT("OK",						(int)wxSTC_STATUS_OK);
		NEWSLOT("FAILURE",					(int)wxSTC_STATUS_FAILURE);
		NEWSLOT("BADALLOC",					(int)wxSTC_STATUS_BADALLOC);
		sq_poptop(v);

		addStaticTable(v, "CURSOR");
		NEWSLOT("NORMAL",					(int)wxSTC_CURSORNORMAL);
		NEWSLOT("WAIT",						(int)wxSTC_CURSORWAIT);
		sq_poptop(v);

		addStaticTable(v, "VISIBLE");
		NEWSLOT("SLOP",						(int)wxSTC_VISIBLE_SLOP);
		NEWSLOT("STRICT",					(int)wxSTC_VISIBLE_STRICT);
		sq_poptop(v);

		addStaticTable(v, "CARET_POLICY");
		NEWSLOT("SLOP",						(int)wxSTC_CARET_SLOP);
		NEWSLOT("STRICT",					(int)wxSTC_CARET_STRICT);
		NEWSLOT("JUMPS",					(int)wxSTC_CARET_JUMPS);
		NEWSLOT("EVEN",						(int)wxSTC_CARET_EVEN);
		sq_poptop(v);

		addStaticTable(v, "SEL");
		NEWSLOT("STREAM",					(int)wxSTC_SEL_STREAM);
		NEWSLOT("RECTANGLE",				(int)wxSTC_SEL_RECTANGLE);
		NEWSLOT("LINES",					(int)wxSTC_SEL_LINES);
		NEWSLOT("THIN",						(int)wxSTC_SEL_THIN);
		sq_poptop(v);

		addStaticTable(v, "ALPHA");
		NEWSLOT("TRANSPARENT",				(int)wxSTC_ALPHA_TRANSPARENT);
		NEWSLOT("OPAQUE",					(int)wxSTC_ALPHA_OPAQUE);
		NEWSLOT("NOALPHA",					(int)wxSTC_ALPHA_NOALPHA);
		sq_poptop(v);

		addStaticTable(v, "CARET_STYLE");
		NEWSLOT("INVISIBLE",				(int)wxSTC_CARETSTYLE_INVISIBLE);
		NEWSLOT("LINE",						(int)wxSTC_CARETSTYLE_LINE);
		NEWSLOT("BLOCK",					(int)wxSTC_CARETSTYLE_BLOCK);
		sq_poptop(v);

		addStaticTable(v, "ANNOTATION");
		NEWSLOT("HIDDEN",					(int)wxSTC_ANNOTATION_HIDDEN);
		NEWSLOT("STANDARD",					(int)wxSTC_ANNOTATION_STANDARD);
		NEWSLOT("BOXED",					(int)wxSTC_ANNOTATION_BOXED);
		sq_poptop(v);

		addStaticTable(v, "EVT_MASK");
		NEWSLOT("INSERT_TEXT",				(int)wxSTC_MOD_INSERTTEXT);
		NEWSLOT("DELETE_TEXT",				(int)wxSTC_MOD_DELETETEXT);
		NEWSLOT("CHANGE_STYLE",				(int)wxSTC_MOD_CHANGESTYLE);
		NEWSLOT("CHANGE_FOLD",				(int)wxSTC_MOD_CHANGEFOLD);
		NEWSLOT("PERFORMED_USER",			(int)wxSTC_PERFORMED_USER);
		NEWSLOT("PERFORMED_UNDO",			(int)wxSTC_PERFORMED_UNDO);
		NEWSLOT("MULTISTEP_UNDO_REDO",		(int)wxSTC_MULTISTEPUNDOREDO);
		NEWSLOT("LASTSTEP_IN_UNDO_REDO",	(int)wxSTC_LASTSTEPINUNDOREDO);
		NEWSLOT("CHANGEMARKER",				(int)wxSTC_MOD_CHANGEMARKER);
		NEWSLOT("BEFORE_INSERT",			(int)wxSTC_MOD_BEFOREINSERT);
		NEWSLOT("BEFORE_DELETE",			(int)wxSTC_MOD_BEFOREDELETE);
		NEWSLOT("MULTILINE_UNDO_REDO",		(int)wxSTC_MULTILINEUNDOREDO);
		NEWSLOT("START_ACTION",				(int)wxSTC_STARTACTION);
		NEWSLOT("CHANGE_INDICATOR",			(int)wxSTC_MOD_CHANGEINDICATOR);
		NEWSLOT("CHANGE_LINESTATE",			(int)wxSTC_MOD_CHANGELINESTATE);
		NEWSLOT("CHANGE_MARGIN",			(int)wxSTC_MOD_CHANGEMARGIN);
		NEWSLOT("CHANGE_ANNOTATION",		(int)wxSTC_MOD_CHANGEANNOTATION);
		NEWSLOT("CONTAINER",				(int)wxSTC_MOD_CONTAINER);
		NEWSLOT("ALL",						(int)wxSTC_MODEVENTMASKALL);
		sq_poptop(v);

		addStaticTable(v, "KEY");
		NEWSLOT("DOWN",						(int)wxSTC_KEY_DOWN);
		NEWSLOT("UP",						(int)wxSTC_KEY_UP);
		NEWSLOT("LEFT",						(int)wxSTC_KEY_LEFT);
		NEWSLOT("RIGHT",					(int)wxSTC_KEY_RIGHT);
		NEWSLOT("HOME",						(int)wxSTC_KEY_HOME);
		NEWSLOT("END",						(int)wxSTC_KEY_END);
		NEWSLOT("PRIOR",					(int)wxSTC_KEY_PRIOR);
		NEWSLOT("NEXT",						(int)wxSTC_KEY_NEXT);
		NEWSLOT("DELETE",					(int)wxSTC_KEY_DELETE);
		NEWSLOT("INSERT",					(int)wxSTC_KEY_INSERT);
		NEWSLOT("ESCAPE",					(int)wxSTC_KEY_ESCAPE);
		NEWSLOT("BACK",						(int)wxSTC_KEY_BACK);
		NEWSLOT("TAB",						(int)wxSTC_KEY_TAB);
		NEWSLOT("RETURN",					(int)wxSTC_KEY_RETURN);
		NEWSLOT("ADD",						(int)wxSTC_KEY_ADD);
		NEWSLOT("SUBTRACT",					(int)wxSTC_KEY_SUBTRACT);
		NEWSLOT("DIVIDE",					(int)wxSTC_KEY_DIVIDE);
		NEWSLOT("WIN",						(int)wxSTC_KEY_WIN);
		NEWSLOT("RWIN",						(int)wxSTC_KEY_RWIN);
		NEWSLOT("MENU",						(int)wxSTC_KEY_MENU);
		sq_poptop(v);

		addStaticTable(v, "SCMOD");
		NEWSLOT("NORM",						(int)wxSTC_SCMOD_NORM);
		NEWSLOT("SHIFT",					(int)wxSTC_SCMOD_SHIFT);
		NEWSLOT("CTRL",						(int)wxSTC_SCMOD_CTRL);
		NEWSLOT("ALT",						(int)wxSTC_SCMOD_ALT);
		NEWSLOT("SUPER",					(int)wxSTC_SCMOD_SUPER);
		sq_poptop(v);

		addStaticTable(v, "CP");
		NEWSLOT("UTF8",						(int)wxSTC_CP_UTF8);
		sq_poptop(v);

		addStaticTable(v, "EOL");
		NEWSLOT("CRLF",						(int)wxSTC_EOL_CRLF);
		NEWSLOT("CR",						(int)wxSTC_EOL_CR);
		NEWSLOT("LF",						(int)wxSTC_EOL_LF);
		sq_poptop(v);

		addStaticTable(v, "WHITESPACE");
		NEWSLOT("INVISIBLE",				(int)wxSTC_WS_INVISIBLE);
		NEWSLOT("VISIBLEALWAYS",			(int)wxSTC_WS_VISIBLEALWAYS);
		NEWSLOT("VISIBLEAFTERINDENT",		(int)wxSTC_WS_VISIBLEAFTERINDENT);
		sq_poptop(v);

		addStaticTable(v, "LEX");
		NEWSLOT("CONTAINER",				(int)wxSTC_LEX_CONTAINER);
		NEWSLOT("NULL",						(int)wxSTC_LEX_NULL);
		NEWSLOT("CPP",						(int)wxSTC_LEX_CPP);
		NEWSLOT("HTML",						(int)wxSTC_LEX_HTML);
		NEWSLOT("XML",						(int)wxSTC_LEX_XML);
		NEWSLOT("SQL",						(int)wxSTC_LEX_SQL);
		NEWSLOT("CONF",						(int)wxSTC_LEX_CONF);
		NEWSLOT("CSS",						(int)wxSTC_LEX_CSS);
		NEWSLOT("AUTOMATIC",				(int)wxSTC_LEX_AUTOMATIC);
		sq_poptop(v);

		addStaticTable(v, "STYLE_TEXT");
		NEWSLOT("DEFAULT",					(int)wxSTC_STYLE_DEFAULT);
		NEWSLOT("LINENUMBER",				(int)wxSTC_STYLE_LINENUMBER);
		NEWSLOT("BRACELIGHT",				(int)wxSTC_STYLE_BRACELIGHT);
		NEWSLOT("BRACEBAD",					(int)wxSTC_STYLE_BRACEBAD);
		NEWSLOT("CONTROLCHAR",				(int)wxSTC_STYLE_CONTROLCHAR);
		NEWSLOT("INDENTGUIDE",				(int)wxSTC_STYLE_INDENTGUIDE);
		NEWSLOT("CALLTIP",					(int)wxSTC_STYLE_CALLTIP);
		NEWSLOT("LASTPREDEFINED",			(int)wxSTC_STYLE_LASTPREDEFINED);
		sq_poptop(v);

		addStaticTable(v, "STYLE_CPP");
		NEWSLOT("DEFAULT",					(int)wxSTC_C_DEFAULT);
		NEWSLOT("COMMENT",					(int)wxSTC_C_COMMENT);
		NEWSLOT("COMMENTLINE",				(int)wxSTC_C_COMMENTLINE);
		NEWSLOT("COMMENTDOC",				(int)wxSTC_C_COMMENTDOC);
		NEWSLOT("NUMBER",					(int)wxSTC_C_NUMBER);
		NEWSLOT("WORD",						(int)wxSTC_C_WORD);
		NEWSLOT("STRING",					(int)wxSTC_C_STRING);
		NEWSLOT("CHARACTER",				(int)wxSTC_C_CHARACTER);
		NEWSLOT("UUID",						(int)wxSTC_C_UUID);
		NEWSLOT("PREPROCESSOR",				(int)wxSTC_C_PREPROCESSOR);
		NEWSLOT("OPERATOR",					(int)wxSTC_C_OPERATOR);
		NEWSLOT("IDENTIFIER",				(int)wxSTC_C_IDENTIFIER);
		NEWSLOT("STRINGEOL",				(int)wxSTC_C_STRINGEOL);
		NEWSLOT("VERBATIM",					(int)wxSTC_C_VERBATIM);
		NEWSLOT("REGEX",					(int)wxSTC_C_REGEX);
		NEWSLOT("COMMENTLINEDOC",			(int)wxSTC_C_COMMENTLINEDOC);
		NEWSLOT("WORD2",					(int)wxSTC_C_WORD2);
		NEWSLOT("COMMENTDOCKEYWORD",		(int)wxSTC_C_COMMENTDOCKEYWORD);
		NEWSLOT("COMMENTDOCKEYWORDERROR",	(int)wxSTC_C_COMMENTDOCKEYWORDERROR);
		NEWSLOT("GLOBALCLASS",				(int)wxSTC_C_GLOBALCLASS);
		sq_poptop(v);

		addStaticTable(v, "STYLE_XML");
		NEWSLOT("DEFAULT",					(int)wxSTC_H_DEFAULT);
		NEWSLOT("TAG",						(int)wxSTC_H_TAG);
		NEWSLOT("TAGUNKNOWN",				(int)wxSTC_H_TAGUNKNOWN);
		NEWSLOT("ATTRIBUTE",				(int)wxSTC_H_ATTRIBUTE);
		NEWSLOT("ATTRIBUTEUNKNOWN",			(int)wxSTC_H_ATTRIBUTEUNKNOWN);
		NEWSLOT("NUMBER",					(int)wxSTC_H_NUMBER);
		NEWSLOT("DOUBLESTRING",				(int)wxSTC_H_DOUBLESTRING);
		NEWSLOT("SINGLESTRING",				(int)wxSTC_H_SINGLESTRING);
		NEWSLOT("OTHER",					(int)wxSTC_H_OTHER);
		NEWSLOT("COMMENT",					(int)wxSTC_H_COMMENT);
		NEWSLOT("ENTITY",					(int)wxSTC_H_ENTITY);
		NEWSLOT("TAGEND",					(int)wxSTC_H_TAGEND);
		NEWSLOT("XMLSTART",					(int)wxSTC_H_XMLSTART);
		NEWSLOT("XMLEND",					(int)wxSTC_H_XMLEND);
		NEWSLOT("SCRIPT",					(int)wxSTC_H_SCRIPT);
		NEWSLOT("ASP",						(int)wxSTC_H_ASP);
		NEWSLOT("ASPAT",					(int)wxSTC_H_ASPAT);
		NEWSLOT("CDATA",					(int)wxSTC_H_CDATA);
		NEWSLOT("QUESTION",					(int)wxSTC_H_QUESTION);
		NEWSLOT("VALUE",					(int)wxSTC_H_VALUE);
		sq_poptop(v);

		addStaticTable(v, "STYLE_JS");
		NEWSLOT("START",					(int)wxSTC_HJ_START);
		NEWSLOT("DEFAULT",					(int)wxSTC_HJ_DEFAULT);
		NEWSLOT("COMMENT",					(int)wxSTC_HJ_COMMENT);
		NEWSLOT("COMMENTLINE",				(int)wxSTC_HJ_COMMENTLINE);
		NEWSLOT("COMMENTDOC",				(int)wxSTC_HJ_COMMENTDOC);
		NEWSLOT("NUMBER",					(int)wxSTC_HJ_NUMBER);
		NEWSLOT("WORD",						(int)wxSTC_HJ_WORD);
		NEWSLOT("KEYWORD",					(int)wxSTC_HJ_KEYWORD);
		NEWSLOT("DOUBLESTRING",				(int)wxSTC_HJ_DOUBLESTRING);
		NEWSLOT("SINGLESTRING",				(int)wxSTC_HJ_SINGLESTRING);
		NEWSLOT("SYMBOLS",					(int)wxSTC_HJ_SYMBOLS);
		NEWSLOT("STRINGEOL",				(int)wxSTC_HJ_STRINGEOL);
		NEWSLOT("REGEX",					(int)wxSTC_HJ_REGEX);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		NEWSLOT("CHANGE",					(int)wxEVT_STC_CHANGE);
		NEWSLOT("STYLENEEDED",				(int)wxEVT_STC_STYLENEEDED);
		NEWSLOT("CHARADDED",				(int)wxEVT_STC_CHARADDED);
		NEWSLOT("SAVEPOINTREACHED",			(int)wxEVT_STC_SAVEPOINTREACHED);
		NEWSLOT("SAVEPOINTLEFT",			(int)wxEVT_STC_SAVEPOINTLEFT);
		NEWSLOT("ROMODIFYATTEMPT",			(int)wxEVT_STC_ROMODIFYATTEMPT);
		NEWSLOT("KEY",						(int)wxEVT_STC_KEY);
		NEWSLOT("DOUBLECLICK",				(int)wxEVT_STC_DOUBLECLICK);
		NEWSLOT("UPDATEUI",					(int)wxEVT_STC_UPDATEUI);
		NEWSLOT("MODIFIED",					(int)wxEVT_STC_MODIFIED);
		NEWSLOT("MACRORECORD",				(int)wxEVT_STC_MACRORECORD);
		NEWSLOT("MARGINCLICK",				(int)wxEVT_STC_MARGINCLICK);
		NEWSLOT("NEEDSHOWN",				(int)wxEVT_STC_NEEDSHOWN);
		NEWSLOT("PAINTED",					(int)wxEVT_STC_PAINTED);
		NEWSLOT("USERLISTSELECTION",		(int)wxEVT_STC_USERLISTSELECTION);
		NEWSLOT("URIDROPPED",				(int)wxEVT_STC_URIDROPPED);
		NEWSLOT("DWELLSTART",				(int)wxEVT_STC_DWELLSTART);
		NEWSLOT("DWELLEND",					(int)wxEVT_STC_DWELLEND);
		NEWSLOT("START_DRAG",				(int)wxEVT_STC_START_DRAG);
		NEWSLOT("DRAG_OVER",				(int)wxEVT_STC_DRAG_OVER);
		NEWSLOT("DO_DROP",					(int)wxEVT_STC_DO_DROP);
		NEWSLOT("ZOOM",						(int)wxEVT_STC_ZOOM);
		NEWSLOT("HOTSPOT_CLICK",			(int)wxEVT_STC_HOTSPOT_CLICK);
		NEWSLOT("HOTSPOT_DCLICK",			(int)wxEVT_STC_HOTSPOT_DCLICK);
		NEWSLOT("CALLTIP_CLICK",			(int)wxEVT_STC_CALLTIP_CLICK);
		NEWSLOT("AUTOCOMP_SELECTION",		(int)wxEVT_STC_AUTOCOMP_SELECTION);
		NEWSLOT("INDICATOR_CLICK",			(int)wxEVT_STC_INDICATOR_CLICK);
		NEWSLOT("INDICATOR_RELEASE",		(int)wxEVT_STC_INDICATOR_RELEASE);
		NEWSLOT("AUTOCOMP_CANCELLED",		(int)wxEVT_STC_AUTOCOMP_CANCELLED);
		NEWSLOT("AUTOCOMP_CHAR_DELETED",	(int)wxEVT_STC_AUTOCOMP_CHAR_DELETED);
		sq_poptop(v);
	}

	static void* GetUserPointer(HSQUIRRELVM v, int idx)
	{
		void* up = NULL;

		if (sq_gettype(v, 2) != OT_USERPOINTER)
			sqx_throw(v, "invalid pointer");
		else
			sq_getuserpointer(v, idx, &up);
		return up;
	}

	NB_PROP_GET(autoCompActive)			{ return push(v, self(v)->AutoCompActive()); }
	NB_PROP_GET(autoCompAutoHide)		{ return push(v, self(v)->AutoCompGetAutoHide()); }
	NB_PROP_GET(autoCompCancelAtStart)	{ return push(v, self(v)->AutoCompGetCancelAtStart()); }
	NB_PROP_GET(autoCompChooseSingle)	{ return push(v, self(v)->AutoCompGetChooseSingle()); }
	NB_PROP_GET(autoCompCurrent)		{ return push(v, self(v)->AutoCompGetCurrent()); }
	NB_PROP_GET(autoCompDropRestOfWord){ return push(v, self(v)->AutoCompGetDropRestOfWord()); }
	NB_PROP_GET(autoCompIgnoreCase)		{ return push(v, self(v)->AutoCompGetIgnoreCase()); }
	NB_PROP_GET(autoCompMaxHeight)		{ return push(v, self(v)->AutoCompGetMaxHeight()); }
	NB_PROP_GET(autoCompMaxWidth)		{ return push(v, self(v)->AutoCompGetMaxWidth()); }
	NB_PROP_GET(autoCompSeparator)		{ return push(v, self(v)->AutoCompGetSeparator()); }
	NB_PROP_GET(autoCompTypeSeparator)	{ return push(v, self(v)->AutoCompGetTypeSeparator()); }
	NB_PROP_GET(autoCompPosStart)		{ return push(v, self(v)->AutoCompPosStart()); }
	NB_PROP_GET(callTipActive)			{ return push(v, self(v)->CallTipActive()); }
	NB_PROP_GET(callTipPosAtStart)		{ return push(v, self(v)->CallTipPosAtStart()); }
	NB_PROP_GET(canPaste)				{ return push(v, self(v)->CanPaste()); }
	NB_PROP_GET(canRedo)				{ return push(v, self(v)->CanRedo()); }
	NB_PROP_GET(canUndo)				{ return push(v, self(v)->CanUndo()); }
	NB_PROP_GET(anchor)					{ return push(v, self(v)->GetAnchor()); }
	NB_PROP_GET(backSpaceUnIndents)		{ return push(v, self(v)->GetBackSpaceUnIndents()); }
	NB_PROP_GET(bufferedDraw)			{ return push(v, self(v)->GetBufferedDraw()); }
	NB_PROP_GET(caretForeColor)			{ return PushWxColor(v, self(v)->GetCaretForeground()); }
	NB_PROP_GET(caretLineBackAlpha)		{ return push(v, self(v)->GetCaretLineBackAlpha()); }
	NB_PROP_GET(caretLineBackColor)		{ return PushWxColor(v, self(v)->GetCaretLineBackground()); }
	NB_PROP_GET(caretLineVisible)		{ return push(v, self(v)->GetCaretLineVisible()); }
	NB_PROP_GET(caretPeriod)			{ return push(v, self(v)->GetCaretPeriod()); }
	NB_PROP_GET(caretSticky)			{ return push(v, self(v)->GetCaretSticky()); }
	NB_PROP_GET(caretWidth)				{ return push(v, self(v)->GetCaretWidth()); }
	NB_PROP_GET(codePage)				{ return push(v, self(v)->GetCodePage()); }
	NB_PROP_GET(lineText)				{ return push(v, self(v)->GetCurLine()); }
	NB_PROP_GET(currentLine)			{ return push(v, self(v)->GetCurrentLine()); }
	NB_PROP_GET(currentPos)				{ return push(v, self(v)->GetCurrentPos()); }
	NB_PROP_GET(docPointer)				{ sq_pushuserpointer(v, self(v)->GetDocPointer()); return 1; }
	NB_PROP_GET(eolMode)				{ return push(v, self(v)->GetEOLMode()); }
	NB_PROP_GET(edgeColor)				{ return PushWxColor(v, self(v)->GetEdgeColour()); }
	NB_PROP_GET(edgeColumn)				{ return push(v, self(v)->GetEdgeColumn()); }
	NB_PROP_GET(edgeMode)				{ return push(v, self(v)->GetEdgeMode()); }
	NB_PROP_GET(endAtLastLine)			{ return push(v, self(v)->GetEndAtLastLine()); }
	NB_PROP_GET(endStyled)				{ return push(v, self(v)->GetEndStyled()); }
	NB_PROP_GET(firstVisibleLine)		{ return push(v, self(v)->GetFirstVisibleLine()); }
	NB_PROP_GET(highlightGuide)			{ return push(v, self(v)->GetHighlightGuide()); }
	NB_PROP_GET(keysUnicode)			{ return push(v, self(v)->GetKeysUnicode()); }
	NB_PROP_GET(extraAscent)			{ return push(v, self(v)->GetExtraAscent()); }
	NB_PROP_GET(extraDescent)			{ return push(v, self(v)->GetExtraDescent()); }
	NB_PROP_GET(indent)					{ return push(v, self(v)->GetIndent()); }
	NB_PROP_GET(indentationGuides)		{ return push(v, self(v)->GetIndentationGuides()); }
	NB_PROP_GET(lastKeydownProcessed)	{ return push(v, self(v)->GetLastKeydownProcessed()); }
	NB_PROP_GET(layoutCache)			{ return push(v, self(v)->GetLayoutCache()); }
	NB_PROP_GET(length)					{ return push(v, self(v)->GetLength()); }
	NB_PROP_GET(lexer)					{ return push(v, self(v)->GetLexer()); }
	NB_PROP_GET(lineCount)				{ return push(v, self(v)->GetLineCount()); }
	NB_PROP_GET(marginLeft)				{ return push(v, self(v)->GetMarginLeft()); }
	NB_PROP_GET(marginRight)			{ return push(v, self(v)->GetMarginRight()); }
	NB_PROP_GET(maxLineState)			{ return push(v, self(v)->GetMaxLineState()); }
	NB_PROP_GET(modEventMask)			{ return push(v, self(v)->GetModEventMask()); }
	NB_PROP_GET(modified)				{ return push(v, self(v)->GetModify()); }
	NB_PROP_GET(mouseDownCaptures)		{ return push(v, self(v)->GetMouseDownCaptures()); }
	NB_PROP_GET(mouseDwellTime)			{ return push(v, self(v)->GetMouseDwellTime()); }
	NB_PROP_GET(overtype)				{ return push(v, self(v)->GetOvertype()); }
	NB_PROP_GET(pasteConvertEndings)	{ return push(v, self(v)->GetPasteConvertEndings()); }
	NB_PROP_GET(printColorMode)			{ return push(v, self(v)->GetPrintColourMode()); }
	NB_PROP_GET(printMagnification)		{ return push(v, self(v)->GetPrintMagnification()); }
	NB_PROP_GET(printWrapMode)			{ return push(v, self(v)->GetPrintWrapMode()); }
	NB_PROP_GET(readOnly)				{ return push(v, self(v)->GetReadOnly()); }
	NB_PROP_GET(stcCursor)				{ return push(v, self(v)->GetSTCCursor()); }
	NB_PROP_GET(stcFocus)				{ return push(v, self(v)->GetSTCFocus()); }
	NB_PROP_GET(scrollWidth)			{ return push(v, self(v)->GetScrollWidth()); }
	NB_PROP_GET(searchFlags)			{ return push(v, self(v)->GetSearchFlags()); }
	NB_PROP_GET(selAlpha)				{ return push(v, self(v)->GetSelAlpha()); }
	NB_PROP_GET(selectedText)			{ return push(v, self(v)->GetSelectedText()); }
	NB_PROP_GET(selectionEnd)			{ return push(v, self(v)->GetSelectionEnd()); }
	NB_PROP_GET(selectionMode)			{ return push(v, self(v)->GetSelectionMode()); }
	NB_PROP_GET(selectionStart)			{ return push(v, self(v)->GetSelectionStart()); }
	NB_PROP_GET(status)					{ return push(v, self(v)->GetStatus()); }
	NB_PROP_GET(styleBits)				{ return push(v, self(v)->GetStyleBits()); }
	NB_PROP_GET(styleBitsNeeded)		{ return push(v, self(v)->GetStyleBitsNeeded()); }
	NB_PROP_GET(tabIndents)				{ return push(v, self(v)->GetTabIndents()); }
	NB_PROP_GET(tabWidth)				{ return push(v, self(v)->GetTabWidth()); }
	NB_PROP_GET(targetEnd)				{ return push(v, self(v)->GetTargetEnd()); }
	NB_PROP_GET(targetStart)			{ return push(v, self(v)->GetTargetStart()); }
	NB_PROP_GET(textLength)				{ return push(v, self(v)->GetTextLength()); }
	NB_PROP_GET(twoPhaseDraw)			{ return push(v, self(v)->GetTwoPhaseDraw()); }
	NB_PROP_GET(undoCollection)			{ return push(v, self(v)->GetUndoCollection()); }
	NB_PROP_GET(useAntiAliasing)		{ return push(v, self(v)->GetUseAntiAliasing()); }
	NB_PROP_GET(useHorizontalScrollBar){ return push(v, self(v)->GetUseHorizontalScrollBar()); }
	NB_PROP_GET(useTabs)				{ return push(v, self(v)->GetUseTabs()); }
	NB_PROP_GET(useVerticalScrollBar)	{ return push(v, self(v)->GetUseVerticalScrollBar()); }
	NB_PROP_GET(viewEol)				{ return push(v, self(v)->GetViewEOL()); }
	NB_PROP_GET(viewWhiteSpace)			{ return push(v, self(v)->GetViewWhiteSpace()); }
	NB_PROP_GET(wrapIndentMode)			{ return push(v, self(v)->GetWrapIndentMode()); }
	NB_PROP_GET(wrapMode)				{ return push(v, self(v)->GetWrapMode()); }
	NB_PROP_GET(wrapStartIndent)		{ return push(v, self(v)->GetWrapStartIndent()); }
	NB_PROP_GET(wrapVisualFlags)		{ return push(v, self(v)->GetWrapVisualFlags()); }
	NB_PROP_GET(wrapVisualFlagsLocation) { return push(v, self(v)->GetWrapVisualFlagsLocation()); }
	NB_PROP_GET(xoffset)				{ return push(v, self(v)->GetXOffset()); }
	NB_PROP_GET(zoom)					{ return push(v, self(v)->GetZoom()); }
	NB_PROP_GET(linesOnScreen)			{ return push(v, self(v)->LinesOnScreen()); }
	NB_PROP_GET(inRectSel)				{ return push(v, self(v)->SelectionIsRectangle()); }
	NB_PROP_GET(whitespaceSize)			{ return push(v, self(v)->GetWhitespaceSize()); }
	NB_PROP_GET(annotationVisible)		{ return push(v, self(v)->AnnotationGetVisible()); }
	NB_PROP_GET(annotationStyleOffset)	{ return push(v, self(v)->AnnotationGetStyleOffset()); }
	NB_PROP_GET(rectSelCaret)			{ return push(v, self(v)->GetRectangularSelectionCaret()); }
	NB_PROP_GET(rectSelAnchor)			{ return push(v, self(v)->GetRectangularSelectionAnchor()); }
	NB_PROP_GET(rectSelCaretVirtualSpace) { return push(v, self(v)->GetRectangularSelectionCaretVirtualSpace()); }
	NB_PROP_GET(rectSelAnchorVirtualSpace) { return push(v, self(v)->GetRectangularSelectionAnchorVirtualSpace()); }
	NB_PROP_GET(virtualSpaceOptions)	{ return push(v, self(v)->GetVirtualSpaceOptions()); }
	NB_PROP_GET(rectSelModifier)		{ return push(v, self(v)->GetRectangularSelectionModifier()); }

	NB_PROP_SET(autoCompAutoHide)		{ self(v)->AutoCompSetAutoHide(getBool(v, 2)); return 0; }
	NB_PROP_SET(autoCompCancelAtStart)	{ self(v)->AutoCompSetCancelAtStart(getBool(v, 2)); return 0; }
	NB_PROP_SET(autoCompChooseSingle)	{ self(v)->AutoCompSetChooseSingle(getBool(v, 2)); return 0; }
	NB_PROP_SET(autoCompDropRestOfWord)	{ self(v)->AutoCompSetDropRestOfWord(getBool(v, 2)); return 0; }
	NB_PROP_SET(autoCompIgnoreCase)		{ self(v)->AutoCompSetIgnoreCase(getBool(v, 2)); return 0; }
	NB_PROP_SET(autoCompMaxHeight)		{ self(v)->AutoCompSetMaxHeight(getInt(v, 2)); return 0; }
	NB_PROP_SET(autoCompMaxWidth)		{ self(v)->AutoCompSetMaxWidth(getInt(v, 2)); return 0; }
	NB_PROP_SET(autoCompSeparator)		{ self(v)->AutoCompSetSeparator(getInt(v, 2)); return 0; }
	NB_PROP_SET(autoCompTypeSeparator)	{ self(v)->AutoCompSetTypeSeparator(getInt(v, 2)); return 0; }
	NB_PROP_SET(backSpaceUnIndents)		{ self(v)->SetBackSpaceUnIndents(getBool(v, 2)); return 0; }
	NB_PROP_SET(bufferedDraw)			{ self(v)->SetBufferedDraw(getBool(v, 2)); return 0; }
	NB_PROP_SET(caretForeColor)			{ self(v)->SetCaretForeground(GetWxColor(v, 2)); return 0; }
	NB_PROP_SET(caretLineBackAlpha)		{ self(v)->SetCaretLineBackAlpha(getInt(v, 2)); return 0; }
	NB_PROP_SET(caretLineBackColor)		{ self(v)->SetCaretLineBackground(GetWxColor(v, 2)); return 0; }
	NB_PROP_SET(caretLineVisible)		{ self(v)->SetCaretLineVisible(getBool(v, 2)); return 0; }
	NB_PROP_SET(caretPeriod)			{ self(v)->SetCaretPeriod(getInt(v, 2)); return 0; }
	NB_PROP_SET(caretSticky)			{ self(v)->SetCaretSticky(getBool(v, 2)); return 0; }
	NB_PROP_SET(caretWidth)				{ self(v)->SetCaretWidth(getInt(v, 2)); return 0; }
	NB_PROP_SET(currentPos)				{ self(v)->SetCurrentPos(getInt(v, 2)); return 0; }
	NB_PROP_SET(docPointer)				{ self(v)->SetDocPointer(GetUserPointer(v, 2)); return 0; }
	NB_PROP_SET(eolMode)				{ self(v)->SetEOLMode(getInt(v, 2)); return 0; }
	NB_PROP_SET(edgeColor)				{ self(v)->SetEdgeColour(GetWxColor(v, 2)); return 0; }
	NB_PROP_SET(edgeColumn)				{ self(v)->SetEdgeColumn(getInt(v, 2)); return 0; }
	NB_PROP_SET(edgeMode)				{ self(v)->SetEdgeMode(getInt(v, 2)); return 0; }
	NB_PROP_SET(endAtLastLine)			{ self(v)->SetEndAtLastLine(getBool(v, 2)); return 0; }
	NB_PROP_SET(firstVisibleLine)		{ self(v)->SetFirstVisibleLine(getInt(v, 2)); return 0; }
	NB_PROP_SET(highlightGuide)			{ self(v)->SetHighlightGuide(getInt(v, 2)); return 0; }
	NB_PROP_SET(keysUnicode)			{ self(v)->SetKeysUnicode(getBool(v, 2)); return 0; }
	NB_PROP_SET(extraAscent)			{ self(v)->SetExtraAscent(getInt(v, 2)); return 0; }
	NB_PROP_SET(extraDescent)			{ self(v)->SetExtraDescent(getInt(v, 2)); return 0; }
	NB_PROP_SET(indent)					{ self(v)->SetIndent(getInt(v, 2)); return 0; }
	NB_PROP_SET(indentationGuides)		{ self(v)->SetIndentationGuides(getInt(v, 2)); return 0; }
	NB_PROP_SET(lastKeydownProcessed)	{ self(v)->SetLastKeydownProcessed(getBool(v, 2)); return 0; }
	NB_PROP_SET(layoutCache)			{ self(v)->SetLayoutCache(getInt(v, 2)); return 0; }
	NB_PROP_SET(lexer)					{ self(v)->SetLexer(getInt(v, 2)); return 0; }
	NB_PROP_SET(marginLeft)				{ self(v)->SetMarginLeft(getInt(v, 2)); return 0; }
	NB_PROP_SET(marginRight)			{ self(v)->SetMarginRight(getInt(v, 2)); return 0; }
	NB_PROP_SET(modEventMask)			{ self(v)->SetModEventMask(getInt(v, 2)); return 0; }
	NB_PROP_SET(mouseDownCaptures)		{ self(v)->SetMouseDownCaptures(getBool(v, 2)); return 0; }
	NB_PROP_SET(mouseDwellTime)			{ self(v)->SetMouseDwellTime(getInt(v, 2)); return 0; }
	NB_PROP_SET(overtype)				{ self(v)->SetOvertype(getBool(v, 2)); return 0; }
	NB_PROP_SET(pasteConvertEndings)	{ self(v)->SetPasteConvertEndings(getBool(v, 2)); return 0; }
	NB_PROP_SET(printColorMode)			{ self(v)->SetPrintColourMode(getInt(v, 2)); return 0; }
	NB_PROP_SET(printMagnification)		{ self(v)->SetPrintMagnification(getInt(v, 2)); return 0; }
	NB_PROP_SET(printWrapMode)			{ self(v)->SetPrintWrapMode(getInt(v, 2)); return 0; }
	NB_PROP_SET(readOnly)				{ self(v)->SetReadOnly(getBool(v, 2)); return 0; }
	NB_PROP_SET(stcCursor)				{ self(v)->SetSTCCursor(getInt(v, 2)); return 0; }
	NB_PROP_SET(stcFocus)				{ self(v)->SetSTCFocus(getBool(v, 2)); return 0; }
	NB_PROP_SET(scrollWidth)			{ self(v)->SetScrollWidth(getInt(v, 2)); return 0; }
	NB_PROP_SET(searchFlags)			{ self(v)->SetSearchFlags(getInt(v, 2)); return 0; }
	NB_PROP_SET(selAlpha)				{ self(v)->SetSelAlpha(getInt(v, 2)); return 0; }
	NB_PROP_SET(selectionEnd)			{ self(v)->SetSelectionEnd(getInt(v, 2)); return 0; }
	NB_PROP_SET(selectionMode)			{ self(v)->SetSelectionMode(getInt(v, 2)); return 0; }
	NB_PROP_SET(selectionStart)			{ self(v)->SetSelectionStart(getInt(v, 2)); return 0; }
	NB_PROP_SET(status)					{ self(v)->SetStatus(getInt(v, 2)); return 0; }
	NB_PROP_SET(styleBits)				{ self(v)->SetStyleBits(getInt(v, 2)); return 0; }
	NB_PROP_SET(tabIndents)				{ self(v)->SetTabIndents(getBool(v, 2)); return 0; }
	NB_PROP_SET(tabWidth)				{ self(v)->SetTabWidth(getInt(v, 2)); return 0; }
	NB_PROP_SET(targetEnd)				{ self(v)->SetTargetEnd(getInt(v, 2)); return 0; }
	NB_PROP_SET(targetStart)			{ self(v)->SetTargetStart(getInt(v, 2)); return 0; }
	NB_PROP_SET(twoPhaseDraw)			{ self(v)->SetTwoPhaseDraw(getBool(v, 2)); return 0; }
	NB_PROP_SET(undoCollection)			{ self(v)->SetUndoCollection(getBool(v, 2)); return 0; }
	NB_PROP_SET(useAntiAliasing)		{ self(v)->SetUseAntiAliasing(getBool(v, 2)); return 0; }
	NB_PROP_SET(useHorizontalScrollBar)	{ self(v)->SetUseHorizontalScrollBar(getBool(v, 2)); return 0; }
	NB_PROP_SET(useTabs)				{ self(v)->SetUseTabs(getBool(v, 2)); return 0; }
	NB_PROP_SET(useVerticalScrollBar)	{ self(v)->SetUseVerticalScrollBar(getBool(v, 2)); return 0; }
	NB_PROP_SET(viewEol)				{ self(v)->SetViewEOL(getBool(v, 2)); return 0; }
	NB_PROP_SET(viewWhiteSpace)			{ self(v)->SetViewWhiteSpace(getInt(v, 2)); return 0; }
	NB_PROP_SET(wrapIndentMode)			{ self(v)->SetWrapIndentMode(getInt(v, 2)); return 0; }
	NB_PROP_SET(wrapMode)				{ self(v)->SetWrapMode(getInt(v, 2)); return 0; }
	NB_PROP_SET(wrapStartIndent)		{ self(v)->SetWrapStartIndent(getInt(v, 2)); return 0; }
	NB_PROP_SET(wrapVisualFlags)		{ self(v)->SetWrapVisualFlags(getInt(v, 2)); return 0; }
	NB_PROP_SET(wrapVisualFlagsLocation) { self(v)->SetWrapVisualFlags(getInt(v, 2)); return 0; }
	NB_PROP_SET(xoffset)				{ self(v)->SetXOffset(getInt(v, 2)); return 0; }
	NB_PROP_SET(zoom)					{ self(v)->SetZoom(getInt(v, 2)); return 0; }
	NB_PROP_SET(whitespaceSize)			{ self(v)->SetWhitespaceSize(getInt(v, 2)); return 0; }
	NB_PROP_SET(annotationVisible)		{ self(v)->AnnotationSetVisible(getBool(v, 2)); return 0; }
	NB_PROP_SET(annotationStyleOffset)	{ self(v)->AnnotationSetStyleOffset(getInt(v, 2)); return 0; }
	NB_PROP_SET(rectSelCaret)			{ self(v)->SetRectangularSelectionCaret(getInt(v, 2)); return 0; }
	NB_PROP_SET(rectSelAnchor)			{ self(v)->SetRectangularSelectionAnchor(getInt(v, 2)); return 0; }
	NB_PROP_SET(rectSelCaretVirtualSpace) { self(v)->SetRectangularSelectionCaretVirtualSpace(getInt(v, 2)); return 0; }
	NB_PROP_SET(rectSelAnchorVirtualSpace) { self(v)->SetRectangularSelectionAnchorVirtualSpace(getInt(v, 2)); return 0; }
	NB_PROP_SET(virtualSpaceOptions)	{ self(v)->SetVirtualSpaceOptions(getInt(v, 2)); return 0; }
	NB_PROP_SET(rectSelModifier)		{ self(v)->SetRectangularSelectionModifier(getInt(v, 2)); return 0; }

	NB_CONS()
	{
		type* self = setSelf(v, new type(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			*opt<wxPoint>(v, 4, wxDefaultPosition),
			*opt<wxSize>(v, 5, wxDefaultSize),
			optInt(v, 6, 0),
			optWxString(v, 7, wxSTCNameStr)
			));
		return SQ_OK;
	}

	NB_FUNC(addRefDocument)				{ self(v)->AddRefDocument(GetUserPointer(v, 2)); return 0; }
	NB_FUNC(addText)					{ self(v)->AddText(getWxString(v, 2)); return 0; }
	NB_FUNC(allocate)					{ self(v)->Allocate(getInt(v, 2)); return 0; }
	NB_FUNC(appendText)					{ const char* str = getString(v, 2); self(v)->AppendTextRaw(str, sq_getsize(v, 2)); return 0; }
	NB_FUNC(autoCompCancel)				{ self(v)->AutoCompCancel(); return 0; }
	NB_FUNC(autoCompComplete)			{ self(v)->AutoCompComplete(); return 0; }
	NB_FUNC(autoCompSetFillUps)			{ self(v)->AutoCompSetFillUps(getWxString(v, 2)); return 0; }
	NB_FUNC(autoCompShow)				{ self(v)->AutoCompShow(getInt(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(autoCompSetStops)			{ self(v)->AutoCompStops(getWxString(v, 2)); return 0; }
	NB_FUNC(beginUndoAction)			{ self(v)->BeginUndoAction(); return 0; }
	NB_FUNC(braceBadLight)				{ self(v)->BraceBadLight(getInt(v, 2)); return 0; }
	NB_FUNC(braceHighlight)				{ self(v)->BraceHighlight(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(braceMatch)					{ self(v)->BraceMatch(getInt(v, 2)); return 0; }
	NB_FUNC(callTipCancel)				{ self(v)->CallTipCancel(); return 0; }
	NB_FUNC(callTipSetBackColor)		{ self(v)->CallTipSetBackground(GetWxColor(v, 2)); return 0; }
	NB_FUNC(callTipSetForeColor)		{ self(v)->CallTipSetForeground(GetWxColor(v, 2)); return 0; }
	NB_FUNC(callTipSetHighlightColor)	{ self(v)->CallTipSetForegroundHighlight(GetWxColor(v, 2)); return 0; }
	NB_FUNC(callTipHighlight)			{ self(v)->CallTipSetHighlight(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(callTipShow)				{ self(v)->CallTipShow(getInt(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(callTipUseStyle)			{ self(v)->CallTipUseStyle(getInt(v, 2)); return 0; }
	NB_FUNC(cancel)						{ self(v)->Cancel(); return 0; }
	NB_FUNC(chooseCaretX)				{ self(v)->ChooseCaretX(); return 0; }
	NB_FUNC(clear)						{ self(v)->Clear(); return 0; }
	NB_FUNC(clearAll)					{ self(v)->ClearAll(); return 0; }
	NB_FUNC(clearDocumentStyle)			{ self(v)->ClearDocumentStyle(); return 0; }
	NB_FUNC(clearRegisteredImages)		{ self(v)->ClearRegisteredImages(); return 0; }
	NB_FUNC(cmdKeyAssign)				{ self(v)->CmdKeyAssign(getInt(v, 2), getInt(v, 3), getInt(v, 4)); return 0; }
	NB_FUNC(cmdKeyClear)				{ self(v)->CmdKeyClear(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(cmdKeyClearAll)				{ self(v)->CmdKeyClearAll(); return 0; }
	NB_FUNC(exec)						{ self(v)->CmdKeyExecute(getInt(v, 2)); return 0; }
	NB_FUNC(colorize)					{ self(v)->Colourise(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(convertEOLs)				{ self(v)->ConvertEOLs(getInt(v, 2)); return 0; }
	NB_FUNC(copy)						{ self(v)->Copy(); return 0; }
	NB_FUNC(copyRange)					{ self(v)->CopyRange(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(copyText)					{ self(v)->CopyText(getInt(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(createDocument)				{ sq_pushuserpointer(v, self(v)->CreateDocument()); return 1; }
	NB_FUNC(cut)						{ self(v)->Cut(); return 0; }
	NB_FUNC(docLineFromVisible)			{ return push(v, self(v)->DocLineFromVisible(getInt(v, 2))); }
	NB_FUNC(editToggleOvertype)			{ self(v)->EditToggleOvertype(); return 0; }
	NB_FUNC(emptyUndoBuffer)			{ self(v)->EmptyUndoBuffer(); return 0; }
	NB_FUNC(endUndoAction)				{ self(v)->EndUndoAction(); return 0; }
	NB_FUNC(ensureCaretVisible)			{ self(v)->EnsureCaretVisible(); return 0; }
	NB_FUNC(ensureVisible)				{ self(v)->EnsureVisible(getInt(v, 2)); return 0; }
	NB_FUNC(ensureVisibleEnforcePolicy){ self(v)->EnsureVisibleEnforcePolicy(getInt(v, 2)); return 0; }
	NB_FUNC(findColumn)					{ return push(v, self(v)->FindColumn(getInt(v, 2), getInt(v, 3))); }
	NB_FUNC(findText)					{ return push(v, self(v)->FindText(getInt(v, 2), getInt(v, 3), getWxString(v, 4), optInt(v, 5, 0))); }
	NB_FUNC(formatRange)				{ return push(v, self(v)->FormatRange(getBool(v, 2), getInt(v, 3), getInt(v, 4), get<wxDC>(v, 5), get<wxDC>(v, 6), *get<wxRect>(v, 7), *get<wxRect>(v, 8))); }
	NB_FUNC(getCharAt)					{ return push(v, self(v)->GetCharAt(getInt(v, 2))); }
	NB_FUNC(getColumn)					{ return push(v, self(v)->GetColumn(getInt(v, 2))); }
	NB_FUNC(getFoldExpanded)			{ return push(v, self(v)->GetFoldExpanded(getInt(v, 2))); }
	NB_FUNC(getFoldLevel)				{ return push(v, self(v)->GetFoldLevel(getInt(v, 2))); }
	NB_FUNC(getFoldParent)				{ return push(v, self(v)->GetFoldParent(getInt(v, 2))); }
	NB_FUNC(getLastChild)				{ return push(v, self(v)->GetLastChild(getInt(v, 2), getInt(v, 3))); }
	NB_FUNC(getLine)					{ return push(v, self(v)->GetLine(getInt(v, 2))); }
	NB_FUNC(getLineEndPosition)			{ return push(v, self(v)->GetLineEndPosition(getInt(v, 2))); }
	NB_FUNC(getLineIndentPosition)		{ return push(v, self(v)->GetLineIndentPosition(getInt(v, 2))); }
	NB_FUNC(getLineIndentation)			{ return push(v, self(v)->GetLineIndentation(getInt(v, 2))); }
	NB_FUNC(getLineSelEndPosition)		{ return push(v, self(v)->GetLineSelEndPosition(getInt(v, 2))); }
	NB_FUNC(getLineSelStartPosition)	{ return push(v, self(v)->GetLineSelStartPosition(getInt(v, 2))); }
	NB_FUNC(getLineState)				{ return push(v, self(v)->GetLineState(getInt(v, 2))); }
	NB_FUNC(getLineVisible)				{ return push(v, self(v)->GetLineVisible(getInt(v, 2))); }
	NB_FUNC(getMarginMask)				{ return push(v, self(v)->GetMarginMask(getInt(v, 2))); }
	NB_FUNC(getMarginSensitive)			{ return push(v, self(v)->GetMarginSensitive(getInt(v, 2))); }
	NB_FUNC(getMarginType)				{ return push(v, self(v)->GetMarginType(getInt(v, 2))); }
	NB_FUNC(getMarginWidth)				{ return push(v, self(v)->GetMarginWidth(getInt(v, 2))); }
	NB_FUNC(getProperty)				{ return push(v, self(v)->GetProperty(getWxString(v, 2))); }
	NB_FUNC(getPropertyExpanded)		{ return push(v, self(v)->GetPropertyExpanded(getWxString(v, 2))); }
	NB_FUNC(getPropertyInt)				{ return push(v, self(v)->GetPropertyInt(getWxString(v, 2))); }
	NB_FUNC(getStyleAt)					{ return push(v, self(v)->GetStyleAt(getInt(v, 2))); }
	NB_FUNC(getText)					{ return push(v, self(v)->GetText()); }
	NB_FUNC(getTextRange)				{ return push(v, self(v)->GetTextRange(getInt(v, 2), getInt(v, 3))); }
	NB_FUNC(gotoLine)					{ self(v)->GotoLine(getInt(v, 2)); return 0; }
	NB_FUNC(gotoPos)					{ self(v)->GotoPos(getInt(v, 2)); return 0; }
	NB_FUNC(hideLines)					{ self(v)->HideLines(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(hideSelection)				{ self(v)->HideSelection(getBool(v, 2)); return 0; }
	NB_FUNC(indicatorGetAlpha)			{ return push(v, self(v)->IndicatorGetAlpha(getInt(v, 2))); }
	NB_FUNC(indicatorSetAlpha)			{ self(v)->IndicatorSetAlpha(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(indicatorGetForeColor)		{ return PushWxColor(v, self(v)->IndicatorGetForeground(getInt(v, 2))); }
	NB_FUNC(indicatorGetStyle)			{ return push(v, self(v)->IndicatorGetStyle(getInt(v, 2))); }
	NB_FUNC(indicatorSetForeColor)		{ self(v)->IndicatorSetForeground(getInt(v, 2), GetWxColor(v, 2)); return 0; }
	NB_FUNC(indicatorSetStyle)			{ self(v)->IndicatorSetStyle(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(insertText)					{ self(v)->InsertText(getInt(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(lineFromPosition)			{ return push(v, self(v)->LineFromPosition(getInt(v, 2))); return 0; }
	NB_FUNC(lineLength)					{ return push(v, self(v)->LineLength(getInt(v, 2))); return 0; }
	NB_FUNC(lineScroll)					{ self(v)->LineScroll(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(linesJoin)					{ self(v)->LinesJoin(); return 0; }
	NB_FUNC(linesSplit)					{ self(v)->LinesSplit(getInt(v, 2)); return 0; }
	NB_FUNC(loadFile)					{ return push(v, self(v)->LoadFile(getWxString(v, 2))); }
	NB_FUNC(markerAdd)					{ return push(v, self(v)->MarkerAdd(getInt(v, 2), getInt(v, 3))); }
	NB_FUNC(markerDefine)				{ self(v)->MarkerDefine(getInt(v, 2), getInt(v, 3), OptWxColor(v, 4, wxNullColour), OptWxColor(v, 5, wxNullColour)); return 0; }
	NB_FUNC(markerDefineBitmap)			{ self(v)->MarkerDefineBitmap(getInt(v, 2), *get<wxBitmap>(v, 3)); return 0; }
	NB_FUNC(markerDelete)				{ self(v)->MarkerDelete(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(markerDeleteAll)			{ self(v)->MarkerDeleteAll(getInt(v, 2)); return 0; }
	NB_FUNC(markerDeleteHandle)			{ self(v)->MarkerDeleteHandle(getInt(v, 2)); return 0; }
	NB_FUNC(markerGet)					{ return push(v, self(v)->MarkerGet(getInt(v, 2))); }
	NB_FUNC(markerLineFromHandle)		{ return push(v, self(v)->MarkerLineFromHandle(getInt(v, 2))); }
	NB_FUNC(markerNext)					{ return push(v, self(v)->MarkerNext(getInt(v, 2), getInt(v, 3))); }
	NB_FUNC(markerPrevious)				{ return push(v, self(v)->MarkerPrevious(getInt(v, 2), getInt(v, 3))); }
	NB_FUNC(markerSetAlpha)				{ self(v)->MarkerSetAlpha(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(markerSetBackColor)			{ self(v)->MarkerSetBackground(getInt(v, 2), GetWxColor(v, 3)); return 0; }
	NB_FUNC(markerSetForeColor)			{ self(v)->MarkerSetForeground(getInt(v, 2), GetWxColor(v, 3)); return 0; }
	NB_FUNC(moveCaretInsideView)		{ self(v)->MoveCaretInsideView(); return 0; }
	NB_FUNC(newLine)					{ self(v)->NewLine(); return 0; }
	NB_FUNC(paste)						{ self(v)->Paste(); return 0; }
	NB_FUNC(pointFromPosition)			{ return push(v, self(v)->PointFromPosition(getInt(v, 2))); }
	NB_FUNC(after)						{ return push(v, self(v)->PositionAfter(getInt(v, 2))); }
	NB_FUNC(before)						{ return push(v, self(v)->PositionBefore(getInt(v, 2))); }
	NB_FUNC(positionFromLine)			{ return push(v, self(v)->PositionFromLine(getInt(v, 2))); }
	NB_FUNC(positionFromPoint)			{ return push(v, self(v)->PositionFromPoint(*get<wxPoint>(v, 2))); }
	NB_FUNC(positionFromPointClose)		{ wxPoint& pos = *get<wxPoint>(v, 2); return push(v, self(v)->PositionFromPointClose(pos.x, pos.y)); }
	NB_FUNC(redo)						{ self(v)->Redo(); return 0; }
	NB_FUNC(registerImage)				{ self(v)->RegisterImage(getInt(v, 2), *get<wxBitmap>(v, 3)); return 0; }
	NB_FUNC(releaseDocument)			{ self(v)->ReleaseDocument(GetUserPointer(v, 2)); return 0; }
	NB_FUNC(replaceSelection)			{ self(v)->ReplaceSelection(getWxString(v, 2)); return 0; }
	NB_FUNC(replaceTarget)				{ self(v)->ReplaceTarget(getWxString(v, 2)); return 0; }
	NB_FUNC(replaceTargetRE)			{ self(v)->ReplaceTargetRE(getWxString(v, 2)); return 0; }
	NB_FUNC(saveFile)					{ return push(v, self(v)->SaveFile(optWxString(v, 2, wxEmptyString))); }
	NB_FUNC(scrollToColumn)				{ self(v)->ScrollToColumn(getInt(v, 2)); return 0; }
	NB_FUNC(scrollToLine)				{ self(v)->ScrollToLine(getInt(v, 2)); return 0; }
	NB_FUNC(searchAnchor)				{ self(v)->SearchAnchor(); return 0; }
	NB_FUNC(searchInTarget)				{ self(v)->SearchInTarget(getWxString(v, 2)); return 0; }
	NB_FUNC(searchNext)					{ return push(v, self(v)->SearchNext(getInt(v, 2), getWxString(v, 3))); }
	NB_FUNC(searchPrev)					{ return push(v, self(v)->SearchPrev(getInt(v, 2), getWxString(v, 3))); }
	NB_FUNC(selectionDuplicate)			{ self(v)->SelectionDuplicate(); return 0; }
	NB_FUNC(sendMsg)					{ return push(v, (int)self(v)->SendMsg(getInt(v, 2), (wxUIntPtr)getInt(v, 3), (wxUIntPtr)getInt(v, 4))); }
	NB_FUNC(setAnchor)					{ self(v)->SetAnchor(getInt(v, 2)); return 0; }
	NB_FUNC(setCharsDefault)			{ self(v)->SetCharsDefault(); return 0; }
	NB_FUNC(setControlCharSymbol)		{ self(v)->SetControlCharSymbol(getInt(v, 2)); return 0; }
	NB_FUNC(setFoldExpanded)			{ self(v)->SetFoldExpanded(getInt(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC(setFoldFlags)				{ self(v)->SetFoldFlags(getInt(v, 2)); return 0; }
	NB_FUNC(setFoldLevel)				{ self(v)->SetFoldLevel(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(setFoldMarginColor)			{ wxColor color = OptWxColor(v, 2, wxNullColour); self(v)->SetFoldMarginColour(color.IsOk(), color); return 0; }
	NB_FUNC(setFoldMarginHiColor)		{ wxColor color = OptWxColor(v, 2, wxNullColour); self(v)->SetFoldMarginHiColour(color.IsOk(), color); return 0; }
	NB_FUNC(setHScrollBar)				{ self(v)->SetHScrollBar(opt<wxScrollBar>(v, 2, NULL)); return 0; }
	NB_FUNC(setHotspotActiveBackColor)	{ wxColor color = OptWxColor(v, 2, wxNullColour); self(v)->SetHotspotActiveBackground(color.IsOk(), color); return 0; }
	NB_FUNC(setHotspotActiveForeColor)	{ wxColor color = OptWxColor(v, 2, wxNullColour); self(v)->SetHotspotActiveForeground(color.IsOk(), color); return 0; }
	NB_FUNC(setHotspotActiveUnderline)	{ self(v)->SetHotspotActiveUnderline(getBool(v, 2)); return 0; }
	NB_FUNC(setHotspotSingleLine)		{ self(v)->SetHotspotSingleLine(getBool(v, 2)); return 0; }
	NB_FUNC(setKeywords)				{ self(v)->SetKeyWords(getInt(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(setLexerLanguage)			{ self(v)->SetLexerLanguage(getWxString(v, 2)); return 0; }
	NB_FUNC(setLineIndentation)			{ self(v)->SetLineIndentation(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(setLineState)				{ self(v)->SetLineState(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(setMarginSensitive)			{ self(v)->SetMarginSensitive(getInt(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC(setMarginType)				{ self(v)->SetMarginType(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(setMarginMask)				{ self(v)->SetMarginMask(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(setMarginWidth)				{ self(v)->SetMarginWidth(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(setMargins)					{ self(v)->SetMargins(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(setProperty)				{ self(v)->SetProperty(getWxString(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(setSavePoint)				{ self(v)->SetSavePoint(); return 0; }
	NB_FUNC(setSelBackColor)			{ wxColor color = OptWxColor(v, 2, wxNullColour); self(v)->SetSelBackground(color.IsOk(), color); return 0; }
	NB_FUNC(setSelForeColor)			{ wxColor color = OptWxColor(v, 2, wxNullColour); self(v)->SetSelForeground(color.IsOk(), color); return 0; }
	NB_FUNC(setSelection)				{ self(v)->SetSelection(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(setStyling)					{ self(v)->SetStyling(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(setText)					{ self(v)->SetText(getWxString(v, 2)); return 0; }
	NB_FUNC(setVScrollBar)				{ self(v)->SetVScrollBar(opt<wxScrollBar>(v, 2, NULL)); return 0; }
	NB_FUNC(setWhitespaceBackColor)		{ wxColor color = OptWxColor(v, 2, wxNullColour); self(v)->SetWhitespaceBackground(color.IsOk(), color); return 0; }
	NB_FUNC(setWhitespaceChars)			{ self(v)->SetWhitespaceChars(getWxString(v, 2)); return 0; }
	NB_FUNC(setWhitespaceForeColor)		{ wxColor color = OptWxColor(v, 2, wxNullColour); self(v)->SetWhitespaceForeground(color.IsOk(), color); return 0; }
	NB_FUNC(setWordChars)				{ self(v)->SetWordChars(getWxString(v, 2)); return 0; }
	NB_FUNC(setXCaretPolicy)			{ self(v)->SetXCaretPolicy(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(setYCaretPolicy)			{ self(v)->SetYCaretPolicy(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(setVisiblePolicy)			{ self(v)->SetVisiblePolicy(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(showLines)					{ self(v)->ShowLines(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(startRecord)				{ self(v)->StartRecord(); return 0; }
	NB_FUNC(startStyling)				{ self(v)->StartStyling(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(stopRecord)					{ self(v)->StopRecord(); return 0; }
	NB_FUNC(styleClearAll)				{ self(v)->StyleClearAll(); return 0; }
	NB_FUNC(styleResetDefault)			{ self(v)->StyleResetDefault(); return 0; }
	NB_FUNC(styleSetBackColor)			{ self(v)->StyleSetBackground(getInt(v, 2), GetWxColor(v, 3)); return 0; }
	NB_FUNC(styleSetBold)				{ self(v)->StyleSetBold(getInt(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC(styleSetCase)				{ self(v)->StyleSetCase(getInt(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC(styleSetChangeable)			{ self(v)->StyleSetChangeable(getInt(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC(styleSetCharacterSet)		{ self(v)->StyleSetCharacterSet(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(styleSetEOLFilled)			{ self(v)->StyleSetEOLFilled(getInt(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC(styleSetFaceName)			{ self(v)->StyleSetFaceName(getInt(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(styleSetFont)				{ self(v)->StyleSetFont(getInt(v, 2), *get<wxFont>(v, 3)); return 0; }
	NB_FUNC(styleSetForeColor)			{ self(v)->StyleSetForeground(getInt(v, 2), GetWxColor(v, 3)); return 0; }
	NB_FUNC(styleSetHotSpot)			{ self(v)->StyleSetHotSpot(getInt(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC(styleSetItalic)				{ self(v)->StyleSetItalic(getInt(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC(styleSetSize)				{ self(v)->StyleSetSize(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(styleSetSpec)				{ self(v)->StyleSetSpec(getInt(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(styleSetUnderline)			{ self(v)->StyleSetUnderline(getInt(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC(styleSetVisible)			{ self(v)->StyleSetVisible(getInt(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC(targetFromSelection)		{ self(v)->TargetFromSelection(); return 0; }
	NB_FUNC(textHeight)					{ return push(v, self(v)->TextHeight(getInt(v, 2))); }
	NB_FUNC(textWidth)					{ return push(v, self(v)->TextWidth(getInt(v, 2), getWxString(v, 3))); }
	NB_FUNC(toggleCaretSticky)			{ self(v)->ToggleCaretSticky(); return 0; }
	NB_FUNC(toggleFold)					{ self(v)->ToggleFold(getInt(v, 2)); return 0; }
	NB_FUNC(undo)						{ self(v)->Undo(); return 0; }
	NB_FUNC(usePopUp)					{ self(v)->UsePopUp(getBool(v, 2)); return 0; }
	NB_FUNC(userListShow)				{ self(v)->UserListShow(getInt(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(visibleFromDocLine)			{ return push(v, self(v)->VisibleFromDocLine(getInt(v, 2))); }
	NB_FUNC(wordEndPosition)			{ return push(v, self(v)->WordEndPosition(getInt(v, 2), getBool(v, 3))); }
	NB_FUNC(wordStartPosition)			{ return push(v, self(v)->WordStartPosition(getInt(v, 2), getBool(v, 3))); }
	NB_FUNC(wrapCount)					{ return push(v, self(v)->WrapCount(getInt(v, 2))); }
	NB_FUNC(zoomIn)						{ self(v)->ZoomIn(); return 0; }
	NB_FUNC(zoomOut)					{ self(v)->ZoomOut(); return 0; }
	NB_FUNC(copyAllowLine)				{ self(v)->CopyAllowLine(); return 0; }
	NB_FUNC(getMarkerSymbolDefined)		{ return push(v, self(v)->GetMarkerSymbolDefined(getInt(v, 2))); }
	NB_FUNC(marginSetText)				{ self(v)->MarginSetText(getInt(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(marginGetText)				{ return push(v, self(v)->MarginGetText(getInt(v, 2))); }
	NB_FUNC(marginSetStyle)				{ self(v)->MarginSetStyle(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(marginGetStyle)				{ return push(v, self(v)->MarginGetStyle(getInt(v, 2))); }
	NB_FUNC(marginSetStyles)			{ self(v)->MarginSetStyles(getInt(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(marginGetStyles)			{ return push(v, self(v)->MarginGetStyles(getInt(v, 2))); }
	NB_FUNC(marginTextClearAll)			{ self(v)->MarginTextClearAll(); return 0; }
	NB_FUNC(marginSetStyleOffset)		{ self(v)->MarginSetStyleOffset(getInt(v, 2)); return 0; }
	NB_FUNC(annotationSetText)			{ self(v)->AnnotationSetText(getInt(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(annotationGetText)			{ return push(v, self(v)->AnnotationGetText(getInt(v, 2))); }
	NB_FUNC(annotationGetStyle)			{ return push(v, self(v)->AnnotationGetStyle(getInt(v, 2))); }
	NB_FUNC(annotationSetStyle)			{ self(v)->AnnotationSetStyle(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(annotationSetStyles)		{ self(v)->AnnotationSetStyles(getInt(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(annotationGetStyles)		{ return push(v, self(v)->AnnotationGetStyles(getInt(v, 2))); }
	NB_FUNC(annotationClearAll)			{ self(v)->AnnotationClearAll(); return 0; }
	NB_FUNC(addUndoAction)				{ self(v)->AddUndoAction(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(charPositionFromPoint)		{ wxPoint& pt = *get<wxPoint>(v, 2); return push(v, self(v)->CharPositionFromPoint(pt.x, pt.y)); }
	NB_FUNC(charPositionFromPointClose){ wxPoint& pt = *get<wxPoint>(v, 2); return push(v, self(v)->CharPositionFromPointClose(pt.x, pt.y)); }
};

////////////////////////////////////////////////////////////////////////////////

SQRESULT NitLibWxTextCtrl(HSQUIRRELVM v)
{
	NB_WxTextCtrl::Register(v);
	NB_WxRichTextCtrl::Register(v);

	NB_WxStyledTextEvent::Register(v);
	NB_WxStyledTextCtrl::Register(v);

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
