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

#include "NITRENDER_pch.h"

#include "nitrender/GLESRenderView_win32.h"

#include "nitrender/RenderContext.h"

#include "nit/input/InputDevice.h"
#include "nit/input/InputUser.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

static void PVRFrameEnableControlWindow(bool bEnable)
{
	HKEY hKey = 0;

	// Open PVRFrame control key, if not exist create it.
	if(ERROR_SUCCESS != RegCreateKeyExW(HKEY_CURRENT_USER,
		L"Software\\Imagination Technologies\\PVRVFRame\\STARTUP\\",
		0,
		0,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		0,
		&hKey,
		NULL))
	{
		return;
	}

	const UniChar * wszValue = L"hide_gui";
	const UniChar * wszNewData = (bEnable) ? L"NO" : L"YES";
	UniChar wszOldData[256] = {0};
	DWORD   dwSize = sizeof(wszOldData);
	LSTATUS status = RegQueryValueExW(hKey, wszValue, 0, NULL, (LPBYTE)wszOldData, &dwSize);
	if (ERROR_FILE_NOT_FOUND == status              // the key not exist
		|| (ERROR_SUCCESS == status                 // or the hide_gui value is exist
		&& 0 != wcscmp(wszNewData, wszOldData)))    // but new data and old data not equal
	{
		dwSize = sizeof(UniChar) * (wcslen(wszNewData) + 1);
		RegSetValueEx(hKey, wszValue, 0, REG_SZ, (const BYTE *)wszNewData, dwSize);
	}

	RegCloseKey(hKey);
}

////////////////////////////////////////////////////////////////////////////////

GLESRenderView_win32::WindowClass* GLESRenderView_win32::WindowClass::obtain()
{
	Weak<WindowClass> s_WindowClass;

	if (s_WindowClass == NULL)
		s_WindowClass = new WindowClass();

	return s_WindowClass;
};

GLESRenderView_win32::WindowClass::WindowClass()
{
	_hinstance = GetModuleHandle(NULL);
	WNDCLASS wc;

	// Redraw On Size, And Own DC For Window.
	wc.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;  
	wc.lpfnWndProc    = windowProcCallback;				// WndProc Handles Messages
	wc.cbClsExtra     = 0;                              // No Extra Window Data
	wc.cbWndExtra     = 0;								// No Extra Window Data
	wc.hInstance      = _hinstance;						// Set The Instance
	wc.hIcon          = LoadIcon( NULL, IDI_WINLOGO );	// Load The Default Icon
	wc.hCursor        = LoadCursor( NULL, IDC_ARROW );	// Load The Arrow Pointer
	wc.hbrBackground  = NULL;                           // No Background Required For GL
	wc.lpszMenuName   = NULL;                           // We Don't Want A Menu
	wc.lpszClassName  = getClassName();                 // Set The Class Name

	if (!RegisterClass(&wc) && GetLastError() != 1410)
		NIT_THROW_FMT(EX_SYSTEM, "!!! can't register render view window class\n");
}

GLESRenderView_win32::WindowClass::~WindowClass()
{
	UnregisterClass(getClassName(), GetModuleHandle(NULL));
}

HWND GLESRenderView_win32::WindowClass::newWindow(const String& title, LPVOID lpParam)
{
	// center window position
	RECT desktopRect;
	GetWindowRect(GetDesktopWindow(), &desktopRect);

	// create window
	HWND hWnd = CreateWindowEx(
		WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,	// Extended Style For The Window
		getClassName(),						// Class Name
		Unicode::toUtf16(title).c_str(),					// Window Title
		WS_CAPTION | WS_POPUPWINDOW | WS_MINIMIZEBOX,		// Defined Window Style
		0, 0,								                // Window Position
		0,                                                  // Window Width
		0,                                                  // Window Height
		NULL,												// No Parent Window
		NULL,												// No Menu
		_hinstance,										// Instance
		lpParam);

	return hWnd;
}

////////////////////////////////////////////////////////////////////////////////

class GLESRenderDevice_win32 : public GLESRenderDevice
{
public:
	virtual bool getNativeDisplay()
	{
		_nativeDisplay = GetDC(_nativeWindow);

		if (_nativeDisplay == 0) 
		{
			LOG(0, "!!! can't get native display for egl\n");
			return false;
		}

		return true;
	}

