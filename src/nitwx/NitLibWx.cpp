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
#include "nitwx/NitLibWxNitApp.h"

#include "nit/content/Image.h"

#include "squirrel/sqstdstring.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class ScriptWxWeakTracker::TrackerNode : public wxTrackerNode
{
public:
	TrackerNode(ScriptWxWeakTracker* tracker, wxTrackable* target, void* basePtr)
		: _tracker(tracker), _target(target), _basePtr(basePtr)
	{
		_target->AddNode(this);
	}

	~TrackerNode()
	{
		if (_target)
			_target->RemoveNode(this);
	}

	virtual void OnObjectDestroy()
	{
		wxTrackable* target = _target;
		_target = NULL; // To be safe with ~wxTrackable() -> ~TrackerNode chain
		_tracker->onDestroy(target, _basePtr);
	}

	ScriptWxWeakTracker* _tracker;
	wxTrackable* _target;
	void* _basePtr;
};

ScriptWxWeakTracker::ScriptWxWeakTracker(ScriptRuntime* env) : _env(env)
{
}

ScriptWxWeakTracker::~ScriptWxWeakTracker()
{
	clear();
}

void ScriptWxWeakTracker::add( wxTrackable* trackable, void* object )
{
	assert(_targets.find(trackable) == _targets.end());

	TrackerNode* tracker = new TrackerNode(this, trackable, object);
	_targets.insert(std::make_pair(trackable, tracker));
}

void ScriptWxWeakTracker::release( wxTrackable* trackable, void* object )
{
	Targets::iterator itr = _targets.find(trackable);
	assert(itr != _targets.end());

	TrackerNode* tracker = itr->second;
	delete tracker;
	_targets.erase(itr);
}

void ScriptWxWeakTracker::clear()
{
	for (Targets::iterator itr = _targets.begin(), end = _targets.end(); itr != end; ++itr)
	{
		TrackerNode* tracker = itr->second;
		delete tracker;
	}
	_targets.clear();
}

void ScriptWxWeakTracker::onDestroy( wxTrackable* object, void* basePtr )
{
	if (!_env->isStarted()) return;

	// Type information has already gone because onDestroy() called by ~wxTrackable() destructor.
	LOG(0, ".. script bound wx.weak 0x%08x destroyed.\n", basePtr);

	HSQUIRRELVM v = _env->getRoot();

	if (SQ_SUCCEEDED(NitBindImpl::getCachedInstance(v, basePtr, NULL)))
	{
		sq_purgeinstance(v, -1); 
		sq_poptop(v);
	}
}

////////////////////////////////////////////////////////////////////////////////

static void PurgePeer(ScriptPeer* peer)
{
	if (peer == NULL) return;

	HSQUIRRELVM v = peer->getWorker();

	if (v == NULL) return;

	peer->pushObject(v);
	sq_purgeinstance(v, -1);

	sq_poptop(v);
}

wxScriptFrame::wxScriptFrame(ScriptPeer* peer) : _peer(peer)
{
	Init();
}

wxScriptFrame::~wxScriptFrame()
{
	PurgePeer(_peer);
}

wxScriptDialog::wxScriptDialog(ScriptPeer* peer) : _peer(peer)
{
//	Init(); // TODO: check if needed win32 and mac
}

wxScriptDialog::~wxScriptDialog()
{
	PurgePeer(_peer);
}

wxScriptPanel::wxScriptPanel(ScriptPeer* peer) : _peer(peer)
{
}

wxScriptPanel::~wxScriptPanel()
{
	PurgePeer(_peer);
}

wxScriptWizard::wxScriptWizard(ScriptPeer* peer) : _peer(peer)
{
	Init();
}

wxScriptWizard::~wxScriptWizard()
{
	PurgePeer(_peer);
}

wxScriptWizardPage::wxScriptWizardPage(ScriptPeer* peer)
{
	Init();
}

wxScriptWizardPage::~wxScriptWizardPage()
{
	PurgePeer(_peer);
}

////////////////////////////////////////////////////////////////////////////////

wxBitmap wxScriptWizardPage::GetBitmap() const
{
	return wxWizardPage::GetBitmap();
}

wxWizardPage* wxScriptWizardPage::GetNext() const
{
	if (_peer == NULL) return NULL;

	HSQUIRRELVM v = _peer->getWorker();

	ScriptResult sr = _peer->callMethod("GetNext", 0, true);

	if (sr != SCRIPT_CALL_OK) return NULL;

	wxWizardPage* page = NitBind::get<wxWizardPage>(v, -1);
	sq_poptop(v);

	return page;
}

wxWizardPage* wxScriptWizardPage::GetPrev() const
{
	if (_peer == NULL) return NULL;

	HSQUIRRELVM v = _peer->getWorker();

	ScriptResult sr = _peer->callMethod("GetPrev", 0, true);

	if (sr != SCRIPT_CALL_OK) return NULL;

	wxWizardPage* page = NitBind::get<wxWizardPage>(v, -1);
	sq_poptop(v);

	return page;
}

////////////////////////////////////////////////////////////////////////////////

wxFileOffset wxNitInputStream::OnSysSeek(wxFileOffset seek, wxSeekMode mode)
{
	if (!_reader->isSeekable()) 
		return wxInvalidOffset;

	switch (mode)
	{
	case wxFromStart:				_reader->seek((size_t)seek); return _reader->tell();
	case wxFromCurrent:				_reader->skip((int)seek); return _reader->tell();
	case wxFromEnd:					
		if (_reader->isSized())
		{
			_reader->seek(_reader->getSize() - (size_t)seek); 
			return _reader->tell();
		}
	}

	return wxInvalidOffset;
}

wxFileOffset wxNitInputStream::OnSysTell() const
{
	return _reader->isSeekable() ? _reader->tell() : wxInvalidOffset;
}

////////////////////////////////////////////////////////////////////////////////

wxFileOffset wxNitOutputStream::OnSysSeek(wxFileOffset seek, wxSeekMode mode)
{
	if (!_writer->isSeekable()) 
		return wxInvalidOffset;

	switch (mode)
	{
	case wxFromStart:				_writer->seek((size_t)seek); return _writer->tell();
	case wxFromCurrent:				_writer->skip((int)seek); return _writer->tell();
	case wxFromEnd:					
		if (_writer->isSized())
		{
			_writer->seek(_writer->getSize() - (size_t)seek); 
			return _writer->tell();
		}
	}

	return wxInvalidOffset;
}

wxFileOffset wxNitOutputStream::OnSysTell() const
{
	return _writer->isSeekable() ? _writer->tell() : wxInvalidOffset;
}

////////////////////////////////////////////////////////////////////////////////

wxImage wxLoadNitImage(Ref<StreamSource> source, wxBitmapType type, int index)
{
	// TODO: support ntex format - add a wxImageHandler impl
	wxNitInputStream strm(source->open());
	return wxImage(strm, type, index);
}

////////////////////////////////////////////////////////////////////////////////

static void RegisterWS(HSQUIRRELVM v)
{
	sq_pushstring(v, "WS", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "VSCROLL",					(int)wxVSCROLL);
	NitBind::newSlot(v, -1, "HSCROLL",					(int)wxHSCROLL);
	NitBind::newSlot(v, -1, "CAPTION",					(int)wxCAPTION);
	NitBind::newSlot(v, -1, "ALWAYS_SHOW_SB",			(int)wxALWAYS_SHOW_SB);
	NitBind::newSlot(v, -1, "CLIP_CHILDREN",			(int)wxCLIP_CHILDREN);
	NitBind::newSlot(v, -1, "CLIP_SIBLINGS",			(int)wxCLIP_SIBLINGS);
	NitBind::newSlot(v, -1, "TRANSPARENT",				(int)wxTRANSPARENT);
	NitBind::newSlot(v, -1, "TAB_TRAVERSAL",			(int)wxTAB_TRAVERSAL);
	NitBind::newSlot(v, -1, "WANTS_CHARS",				(int)wxWANTS_CHARS);
	NitBind::newSlot(v, -1, "RETAINED",					(int)wxRETAINED);
	NitBind::newSlot(v, -1, "BACKINGSTORE",				(int)wxBACKINGSTORE);
	NitBind::newSlot(v, -1, "POPUP",					(int)wxPOPUP_WINDOW);
	NitBind::newSlot(v, -1, "FULL_REPAINT_ON_RESIZE",	(int)wxFULL_REPAINT_ON_RESIZE);
	NitBind::newSlot(v, -1, "MASK",						(int)wxWINDOW_STYLE_MASK);

	NitBind::newSlot(v, -1, "EX_VALIDATE_RECURSIVELY",	(int)wxWS_EX_VALIDATE_RECURSIVELY);
	NitBind::newSlot(v, -1, "EX_BLOCK_EVENTS",			(int)wxWS_EX_BLOCK_EVENTS);
	NitBind::newSlot(v, -1, "EX_TRANSIENT",				(int)wxWS_EX_TRANSIENT);
	NitBind::newSlot(v, -1, "EX_THEMED_BACKGROUND",		(int)wxWS_EX_THEMED_BACKGROUND);
	NitBind::newSlot(v, -1, "EX_PROCESS_IDLE",			(int)wxWS_EX_PROCESS_IDLE);
	NitBind::newSlot(v, -1, "EX_PROCESS_UI_UPDATES",	(int)wxWS_EX_PROCESS_UI_UPDATES);
	NitBind::newSlot(v, -1, "EX_CONTEXTHELP",			(int)wxWS_EX_CONTEXTHELP);

	NitBind::newSlot(v, -1, "FRAME_EX_METAL",			(int)wxFRAME_EX_METAL);
	NitBind::newSlot(v, -1, "FRAME_DEFAULT",			(int)wxDEFAULT_FRAME_STYLE);
	NitBind::newSlot(v, -1, "FRAME_EX_CONTEXTHELP",		(int)wxFRAME_EX_CONTEXTHELP);
	NitBind::newSlot(v, -1, "FRAME_DRAWER",				(int)wxFRAME_DRAWER);
	NitBind::newSlot(v, -1, "FRAME_NO_WINDOW_MENU",		(int)wxFRAME_NO_WINDOW_MENU);

	NitBind::newSlot(v, -1, "DIALOG_EX_METAL",			(int)wxDIALOG_EX_METAL);
	NitBind::newSlot(v, -1, "DIALOG_EX_CONTEXTHELP",	(int)wxDIALOG_EX_CONTEXTHELP);

	NitBind::newSlot(v, -1, "MB_DOCKABLE",				(int)wxMB_DOCKABLE);
	NitBind::newSlot(v, -1, "MENU_TEAROFF",				(int)wxMENU_TEAROFF);
	NitBind::newSlot(v, -1, "COLOURED",					(int)wxCOLOURED);
	NitBind::newSlot(v, -1, "FIXED_LENGTH",				(int)wxFIXED_LENGTH);

	NitBind::newSlot(v, -1, "SB_HORIZONTAL",			(int)wxSB_HORIZONTAL);
	NitBind::newSlot(v, -1, "SB_VERTICAL",				(int)wxSB_VERTICAL);
	NitBind::newSlot(v, -1, "SB_NORMAL",				(int)wxSB_NORMAL);
	NitBind::newSlot(v, -1, "SB_FLAT",					(int)wxSB_FLAT);
	NitBind::newSlot(v, -1, "SB_RAISED",				(int)wxSB_RAISED);
	NitBind::newSlot(v, -1, "SP_HORIZONTAL",			(int)wxSP_HORIZONTAL);
	NitBind::newSlot(v, -1, "SP_VERTICAL",				(int)wxSP_VERTICAL);
	NitBind::newSlot(v, -1, "SP_ARROW_KEYS",			(int)wxSP_ARROW_KEYS);
	NitBind::newSlot(v, -1, "SP_WRAP",					(int)wxSP_WRAP);

	NitBind::newSlot(v, -1, "TC_RIGHTJUSTIFY",			(int)wxTC_RIGHTJUSTIFY);
	NitBind::newSlot(v, -1, "TC_FIXEDWIDTH",			(int)wxTC_FIXEDWIDTH);
	NitBind::newSlot(v, -1, "TC_TOP",					(int)wxTC_TOP);
	NitBind::newSlot(v, -1, "TC_LEFT",					(int)wxTC_LEFT);
	NitBind::newSlot(v, -1, "TC_RIGHT",					(int)wxTC_RIGHT);
	NitBind::newSlot(v, -1, "TC_BOTTOM",				(int)wxTC_BOTTOM);
	NitBind::newSlot(v, -1, "TC_MULTILINE",				(int)wxTC_MULTILINE);
	NitBind::newSlot(v, -1, "TC_OWNERDRAW",				(int)wxTC_OWNERDRAW);

	NitBind::newSlot(v, -1, "BI_EXPAND",				(int)wxBI_EXPAND);

	NitBind::newSlot(v, -1, "LI_HORIZONTAL",			(int)wxLI_HORIZONTAL);
	NitBind::newSlot(v, -1, "LI_VERTICAL",				(int)wxLI_VERTICAL);
	sq_newslot(v, -3, false);
}

