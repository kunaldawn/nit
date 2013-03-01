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

#include "nitrender/GLESRenderView_android.h"

#include "nit/input/InputDevice.h"
#include "nit/input/InputUser.h"

#include <android/input.h>
#include <android/window.h>

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class GLESRenderDevice_android : public GLESRenderDevice
{
public:
	virtual bool getNativeDisplay()
	{
		_nativeDisplay = EGL_DEFAULT_DISPLAY;

		return true;
	}

	virtual void releaseNativeDisplay()
	{
		// Do nothing
	}

	virtual EGLint chooseConfig()
	{
		const EGLint attribs[] =
		{
			EGL_SURFACE_TYPE,		EGL_WINDOW_BIT,
			EGL_BLUE_SIZE,			8,
			EGL_GREEN_SIZE,			8,
			EGL_RED_SIZE,			8,
			EGL_DEPTH_SIZE,			16,
			EGL_RENDERABLE_TYPE,	EGL_OPENGL_ES_BIT,
			EGL_NONE
		};

		EGLint numConfigs = 0;

		eglChooseConfig(_display, attribs, &_config, 1, &numConfigs);

		if (numConfigs > 0)
		{
			EGLint format;
			eglGetConfigAttrib(_display, _config, EGL_NATIVE_VISUAL_ID, &format);
			ANativeWindow_setBuffersGeometry(_nativeWindow, 0, 0, format);
		}

		return numConfigs;
	}
};

////////////////////////////////////////////////////////////////////////////////

class AndroidWindowInput : public InputDevice, public IEventSink
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

//			LOG(0, "-- %s: %s (%.3f %.3f %.3f)\n", _name.c_str(), on ? "on" : "off", newPos.x, newPos.y, newPos.z);

			_on = on;
			_value = newPos;

			if (nowOn)
				svc->post(Events::OnPointerOn, evt);

			if (moved)
			{
				if (on)
					svc->post(Events::OnPointerDrag, evt);

				else
					svc->post(Events::OnPointerMove, evt);
			}

			if (nowOff)
			{
				svc->post(Events::OnPointerOff, evt);
			}
		}

	protected: // InputSource Impl
		virtual void onReset() 
		{ 
			if (_on)
				svc_Input->post(Events::OnPointerCancel, new InputPointerEvent(this, _value, _value));

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
	AndroidWindowInput(RenderView* view, const String& name)
		: InputDevice(name)
	{
		_view = view;
		_inputScale = 1.0f;

		for (uint i=0; i < COUNT_OF(_touch); ++i)
		{
			_touch[i].index = i;
			_touch[i].sysIndex = -1;
			_touch[i].pointer = new TouchPointer(StringUtil::format("touch%d_pos", i));
			_touch[i].trigger = new TouchTrigger(StringUtil::format("touch%d_trig", i));
			attach(_touch[i].pointer);
			attach(_touch[i].trigger);
		}

		// TODO: Implement Trigger related functions

		NitRuntime* rt = NitRuntime::getSingleton();

		rt->channel()->bind(Events::OnAndroidInput, this, &AndroidWindowInput::onAndroidInput);
		rt->channel()->bind(Events::OnAndroidSensor, this, &AndroidWindowInput::onAndroidSensor);
	}

	virtual bool isEventActive()		{ return true; }
	virtual bool isDisposed()			{ return false; }

	void onAndroidInput(const AndroidInputEvent* evt)
	{
		AInputEvent* input = evt->getInput();

		if (AInputEvent_getType(input) == AINPUT_EVENT_TYPE_MOTION)
		{
			handleMotionEvent(input);
		}
	}

	void handleMotionEvent(AInputEvent* input)
	{
		int32_t action = AMotionEvent_getAction(input);
		int actionMasked = action & AMOTION_EVENT_ACTION_MASK;
		int actionPointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

// 		LOG(0, ".. motion: %d %d\n", actionMasked, actionPointerIndex);
		size_t numPointers = AMotionEvent_getPointerCount(input);

		float scale = 1.0f / _inputScale;

		for (size_t i=0; i<numPointers; ++i)
		{
			int sysIndex = i;
			Touch* touch = NULL;
			bool on = false;

			switch (actionMasked)
			{
			case AMOTION_EVENT_ACTION_DOWN:
			case AMOTION_EVENT_ACTION_POINTER_DOWN:
				touch = assignTouch(sysIndex);
				on = true;
				break;

			case AMOTION_EVENT_ACTION_MOVE:
				touch = getTouch(sysIndex);
				on = true;
				break;

			case AMOTION_EVENT_ACTION_POINTER_UP:
			case AMOTION_EVENT_ACTION_UP:
				touch = getTouch(sysIndex);
				on = sysIndex != actionPointerIndex;
				break;
			}

			if (touch == NULL) continue;

			float x = AMotionEvent_getX(input, sysIndex);
			float y = AMotionEvent_getY(input, sysIndex);
			Vector3 pos(x, y, 0);
			pos *= scale;
// 			LOG(0, ".. pointer %d %f %f\n", sysIndex, pos.x, pos.y);

			touch->pointer->move(on, pos);
		}

		if (actionMasked == AMOTION_EVENT_ACTION_POINTER_UP || actionMasked == AMOTION_EVENT_ACTION_UP)
			releaseTouch(actionPointerIndex);
	}

	void onAndroidSensor(const AndroidSensorEvent* evt)
	{
		// TODO: impl
	}

	struct Touch
	{
		uint							index;
		int								sysIndex;
		Ref<TouchPointer>				pointer;
		Ref<TouchTrigger>				trigger;
	};

	Touch* getTouch(int sysIndex)
	{
		for (uint i=0; i<COUNT_OF(_touch); ++i)
		{
			Touch* touch = &_touch[i];
			if (touch->sysIndex == sysIndex)
				return touch;
		}

		return NULL;
	}

	Touch* assignTouch(int sysIndex)
	{
		Touch* touch = getTouch(sysIndex);
		if (touch) return touch;

		for (uint i=0; i<COUNT_OF(_touch); ++i)
		{
			touch = &_touch[i];
			if (touch->sysIndex == -1)
			{
				touch->sysIndex = sysIndex;
				return touch;
			}
		}

		return NULL;
	}

	void releaseTouch(int sysIndex)
	{
		for (uint i=0; i < COUNT_OF(_touch); ++i)
		{
			Touch* touch = &_touch[i];
			if (touch->sysIndex == sysIndex)
				touch->sysIndex = -1;
			else if (touch->sysIndex > sysIndex)
				--touch->sysIndex;
		}
	}

	float								_inputScale;
	Touch								_touch[5];

	Weak<RenderView>					_view;
};

