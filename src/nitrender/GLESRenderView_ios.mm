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

#include "nitrender/GLESRenderView_ios.h"
#import "nitrender/EAGLView_ios.h"

#include "nit/input/InputDevice.h"
#include "nit/input/InputUser.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class GLESRenderDevice_ios : public GLESRenderDevice
{
public:
	virtual bool getNativeDisplay()
	{
		EAGLView2* view = (EAGLView2*)_nativeWindow;
		
		id layer = [view layer];
		_nativeDisplay = layer;
		[layer retain];

		return true;
	}

	virtual void releaseNativeDisplay()
	{
		id layer = (id)_nativeDisplay;
		[layer release];
	}

	virtual EGLint chooseConfig()
	{
		const EGLint attribs[] =
		{
			EGL_SURFACE_TYPE,		EGL_WINDOW_BIT,
			EGL_ALPHA_SIZE,			0,
			EGL_BLUE_SIZE,			8,
			EGL_GREEN_SIZE,			8,
			EGL_RED_SIZE,			8,
			EGL_DEPTH_SIZE,			16,
			EGL_RENDERABLE_TYPE,	EGL_OPENGL_ES_BIT,
			EGL_NONE
		};
		
		EGLint numConfigs = 0;

		eglChooseConfig(_display, attribs, &_config, 1, &numConfigs);

		return numConfigs;
	}
};

////////////////////////////////////////////////////////////////////////////////

