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

#include "nitwx/NitLibWxNit.h"

#include "nit/net/RemoteDebugger.h"

#ifdef NIT_WIN32
#   include <wx/msw/private.h>
#endif

#include <wx/fontutil.h>

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

wxNitPackDataModel::~wxNitPackDataModel()
{
	LOG(0, "++ wxNitPackDataModel::~wxNitPackDataModel(%08x)\n", this);
}

unsigned int wxNitPackDataModel::GetChildren(const wxDataViewItem& item, wxDataViewItemArray& children) const
{
	if (item.IsOk())
		return 0;

	StreamSourceMap files;
	_pack->findLocal("*", files);
	for (StreamSourceMap::iterator itr = files.begin(), end = files.end(); itr != end; ++itr)
		children.push_back(wxDataViewItem(itr->second));

	return children.Count();
}

bool wxNitPackDataModel::IsContainer(const wxDataViewItem& item) const
{
	if (!item.IsOk())
		return true;

	return false;
}

wxDataViewItem wxNitPackDataModel::GetParent(const wxDataViewItem& item) const
{
	if (!item.IsOk())
		return wxDataViewItem(0);

	PackArchive::File* file = (PackArchive::File*)item.GetID();

	return wxDataViewItem(file->getPack());
}

unsigned int wxNitPackDataModel::GetColumnCount() const
{
	return 12;
}

wxString wxNitPackDataModel::GetColumnType(unsigned int col) const
{
	switch (col)
	{
	case 0:								return "string";		// name
	case 1:								return "string";		// content-type
	case 2:								return "long";			// stream size
	case 3:								return "long";			// memory size
	case 4:								return "datetime";		// timestamp

	case 5:								return "longlong";		// offset
	case 6:								return "string";		// payload type
	case 7:								return "long";			// payload crc32
	case 8:								return "long";			// payload param0
	case 9:								return "long";			// payload param1
	case 10:							return "long";			// source size
	case 11:							return "long";			// source crc32

	default:							return "string";
	}
}

void wxNitPackDataModel::GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const
{
	PackArchive::File* file = (PackArchive::File*)item.GetID();

	switch (col)
	{
	case 0:								variant = wxString(file->getName().c_str()); return;
	case 1:								variant = wxString(file->getContentType().getName().c_str()); return;
	case 2:								variant = (long)file->getStreamSize(); return;
	case 3:								variant = (long)file->getMemorySize(); return;
	case 4:								variant = wxDateTime((time_t)file->getTimestamp().getUnixTime64()); return;

	case 5:								variant = (long)file->getOffset(); return;
	case 6:
		switch (file->getPayloadType())
		{
		case PackArchive::PAYLOAD_RAW:	variant = "raw"; return;
		case PackArchive::PAYLOAD_VOID:	variant = "void"; return;
		case PackArchive::PAYLOAD_ZLIB:	variant = "zlib"; return;
		case PackArchive::PAYLOAD_ZLIB_FAST: variant = "zlib_fast"; return;
		default:						variant = "???"; return;
		}

	case 7:								variant = (long)file->getPayloadCRC32(); return;
	case 8:								variant = (long)file->getPayloadParam0(); return;
	case 9:								variant = (long)file->getPayloadParam1(); return;
	case 10:							variant = (long)file->getSourceSize(); return;
	case 11:							variant = (long)file->getSourceCRC32(); return;

	default:							variant = "???";
	}
}

bool wxNitPackDataModel::SetValue(const wxVariant& variant, const wxDataViewItem& item, unsigned int col)
{
	// read-only
	return false;
}

bool wxNitPackDataModel::IsEnabled(const wxDataViewItem& item, unsigned int col) const
{
	// read only
	return false;
}

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXREF(NITWX_API, NitPackDataModel, wxDataViewModel);

class NB_WxNitPackDataModel : TNitClass<wxNitPackDataModel>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(pack),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(pack: PackArchive)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(pack)					{ return push(v, self(v)->GetPack()); }

	NB_CONS()							{ setSelf(v, new wxNitPackDataModel(get<PackArchive>(v, 2)))->DecRef(); return SQ_OK; }
};

////////////////////////////////////////////////////////////////////////////////

#ifdef NIT_WIN32

wxDEFINE_EVENT(EVT_CONSOLE_KEY, wxKeyEvent);
wxDEFINE_EVENT(EVT_CONSOLE_COMMAND, wxNitConsoleEvent);

class wxNitConsole::ConsoleListener : public ConsoleWindow::IListener
{
public:
	ConsoleListener(wxNitConsole* owner)
		: _owner(owner)
	{
	}

	virtual bool onConsoleKey(ConsoleWindow* con, WPARAM wparam, LPARAM lparam)
	{
		wxKeyEvent evt = _owner->CreateKeyEvent(wxEVT_KEY_DOWN, wparam, lparam);
		evt.SetEventObject(_owner);
		bool handled = _owner->ProcessWindowEvent(evt);

		return handled && !evt.GetSkipped();
	}

