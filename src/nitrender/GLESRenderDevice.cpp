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

#include "nitrender/GLESRenderDevice.h"

#include "nitrender/RenderContext.h"
#include "nitrender/RenderView.h"

#include "nitrender/GLESRenderSpec.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

GLESRenderDevice::GLESRenderDevice()
{
	_nativeWindow	= NULL;
	_nativeDisplay = EGL_DEFAULT_DISPLAY;
	_display		= EGL_NO_DISPLAY;
	_config		= 0;
	_surface		= EGL_NO_SURFACE;
	_context		= EGL_NO_CONTEXT;

	_cache			= new CacheManager("gles");

	registerCache("gles", _cache);
}

GLESRenderDevice::~GLESRenderDevice()
{
}

bool GLESRenderDevice::initEGL(EGLNativeWindowType nativeWindow)
{
	if (nativeWindow == 0) 
	{
		LOG(0, "!!! null native window assigned\n");
		return false;
	}

	_nativeWindow = nativeWindow;

	if (!getNativeDisplay())
		return false;

	_display = eglGetDisplay(_nativeDisplay);
	if (_display == EGL_NO_DISPLAY)
	{
		LOG(0, "!!! can't get egl display\n");
		return false;
	}

	EGLint verMajor, verMinor;
	if (eglInitialize(_display, &verMajor, &verMinor) == EGL_FALSE)
	{
		LOG(0, "!!! can't initialize egl\n");
		return false;
	}

	LOG(0, "++ EGL %d.%d Initialized\n", verMajor, verMinor);

	EGLint numConfigs = chooseConfig();

	if (numConfigs == 0)
	{
		LOG(0, "!!! no suitable egl config found\n");
		return false;
	}

	EGLContext sharedContext = EGL_NO_CONTEXT; // TODO: support?
	_context = eglCreateContext(_display, _config, sharedContext, NULL);

	if (_context == EGL_NO_CONTEXT)
	{
		LOG(0, "!!! can't create egl context\n");
		return false;
	}

	_surface = eglCreateWindowSurface(_display, _config, _nativeWindow, NULL);
	if (_surface == EGL_NO_SURFACE)
	{
		LOG(0, "!!! can't create egl window surface\n");
		return false;
	}

	if (eglMakeCurrent(_display, _surface, _surface, _context) == EGL_FALSE)
	{
		LOG(0, "!!! can't make current egl context\n");
		return false;
	}

	_renderSpec = new GLESRenderSpec(this);
	_renderSpec->report();

	_valid = true;

	return true;
}

void GLESRenderDevice::finishEGL()
{
	if (_display)
	{
		eglMakeCurrent(_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

		if (_surface != EGL_NO_SURFACE)
			eglDestroySurface(_display, _surface);

		if (_context != EGL_NO_CONTEXT)
			eglDestroyContext(_display, _context);

		eglTerminate(_display);
	}

	if (_nativeDisplay)
	{
		releaseNativeDisplay();
	}

	_nativeWindow	= NULL;
	_nativeDisplay = EGL_DEFAULT_DISPLAY;
	_display		= EGL_NO_DISPLAY;
	_config		= 0;
	_surface		= EGL_NO_SURFACE;
	_context		= EGL_NO_CONTEXT;
}

void GLESRenderDevice::resizeEGLSurface()
{
	if (0)
	{
		// TODO: Maybe we don't have to do this.

		if (! _nativeWindow || EGL_NO_DISPLAY == _display)
			return;

		// release old surface
		if (EGL_NO_SURFACE != _surface)
		{
			eglDestroySurface(_display, _surface);
			_surface = EGL_NO_SURFACE;
		}

		// create new surface and make current
		_surface = eglCreateWindowSurface(_display, _config, _nativeWindow, NULL);
		eglMakeCurrent(_display, _surface, _surface, _context);
	}
}

bool GLESRenderDevice::onReset()
{
	EGLNativeWindowType nativeWindow = _nativeWindow;

	finishEGL();

	return initEGL(nativeWindow);
}

void GLESRenderDevice::onBeginFrame()
{
	// nothing to do yet
}

void GLESRenderDevice::onBeginContext(RenderContext* ctx)
{
	// nothing to do yet
}

void GLESRenderDevice::onEndContext(RenderContext* ctx)
{
	// nothing to do yet
}

void GLESRenderDevice::onEndFrame()
{
	// nothing to do yet
}

void GLESRenderDevice::onSwapBuffers()
{
	if (_display != EGL_NO_DISPLAY)
		eglSwapBuffers(_display, _surface);
}

void GLESRenderDevice::onClearCache()
{
	// nothing to do yet
}

void GLESRenderDevice::onInvalidateHandles()
{
	// nothing to do yet
}

void GLESRenderDevice::onInvalidate()
{
	finishEGL();
}

////////////////////////////////////////////////////////////////////////////////

GLESRenderContext::GLESRenderContext(RenderView* view)
: RenderContext(view)
{
	_device = dynamic_cast<GLESRenderDevice*>(view->getRenderDevice());
}

////////////////////////////////////////////////////////////////////////////////

GLESRenderHandle::GLESRenderHandle()
: _handle(0)
{

}

bool GLESRenderHandle::isValid(RenderContext* ctx)
{
	return _handle != 0 && isCacheValid();
}

bool GLESRenderHandle::onInvalidate()
{
	_handle = 0;
	return true;
}

////////////////////////////////////////////////////////////////////////////////

bool GLESTextureHandle::onGenerate(RenderContext* ctx)
{
	GLESRenderDevice* device = (GLESRenderDevice*)ctx->getDevice();
	CacheManager* cache = device->getTextureCache();

	if (!validate(cache)) return false;

	glGenTextures(1, &_handle);

	return true;
}

bool GLESTextureHandle::onValidate()
{
	return true;
}

bool GLESTextureHandle::onInvalidate()
{
	if (_handle != 0)
		glDeleteTextures(1, &_handle);

	return GLESRenderHandle::onInvalidate();
}

////////////////////////////////////////////////////////////////////////////////

bool GLESBufferHandle::onGenerate(RenderContext* ctx)
{
	GLESRenderDevice* device = (GLESRenderDevice*)ctx->getDevice();
	CacheManager* cache = device->getBufferCache();

	if (!validate(cache)) return false;

	glGenTextures(1, &_handle);

	return true;
}

bool GLESBufferHandle::onValidate()
{
	return true;
}

bool GLESBufferHandle::onInvalidate()
{
	if (_handle != 0)
		glDeleteBuffers(1, &_handle);

	return GLESRenderHandle::onInvalidate();
}

////////////////////////////////////////////////////////////////////////////////

bool GLESFrameBufferHandle::onGenerate(RenderContext* ctx)
{
	GLESRenderDevice* device = (GLESRenderDevice*)ctx->getDevice();
	CacheManager* cache = device->getFrameBufferCache();

	if (!validate(cache)) return false;

	glGenFramebuffersOES(1, &_handle);
	return true;
}

bool GLESFrameBufferHandle::onValidate()
{
	return true;
}

bool GLESFrameBufferHandle::onInvalidate()
{
	if (_handle != 0)
		glDeleteFramebuffersOES(1, &_handle);

	return GLESRenderHandle::onInvalidate();
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