static void RegisterEVT(HSQUIRRELVM v)
{
	sq_pushstring(v, "EVT", -1);
	sq_newtable(v);

	// shortcuts (only which defined in macro)
	NitBind::newSlot(v, -1, "SCROLLBAR",							(int)wxEVT_COMMAND_SCROLLBAR_UPDATED);
	NitBind::newSlot(v, -1, "VLBOX",								(int)wxEVT_COMMAND_VLBOX_SELECTED);

	// full event names (which are not specified in corresponding controls)
	NitBind::newSlot(v, -1, "COMMAND_LEFT_CLICK",					(int)wxEVT_COMMAND_LEFT_CLICK);
	NitBind::newSlot(v, -1, "COMMAND_LEFT_DCLICK",					(int)wxEVT_COMMAND_LEFT_DCLICK);
	NitBind::newSlot(v, -1, "COMMAND_RIGHT_CLICK",					(int)wxEVT_COMMAND_RIGHT_CLICK);
	NitBind::newSlot(v, -1, "COMMAND_RIGHT_DCLICK",					(int)wxEVT_COMMAND_RIGHT_DCLICK);
	NitBind::newSlot(v, -1, "COMMAND_SET_FOCUS",					(int)wxEVT_COMMAND_SET_FOCUS);
	NitBind::newSlot(v, -1, "COMMAND_KILL_FOCUS",					(int)wxEVT_COMMAND_KILL_FOCUS);
	NitBind::newSlot(v, -1, "COMMAND_ENTER",						(int)wxEVT_COMMAND_ENTER);

	NitBind::newSlot(v, -1, "COMMAND_TOOL_CLICKED",					(int)wxEVT_COMMAND_TOOL_CLICKED);
	NitBind::newSlot(v, -1, "COMMAND_TEXT_UPDATED",					(int)wxEVT_COMMAND_TEXT_UPDATED);

	NitBind::newSlot(v, -1, "MOUSE_MOTION",							(int)wxEVT_MOTION);
	NitBind::newSlot(v, -1, "MOUSE_ENTER",							(int)wxEVT_ENTER_WINDOW);
	NitBind::newSlot(v, -1, "MOUSE_LEAVE",							(int)wxEVT_LEAVE_WINDOW);
	NitBind::newSlot(v, -1, "MOUSE_WHEEL",							(int)wxEVT_MOUSEWHEEL);
	NitBind::newSlot(v, -1, "MOUSE_LEFT_DOWN",						(int)wxEVT_LEFT_DOWN);
	NitBind::newSlot(v, -1, "MOUSE_LEFT_UP",						(int)wxEVT_LEFT_UP);
	NitBind::newSlot(v, -1, "MOUSE_LEFT_DCLICK",					(int)wxEVT_LEFT_DCLICK);
	NitBind::newSlot(v, -1, "MOUSE_MIDDLE_DOWN",					(int)wxEVT_MIDDLE_DOWN);
	NitBind::newSlot(v, -1, "MOUSE_MIDDLE_UP",						(int)wxEVT_MIDDLE_UP);
	NitBind::newSlot(v, -1, "MOUSE_MIDDLE_DCLICK",					(int)wxEVT_MIDDLE_DCLICK);
	NitBind::newSlot(v, -1, "MOUSE_RIGHT_DOWN",						(int)wxEVT_RIGHT_DOWN);
	NitBind::newSlot(v, -1, "MOUSE_RIGHT_UP",						(int)wxEVT_RIGHT_UP);
	NitBind::newSlot(v, -1, "MOUSE_RIGHT_DCLICK",					(int)wxEVT_RIGHT_DCLICK);
	NitBind::newSlot(v, -1, "MOUSE_AUX1_DOWN",						(int)wxEVT_AUX1_DOWN);
	NitBind::newSlot(v, -1, "MOUSE_AUX1_UP",						(int)wxEVT_AUX1_UP);
	NitBind::newSlot(v, -1, "MOUSE_AUX1_DCLICK",					(int)wxEVT_AUX1_DCLICK);
	NitBind::newSlot(v, -1, "MOUSE_AUX2_DOWN",						(int)wxEVT_AUX2_DOWN);
	NitBind::newSlot(v, -1, "MOUSE_AUX2_UP",						(int)wxEVT_AUX2_UP);
	NitBind::newSlot(v, -1, "MOUSE_AUX2_DCLICK",					(int)wxEVT_AUX2_DCLICK);

	NitBind::newSlot(v, -1, "SET_FOCUS",							(int)wxEVT_SET_FOCUS);
	NitBind::newSlot(v, -1, "KILL_FOCUS",							(int)wxEVT_KILL_FOCUS);
	NitBind::newSlot(v, -1, "CHILD_FOCUS",							(int)wxEVT_CHILD_FOCUS);

	NitBind::newSlot(v, -1, "CHAR",									(int)wxEVT_CHAR);
	NitBind::newSlot(v, -1, "CHAR_HOOK",							(int)wxEVT_CHAR_HOOK);
	NitBind::newSlot(v, -1, "NAVIGATION_KEY",						(int)wxEVT_NAVIGATION_KEY);
	NitBind::newSlot(v, -1, "KEY_DOWN",								(int)wxEVT_KEY_DOWN);
	NitBind::newSlot(v, -1, "KEY_UP",								(int)wxEVT_KEY_UP);
	NitBind::newSlot(v, -1, "HOTKEY",								(int)wxEVT_HOTKEY);

	NitBind::newSlot(v, -1, "SET_CURSOR",							(int)wxEVT_SET_CURSOR);

	NitBind::newSlot(v, -1, "SCROLL_TOP",							(int)wxEVT_SCROLL_TOP);
	NitBind::newSlot(v, -1, "SCROLL_BOTTOM",						(int)wxEVT_SCROLL_BOTTOM);
	NitBind::newSlot(v, -1, "SCROLL_LINEUP",						(int)wxEVT_SCROLL_LINEUP);
	NitBind::newSlot(v, -1, "SCROLL_LINEDOWN",						(int)wxEVT_SCROLL_LINEDOWN);
	NitBind::newSlot(v, -1, "SCROLL_PAGEUP",						(int)wxEVT_SCROLL_PAGEUP);
	NitBind::newSlot(v, -1, "SCROLL_PAGEDOWN",						(int)wxEVT_SCROLL_PAGEDOWN);
	NitBind::newSlot(v, -1, "SCROLL_THUMBTRACK",					(int)wxEVT_SCROLL_THUMBTRACK);
	NitBind::newSlot(v, -1, "SCROLL_THUMBRELEASE",					(int)wxEVT_SCROLL_THUMBRELEASE);
	NitBind::newSlot(v, -1, "SCROLL_CHANGED",						(int)wxEVT_SCROLL_CHANGED);

	NitBind::newSlot(v, -1, "SPIN_UP",								(int)wxEVT_SPIN_UP);
	NitBind::newSlot(v, -1, "SPIN_DOWN",							(int)wxEVT_SPIN_DOWN);
	NitBind::newSlot(v, -1, "SPIN",									(int)wxEVT_SPIN);

	NitBind::newSlot(v, -1, "SCROLLWIN_TOP",						(int)wxEVT_SCROLLWIN_TOP);
	NitBind::newSlot(v, -1, "SCROLLWIN_BOTTOM",						(int)wxEVT_SCROLLWIN_BOTTOM);
	NitBind::newSlot(v, -1, "SCROLLWIN_LINEUP",						(int)wxEVT_SCROLLWIN_LINEUP);
	NitBind::newSlot(v, -1, "SCROLLWIN_LINEDOWN",					(int)wxEVT_SCROLLWIN_LINEDOWN);
	NitBind::newSlot(v, -1, "SCROLLWIN_PAGEUP",						(int)wxEVT_SCROLLWIN_PAGEUP);
	NitBind::newSlot(v, -1, "SCROLLWIN_PAGEDOWN",					(int)wxEVT_SCROLLWIN_PAGEDOWN);
	NitBind::newSlot(v, -1, "SCROLLWIN_THUMBTRACK",					(int)wxEVT_SCROLLWIN_THUMBTRACK);
	NitBind::newSlot(v, -1, "SCROLLWIN_THUMBRELEASE",				(int)wxEVT_SCROLLWIN_THUMBRELEASE);

	NitBind::newSlot(v, -1, "ACTIVATE",								(int)wxEVT_ACTIVATE);
	NitBind::newSlot(v, -1, "CHILD_FOCUS",							(int)wxEVT_CHILD_FOCUS);
	NitBind::newSlot(v, -1, "CONTEXT_MENU",							(int)wxEVT_CONTEXT_MENU);
	NitBind::newSlot(v, -1, "HELP",									(int)wxEVT_HELP);
	NitBind::newSlot(v, -1, "DROP_FILES",							(int)wxEVT_DROP_FILES);
	NitBind::newSlot(v, -1, "ERASE_BACKGROUND",						(int)wxEVT_ERASE_BACKGROUND);
	NitBind::newSlot(v, -1, "SET_FOCUS",							(int)wxEVT_SET_FOCUS);
	NitBind::newSlot(v, -1, "KILL_FOCUS",							(int)wxEVT_KILL_FOCUS);
	NitBind::newSlot(v, -1, "IDLE",									(int)wxEVT_IDLE);
	NitBind::newSlot(v, -1, "KEY_DOWN",								(int)wxEVT_KEY_DOWN);
	NitBind::newSlot(v, -1, "KEY_UP",								(int)wxEVT_KEY_UP);
	NitBind::newSlot(v, -1, "CHAR",									(int)wxEVT_CHAR);
	NitBind::newSlot(v, -1, "MOUSE_CAPTURE_LOST",					(int)wxEVT_MOUSE_CAPTURE_LOST);
	NitBind::newSlot(v, -1, "MOUSE_CAPTURE_CHANGED",				(int)wxEVT_MOUSE_CAPTURE_CHANGED);
	NitBind::newSlot(v, -1, "PAINT",								(int)wxEVT_PAINT);
	NitBind::newSlot(v, -1, "SET_CURSOR",							(int)wxEVT_SET_CURSOR);
	NitBind::newSlot(v, -1, "SHOW",									(int)wxEVT_SHOW);
	NitBind::newSlot(v, -1, "SIZE",									(int)wxEVT_SIZE);
	NitBind::newSlot(v, -1, "SYS_COLOR_CHANGED",					(int)wxEVT_SYS_COLOUR_CHANGED);

	NitBind::newSlot(v, -1, "MOVE",									(int)wxEVT_MOVE);
	NitBind::newSlot(v, -1, "CREATE",								(int)wxEVT_CREATE);
	NitBind::newSlot(v, -1, "DESTROY",								(int)wxEVT_DESTROY);
	NitBind::newSlot(v, -1, "MAXIMIZE",								(int)wxEVT_MAXIMIZE);

	NitBind::newSlot(v, -1, "MOUSE_CAPTURE_CHANGED",				(int)wxEVT_MOUSE_CAPTURE_CHANGED);
	NitBind::newSlot(v, -1, "MOUSE_CAPTURE_LOST",					(int)wxEVT_MOUSE_CAPTURE_LOST);

	NitBind::newSlot(v, -1, "PAINT",								(int)wxEVT_PAINT);
	NitBind::newSlot(v, -1, "ERASE_BACKGROUND",						(int)wxEVT_ERASE_BACKGROUND);
	NitBind::newSlot(v, -1, "NC_PAINT",								(int)wxEVT_NC_PAINT);

	NitBind::newSlot(v, -1, "DISPLAY_CHANGED",						(int)wxEVT_DISPLAY_CHANGED);
	NitBind::newSlot(v, -1, "QUERY_NEW_PALETTE",					(int)wxEVT_QUERY_NEW_PALETTE);
	NitBind::newSlot(v, -1, "PALETTE_CHANGED",						(int)wxEVT_PALETTE_CHANGED);
	NitBind::newSlot(v, -1, "JOY_BUTTON_DOWN",						(int)wxEVT_JOY_BUTTON_DOWN);
	NitBind::newSlot(v, -1, "JOY_BUTTON_UP",						(int)wxEVT_JOY_BUTTON_UP);
	NitBind::newSlot(v, -1, "JOY_MOVE",								(int)wxEVT_JOY_MOVE);
	NitBind::newSlot(v, -1, "JOY_ZMOVE",							(int)wxEVT_JOY_ZMOVE);
	NitBind::newSlot(v, -1, "DROP_FILES",							(int)wxEVT_DROP_FILES);
	NitBind::newSlot(v, -1, "UPDATE_UI",							(int)wxEVT_UPDATE_UI);
	NitBind::newSlot(v, -1, "SIZING",								(int)wxEVT_SIZING);
	NitBind::newSlot(v, -1, "MOVING",								(int)wxEVT_MOVING);
	NitBind::newSlot(v, -1, "MOVE_START",							(int)wxEVT_MOVE_START);
	NitBind::newSlot(v, -1, "MOVE_END",								(int)wxEVT_MOVE_END);

	NitBind::newSlot(v, -1, "HELP",									(int)wxEVT_HELP);
	NitBind::newSlot(v, -1, "DETAILED_HELP",						(int)wxEVT_DETAILED_HELP);
	sq_newslot(v, -3, false);
}

