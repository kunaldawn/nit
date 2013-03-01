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

#include "nit/io/FileLocator.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

// Platform-wise implementation moved to each 'FileLocator_<platform>.cpp' file

////////////////////////////////////////////////////////////////////////////////

// Common implementations

////////////////////////////////////////////////////////////////////////////////

void FileUtil::normalizeSeparator(String& varPath)
{
	if (s_PathSeparator != s_PathAntiSeparator)
	{
		std::replace(varPath.begin(), varPath.end(), s_PathAntiSeparator, s_PathSeparator);
	}

	while(!varPath.empty() && varPath[varPath.length()-1] == s_PathSeparator)
		varPath.resize(varPath.length()-1);
}

////////////////////////////////////////////////////////////////////////////////

void FileLocator::findLocal(const String& pattern, StreamSourceMap& varResults)
{
	if (_filteredOnly)
		findFiltered(pattern, varResults);
	else
		findFiles(pattern, varResults, _findRecursive);
}

String FileLocator::extractFirstDirectory(const String& path, size_t* splitPos)
{
	size_t pos1 = path.rfind(FileUtil::getPathSeparator());
	size_t pos2 = path.rfind(FileUtil::getPathAntiSeparator());

	if (pos1 == path.npos || ((pos2 != path.npos) && (pos1 < pos2)))
		pos1 = pos2;

	String directory;
	if (pos1 != path.npos)
		directory = path.substr(0, pos1 + 1);

	if (splitPos)
		*splitPos = pos1;

	return directory;
}

String FileLocator::normalizePath(const String& path)
{
	String fullpath;

	if (_baseUrl.empty() || FileUtil::isAbsolutePath(path.c_str()))
		fullpath = path;
	else
		fullpath = _baseUrl + path;

	FileUtil::normalizeSeparator(fullpath);

	return fullpath;
}

void FileLocator::addFiltered(const String& pattern, bool recursive)
{
	StreamSourceMap sl;
	findFiles(pattern, sl, recursive);

	for (StreamSourceMap::iterator itr = sl.begin(), end = sl.end(); itr != end; ++itr)
	{
		if (_filtered.find(itr->first) != _filtered.end())
			continue;

		_filtered.insert(std::make_pair(itr->first, itr->second));
	}
}

void FileLocator::removeFiltered(const String& pattern)
{
	for (FilteredSources::iterator itr = _filtered.begin(); itr != _filtered.end(); )
	{
		if (Wildcard::match(pattern, itr->first, !isCaseSensitive()))
			_filtered.erase(itr++);
		else
			itr++;
	}
}

StreamSource* FileLocator::locateFiltered(const String& streamName)
{
	FilteredSources::iterator itr = _filtered.find(streamName);

	return itr != _filtered.end() ? itr->second : NULL;
}

void FileLocator::findFiltered(const String& pattern, StreamSourceMap& varResults)
{
	for (FilteredSources::iterator itr = _filtered.begin(); itr != _filtered.end(); ++itr)
	{
		if (Wildcard::match(pattern, itr->first, !isCaseSensitive()) && varResults.find(itr->first) == varResults.end())
			varResults.insert(std::make_pair(itr->first, itr->second));
	}
}

String FileLocator::makeUrl(const String& sourceName)
{
	String url = _baseUrl + sourceName;
	if (!isCaseSensitive())
		StringUtil::toLowerCase(url);
	return url;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
