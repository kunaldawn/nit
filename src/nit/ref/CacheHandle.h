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

#include "nit/nit.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class CacheManager;

class NIT_API CacheHandle : public RefCounted
{
public: 
	CacheHandle();

public:
	CacheManager*						getCacheManager()						{ return _cacheManager; }
	virtual void						setCacheFootprint(size_t size);
	size_t								getCacheFootprint()						{ return _cacheFootprint; }

public:
	bool								isCacheValid()							{ return _cacheManager != NULL; }
	bool								isCacheWired()							{ return _cacheWireCount > 0; }

	bool								wire();			// TODO: make sure that Wired handles never invalidate
	void								unwire();

	bool								touch();
	bool								invalidate();

protected:
	bool								validate(CacheManager* manager);
	virtual bool						onValidate() = 0;
	virtual bool						onInvalidate() = 0;
	virtual bool						onInvalidateWired()						{ return false; }

protected:
	virtual void						onDelete();

private:
	friend class						CacheManager;
	CacheManager*						_cacheManager;
	size_t								_cacheFootprint;
	int									_cacheWireCount;
	int64								_cacheFrameNo;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API CacheManager : public WeakSupported
{
public:
	CacheManager(const String& name);
	virtual ~CacheManager();

public:
	const String&						getName()								{ return _name; }

	uint								getInactiveAgeThreshold()				{ return _inactiveAgeLimit; }
	size_t								getFootprintThreshold()					{ return _inactiveFootprintLimit; }

	void								setInactiveAgeThreshold(uint threshold)	{ _inactiveAgeLimit = threshold; }
	void								setFootprintThreshold(uint threshold)	{ _inactiveFootprintLimit = threshold; }

public:
	void								beginFrame();
	void								touch(CacheHandle* handle);
	void								endFrame();

	void								cacheOut(bool cleanup=false, size_t footprintLimit = 0);
	void								invalidateAll();

public:
	uint								getValidCount()							{ return _wiredCount + _activeCount + _inactiveCount; }
	uint								getValidFootprint()						{ return _wiredFootprint + _activeFootprint + _inactiveFootprint; }

	uint								getWiredCount()							{ return _wiredCount; }
	size_t								getWiredFootprint()						{ return _wiredFootprint; }
	uint								getActiveCount()						{ return _activeCount; }
	size_t								getActiveFootprint()					{ return _activeFootprint; }
	uint								getInactiveCount()						{ return _inactiveCount; }
	size_t								getInactiveFootprint()					{ return _inactiveFootprint; }

private:
	int64								_frameNo;
	int									_inactiveAgeLimit;
	int									_inactiveFootprintLimit;

	typedef set<CacheHandle*>::type		HandleSet;
	HandleSet							_validHandles;

	int									_wiredCount;
	int									_wiredFootprint;

	int									_activeCount;
	int									_activeFootprint;

	int									_inactiveCount;
	int									_inactiveFootprint;

	String								_name;

	friend class CacheHandle;
	void								onHandleValidate(CacheHandle* handle);
	void								onHandleInvalidate(CacheHandle* handle);
	void								onHandleFootprintChanged(CacheHandle* handle, size_t oldFootprint);
	void								onHandleWire(CacheHandle* handle);
	void								onHandleUnwire(CacheHandle* handle);

	static bool							olderHandleFirst(CacheHandle* a, CacheHandle* b);
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
