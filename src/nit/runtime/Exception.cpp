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

#include "Exception.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

int Exception::s_LastExceptionCode = EX_NULL;

Exception::Exception(int code)
: _code(code), _line(-1)
{
}

Exception::Exception(int code, const String& desc)
: _code(code), _line(-1), _description(desc)
{
}

Exception::Exception(int code, const String& desc, const String& source, const String& file, int line)
: _code(code), _line(line), _description(desc), _source(source), _file(file)
{
}

Exception::Exception(const Exception& other)
: _code(other._code), _line(other._line), _description(other._description), _source(other._source), _file(other._file)
{
}

Exception& Exception::operator=(const Exception& other)
{
	if (&other != this)
	{
		_code = other._code;
		_line = other._line;
		_description = other._description;
		_source = other._source;
		_file = other._file;
	}
	return *this;
}

const String& Exception::getFullDescription() const
{
	if (_fullDesc.empty())
	{
		char buf[64];

		_fullDesc.append(name());
		sprintf(buf, "(#%d", _code);
		_fullDesc.append(buf);

		if (_description.empty())
			_fullDesc.push_back(')');
		else
		{
			_fullDesc.append("): ");
			_fullDesc.append(_description);
		}

		if (!_source.empty())
		{
			_fullDesc.append(" in ");
			_fullDesc.append(_source);
		}

		if (!_file.empty())
		{
			_fullDesc.append(" at ");
			_fullDesc.append(_file);
			if (_line > 0)
			{
				sprintf(buf, " (line %d)", _line);
				_fullDesc.append(buf);
			}
		}
	}

	return _fullDesc;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
