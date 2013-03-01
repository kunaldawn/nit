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

#include "nit/io/Stream.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NIT_API Archive : public StreamLocator, public PooledAlloc
{
public:
	Archive(const String& name);

public:
	const String&						getName()								{ return this ? _name : StringUtil::BLANK(); }

public:
	virtual void						load() = 0;
	virtual void						unload() = 0;

	virtual bool						isReadOnly() = 0;

	virtual StreamReader*				open(const String& streamName);
	virtual StreamWriter*				create(const String& streamName) = 0;
	virtual StreamWriter*				modify(const String& streamName) = 0;
	virtual void						remove(const String& pattern) = 0;
	virtual void						rename(const String& streamName, const String& newName) = 0;

protected:
	virtual	void						onDelete();
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API CustomArchive : public Archive
{
public:
	CustomArchive(const String& name);

public:									// StreamLocator implementation
	virtual bool						isCaseSensitive()						{ return _caseSensitive; }

	virtual StreamSource*				locateLocal(const String& streamName);
	virtual void						findLocal(const String& pattern, StreamSourceMap& varResults);

public:									// Archive implementation
	virtual void						load()									{ }
	virtual void						unload()								{ }

	virtual bool						isReadOnly()							{ return _readOnly; }

	virtual StreamWriter*				create(const String& streamName);
	virtual StreamWriter*				modify(const String& streamName);
	virtual void						remove(const String& pattern);
	virtual void						rename(const String& streamName, const String& newName);

public:
	void								addLocal(const String& streamName, StreamSource* source);

protected:
	typedef map<String, RefCache<StreamSource> >::type Sources;

	Sources								_sources;

	bool								_readOnly : 1;
	bool								_caseSensitive : 1;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