static void RegisterID(HSQUIRRELVM v)
{
	sq_pushstring(v, "ID", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "NONE",						(int)wxID_NONE);
	NitBind::newSlot(v, -1, "SEPARATOR",				(int)wxID_SEPARATOR);
	NitBind::newSlot(v, -1, "ANY",						(int)wxID_ANY);

	NitBind::newSlot(v, -1, "OPEN",						(int)wxID_OPEN);
	NitBind::newSlot(v, -1, "CLOSE",					(int)wxID_CLOSE);
	NitBind::newSlot(v, -1, "NEW",						(int)wxID_NEW);
	NitBind::newSlot(v, -1, "SAVE",						(int)wxID_SAVE);
	NitBind::newSlot(v, -1, "SAVEAS",					(int)wxID_SAVEAS);
	NitBind::newSlot(v, -1, "REVERT",					(int)wxID_REVERT);
	NitBind::newSlot(v, -1, "EXIT",						(int)wxID_EXIT);
	NitBind::newSlot(v, -1, "UNDO",						(int)wxID_UNDO);
	NitBind::newSlot(v, -1, "REDO",						(int)wxID_REDO);
	NitBind::newSlot(v, -1, "HELP",						(int)wxID_HELP);
	NitBind::newSlot(v, -1, "PRINT",					(int)wxID_PRINT);
	NitBind::newSlot(v, -1, "PRINT_SETUP",				(int)wxID_PRINT_SETUP);
	NitBind::newSlot(v, -1, "PAGE_SETUP",				(int)wxID_PAGE_SETUP);
	NitBind::newSlot(v, -1, "PREVIEW",					(int)wxID_PREVIEW);
	NitBind::newSlot(v, -1, "ABOUT",					(int)wxID_ABOUT);
	NitBind::newSlot(v, -1, "HELP_CONTENTS",			(int)wxID_HELP_CONTENTS);
	NitBind::newSlot(v, -1, "HELP_INDEX",				(int)wxID_HELP_INDEX);
	NitBind::newSlot(v, -1, "HELP_SEARCH",				(int)wxID_HELP_SEARCH);
	NitBind::newSlot(v, -1, "HELP_COMMANDS",			(int)wxID_HELP_COMMANDS);
	NitBind::newSlot(v, -1, "HELP_PROCEDURES",			(int)wxID_HELP_PROCEDURES);
	NitBind::newSlot(v, -1, "HELP_CONTEXT",				(int)wxID_HELP_CONTEXT);
	NitBind::newSlot(v, -1, "CLOSE_ALL",				(int)wxID_CLOSE_ALL);

	NitBind::newSlot(v, -1, "EDIT",						(int)wxID_EDIT);
	NitBind::newSlot(v, -1, "CUT",						(int)wxID_CUT);
	NitBind::newSlot(v, -1, "COPY",						(int)wxID_COPY);
	NitBind::newSlot(v, -1, "PASTE",					(int)wxID_PASTE);
	NitBind::newSlot(v, -1, "CLEAR",					(int)wxID_CLEAR);
	NitBind::newSlot(v, -1, "FIND",						(int)wxID_FIND);
	NitBind::newSlot(v, -1, "DUPLICATE",				(int)wxID_DUPLICATE);
	NitBind::newSlot(v, -1, "SELECTALL",				(int)wxID_SELECTALL);
	NitBind::newSlot(v, -1, "DELETE",					(int)wxID_DELETE);
	NitBind::newSlot(v, -1, "REPLACE",					(int)wxID_REPLACE);
	NitBind::newSlot(v, -1, "REPLACE_ALL",				(int)wxID_REPLACE_ALL);
	NitBind::newSlot(v, -1, "PROPERTIES",				(int)wxID_PROPERTIES);

	NitBind::newSlot(v, -1, "VIEW_DETAILS",				(int)wxID_VIEW_DETAILS);
	NitBind::newSlot(v, -1, "VIEW_LARGEICONS",			(int)wxID_VIEW_LARGEICONS);
	NitBind::newSlot(v, -1, "VIEW_SMALLICONS",			(int)wxID_VIEW_SMALLICONS);
	NitBind::newSlot(v, -1, "VIEW_LIST",				(int)wxID_VIEW_LIST);
	NitBind::newSlot(v, -1, "VIEW_SORTDATE",			(int)wxID_VIEW_SORTDATE);
	NitBind::newSlot(v, -1, "VIEW_SORTNAME",			(int)wxID_VIEW_SORTNAME);
	NitBind::newSlot(v, -1, "VIEW_SORTSIZE",			(int)wxID_VIEW_SORTSIZE);
	NitBind::newSlot(v, -1, "VIEW_SORTTYPE",			(int)wxID_VIEW_SORTTYPE);

	NitBind::newSlot(v, -1, "FILE",						(int)wxID_FILE);
	NitBind::newSlot(v, -1, "FILE1",					(int)wxID_FILE1);
	NitBind::newSlot(v, -1, "FILE2",					(int)wxID_FILE2);
	NitBind::newSlot(v, -1, "FILE3",					(int)wxID_FILE3);
	NitBind::newSlot(v, -1, "FILE4",					(int)wxID_FILE4);
	NitBind::newSlot(v, -1, "FILE5",					(int)wxID_FILE5);
	NitBind::newSlot(v, -1, "FILE6",					(int)wxID_FILE6);
	NitBind::newSlot(v, -1, "FILE7",					(int)wxID_FILE7);
	NitBind::newSlot(v, -1, "FILE8",					(int)wxID_FILE8);
	NitBind::newSlot(v, -1, "FILE9",					(int)wxID_FILE9);

	NitBind::newSlot(v, -1, "OK",						(int)wxID_OK);
	NitBind::newSlot(v, -1, "CANCEL",					(int)wxID_CANCEL);
	NitBind::newSlot(v, -1, "APPLY",					(int)wxID_APPLY);
	NitBind::newSlot(v, -1, "YES",						(int)wxID_YES);
	NitBind::newSlot(v, -1, "NO",						(int)wxID_NO);
	NitBind::newSlot(v, -1, "STATIC",					(int)wxID_STATIC);
	NitBind::newSlot(v, -1, "FORWARD",					(int)wxID_FORWARD);
	NitBind::newSlot(v, -1, "BACKWARD",					(int)wxID_BACKWARD);
	NitBind::newSlot(v, -1, "DEFAULT",					(int)wxID_DEFAULT);
	NitBind::newSlot(v, -1, "MORE",						(int)wxID_MORE);
	NitBind::newSlot(v, -1, "SETUP",					(int)wxID_SETUP);
	NitBind::newSlot(v, -1, "RESET",					(int)wxID_RESET);
	NitBind::newSlot(v, -1, "CONTEXT_HELP",				(int)wxID_CONTEXT_HELP);
	NitBind::newSlot(v, -1, "YESTOALL",					(int)wxID_YESTOALL);
	NitBind::newSlot(v, -1, "NOTOALL",					(int)wxID_NOTOALL);
	NitBind::newSlot(v, -1, "ABORT",					(int)wxID_ABORT);
	NitBind::newSlot(v, -1, "RETRY",					(int)wxID_RETRY);
	NitBind::newSlot(v, -1, "IGNORE",					(int)wxID_IGNORE);
	NitBind::newSlot(v, -1, "ADD",						(int)wxID_ADD);
	NitBind::newSlot(v, -1, "REMOVE",					(int)wxID_REMOVE);

	NitBind::newSlot(v, -1, "UP",						(int)wxID_UP);
	NitBind::newSlot(v, -1, "DOWN",						(int)wxID_DOWN);
	NitBind::newSlot(v, -1, "HOME",						(int)wxID_HOME);
	NitBind::newSlot(v, -1, "REFRESH",					(int)wxID_REFRESH);
	NitBind::newSlot(v, -1, "STOP",						(int)wxID_STOP);
	NitBind::newSlot(v, -1, "INDEX",					(int)wxID_INDEX);

	NitBind::newSlot(v, -1, "BOLD",						(int)wxID_BOLD);
	NitBind::newSlot(v, -1, "ITALIC",					(int)wxID_ITALIC);
	NitBind::newSlot(v, -1, "JUSTIFY_CENTER",			(int)wxID_JUSTIFY_CENTER);
	NitBind::newSlot(v, -1, "JUSTIFY_FILL",				(int)wxID_JUSTIFY_FILL);
	NitBind::newSlot(v, -1, "JUSTIFY_RIGHT",			(int)wxID_JUSTIFY_RIGHT);
	NitBind::newSlot(v, -1, "JUSTIFY_LEFT",				(int)wxID_JUSTIFY_LEFT);
	NitBind::newSlot(v, -1, "UNDERLINE",				(int)wxID_UNDERLINE);
	NitBind::newSlot(v, -1, "INDENT",					(int)wxID_INDENT);
	NitBind::newSlot(v, -1, "UNINDENT",					(int)wxID_UNINDENT);
	NitBind::newSlot(v, -1, "ZOOM_100",					(int)wxID_ZOOM_100);
	NitBind::newSlot(v, -1, "ZOOM_FIT",					(int)wxID_ZOOM_FIT);
	NitBind::newSlot(v, -1, "ZOOM_IN",					(int)wxID_ZOOM_IN);
	NitBind::newSlot(v, -1, "ZOOM_OUT",					(int)wxID_ZOOM_OUT);
	NitBind::newSlot(v, -1, "UNDELETE",					(int)wxID_UNDELETE);
	NitBind::newSlot(v, -1, "REVERT_TO_SAVED",			(int)wxID_REVERT_TO_SAVED);
	NitBind::newSlot(v, -1, "CDROM",					(int)wxID_CDROM);
	NitBind::newSlot(v, -1, "CONVERT",					(int)wxID_CONVERT);
	NitBind::newSlot(v, -1, "EXECUTE",					(int)wxID_EXECUTE);
	NitBind::newSlot(v, -1, "FLOPPY",					(int)wxID_FLOPPY);
	NitBind::newSlot(v, -1, "HARDDISK",					(int)wxID_HARDDISK);
	NitBind::newSlot(v, -1, "BOTTOM",					(int)wxID_BOTTOM);
	NitBind::newSlot(v, -1, "FIRST",					(int)wxID_FIRST);
	NitBind::newSlot(v, -1, "LAST",						(int)wxID_LAST);
	NitBind::newSlot(v, -1, "TOP",						(int)wxID_TOP);
	NitBind::newSlot(v, -1, "INFO",						(int)wxID_INFO);
	NitBind::newSlot(v, -1, "JUMP_TO",					(int)wxID_JUMP_TO);
	NitBind::newSlot(v, -1, "NETWORK",					(int)wxID_NETWORK);
	NitBind::newSlot(v, -1, "SELECT_COLOR",				(int)wxID_SELECT_COLOR);
	NitBind::newSlot(v, -1, "SELECT_FONT",				(int)wxID_SELECT_FONT);
	NitBind::newSlot(v, -1, "SORT_ASCENDING",			(int)wxID_SORT_ASCENDING);
	NitBind::newSlot(v, -1, "SORT_DESCENDING",			(int)wxID_SORT_DESCENDING);
	NitBind::newSlot(v, -1, "SPELL_CHECK",				(int)wxID_SPELL_CHECK);
	NitBind::newSlot(v, -1, "STRIKETHROUGH",			(int)wxID_STRIKETHROUGH);

	NitBind::newSlot(v, -1, "SYSTEM_MENU",				(int)wxID_SYSTEM_MENU);
	NitBind::newSlot(v, -1, "CLOSE_FRAME",				(int)wxID_CLOSE_FRAME);
	NitBind::newSlot(v, -1, "MOVE_FRAME",				(int)wxID_MOVE_FRAME);
	NitBind::newSlot(v, -1, "RESIZE_FRAME",				(int)wxID_RESIZE_FRAME);
	NitBind::newSlot(v, -1, "MAXIMIZE_FRAME",			(int)wxID_MAXIMIZE_FRAME);
	NitBind::newSlot(v, -1, "ICONIZE_FRAME",			(int)wxID_ICONIZE_FRAME);
	NitBind::newSlot(v, -1, "RESTORE_FRAME",			(int)wxID_RESTORE_FRAME);

	NitBind::newSlot(v, -1, "MDI_WINDOW_FIRST",			(int)wxID_MDI_WINDOW_FIRST);
	NitBind::newSlot(v, -1, "MDI_WINDOW_CASCADE",		(int)wxID_MDI_WINDOW_CASCADE);
	NitBind::newSlot(v, -1, "MDI_WINDOW_TILE_HORZ",		(int)wxID_MDI_WINDOW_TILE_HORZ);
	NitBind::newSlot(v, -1, "MDI_WINDOW_TILE_VERT",		(int)wxID_MDI_WINDOW_TILE_VERT);
	NitBind::newSlot(v, -1, "MDI_WINDOW_ARRANGE_ICONS",	(int)wxID_MDI_WINDOW_ARRANGE_ICONS);
	NitBind::newSlot(v, -1, "MDI_WINDOW_PREV",			(int)wxID_MDI_WINDOW_PREV);
	NitBind::newSlot(v, -1, "MDI_WINDOW_NEXT",			(int)wxID_MDI_WINDOW_NEXT);
	NitBind::newSlot(v, -1, "MDI_WINDOW_LAST",			(int)wxID_MDI_WINDOW_LAST);

	NitBind::newSlot(v, -1, "FILEDLGG",					(int)wxID_FILEDLGG);

	NitBind::newSlot(v, -1, "FILECTRL",					(int)wxID_FILECTRL);

	NitBind::newSlot(v, -1, "LOWEST",					(int)wxID_LOWEST);
	NitBind::newSlot(v, -1, "HIGHEST",					(int)wxID_HIGHEST);

	NitBind::newSlot(v, -1, "AUTO_LOWEST",				(int)wxID_AUTO_LOWEST);
	NitBind::newSlot(v, -1, "AUTO_HIGHEST",				(int)wxID_AUTO_HIGHEST);

	sq_newslot(v, -3, false);
}

