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

#include "nit/io/Stream.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NIT_API ZStreamReader : public StreamReader
{
public:
	ZStreamReader(StreamReader* from, size_t bufSize=0);

public:
	StreamReader*						getFrom()								{ return _from; }
	uint32								finish();

public:									// StreamReader impl
	virtual StreamSource*				getSource()								{ return _from->getSource(); }
	virtual bool						isBuffered()							{ return false; }
	virtual bool						isSized()								{ return false; }
	virtual bool						isSeekable()							{ return false; }
	virtual bool						isEof();
	virtual size_t						getSize()								{ NIT_THROW(EX_NOT_SUPPORTED); }
	virtual void						skip(int count)							{ NIT_THROW(EX_NOT_SUPPORTED); }
	virtual void						seek(size_t pos)						{ NIT_THROW(EX_NOT_SUPPORTED); }
	virtual size_t						tell()									{ NIT_THROW(EX_NOT_SUPPORTED); }
	virtual size_t						readRaw(void* buf, size_t size);

protected:
	virtual void						onDelete();

private:
	Ref<StreamReader>					_from;
	void*								_zstream;
	uint8*								_buffer;
	size_t								_bufSize;
	uint32								_adler32;

	void								close();
};

////////////////////////////////////////////////////////////////////////////////

// NOTE: You may be suprised when zlib does so called 'compression' with a png or jpg file 
// larger than their original size. It's best practice that compare actual compressed size
// to original size when you handle such files which are of high-entrophy.

class NIT_API ZStreamWriter : public StreamWriter
{
public:
	ZStreamWriter(StreamWriter* to, bool moreSpeed = false, bool flushOnlyOnClose = true, size_t bufSize=0);

public:
	StreamWriter*						getTo()									{ return _to; }
	uint32								finish();

public:									// StreamWriter impl
	virtual StreamSource*				getSource()								{ return _to->getSource(); }
	virtual bool						isBuffered()							{ return false; }
	virtual bool						isSized()								{ return false; }
	virtual bool						isSeekable()							{ return false; }
	virtual size_t						getSize()								{ NIT_THROW(EX_NOT_SUPPORTED); }
	virtual void						skip(int count)							{ NIT_THROW(EX_NOT_SUPPORTED); }
	virtual void						seek(size_t pos)						{ NIT_THROW(EX_NOT_SUPPORTED); }
	virtual size_t						tell()									{ NIT_THROW(EX_NOT_SUPPORTED); }
	virtual size_t						writeRaw(const void* buf, size_t size);
	virtual bool						flush();

protected:
	virtual void						onDelete();

private:
	Ref<StreamWriter>					_to;
	void*								_zstream;
	uint8*								_buffer;
	size_t								_bufSize;
	uint32								_adler32;
	bool								_flushOnlyOnClose;

	bool								doFlush(bool finish);
	void								close();
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
