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

#include "nitwx/NitLibWx.h"

#include "nit/app/PackArchive.h"

#ifdef NIT_WIN32
#   include "nitshell/ConsoleWindow.h"
#endif

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NITWX_API wxNitPackDataModel : public wxDataViewModel
{
public:
	wxNitPackDataModel(PackArchive* pack) : _pack(pack)						{ }
	virtual ~wxNitPackDataModel();

public:
	PackArchive*						GetPack()								{ return _pack; }

public:									// wxDataViewModel impl
	virtual unsigned int				GetChildren(const wxDataViewItem& item, wxDataViewItemArray& children) const;
	virtual bool						IsContainer(const wxDataViewItem& item) const;
	virtual wxDataViewItem				GetParent(const wxDataViewItem& item) const;
	virtual unsigned int				GetColumnCount() const;
	virtual wxString					GetColumnType(unsigned int col) const;
	virtual void						GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const;
	virtual bool						SetValue(const wxVariant& variant, const wxDataViewItem& item, unsigned int col);
	virtual bool						IsEnabled(const wxDataViewItem& item, unsigned int col) const;

protected:
	Ref<PackArchive>					_pack;
};

////////////////////////////////////////////////////////////////////////////////

#ifdef NIT_WIN32

class RemoteNotifyEvent;

class NITWX_API wxNitConsole : public wxWindow
{
public:
	wxNitConsole();

	wxNitConsole(
		wxWindow* parent, 
		wxWindowID id = wxID_ANY, 
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0);

	virtual ~wxNitConsole();

	bool Create(
		wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0);

public:
	void								Clear();

	void								SetPrompt(const wxString& prompt);
	wxString							GetPrompt();

	void								SetCommand(const wxString& cmd);
	wxString							GetCommand();

	void								Print(const wxString& str);
	void								Print(const char* str, int len = -1, const wxColor& color=*wxBLACK);
	void								PrintRemoteLog(const RemoteNotifyEvent* evt);

	void								EnterCommand(const wxString& cmd);

	bool								IsLoggerAttached()						{ return _logger != NULL; }
	bool								AttachLogger(LogLevel logLevel);
	void								DetachLogger();

public:									// wxWindow overrides
	virtual void						DoSetSize(int x, int y, int width, int height, int sizeFlags);
	virtual void						SetFocus();
	virtual bool						SetFont(const wxFont& f);

private:
	ConsoleWindow						_consoleWin;
	class ConsoleListener;
	ConsoleListener*					_listener;
	nit::Ref<Logger>					_logger;

	void								Init();
};

////////////////////////////////////////////////////////////////////////////////

class NITWX_API wxNitConsoleEvent : public wxEvent
{
public:
	wxNitConsoleEvent(int winId, wxEventType evtType, const wxString& cmd)
		: wxEvent(winId, evtType), _command(cmd)
	{
	}

	const wxString&						GetCommand()							{ return _command; }

	virtual wxEvent*					Clone() const							{ return new wxNitConsoleEvent(*this); }

private:
	wxString							_command;
};

wxDECLARE_EXPORTED_EVENT(NITWX_API, EVT_CONSOLE_KEY, wxKeyEvent);
wxDECLARE_EXPORTED_EVENT(NITWX_API, EVT_CONSOLE_COMMAND, wxNitConsoleEvent);

////////////////////////////////////////////////////////////////////////////////

#endif // #ifdef NIT_WIN32

NS_NIT_END;
