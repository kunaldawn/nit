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

#include "nit/io/Archive.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

Archive::Archive(const String& name)
: StreamLocator(name)
{
}

StreamReader* Archive::open(const String& streamName)
{
	return locate(streamName)->open();
}

void Archive::onDelete()
{
	unload();
	StreamLocator::onDelete();
}

////////////////////////////////////////////////////////////////////////////////

CustomArchive::CustomArchive(const String& name)
: Archive(name)
{
	_readOnly = true;
	_caseSensitive = false;
}

StreamSource* CustomArchive::locateLocal(const String& streamName)
{
	Sources::iterator itr = _sources.find(streamName);

	return itr != _sources.end() ? itr->second : NULL;
}

void CustomArchive::findLocal(const String& pattern, StreamSourceMap& varResults)
{
	for (Sources::iterator itr = _sources.begin(); itr != _sources.end(); ++itr)
	{
		if (Wildcard::match(pattern, itr->first, !_caseSensitive) && varResults.find(itr->first) == varResults.end())
			varResults.insert(std::make_pair(itr->first, itr->second));
	}
}

StreamWriter* CustomArchive::create(const String& streamName)
{
	NIT_THROW(EX_NOT_IMPLEMENTED);
}

StreamWriter* CustomArchive::modify(const String& streamName)
{
	NIT_THROW(EX_NOT_IMPLEMENTED);
}

void CustomArchive::remove(const String& pattern)
{
	NIT_THROW(EX_NOT_IMPLEMENTED);
}

void CustomArchive::rename(const String& streamName, const String& newName)
{
	NIT_THROW(EX_NOT_IMPLEMENTED);
}

void CustomArchive::addLocal(const String& streamName, StreamSource* source)
{
	if (_sources.find(streamName) != _sources.end())
		NIT_THROW_FMT(EX_DUPLICATED, "'%s' already exists", streamName.c_str());

	_sources.insert(std::make_pair(streamName, source));
}

///////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
