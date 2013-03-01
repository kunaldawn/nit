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

#include <windows.h>
#include <string>
#include <deque>
#include <vector>
#include <algorithm>
#include <assert.h>

#ifndef COUNT_OF
#define COUNT_OF(x) (sizeof(x)/sizeof(*x))
#endif

//////////////////////////////////////////////////////////////////////////

class ConsoleWindow
{
public:
	enum WinType
	{
		CWT_DEFAULT = 0x00,
		CWT_CONTROL = 0x01,
		CWT_TOOLWIN = 0x02,
	};

public:
	class IListener
	{
	public:
		virtual ~IListener() { }

		virtual bool onConsoleKey(ConsoleWindow* con, WPARAM wparam, LPARAM lparam)		{ return false; }
		virtual void onConsoleCommand(ConsoleWindow* con, const std::string& cmd)		{ }
		virtual void onConsoleShow(ConsoleWindow* con)									{ }
		virtual void onConsoleHide(ConsoleWindow* con)									{ }
		virtual void onConsoleUpdate(ConsoleWindow* con)								{ }
	};

public:
	ConsoleWindow();
	virtual ~ConsoleWindow();

public:
	virtual void						create(HINSTANCE hinstance, HWND hwndOwner, WinType winType = CWT_DEFAULT);
	void 								show();
	void								hide();
	void								clear();
	bool								isVisible();

	HWND								getHwnd()												{ return _hwnd; }

	void								setFont(const char* faceName, int sizeInPt, int lineSpacing);
	void								setFont(const LOGFONT& lf);

	void								setFocus();

	void								setListener(IListener* listener);

	std::string							getCommand();
	void								setCommand(const char* cmd, bool updateRect = true);

	int									run(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPCSTR lpCmdLine, int nCmdShow);

public:
	void 								setTitle(const std::string& title);
	const std::string&					getTitle()												{ return _title; }

	void 								setPrompt(const std::string& prompt);
	const std::string&					getPrompt()												{ return _prompt; }

	void 								enterCommand(const std::string& command);

	void 								printString(const char* str, int len = -1, COLORREF color = RGB(0x00, 0x00, 0x00));

	void 								moveWindow(int x, int y, int width, int height);

	void								beginPrint();
	void								endPrint();

	std::wstring						toUtf16(const std::string& utf8str)		{ return toUtf16(utf8str.c_str(), utf8str.length()); }
	std::wstring						toUtf16(const char* utf8str, int len=0);

	std::string							toUtf8(const std::wstring& utf16str)		{ return toUtf8(utf16str.c_str(), utf16str.length()); }
	std::string							toUtf8(const wchar_t* utf16str, int len=0);

	void								saveHistory(const char* filename);
	void								loadHistory(const char* filename);

protected:
	virtual LRESULT CALLBACK			windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	virtual LRESULT CALLBACK			displaySubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	virtual LRESULT CALLBACK			commandSubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	virtual LRESULT CALLBACK			promptSubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	virtual LRESULT CALLBACK			historySubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	void 								echo(const std::string& command);
	void 								updateRect();

	virtual bool 						onCommand(const std::string& command);
	virtual bool 						onKey(WPARAM wparam, LPARAM lparam);

	std::deque<std::string>				_history;
	int									_historyPos;

	HWND 								_hwnd;
	HWND 								_hwndDisplay;
	HWND 								_hwndCommand;
	HWND 								_hwndPrompt;
	HWND								_hwndHistory;

	std::string 						_title;
	std::string 						_prompt;
	int									_maxLineCount;
	int									_printCounter;
	int									_pendingLines;
	
	int									_lineSpacing;

	int									getCmdLineCount();

private:
	static LRESULT CALLBACK 			wndProcHook(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK 			subclassProcHook(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	WNDPROC 							_displayOrigWndProc;
	WNDPROC 							_commandOrigWndProc;
	WNDPROC 							_promptOrigWndProc;
	WNDPROC								_historyOrigWndProc;

	HFONT								_font;

	IListener*							_listener;
	bool								_visible;

	template <typename T>
	static inline const T&				clamp(const T& value, const T& min, const T& max)	{ return value < min ? min : value > max ? max : value; }

	bool								preTranslateMessage(MSG* msg);

	bool								enterHistoryCommands(size_t ifMoreThan);
	bool								copyClipboardHistoryCommands(size_t ifMoreThan);
	bool								deleteHistoryCommands(size_t ifMoreThan);
};

//////////////////////////////////////////////////////////////////////////