static void RegisterKEY(HSQUIRRELVM v)
{
	sq_pushstring(v, "KEY", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "BACK",					(int)WXK_BACK);
	NitBind::newSlot(v, -1, "TAB",					(int)WXK_TAB);
	NitBind::newSlot(v, -1, "RETURN",				(int)WXK_RETURN);
	NitBind::newSlot(v, -1, "ESCAPE",				(int)WXK_ESCAPE);
	NitBind::newSlot(v, -1, "SPACE",				(int)WXK_SPACE);
	NitBind::newSlot(v, -1, "DELETE",				(int)WXK_DELETE);
	NitBind::newSlot(v, -1, "START",				(int)WXK_START);
	NitBind::newSlot(v, -1, "LBUTTON",				(int)WXK_LBUTTON);
	NitBind::newSlot(v, -1, "RBUTTON",				(int)WXK_RBUTTON);
	NitBind::newSlot(v, -1, "CANCEL",				(int)WXK_CANCEL);
	NitBind::newSlot(v, -1, "MBUTTON",				(int)WXK_MBUTTON);
	NitBind::newSlot(v, -1, "CLEAR",				(int)WXK_CLEAR);
	NitBind::newSlot(v, -1, "SHIFT",				(int)WXK_SHIFT);
	NitBind::newSlot(v, -1, "ALT",					(int)WXK_ALT);
	NitBind::newSlot(v, -1, "CONTROL",				(int)WXK_CONTROL);
	NitBind::newSlot(v, -1, "MENU",					(int)WXK_MENU);
	NitBind::newSlot(v, -1, "PAUSE",				(int)WXK_PAUSE);
	NitBind::newSlot(v, -1, "CAPITAL",				(int)WXK_CAPITAL);
	NitBind::newSlot(v, -1, "END",					(int)WXK_END);
	NitBind::newSlot(v, -1, "HOME",					(int)WXK_HOME);
	NitBind::newSlot(v, -1, "LEFT",					(int)WXK_LEFT);
	NitBind::newSlot(v, -1, "UP",					(int)WXK_UP);
	NitBind::newSlot(v, -1, "RIGHT",				(int)WXK_RIGHT);
	NitBind::newSlot(v, -1, "DOWN",					(int)WXK_DOWN);
	NitBind::newSlot(v, -1, "SELECT",				(int)WXK_SELECT);
	NitBind::newSlot(v, -1, "PRINT",				(int)WXK_PRINT);
	NitBind::newSlot(v, -1, "EXECUTE",				(int)WXK_EXECUTE);
	NitBind::newSlot(v, -1, "SNAPSHOT",				(int)WXK_SNAPSHOT);
	NitBind::newSlot(v, -1, "INSERT",				(int)WXK_INSERT);
	NitBind::newSlot(v, -1, "HELP",					(int)WXK_HELP);
	NitBind::newSlot(v, -1, "NUMPAD0",				(int)WXK_NUMPAD0);
	NitBind::newSlot(v, -1, "NUMPAD1",				(int)WXK_NUMPAD1);
	NitBind::newSlot(v, -1, "NUMPAD2",				(int)WXK_NUMPAD2);
	NitBind::newSlot(v, -1, "NUMPAD3",				(int)WXK_NUMPAD3);
	NitBind::newSlot(v, -1, "NUMPAD4",				(int)WXK_NUMPAD4);
	NitBind::newSlot(v, -1, "NUMPAD5",				(int)WXK_NUMPAD5);
	NitBind::newSlot(v, -1, "NUMPAD6",				(int)WXK_NUMPAD6);
	NitBind::newSlot(v, -1, "NUMPAD7",				(int)WXK_NUMPAD7);
	NitBind::newSlot(v, -1, "NUMPAD8",				(int)WXK_NUMPAD8);
	NitBind::newSlot(v, -1, "NUMPAD9",				(int)WXK_NUMPAD9);
	NitBind::newSlot(v, -1, "MULTIPLY",				(int)WXK_MULTIPLY);
	NitBind::newSlot(v, -1, "ADD",					(int)WXK_ADD);
	NitBind::newSlot(v, -1, "SEPARATOR",			(int)WXK_SEPARATOR);
	NitBind::newSlot(v, -1, "SUBTRACT",				(int)WXK_SUBTRACT);
	NitBind::newSlot(v, -1, "DECIMAL",				(int)WXK_DECIMAL);
	NitBind::newSlot(v, -1, "DIVIDE",				(int)WXK_DIVIDE);
	NitBind::newSlot(v, -1, "F1",					(int)WXK_F1);
	NitBind::newSlot(v, -1, "F2",					(int)WXK_F2);
	NitBind::newSlot(v, -1, "F3",					(int)WXK_F3);
	NitBind::newSlot(v, -1, "F4",					(int)WXK_F4);
	NitBind::newSlot(v, -1, "F5",					(int)WXK_F5);
	NitBind::newSlot(v, -1, "F6",					(int)WXK_F6);
	NitBind::newSlot(v, -1, "F7",					(int)WXK_F7);
	NitBind::newSlot(v, -1, "F8",					(int)WXK_F8);
	NitBind::newSlot(v, -1, "F9",					(int)WXK_F9);
	NitBind::newSlot(v, -1, "F10",					(int)WXK_F10);
	NitBind::newSlot(v, -1, "F11",					(int)WXK_F11);
	NitBind::newSlot(v, -1, "F12",					(int)WXK_F12);
	NitBind::newSlot(v, -1, "F13",					(int)WXK_F13);
	NitBind::newSlot(v, -1, "F14",					(int)WXK_F14);
	NitBind::newSlot(v, -1, "F15",					(int)WXK_F15);
	NitBind::newSlot(v, -1, "F16",					(int)WXK_F16);
	NitBind::newSlot(v, -1, "F17",					(int)WXK_F17);
	NitBind::newSlot(v, -1, "F18",					(int)WXK_F18);
	NitBind::newSlot(v, -1, "F19",					(int)WXK_F19);
	NitBind::newSlot(v, -1, "F20",					(int)WXK_F20);
	NitBind::newSlot(v, -1, "F21",					(int)WXK_F21);
	NitBind::newSlot(v, -1, "F22",					(int)WXK_F22);
	NitBind::newSlot(v, -1, "F23",					(int)WXK_F23);
	NitBind::newSlot(v, -1, "F24",					(int)WXK_F24);
	NitBind::newSlot(v, -1, "NUMLOCK",				(int)WXK_NUMLOCK);
	NitBind::newSlot(v, -1, "SCROLL",				(int)WXK_SCROLL);
	NitBind::newSlot(v, -1, "PAGEUP",				(int)WXK_PAGEUP);
	NitBind::newSlot(v, -1, "PAGEDOWN",				(int)WXK_PAGEDOWN);
	NitBind::newSlot(v, -1, "NUMPAD_SPACE",			(int)WXK_NUMPAD_SPACE);
	NitBind::newSlot(v, -1, "NUMPAD_TAB",			(int)WXK_NUMPAD_TAB);
	NitBind::newSlot(v, -1, "NUMPAD_ENTER",			(int)WXK_NUMPAD_ENTER);
	NitBind::newSlot(v, -1, "NUMPAD_F1",			(int)WXK_NUMPAD_F1);
	NitBind::newSlot(v, -1, "NUMPAD_F2",			(int)WXK_NUMPAD_F2);
	NitBind::newSlot(v, -1, "NUMPAD_F3",			(int)WXK_NUMPAD_F3);
	NitBind::newSlot(v, -1, "NUMPAD_F4",			(int)WXK_NUMPAD_F4);
	NitBind::newSlot(v, -1, "NUMPAD_HOME",			(int)WXK_NUMPAD_HOME);
	NitBind::newSlot(v, -1, "NUMPAD_LEFT",			(int)WXK_NUMPAD_LEFT);
	NitBind::newSlot(v, -1, "NUMPAD_UP",			(int)WXK_NUMPAD_UP);
	NitBind::newSlot(v, -1, "NUMPAD_RIGHT",			(int)WXK_NUMPAD_RIGHT);
	NitBind::newSlot(v, -1, "NUMPAD_DOWN",			(int)WXK_NUMPAD_DOWN);
	NitBind::newSlot(v, -1, "NUMPAD_PAGEUP",		(int)WXK_NUMPAD_PAGEUP);
	NitBind::newSlot(v, -1, "NUMPAD_PAGEDOWN",		(int)WXK_NUMPAD_PAGEDOWN);
	NitBind::newSlot(v, -1, "NUMPAD_END",			(int)WXK_NUMPAD_END);
	NitBind::newSlot(v, -1, "NUMPAD_BEGIN",			(int)WXK_NUMPAD_BEGIN);
	NitBind::newSlot(v, -1, "NUMPAD_INSERT",		(int)WXK_NUMPAD_INSERT);
	NitBind::newSlot(v, -1, "NUMPAD_DELETE",		(int)WXK_NUMPAD_DELETE);
	NitBind::newSlot(v, -1, "NUMPAD_EQUAL",			(int)WXK_NUMPAD_EQUAL);
	NitBind::newSlot(v, -1, "NUMPAD_MULTIPLY",		(int)WXK_NUMPAD_MULTIPLY);
	NitBind::newSlot(v, -1, "NUMPAD_ADD",			(int)WXK_NUMPAD_ADD);
	NitBind::newSlot(v, -1, "NUMPAD_SEPARATOR",		(int)WXK_NUMPAD_SEPARATOR);
	NitBind::newSlot(v, -1, "NUMPAD_SUBTRACT",		(int)WXK_NUMPAD_SUBTRACT);
	NitBind::newSlot(v, -1, "NUMPAD_DECIMAL",		(int)WXK_NUMPAD_DECIMAL);
	NitBind::newSlot(v, -1, "NUMPAD_DIVIDE",		(int)WXK_NUMPAD_DIVIDE);
	NitBind::newSlot(v, -1, "WINDOWS_LEFT",			(int)WXK_WINDOWS_LEFT);
	NitBind::newSlot(v, -1, "WINDOWS_RIGHT",		(int)WXK_WINDOWS_RIGHT);
	NitBind::newSlot(v, -1, "WINDOWS_MENU",			(int)WXK_WINDOWS_MENU);
	NitBind::newSlot(v, -1, "COMMAND",				(int)WXK_COMMAND);
	NitBind::newSlot(v, -1, "SPECIAL1",				(int)WXK_SPECIAL1);
	NitBind::newSlot(v, -1, "SPECIAL2",				(int)WXK_SPECIAL2);
	NitBind::newSlot(v, -1, "SPECIAL3",				(int)WXK_SPECIAL3);
	NitBind::newSlot(v, -1, "SPECIAL4",				(int)WXK_SPECIAL4);
	NitBind::newSlot(v, -1, "SPECIAL5",				(int)WXK_SPECIAL5);
	NitBind::newSlot(v, -1, "SPECIAL6",				(int)WXK_SPECIAL6);
	NitBind::newSlot(v, -1, "SPECIAL7",				(int)WXK_SPECIAL7);
	NitBind::newSlot(v, -1, "SPECIAL8",				(int)WXK_SPECIAL8);
	NitBind::newSlot(v, -1, "SPECIAL9",				(int)WXK_SPECIAL9);
	NitBind::newSlot(v, -1, "SPECIAL10",			(int)WXK_SPECIAL10);
	NitBind::newSlot(v, -1, "SPECIAL11",			(int)WXK_SPECIAL11);
	NitBind::newSlot(v, -1, "SPECIAL12",			(int)WXK_SPECIAL12);
	NitBind::newSlot(v, -1, "SPECIAL13",			(int)WXK_SPECIAL13);
	NitBind::newSlot(v, -1, "SPECIAL14",			(int)WXK_SPECIAL14);
	NitBind::newSlot(v, -1, "SPECIAL15",			(int)WXK_SPECIAL15);
	NitBind::newSlot(v, -1, "SPECIAL16",			(int)WXK_SPECIAL16);
	NitBind::newSlot(v, -1, "SPECIAL17",			(int)WXK_SPECIAL17);
	NitBind::newSlot(v, -1, "SPECIAL18",			(int)WXK_SPECIAL18);
	NitBind::newSlot(v, -1, "SPECIAL19",			(int)WXK_SPECIAL19);
	NitBind::newSlot(v, -1, "SPECIAL20",			(int)WXK_SPECIAL20);
	sq_newslot(v, -3, false);
}

