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

static wxPGPropArgCls GetPropArg(HSQUIRRELVM v, int idx)
{
	// NOTE: PGPropArgCls uses a real c_str. 
	// If we use wxString here, The value will be filled with garbages due to release of temp-local variable.
	if (NitBind::isString(v, idx))
		return NitBind::getString(v, idx); 
	else
		return NitBind::get<wxPGProperty>(v, idx);
}

class NB_WxPropertyGridInterface : TNitInterface<wxPropertyGridInterface>
{
public:
	template <typename TClass>
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] = 
		{
			PROP_ENTRY_R(selection),
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY_H(clear,						"()"),
			FUNC_ENTRY_H(append,					"(property: PGProperty): PGProperty"),
			FUNC_ENTRY_H(appendIn,					"(parent: prop or name, property: PGProperty): PGProperty"),
			FUNC_ENTRY_H(deleteProperty,			"(prop or name)"),
			FUNC_ENTRY_H(replaceProperty,			"(prop or name, property: PGProperty): PGProperty"),
			FUNC_ENTRY_H(enableProperty,			"(prop or name, enable=true): bool"),
			FUNC_ENTRY_H(hideProperty,				"(prop or name, hide=true, recurse=true): bool"),
			FUNC_ENTRY_H(beginAddChildren,			"(prop or name)"),
			FUNC_ENTRY_H(endAddChildren,			"(prop or name)"),

			FUNC_ENTRY_H(collapse,					"(prop or name): bool"),
			FUNC_ENTRY_H(collapseAll,				"(): bool"),
			FUNC_ENTRY_H(expand,					"(prop or name): bool"),
			FUNC_ENTRY_H(expandAll,					"(expand=true): bool"),

			FUNC_ENTRY_H(isPropertyCategory,		"(prop or name): bool"),
			FUNC_ENTRY_H(isPropertyEnabled,			"(prop or name): bool"),
			FUNC_ENTRY_H(isPropertyExpanded,		"(prop or name): bool"),
			FUNC_ENTRY_H(isPropertyModified,		"(prop or name): bool"),
			FUNC_ENTRY_H(isPropertySelected,		"(prop or name): bool"),
			FUNC_ENTRY_H(isPropertyShown,			"(prop or name): bool"),
			FUNC_ENTRY_H(isPropertyValueUnspecified, "(prop or name): bool"),

			FUNC_ENTRY_H(getPropertyValue,			"(prop or name): variant"),
			FUNC_ENTRY_H(setPropertyValue,			"(prop or name, value: variant)"),
			FUNC_ENTRY_H(setPropertyValueUnspecified, "(prop or name)"),
			FUNC_ENTRY_H(setPropertyHelpString,		"(prop or name, help: string)"),
			FUNC_ENTRY_H(setPropertyImage,			"(prop or name, bmp: Bitmap)"),
			FUNC_ENTRY_H(setPropertyValidator,		"(prop or name, validator: Validator)"),
			FUNC_ENTRY_H(changePropertyValue,		"(prop or name, value: variant) // as if user (contrary to TextCtrl.ChangeValue)"),

			FUNC_ENTRY_H(clearModifiedStatus,		"()"),
			FUNC_ENTRY_H(editorValidate,			"(): bool"),

			FUNC_ENTRY_H(getPropertyAttribute,		"(prop or name, attrName: string): variant"),
			FUNC_ENTRY_H(setPropertyAttribute,		"(prop or name, attrName: string, attr: variant, argFlags=0)"),
			FUNC_ENTRY_H(getColumnProportion,		"(column: int): int"),
			FUNC_ENTRY_H(setColumnProportion,		"(column: int, proportion: int): bool"),
			FUNC_ENTRY_H(setBoolChoices,			"[class] (trueChoice, falseChoice: string)"),
			FUNC_ENTRY_H(setValidationFailureBehavior, "(VFB)"),

			FUNC_ENTRY_H(sort,						"(flags=0: SORT)"),
			FUNC_ENTRY_H(sortChildren,				"(prop or name, flags=0: SORT)"),

			FUNC_ENTRY_H(clearSelection,			"(validate=false): bool"),
			NULL
		};

		bind<TClass>(v, props, funcs);

		addStaticTable<TClass>(v, "STYLE");
		newSlot(v, -1, "DEFAULT",					(int)wxPG_DEFAULT_STYLE);
		newSlot(v, -1, "AUTO_SORT",					(int)wxPG_AUTO_SORT);
		newSlot(v, -1, "HIDE_CATEGORIES",			(int)wxPG_HIDE_CATEGORIES);
		newSlot(v, -1, "ALPHABETIC_MODE",			(int)wxPG_ALPHABETIC_MODE);
		newSlot(v, -1, "BOLD_MODIFIED",				(int)wxPG_BOLD_MODIFIED);
		newSlot(v, -1, "SPLITTER_AUTO_CENTER",		(int)wxPG_SPLITTER_AUTO_CENTER);
		newSlot(v, -1, "TOOLTIPS",					(int)wxPG_TOOLTIPS);
		newSlot(v, -1, "HIDE_MARGIN",				(int)wxPG_HIDE_MARGIN);
		newSlot(v, -1, "STATIC_SPLITTER",			(int)wxPG_STATIC_SPLITTER);
		newSlot(v, -1, "STATIC_LAYOUT",				(int)wxPG_STATIC_LAYOUT);
		newSlot(v, -1, "LIMITED_EDITING",			(int)wxPG_LIMITED_EDITING);

		newSlot(v, -1, "EX_INIT_NOCAT",						(int)wxPG_EX_INIT_NOCAT);
		newSlot(v, -1, "EX_NO_FLAT_TOOLBAR",				(int)wxPG_EX_NO_FLAT_TOOLBAR);
		newSlot(v, -1, "EX_MODE_BUTTONS",					(int)wxPG_EX_MODE_BUTTONS);
		newSlot(v, -1, "EX_HELP_AS_TOOLTIPS",				(int)wxPG_EX_HELP_AS_TOOLTIPS);
		newSlot(v, -1, "EX_NATIVE_DOUBLE_BUFFERING",		(int)wxPG_EX_NATIVE_DOUBLE_BUFFERING);
		newSlot(v, -1, "EX_AUTO_UNSPECIFIED_VALUES",		(int)wxPG_EX_AUTO_UNSPECIFIED_VALUES);
		newSlot(v, -1, "EX_WRITEONLY_BUILTIN_ATTRIBUTES",	(int)wxPG_EX_WRITEONLY_BUILTIN_ATTRIBUTES);
		newSlot(v, -1, "EX_HIDE_PAGE_BUTTONS",				(int)wxPG_EX_HIDE_PAGE_BUTTONS);
		newSlot(v, -1, "EX_MULTIPLE_SELECTION",				(int)wxPG_EX_MULTIPLE_SELECTION);
		newSlot(v, -1, "EX_ENABLE_TLP_TRACKING",			(int)wxPG_EX_ENABLE_TLP_TRACKING);
		newSlot(v, -1, "EX_NO_TOOLBAR_DIVIDER",				(int)wxPG_EX_NO_TOOLBAR_DIVIDER);
		newSlot(v, -1, "EX_TOOLBAR_SEPARATOR",				(int)wxPG_EX_TOOLBAR_SEPARATOR);
		sq_poptop(v);

		addStaticTable<TClass>(v, "ATTR");
		newSlot(v, -1, "DEFAULT_VALUE",						wxString(wxPG_ATTR_DEFAULT_VALUE));
		newSlot(v, -1, "MIN",								wxString(wxPG_ATTR_MIN));
		newSlot(v, -1, "MAX",								wxString(wxPG_ATTR_MAX));
		newSlot(v, -1, "UNITS",								wxString(wxPG_ATTR_UNITS));
		newSlot(v, -1, "HINT",								wxString(wxPG_ATTR_HINT));
		newSlot(v, -1, "AUTOCOMPLETE",						wxString(wxPG_ATTR_AUTOCOMPLETE));
		newSlot(v, -1, "BOOL_USE_CHECKBOX",					wxString(wxPG_BOOL_USE_CHECKBOX));
		newSlot(v, -1, "BOOL_USE_DOUBLE_CLICK_CYCLING",		wxString(wxPG_BOOL_USE_DOUBLE_CLICK_CYCLING));
		newSlot(v, -1, "FLOAT_PRECISION",					wxString(wxPG_FLOAT_PRECISION));
		newSlot(v, -1, "STRING_PASSWORD",					wxString(wxPG_STRING_PASSWORD));
		newSlot(v, -1, "UINT_BASE",							wxString(wxPG_UINT_BASE));
		newSlot(v, -1, "UINT_PREFIX",						wxString(wxPG_UINT_PREFIX));
		newSlot(v, -1, "FILE_WILDCARD",						wxString(wxPG_FILE_WILDCARD));
		newSlot(v, -1, "FILE_SHOW_FULL_PATH",				wxString(wxPG_FILE_SHOW_FULL_PATH));
		newSlot(v, -1, "FILE_SHOW_RELATIVE_PATH",			wxString(wxPG_FILE_SHOW_RELATIVE_PATH));
		newSlot(v, -1, "FILE_INITIAL_PATH",					wxString(wxPG_FILE_INITIAL_PATH));
		newSlot(v, -1, "FILE_DIALOG_TITLE",					wxString(wxPG_FILE_DIALOG_TITLE));
		newSlot(v, -1, "FILE_DIALOG_STYLE",					wxString(wxPG_FILE_DIALOG_STYLE));
		newSlot(v, -1, "DIR_DIALOG_MESSAGE",				wxString(wxPG_DIR_DIALOG_MESSAGE));
		newSlot(v, -1, "ARRAY_DELIMITER",					wxString(wxPG_ARRAY_DELIMITER));
		newSlot(v, -1, "DATE_FORMAT",						wxString(wxPG_DATE_FORMAT));
		newSlot(v, -1, "DATE_PICKER_STYLE",					wxString(wxPG_DATE_PICKER_STYLE));
		newSlot(v, -1, "SPINCTRL_STEP",						wxString(wxPG_ATTR_SPINCTRL_STEP));
		newSlot(v, -1, "SPINCTRL_WRAP",						wxString(wxPG_ATTR_SPINCTRL_WRAP));
