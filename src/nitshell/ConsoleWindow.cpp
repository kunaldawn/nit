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

#include "ConsoleWindow.h"

#include "CommCtrl.h"
#include "Richedit.h"

//////////////////////////////////////////////////////////////////////////

ConsoleWindow::ConsoleWindow()
{
	_hwnd = 0;
	_hwndDisplay = 0;
	_hwndCommand = 0;
	_title = "Console";
	_prompt = "> ";
	_historyPos = -1;
	_font = 0;
	_maxLineCount = 1024;
	_printCounter = 0;
	_pendingLines = 0;
	_listener = NULL;
	_lineSpacing = 0;
}

ConsoleWindow::~ConsoleWindow()
{
	if (_font)
		DeleteObject(_font);
}

void ConsoleWindow::create(HINSTANCE hinstance, HWND hwndOwner, WinType winType)
{
	if (_hwnd) return;

	InitCommonControls();
	HMODULE hRichEdit = LoadLibrary( TEXT( "msftedit.dll" ) );
	assert( hRichEdit );

	WNDCLASSEX wcx;
	ZeroMemory(&wcx, sizeof(wcx));

	wcx.cbSize			= sizeof(wcx);
	wcx.style			= CS_HREDRAW | CS_VREDRAW;
	wcx.lpfnWndProc 	= wndProcHook;
	wcx.cbClsExtra		= 0;
	wcx.cbWndExtra		= 0;
	wcx.hInstance		= hinstance;
	wcx.hIcon			= 0;
	wcx.hCursor			= 0;
	wcx.hbrBackground	= 0;
	wcx.lpszClassName	= TEXT("CONSOLEWIN");
	RegisterClassEx(&wcx);
	
	HWND hwnd;

	switch (winType)
	{
	case CWT_CONTROL:
		hwnd = CreateWindowW(
			L"CONSOLEWIN",
			NULL,
			WS_CHILD | WS_VISIBLE,
			0, 0, 0, 0,
			hwndOwner,
			(HMENU)NULL,
			hinstance,
			(LPVOID)this
			);
		break;

	case CWT_DEFAULT:
	case CWT_TOOLWIN:
	default:
		hwnd = CreateWindowW(
			L"CONSOLEWIN",
			toUtf16(_title).c_str(),
			WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			hwndOwner,
			(HMENU)NULL,
			hinstance,
			(LPVOID)this
			);
		if (winType == CWT_TOOLWIN)
			SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW);
		break;
	}

	if (!hwnd) return;

	_hwnd = hwnd;

	setFont("Courier New", 9, 0);

	UpdateWindow(hwnd);

	_visible = false;
}

void ConsoleWindow::setFont(const LOGFONT& lf)
{
	if (_font)
		DeleteObject(_font);

	_font = CreateFontIndirect(&lf);

	SendMessageW(_hwndDisplay, WM_SETFONT, (WPARAM)_font, TRUE);
	SendMessageW(_hwndCommand, WM_SETFONT, (WPARAM)_font, TRUE);
	SendMessageW(_hwndPrompt, WM_SETFONT, (WPARAM)_font, TRUE);
	SendMessageW(_hwndHistory, WM_SETFONT, (WPARAM)_font, TRUE);

	updateRect();

	SendMessageW(_hwndDisplay, EM_SCROLLCARET, 0, 0);
}

void ConsoleWindow::setFont(const char* faceName, int sizeInPt, int lineSpacing)
{
	if (_font)
		DeleteObject(_font);

	HDC dc = GetDC(NULL);
	_font = CreateFontW(
		-MulDiv(sizeInPt, GetDeviceCaps(dc, LOGPIXELSY), 72),
		0,
		0,
		0, 
		FW_DONTCARE,
		0,
		0,
		0,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, 
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		toUtf16(faceName).c_str()
		);
	ReleaseDC(NULL, dc);

	SendMessageW(_hwndDisplay, WM_SETFONT, (WPARAM)_font, TRUE);
	SendMessageW(_hwndCommand, WM_SETFONT, (WPARAM)_font, TRUE);
	SendMessageW(_hwndPrompt, WM_SETFONT, (WPARAM)_font, TRUE);
	SendMessageW(_hwndHistory, WM_SETFONT, (WPARAM)_font, TRUE);

	_lineSpacing = lineSpacing;

	updateRect();

	SendMessageW(_hwndDisplay, EM_SCROLLCARET, 0, 0);
}