	virtual void releaseNativeDisplay()
	{
		ReleaseDC(_nativeWindow, _nativeDisplay);
	}

	virtual EGLint chooseConfig()
	{
		const EGLint attribs[] =
		{
			EGL_LEVEL,				0,
			EGL_SURFACE_TYPE,		EGL_WINDOW_BIT,
			EGL_RENDERABLE_TYPE,	EGL_OPENGL_ES2_BIT,
			EGL_NATIVE_RENDERABLE,	EGL_FALSE,	// pvrframe supports emulation only
			EGL_DEPTH_SIZE,			16,
			EGL_NONE
		};

		EGLint numConfigs = 0;

		eglChooseConfig(_display, attribs, &_config, 1, &numConfigs);
		return numConfigs;
	}
};

////////////////////////////////////////////////////////////////////////////////

class WindowInputDevice : public InputDevice
{
public:
	class TouchPointer : public InputPointer
	{
	public:
		TouchPointer(const String& name) : InputPointer(name) { }

	public:
		void move(bool on, const Vector3& newPos)
		{
			InputService* svc = svc_Input;

			Ref<InputPointerEvent> evt = new InputPointerEvent(this, _value, newPos);

			bool nowOn = !_on && on;
			bool nowOff = _on && !on;
			bool moved = _on == on && _value.squaredDistance(newPos) > 0.001f;

			_on = on;
			_value = newPos;

			if (nowOn)
				svc->post(EVT::POINTER_ON, evt);

			if (moved)
			{
				if (on)
					svc->post(EVT::POINTER_DRAG, evt);

				else
					svc->post(EVT::POINTER_MOVE, evt);
			}

			if (nowOff)
			{
				svc->post(EVT::POINTER_OFF, evt);
			}
		}

	protected: // InputSource Impl
		virtual void onReset() 
		{ 
			if (_on)
				svc_Input->post(EVT::POINTER_CANCEL, new InputPointerEvent(this, _value, _value));

			_on = false;
		}

		virtual void onUpdate(int frame) 
		{
			// nothing to do
		}
	};

	class TouchTrigger : public InputTrigger
	{
	public:
		TouchTrigger(const String& name) : InputTrigger(name) { }

	protected:							// InputSource Impl
		virtual void					onReset() { }
		virtual void					onUpdate(int frame) { }    
	};

public:
	WindowInputDevice(RenderView* view, const String& name)
		: InputDevice(name)
	{
		_view = view;

		_touch[0].pointer = new TouchPointer("touch0_pos");
		_touch[0].trigger = new TouchTrigger("touch0_trig");

		_touch[1].pointer = new TouchPointer("touch1_pos");
		_touch[1].trigger = new TouchTrigger("touch1_trig");

		attach(_touch[0].pointer);
		attach(_touch[0].trigger);

		attach(_touch[1].pointer);
		attach(_touch[1].trigger);

		// TODO: implement trigger related functions
		// TODO: implement touch1
	}

	struct Touch
	{
		Ref<TouchPointer>				pointer;
		Ref<TouchTrigger>				trigger;
	};

	Touch								_touch[2];

	Weak<RenderView>					_view;
};

////////////////////////////////////////////////////////////////////////////////

GLESRenderView_win32::GLESRenderView_win32(AppBase* app)
{
	_app			= app;
	_hwnd			= NULL;

	_renderDevice	= NULL;
	_inputDevice	= NULL;
	_mouseCaptured	= false;

	_renderDevice = new GLESRenderDevice_win32();
}

void GLESRenderView_win32::onDelete()
{
	close();
	safeDelete(_renderDevice);
	safeDelete(_inputDevice);
}

RenderContext* GLESRenderView_win32::onNewRenderContext()
{
	RenderContext* ctx = new GLESRenderContext(this);
	return ctx;
}

bool GLESRenderView_win32::create(const String& title, int width, int height)
{
	PVRFrameEnableControlWindow(false);

	if (_hwnd != NULL)
		return false;

	_windowClass = WindowClass::obtain();

	if (create(_windowClass->newWindow(title, (LPVOID)this)))
	{
		resize(width, height);
		return true;
	}

	return false;
}