	virtual void onConsoleCommand(ConsoleWindow* con, const std::string& cmd)
	{
		wxNitConsoleEvent evt(_owner->GetId(), EVT_CONSOLE_COMMAND, wxString::FromUTF8Unchecked(cmd.c_str(), cmd.size()));
		evt.SetEventObject(_owner);
		evt.ResumePropagation(wxEVENT_PROPAGATE_MAX); // Same propagation level with wxCommandEvent

		bool handled = _owner->ProcessWindowEvent(evt);
	}

	wxNitConsole* _owner;
};

wxNitConsole::wxNitConsole()
{
	Init();
}

wxNitConsole::wxNitConsole(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
: wxWindow(parent, id, pos, size, style)
{
	Init();

	Create(parent, id, pos, size, style);
}

void wxNitConsole::Init()
{
	_listener = new ConsoleListener(this);
}

wxNitConsole::~wxNitConsole()
{
	DetachLogger();
	safeDelete(_listener);
}

bool wxNitConsole::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
	SetSizeHints(200, 200);

	_consoleWin.create(wxGetInstance(), GetHwnd(), ConsoleWindow::CWT_CONTROL);
	_consoleWin.show();

	_consoleWin.setListener(_listener);

	return true;
}

void wxNitConsole::Clear()
{
	_consoleWin.clear();
}

void wxNitConsole::SetPrompt(const wxString& prompt)
{
	_consoleWin.setPrompt((const char*)prompt.utf8_str());
}

wxString wxNitConsole::GetPrompt()
{
	std::string prompt = _consoleWin.getPrompt();
	return wxString::FromUTF8Unchecked(prompt.c_str(), prompt.size());
}

void wxNitConsole::SetCommand(const wxString& cmd)
{
	_consoleWin.setCommand(cmd.utf8_str());
}

wxString wxNitConsole::GetCommand()
{
	std::string cmd = _consoleWin.getCommand();
	return wxString::FromUTF8Unchecked(cmd.c_str(), cmd.size());
}

void wxNitConsole::Print(const wxString& str)
{
	_consoleWin.printString(str.utf8_str());
}

void wxNitConsole::Print(const char* str, int len, const wxColor& color)
{
	_consoleWin.printString(str, len, color.GetRGB());
}

void wxNitConsole::EnterCommand(const wxString& cmd)
{
	_consoleWin.enterCommand((const char*)cmd.utf8_str());
}

void wxNitConsole::DoSetSize(int x, int y, int width, int height, int sizeFlags)
{
	wxWindow::DoSetSize(x, y, width, height, sizeFlags);

	wxRect cr = GetClientRect();
	MoveWindow(_consoleWin.getHwnd(), cr.x, cr.y, cr.width, cr.height, TRUE);
}

void wxNitConsole::SetFocus()
{
	wxWindow::SetFocus();
	_consoleWin.setFocus();
}

bool wxNitConsole::SetFont(const wxFont& f)
{
	if (!wxWindow::SetFont(f)) 
		return false;

	_consoleWin.setFont(f.GetNativeFontInfo()->lf);

	return true;
}

class wxNitConsoleLogger : public Logger
{
public:
	wxNitConsoleLogger(ConsoleWindow* console)
	{
		_console = console;
	}

	virtual void doLog(const LogEntry* entry)
	{
		COLORREF color;

		switch (entry->logLevel)
		{
		case LOG_LEVEL_VERBOSE:			color = RGB(0x80, 0x80, 0x80); break;
		case LOG_LEVEL_DEBUG:			color = RGB(0x00, 0x00, 0x00); break;
		case LOG_LEVEL_INFO:			color = RGB(0x00, 0x80, 0x00); break;
		case LOG_LEVEL_WARNING:			color = RGB(0xFF, 0x00, 0xFF); break;
		case LOG_LEVEL_ERROR:			color = RGB(0xFF, 0x00, 0x00); break;
		case LOG_LEVEL_FATAL:			color = RGB(0xFF, 0x00, 0x00); break;

		default:						color = RGB(0, 0, 0);
		}

		char buf[LogManager::MAX_BUF_SIZE];
		size_t bufSize = sizeof(buf) - 1;

		if (_lineStart)
		{
			int len = formatLog(entry, buf, sizeof(buf));

			if (len > 0)
				_console->printString(buf, len, color);
		}

		_console->printString(entry->message, entry->messageLen, color);

		bool enableErrorLog = false;

		if (enableErrorLog && entry->logLevel >= LOG_LEVEL_ERROR && entry->srcName && entry->fnName)
		{
			int sz = _snprintf(buf, sizeof(buf)-1, " at %s() line %d '%s'",
				entry->fnName, entry->line, entry->srcName);

			if (sz)
				_console->printString(buf, sz, color);
		}

		if (entry->lineEnd)
			_console->printString("\n", -1, color);
	}

private:
	ConsoleWindow* _console;
};

bool wxNitConsole::AttachLogger(LogLevel logLevel)
{
	if (_logger) 
	{
		_logger->setLogLevel(logLevel);
		return false;
	}

	_logger = new wxNitConsoleLogger(&_consoleWin);
	_logger->setLogLevel(logLevel);
	nit::LogManager::getSingleton().attach(_logger);

	return true;
}

