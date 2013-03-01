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

#include "nit_pch.h"

#include "CacheHandle.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

CacheHandle::CacheHandle()
: _cacheManager(NULL)
, _cacheFootprint(0)
, _cacheWireCount(0)
, _cacheFrameNo(0)
{

}

bool CacheHandle::validate(CacheManager* manager)
{
	if (manager == NULL) 
		return false;

	if (_cacheManager == manager)
		return true;

	if (_cacheManager != NULL)
		return false;

	_cacheFrameNo = 0;

	if (!onValidate())
		return false;

	if (manager)
	{
		manager->onHandleValidate(this);
		_cacheManager = manager;
	}

	return true;
}

void CacheHandle::onDelete()
{
	invalidate();
}

bool CacheHandle::invalidate()
{
	if (_cacheManager == NULL)
		return true;

	if (isCacheWired())
	{
		if (!onInvalidateWired())
		{
			LOG(0, "*** '%s': can't invalidate wired cache handle\n", _cacheManager->getName().c_str());
			return false;
		}
	}
	else
	{
		onInvalidate();
	}

	_cacheManager->onHandleInvalidate(this);
	_cacheManager = NULL;
	return true;
}

void CacheHandle::setCacheFootprint(size_t size)
{
	size_t oldFootprint = _cacheFootprint;
	_cacheFootprint = size;

	if (_cacheManager)
		_cacheManager->onHandleFootprintChanged(this, oldFootprint);
}

bool CacheHandle::wire()
{
	if (_cacheManager == NULL)
		return false;

	if (++_cacheWireCount == 1)
		_cacheManager->onHandleWire(this);

	return true;
}

void CacheHandle::unwire()
{
	if (_cacheManager == NULL)
		return;

	if (--_cacheWireCount == 0)
		_cacheManager->onHandleUnwire(this);
}

bool CacheHandle::touch()
{
	if (_cacheManager == NULL)
		return false;

	_cacheManager->touch(this);
	return true;
}

////////////////////////////////////////////////////////////////////////////////

CacheManager::CacheManager(const String& name)
{
	_name = name;

	_frameNo					= 1;				// to here set 1, to handle set zero (prevent handles to be active at the beginning)

	_wiredCount				= 0;
	_wiredFootprint			= 0;
	_activeCount				= 0;
	_activeFootprint			= 0;
	_inactiveCount				= 0;
	_inactiveFootprint			= 0;

	_inactiveFootprintLimit	= 4 * 1024 * 1204;	// when total of inactive targets' size over this limit, start caching out
	_inactiveAgeLimit			= 3;				// objects are preserved at least these frames even when caching out
}

CacheManager::~CacheManager()
{
}

void CacheManager::touch(CacheHandle* handle)
{
	if (handle->_cacheFrameNo == _frameNo || handle->isCacheWired())
		return;

	// touch
	handle->_cacheFrameNo = _frameNo;

	--_inactiveCount;
	_inactiveFootprint -= handle->getCacheFootprint();

	++_activeCount;
	_activeFootprint += handle->getCacheFootprint();
}

void CacheManager::onHandleValidate(CacheHandle* handle)
{
	_validHandles.insert(handle);

	if (handle->isCacheWired())
	{
		++_wiredCount;
		_wiredFootprint += handle->getCacheFootprint();
	}
	else
	{
		// treat them active
		handle->_cacheFrameNo = _frameNo;
		++_activeCount;
		_activeFootprint += handle->getCacheFootprint();
	}
}

void CacheManager::onHandleInvalidate(CacheHandle* handle)
{
	if (handle->_cacheFrameNo == _frameNo)
	{
		--_activeCount;
		_activeFootprint -= handle->getCacheFootprint();
	}
	else
	{
		--_inactiveCount;
		_inactiveFootprint -= handle->getCacheFootprint();
	}

	_validHandles.erase(handle);
}

void CacheManager::onHandleFootprintChanged(CacheHandle* handle, size_t oldFootprint)
{
	if (handle->isCacheWired())
	{
		_wiredFootprint -= oldFootprint;
		_wiredFootprint += handle->getCacheFootprint();
	}
	else if (handle->_cacheFrameNo == _frameNo)
	{
		_activeFootprint -= oldFootprint;
		_activeFootprint += handle->getCacheFootprint();
	}
	else
	{
		_inactiveFootprint -= oldFootprint;
		_inactiveFootprint += handle->getCacheFootprint();
	}
}

void CacheManager::onHandleWire(CacheHandle* handle)
{
	if (handle->_cacheFrameNo == _frameNo)
	{
		--_activeCount;
		_activeFootprint -= handle->getCacheFootprint();
	}
	else
	{
		--_inactiveCount;
		_inactiveFootprint -= handle->getCacheFootprint();
	}

	++_wiredCount;
	_wiredFootprint += handle->getCacheFootprint();
}