void ConsoleWindow::setFocus()
{
	::SetFocus(_hwndCommand);
}

void ConsoleWindow::show()
{
	ShowWindow(_hwnd, SW_SHOWDEFAULT);
	BringWindowToTop(_hwnd);
	_visible = true;

	if (_listener)
		_listener->onConsoleShow(this);
}

bool ConsoleWindow::isVisible()
{
	return _visible;
}

void ConsoleWindow::hide()
{
	ShowWindow(_hwnd, SW_HIDE);
	_visible = false;

	if (_listener)
		_listener->onConsoleHide(this);
}

void ConsoleWindow::clear()
{
	SendMessageW(_hwndDisplay, WM_SETTEXT, 0, (LPARAM)L"");
}

LRESULT CALLBACK ConsoleWindow::wndProcHook(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ConsoleWindow* thisWindow = NULL;

	if (message == WM_CREATE)
	{
		thisWindow = (ConsoleWindow*) ((CREATESTRUCT*)lParam)->lpCreateParams;
		SetWindowLong(hwnd, GWL_USERDATA, (LONG)thisWindow);
	}
	else
	{
		thisWindow = (ConsoleWindow*)GetWindowLong(hwnd, GWL_USERDATA);
	}

	if (thisWindow == NULL)
	{
		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	return thisWindow->windowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK ConsoleWindow::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		// Log window
		// NOTE: We need ES_NOHIDESEL to provide autoscroll on Richedit 5.0
		_hwndDisplay = CreateWindowW(
			MSFTEDIT_CLASS,
			NULL,
			WS_CHILD | WS_VISIBLE | WS_VSCROLL |
			ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | ES_NOHIDESEL,
			0, 0, 0, 0,
			hwnd,
			0,
			(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
			NULL
			);

		SetWindowLong(_hwndDisplay, GWL_USERDATA, (LONG)this);

		_displayOrigWndProc = (WNDPROC)SetWindowLong(_hwndDisplay, GWL_WNDPROC, (LONG) subclassProcHook);

		SendMessageW(_hwndDisplay, EM_SETEDITSTYLE, SES_NOIME, SES_NOIME);

		// Command line
		_hwndCommand = CreateWindowW(
			MSFTEDIT_CLASS,
			NULL,
			WS_CHILD | WS_VISIBLE |
			ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_NOHIDESEL ,
			0, 0, 0, 0,
			hwnd,
			0,
			(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
			NULL
			);

		SetWindowLong(_hwndCommand, GWL_USERDATA, (LONG)this);
		_commandOrigWndProc = (WNDPROC)SetWindowLong(_hwndCommand, GWL_WNDPROC, (LONG) subclassProcHook);

//		SendMessageW(_hwndCommand, EM_SETEDITSTYLE, SES_NOIME, SES_NOIME);

		// Prompt
		_hwndPrompt = CreateWindowW(
			L"STATIC",
			NULL,
			WS_CHILD | WS_VISIBLE | 
			SS_LEFT,
			0, 0, 0, 0,
			hwnd,
			0,
			(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
			NULL
			);

		SendMessageW(_hwndPrompt, WM_SETTEXT, 0, (LPARAM)toUtf16(_prompt).c_str());
		SetWindowLong(_hwndPrompt, GWL_USERDATA, (LONG)this);
		_promptOrigWndProc = (WNDPROC)SetWindowLong(_hwndPrompt, GWL_WNDPROC, (LONG) subclassProcHook);

		// History
		_hwndHistory = CreateWindowW(
			L"LISTBOX",
			NULL,
			WS_CHILD |
			LBS_NOTIFY | LBS_EXTENDEDSEL | ES_AUTOVSCROLL ,
			0, 0, 0, 0,
			hwnd,
			0,
			(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
			NULL
			);

		SetWindowLong(_hwndHistory, GWL_USERDATA, (LONG)this);
		_historyOrigWndProc = (WNDPROC)SetWindowLong(_hwndHistory, GWL_WNDPROC, (LONG) subclassProcHook);

		for (size_t i=0; i<_history.size(); ++i)
		{
			SendMessageW(_hwndHistory, LB_ADDSTRING, 0, (LPARAM)toUtf16(_history[i]).c_str());
		}

		return 0;

	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;
	case WM_SIZE:
		updateRect();
		return 0;
	case WM_ERASEBKGND:
		return 0;
	case WM_SETFOCUS:
		::SetFocus(_hwndCommand);
		break;
	case WM_CLOSE:
		hide();
		return 0;

	case WM_COMMAND:
		switch (HIWORD(wParam))
		{
		case LBN_SELCHANGE:
			if (SendMessageW(_hwndHistory, LB_GETSELCOUNT, 0, 0))
			{
				bool useAnchor = false;

				int sel = -1;

				if (useAnchor)
					sel = SendMessageW(_hwndHistory, LB_GETANCHORINDEX, 0, 0);

				if (sel == -1 || SendMessageW(_hwndHistory, LB_GETSEL, sel, 0) == 0)
					SendMessageW(_hwndHistory, LB_GETSELITEMS, 1, (LPARAM)&sel);

				setCommand(_history[sel].c_str(), false);
			}
			break;

		case LBN_SELCANCEL:
			setCommand("", false);
			break;
		}
		break;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT ConsoleWindow::subclassProcHook(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ConsoleWindow* thisWindow = (ConsoleWindow*)GetWindowLong(hwnd, GWL_USERDATA);

	if (thisWindow)
	{
		if (hwnd == thisWindow->_hwndCommand)
			return thisWindow->commandSubclassProc(hwnd, message, wParam, lParam);
		else if (hwnd == thisWindow->_hwndDisplay)
			return thisWindow->displaySubclassProc(hwnd, message, wParam, lParam);
		else if (hwnd == thisWindow->_hwndPrompt)
			return thisWindow->promptSubclassProc(hwnd, message, wParam, lParam);
		else if (hwnd == thisWindow->_hwndHistory)
			return thisWindow->historySubclassProc(hwnd, message, wParam, lParam);
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT ConsoleWindow::displaySubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CHAR:
		switch (wParam)
		{
		case 'A' - 64:	// ctrl + A
			// select all
			SendMessageW(_hwndDisplay, EM_SETSEL, 0, INT_MAX);
			return 0;
		case 'C' - 64:	// ctrl + C
			// Handle as usual
			break; 
		default:
			// Forcibly send focus
			::SetFocus(_hwndCommand);
			SendMessageW(_hwndCommand, message, wParam, lParam);
			return 0;  
		}
		break;
	}

	return CallWindowProc(_displayOrigWndProc, hwnd, message, wParam, lParam);
}

int ConsoleWindow::getCmdLineCount()
{
	return SendMessageW(_hwndCommand, EM_GETLINECOUNT, 0, 0);
}

std::string ConsoleWindow::getCommand()
{
	int numChar = (int)SendMessageW(_hwndCommand, WM_GETTEXTLENGTH, 0, 0);
	std::vector<wchar_t> buf;
	buf.resize(numChar+1);
	SendMessageW(_hwndCommand, WM_GETTEXT, (WPARAM)numChar+1, (LPARAM)&buf[0]);
	buf[numChar] = 0;

	return toUtf8(&buf[0]);
}

void ConsoleWindow::setCommand(const char* cmd, bool updateRect)
{
	SendMessageW(_hwndCommand, WM_SETTEXT, 0, (LPARAM)toUtf16(cmd).c_str());

	CHARRANGE cr = { -1, -2 };
	SendMessageW(_hwndCommand, EM_EXSETSEL, 0, (LPARAM)&cr);
	SendMessageW(_hwndCommand, EM_SCROLLCARET, 0, 0L);

	if (updateRect)
	{
		if (IsWindowVisible(_hwndHistory))
		{
			SendMessage(_hwndHistory, LB_SELITEMRANGE, FALSE, 0xFFFF0000);
			ShowWindow(_hwndHistory, SW_HIDE);
		}

		this->updateRect();
	}
}

LRESULT ConsoleWindow::commandSubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_UP:
		case VK_DOWN:
			if ((getCmdLineCount() == 1 || (GetAsyncKeyState(VK_CONTROL) & 0x8000) || (GetAsyncKeyState(VK_MENU) & 0x8001)) 
				&& !_history.empty())
			{
				int index = _historyPos;
				if (wParam == VK_UP) ++index;
				if (wParam == VK_DOWN) --index;

				index = clamp(index, 0, (int)_history.size()-1);

				_historyPos = index;

				int itemIndex = _history.size() - 1 - index;
				setCommand(_history[itemIndex].c_str(), false);

				if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) == 0)
				{
					SendMessage(_hwndHistory, LB_SELITEMRANGE, FALSE, 0xFFFF0000);
				}

				SendMessage(_hwndHistory, LB_SETSEL, TRUE, itemIndex);

				ShowWindow(_hwndHistory, SW_SHOW);
				updateRect();
				return 0;
			}
			break;
		case VK_PRIOR:
			SendMessageW(_hwndDisplay, EM_SCROLL, SB_PAGEUP, 0);
			return 0;
		case VK_NEXT:
			SendMessageW(_hwndDisplay, EM_SCROLL, SB_PAGEDOWN, 0);
			return 0;
		case VK_ESCAPE:
			setCommand("");
			return 0;
		case VK_RETURN:
			// NOTE: If we handle this on WM_CHAR, enter key will be consume and then get here (on RichEdit 5.0)
			// So we have to do it here
			if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) || (GetAsyncKeyState(VK_CONTROL) & 0x8000))
			{
				break;
			}
			else
			{
				if (!enterHistoryCommands(1))
				{
					int numChar = (int)SendMessageW(_hwndCommand, WM_GETTEXTLENGTH, 0, 0);
					wchar_t* buf = new wchar_t[numChar+1];
					SendMessageW(_hwndCommand, WM_GETTEXT, (WPARAM)numChar+1, (LPARAM)buf);
					buf[numChar] = 0;

					enterCommand(toUtf8(buf));
					setCommand("");
					delete[] buf;
				}
			}
			return 0;
		}
		if (onKey(wParam, lParam)) return 0;
		break;
	case WM_CHAR:
		{
			if (IsWindowVisible(_hwndHistory))
			{
				switch (wParam)
				{
				case 'A' - 64: // ctrl + a
					SendMessage(_hwndHistory, LB_SELITEMRANGE, TRUE, 0xFFFF0000);
					return 0;
				case 'C' - 64: // ctrl + c
					if (copyClipboardHistoryCommands(1))
						return 0;
				}

				SendMessage(_hwndHistory, LB_SELITEMRANGE, FALSE, 0xFFFF0000);
				ShowWindow(_hwndHistory, SW_HIDE);
				updateRect();
			}

			switch (wParam)
			{
			case 'A' - 64: // ctrl + a
				SendMessageW(_hwndCommand, EM_SETSEL, 0, INT_MAX);
				return 0;
			case 0x7F: // ctrl + backspace
				{
					DWORD start, end;
					SendMessageW(_hwndCommand, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
					SendMessageW(_hwndCommand, WM_KEYDOWN, VK_LEFT, 0);
					SendMessageW(_hwndCommand, EM_GETSEL, (WPARAM)&start, NULL);
					SendMessageW(_hwndCommand, EM_SETSEL, start, end);
					SendMessageW(_hwndCommand, EM_REPLACESEL, TRUE, (LPARAM)L"");
				}
				return 0;
			default:
				_historyPos = -1;
				break;
			}
		}
	}

	LRESULT ret = CallWindowProc(_commandOrigWndProc, hwnd, message, wParam, lParam);

	switch (message)
	{
	case WM_KEYDOWN:
	case WM_CHAR:
		if (wParam <= 127)
			updateRect();
		break;
	}

	return ret;
}