class NB_WxRootLib : NitBind
{
public:
	static void Register(HSQUIRRELVM v)
	{
		// load 'wx' namespace
		sq_pushroottable(v);
		sq_pushstring(v, "wx", -1);
		sq_get(v, -2);

		// Register functions //////////////////////////////////

		sq_register_h(v, "newId", NewId,
			"(): int");

		sq_register_h(v, "messageBox", MessageBox, 
			"(msg: string, caption=null, style=MB.OK|MB.CENTER, parent: Window=null, x=null, y=null): MB");

		sq_register_h(v, "aboutBox", AboutBox,
			"(parent=null, icon=null, name=null, version=null, description=null, copyright=null, license=null, web=null, developers: []=null, docwriters: []=null, artists: []=null, translators: []=null)");

		sq_register_h(v, "getSingleChoice", GetSingleChoice,
			"(message, caption, choices: array, initialSelection=0, parent: Window=null"
			" x=null, y=null, center=true, w=null, h=null"
			"): int // returns -1 if canceled otherwise index of the choice");

		sq_register_h(v, "getMultiChoices", GetMultiChoices,
			"(message, caption, choices: array, selected: int[]=null, parent: Window=null"
			" x=null, y=null, center=true, w=null, h=null"
			"): [] // returns null if canceled otherwise array of selected");

		sq_register_h(v, "dirSelector", DirSelector,
			"(message=STR.DirSelectorPrompt, path=\"\", style=0, pos=null, parent: Window=null): string");

		sq_register_h(v, "fileSelector", FileSelector,
			"(message, path=\"\", filename=\"\", ext=\"\", wildcard=null, flags=0: FileDialog.STYLE.OPEN | FileDialog.STYLE.SAVE | FileDialog.STYLE.OVERWRITE_PROMPT | FileDialog.STYLE.FILE_MUST_EXIST, parent: Window=null, x=null, y=null): string");

		sq_register_h(v, "getColorFromUser", GetColorFromUser,
			"(parent: Window, color: Color, caption=\"\"): Color"); 

		// 		sq_register_h(v, "GetFontFromUser", NitLibWx::GetFontFromUser,
// 			"(parent: Window=null, fontInit=NULL_FONT, caption=\"\")");

		sq_register_h(v, "getTextFromUser", GetTextFromUser,
			"(msg: string, caption=null, default_value=\"\", parent: Window=null, x=null, y=null, center=true): string");

		sq_register_h(v, "getPasswordFromUser", GetPasswordFromUser,
			"(msg: string, caption=null, default_value=\"\", parent: Window=null, x=null, y=null, center=true): string");

		sq_register_h(v, "logMessage", LogMessage,
			"(msg: string)\n(fmt: string, ...)");

		sq_register_h(v, "logWarning", LogWarning,
			"(msg: string)\n(fmt: string, ...)");

		sq_register_h(v, "logError", LogError,
			"(msg: string)\n(fmt: string, ...)");

		sq_register_h(v, "logFlush", LogFlush, "()");

		if (dynamic_cast<wxNitApp*>(wxTheApp))
		{
			newSlot(v, -1, "app", static_cast<wxNitApp*>(wxTheApp));
		}
		else
		{
			newSlot(v, -1, "app", wxTheApp);
		}

		sq_poptop(v); // "wx"
	}

	static SQInteger NewId(HSQUIRRELVM v)
	{
		return push(v, wxNewId());
	}

	static SQInteger MessageBox(HSQUIRRELVM v)
	{
		wxString msg = getWxString(v, 2);
		wxString caption = optWxString(v, 3, wxMessageBoxCaptionStr);
		long style = optInt(v, 4, wxOK | wxCENTRE);
		wxWindow* parent = opt<wxWindow>(v, 5, NULL);
		int x = optInt(v, 6, wxDefaultCoord);
		int y = optInt(v, 7, wxDefaultCoord);

		return push(v, wxMessageBox(msg, caption, style, parent, x, y));
	}

	static SQInteger AboutBox(HSQUIRRELVM v)
	{
		wxAboutDialogInfo info;

		wxWindow* parent = opt<wxWindow>(v, 2, NULL);

 		if (!isNone(v, 3) && is<wxIcon>(v, 3))
 			info.SetIcon(*get<wxIcon>(v, 3));

		if (!isNone(v, 4) && !isNull(v, 4))
			info.SetName(getWxString(v, 4));

		if (!isNone(v, 5) && !isNull(v, 5))
			info.SetVersion(getWxString(v, 5));

		if (!isNone(v, 6) && !isNull(v, 6))
			info.SetDescription(getWxString(v, 6));

		if (!isNone(v, 7) && !isNull(v, 7))
			info.SetCopyright(getWxString(v, 7));

		if (!isNone(v, 8) && !isNull(v, 8))
			info.SetLicense(getWxString(v, 8));

		if (!isNone(v, 9) && !isNull(v, 9))
			info.SetWebSite(getWxString(v, 9));

		if (!isNone(v, 10) && sq_gettype(v, 10) == OT_ARRAY)
		{
			wxArrayString persons;
			for (NitIterator itr(v, 10); itr.hasNext(); itr.next())
			{
				sq_tostring(v, itr.valueIndex());
				persons.Add(getWxString(v, -1));
				sq_poptop(v);
			}
			info.SetDevelopers(persons);
		}

		if (!isNone(v, 11) && sq_gettype(v, 11) == OT_ARRAY)
		{
			wxArrayString persons;
			for (NitIterator itr(v, 11); itr.hasNext(); itr.next())
			{
				sq_tostring(v, itr.valueIndex());
				persons.Add(getWxString(v, -1));
				sq_poptop(v);
			}
			info.SetDocWriters(persons);
		}

		if (!isNone(v, 12) && sq_gettype(v, 12) == OT_ARRAY)
		{
			wxArrayString persons;
			for (NitIterator itr(v, 12); itr.hasNext(); itr.next())
			{
				sq_tostring(v, itr.valueIndex());
				persons.Add(getWxString(v, -1));
				sq_poptop(v);
			}
			info.SetArtists(persons);
		}

		if (!isNone(v, 13) && sq_gettype(v, 13) == OT_ARRAY)
		{
			wxArrayString persons;
			for (NitIterator itr(v, 13); itr.hasNext(); itr.next())
			{
				sq_tostring(v, itr.valueIndex());
				persons.Add(getWxString(v, -1));
				sq_poptop(v);
			}
			info.SetTranslators(persons);
		}

		wxAboutBox(info, NULL);
		return 0;
	}

	static SQInteger GetSingleChoice(HSQUIRRELVM v)
	{
		if (sq_gettype(v, 4) != OT_ARRAY) 
			return sq_throwerror(v, "choices must be array");

		wxString msg = getWxString(v, 2);
		wxString caption = getWxString(v, 3);

		wxArrayString choices;
		for (NitIterator itr(v, 4); itr.hasNext(); itr.next())
		{
			sq_tostring(v, itr.valueIndex());
			choices.Add(getWxString(v, -1));
			sq_poptop(v);
		}

		int initSel = optInt(v, 5, 0);
		wxWindow* parent = opt<wxWindow>(v, 6, NULL);
		int x = optInt(v, 7, wxDefaultCoord);
		int y = optInt(v, 8, wxDefaultCoord);
		bool center = optBool(v, 9, true);
		int w = optInt(v, 10, wxCHOICE_WIDTH);
		int h = optInt(v, 11, wxCHOICE_HEIGHT);

		int sel = wxGetSingleChoiceIndex(msg, caption, choices, parent, x, y, center, w, h, initSel);

		return push(v, sel);
	}

	static SQInteger GetMultiChoices(HSQUIRRELVM v)
	{
		if (sq_gettype(v, 4) != OT_ARRAY) 
			return sq_throwerror(v, "choices must be array");

		if (sq_gettype(v, 5) != OT_ARRAY && sq_gettype(v, 5) != OT_NULL) 
			return sq_throwerror(v, "choices must be array or null");

		wxString msg = getWxString(v, 2);
		wxString caption = getWxString(v, 3);

		wxArrayString choices;
		for (NitIterator itr(v, 4); itr.hasNext(); itr.next())
		{
			sq_tostring(v, itr.valueIndex());
			choices.Add(getWxString(v, -1));
			sq_poptop(v);
		}

		wxArrayInt selected;
		if (sq_gettype(v, 5) == OT_ARRAY)
		{
			for (NitIterator itr(v, 5); itr.hasNext(); itr.next())
			{
				int idx = getInt(v, itr.valueIndex());
				if (idx < 0 || idx >= (int)choices.size())
					return sq_throwerror(v, "one of selected out of range");
				selected.Add(idx);
			}
		}

		wxWindow* parent = opt<wxWindow>(v, 6, NULL);
		int x = optInt(v, 7, wxDefaultCoord);
		int y = optInt(v, 8, wxDefaultCoord);
		bool center = optBool(v, 9, true);
		int w = optInt(v, 10, wxCHOICE_WIDTH);
		int h = optInt(v, 11, wxCHOICE_HEIGHT);

		int numSel = wxGetSelectedChoices(selected, msg, caption, choices, parent, x, y, center, w, h);

		if (numSel == -1)
			return 0;

		sq_newarray(v, 0);

		for (uint i=0; i<selected.Count(); ++i)
			arrayAppend(v, -1, selected[i]);

		return 1;
	}

	static SQInteger DirSelector(HSQUIRRELVM v)
	{
		wxString msg = optWxString(v, 2, wxDirSelectorPromptStr);
		wxString path = optWxString(v, 3, "");
		long style = optInt(v, 4, 0);
		const wxPoint& pos = *opt<wxPoint>(v, 5, wxDefaultPosition);
		wxWindow* parent = opt<wxWindow>(v, 6, NULL);

		return push(v, wxDirSelector(msg, path, style, pos, parent));
	}

	static SQInteger FileSelector(HSQUIRRELVM v)
	{
		wxString msg = getWxString(v, 2);
		wxString path = optWxString(v, 3, "");
		wxString filename = optWxString(v, 4, "");
		wxString ext = optWxString(v, 5, "");
		wxString wildcard = optWxString(v, 6, wxFileSelectorDefaultWildcardStr);
		int flags = optInt(v, 7, 0);
		wxWindow* parent = opt<wxWindow>(v, 8, NULL);
		int x = optInt(v, 9, wxDefaultCoord);
		int y = optInt(v, 10, wxDefaultCoord);

		return push(v, wxFileSelector(msg, path, filename, ext, wildcard, flags, parent, x, y));
	}

	static SQInteger GetColorFromUser(HSQUIRRELVM v)
	{
		wxWindow* parent = opt<wxWindow>(v, 2, NULL);
		wxColour color = GetWxColor(v, 3);
		wxString caption = optWxString(v, 4, "");

		color = wxGetColourFromUser (parent, color, caption);
		return PushWxColor(v, color);
	}

	static SQInteger GetTextFromUser(HSQUIRRELVM v)
	{
		wxString msg = getWxString(v, 2);
		wxString caption = optWxString(v, 3, wxGetTextFromUserPromptStr);
		wxString defaultValue = optWxString(v, 4, "");
		wxWindow* parent = opt<wxWindow>(v, 5, NULL);
		wxCoord x = optInt(v, 6, wxDefaultCoord);
		wxCoord y = optInt(v, 7, wxDefaultCoord);
		bool centre = optBool(v, 8, true);

		return push(v, wxGetTextFromUser(msg, caption, defaultValue, parent, x, y, centre));
	}

