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

#include "nitrender/RenderDevice.h"

#include "nitrender/RenderSpec.h"

#include "nit/app/AppBase.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

RenderDevice::RenderDevice()
{
	_valid			= false;
	_renderSpec	= NULL;
}

RenderDevice::~RenderDevice()
{
	safeDelete(_renderSpec);

	for (CacheLookup::iterator itr = _caches.begin(), end = _caches.end(); itr != end; ++itr)
		delete itr->second;
}

bool RenderDevice::reset()
{
	_valid = false;

	invalidateHandles();

	return onReset();
}

void RenderDevice::beginFrame()
{
	if (!_valid) return;

	for (CacheLookup::iterator itr = _caches.begin(), end = _caches.end(); itr != end; ++itr)
		itr->second->beginFrame();

	onBeginFrame();
}

void RenderDevice::beginContext(RenderContext* ctx)
{
	if (!_valid) return;

	onBeginContext(ctx);
}

void RenderDevice::endContext(RenderContext* ctx)
{
	if (!_valid) return;

	onEndContext(ctx);
}

void RenderDevice::endFrame()
{
	if (!_valid) return;

	onEndFrame();

	for (CacheLookup::iterator itr = _caches.begin(), end = _caches.end(); itr != end; ++itr)
		itr->second->endFrame();
}

void RenderDevice::swapBuffers()
{
	if (!_valid) return;

	onSwapBuffers();
}

void RenderDevice::clearCaches()
{
	if (!_valid) return;

	onClearCache();

	for (CacheLookup::iterator itr = _caches.begin(), end = _caches.end(); itr != end; ++itr)
		itr->second->cacheOut(true);
}

void RenderDevice::invalidateHandles()
{	
	if (!_valid) return;

	LOG_TIMESCOPE(0, "++ RenderDevice: Invalidate Handles");

	onInvalidateHandles();

	for (CacheLookup::iterator itr = _caches.begin(), end = _caches.end(); itr != end; ++itr)
		itr->second->invalidateAll();

}

void RenderDevice::invalidate()
{
	if (!_valid) return;

	LOG_TIMESCOPE(0, "++ RenderDevice: Invalidate");

	invalidateHandles();

	_valid = false;
    
	onInvalidate();
}

uint RenderDevice::getHandleCount()
{
	uint handleCount = 0;
	for (CacheLookup::iterator itr = _caches.begin(), end = _caches.end(); itr != end; ++itr)
		handleCount += itr->second->getValidCount();

	return handleCount;
}

uint RenderDevice::getWiredHandleCount()
{
	uint handleCount = 0;
	for (CacheLookup::iterator itr = _caches.begin(), end = _caches.end(); itr != end; ++itr)
		handleCount += itr->second->getWiredCount();

	return handleCount;
}

uint RenderDevice::getActiveHandleCount()
{
	uint handleCount = 0;
	for (CacheLookup::iterator itr = _caches.begin(), end = _caches.end(); itr != end; ++itr)
		handleCount += itr->second->getActiveCount();

	return handleCount;
}

uint RenderDevice::getInactiveHandleCount()
{
	uint handleCount = 0;
	for (CacheLookup::iterator itr = _caches.begin(), end = _caches.end(); itr != end; ++itr)
		handleCount += itr->second->getInactiveCount();

	return handleCount;
}

size_t RenderDevice::getHandleFootprint()
{
	size_t footprint = 0;
	for (CacheLookup::iterator itr = _caches.begin(), end = _caches.end(); itr != end; ++itr)
		footprint += itr->second->getValidFootprint();

	return footprint;
}

size_t RenderDevice::getInactiveFootprint()
{
	size_t footprint = 0;
	for (CacheLookup::iterator itr = _caches.begin(), end = _caches.end(); itr != end; ++itr)
		footprint += itr->second->getInactiveFootprint();

	return footprint;
}

void RenderDevice::registerCache(const String& name, CacheManager* mgr)
{
	ASSERT_THROW(_caches.find(name) == _caches.end(), EX_DUPLICATED);

	_caches.insert(std::make_pair(name, mgr));

	DataValue footprintThreshold = (int)mgr->getFootprintThreshold();
	DataValue ageThreshold = (int)mgr->getInactiveAgeThreshold();

	footprintThreshold = g_App->getConfig(StringUtil::format("render/%s_footprint_threshold", mgr->getName().c_str()), footprintThreshold.toString());
	ageThreshold = g_App->getConfig(StringUtil::format("render/%s_age_threshold", mgr->getName().c_str()), ageThreshold.toString());

	mgr->setFootprintThreshold(footprintThreshold.toInt());
	mgr->setInactiveAgeThreshold(ageThreshold.toInt());
}

CacheManager* RenderDevice::getCache(const String& name)
{
	CacheLookup::iterator itr = _caches.find(name);
	return itr != _caches.end() ? itr->second : NULL;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