class IOSViewInput : public InputDevice, public IEventSink
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
	IOSViewInput(RenderView* view, const String& name)
		: InputDevice(name)
	{
		_view = view;
		_inputScale = 1.0f;
		
		for (uint i=0; i < COUNT_OF(_touch); ++i)
		{
			_touch[i].index = i;
			_touch[i].sysTouch = nil;
			_touch[i].pointer = new TouchPointer(StringUtil::format("touch%d_pos", i));
			_touch[i].trigger = new TouchTrigger(StringUtil::format("touch%d_trig", i));
			attach(_touch[i].pointer);
			attach(_touch[i].trigger);
		}
		
		ReleaseAllTouches();

		id eaglView = [EAGLView2 sharedView];
		EventChannel* channel = [eaglView channel];
		
		channel->bind(Events::OnIOSTouchesBegan, this, &IOSViewInput::onIOSTouchesBegan);
		channel->bind(Events::OnIOSTouchesMoved, this, &IOSViewInput::onIOSTouchesMoved);
		channel->bind(Events::OnIOSTouchesEnded, this, &IOSViewInput::onIOSTouchesEnded);
		channel->bind(Events::OnIOSTouchesCancelled, this, &IOSViewInput::onIOSTouchesCancelled);

		// TODO: Implement Trigger related functions
	}
	
	virtual void onDelete()
	{
		ReleaseAllTouches();
	}
	
	virtual bool isEventActive() 
	{ 
		return true; 
	}
	
	virtual bool isDisposed()
	{
		return false;
	}
	
	void onIOSTouchesBegan(const Event* evt)
	{
		const IOSTouchEvent* e = (const IOSTouchEvent*)evt;
		id eaglView = [EAGLView2 sharedView];
		float scale = [eaglView contentScaleFactor] / _inputScale;
		
		for (UITouch* sysTouch in e->getTouches())
		{
			Touch* touch = AssignTouch(sysTouch);
			if (touch == NULL) continue; // FULL?
			
			float x = [sysTouch locationInView: eaglView].x;
			float y = [sysTouch locationInView: eaglView].y;
			Vector3 pos(x, y, 0);
			pos *= scale;
			touch->pointer->move(true, pos);
		}
	}
	
	void onIOSTouchesMoved(const Event* evt)
	{
		const IOSTouchEvent* e = (const IOSTouchEvent*)evt;
		id eaglView = [EAGLView2 sharedView];
		float scale = [eaglView contentScaleFactor] / _inputScale;

		for (UITouch* sysTouch in e->getTouches())
		{
			Touch* touch = AssignTouch(sysTouch);
			if (touch == NULL) continue;
			
			float x = [sysTouch locationInView: eaglView].x;
			float y = [sysTouch locationInView: eaglView].y;
			Vector3 pos(x, y, 0);
			pos *= scale;
			touch->pointer->move(true, pos);
		}
	}
	
	void onIOSTouchesEnded(const Event* evt)
	{
		const IOSTouchEvent* e = (const IOSTouchEvent*)evt;
		id eaglView = [EAGLView2 sharedView];
		float scale = [eaglView contentScaleFactor] / _inputScale;
		
		for (UITouch* sysTouch in e->getTouches())
		{
			Touch* touch = ReleaseTouch(sysTouch);
			if (touch == NULL) continue;
			
			float x = [sysTouch locationInView: eaglView].x;
			float y = [sysTouch locationInView: eaglView].y;
			Vector3 pos(x, y, 0);
			pos *= scale;
			touch->pointer->move(false, pos);
		}
	}
	
	void onIOSTouchesCancelled(const Event* evt)
	{
		const IOSTouchEvent* e = (const IOSTouchEvent*)evt;
		id eaglView = [EAGLView2 sharedView];
		float scale = [eaglView contentScaleFactor] / _inputScale;
		
		for (UITouch* sysTouch in e->getTouches())
		{
			Touch* touch = ReleaseTouch(sysTouch);
			if (touch == NULL) continue;
			
			float x = [sysTouch locationInView: eaglView].x;
			float y = [sysTouch locationInView: eaglView].y;
			Vector3 pos(x, y, 0);
			pos *= scale;
			touch->pointer->move(false, pos); // TODO: Implement cancel
		}
	}
	
	struct Touch
	{
		uint							index;
		UITouch*						sysTouch;
		Ref<TouchPointer>				pointer;
		Ref<TouchTrigger>				trigger;
	};

	// For the maximum number of touches are small, and it's preferrable to start index of touches from zero,
	// use array search instead of map + stacked pool
	
	Touch* GetTouch(UITouch* sysTouch)
	{
		for (uint i=0; i<COUNT_OF(_touch); ++i)
		{
			Touch* touch = &_touch[i];
			if (touch->sysTouch == sysTouch)
				return touch;
		}
		
		return NULL;
	}
	
	Touch* AssignTouch(UITouch* sysTouch)
	{
		Touch* touch = GetTouch(sysTouch);
		if (touch) return touch;
		
		for (uint i=0; i<COUNT_OF(_touch); ++i)
		{
			touch = &_touch[i];
			if (touch->sysTouch == NULL)
			{
				touch->sysTouch = sysTouch;
				[sysTouch retain];
				return touch;
			}
		}
		
		return NULL;
	}
	
	Touch* ReleaseTouch(UITouch* sysTouch)
	{
		for (uint i=0; i<COUNT_OF(_touch); ++i)
		{
			Touch* touch = &_touch[i];
			if (touch->sysTouch == sysTouch)
			{
				[touch->sysTouch release];
				touch->sysTouch = nil;
				return touch;
			}
		}
		
		return NULL;
	}
	
	void ReleaseAllTouches()
	{
		for (uint i=0; i<COUNT_OF(_touch); ++i)
		{
			Touch* touch = &_touch[i];
			if (touch->sysTouch)
			{
				[touch->sysTouch release];
				touch->sysTouch = nil;
			}
		}
	}

	float								_inputScale;
	Touch								_touch[5];

	Weak<RenderView>					_view;
	id									_eaglView;
};

////////////////////////////////////////////////////////////////////////////////