////////////////////////////////////////////////////////////////////////////////

GLESRenderView_android::GLESRenderView_android(AppBase* nitApp)
{
	_app			= nitApp;
	_renderDevice	= new GLESRenderDevice_android();
	_inputDevice	= NULL;

	NitRuntime* rt = NitRuntime::getSingleton();

	rt->channel()->bind(Events::OnAndroidAppCmd, this, &GLESRenderView_android::onAndroidAppCmd);

	android_app* app = NitRuntime::getSingleton()->getAndroidApp();

	if (app->window != NULL)
	{
		// On app.Restart(), we already got a window. For this case do init at once.
		onInitWindow(app);
	}
}

void GLESRenderView_android::onDelete()
{
	_renderDevice->invalidate();

	safeDelete(_inputDevice);
	safeDelete(_renderDevice);

	LOG(0, "++ GLESRenderView_android: OnDelete\n");
}

void GLESRenderView_android::onAndroidAppCmd(const AndroidAppCmdEvent* evt)
{
	android_app* app = evt->getApp();

	// TODO: Refactor to app those are not related to window
	AppBase::ScopedEnter sc(_app);

	switch (evt->getCmd())
	{
	case APP_CMD_INIT_WINDOW:
		LOG(0, "++ GLESRenderView_android: APP_CMD_INIT_WINDOW\n");

		if (_renderDevice->isValid())
		{
			LOG(0, "?? GLESRenderView_android: device already ready\n");
			_renderDevice->invalidate();
		}

		// The window was not exist and here for the first time we got one.
		onInitWindow(app);

		if (_app && _androidAppResumed)
			_app->_setSuspended(false);
		break;

	case APP_CMD_TERM_WINDOW:
		LOG(0, "++ GLESRenderView_android: APP_CMD_TERM_WINDOW\n");
		_renderDevice->invalidate();
		if (_app) 
			_app->_setSuspended(true);
		break;

	case APP_CMD_START:
		LOG(0, "++ GLESRenderView_android: APP_CMD_START\n");
		if (_app)
			_app->_setActive(true);
		break;

	case APP_CMD_RESUME:
		LOG(0, "++ GLESRenderView_android: APP_CMD_RESUME\n");
		_androidAppResumed = true;

		if (_app && _renderDevice->isValid())
			_app->_setSuspended(false);
		break;

	case APP_CMD_PAUSE:
		LOG(0, "++ GLESRenderView_android: APP_CMD_PAUSE\n");
		_androidAppResumed = false;

		// HACK: when TERM_WINDOW it seems that window was already disposed.
		// For this, Device Invalidate() makes egl_queue errors.
		// So release handles on pause (a device is hard to restore - so only handles)
		// (When user turns the screen off, we get pause not term_window)
		// TODO: It takes longer time for handle resurrection. Investigate TERM_WINDOW more.
		_renderDevice->invalidateHandles();

		if (_app)
			_app->_setSuspended(true);
		break;

	case APP_CMD_STOP:
		LOG(0, "++ GLESRenderView_android: APP_CMD_STOP\n");
		if (_app)
			_app->_setActive(false);
		break;

	case APP_CMD_LOW_MEMORY:
		LOG(0, "++ GLESRenderView_android: APP_CMD_LOW_MEMORY\n");

		if (_app)
			_app->_notifyLowMemory();
		break;

	case APP_CMD_SAVE_STATE:
		LOG(0, "++ GLESRenderView_android: APP_CMD_SAVE_STATE\n");
		if (_app)
			_app->saveNow();
		break;
	}
}

