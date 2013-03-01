/*
-----------------------------------------------------------------------------
This source file is from part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#include "nit_pch.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

void StringUtil::trim(String& str, bool left, bool right)
{
	static const String delims = " \t\r\n";
	if(right)
		str.erase(str.find_last_not_of(delims)+1); // trim right
	if(left)
		str.erase(0, str.find_first_not_of(delims)); // trim left
}

vector<String>::type StringUtil::split( const String& str, const String& delims, unsigned int maxSplits)
{
	vector<String>::type ret;
	// Pre-allocate some space for performance
	ret.reserve(maxSplits ? maxSplits+1 : 10);    // 10 is guessed capacity for most case

	unsigned int numSplits = 0;

	// Use STL methods 
	size_t start, pos;
	start = 0;
	do 
	{
		pos = str.find_first_of(delims, start);
		if (pos == start)
		{
			// Do nothing
			start = pos + 1;
		}
		else if (pos == String::npos || (maxSplits && numSplits == maxSplits))
		{
			// Copy the rest of the string
			ret.push_back( str.substr(start) );
			break;
		}
		else
		{
			// Copy up to delimiter
			ret.push_back( str.substr(start, pos - start) );
			start = pos + 1;
		}
		// parse up to next real data
		start = str.find_first_not_of(delims, start);
		++numSplits;

	} while (pos != String::npos);



	return ret;
}

vector< String >::type StringUtil::tokenise( const String& str, const String& singleDelims, const String& doubleDelims, unsigned int maxSplits)
{
	vector<String>::type ret;
	// Pre-allocate some space for performance
	ret.reserve(maxSplits ? maxSplits+1 : 10);    // 10 is guessed capacity for most case

	unsigned int numSplits = 0;
	String delims = singleDelims + doubleDelims;

	// Use STL methods 
	size_t start, pos;
	char curDoubleDelim = 0;
	start = 0;
	do 
	{
		if (curDoubleDelim != 0)
		{
			pos = str.find(curDoubleDelim, start);
		}
		else
		{
			pos = str.find_first_of(delims, start);
		}

		if (pos == start)
		{
			char curDelim = str.at(pos);
			if (doubleDelims.find_first_of(curDelim) != String::npos)
			{
				curDoubleDelim = curDelim;
			}
			// Do nothing
			start = pos + 1;
		}
		else if (pos == String::npos || (maxSplits && numSplits == maxSplits))
		{
			if (curDoubleDelim != 0)
			{
				//Missing closer. Warn or throw exception?
			}
			// Copy the rest of the string
			ret.push_back( str.substr(start) );
			break;
		}
		else
		{
			if (curDoubleDelim != 0)
			{
				curDoubleDelim = 0;
			}

			// Copy up to delimiter
			ret.push_back( str.substr(start, pos - start) );
			start = pos + 1;
		}
		if (curDoubleDelim == 0)
		{
			// parse up to next real data
			start = str.find_first_not_of(singleDelims, start);
			if (start == String::npos)
				break;
		}

		++numSplits;

	} while (pos != String::npos);

	return ret;
}

void StringUtil::toLowerCase(String& str)
{
	std::transform(
		str.begin(),
		str.end(),
		str.begin(),
		tolower);
}

void StringUtil::toUpperCase(String& str) 
{
	std::transform(
		str.begin(),
		str.end(),
		str.begin(),
		toupper);
}
//-----------------------------------------------------------------------
bool StringUtil::startsWith(const String& str, const String& pattern, bool lowerCase)
{
	size_t thisLen = str.length();
	size_t patternLen = pattern.length();
    if (patternLen == 0) return true;
    
	if (thisLen < patternLen)
		return false;

	String startOfThis = str.substr(0, patternLen);
	if (lowerCase)
		StringUtil::toLowerCase(startOfThis);

	return (startOfThis == pattern);
}

bool StringUtil::endsWith(const String& str, const String& pattern, bool lowerCase)
{
	size_t thisLen = str.length();
	size_t patternLen = pattern.length();
	if (thisLen < patternLen || patternLen == 0)
		return false;

	String endOfThis = str.substr(thisLen - patternLen, patternLen);
	if (lowerCase)
		StringUtil::toLowerCase(endOfThis);

	return (endOfThis == pattern);
}

String StringUtil::standardisePath(const String& init)
{
	String path = init;

#if !defined(NIT_FAMILY_WIN32)
	std::replace( path.begin(), path.end(), '\\', '/' );
	if( path[path.length() - 1] != '/' )
		path += '/';
#else
	std::replace( path.begin(), path.end(), '/', '\\' );
	if( path[path.length() - 1] != '\\' )
		path += '\\';
#endif

	return path;
}

void StringUtil::splitFilename(const String& qualifiedName, 
							   String& outBasename, String& outPath)
{
	String path = qualifiedName;
	// Replace \ with / first

#if defined(NIT_FAMILY_UNIX)
	std::replace( path.begin(), path.end(), '\\', '/' );
	// split based on final /
	size_t i = path.find_last_of('/');
#elif defined(NIT_FAMILY_WIN32)
	std::replace( path.begin(), path.end(), '/', '\\' );
	// split based on final /
	size_t i = path.find_last_of('\\');
#endif

	if (i == String::npos)
	{
		outPath.clear();
		outBasename = qualifiedName;
	}
	else
	{
		outBasename = path.substr(i+1, path.size() - i - 1);
		outPath = path.substr(0, i+1);
	}

}

void StringUtil::splitBaseFilename(const String& fullName, 
								   String& outBasename, String& outExtention)
{
	size_t i = fullName.find_last_of(".");
	if (i == String::npos)
	{
		outExtention.clear();
		outBasename = fullName;
	}
	else
	{
		outExtention = fullName.substr(i+1);
		outBasename = fullName.substr(0, i);
	}
}

void StringUtil::splitFullFilename(	const String& qualifiedName, 
								   String& outBasename, String& outExtention, String& outPath )
{
	String fullName;
	splitFilename( qualifiedName, fullName, outPath );
	splitBaseFilename( fullName, outBasename, outExtention );
}

const String StringUtil::replaceAll(const String& source, const String& replaceWhat, const String& replaceWithWhat)
{
	String result = source;
	while(1)
	{
		String::size_type pos = result.find(replaceWhat);
		if (pos == String::npos) break;
		result.replace(pos,replaceWhat.size(),replaceWithWhat);
	}
	return result;
}

unsigned long StringUtil::hashString(const char* str)
{
	if (!str || strlen(str) == 0)
		return (unsigned long)0;

	unsigned long hash = 5381;
	int c;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c;

	return hash;
}

#pragma warning (disable: 4996) // "may be unsafe using xxx instead"

String StringUtil::format(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	String ret = vformat(fmt, args);
	va_end(args);
	return ret;
}

String StringUtil::vformat(const char* fmt, va_list args)
{
	char buf[MAX_BUF_SIZE];
	int bufLen = vsnprintf(buf, MAX_BUF_SIZE, fmt, args);
	ASSERT_THROW(bufLen >= 0, EX_OVERFLOW);
	return String(buf, bufLen);
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;