GLESRenderView_ios::GLESRenderView_ios()
{
    g_App->channel()->priority(-100000)->bind(Events::OnAppSuspend, this, &GLESRenderView_ios::onAppSuspend);
    g_App->channel()->priority(-100000)->bind(Events::OnAppResume, this, &GLESRenderView_ios::onAppResume);
    
	id eaglView = [EAGLView2 sharedView];
	
	_renderDevice	= new GLESRenderDevice_ios();

	_renderDevice->initEGL(eaglView);

	int width = 0, height = 0;
	eglQuerySurface(_renderDevice->getDisplay(), _renderDevice->getSurface(), EGL_WIDTH, &width);
	eglQuerySurface(_renderDevice->getDisplay(), _renderDevice->getSurface(), EGL_HEIGHT, &height);

	_left		= 0;
	_top		= 0;
	_width		= width;
	_height     = height;

	_inputDevice	= new IOSViewInput(this, "EAGLViewInput");
}

void GLESRenderView_ios::onDelete()
{
	_renderDevice->invalidate();

	safeDelete(_inputDevice);
	safeDelete(_renderDevice);
}

void GLESRenderView_ios::onAppSuspend(const Event *evt)
{
    if (_renderDevice)
    {
        LOG_TIMESCOPE(0, "++ GLESRenderView: app suspended, invalidating device");
        _renderDevice->invalidate();
    }
}

void GLESRenderView_ios::onAppResume(const Event *evt)
{
    if (_renderDevice)
    {
        LOG_TIMESCOPE(0, "++ GLESRenderView: app resumed, restoring device");

        id eaglView = [EAGLView2 sharedView];
        
        _renderDevice	= new GLESRenderDevice_ios();
        
        _renderDevice->initEGL(eaglView);
        
        int width = 0, height = 0;
        eglQuerySurface(_renderDevice->getDisplay(), _renderDevice->getSurface(), EGL_WIDTH, &width);
        eglQuerySurface(_renderDevice->getDisplay(), _renderDevice->getSurface(), EGL_HEIGHT, &height);
        
        _left		= 0;
        _top		= 0;
        _width		= width;
        _height     = height;
    }
}

void GLESRenderView_ios::setOrientation(Orientation orient)
{
	_orientation = orient;

	UIDeviceOrientation orientReq;
	UIInterfaceOrientation appOrient;

	switch (orient)
	{
		case ORIENT_PORTRAIT:				
			orientReq = UIDeviceOrientationPortrait;
			appOrient = UIInterfaceOrientationPortrait;
			break;
		case ORIENT_PORTRAIT_FLIP:
			orientReq = UIDeviceOrientationPortraitUpsideDown; 
			appOrient = UIInterfaceOrientationPortraitUpsideDown;
			break;
		case ORIENT_LANDSCAPE:
			orientReq = UIDeviceOrientationLandscapeRight; 
			appOrient = UIInterfaceOrientationLandscapeRight;
			break;
		case ORIENT_LANDSCAPE_FLIP:
			orientReq = UIDeviceOrientationLandscapeLeft; 
			appOrient = UIInterfaceOrientationLandscapeLeft;
			break;
		default:
			orientReq = UIDeviceOrientationUnknown; 
			appOrient = UIInterfaceOrientationPortrait;
			break;
	}

	[[UIApplication sharedApplication] setStatusBarOrientation: appOrient];
}

void GLESRenderView_ios::setScale(float scale)
{
	// It's not possible to resize the window on ios, 
	// so treat the scale as DPI adjustment, do not apply scale factor.

	// But input must respect the scale so set the scale to input device
	IOSViewInput* device = (IOSViewInput*)getInputDevice();
	device->_inputScale = scale;
}

RenderContext* GLESRenderView_ios::onNewRenderContext()
{
	RenderContext* ctx = new GLESRenderContext(this);
	return ctx;
}

InputDevice* GLESRenderView_ios::getInputDevice()
{
	if (_inputDevice == NULL)
		_inputDevice = new IOSViewInput(this, "RenderViewTouchScreen");

	return _inputDevice;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