LRESULT ConsoleWindow::promptSubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
		PAINTSTRUCT ps;
		HDC dc = BeginPaint(hwnd, &ps);
		RECT rt;
		GetClientRect(hwnd, &rt);
		FillRect(dc, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));
		SelectObject(dc, _font);
		std::wstring prompt = toUtf16(_prompt);
		DrawTextW(dc, prompt.c_str(), (int)prompt.size(), &rt, 0);
		EndPaint(hwnd, &ps);
		return 0;
	}
	return CallWindowProc(_promptOrigWndProc, hwnd, message, wParam, lParam);
}

LRESULT ConsoleWindow::historySubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			setCommand("");
			return 0;

		case VK_DELETE:
			deleteHistoryCommands(0);
			return 0;

		case VK_RETURN:
			enterHistoryCommands(0);
			return 0;
		}
		break;

	case WM_LBUTTONDBLCLK:
		enterHistoryCommands(0);
		return 0;

	case WM_CHAR:
		switch (wParam)
		{
		case 'A' - 64: // ctrl + a
			SendMessage(_hwndHistory, LB_SELITEMRANGE, TRUE, 0xFFFF0000);
			return 0;
		case 'C' - 64: // ctrl + c
			copyClipboardHistoryCommands(0);
			return 0;
		}
	}

	return CallWindowProc(_historyOrigWndProc, hwnd, message, wParam, lParam);
}

