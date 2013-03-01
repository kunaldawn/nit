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

#include "nitrender/nitrender.h"

#include "nitrender/RenderDevice.h"
#include "nitrender/RenderHandle.h"
#include "nitrender/RenderContext.h"

#if defined(NIT_WIN32) || defined(NIT_ANDROID)
#	include <EGL/egl.h>
#	include <gles/gl.h>
#endif

#if defined(NIT_IOS)
#	include "nitrender/egl_ios.h"
#endif

////////////////////////////////////////////////////////////////////////////////

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class GLESRenderHandle;
class GLESTextureHandle;
class GLESBufferHandle;
class GLESFrameBufferHandle;

////////////////////////////////////////////////////////////////////////////////

class NITRENDER_API GLESRenderDevice : public RenderDevice
{
public:
	GLESRenderDevice();
	virtual ~GLESRenderDevice();

public:
	EGLDisplay							getDisplay()							{ return _display; }
	EGLConfig							getConfig()								{ return _config; }
	EGLSurface							getSurface()							{ return _surface; }
	EGLContext							getContext()							{ return _context; }

public:
	CacheManager*						getTextureCache()						{ return _cache; }
	CacheManager*						getBufferCache()						{ return _cache; }
	CacheManager*						getFrameBufferCache()					{ return _cache; }

public:
	bool								initEGL(EGLNativeWindowType nativeWindow);
	void								resizeEGLSurface();

protected:								// RenderDevice impl
	virtual bool						onReset();
	virtual void						onBeginFrame();
	virtual void						onBeginContext(RenderContext* ctx);
	virtual void						onEndContext(RenderContext* ctx);
	virtual void						onEndFrame();
	virtual void						onSwapBuffers();
	virtual void						onClearCache();
	virtual void						onInvalidateHandles();
	virtual void						onInvalidate();

protected:
	EGLNativeWindowType					_nativeWindow;
	EGLNativeDisplayType				_nativeDisplay;
	EGLDisplay							_display;
	EGLConfig							_config;
	EGLSurface							_surface;
	EGLContext							_context;

	void								finishEGL();

	virtual bool						getNativeDisplay() = 0;
	virtual void						releaseNativeDisplay() = 0;
	virtual EGLint						chooseConfig() = 0;

	CacheManager*						_cache;
};

////////////////////////////////////////////////////////////////////////////////

class NITRENDER_API GLESRenderContext : public RenderContext
{
public:
	GLESRenderContext(RenderView* view);
};

////////////////////////////////////////////////////////////////////////////////

class NITRENDER_API GLESRenderHandle : public RenderHandle
{
public:
	GLESRenderHandle();

public:
	GLuint								useHandle(RenderContext* ctx)			{ return touch() ? _handle : 0; }

public:									// RenderHandle Impl
	bool								isValid(RenderContext* ctx);

protected:								// RenderHandle Impl
	virtual bool						onInvalidate();

protected:
	GLuint								_handle;
};

////////////////////////////////////////////////////////////////////////////////

class NITRENDER_API GLESTextureHandle : public GLESRenderHandle
{
protected:								// RenderHandle Impl
	virtual bool						onGenerate(RenderContext* ctx);
	virtual bool						onValidate();
	virtual bool						onInvalidate();
};

////////////////////////////////////////////////////////////////////////////////

class NITRENDER_API GLESBufferHandle : public GLESRenderHandle
{
protected:								// RenderHandle Impl
	virtual bool						onGenerate(RenderContext* ctx);
	virtual bool						onValidate();
	virtual bool						onInvalidate();
};

////////////////////////////////////////////////////////////////////////////////

class NITRENDER_API GLESFrameBufferHandle : public GLESRenderHandle
{
protected:								// RenderHandle Impl
	virtual bool						onGenerate(RenderContext* ctx);
	virtual bool						onValidate();
	virtual bool						onInvalidate();
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