void CacheManager::onHandleUnwire(CacheHandle* handle)
{
	if (handle->_cacheFrameNo == _frameNo)
	{
		++_activeCount;
		_activeFootprint += handle->getCacheFootprint();
	}
	else
	{
		++_inactiveCount;
		_inactiveFootprint += handle->getCacheFootprint();
	}

	--_wiredCount;
	_wiredFootprint -= handle->getCacheFootprint();
}

bool CacheManager::olderHandleFirst(CacheHandle* a, CacheHandle* b)
{
	// treat frameNo as zero when handle became null
	int64 fa = a ? a->_cacheFrameNo : 0;
	int64 fb = b ? b->_cacheFrameNo : 0;

	return fa < fb;
}

void CacheManager::cacheOut(bool cleanup, size_t footprintLimit)
{
	bool inactiveOnly = footprintLimit == 0;

	// Cache out only when footprint exceeds limit
	if (!cleanup)
	{
		if (inactiveOnly && _inactiveFootprint <= _inactiveFootprintLimit) return;
		else if (getValidFootprint() < footprintLimit) return;
	}

	//	LOG_TIMESCOPE(0, "++ '%s' CacheOut", _name.c_str());

	// forced : cache-out all active, inactive targets (except wired target)
	// inactive : cache-out only inactive targets

	vector<CacheHandle*>::type targets;

	for (HandleSet::iterator itr = _validHandles.begin(), end = _validHandles.end(); itr != end; ++itr)
	{
		CacheHandle* handle = *itr;

		// don't cache out wired handles
		if (handle->isCacheWired()) continue;

		int64 age = _frameNo - handle->_cacheFrameNo;
		bool active = age == 0;

		if (inactiveOnly && active) continue;

		if (!cleanup)
		{
			// Assume handles with zero footprint have some reason not to cache-out
			if (inactiveOnly && handle->getCacheFootprint() == 0) 
				continue;

			// preserve active and have age limit left targets
			if (inactiveOnly && age <= _inactiveAgeLimit)
				continue;
		}

		targets.push_back(handle);
	}

	size_t oldFootprint = inactiveOnly ? _inactiveFootprint : getValidFootprint();
	int killCount = 0;

	// remove old one first
	// NOTE: We may remove larger one first, but larger one tends to need more time to reload,
	// so we decided that old one will get removed first.
	std::sort(targets.begin(), targets.end(), olderHandleFirst);

	// Invalidate all inactive targets
	for (uint i=0; i<targets.size(); ++i)
	{
		CacheHandle* handle = targets[i];

		if (handle->invalidate())
			++killCount;

		// If we acquire enough memory during this process, stop caching-out
		if (!cleanup)
		{
			int footprint = inactiveOnly ? _inactiveFootprint : getValidFootprint();
			if (footprint <= _inactiveFootprintLimit) break;
		}
	}

	if (killCount > 0)
	{
		size_t footprint = inactiveOnly ? _inactiveFootprint : getValidFootprint();
		size_t killBytes = oldFootprint - footprint;
		LOG(0, "++ %s: cache out %d (%dkb) -> %d wired (%dkb) + %d active (%dkb) + %d inactive (%dkb) = %d total (%dkb)\n",
			_name.c_str(),
			killCount,
			killBytes / 1024,
			_wiredCount,
			_wiredFootprint / 1024,
			_activeCount,
			_activeFootprint / 1024,
			_inactiveCount,
			_inactiveFootprint / 1024,
			getValidCount(),
			getValidFootprint() / 1024
			);

		if (_wiredCount + _activeCount + _inactiveCount != _validHandles.size())
		{
			LOG(0, "*** %s: corrupted ValidCount: %d\n", _name.c_str(), _validHandles.size());
		}
	}
}

void CacheManager::beginFrame()
{
	// Targets are active when its frame number matches this frame number.
	// Otherwise they are inactive.

	++_frameNo; // so this increment effectively inactivates all targets until they touch their frame number later.

	_inactiveCount += _activeCount;
	_inactiveFootprint += _activeFootprint;

	_activeCount = 0;
	_activeFootprint = 0;
}

void CacheManager::endFrame()
{
	cacheOut(false, 0);
}

void CacheManager::invalidateAll()
{
	HandleSet workset;

	workset.swap(_validHandles);
	for (HandleSet::iterator itr = workset.begin(), end = workset.end(); itr != end; ++itr)
	{
		CacheHandle* handle = *itr;
		handle->invalidate();
	}
	workset.clear();

	_wiredCount = 0;
	_wiredFootprint = 0;

	_activeCount = 0;
	_activeFootprint = 0;

	_inactiveCount = 0;
	_inactiveFootprint = 0;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