bool ConsoleWindow::deleteHistoryCommands(size_t ifMoreThan)
{
	size_t numSel = SendMessageW(_hwndHistory, LB_GETSELCOUNT, 0, 0);
	if (numSel <= ifMoreThan) return false;

	std::vector<int> sel;
	sel.resize(numSel);

	SendMessage(_hwndHistory, LB_GETSELITEMS, numSel, (LPARAM)&sel[0]);

	std::sort(sel.begin(), sel.end());

	for (int i = (int)numSel - 1; i >= 0; --i)
	{
		_history.erase(_history.begin() + sel[i]);
		SendMessage(_hwndHistory, LB_DELETESTRING, sel[i], 0);
	}

	return true;
}

bool ConsoleWindow::copyClipboardHistoryCommands(size_t ifMoreThan)
{
	size_t numSel = SendMessageW(_hwndHistory, LB_GETSELCOUNT, 0, 0);
	if (numSel <= ifMoreThan) return false;

	std::vector<int> sel;
	sel.resize(numSel);

	SendMessage(_hwndHistory, LB_GETSELITEMS, numSel, (LPARAM)&sel[0]);

	std::wstring text;

	for (size_t i=0; i < numSel; ++i)
	{
		if (i > 0)
			text.append(L"\r\n");

		text.append(toUtf16(_history[sel[i]]));
	}

	// the text should be placed in "global" memory 
	HGLOBAL hMem = GlobalAlloc(GHND, (text.length()+1)*sizeof(wchar_t)); 
	wchar_t* clipStr = (wchar_t*)GlobalLock(hMem);
	memcpy(clipStr, text.c_str(), (text.length()+1) * sizeof(wchar_t));
	GlobalUnlock(hMem);

	if (OpenClipboard(_hwnd))
	{
		EmptyClipboard();
		SetClipboardData(CF_UNICODETEXT, hMem);
		CloseClipboard();
	}
	else
	{
		GlobalFree(hMem);
	}

	return true;
}