	static SQInteger GetPasswordFromUser(HSQUIRRELVM v)
	{
		wxString msg = getWxString(v, 2);
		wxString caption = optWxString(v, 3, wxGetTextFromUserPromptStr);
		wxString defaultValue = optWxString(v, 4, "");
		wxWindow* parent = opt<wxWindow>(v, 5, NULL);
		wxCoord x = optInt(v, 6, wxDefaultCoord);
		wxCoord y = optInt(v, 7, wxDefaultCoord);
		bool centre = optBool(v, 8, true);

		return push(v, wxGetPasswordFromUser(msg, caption, defaultValue, parent, x, y, centre));
	}

	static SQInteger LogMessage(HSQUIRRELVM v)
	{
		if (isNone(v, 3))
		{
			wxLogMessage(getWxString(v, 2));
			return 0;
		}

		SQChar* msg;
		int len;
		if (SQ_FAILED(sqstd_format(v, 2, &len, &msg)))
			return SQ_ERROR;

		wxLogMessage(wxString::FromUTF8Unchecked(msg));
		return 0;
	}

	static SQInteger LogWarning(HSQUIRRELVM v)
	{
		if (isNone(v, 3))
		{
			wxLogWarning(getWxString(v, 2));
			return 0;
		}

		SQChar* msg;
		int len;
		if (SQ_FAILED(sqstd_format(v, 2, &len, &msg)))
			return SQ_ERROR;

		wxLogWarning(wxString::FromUTF8Unchecked(msg));
		return 0;
	}

	static SQInteger LogError(HSQUIRRELVM v)
	{
		if (isNone(v, 3))
		{
			wxLogError(getWxString(v, 2));
			return 0;
		}

		SQChar* msg;
		int len;
		if (SQ_FAILED(sqstd_format(v, 2, &len, &msg)))
			return SQ_ERROR;

		wxLogError(wxString::FromUTF8Unchecked(msg));
		return 0;
	}

	static SQInteger LogFlush(HSQUIRRELVM v)
	{
		wxLog::FlushActive();
		return 0;
	}
};

static int _RGB(const wxColour& color)
{
	return (color.Red() << 16) | (color.Green() << 8) | color.Blue();
}

////////////////////////////////////////////////////////////////////////////////

