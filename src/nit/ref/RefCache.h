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

#include "RefCounted.h"

NS_NIT_BEGIN;

// TODO: NOT YET COMPLETED MT-SAFE (CAUTION WITH ASYNC USAGE)

////////////////////////////////////////////////////////////////////////////////

template <typename T>
class RefCache
{
private:
	typedef typename T::CacheEntry CacheEntry;

public:
	inline RefCache(T* obj=NULL) : _entry(obj ? obj->getCacheEntry() : NULL)	{ }
	inline RefCache(const RefCache& other) : _entry(other._entry)				{ }

	// auto casting
	inline operator T* () const													{ return get(); }
	inline T& operator* () const												{ return *get(); }
	inline T* operator-> () const												{ return get(); }

	// assignment
	inline RefCache& operator= (const RefCache& other)							{ _entry = other._entry; return *this; }
	inline RefCache& operator= (T* obj)											{ _entry = obj ? obj->getCacheEntry() : NULL; return *this; }

	// comparisons
	inline bool operator== (T* obj) const										{ return get() == obj; }
	inline bool operator!= (T* obj) const										{ return get() != obj; }
	inline bool operator== (const RefCache& other) const						{ return _entry == other._entry; }
	inline bool operator!= (const RefCache& other) const						{ return _entry != other._entry; }
	inline bool operator< (const RefCache& other) const							{ return _entry < other._entry; }

	// utility
	inline T* get() const														{ return static_cast<T*>(_entry ? _entry->getCached() : NULL); }

protected:
	Ref<CacheEntry>						_entry;
};

////////////////////////////////////////////////////////////////////////////////

template <typename TClass, typename TCacheManager, typename TBaseRefCounted>
class TCachableRefCounted : public TBaseRefCounted
{
protected:
	TCachableRefCounted(TCacheManager* cacheManager)
		: _cacheManager(cacheManager), _cacheEntry(NULL)
	{
	}

public:
	class CacheEntry : public MTRefCounted
	{
	public:
		CacheEntry(TCachableRefCounted* cached, TCacheManager* manager) 
			: _cached(cached), _manager(manager) 
		{ 
		}

		virtual ~CacheEntry()
		{
			discard();
		}

	public:
		inline TCachableRefCounted* getCached() 
		{ 
			return _cached; 
		}

		void discard()
		{
			if (_cached && _cached->_cacheEntry == this)
			{
				// TCached::OnZeroRef() will not be deleted when CacheEntry alive
				_cached->incRefCount();
				_cached->_cacheEntry = NULL;
				_cached->decRefCount(); 
				_cached = NULL;
				_manager = NULL;

				// OnZeroRef will invoked again when ref of cached was zero.
				// And then deleted normally as no cache entry bound to the ref.
			}
		}

	private:
		friend class TCachableRefCounted;
		TCachableRefCounted*			_cached;
		Weak<TCacheManager>				_manager;
	};

public:
	inline bool hasCacheEntry()
	{
		return _cacheEntry != NULL;
	}

	inline bool IsCached()
	{ 
		return _cacheEntry && TCachableRefCounted::getRefCount() == 0;
	}

	Ref<CacheEntry> getCacheEntry()
	{
		if (_cacheEntry)
			return _cacheEntry;

		_cacheEntry = new CacheEntry(static_cast<TClass*>(this), _cacheManager);
		return _cacheEntry;
	}

protected:
	inline TCacheManager* GetCacheManager()
	{
		if (_cacheManager == NULL && _cacheEntry)
			_cacheManager = _cacheEntry->_manager.get();

		return _cacheManager;
	}

	inline void SetCacheManager(TCacheManager* manager)
	{
		if (TCachableRefCounted::getRefCount() != 0)
			_cacheManager = manager;

		if (_cacheEntry)
			_cacheEntry->_manager = manager;
	}

	virtual void onZeroRef()
	{
		// If someone caching this ref, do not delete now (will be deleted when the cache is removed and no other ref present)
		if (_cacheEntry)
		{
			// NOTE: To cope with 'the tree hung from the last leaf' problem depicted as:
			// When a PackArchive has many files as cache, and all the refs for the PackArchive are gone,
			// somewhere user code clings to 'app.cfg' file of the Archive.
			// As the last ref alive, the file's count should be one. And the PackArchive is hung from the file.
			// At this situation, later user code releases the file,
			// OnZeroRef() for app.cfg will release the PackArchive also, 
			// and this causes re-entry to OnZeroRef() without _cacheManager variable set to NULL properly.

			Ref<TCacheManager> safe = _cacheManager;

			// Breaks circular reference to the manager temporary (Later, cache will relink the reference)
			_cacheManager = NULL;
			return; // As cache still alive, do not delete here (no super call)
		}

		TBaseRefCounted::onZeroRef();
	}

private:
	friend class CacheEntry;
	Ref<TCacheManager>					_cacheManager;
	CacheEntry*							_cacheEntry;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