bool ConsoleWindow::enterHistoryCommands(size_t ifMoreThan)
{
	size_t numSel = SendMessageW(_hwndHistory, LB_GETSELCOUNT, 0, 0);
	if (numSel <= ifMoreThan) return false;

	std::vector<int> sel;
	sel.resize(numSel);

	SendMessage(_hwndHistory, LB_GETSELITEMS, numSel, (LPARAM)&sel[0]);

	std::vector<std::string> commands;
	for (size_t i=0; i<numSel; ++i)
		commands.push_back(_history[sel[i]]);

	for (size_t i=0; i<commands.size(); ++i)
	{
		enterCommand(commands[i]);
	}
	setCommand("");

	return true;
}

void ConsoleWindow::enterCommand(const std::string& command)
{
	if (onCommand(command))
	{
		// Assume that the index matches perfectly between _history and history list.
		std::deque<std::string>::iterator itr = std::find(_history.begin(), _history.end(), command);
		if (itr != _history.end()) 
		{
			int index = itr - _history.begin();
			SendMessage(_hwndHistory, LB_DELETESTRING, index, 0);
			_history.erase(itr);
		}

		if (!command.empty())
		{
			_history.push_back(command);
			_historyPos = -1;

			SendMessage(_hwndHistory, LB_ADDSTRING, 0, (LPARAM)toUtf16(command).c_str());
		}
	}
}

