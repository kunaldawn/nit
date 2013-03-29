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

#include "nitrender/RenderHandle.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class RenderSpec;

////////////////////////////////////////////////////////////////////////////////

class NITRENDER_API RenderDevice : public WeakSupported
{
public:
	RenderDevice();
	virtual ~RenderDevice();

public:
	bool								isValid()								{ return _valid; }
	RenderSpec*							getSpec()								{ return _renderSpec; }

public:
	bool								reset();
	void								beginFrame();
	void								beginContext(RenderContext* ctx);
	void								endContext(RenderContext* ctx);
	void								endFrame();

	void								swapBuffers();

public:
	void								clearCaches();
	void								invalidateHandles();
	void								invalidate();

protected:
	friend class NB_RenderDevice;
	typedef map<String, CacheManager*>::type CacheLookup;

	void								registerCache(const String& name, CacheManager* mgr);
	CacheLookup&						getCacheLookup()						{ return _caches; }

public:
	CacheManager*						getCache(const String& name);

	uint								getHandleCount();
	size_t								getHandleFootprint();

	uint								getWiredHandleCount();
	uint								getActiveHandleCount();
	uint								getInactiveHandleCount();
	size_t								getInactiveFootprint();

protected:
	virtual bool						onReset() = 0;

	virtual void						onBeginFrame() = 0;
	virtual void						onBeginContext(RenderContext* ctx) = 0;
	virtual void						onEndContext(RenderContext* ctx) = 0;
	virtual void						onEndFrame() = 0;

	virtual void						onSwapBuffers() = 0;

	virtual void						onClearCache() = 0;
	virtual void						onInvalidateHandles() = 0;
	virtual void						onInvalidate() = 0;

	bool								_valid;

protected:
	RenderSpec*							_renderSpec;
	CacheLookup							_caches;
};

////////////////////////////////////////////////////////////////////////////////

class NITRENDER_API RenderDeviceEvent : public Event
{
public:
	RenderDeviceEvent()															{ }
	RenderDeviceEvent(RenderDevice* device) : _device(device)					{ }

public:
	RenderDevice*						getDevice() const						{ return _device; }

protected:
	Weak<RenderDevice>					_device;
};

NIT_EVENT_DECLARE(NITRENDER_API, RENDER_DEVICE_RESET, RenderDeviceEvent);
NIT_EVENT_DECLARE(NITRENDER_API, RENDER_DEVICE_LOST, RenderDeviceEvent);

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