void GLESRenderView_android::onInitWindow(android_app* app)
{
	LOG_TIMESCOPE(0, "++ GLEsRenderView_android: InitWindow");

#ifndef NIT_SHIPPING
	// Prevent screen not to be turned off
	// Do not use on SHIPPING version cause this can ruin AMOLED screens.
	ANativeActivity_setWindowFlags(app->activity, AWINDOW_FLAG_KEEP_SCREEN_ON, 0);
#endif

	_renderDevice->initEGL(app->window);

	int width, height;
	eglQuerySurface(_renderDevice->getDisplay(), _renderDevice->getSurface(), EGL_WIDTH, &width);
	eglQuerySurface(_renderDevice->getDisplay(), _renderDevice->getSurface(), EGL_HEIGHT, &height);

	// At first, initialize width and height with surface's value
	// Later SetOrientation will adjust or swap appropriately
	_left		= 0;
	_top		= 0;
	_width		= width;
	_height	= height;

	// Apply orientation
	setOrientation(_orientation);
}

void GLESRenderView_android::setOrientation(Orientation orient)
{
	_orientation = orient;

	int orientReq;

	// see Activity.ScreenOrientation
	switch (orient)
	{
	case ORIENT_PORTRAIT:				orientReq = 1; break;	// portrait
	case ORIENT_PORTRAIT_FLIP:			orientReq = 9; break;	// reversePortrait
	case ORIENT_LANDSCAPE:				orientReq = 0; break;	// landscape
	case ORIENT_LANDSCAPE_FLIP:			orientReq = 8; break;	// reverseLandscape
	default:							orientReq = -1; break;	// unspecified
	}

	LOG(0, "++ activity.setRequestedOrientation(%d)\n", orientReq);

	AndroidJNI::MethodInfo m;
	AndroidJNI* jni = g_AndroidJNI;
	jobject javaActivity = jni->getActivityInstance();

	bool ok = jni->getMethodInfo(m, javaActivity, "setRequestedOrientation", "(I)V");

	if (ok)
	{
		m.env->CallVoidMethod(javaActivity, m.methodID, orientReq);
		m.env->DeleteLocalRef(m.javaClass);
		ok = m.env->ExceptionCheck() == false;
	}

	if (!ok)
	{
		m.env->ExceptionClear();
		LOG(0, "*** NitActivity.setRequestedOrientation() failed\n");
	}

	int newWidth;
	int newHeight;

	if (isPortrait())
	{
		// Make taller rather than wider
		LOG(0, "-- set to portrait\n");
		newWidth = std::min(_width, _height);
		newHeight = std::max(_width, _height);
	}
	else
	{
		// Make wider rather than taller
		LOG(0, "-- set to landscape\n");
		newWidth = std::max(_width, _height);
		newHeight = std::min(_width, _height);
	}

	LOG(0, "-- %d -> %d, %d -> %d\n", _width, newWidth, _height, newHeight);

	_left		= 0;
	_top		= 0;
	_width		= newWidth;
	_height	= newHeight;
}

void GLESRenderView_android::setScale(float scale)
{
	// It's not possible to resize the window on android, 
	// so treat the scale as DPI adjustment, do not apply scale factor.

	// But input must respect the scale so set the scale to input device
	AndroidWindowInput* device = (AndroidWindowInput*)getInputDevice();
	device->_inputScale = scale;
}

RenderContext* GLESRenderView_android::onNewRenderContext()
{
	RenderContext* ctx = new GLESRenderContext(this);
	return ctx;
}

InputDevice* GLESRenderView_android::getInputDevice()
{
	if (_inputDevice == NULL)
		_inputDevice = new AndroidWindowInput(this, "RenderViewTouchScreen");

	return _inputDevice;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