void ConsoleWindow::echo(const std::string& command)
{
	printString(_prompt.c_str());
	printString(command.c_str());
	printString("\r\n");
}

void ConsoleWindow::beginPrint()
{
}

void ConsoleWindow::endPrint()
{
}

void ConsoleWindow::printString(const char* str, int len, COLORREF color)
{
	if (len == -1) len = strlen(str);

	CHARFORMAT cf = { 0 };
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = 0;
	cf.crTextColor = color;
	SendMessage(_hwndDisplay, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

	// Move the selection to the end
	CHARRANGE cr = { -1, -2 };
	SendMessageW(_hwndDisplay, EM_EXSETSEL, 0, (LPARAM)&cr);

	// Add the text and scroll it into view
	SendMessageW(_hwndDisplay, EM_REPLACESEL, 0, (LONG) (LPSTR) toUtf16(str, len).c_str());
	SendMessageW(_hwndDisplay, EM_EXSETSEL, 0, (LPARAM)&cr);
	SendMessageW(_hwndDisplay, EM_SCROLLCARET, 0, 0L);

	++_pendingLines;
}

void ConsoleWindow::setTitle(const std::string& title)
{
	_title = title;

	SetWindowTextW(_hwnd, toUtf16(_title).c_str());
}

void ConsoleWindow::setPrompt(const std::string& prompt)
{
	_prompt = prompt;
	SendMessageW(_hwndPrompt, WM_SETTEXT, 0, (LPARAM)toUtf16(_prompt).c_str());
	updateRect();
}

void ConsoleWindow::updateRect()
{
	RECT rt;
	GetClientRect(_hwnd, &rt);
	int width = rt.right - rt.left;
	int height = rt.bottom - rt.top;

	SIZE promptSize;
	HDC promptDC = GetDC(_hwndPrompt);
	SelectObject(promptDC, _font);
	GetTextExtentPoint32A(promptDC, _prompt.c_str(), (int)_prompt.size(), &promptSize);
	ReleaseDC(_hwndPrompt, promptDC);

	promptSize.cy += _lineSpacing;

	int cmdLineCount = getCmdLineCount();

	int cmdHeight = promptSize.cy * (cmdLineCount + 1);

	int displayHeight = height - cmdHeight;

	if (IsWindowVisible(_hwndHistory))
	{
		int historyHeight = (height - cmdHeight) / 4;
		if (historyHeight < 100) historyHeight = 100;
		::MoveWindow(_hwndHistory, 0, (height - cmdHeight) - historyHeight, width, historyHeight, TRUE);
		displayHeight -= historyHeight;
	}

	::MoveWindow(_hwndDisplay, 0, 0, width, displayHeight, TRUE);

	::MoveWindow(_hwndCommand, promptSize.cx, height - cmdHeight, width - promptSize.cx, cmdHeight, TRUE);
	::MoveWindow(_hwndPrompt, 0, height - cmdHeight, promptSize.cx, cmdHeight, TRUE);

	SendMessageW(_hwndDisplay, EM_SCROLLCARET, 0, 0);

	POINT pt = { 0, 0 };
	SendMessageW(_hwndCommand, EM_SETSCROLLPOS, 0, (LPARAM) &pt);
	SendMessageW(_hwndCommand, EM_SCROLLCARET, 0, 0);
}

void ConsoleWindow::moveWindow(int x, int y, int width, int height)
{
	::MoveWindow(_hwnd, x, y, width, height, TRUE);
}

bool ConsoleWindow::onCommand(const std::string& command)
{
	echo(command);
	
	if (command == "clear")
	{
		clear();
		return true;
	}

	if (_listener)
		_listener->onConsoleCommand(this, command);

	return true;
}

bool ConsoleWindow::onKey(WPARAM wparam, LPARAM lparam)
{
	if (_listener) 
		return _listener->onConsoleKey(this, wparam, lparam);

	return false;
}

void ConsoleWindow::setListener(IListener* listener)
{
	_listener = listener;
}

int ConsoleWindow::run(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPCSTR lpCmdLine, int nCmdShow)
{
	if (_hwnd == NULL)
	{
		create(hInstance, 0);
	}

	show();

	// TODO: Need we this?
	HACCEL accelTable = NULL;

	// Main message loop:
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);

	while (msg.message != WM_QUIT && _visible)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			if (!preTranslateMessage(&msg))
			{
				if (accelTable == NULL || msg.hwnd == NULL || !TranslateAccelerator(msg.hwnd, accelTable, &msg)) 
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		}
		else
		{
			Sleep(1);
		}

		if (_listener)
			_listener->onConsoleUpdate(this);
	}

	return (int)msg.wParam;
}