SQRESULT NitLibWx(HSQUIRRELVM v)
{
	// Install ScriptWxWeakTracker
	ScriptRuntime* runtime = ScriptRuntime::getRuntime(v);
	runtime->setWxWeakTracker(new ScriptWxWeakTracker(runtime));

	// Register classes ///////////////////////////////////

	NB_WxRootLib::Register(v);

	// load 'wx' namespace
	sq_pushroottable(v);
	sq_pushstring(v, "wx", -1);
	sq_get(v, -2);

	// Register functions //////////////////////////////////

	sq_register_h(v, "MessageBox", NB_WxRootLib::MessageBox, "(msg: string, caption=STR.MSGBOX_CAPTION, style=wx.MB.OK|wx.MB.CENTER|wx.ICON.INFORMATION, parent: Window=null, x=DEFAULT.COORD, y=DEFAULT.COORD): WS");

	// Register constants //////////////////////////////////

	// very common constants in wx root namespace
	NitBind::newSlot(v, -1, "VERTICAL",				(int)wxVERTICAL);
	NitBind::newSlot(v, -1, "HORIZONTAL",			(int)wxHORIZONTAL);
	NitBind::newSlot(v, -1, "BOTH",					(int)wxBOTH);

	// Big Constants /////////////

	// Window Styles
	RegisterWS(v);	

	// Keys
	RegisterKEY(v);

	// Event Types
	RegisterEVT(v);

	// Predefined IDs
	RegisterID(v);

	// Other Constants ///////////

	sq_pushstring(v, "DEFAULT", -1);
	sq_newtable(v);	
	NitBind::newSlot(v, -1, "SIZE",					wxDefaultSize);
	NitBind::newSlot(v, -1, "POS",					wxDefaultPosition);
	NitBind::newSlot(v, -1, "VALIDATOR",			(wxValidator*)&wxDefaultValidator);
	sq_newslot(v, -3, false);

	sq_pushstring(v, "ORIENT", -1);
	sq_newtable(v);	
	NitBind::newSlot(v, -1, "VERTICAL",				(int)wxVERTICAL);
	NitBind::newSlot(v, -1, "HORIZONTAL",			(int)wxHORIZONTAL);
	NitBind::newSlot(v, -1, "BOTH",					(int)wxBOTH);
	NitBind::newSlot(v, -1, "MASK",					(int)wxORIENTATION_MASK);
	sq_newslot(v, -3, false);

	sq_pushstring(v, "DIR", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "LEFT",					(int)wxLEFT);
	NitBind::newSlot(v, -1, "RIGHT",				(int)wxRIGHT);
	NitBind::newSlot(v, -1, "UP",					(int)wxUP);
	NitBind::newSlot(v, -1, "DOWN",					(int)wxDOWN);
	NitBind::newSlot(v, -1, "TOP",					(int)wxTOP);
	NitBind::newSlot(v, -1, "BOTTOM",				(int)wxBOTTOM);
	NitBind::newSlot(v, -1, "NORTH",				(int)wxNORTH);
	NitBind::newSlot(v, -1, "SOUTH",				(int)wxSOUTH);
	NitBind::newSlot(v, -1, "WEST",					(int)wxWEST);
	NitBind::newSlot(v, -1, "EAST",					(int)wxEAST);
	NitBind::newSlot(v, -1, "ALL",					(int)wxALL);
	NitBind::newSlot(v, -1, "MASK",					(int)wxDIRECTION_MASK);
	sq_newslot(v, -3, false);

	sq_pushstring(v, "ALIGN", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "INVALID",				(int)wxALIGN_INVALID);
	NitBind::newSlot(v, -1, "NOT",					(int)wxALIGN_NOT);
	NitBind::newSlot(v, -1, "CENTER_HORIZONTAL",	(int)wxALIGN_CENTER_HORIZONTAL);
	NitBind::newSlot(v, -1, "LEFT",					(int)wxALIGN_LEFT);
	NitBind::newSlot(v, -1, "TOP",					(int)wxALIGN_TOP);
	NitBind::newSlot(v, -1, "RIGHT",				(int)wxALIGN_RIGHT);
	NitBind::newSlot(v, -1, "BOTTOM",				(int)wxALIGN_BOTTOM);
	NitBind::newSlot(v, -1, "CENTER_VERTICAL",		(int)wxALIGN_CENTER_VERTICAL);
	NitBind::newSlot(v, -1, "CENTER",				(int)wxALIGN_CENTER);
	NitBind::newSlot(v, -1, "MASK",					(int)wxALIGN_MASK);
	sq_newslot(v, -3, false);

	sq_pushstring(v, "SIZER", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "FIXED_MINSIZE",			(int)wxFIXED_MINSIZE);
	NitBind::newSlot(v, -1, "RESERVE_SPACE_EVEN_IF_HIDDEN",	(int)wxRESERVE_SPACE_EVEN_IF_HIDDEN);
	NitBind::newSlot(v, -1, "MASK",						(int)wxSIZER_FLAG_BITS_MASK);
	sq_newslot(v, -3, false);

	sq_pushstring(v, "STRETCH", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "NOT",					(int)wxSTRETCH_NOT);
	NitBind::newSlot(v, -1, "SHRINK",				(int)wxSHRINK);
	NitBind::newSlot(v, -1, "GROW",					(int)wxGROW);
	NitBind::newSlot(v, -1, "EXPAND",				(int)wxEXPAND);
	NitBind::newSlot(v, -1, "SHAPED",				(int)wxSHAPED);
	NitBind::newSlot(v, -1, "TILE",					(int)wxTILE);
	NitBind::newSlot(v, -1, "MASK",					(int)wxSTRETCH_MASK);
	sq_newslot(v, -3, false);

	sq_pushstring(v, "BORDER", -1);
	sq_newtable(v);	
	NitBind::newSlot(v, -1, "DEFAULT",				(int)wxBORDER_DEFAULT);
	NitBind::newSlot(v, -1, "DEFAULT_CONTROL",		(int)wxDEFAULT_CONTROL_BORDER);
	NitBind::newSlot(v, -1, "NONE",					(int)wxBORDER_NONE);
	NitBind::newSlot(v, -1, "STATIC",				(int)wxBORDER_STATIC);
	NitBind::newSlot(v, -1, "SIMPLE",				(int)wxBORDER_SIMPLE);
	NitBind::newSlot(v, -1, "RAISED",				(int)wxBORDER_RAISED);
	NitBind::newSlot(v, -1, "SUNKEN",				(int)wxBORDER_SUNKEN);
	NitBind::newSlot(v, -1, "THEME",				(int)wxBORDER_THEME);
	NitBind::newSlot(v, -1, "MASK",					(int)wxBORDER_MASK);
	sq_newslot(v, -3, false);

	sq_pushstring(v, "CURSOR", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "NONE", 				(int)wxCURSOR_NONE);
	NitBind::newSlot(v, -1, "ARROW", 				(int)wxCURSOR_ARROW);
	NitBind::newSlot(v, -1, "RIGHT_ARROW", 			(int)wxCURSOR_RIGHT_ARROW);
	NitBind::newSlot(v, -1, "BULLSEYE", 			(int)wxCURSOR_BULLSEYE);
	NitBind::newSlot(v, -1, "CHAR", 				(int)wxCURSOR_CHAR);
	NitBind::newSlot(v, -1, "CROSS", 				(int)wxCURSOR_CROSS);
	NitBind::newSlot(v, -1, "HAND", 				(int)wxCURSOR_HAND);
	NitBind::newSlot(v, -1, "IBEAM", 				(int)wxCURSOR_IBEAM);
	NitBind::newSlot(v, -1, "LEFT_BUTTON", 			(int)wxCURSOR_LEFT_BUTTON);
	NitBind::newSlot(v, -1, "MAGNIFIER", 			(int)wxCURSOR_MAGNIFIER);
	NitBind::newSlot(v, -1, "MIDDLE_BUTTON", 		(int)wxCURSOR_MIDDLE_BUTTON);
	NitBind::newSlot(v, -1, "NO_ENTRY", 			(int)wxCURSOR_NO_ENTRY);
	NitBind::newSlot(v, -1, "PAINT_BRUSH", 			(int)wxCURSOR_PAINT_BRUSH);
	NitBind::newSlot(v, -1, "PENCIL", 				(int)wxCURSOR_PENCIL);
	NitBind::newSlot(v, -1, "POINT_LEFT", 			(int)wxCURSOR_POINT_LEFT);
	NitBind::newSlot(v, -1, "POINT_RIGHT", 			(int)wxCURSOR_POINT_RIGHT);
	NitBind::newSlot(v, -1, "QUESTION_ARROW", 		(int)wxCURSOR_QUESTION_ARROW);
	NitBind::newSlot(v, -1, "RIGHT_BUTTON", 		(int)wxCURSOR_RIGHT_BUTTON);
	NitBind::newSlot(v, -1, "SIZENESW", 			(int)wxCURSOR_SIZENESW);
	NitBind::newSlot(v, -1, "SIZENS", 				(int)wxCURSOR_SIZENS);
	NitBind::newSlot(v, -1, "SIZENWSE", 			(int)wxCURSOR_SIZENWSE);
	NitBind::newSlot(v, -1, "SIZEWE", 				(int)wxCURSOR_SIZEWE);
	NitBind::newSlot(v, -1, "SIZING", 				(int)wxCURSOR_SIZING);
	NitBind::newSlot(v, -1, "SPRAYCAN", 			(int)wxCURSOR_SPRAYCAN);
	NitBind::newSlot(v, -1, "WAIT", 				(int)wxCURSOR_WAIT);
	NitBind::newSlot(v, -1, "WATCH", 				(int)wxCURSOR_WATCH);
	NitBind::newSlot(v, -1, "BLANK", 				(int)wxCURSOR_BLANK);
	sq_newslot(v, -3, false);

	sq_pushstring(v, "ICON", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "EXCLAMATION",			(int)wxICON_EXCLAMATION);
	NitBind::newSlot(v, -1, "HAND",					(int)wxICON_HAND);
	NitBind::newSlot(v, -1, "WARNING",				(int)wxICON_WARNING);
	NitBind::newSlot(v, -1, "ERROR",				(int)wxICON_ERROR);
	NitBind::newSlot(v, -1, "QUESTION",				(int)wxICON_QUESTION);
	NitBind::newSlot(v, -1, "INFORMATION",			(int)wxICON_INFORMATION);
	NitBind::newSlot(v, -1, "STOP",					(int)wxICON_STOP);
	NitBind::newSlot(v, -1, "ASTERISK",				(int)wxICON_ASTERISK);
	NitBind::newSlot(v, -1, "NONE",					(int)wxICON_NONE);
	NitBind::newSlot(v, -1, "MASK",					(int)wxICON_MASK);
	sq_newslot(v, -3, false);

	sq_pushstring(v, "BG_STYLE", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "ERASE",				(int)wxBG_STYLE_ERASE);
	NitBind::newSlot(v, -1, "SYSTEM",				(int)wxBG_STYLE_SYSTEM);
	NitBind::newSlot(v, -1, "PAINT",				(int)wxBG_STYLE_PAINT);
	NitBind::newSlot(v, -1, "TRANSPARENT",			(int)wxBG_STYLE_TRANSPARENT);
	NitBind::newSlot(v, -1, "COLOR",				(int)wxBG_STYLE_COLOUR);
	NitBind::newSlot(v, -1, "CUSTOM",				(int)wxBG_STYLE_CUSTOM);
	sq_newslot(v, -3, false);

	sq_pushstring(v, "ITEM", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "SEPARATOR",			(int)wxITEM_SEPARATOR);
	NitBind::newSlot(v, -1, "NORMAL",				(int)wxITEM_NORMAL);
	NitBind::newSlot(v, -1, "CHECK",				(int)wxITEM_CHECK);
	NitBind::newSlot(v, -1, "RADIO",				(int)wxITEM_RADIO);
	NitBind::newSlot(v, -1, "DROPDOWN",				(int)wxITEM_DROPDOWN);
	sq_newslot(v, -3, false);

	sq_pushstring(v, "HT", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "NOWHERE",					(int)wxHT_NOWHERE);
	NitBind::newSlot(v, -1, "SCROLLBAR_FIRST",			(int)wxHT_SCROLLBAR_FIRST);
	NitBind::newSlot(v, -1, "SCROLLBAR_ARROW_LINE_1",	(int)wxHT_SCROLLBAR_ARROW_LINE_1);
	NitBind::newSlot(v, -1, "SCROLLBAR_ARROW_LINE_2",	(int)wxHT_SCROLLBAR_ARROW_LINE_2);
	NitBind::newSlot(v, -1, "SCROLLBAR_ARROW_PAGE_1",	(int)wxHT_SCROLLBAR_ARROW_PAGE_1);
	NitBind::newSlot(v, -1, "SCROLLBAR_ARROW_PAGE_2",	(int)wxHT_SCROLLBAR_ARROW_PAGE_2);
	NitBind::newSlot(v, -1, "SCROLLBAR_THUMB",			(int)wxHT_SCROLLBAR_THUMB);
	NitBind::newSlot(v, -1, "SCROLLBAR_BAR_1",			(int)wxHT_SCROLLBAR_BAR_1);
	NitBind::newSlot(v, -1, "SCROLLBAR_BAR_2",			(int)wxHT_SCROLLBAR_BAR_2);
	NitBind::newSlot(v, -1, "SCROLLBAR_LAST",			(int)wxHT_SCROLLBAR_LAST);
	NitBind::newSlot(v, -1, "WINDOW_OUTSIDE",			(int)wxHT_WINDOW_OUTSIDE);
	NitBind::newSlot(v, -1, "WINDOW_INSIDE",			(int)wxHT_WINDOW_INSIDE);
	NitBind::newSlot(v, -1, "WINDOW_VERT_SCROLLBAR",	(int)wxHT_WINDOW_VERT_SCROLLBAR);
	NitBind::newSlot(v, -1, "WINDOW_HORZ_SCROLLBAR",	(int)wxHT_WINDOW_HORZ_SCROLLBAR);
	NitBind::newSlot(v, -1, "WINDOW_CORNER",			(int)wxHT_WINDOW_CORNER);
	NitBind::newSlot(v, -1, "MAX",						(int)wxHT_MAX);
	sq_newslot(v, -3, false);

	sq_pushstring(v, "SIZE", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "AUTO_WIDTH",			(int)wxSIZE_AUTO_WIDTH);
	NitBind::newSlot(v, -1, "AUTO_HEIGHT",			(int)wxSIZE_AUTO_HEIGHT);
	NitBind::newSlot(v, -1, "AUTO",					(int)wxSIZE_AUTO);
	NitBind::newSlot(v, -1, "USE_EXISTING",			(int)wxSIZE_USE_EXISTING);
	NitBind::newSlot(v, -1, "ALLOW_MINUS_ONE",		(int)wxSIZE_ALLOW_MINUS_ONE);
	NitBind::newSlot(v, -1, "NO_ADJUSTMENTS",		(int)wxSIZE_NO_ADJUSTMENTS);
	NitBind::newSlot(v, -1, "FORCE",				(int)wxSIZE_FORCE);
	NitBind::newSlot(v, -1, "FORCE_EVENT",			(int)wxSIZE_FORCE_EVENT);
	sq_newslot(v, -3, false);

	sq_pushstring(v, "DF", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "INVALID",				(int)wxDF_INVALID);
	NitBind::newSlot(v, -1, "TEXT",					(int)wxDF_TEXT);
	NitBind::newSlot(v, -1, "BITMAP",				(int)wxDF_BITMAP);
	NitBind::newSlot(v, -1, "METAFILE",				(int)wxDF_METAFILE);
	NitBind::newSlot(v, -1, "SYLK",					(int)wxDF_SYLK);
	NitBind::newSlot(v, -1, "DIF",					(int)wxDF_DIF);
	NitBind::newSlot(v, -1, "TIFF",					(int)wxDF_TIFF);
	NitBind::newSlot(v, -1, "OEMTEXT",				(int)wxDF_OEMTEXT);
	NitBind::newSlot(v, -1, "DIB",					(int)wxDF_DIB);
	NitBind::newSlot(v, -1, "PALETTE",				(int)wxDF_PALETTE);
	NitBind::newSlot(v, -1, "PENDATA",				(int)wxDF_PENDATA);
	NitBind::newSlot(v, -1, "RIFF",					(int)wxDF_RIFF);
	NitBind::newSlot(v, -1, "WAVE",					(int)wxDF_WAVE);
	NitBind::newSlot(v, -1, "UNICODETEXT",			(int)wxDF_UNICODETEXT);
	NitBind::newSlot(v, -1, "ENHMETAFILE",			(int)wxDF_ENHMETAFILE);
	NitBind::newSlot(v, -1, "FILENAME",				(int)wxDF_FILENAME);
	NitBind::newSlot(v, -1, "LOCALE",				(int)wxDF_LOCALE);
	NitBind::newSlot(v, -1, "PRIVATE",				(int)wxDF_PRIVATE);
	NitBind::newSlot(v, -1, "HTML",					(int)wxDF_HTML);
	NitBind::newSlot(v, -1, "MAX",					(int)wxDF_MAX);
	sq_newslot(v, -3, false);

	sq_pushstring(v, "MOD", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "NONE",					(int)wxMOD_NONE);
	NitBind::newSlot(v, -1, "ALT",					(int)wxMOD_ALT);
	NitBind::newSlot(v, -1, "CONTROL",				(int)wxMOD_CONTROL);
	NitBind::newSlot(v, -1, "ALTGR",				(int)wxMOD_ALTGR);
	NitBind::newSlot(v, -1, "SHIFT",				(int)wxMOD_SHIFT);
	NitBind::newSlot(v, -1, "META",					(int)wxMOD_META);
	NitBind::newSlot(v, -1, "WIN",					(int)wxMOD_WIN);
	NitBind::newSlot(v, -1, "CMD",					(int)wxMOD_CMD);
	NitBind::newSlot(v, -1, "ALL",					(int)wxMOD_ALL);
	sq_newslot(v, -3, false);

	sq_pushstring(v, "UPDATE_UI", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "NONE",					(int)wxUPDATE_UI_NONE);
	NitBind::newSlot(v, -1, "RECURSE",				(int)wxUPDATE_UI_RECURSE);
	NitBind::newSlot(v, -1, "FROMIDLE",				(int)wxUPDATE_UI_FROMIDLE);
	sq_newslot(v, -3, false);

	sq_pushstring(v, "STB", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "SIZEGRIP",				(int)wxSTB_SIZEGRIP);
	NitBind::newSlot(v, -1, "SHOW_TIPS",			(int)wxSTB_SHOW_TIPS);
	NitBind::newSlot(v, -1, "ELLIPSIZE_START",		(int)wxSTB_ELLIPSIZE_START);
	NitBind::newSlot(v, -1, "ELLIPSIZE_MIDDLE",		(int)wxSTB_ELLIPSIZE_MIDDLE);
	NitBind::newSlot(v, -1, "ELLIPSIZE_END",		(int)wxSTB_ELLIPSIZE_END);
	NitBind::newSlot(v, -1, "DEFAULT",				(int)wxSTB_DEFAULT_STYLE);
	sq_newslot(v, -3, false);

	sq_pushstring(v, "DRAG_FLAG", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "COPY_ONLY",			(int)wxDrag_CopyOnly);
	NitBind::newSlot(v, -1, "ALLOW_MOVE",			(int)wxDrag_AllowMove);
	NitBind::newSlot(v, -1, "DEFAULT_MOVE",			(int)wxDrag_DefaultMove);
	sq_newslot(v, -3, false);

	sq_pushstring(v, "DRAG_RESULT", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "ERROR",				(int)wxDragError);
	NitBind::newSlot(v, -1, "NONE",					(int)wxDragNone);
	NitBind::newSlot(v, -1, "COPY",					(int)wxDragCopy);
	NitBind::newSlot(v, -1, "MOVE",					(int)wxDragMove);
	NitBind::newSlot(v, -1, "LINK",					(int)wxDragLink);
	NitBind::newSlot(v, -1, "CANCEL",				(int)wxDragCancel);
	sq_newslot(v, -3, false);

	sq_pushstring(v, "ART", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "ADD_BOOKMARK",			wxART_ADD_BOOKMARK);
	NitBind::newSlot(v, -1, "DEL_BOOKMARK",			wxART_DEL_BOOKMARK);
	NitBind::newSlot(v, -1, "HELP_SIDE_PANEL",		wxART_HELP_SIDE_PANEL);
	NitBind::newSlot(v, -1, "HELP_SETTINGS",		wxART_HELP_SETTINGS);
	NitBind::newSlot(v, -1, "HELP_BOOK",			wxART_HELP_BOOK);
	NitBind::newSlot(v, -1, "HELP_FOLDER",			wxART_HELP_FOLDER);
	NitBind::newSlot(v, -1, "HELP_PAGE",			wxART_HELP_PAGE);
	NitBind::newSlot(v, -1, "GO_BACK",				wxART_GO_BACK);
	NitBind::newSlot(v, -1, "GO_FORWARD",			wxART_GO_FORWARD);
	NitBind::newSlot(v, -1, "GO_UP",				wxART_GO_UP);
	NitBind::newSlot(v, -1, "GO_DOWN",				wxART_GO_DOWN);
	NitBind::newSlot(v, -1, "GO_TO_PARENT",			wxART_GO_TO_PARENT);
	NitBind::newSlot(v, -1, "GO_HOME",				wxART_GO_HOME);
	NitBind::newSlot(v, -1, "GOTO_FIRST",			wxART_GOTO_FIRST);
	NitBind::newSlot(v, -1, "GOTO_LAST",			wxART_GOTO_LAST);
	NitBind::newSlot(v, -1, "FILE_OPEN",			wxART_FILE_OPEN);
	NitBind::newSlot(v, -1, "FILE_SAVE",			wxART_FILE_SAVE);
	NitBind::newSlot(v, -1, "FILE_SAVE_AS",			wxART_FILE_SAVE_AS);
	NitBind::newSlot(v, -1, "PRINT",				wxART_PRINT);
	NitBind::newSlot(v, -1, "HELP",					wxART_HELP);
	NitBind::newSlot(v, -1, "TIP",					wxART_TIP);
	NitBind::newSlot(v, -1, "REPORT_VIEW",			wxART_REPORT_VIEW);
	NitBind::newSlot(v, -1, "LIST_VIEW",			wxART_LIST_VIEW);
	NitBind::newSlot(v, -1, "NEW_DIR",				wxART_NEW_DIR);
	NitBind::newSlot(v, -1, "HARDDISK",				wxART_HARDDISK);
	NitBind::newSlot(v, -1, "FLOPPY",				wxART_FLOPPY);
	NitBind::newSlot(v, -1, "CDROM",				wxART_CDROM);
	NitBind::newSlot(v, -1, "REMOVABLE",			wxART_REMOVABLE);
	NitBind::newSlot(v, -1, "FOLDER",				wxART_FOLDER);
	NitBind::newSlot(v, -1, "FOLDER_OPEN",			wxART_FOLDER_OPEN);
	NitBind::newSlot(v, -1, "GO_DIR_UP",			wxART_GO_DIR_UP);
	NitBind::newSlot(v, -1, "EXECUTABLE_FILE",		wxART_EXECUTABLE_FILE);
	NitBind::newSlot(v, -1, "NORMAL_FILE",			wxART_NORMAL_FILE);
	NitBind::newSlot(v, -1, "TICK_MARK",			wxART_TICK_MARK);
	NitBind::newSlot(v, -1, "CROSS_MARK",			wxART_CROSS_MARK);
	NitBind::newSlot(v, -1, "ERROR",				wxART_ERROR);
	NitBind::newSlot(v, -1, "QUESTION",				wxART_QUESTION);
	NitBind::newSlot(v, -1, "WARNING",				wxART_WARNING);
	NitBind::newSlot(v, -1, "INFORMATION",			wxART_INFORMATION);
	NitBind::newSlot(v, -1, "MISSING_IMAGE",		wxART_MISSING_IMAGE);

	NitBind::newSlot(v, -1, "COPY",					wxART_COPY);
	NitBind::newSlot(v, -1, "CUT",					wxART_CUT);
	NitBind::newSlot(v, -1, "PASTE",				wxART_PASTE);
	NitBind::newSlot(v, -1, "DELETE",				wxART_DELETE);
	NitBind::newSlot(v, -1, "NEW",					wxART_NEW);

	NitBind::newSlot(v, -1, "UNDO",					wxART_UNDO);
	NitBind::newSlot(v, -1, "REDO",					wxART_REDO);
	NitBind::newSlot(v, -1, "PLUS",					wxART_PLUS);
	NitBind::newSlot(v, -1, "MINUS",				wxART_MINUS);
	NitBind::newSlot(v, -1, "CLOSE",				wxART_CLOSE);
	NitBind::newSlot(v, -1, "QUIT",					wxART_QUIT);
	NitBind::newSlot(v, -1, "FIND",					wxART_FIND);
	NitBind::newSlot(v, -1, "FIND_AND_REPLACE",		wxART_FIND_AND_REPLACE);
	sq_newslot(v, -3, false);

	sq_pushstring(v, "SHOW_EFFECT", -1);
	sq_newtable(v);
    NitBind::newSlot(v, -1, "NONE",				(int)wxSHOW_EFFECT_NONE);
    NitBind::newSlot(v, -1, "ROLL_TO_LEFT",		(int)wxSHOW_EFFECT_ROLL_TO_LEFT);
    NitBind::newSlot(v, -1, "ROLL_TO_RIGHT",	(int)wxSHOW_EFFECT_ROLL_TO_RIGHT);
    NitBind::newSlot(v, -1, "ROLL_TO_TOP",		(int)wxSHOW_EFFECT_ROLL_TO_TOP);
    NitBind::newSlot(v, -1, "ROLL_TO_BOTTOM",	(int)wxSHOW_EFFECT_ROLL_TO_BOTTOM);
    NitBind::newSlot(v, -1, "SLIDE_TO_LEFT",	(int)wxSHOW_EFFECT_SLIDE_TO_LEFT);
    NitBind::newSlot(v, -1, "SLIDE_TO_RIGHT",	(int)wxSHOW_EFFECT_SLIDE_TO_RIGHT);
    NitBind::newSlot(v, -1, "SLIDE_TO_TOP",		(int)wxSHOW_EFFECT_SLIDE_TO_TOP);
    NitBind::newSlot(v, -1, "SLIDE_TO_BOTTOM",	(int)wxSHOW_EFFECT_SLIDE_TO_BOTTOM);
    NitBind::newSlot(v, -1, "EFFECT_BLEND",		(int)wxSHOW_EFFECT_BLEND);
    NitBind::newSlot(v, -1, "EFFECT_EXPAND",	(int)wxSHOW_EFFECT_EXPAND);
	sq_newslot(v, -3, false);

	sq_pushstring(v, "ART_CLIENT", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "TOOLBAR",				wxART_TOOLBAR);
	NitBind::newSlot(v, -1, "MENU",					wxART_MENU);
	NitBind::newSlot(v, -1, "FRAME_ICON",			wxART_FRAME_ICON);
	NitBind::newSlot(v, -1, "CMN_DIALOG",			wxART_CMN_DIALOG);
	NitBind::newSlot(v, -1, "HELP_BROWSER",			wxART_HELP_BROWSER);
	NitBind::newSlot(v, -1, "MESSAGE_BOX",			wxART_MESSAGE_BOX);
	NitBind::newSlot(v, -1, "BUTTON",				wxART_BUTTON);
	NitBind::newSlot(v, -1, "LIST",					wxART_LIST);
	NitBind::newSlot(v, -1, "OTHER",				wxART_OTHER);

	sq_newslot(v, -3, false);

	sq_pushstring(v, "COLOR", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "BLACK",				_RGB(*wxStockGDI::GetColour(wxStockGDI::COLOUR_BLACK)));
	NitBind::newSlot(v, -1, "BLUE",					_RGB(*wxStockGDI::GetColour(wxStockGDI::COLOUR_BLUE)));
	NitBind::newSlot(v, -1, "CYAN",					_RGB(*wxStockGDI::GetColour(wxStockGDI::COLOUR_CYAN)));
	NitBind::newSlot(v, -1, "GREEN",				_RGB(*wxStockGDI::GetColour(wxStockGDI::COLOUR_GREEN)));
	NitBind::newSlot(v, -1, "YELLOW",				_RGB(*wxStockGDI::GetColour(wxStockGDI::COLOUR_YELLOW)));
	NitBind::newSlot(v, -1, "LIGHTGREY",			_RGB(*wxStockGDI::GetColour(wxStockGDI::COLOUR_LIGHTGREY)));
	NitBind::newSlot(v, -1, "RED",					_RGB(*wxStockGDI::GetColour(wxStockGDI::COLOUR_RED)));
	NitBind::newSlot(v, -1, "WHITE",				_RGB(*wxStockGDI::GetColour(wxStockGDI::COLOUR_WHITE)));
	sq_newslot(v, -3, false);

	sq_pushstring(v, "SYS_COLOR", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "SCROLLBAR",			_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_SCROLLBAR)));
	NitBind::newSlot(v, -1, "DESKTOP",				_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_DESKTOP)));
	NitBind::newSlot(v, -1, "ACTIVECAPTION",		_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION)));
	NitBind::newSlot(v, -1, "INACTIVECAPTION",		_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_INACTIVECAPTION)));
	NitBind::newSlot(v, -1, "MENU",					_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU)));
	NitBind::newSlot(v, -1, "WINDOW",				_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)));
	NitBind::newSlot(v, -1, "WINDOWFRAME",			_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWFRAME)));
	NitBind::newSlot(v, -1, "MENUTEXT",				_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_MENUTEXT)));
	NitBind::newSlot(v, -1, "WINDOWTEXT",			_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT)));
	NitBind::newSlot(v, -1, "CAPTIONTEXT",			_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_CAPTIONTEXT)));
	NitBind::newSlot(v, -1, "ACTIVEBORDER",			_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVEBORDER)));
	NitBind::newSlot(v, -1, "INACTIVEBORDER",		_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_INACTIVEBORDER)));
	NitBind::newSlot(v, -1, "APPWORKSPACE",			_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE)));
	NitBind::newSlot(v, -1, "HIGHLIGHT",			_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)));
	NitBind::newSlot(v, -1, "HIGHLIGHTTEXT",		_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT)));
	NitBind::newSlot(v, -1, "BTNFACE",				_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE)));
	NitBind::newSlot(v, -1, "BTNSHADOW",			_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW)));
	NitBind::newSlot(v, -1, "GRAYTEXT",				_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT)));
	NitBind::newSlot(v, -1, "BTNTEXT",				_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT)));
	NitBind::newSlot(v, -1, "INACTIVECAPTIONTEXT",	_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_INACTIVECAPTIONTEXT)));
	NitBind::newSlot(v, -1, "BTNHIGHLIGHT",			_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT)));
	NitBind::newSlot(v, -1, "DKSHADOW3D",			_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW)));
	NitBind::newSlot(v, -1, "LIGHT3D",				_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_3DLIGHT)));
	NitBind::newSlot(v, -1, "INFOTEXT",				_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT)));
	NitBind::newSlot(v, -1, "INFOBK",				_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK)));
	NitBind::newSlot(v, -1, "LISTBOX",				_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX)));
	NitBind::newSlot(v, -1, "HOTLIGHT",				_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_HOTLIGHT)));
	NitBind::newSlot(v, -1, "GRADIENTACTIVECAPTION",	_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_GRADIENTACTIVECAPTION)));
	NitBind::newSlot(v, -1, "GRADIENTINACTIVECAPTION",	_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_GRADIENTINACTIVECAPTION)));
	NitBind::newSlot(v, -1, "MENUHILIGHT",			_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_MENUHILIGHT)));
	NitBind::newSlot(v, -1, "MENUBAR",				_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_MENUBAR)));
	NitBind::newSlot(v, -1, "LISTBOXTEXT",			_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT)));
	NitBind::newSlot(v, -1, "LISTBOXHIGHLIGHTTEXT",		_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXHIGHLIGHTTEXT)));

	NitBind::newSlot(v, -1, "BACKGROUND",			_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND)));
	NitBind::newSlot(v, -1, "FACE3D",				_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE)));
	NitBind::newSlot(v, -1, "SHADOW3D",				_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW)));
	NitBind::newSlot(v, -1, "BTNHILIGHT",			_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHILIGHT)));
	NitBind::newSlot(v, -1, "HIGHLIGHT3D",			_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_3DHIGHLIGHT)));
	NitBind::newSlot(v, -1, "HILIGHT3D",			_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_3DHILIGHT)));
	NitBind::newSlot(v, -1, "FRAMEBK",				_RGB(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK)));
	sq_newslot(v, -3, false);

	sq_pushstring(v, "FONT", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "ITALIC",				*wxITALIC_FONT);
	NitBind::newSlot(v, -1, "NORMAL",				*wxNORMAL_FONT);
	NitBind::newSlot(v, -1, "SMALL",				*wxSMALL_FONT);
	NitBind::newSlot(v, -1, "SWISS",				*wxSWISS_FONT);
	sq_newslot(v, -3, false);

	sq_pushstring(v, "CURSOR", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "CROSS",				*wxCROSS_CURSOR);
	NitBind::newSlot(v, -1, "HOURGLASS",			*wxHOURGLASS_CURSOR);
	NitBind::newSlot(v, -1, "STANDARD",				*wxSTANDARD_CURSOR);
	sq_newslot(v, -3, false);

	sq_pushstring(v, "MB", -1);
	sq_newtable(v);
	NitBind::newSlot(v, -1, "CENTER",				(int)wxCENTER);
	NitBind::newSlot(v, -1, "YES",					(int)wxYES);
	NitBind::newSlot(v, -1, "OK",					(int)wxOK);
	NitBind::newSlot(v, -1, "NO",					(int)wxNO);
	NitBind::newSlot(v, -1, "YES_NO",				(int)wxYES_NO);
	NitBind::newSlot(v, -1, "CANCEL",				(int)wxCANCEL);
	NitBind::newSlot(v, -1, "APPLY",				(int)wxAPPLY);
	NitBind::newSlot(v, -1, "CLOSE",				(int)wxCLOSE);
	NitBind::newSlot(v, -1, "OK_DEFAULT",			(int)wxOK_DEFAULT);
	NitBind::newSlot(v, -1, "YES_DEFAULT",			(int)wxYES_DEFAULT);
	NitBind::newSlot(v, -1, "NO_DEFAULT",			(int)wxNO_DEFAULT);
	NitBind::newSlot(v, -1, "CANCEL_DEFAULT",		(int)wxCANCEL_DEFAULT);
	NitBind::newSlot(v, -1, "FORWARD",				(int)wxFORWARD);
	NitBind::newSlot(v, -1, "BACKWARD",				(int)wxBACKWARD);
	NitBind::newSlot(v, -1, "RESET",				(int)wxRESET);
	NitBind::newSlot(v, -1, "HELP",					(int)wxHELP);
	NitBind::newSlot(v, -1, "MORE",					(int)wxMORE);
	NitBind::newSlot(v, -1, "SETUP",				(int)wxSETUP);
	sq_newslot(v, -3, false);

	/*** template
	sq_pushstring(v, "", -1);
	sq_newtable(v);
	NitBind::NewSlot(v, -1, "",			(int)wx);
	NitBind::NewSlot(v, -1, "",			(int)wx);
	NitBind::NewSlot(v, -1, "",			(int)wx);
	NitBind::NewSlot(v, -1, "",			(int)wx);
	NitBind::NewSlot(v, -1, "",			(int)wx);
	sq_newslot(v, -3, false);
	***/

	sq_poptop(v); // "wx"

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