void wxNitConsole::DetachLogger()
{
	if (_logger)
	{
		LogManager::getSingleton().detach(_logger);
		_logger = NULL;
	}
}

void wxNitConsole::PrintRemoteLog(const RemoteNotifyEvent* evt)
{
	size_t blobSize;
	const void* blob = evt->param.toBlob(&blobSize);

	RemoteLogEntry entry(blob, blobSize);

	COLORREF color;

	switch (entry.logLevel)
	{
	case LOG_LEVEL_VERBOSE:			color = RGB(0x80, 0x80, 0x80); break;
	case LOG_LEVEL_DEBUG:			color = RGB(0x00, 0x00, 0x00); break;
	case LOG_LEVEL_INFO:			color = RGB(0x00, 0x80, 0x00); break;
	case LOG_LEVEL_WARNING:			color = RGB(0xFF, 0x00, 0xFF); break;
	case LOG_LEVEL_ERROR:			color = RGB(0xFF, 0x00, 0x00); break;
	case LOG_LEVEL_FATAL:			color = RGB(0xFF, 0x00, 0x00); break;

	default:						color = RGB(0, 0, 0);
	}

	if (entry.flags & entry.FLAG_LINESTART)
	{
		char buf[LogManager::MAX_BUF_SIZE];
		size_t bufSize = sizeof(buf) - 1;

		int len = entry.formatHeader(buf, sizeof(buf));

		if (len > 0)
			_consoleWin.printString(buf, len, color);
	}

	_consoleWin.printString(entry.message, entry.messageLen, color);

	if (entry.flags & entry.FLAG_LINEEND)
		_consoleWin.printString("\n", -1, color);
}

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, NitConsole, wxWindow);

class NB_WxNitConsole : TNitClass<wxNitConsole>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(font),
			PROP_ENTRY	(command),
			PROP_ENTRY	(prompt),
			PROP_ENTRY_R(loggerAttached),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: wx.Window=null, id=wx.ID.ANY, pos=null, size=null, style=0)"),
			FUNC_ENTRY_H(print,			"(str: string, color=Color.Black)"),
			FUNC_ENTRY_H(printRemoteLog,"(evt: RemoteNotifyEvent)"),
			FUNC_ENTRY_H(enterCommand,	"(cmd: string)"),
			FUNC_ENTRY_H(attachLogger,	"(logLevel: LogManager.LOG_LEVEL): bool // returns if already attached"),
			FUNC_ENTRY_H(detachLogger,	"()"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "KEY",			(int)EVT_CONSOLE_KEY);
		newSlot(v, -1, "COMMAND",		(int)EVT_CONSOLE_COMMAND);
		sq_poptop(v);
	}

	NB_PROP_GET(font)					{ const wxFont& font = self(v)->GetFont(); return font.IsOk() ? push(v, font) : 0; }
	NB_PROP_GET(command)				{ return push(v, self(v)->GetCommand()); }
	NB_PROP_GET(prompt)					{ return push(v, self(v)->GetPrompt()); }
	NB_PROP_GET(loggerAttached)			{ return push(v, self(v)->IsLoggerAttached()); }

	NB_PROP_SET(font)					{ self(v)->SetFont(*get<wxFont>(v, 2)); return 0; }
	NB_PROP_SET(command)				{ self(v)->SetCommand(getWxString(v, 2)); return 0; }
	NB_PROP_SET(prompt)					{ self(v)->SetPrompt(getWxString(v, 2)); return 0; }

	NB_CONS()
	{
		setSelf(v, new wxNitConsole(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			*opt<wxPoint>(v, 4, wxDefaultPosition),
			*opt<wxSize>(v, 5, wxDefaultSize),
			optInt(v, 6, 0)
			));

		return SQ_OK;
	}

	NB_FUNC(print)
	{
		const char* str = getString(v, 2);
		int len = sq_getsize(v, 2);
		wxColor color = OptWxColor(v, 3, *wxBLACK);

		self(v)->Print(str, len, color); 
		return 0; 
	}

	NB_FUNC(printRemoteLog)				{ self(v)->PrintRemoteLog(get<RemoteNotifyEvent>(v, 2)); return 0; } 

	NB_FUNC(enterCommand)				{ self(v)->EnterCommand(getWxString(v, 2)); return 0; }

	NB_FUNC(attachLogger)				{ return push(v, self(v)->AttachLogger((LogLevel)getInt(v, 2))); }
	NB_FUNC(detachLogger)				{ self(v)->DetachLogger(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, NitConsoleEvent, wxEvent);

class NB_WxNitConsoleEvent : TNitClass<wxNitConsoleEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(command),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(command)				{ return push(v, self(v)->GetCommand()); }
};

#endif

////////////////////////////////////////////////////////////////////////////////

SQRESULT NitLibWxNit(HSQUIRRELVM v)
{
	NB_WxNitPackDataModel::Register(v);

#ifdef NIT_WIN32
	NB_WxNitConsole::Register(v);
	NB_WxNitConsoleEvent::Register(v);
#endif

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