bool ConsoleWindow::preTranslateMessage(MSG* msg)
{
	return false;
}

std::wstring ConsoleWindow::toUtf16(const char* utf8str, int len)
{
	len = len ? len : strlen(utf8str);

	// determine length of buffer
	int bufSize = MultiByteToWideChar(CP_UTF8, 0, utf8str, len, 0, 0);
	wchar_t *buf = new wchar_t[bufSize + 1];

	// Convert to UTF16
	MultiByteToWideChar(CP_UTF8, 0, utf8str, len, buf, bufSize);
	buf[bufSize] = 0;

	std::wstring ret;
	ret.assign(buf, bufSize);

	delete[] buf;

	return ret;
}

std::string ConsoleWindow::toUtf8( const wchar_t* utf16str, int len/*=0*/ )
{
	len = len ? len : wcslen(utf16str);

	// determine length of buffer
	int bufSize = WideCharToMultiByte(CP_UTF8, 0, utf16str, len, 0, 0, 0, 0);
	char *buf = new char[bufSize + 1];

	// Convert to UTF16
	WideCharToMultiByte(CP_UTF8, 0, utf16str, len, buf, bufSize, 0, 0);
	buf[bufSize] = 0;

	std::string ret;
	ret.assign(buf, bufSize);

	delete[] buf;

	return ret;
}

static bool ReadLine(HANDLE file, std::string& outString)
{
	char buf[4096];
	DWORD readCount;

	outString.resize(0);

	while (true)
	{
		if (ReadFile(file, buf, sizeof(buf)-1, &readCount, NULL) == 0) return true;
		if (readCount == 0) return true;

		buf[readCount] = 0;

		char* p = strchr(buf, '\n');
		if (p != 0)
		{
			SetFilePointer(file, (long)(p + 1 - buf - readCount), 0, FILE_CURRENT);
			*p = 0;
		}

		outString += buf;

		if (p != 0)
		{
			if (outString.length() && outString[outString.length()-1] == '\r')
			{
				outString.resize(outString.length()-1);
			}

			break;
		}
	}

	return false;
}

void ConsoleWindow::loadHistory(const char* filename)
{
	HANDLE file = CreateFileW(toUtf16(filename).c_str(),
		GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	if (file == INVALID_HANDLE_VALUE)
		return;

	unsigned char bom[3];
	DWORD readCount = 0;

	bool end = ReadFile(file, bom, 3, &readCount, NULL) == 0;

	bool rewind = true;
	bool utf8 = false;

	if (!end && readCount == 3)
	{
		if (bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)
		{
			// UTF-8 BOM
			rewind = false;
			utf8 = true;
		}
	}

	if (rewind)
		SetFilePointer(file, 0, 0, FILE_BEGIN);

	std::string history;
	std::string line;

	while (!end)
	{
		end = ReadLine(file, line);

		if (line.empty())
		{
			if (!history.empty())
				_history.push_back(history);

			history.resize(0);
		}
		else
		{
			if (!history.empty())
				history.append("\r\n");
			history.append(line);
		}
	}

	CloseHandle(file);
}

void ConsoleWindow::saveHistory(const char* filename)
{
	HANDLE file = CreateFileW(toUtf16(filename).c_str(),
		GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);

	if (file == INVALID_HANDLE_VALUE)
		return;

	DWORD written = 0;

	// write utf-8 bom header
	unsigned char bom[3] = { 0xEF, 0xBB, 0xBF };
	WriteFile(file, bom, 3, &written, NULL);

	for (size_t i=0; i<_history.size(); ++i)
	{
		WriteFile(file, _history[i].c_str(), _history[i].size(), &written, NULL);
		WriteFile(file, "\r\n\r\n", 4, &written, NULL);
	}

	CloseHandle(file);
}
