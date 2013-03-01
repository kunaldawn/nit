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

#include "nit/content/Content.h"

#include "nit/content/ContentManager.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

Content::Content(ContentManager* manager)
: TCachableRefCounted<Content, ContentManager, ContentBase>(manager)
, _linked(false)
, _headerLoaded(false)
, _loading(false)
, _loaded(false)
, _disposed(false)
, _error(false)
{
	_timestamp = Timestamp::now();
}

Content::~Content()
{
	if (!_disposed)
		LOG(0, "*** Content %08x deleted without disposed\n", this);
}

void Content::load(bool activate)
{
	if (_disposed) NIT_THROW(EX_INVALID_STATE);

	ASSERT_THROW(_linked, EX_INVALID_STATE);

	if (_loaded) 
		return;

	if (_loading) 
		return; // TODO: Should we wait till loaded this case?

	_error = false;

	// TODO: detect if it is async (Thread::Current() == NULL?)

	{
		struct LoadingScope
		{
			Weak<Content> res;
			LoadingScope(Content* res)	: res(res)	{ res->_loading = true; }
			~LoadingScope()					{ if (res) res->_loading = false; }
		};

		LoadingScope lscope(this);

		onLoad(false);
	}

	if (_error)
		return;

	_loaded = true;
}

void Content::unload()
{
	if (!_loaded) return;

	if (_loading)
	{
		NIT_THROW_FMT(EX_INVALID_STATE, "'%s': can't unload while loading", getSourceUrl().c_str());
		// It would be headache when async process try to dispose..
		// TODO: just cancel it or resource manager should treat them by a list
	}

	onUnload();

	_loaded = false;
}

void Content::dispose()
{
	if (_disposed)
		return;

	Ref<Content> safe = this;

	_disposed			= true;

	try
	{
		unload();
	}
	catch (Exception& ex)
	{
		LOG(0, "*** '%s': disposed but unload failed: %s", getSourceUrl().c_str(), ex.getFullDescription().c_str());
	}

	ContentManager* manager = getManager();
	if (manager)
		manager->onDispose(this);

	onDispose();

	_source			= NULL;
	_proxy				= NULL;
	_linked			= false;
	_loading			= false;
	_loaded			= false;

	if (hasCacheEntry())
		getCacheEntry()->discard();
}

void Content::onDelete()
{
	dispose();

	RefCounted::onDelete();
}

////////////////////////////////////////////////////////////////////////////////

ContentSource::ContentSource(Package* package, const String& name, StreamSource* streamSource, Content* content) 
: StreamSource(package, name)
{
	if (streamSource)
	{
		_streamSource	= streamSource;
		_contentType	= streamSource->getContentType();
	}
	_content		= content;
}

StreamSource* ContentSource::getStreamSource()
{
	if (_streamSource)
		return _streamSource;

	return NULL;
}

size_t ContentSource::getStreamSize()
{
	if (_streamSource)
		return _streamSource->getStreamSize();

	return 0;
}

size_t ContentSource::getMemorySize()
{
	if (_content) 
		return _content->getMemorySize();

	if (_streamSource)
		return _streamSource->getMemorySize();

	return 0;
}

Timestamp ContentSource::getTimestamp()
{
	if (_content)
		return _content->getTimestamp();

	if (_streamSource)
		return _streamSource->getTimestamp();

	return Timestamp();
}

StreamReader* ContentSource::open()
{
	if (_streamSource) 
		return _streamSource->open();

	NIT_THROW(EX_NOT_SUPPORTED);
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