bool GLESRenderView_win32::create(HWND hwnd)
{
	_hwnd = hwnd;

	if (!_renderDevice->initEGL(hwnd))
		return false;

	return true;
}

void GLESRenderView_win32::close()
{
	_renderDevice->invalidate();

	if (_windowClass)
		DestroyWindow(_hwnd);

	_hwnd = NULL;
	_windowClass = NULL;
}

LRESULT CALLBACK GLESRenderView_win32::windowProcCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	GLESRenderView_win32* view = NULL;

	if (uMsg == WM_CREATE)
	{
		view = (GLESRenderView_win32*) ((CREATESTRUCT*)lParam)->lpCreateParams;
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)view);
	}
	else
	{
		view = (GLESRenderView_win32*) GetWindowLong(hWnd, GWL_USERDATA);
	}

	HWND handle = view ? (HWND)view->_hwnd : NULL;

	if (handle == hWnd)
		return view->windowProc(hWnd, uMsg, wParam, lParam);
	else
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT GLESRenderView_win32::windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		return DefWindowProc(_hwnd, uMsg, wParam, lParam);

	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(_hwnd, &ps);
			EndPaint(_hwnd, &ps);
		}
		break;

	case WM_SIZE:
		switch (wParam)
		{
		case SIZE_RESTORED:
			if (_app)
			{
				AppBase::ScopedEnter sc(_app);
				_app->_setActive(true);
				_app->_setSuspended(false);
			}
			break;
		case SIZE_MINIMIZED:
			if (_app)
			{
				AppBase::ScopedEnter sc(_app);
				_app->saveNow();
				_app->_setSuspended(true);
				_app->_setActive(false);
			}
			break;
		}

	case WM_LBUTTONDOWN:
		return handleLButtonDown(hWnd, wParam, lParam);

	case WM_MOUSEMOVE:
		return handleMouseMove(hWnd, wParam, lParam);

	case WM_LBUTTONUP:
		return handleLButtonUp(hWnd, wParam, lParam);

	default:
		return DefWindowProc(_hwnd, uMsg, wParam, lParam);
	}

	return 0;
}

