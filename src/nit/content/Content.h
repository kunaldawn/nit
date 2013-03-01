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

class Content;
class ContentManager;
class ContentSource;
class Package;

////////////////////////////////////////////////////////////////////////////////

// TODO: consider async processing

class NIT_API Content : public TCachableRefCounted<Content, ContentManager, ContentBase>
{
public:
	Content(ContentManager* manager);
	virtual ~Content();

public:
	ContentManager*						getManager()							{ return GetCacheManager(); }

	StreamSource*						getSource()								{ return _source; }
	virtual String						getSourceUrl()							{ return _source ? _source->getUrl().c_str() : ""; }
	Timestamp							getTimestamp()							{ return _timestamp; }
	virtual size_t						getMemorySize() = 0;

	bool								hasProxy()								{ return _proxy != NULL; }
	Content*							getProxy()								{ return _proxy ? _proxy->getProxy() : this; }
	void								setProxy(Content* proxy)				{ _proxy = proxy; }	// TODO: need a event such as OnProxyChanged?

public:
	// Life Cycle:
	//    Create (from Manager)
	// -> Link (with source)			// assigned to a stream source or content source - content source can access to this content at any time
	// -> Load or LoadAsync (from stream)
	// -> Activate (on target system)   // only when active=true
	// -> Deactivate (on target system) // target system can deactivate/reactivate regardless active flag
	// -> Unload (remove cached buffer) // can load again
	// -> Dispose (from content cache)  // can't link again

	bool								isLinked()								{ return _linked; }
	bool								isHeaderLoaded()						{ return _headerLoaded; }
	bool								isLoading()								{ return _loading; }
	bool								isLoaded()								{ return _loaded; }
	bool								isDisposed()							{ return _disposed; }
	bool								hasError()								{ return _error; }

public:
	void								load(bool activate = true);
	void								unload();
	void								dispose();

protected:
	virtual void						onLoad(bool async) = 0;
	virtual void						onUnload() = 0;
	virtual void						onDispose() = 0;
	virtual void						onDelete();

	Ref<StreamSource>					_source;
	Ref<Content>						_proxy;

	Timestamp							_timestamp;

	bool								_linked : 1;
	bool								_headerLoaded : 1;
	bool								_loading : 1;
	bool								_loaded : 1;
	bool								_disposed : 1;
	bool								_error : 1;
};

/////////////////////////////////////////////////////////////////////////////////

class NIT_API ContentSource : public StreamSource
{
public:
	ContentSource(Package* package, const String& name, StreamSource* streamSource, Content* content);

public:
	Package*							getPackage()							{ return static_cast<Package*>(getRealLocator()); }
	StreamSource*						getStreamSource();
	virtual ContentBase*				getContent()							{ return _content; }

protected:								// StreamSource Impl
	virtual size_t						getStreamSize();
	virtual size_t						getMemorySize();
	virtual Timestamp					getTimestamp();
	virtual StreamReader*				open();
	virtual String						_makeUrl()								{ return _streamSource ? _streamSource->_makeUrl() : StreamSource::_makeUrl(); }

protected:
	friend class Package;
	RefCache<StreamSource>				_streamSource;
	RefCache<Content>					_content;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