//		NewSlot(v, -1, "SPINCTRL_MOTIONSPIN",				wxString(wxPG_ATTR_SPINCTRL_MOTIONSPIN));	// TODO: can't find yet
		newSlot(v, -1, "MULTICHOICE_USERSTRINGMODE",		wxString(wxPG_ATTR_MULTICHOICE_USERSTRINGMODE));
		newSlot(v, -1, "COLOR_ALLOW_CUSTOM",				wxString(wxPG_COLOUR_ALLOW_CUSTOM));
		newSlot(v, -1, "COLOR_HAS_ALPHA",					wxString(wxPG_COLOUR_HAS_ALPHA));
		sq_poptop(v);

		addStaticTable<TClass>(v, "EVT");
		newSlot(v, -1, "SELECTED",					(int)wxEVT_PG_SELECTED);
		newSlot(v, -1, "CHANGED",					(int)wxEVT_PG_CHANGED);
		newSlot(v, -1, "CHANGING",					(int)wxEVT_PG_CHANGING);
		newSlot(v, -1, "HIGHLIGHTED",				(int)wxEVT_PG_HIGHLIGHTED);
		newSlot(v, -1, "RIGHT_CLICK",				(int)wxEVT_PG_RIGHT_CLICK);
		newSlot(v, -1, "DOUBLE_CLICK",				(int)wxEVT_PG_DOUBLE_CLICK);
		newSlot(v, -1, "ITEM_COLLAPSED",			(int)wxEVT_PG_ITEM_COLLAPSED);
		newSlot(v, -1, "ITEM_EXPANDED",				(int)wxEVT_PG_ITEM_EXPANDED);
		newSlot(v, -1, "LABEL_EDIT_BEGIN",			(int)wxEVT_PG_LABEL_EDIT_BEGIN);
		newSlot(v, -1, "LABEL_EDIT_ENDING",			(int)wxEVT_PG_LABEL_EDIT_ENDING);
		newSlot(v, -1, "COL_BEGIN_DRAG",			(int)wxEVT_PG_COL_BEGIN_DRAG);
		newSlot(v, -1, "COL_DRAGGING",				(int)wxEVT_PG_COL_DRAGGING);
		newSlot(v, -1, "COL_END_DRAG",				(int)wxEVT_PG_COL_END_DRAG);
		sq_poptop(v);

		addStaticTable<TClass>(v, "ACTION");
		newSlot(v, -1, "INVALID",					(int)wxPG_ACTION_INVALID);
		newSlot(v, -1, "NEXT_PROPERTY",				(int)wxPG_ACTION_NEXT_PROPERTY);
		newSlot(v, -1, "PREV_PROPERTY",				(int)wxPG_ACTION_PREV_PROPERTY);
		newSlot(v, -1, "EXPAND_PROPERTY",			(int)wxPG_ACTION_EXPAND_PROPERTY);
		newSlot(v, -1, "COLLAPSE_PROPERTY",			(int)wxPG_ACTION_COLLAPSE_PROPERTY);
		newSlot(v, -1, "CANCEL_EDIT",				(int)wxPG_ACTION_CANCEL_EDIT);
		sq_poptop(v);

		addStaticTable<TClass>(v, "VFB");
		newSlot(v, -1, "STAY_IN_PROPERTY",			(int)wxPG_VFB_STAY_IN_PROPERTY);
		newSlot(v, -1, "BEEP",						(int)wxPG_VFB_BEEP);
		newSlot(v, -1, "MARK_CELL",					(int)wxPG_VFB_MARK_CELL);
		newSlot(v, -1, "SHOW_MESSAGE",				(int)wxPG_VFB_SHOW_MESSAGE);
		newSlot(v, -1, "SHOW_MESSAGEBOX",			(int)wxPG_VFB_SHOW_MESSAGEBOX);
		newSlot(v, -1, "SHOW_MESSAGE_ON_STATUSBAR",	(int)wxPG_VFB_SHOW_MESSAGE_ON_STATUSBAR);
		newSlot(v, -1, "DEFAULT",					(int)wxPG_VFB_DEFAULT);
		sq_poptop(v);

		addStaticTable<TClass>(v, "FLAG");
		newSlot(v, -1, "KEEP_STRUCTURE",			(int)wxPG_KEEP_STRUCTURE);
		newSlot(v, -1, "RECURSE",					(int)wxPG_RECURSE);
		newSlot(v, -1, "INC_ATTRIBUTES",			(int)wxPG_INC_ATTRIBUTES);
		newSlot(v, -1, "RECURSE_STARTS",			(int)wxPG_RECURSE_STARTS);
		newSlot(v, -1, "FORCE",						(int)wxPG_FORCE);
		newSlot(v, -1, "SORT_TOP_LEVEL_ONLY",		(int)wxPG_INC_ATTRIBUTES);
		newSlot(v, -1, "DONT_RECURSE",				(int)wxPG_DONT_RECURSE);
		sq_poptop(v);
	}

	NB_PROP_GET(selection)				{ return push(v, self(v)->wxPropertyGridInterface::GetSelection()); }

	NB_FUNC(clear)						{ self(v)->Clear(); return 0; }
	NB_FUNC(append)						{ return push(v, self(v)->Append(get<wxPGProperty>(v, 2))); }
	NB_FUNC(appendIn)					{ return push(v, self(v)->AppendIn(GetPropArg(v, 2), get<wxPGProperty>(v, 3))); }
	NB_FUNC(deleteProperty)				{ self(v)->DeleteProperty(GetPropArg(v, 2)); return 0; }
	NB_FUNC(replaceProperty)			{ return push(v, self(v)->ReplaceProperty(GetPropArg(v, 2), get<wxPGProperty>(v, 3))); }
	NB_FUNC(enableProperty)				{ self(v)->EnableProperty(GetPropArg(v, 2), optBool(v, 3, true)); return 0; }
	NB_FUNC(hideProperty)				{ return push(v, self(v)->HideProperty(GetPropArg(v, 2), optBool(v, 3, true), optBool(v, 4, true) ? wxPG_RECURSE : wxPG_DONT_RECURSE)); }
	NB_FUNC(beginAddChildren)			{ self(v)->BeginAddChildren(GetPropArg(v, 2)); return 0; }
	NB_FUNC(endAddChildren)				{ self(v)->EndAddChildren(GetPropArg(v, 2)); return 0; }

	NB_FUNC(collapse)					{ return push(v, self(v)->Collapse(GetPropArg(v, 2))); }
	NB_FUNC(collapseAll)				{ return push(v, self(v)->CollapseAll()); }
	NB_FUNC(expand)						{ return push(v, self(v)->Expand(GetPropArg(v, 2))); }
	NB_FUNC(expandAll)					{ return push(v, self(v)->ExpandAll(optBool(v, 2, true))); }

	NB_FUNC(isPropertyCategory)			{ return push(v, self(v)->IsPropertyCategory(GetPropArg(v, 2))); }
	NB_FUNC(isPropertyEnabled)			{ return push(v, self(v)->IsPropertyEnabled(GetPropArg(v, 2))); }
	NB_FUNC(isPropertyExpanded)			{ return push(v, self(v)->IsPropertyExpanded(GetPropArg(v, 2))); }
	NB_FUNC(isPropertyModified)			{ return push(v, self(v)->IsPropertyModified(GetPropArg(v, 2))); }
	NB_FUNC(isPropertySelected)			{ return push(v, self(v)->IsPropertySelected(GetPropArg(v, 2))); }
	NB_FUNC(isPropertyShown)			{ return push(v, self(v)->IsPropertyShown(GetPropArg(v, 2))); }
	NB_FUNC(isPropertyValueUnspecified)	{ return push(v, self(v)->IsPropertyValueUnspecified(GetPropArg(v, 2))); }

	NB_FUNC(getPropertyValue)			{ return PushWxVariant(v, self(v)->GetPropertyValue(GetPropArg(v, 2))); }
	NB_FUNC(setPropertyValue)			{ self(v)->SetPropertyValue(GetPropArg(v, 2), GetWxVariant(v, 3)); return 0; }
	NB_FUNC(setPropertyValueUnspecified)	{ self(v)->SetPropertyValueUnspecified(GetPropArg(v, 2)); return 0; }
	NB_FUNC(setPropertyHelpString)		{ self(v)->SetPropertyHelpString(GetPropArg(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(setPropertyImage)			{ self(v)->SetPropertyImage(GetPropArg(v, 2), *get<wxBitmap>(v, 3)); return 0; }
	NB_FUNC(setPropertyValidator)		{ self(v)->SetPropertyValidator(GetPropArg(v, 2), *opt<wxValidator>(v, 3, wxDefaultValidator)); return 0; }
	NB_FUNC(changePropertyValue)		{ self(v)->ChangePropertyValue(GetPropArg(v, 2), GetWxVariant(v, 3)); return 0; }

	NB_FUNC(clearModifiedStatus)		{ self(v)->ClearModifiedStatus(); return 0; }
	NB_FUNC(editorValidate)				{ return push(v, self(v)->EditorValidate()); }

	NB_FUNC(getPropertyAttribute)		{ return PushWxVariant(v, self(v)->GetPropertyAttribute(GetPropArg(v, 2), getWxString(v, 3))); }
	NB_FUNC(setPropertyAttribute)		{ self(v)->SetPropertyAttribute(GetPropArg(v, 2), getWxString(v, 3), GetWxVariant(v, 4), optInt(v, 5, 0)); return 0; }
	NB_FUNC(getColumnProportion)		{ return push(v, self(v)->GetColumnProportion(getInt(v, 2))); }
	NB_FUNC(setColumnProportion)		{ return push(v, self(v)->SetColumnProportion(getInt(v, 2), getInt(v, 3))); }
	NB_FUNC(setBoolChoices)				{ type::SetBoolChoices(getWxString(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(setValidationFailureBehavior)	{ self(v)->SetValidationFailureBehavior(getInt(v, 2)); return 0; }

	NB_FUNC(sort)						{ self(v)->Sort(optInt(v, 2, 0)); return 0; }
	NB_FUNC(sortChildren)				{ self(v)->SortChildren(GetPropArg(v, 2), optInt(v, 3, 0)); return 0; }

	NB_FUNC(clearSelection)				{ return push(v, self(v)->ClearSelection(optBool(v, 2, false))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, PropertyGrid, wxControl);

class NB_WxPropertyGrid : TNitClass<wxPropertyGrid>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(grid),
			PROP_ENTRY	(columnCount),
			PROP_ENTRY_R(editorFocused),
			PROP_ENTRY_R(fontHeight),
			PROP_ENTRY_R(root),
			PROP_ENTRY_R(rowHeight),
			PROP_ENTRY	(splitterPosition),
			PROP_ENTRY	(verticalSpacing),
			PROP_ENTRY_R(selectedProperty),
			PROP_ENTRY_R(anyModified),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(						"(parent: Window=null, id=ID.ANY, pos=null, size=null, style=STYLE.DEFAULT, name=null)"),

 			FUNC_ENTRY_H(enableCategories,		"(enable: bool): bool"),

			FUNC_ENTRY_H(ensureVisible,			"(prop or name): bool"),

			FUNC_ENTRY_H(setCurrentCategory,	"(prop or name)"),

			FUNC_ENTRY_H(addToSelection,		"(prop or name): bool"),
			FUNC_ENTRY_H(removeFromSelection,	"(prop or name): bool"),
			FUNC_ENTRY_H(selectProperty,		"(prop or name, focus=false): bool"),

			FUNC_ENTRY_H(makeColumnEditable,	"(column: int, editable=true)"),

			FUNC_ENTRY_H(resetColumnSizes,		"(enableAutoResizing=false)"),
			FUNC_ENTRY_H(fitColumns,			"(): Size"),
			FUNC_ENTRY_H(setSplitterLeft,		"(privateChildrenToo=false)"),

			FUNC_ENTRY_H(refreshEditor,			"()"),

			FUNC_ENTRY_H(getSplitterPosition,	"(splitterIdx: int): int"),
			FUNC_ENTRY_H(setSplitterPosition,	"(newXpos: int, splitterIdx: int)"),

			FUNC_ENTRY_H(addActionTrigger,		"(action: ACTION, keycode: KEY, modifiers: MOD)"),
			FUNC_ENTRY_H(clearActionTriggers,	"(action: ACTION)"),
			FUNC_ENTRY_H(dedicateKey,			"(keycode: KEY)"),
//			FUNC_ENTRY_H(setButtonShortcut,		"(keycode: KEY, ctrlDown=false, altDown=false)"), // TODO: not yet 2.9.1?
			NULL
		};

		bind(v, props, funcs);

		NB_WxScrollHelper::Register<type>(v);
		NB_WxPropertyGridInterface::Register<type>(v);
	}

	NB_CONS()
	{
		wxWindow* wnd = new wxPropertyGrid(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			*opt<wxPoint>(v, 4, wxDefaultPosition),
			*opt<wxSize>(v, 5, wxDefaultSize),
			optInt(v, 6, wxPG_DEFAULT_STYLE),
			optWxString(v, 7, wxPropertyGridNameStr)
			);

		sq_setinstanceup(v, 1, wnd);
		return 0; 
	}

	NB_PROP_GET(grid)					{ return push(v, self(v)->GetGrid()); }
 	NB_PROP_GET(columnCount)			{ return push(v, self(v)->GetColumnCount()); }
	NB_PROP_GET(editorFocused)			{ return push(v, self(v)->IsEditorFocused()); }
	NB_PROP_GET(fontHeight)				{ return push(v, self(v)->GetFontHeight()); }
	NB_PROP_GET(root)					{ return push(v, self(v)->GetRoot()); }
	NB_PROP_GET(rowHeight)				{ return push(v, self(v)->GetRowHeight()); }
	NB_PROP_GET(splitterPosition)		{ return push(v, self(v)->GetSplitterPosition()); }
	NB_PROP_GET(verticalSpacing)		{ return push(v, self(v)->GetVerticalSpacing()); }
	NB_PROP_GET(selectedProperty)		{ return push(v, self(v)->GetSelectedProperty()); }
	NB_PROP_GET(anyModified)			{ return push(v, self(v)->IsAnyModified()); }

	NB_PROP_SET(columnCount)			{ self(v)->SetColumnCount(getInt(v, 2)); return 0; }
	NB_PROP_SET(splitterPosition)		{ self(v)->SetSplitterPosition(getInt(v, 2)); return 0; }
	NB_PROP_SET(verticalSpacing)		{ self(v)->SetVerticalSpacing(getInt(v, 2)); return 0; }

	NB_FUNC(enableCategories)			{ return push(v, self(v)->EnableCategories(getBool(v, 2))); }

	NB_FUNC(setCurrentCategory)			{ self(v)->SetCurrentCategory(GetPropArg(v, 2)); return 0; }

	NB_FUNC(ensureVisible)				{ return push(v, self(v)->EnsureVisible(GetPropArg(v, 2))); }

	NB_FUNC(addToSelection)				{ return push(v, self(v)->AddToSelection(GetPropArg(v, 2))); }
	NB_FUNC(removeFromSelection)		{ return push(v, self(v)->RemoveFromSelection(GetPropArg(v, 2))); }
	NB_FUNC(selectProperty)				{ return push(v, self(v)->SelectProperty(GetPropArg(v, 2), optBool(v, 3, false))); }

	NB_FUNC(makeColumnEditable)			{ self(v)->MakeColumnEditable(getInt(v, 2), optBool(v, 3, true)); return 0; }

	NB_FUNC(resetColumnSizes)			{ self(v)->ResetColumnSizes(optBool(v, 2, false)); return 0; }
	NB_FUNC(fitColumns)					{ return push(v, self(v)->FitColumns()); }
	NB_FUNC(setSplitterLeft)			{ self(v)->SetSplitterLeft(optBool(v, 2, false)); return 0; }

	NB_FUNC(refreshEditor)				{ self(v)->RefreshEditor(); return 0; }

	NB_FUNC(getSplitterPosition)		{ return push(v, self(v)->GetSplitterPosition(getInt(v, 2))); }
	NB_FUNC(setSplitterPosition)		{ self(v)->SetSplitterPosition(getInt(v, 2), getInt(v, 3)); return 0; }

	NB_FUNC(addActionTrigger)			{ self(v)->AddActionTrigger(getInt(v, 2), getInt(v, 3), optInt(v, 4, 0)); return 0; }
	NB_FUNC(clearActionTriggers)		{ self(v)->ClearActionTriggers(getInt(v, 2)); return 0; }
	NB_FUNC(dedicateKey)				{ self(v)->DedicateKey(getInt(v, 2)); return 0; }
//	NB_FUNC(setButtonShortcut)			{ Self(v)->SetButtonShortut(GetInt(v, 2), OptBool(v, 3, false), OptBool(v, 4, false)); return 0; }
};
////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, PropertyGridManager, wxPanel);

class NB_WxPropertyGridManager : TNitClass<wxPropertyGridManager>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(grid),
			PROP_ENTRY	(columnCount),
			PROP_ENTRY	(descBoxHeight),
			PROP_ENTRY_R(currentPage),
			PROP_ENTRY_R(pageCount),
			PROP_ENTRY_R(selectedPage),
			PROP_ENTRY_R(toolBar),
			PROP_ENTRY_R(selectedProperty),
			PROP_ENTRY_R(anyModified),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(					"(parent: Window=null, id=ID.ANY, pos=null, size=null, style=STYLE.DEFAULT, name=null)"),

			FUNC_ENTRY_H(enableCategories,	"(enable: bool): bool"),

			FUNC_ENTRY_H(clearPage,			"(page: int)"),
			FUNC_ENTRY_H(addPage,			"(label=\"\", bmp: Bitmap=null, pageObj: PropertyGridPage=null): PropertyGridPage"),
			FUNC_ENTRY_H(insertPage,		"(index: int, label=\"\", bmp: Bitmap=null, pageObj: PropertyGridPage=null): PropertyGridPage"),
			FUNC_ENTRY_H(getPageByName,		"(name: string): int"),
			FUNC_ENTRY_H(getPage,			"(page: int or string): PropertyGridPage"),
			FUNC_ENTRY_H(getPageName,		"(index: int): string"),
			FUNC_ENTRY_H(getPageRoot,		"(index: int): PGProperty"),

			FUNC_ENTRY_H(isPageModified,	"(index: int): bool"),

			FUNC_ENTRY_H(selectPage,		"(page: int or string or PropertyGridPage)"),

			FUNC_ENTRY_H(showHeader,		"(show=true)"),
			FUNC_ENTRY_H(setColumnTitle,	"(column: int, title: string)"),
			FUNC_ENTRY_H(setDescription,	"(label, content: string)"),

			FUNC_ENTRY_H(setSplitterLeft,	"(subProps=false, allPages=true)"),

			FUNC_ENTRY_H(ensureVisible,		"(prop or name): bool"),
			FUNC_ENTRY_H(selectProperty,	"(prop or name, focus=false): bool"),
			NULL
		};

		bind(v, props, funcs);

		NB_WxPropertyGridInterface::Register<type>(v);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "DEFAULT",				(int)wxPGMAN_DEFAULT_STYLE);
		newSlot(v, -1, "TOOLBAR",				(int)wxPG_TOOLBAR);
		newSlot(v, -1, "DESCRIPTION",			(int)wxPG_DESCRIPTION);
		newSlot(v, -1, "NO_INTERNAL_BORDER",	(int)wxPG_NO_INTERNAL_BORDER);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxWindow* wnd = new wxPropertyGridManager(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			*opt<wxPoint>(v, 4, wxDefaultPosition),
			*opt<wxSize>(v, 5, wxDefaultSize),
			optInt(v, 6, wxPGMAN_DEFAULT_STYLE),
			optWxString(v, 7, wxPropertyGridManagerNameStr)
			);

		sq_setinstanceup(v, 1, wnd);
		return 0; 
	}

	NB_PROP_GET(grid)					{ return push(v, self(v)->GetGrid()); }
	NB_PROP_GET(columnCount)			{ return push(v, self(v)->GetColumnCount()); }
	NB_PROP_GET(descBoxHeight)			{ return push(v, self(v)->GetDescBoxHeight()); }
	NB_PROP_GET(currentPage)			{ return push(v, self(v)->GetCurrentPage()); }
	NB_PROP_GET(pageCount)				{ return push(v, self(v)->GetPageCount()); }
	NB_PROP_GET(selectedPage)			{ return push(v, self(v)->GetSelectedPage()); }
	NB_PROP_GET(toolBar)				{ return push(v, self(v)->GetToolBar()); }
	NB_PROP_GET(selectedProperty)		{ return push(v, self(v)->GetSelectedProperty()); }
	NB_PROP_GET(anyModified)			{ return push(v, self(v)->IsAnyModified()); }

	NB_PROP_SET(columnCount)			{ self(v)->SetColumnCount(getInt(v, 2)); return 0; }
	NB_PROP_SET(descBoxHeight)			{ self(v)->SetDescBoxHeight(getInt(v, 2)); return 0; }

	NB_FUNC(enableCategories)			{ return push(v, self(v)->EnableCategories(getBool(v, 2))); }

	NB_FUNC(clearPage)					{ self(v)->ClearPage(getInt(v, 2)); return 0; }
	NB_FUNC(addPage)					{ return push(v, self(v)->AddPage(optWxString(v, 2, ""), *opt<wxBitmap>(v, 3, wxPG_NULL_BITMAP), opt<wxPropertyGridPage>(v, 4, NULL))); }
	NB_FUNC(insertPage)					{ return push(v, self(v)->InsertPage(getInt(v, 2), optWxString(v, 3, ""), *opt<wxBitmap>(v, 4, wxPG_NULL_BITMAP), opt<wxPropertyGridPage>(v, 5, NULL))); }
	NB_FUNC(getPageByName)				{ return push(v, self(v)->GetPageByName(getWxString(v, 2))); }

	NB_FUNC(getPage)
	{
		if (isInt(v, 2))
			return push(v, self(v)->GetPage(getInt(v, 2)));
		else
			return push(v, self(v)->GetPage(getWxString(v, 2)));
	}

	NB_FUNC(getPageName)				{ return push(v, self(v)->GetPageName(getInt(v, 2))); }
	NB_FUNC(getPageRoot)				{ return push(v, self(v)->GetPageRoot(getInt(v, 2))); }

	NB_FUNC(isPageModified)				{ return push(v, self(v)->IsPageModified(getInt(v, 2))); }

	NB_FUNC(selectPage)
	{
		if (isInt(v, 2))
			self(v)->SelectPage(getInt(v, 2));
		else if (isString(v, 2))
			self(v)->SelectPage(getWxString(v, 2));
		else
			self(v)->SelectPage(get<wxPropertyGridPage>(v, 2));
		return 0;
	}

	NB_FUNC(showHeader)					{ self(v)->ShowHeader(optBool(v, 2, true)); return 0; }
	NB_FUNC(setColumnTitle)				{ self(v)->SetColumnTitle(getInt(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(setDescription)				{ self(v)->SetDescription(getWxString(v, 2), getWxString(v, 3)); return 0; }

	NB_FUNC(setSplitterLeft)			{ self(v)->SetSplitterLeft(optBool(v, 2, false), optBool(v, 3, true)); return 0; }

	NB_FUNC(ensureVisible)				{ return push(v, self(v)->EnsureVisible(GetPropArg(v, 2))); }
	NB_FUNC(selectProperty)				{ return push(v, self(v)->SelectProperty(GetPropArg(v, 2), optBool(v, 3, false))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, PropertyGridPage, wxEvtHandler);

class NB_WxPropertyGridPage : TNitClass<wxPropertyGridPage>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(grid),
			PROP_ENTRY	(columnCount),
			PROP_ENTRY_R(index),
			PROP_ENTRY_R(toolId),
			PROP_ENTRY	(splitterPosition),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(enableCategories,		"(enable: bool): bool"),

			FUNC_ENTRY_H(fitColumns,			"(): Size"),
			FUNC_ENTRY_H(getSplitterPosition,	"(splitterIdx: int): int"),
			FUNC_ENTRY_H(setSplitterPosition,	"(newXpos: int, splitterIdx: int)"),
			NULL
		};

		bind(v, props, funcs);

		NB_WxPropertyGridInterface::Register<type>(v);
	}

	NB_PROP_GET(grid)					{ return push(v, self(v)->GetGrid()); }
	NB_PROP_GET(columnCount)			{ return push(v, self(v)->GetColumnCount()); }
	NB_PROP_GET(index)					{ return push(v, self(v)->GetIndex()); }
	NB_PROP_GET(toolId)					{ return push(v, self(v)->GetToolId()); }
	NB_PROP_GET(splitterPosition)		{ return push(v, self(v)->GetSplitterPosition()); }

	NB_PROP_SET(columnCount)			{ self(v)->SetColumnCount(getInt(v, 2)); return 0; }
	NB_PROP_SET(splitterPosition)		{ self(v)->SetSplitterPosition(getInt(v, 2)); return 0; }

	NB_FUNC(enableCategories)			{ return push(v, self(v)->EnableCategories(getBool(v, 2))); }

	NB_FUNC(fitColumns)					{ return push(v, self(v)->FitColumns()); }
	NB_FUNC(getSplitterPosition)		{ return push(v, self(v)->GetSplitterPosition(getInt(v, 2))); }
	NB_FUNC(setSplitterPosition)		{ self(v)->SetSplitterPosition(getInt(v, 2), getInt(v, 3)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, PGCell, wxObject);

class NB_WxPGCell : TNitClass<wxPGCell>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(font),
			PROP_ENTRY	(bitmap),
			PROP_ENTRY	(foreColor),
			PROP_ENTRY	(backColor),
			PROP_ENTRY_R(hasText),
			PROP_ENTRY	(text),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(mergeFrom,		"(src: wx.PGCell)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(font)					{ return push(v, self(v)->GetFont()); }
	NB_PROP_GET(bitmap)					{ return push(v, self(v)->GetBitmap()); }
	NB_PROP_GET(foreColor)				{ return PushWxColor(v, self(v)->GetFgCol()); }
	NB_PROP_GET(backColor)				{ return PushWxColor(v, self(v)->GetBgCol()); }
	NB_PROP_GET(hasText)				{ return push(v, self(v)->HasText()); }
	NB_PROP_GET(text)					{ return push(v, self(v)->GetText()); }

	NB_PROP_SET(font)					{ self(v)->SetFont(*get<wxFont>(v, 2)); return 0; }
	NB_PROP_SET(bitmap)					{ self(v)->SetBitmap(*get<wxBitmap>(v, 2)); return 0; }
	NB_PROP_SET(foreColor)				{ self(v)->SetFgCol(GetWxColor(v, 2)); return 0; }
	NB_PROP_SET(backColor)				{ self(v)->SetBgCol(GetWxColor(v, 2)); return 0; }
	NB_PROP_SET(text)					{ self(v)->SetText(getWxString(v, 2)); return 0; }

	NB_FUNC(mergeFrom)					{ self(v)->MergeFrom(*get<wxPGCell>(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, PGProperty, wxObject);

class NB_WxPGProperty : TNitClass<wxPGProperty>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(value),
			PROP_ENTRY	(editorClass),
			PROP_ENTRY	(defaultValue),
			PROP_ENTRY	(clientObject),
			PROP_ENTRY	(choiceSelection),
			PROP_ENTRY	(helpString),
			PROP_ENTRY	(label),
			PROP_ENTRY	(maxLength),
			PROP_ENTRY	(modified),
			PROP_ENTRY	(name),
			PROP_ENTRY	(parentalType),
			PROP_ENTRY_R(attributes),
			PROP_ENTRY_R(baseName),
			PROP_ENTRY_R(childCount),
			PROP_ENTRY_R(displayedString),
			PROP_ENTRY_R(flags),
			PROP_ENTRY_R(grid),
			PROP_ENTRY_R(gridIfDisplayed),
			PROP_ENTRY_R(indexInParent),
			PROP_ENTRY_R(lastVisibleSubItem),
			PROP_ENTRY_R(mainParent),
			PROP_ENTRY_R(parent),
			PROP_ENTRY_R(y),
			PROP_ENTRY	(enabled),
			PROP_ENTRY	(visible),
			PROP_ENTRY_R(category),
			PROP_ENTRY_R(aggregate),
			PROP_ENTRY_R(expanded),
			PROP_ENTRY_R(root),
			PROP_ENTRY_R(textEditable),
			PROP_ENTRY_R(valueUnspecified),
			PROP_ENTRY	(autoUnspecified),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(						"()"
			"\n"								"(label: string, name: string)"),
			FUNC_ENTRY_H(refreshChildren,		"()"),
			FUNC_ENTRY_H(addChoice,				"(label: string, value: int=null)"),
			FUNC_ENTRY_H(addPrivateChild,		"(child: wx.PGProperty)"),
			FUNC_ENTRY_H(appendChild,			"(child: wx.PGProperty): wx.PGProperty"),
			FUNC_ENTRY_H(areAllChildrenSpecified,	"(): bool"),
			FUNC_ENTRY_H(areChildrenComponents,		"(): bool"),
			FUNC_ENTRY_H(changeFlag,			"(flags: FLAGS, set: bool)"),
			FUNC_ENTRY_H(deleteChildren,		"()"),
			FUNC_ENTRY_H(deleteChoice,			"(index: int)"),
			FUNC_ENTRY_H(generateComposedValue, "(): string"),
			FUNC_ENTRY_H(getAttribute,			"(name: string): value"),
			FUNC_ENTRY_H(setAttribute,			"(name: string, value)"),
			FUNC_ENTRY_H(setBackColor,			"(color: Color, flags=wx.PropertyGrid.FLAG.RECURSE)"),
			FUNC_ENTRY_H(setTextColor,			"(color: Color, flags=wx.PropertyGrid.FLAG.RECURSE)"),
			FUNC_ENTRY_H(getCell,				"(column: int): wx.PGCell"),
			FUNC_ENTRY_H(createCell,			"(column: int): wx.PGCell // binds to GetOrCreateCell()"),
			FUNC_ENTRY_H(getPropertyByName,		"(name: string): wx.PGProperty"),
			FUNC_ENTRY_H(hasFlag,				"(flag: FLAGS): bool"),
			FUNC_ENTRY_H(hasVisibleChildren,	"(): bool"),
			FUNC_ENTRY_H(insertChild,			"(index: int, child: wx.PGProperty): wx.PGProperty"),
			FUNC_ENTRY_H(item,					"(i: int): wx.PGProperty"),
			FUNC_ENTRY_H(refreshEditor,			"()"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "FLAGS");
		newSlot(v, -1, "MODIFIED",				(int)wxPG_PROP_MODIFIED);
		newSlot(v, -1, "DISABLED",				(int)wxPG_PROP_DISABLED);
		newSlot(v, -1, "HIDDEN",				(int)wxPG_PROP_HIDDEN);
		newSlot(v, -1, "CUSTOMIMAGE",			(int)wxPG_PROP_CUSTOMIMAGE);
		newSlot(v, -1, "NOEDITOR",				(int)wxPG_PROP_NOEDITOR);
		newSlot(v, -1, "COLLAPSED",				(int)wxPG_PROP_COLLAPSED);
		newSlot(v, -1, "INVALID_VALUE",			(int)wxPG_PROP_INVALID_VALUE);
		newSlot(v, -1, "WAS_MODIFIED",			(int)wxPG_PROP_WAS_MODIFIED);
		newSlot(v, -1, "AGGREGATE",				(int)wxPG_PROP_AGGREGATE);
		newSlot(v, -1, "CHILDREN_ARE_COPIES",	(int)wxPG_PROP_CHILDREN_ARE_COPIES);
		newSlot(v, -1, "PROPERTY",				(int)wxPG_PROP_PROPERTY);
		newSlot(v, -1, "CATEGORY",				(int)wxPG_PROP_CATEGORY);
		newSlot(v, -1, "MISC_PARENT",			(int)wxPG_PROP_MISC_PARENT);
		newSlot(v, -1, "READONLY",				(int)wxPG_PROP_READONLY);
		newSlot(v, -1, "COMPOSED_VALUE",		(int)wxPG_PROP_COMPOSED_VALUE);
		newSlot(v, -1, "USES_COMMON_VALUE",		(int)wxPG_PROP_USES_COMMON_VALUE);
		newSlot(v, -1, "AUTO_UNSPECIFIED",		(int)wxPG_PROP_AUTO_UNSPECIFIED);
		newSlot(v, -1, "CLASS_SPECIFIC_1",		(int)wxPG_PROP_CLASS_SPECIFIC_1);
		newSlot(v, -1, "CLASS_SPECIFIC_2",		(int)wxPG_PROP_CLASS_SPECIFIC_2);
		newSlot(v, -1, "BEING_DELETED",			(int)wxPG_PROP_BEING_DELETED);
		sq_poptop(v);
	}

	NB_PROP_GET(value)					{ return PushWxVariant(v, self(v)->GetValue()); }
	NB_PROP_GET(editorClass)			{ return push(v, self(v)->GetEditorClass()->GetName()); }
	NB_PROP_GET(defaultValue)			{ return PushWxVariant(v, self(v)->GetDefaultValue()); }
	NB_PROP_GET(clientObject)			{ return PushWxScriptData(v, self(v)->GetClientObject()); }
	NB_PROP_GET(choiceSelection)		{ return push(v, self(v)->GetChoiceSelection()); }
	NB_PROP_GET(helpString)				{ return push(v, self(v)->GetHelpString()); }
	NB_PROP_GET(label)					{ return push(v, self(v)->GetLabel()); }
	NB_PROP_GET(maxLength)				{ return push(v, self(v)->GetMaxLength()); }
	NB_PROP_GET(modified)				{ return push(v, self(v)->HasFlag(wxPG_PROP_MODIFIED)); }
	NB_PROP_GET(name)					{ return push(v, self(v)->GetName()); }
	NB_PROP_GET(parentalType)			{ return push(v, self(v)->GetFlags() & (wxPG_PROP_CATEGORY | wxPG_PROP_AGGREGATE | wxPG_PROP_MISC_PARENT)); }
	NB_PROP_GET(baseName)				{ return push(v, self(v)->GetBaseName()); }
	NB_PROP_GET(childCount)				{ return push(v, self(v)->GetChildCount()); }
	NB_PROP_GET(displayedString)		{ return push(v, self(v)->GetDisplayedString()); }
	NB_PROP_GET(flags)					{ return push(v, self(v)->GetFlags()); }
	NB_PROP_GET(grid)					{ return push(v, self(v)->GetGrid()); }
	NB_PROP_GET(gridIfDisplayed)		{ return push(v, self(v)->GetGridIfDisplayed()); }
	NB_PROP_GET(indexInParent)			{ return push(v, self(v)->GetIndexInParent()); }
	NB_PROP_GET(lastVisibleSubItem)		{ return push(v, const_cast<wxPGProperty*>(self(v)->GetLastVisibleSubItem())); }
	NB_PROP_GET(mainParent)				{ return push(v, self(v)->GetMainParent()); }
	NB_PROP_GET(parent)					{ return push(v, self(v)->GetParent()); }
	NB_PROP_GET(y)						{ return push(v, self(v)->GetY()); }
	NB_PROP_GET(enabled)				{ return push(v, self(v)->IsEnabled()); }
	NB_PROP_GET(visible)				{ return push(v, self(v)->IsVisible()); }
	NB_PROP_GET(category)				{ return push(v, self(v)->IsCategory()); }
	NB_PROP_GET(aggregate)				{ return push(v, self(v)->HasFlag(wxPG_PROP_AGGREGATE)); }
	NB_PROP_GET(expanded)				{ return push(v, self(v)->IsExpanded()); }
	NB_PROP_GET(root)					{ return push(v, self(v)->IsRoot()); }
	NB_PROP_GET(textEditable)			{ return push(v, self(v)->IsTextEditable()); }
	NB_PROP_GET(valueUnspecified)		{ return push(v, self(v)->IsValueUnspecified()); }
	NB_PROP_GET(autoUnspecified)		{ return push(v, self(v)->UsesAutoUnspecified()); }

	NB_PROP_SET(value)					{ self(v)->SetValue(GetWxVariant(v, 2)); return 0; }
	NB_PROP_SET(editorClass)			{ self(v)->SetEditor(getWxString(v, 2)); return 0; }
	NB_PROP_SET(defaultValue)			{ wxVariant var = GetWxVariant(v, 2); self(v)->SetDefaultValue(var); return 0; }
	NB_PROP_SET(choiceSelection)		{ self(v)->SetChoiceSelection(getInt(v, 2)); return 0; }
	NB_PROP_SET(helpString)				{ self(v)->SetHelpString(getWxString(v, 2)); return 0; }
	NB_PROP_SET(label)					{ self(v)->SetLabel(getWxString(v, 2)); return 0; }
	NB_PROP_SET(maxLength)				{ self(v)->SetMaxLength(getInt(v, 2)); return 0; }
	NB_PROP_SET(modified)				{ self(v)->SetModifiedStatus(getBool(v, 2)); return 0; }
	NB_PROP_SET(name)					{ self(v)->SetName(getWxString(v, 2)); return 0; }
	NB_PROP_SET(parentalType)			{ self(v)->SetParentalType(getInt(v, 2)); return 0; }
	NB_PROP_SET(enabled)				{ self(v)->Enable(getBool(v, 2)); return 0; }
	NB_PROP_SET(visible)				{ self(v)->Hide(!getBool(v, 2)); return 0; }
	NB_PROP_SET(autoUnspecified)		{ self(v)->SetAutoUnspecified(getBool(v, 2)); return 0; }

	NB_PROP_GET(attributes)
	{ 
		sq_newtable(v);
		wxVariant attrs = self(v)->GetAttributesAsList();
		for (uint i=0; i < attrs.GetCount(); ++i)
		{
			wxVariant& attr = attrs[i];
			push(v, attr.GetName());
			PushWxVariant(v, attr);
			sq_newslot(v, -3, false);
		}

		return 1;
	}

	NB_PROP_SET(clientObject)			
	{ 
		type* o = self(v);
		wxClientData* obj = o->GetClientObject();
		wxClientData* nobj = GetWxScriptData(v, 2, obj);

		if (obj != nobj)
			o->SetClientObject(nobj);
		return 0;
	}

	NB_CONS()
	{ 
		if (isNone(v, 2))
			setSelf(v, new wxPGProperty());
		else
			setSelf(v, new wxPGProperty(getWxString(v, 2), getWxString(v, 3)));
		return SQ_OK;
	}

	NB_FUNC(refreshChildren)			{ self(v)->RefreshChildren(); return 0; }
	NB_FUNC(addChoice)					{ self(v)->AddChoice(getWxString(v, 2), optInt(v, 3, wxPG_INVALID_VALUE)); return 0; }
	NB_FUNC(addPrivateChild)			{ self(v)->AddPrivateChild(get<wxPGProperty>(v, 2)); return 0; }
	NB_FUNC(appendChild)				{ return push(v, self(v)->AppendChild(get<wxPGProperty>(v, 2))); }
	NB_FUNC(areAllChildrenSpecified)	{ return push(v, self(v)->AreAllChildrenSpecified()); }
	NB_FUNC(areChildrenComponents)		{ return push(v, self(v)->AreChildrenComponents()); }
	NB_FUNC(changeFlag)					{ self(v)->ChangeFlag((wxPGPropertyFlags)getInt(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC(deleteChildren)				{ self(v)->DeleteChildren(); return 0; }
	NB_FUNC(deleteChoice)				{ self(v)->DeleteChoice(getInt(v, 2)); return 0; }
	NB_FUNC(generateComposedValue)		{ return push(v, self(v)->GenerateComposedValue()); }
	NB_FUNC(getAttribute)				{ return PushWxVariant(v, self(v)->GetAttribute(getWxString(v, 2))); }
	NB_FUNC(setAttribute)				{ self(v)->SetAttribute(getWxString(v, 2), GetWxVariant(v, 3)); return 0; }
	NB_FUNC(setBackColor)				{ self(v)->SetBackgroundColour(GetWxColor(v, 2), optInt(v, 3, wxPG_RECURSE)); return 0; }
	NB_FUNC(setTextColor)				{ self(v)->SetTextColour(GetWxColor(v, 2), optInt(v, 3, wxPG_RECURSE)); return 0; }
	NB_FUNC(getCell)					{ return push(v, &self(v)->GetCell(getInt(v, 2))); }
	NB_FUNC(createCell)					{ return push(v, &self(v)->GetOrCreateCell(getInt(v, 2))); }
	NB_FUNC(getPropertyByName)			{ return push(v, self(v)->GetPropertyByName(getWxString(v, 2))); }
	NB_FUNC(hasFlag)					{ return push(v, self(v)->HasFlag((wxPGPropertyFlags)getInt(v, 2))); }
	NB_FUNC(hasVisibleChildren)			{ return push(v, self(v)->HasVisibleChildren()); }
	NB_FUNC(insertChild)				{ return push(v, self(v)->InsertChild(getInt(v, 2), get<wxPGProperty>(v, 3))); }
	NB_FUNC(item)						{ return push(v, self(v)->Item(getInt(v, 2))); }
	NB_FUNC(refreshEditor)				{ self(v)->RefreshEditor(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, PropertyCategory, wxPGProperty);

class NB_WxPropertyCategory : TNitClass<wxPropertyCategory>
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
			CONS_ENTRY_H(				"(label: string, name=null)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		wxPGProperty* p = new wxPropertyCategory(getWxString(v, 2), optWxString(v, 3, wxPG_LABEL));
		sq_setinstanceup(v, 1, p);
		return 0; 
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, BoolProperty, wxPGProperty);

class NB_WxBoolProperty : TNitClass<wxBoolProperty>
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
			CONS_ENTRY_H(				"(label: string, name: string=null, value=false)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		wxPGProperty* p = new wxBoolProperty(optWxString(v, 2, wxPG_LABEL), optWxString(v, 3, wxPG_LABEL), optBool(v, 4, false));
		sq_setinstanceup(v, 1, p);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, IntProperty, wxPGProperty);

class NB_WxIntProperty : TNitClass<wxIntProperty>
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
			CONS_ENTRY_H(				"(label: string, name: string=null, value=0)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		wxPGProperty* p = new wxIntProperty(optWxString(v, 2, wxPG_LABEL), optWxString(v, 3, wxPG_LABEL), optInt(v, 4, 0));
		sq_setinstanceup(v, 1, p);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, UIntProperty, wxPGProperty);

class NB_WxUIntProperty : TNitClass<wxUIntProperty>
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
			CONS_ENTRY_H(				"(label: string, name: string=null, value=0)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		wxPGProperty* p = new wxUIntProperty(optWxString(v, 2, wxPG_LABEL), optWxString(v, 3, wxPG_LABEL), optInt(v, 4, 0));
		sq_setinstanceup(v, 1, p);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, EnumProperty, wxPGProperty);

class NB_WxEnumProperty : TNitClass<wxEnumProperty>
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
			CONS_ENTRY_H(				"(label: string, name: string=null, labels: string[], values: int[]=null, value=0)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		wxArrayString labels;
		int numItems = GetWxArrayString(v, 4, labels);
		if (numItems <= 0)
			return sq_throwerror(v, "invalid labels");

		wxArrayInt values;
		numItems = GetWxArrayInt(v, 5, values);
		if (numItems < 0)
			return sq_throwerror(v, "invalid values");

		if (values.size() && labels.size() != values.size())
			return sq_throwerror(v, "labels.len() != values.len()");
		
		wxPGProperty* p = new wxEnumProperty(optWxString(v, 2, wxPG_LABEL), optWxString(v, 3, wxPG_LABEL), labels, values, optInt(v, 6, 0));
		sq_setinstanceup(v, 1, p);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, EditEnumProperty, wxEnumProperty);

class NB_WxEditEnumProperty : TNitClass<wxEditEnumProperty>
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
			CONS_ENTRY_H(				"(label: string, name: string=null, labels: string[]=null, values: int[]=null, value=\"\")"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		wxArrayString labels;
		int numItems = GetWxArrayString(v, 4, labels);
		if (numItems < 0)
			return sq_throwerror(v, "invalid labels");

		wxArrayInt values;
		numItems = GetWxArrayInt(v, 5, values);
		if (numItems < 0)
			return sq_throwerror(v, "invalid values");

		wxPGProperty* p = new wxEditEnumProperty(optWxString(v, 2, wxPG_LABEL), optWxString(v, 3, wxPG_LABEL), labels, values, optWxString(v, 6, ""));
		sq_setinstanceup(v, 1, p);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, FlagsProperty, wxPGProperty);

class NB_WxFlagsProperty : TNitClass<wxFlagsProperty>
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
			CONS_ENTRY_H(				"(label: string, name: string=null, labels: string[]=null, values: int[]=null, value=\"\")"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		wxArrayString labels;
		int numItems = GetWxArrayString(v, 4, labels);
		if (numItems < 0)
			return sq_throwerror(v, "invalid labels");

		wxArrayInt values;
		numItems = GetWxArrayInt(v, 5, values);
		if (numItems < 0)
			return sq_throwerror(v, "invalid values");

		if (values.size() && labels.size() != values.size())
			return sq_throwerror(v, "labels.len() != values.len()");

		int value = optInt(v, 6, 0);

		wxPGProperty* p = new wxFlagsProperty(optWxString(v, 2, wxPG_LABEL), optWxString(v, 3, wxPG_LABEL), labels, values, value);
		sq_setinstanceup(v, 1, p);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, FloatProperty, wxPGProperty);

class NB_WxFloatProperty : TNitClass<wxFloatProperty>
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
			CONS_ENTRY_H(				"(label: string, name: string=null, value=0.0)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		wxPGProperty* p = new wxFloatProperty(optWxString(v, 2, wxPG_LABEL), optWxString(v, 3, wxPG_LABEL), optFloat(v, 4, 0.0f));
		sq_setinstanceup(v, 1, p);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, StringProperty, wxPGProperty);

class NB_WxStringProperty : TNitClass<wxStringProperty>
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
			CONS_ENTRY_H(				"(label: string, name: string=null, value=\"\")"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		wxPGProperty* p = new wxStringProperty(optWxString(v, 2, wxPG_LABEL), optWxString(v, 3, wxPG_LABEL), optWxString(v, 4, ""));
		sq_setinstanceup(v, 1, p);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, LongStringProperty, wxPGProperty);

class NB_WxLongStringProperty : TNitClass<wxLongStringProperty>
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
			CONS_ENTRY_H(				"(label: string, name: string=null, value=\"\")"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		wxPGProperty* p = new wxLongStringProperty(optWxString(v, 2, wxPG_LABEL), optWxString(v, 3, wxPG_LABEL), optWxString(v, 4, ""));
		sq_setinstanceup(v, 1, p);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, DirProperty, wxPGProperty);

class NB_WxDirProperty : TNitClass<wxDirProperty>
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
			CONS_ENTRY_H(				"(label: string, name: string=null, value=\"\")"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		wxPGProperty* p = new wxDirProperty(optWxString(v, 2, wxPG_LABEL), optWxString(v, 3, wxPG_LABEL), optWxString(v, 4, ""));
		sq_setinstanceup(v, 1, p);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, FileProperty, wxPGProperty);

class NB_WxFileProperty : TNitClass<wxFileProperty>
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
			CONS_ENTRY_H(				"(label: string, name: string=null, value=\"\")"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		wxPGProperty* p = new wxFileProperty(optWxString(v, 2, wxPG_LABEL), optWxString(v, 3, wxPG_LABEL), optWxString(v, 4, ""));
		sq_setinstanceup(v, 1, p);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, ArrayStringProperty, wxPGProperty);

class NB_WxArrayStringProperty : TNitClass<wxArrayStringProperty>
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
			CONS_ENTRY_H(				"(label: string, name: string=null, value: string[]=null)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		wxArrayString value;
		if (GetWxArrayString(v, 4, value) < 0)
			return sq_throwerror(v, "invalid value");

		wxPGProperty* p = new wxArrayStringProperty(optWxString(v, 2, wxPG_LABEL), optWxString(v, 3, wxPG_LABEL), value);
		sq_setinstanceup(v, 1, p);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, MultiChoiceProperty, wxPGProperty);

class NB_WxMultiChoiceProperty : TNitClass<wxMultiChoiceProperty>
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
			CONS_ENTRY_H(				"(label: string, name: string=null, strings: string[], values: strings[])"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		wxArrayString strings;
		int numItems = GetWxArrayString(v, 4, strings);
		if (numItems <= 0)
			return sq_throwerror(v, "invalid labels");

		wxArrayString values;
		numItems = GetWxArrayString(v, 5, values);
		if (numItems < 0)
			return sq_throwerror(v, "invalid values");

		wxPGProperty* p = new wxMultiChoiceProperty(optWxString(v, 2, wxPG_LABEL), optWxString(v, 3, wxPG_LABEL), strings, values);
		sq_setinstanceup(v, 1, p);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

// rename
typedef wxColourProperty wxColorProperty;

NB_TYPE_WXOBJ(NITWX_API, ColorProperty, wxPGProperty);

class NB_WxColorProperty : TNitClass<wxColourProperty>
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
  			CONS_ENTRY_H(				"(label: string, name: string=null, value=Color.White)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		wxPGProperty* p = new wxColourProperty(optWxString(v, 2, wxPG_LABEL), optWxString(v, 3, wxPG_LABEL), OptWxColor(v, 4, *wxWHITE));
		sq_setinstanceup(v, 1, p);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, PropertyGridEvent, wxCommandEvent);

class NB_WxPropertyGridEvent : TNitClass<wxPropertyGridEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(canVeto),
			PROP_ENTRY_R(column),
			PROP_ENTRY_R(mainParent),
			PROP_ENTRY_R(prop),
			PROP_ENTRY_R(validationFailureBehavior),
			PROP_ENTRY_R(propName),
			PROP_ENTRY_R(propValue),
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
	NB_PROP_GET(column)					{ return push(v, self(v)->GetColumn()); }
	NB_PROP_GET(mainParent)				{ return push(v, self(v)->GetMainParent()); }
	NB_PROP_GET(prop)					{ return push(v, self(v)->GetProperty()); }
	NB_PROP_GET(validationFailureBehavior)		{ return push(v, (int)self(v)->GetValidationFailureBehavior()); }
	NB_PROP_GET(propName)				{ return push(v, self(v)->GetPropertyName()); }
	NB_PROP_GET(propValue)				{ return PushWxVariant(v, self(v)->GetPropertyValue()); }

	NB_FUNC(veto)						{ self(v)->Veto(optBool(v, 2, true)); return 0; }
};


////////////////////////////////////////////////////////////////////////////////

SQRESULT NitLibWxPropGrid(HSQUIRRELVM v)
{
	NB_WxPropertyGrid::Register(v);
	NB_WxPropertyGridManager::Register(v);
	NB_WxPropertyGridPage::Register(v);

	NB_WxPGCell::Register(v);

	NB_WxPGProperty::Register(v);
	NB_WxPropertyCategory::Register(v);
	NB_WxBoolProperty::Register(v);
	NB_WxIntProperty::Register(v);
	NB_WxUIntProperty::Register(v);
	NB_WxEnumProperty::Register(v);
	NB_WxFlagsProperty::Register(v);
	NB_WxEditEnumProperty::Register(v);
	NB_WxFloatProperty::Register(v);
	NB_WxStringProperty::Register(v);
	NB_WxLongStringProperty::Register(v);
	NB_WxDirProperty::Register(v);
	NB_WxFileProperty::Register(v);
	NB_WxArrayStringProperty::Register(v);
	NB_WxMultiChoiceProperty::Register(v);
	NB_WxColorProperty::Register(v);

	NB_WxPropertyGridEvent::Register(v);

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