void GLESRenderView_win32::resize(int width, int height)
{
	if (_hwnd == NULL)
		return;

	if (_mouseCaptured)
	{
		ReleaseCapture();
		_mouseCaptured = false;
	}

	int scaledWidth = int(width * _scale);
	int scaledHeight = int(height * _scale);

	RECT rcClient;
	GetClientRect(_hwnd, &rcClient);
	if (rcClient.right - rcClient.left == scaledWidth && rcClient.bottom - rcClient.top == scaledHeight)
		return;

	_width		= _left + width;
	_height	= _top + height;

	// Do not resize when we are borrowing the window.
	bool resizableWindow = _windowClass != NULL;

	resizableWindow = true; // TODO: Is this needed?

	if (resizableWindow)
	{
		// calculate new window width and height
		rcClient.right = rcClient.left + scaledWidth;
		rcClient.bottom = rcClient.top + scaledHeight;
		AdjustWindowRectEx(&rcClient, GetWindowLong(_hwnd, GWL_STYLE), false, GetWindowLong(_hwnd, GWL_EXSTYLE));

		// change width and height
		SetWindowPos(_hwnd, 0, 0, 0, rcClient.right - rcClient.left, 
			rcClient.bottom - rcClient.top, SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
 	}

	_renderDevice->resizeEGLSurface();
}

void GLESRenderView_win32::centerWindow()
{
	if (_windowClass == NULL || _hwnd == NULL) // Do not centerWindow() when we are borrowing the window.
		return;

	RECT rcDesktop, rcWindow;
	GetWindowRect(GetDesktopWindow(), &rcDesktop);

	// substract the task bar
	HWND hTaskBar = FindWindow(TEXT("Shell_TrayWnd"), NULL);
	if (hTaskBar != NULL)
	{
		APPBARDATA abd;

		abd.cbSize = sizeof(APPBARDATA);
		abd.hWnd = hTaskBar;

		SHAppBarMessage(ABM_GETTASKBARPOS, &abd);
		SubtractRect(&rcDesktop, &rcDesktop, &abd.rc);
	}
	GetWindowRect(_hwnd, &rcWindow);

	int offsetX = (rcDesktop.right - rcDesktop.left - (rcWindow.right - rcWindow.left)) / 2;
	offsetX = (offsetX > 0) ? offsetX : rcDesktop.left;
	int offsetY = (rcDesktop.bottom - rcDesktop.top - (rcWindow.bottom - rcWindow.top)) / 2;
	offsetY = (offsetY > 0) ? offsetY : rcDesktop.top;

	SetWindowPos(_hwnd, 0, offsetX, offsetY, 0, 0, SWP_NOCOPYBITS | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER);
}

void GLESRenderView_win32::show()
{
	::ShowWindow(_hwnd, SW_SHOW);
}

void GLESRenderView_win32::hide()
{
	::ShowWindow(_hwnd, SW_HIDE);
}

InputDevice* GLESRenderView_win32::getInputDevice()
{
	if (_inputDevice == NULL)
		_inputDevice = new WindowInputDevice(this, "RenderViewTouchScreen");

	return _inputDevice;
}

LRESULT GLESRenderView_win32::handleLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// HACK: WM_LBUTTON_DOWN comes without MK_LBUTTON on windows7
	if ((MK_LBUTTON & wParam) == 0) return 0; 

	if (_inputDevice == NULL) return 0;

	POINT pt = { LOWORD(lParam), HIWORD(lParam) };
	RECT rcViewport = { _left, _top, _left + _width, _top + _height };

	// 	if (!PtInRect(&rcViewport, pt))
	// 		return 0;

	_mouseCaptured = true;
	SetCapture(hWnd);

	Vector3 pos;
	pos.x = (float)((short)LOWORD(lParam) - _left);
	pos.y = (float)((short)HIWORD(lParam) - _top);
	pos /= _scale;

	// Send event
	AppBase::ScopedEnter sc(_app);

	WindowInputDevice* device = static_cast<WindowInputDevice*>(_inputDevice);

	WindowInputDevice::TouchPointer* pos0 = device->_touch[0].pointer;
	pos0->move(true, pos);

	return 0;
}

LRESULT GLESRenderView_win32::handleMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (_inputDevice == NULL) return 0;

	POINT pt = { LOWORD(lParam), HIWORD(lParam) };
	RECT rcViewport = { _left, _top, _left + _width, _top + _height };

	// 	if (!_mouseCaptured && !PtInRect(&rcViewport, pt))
	// 		return 0; // TODO: need PointerEnter, PointerExit

	Vector3 pos;
	pos.x = (float)((short)LOWORD(lParam) - _left);
	pos.y = (float)((short)HIWORD(lParam) - _top);
	pos /= _scale;

	// Send event
	AppBase::ScopedEnter sc(_app);

	WindowInputDevice* device = static_cast<WindowInputDevice*>(_inputDevice);

	WindowInputDevice::TouchPointer* pos0 = device->_touch[0].pointer;
	pos0->move(pos0->isOn(), pos);

	return 0;
}

LRESULT GLESRenderView_win32::handleLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (!_mouseCaptured)
		return 0;

	ReleaseCapture();
	_mouseCaptured = false; // TODO: Maybe useful to WM_HIDE, WM_CLOSE

	if (_inputDevice == NULL) return 0;

	Vector3 pos;
	pos.x = (float)((short)LOWORD(lParam) - _left);
	pos.y = (float)((short)HIWORD(lParam) - _top);
	pos /= _scale;

	// Send event
	AppBase::ScopedEnter sc(_app);

	WindowInputDevice* device = static_cast<WindowInputDevice*>(_inputDevice);

	WindowInputDevice::TouchPointer* pos0 = device->_touch[0].pointer;
	pos0->move(false, pos);

	return 0;
}

void GLESRenderView_win32::setOrientation(Orientation orient)
{
	if (_orientation == orient)
		return;

	bool portrait = isPortrait();

	_orientation = orient;

	if (isPortrait() != portrait)
	{
		// swap width and height
		resize(_height, _width);
		centerWindow();
	}
}

void GLESRenderView_win32::setScale(float scale)
{
	if (scale == _scale)
		return;

	_scale = scale;
	resize(_width, _height);
	centerWindow();
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
