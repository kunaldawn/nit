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

#include "nit/io/ZStream.h"

#include "nit/runtime/MemManager.h"

#include "nit/io/MemoryBuffer.h"

#include "zlib.h"

#define AUTODETECT_ZLIB_GZIP (15+32) // see initInflate2()

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

static voidpf nit_zalloc(voidpf opaque, uInt items, uInt size)
{
	return NIT_ALLOC(items * size);
}

static void nit_zfree(voidpf opaque, voidpf address)
{
	NIT_DEALLOC(address, 0);
}

////////////////////////////////////////////////////////////////////////////////

ZStreamReader::ZStreamReader(StreamReader* from, size_t bufSize)
: _from(from)
, _zstream(NULL)
, _buffer(NULL)
, _adler32(0)
{
	if (bufSize == 0)
		bufSize = 4096;

	_bufSize = bufSize;
	_zstream = NIT_ALLOC(sizeof(z_stream));
	memset(_zstream, 0, sizeof(z_stream));

	z_stream* zs = (z_stream*)_zstream;
	zs->opaque = this;
	zs->zalloc = nit_zalloc;
	zs->zfree = nit_zfree;

	if (inflateInit2(zs, AUTODETECT_ZLIB_GZIP) != Z_OK)
	{
		NIT_DEALLOC(_zstream, sizeof(z_stream));
		NIT_THROW(EX_IO);
	}

	_buffer = (uint8*)NIT_ALLOC(bufSize);
}

bool ZStreamReader::isEof()
{
	return _zstream == NULL;
}

size_t ZStreamReader::readRaw(void* buf, size_t size)
{
	if (size == 0 || isEof())
		return 0;

	z_stream* zs = (z_stream*)_zstream;
	zs->next_out = (uint8*)buf;
	zs->avail_out = size;

	int err = Z_OK;
	while (err == Z_OK && zs->avail_out > 0)
	{
		if (zs->avail_in == 0 && !_from->isEof())
		{
			size_t bytesRead = _from->readRaw(_buffer, _bufSize);
			zs->next_in = _buffer;
			zs->avail_in = bytesRead;
		}
		err = inflate(zs, Z_SYNC_FLUSH);
	}

	switch (err)
	{
	case Z_OK:
		size -= zs->avail_out;
		break;

	case Z_STREAM_END:
		// Consume all the contents from the source stream
		if (zs->avail_in)
		{
			// Rewind if we have read more than needed
			if (_from->isSeekable())
				_from->skip(-int(zs->avail_in));
			else
			{
				// Eek. Have to throw when seeking not supported
				close();
				NIT_THROW(EX_CORRUPTED);
			}
			zs->avail_in = 0;
		}

		size -= zs->avail_out;
		close();
		break;

	case Z_BUF_ERROR:
		// zlib expects more bytes, but 'from' ran out.
		close();
		NIT_THROW_FMT(EX_READ, "unexpected EOF in source stream");
		break;

	case Z_DATA_ERROR:
		close();
		NIT_THROW_FMT(EX_READ, "data error in zstream");
		break;

	default:
		close();
		NIT_THROW_FMT(EX_READ, "zstream error(%d)", err);
	}

	return size;
}

void ZStreamReader::onDelete()
{
	close();

	StreamReader::onDelete();
}

void ZStreamReader::close()
{
	if (_zstream)
	{
		z_stream* zs = (z_stream*)_zstream;
		_adler32 = zs->adler;
		inflateEnd(zs);
		NIT_DEALLOC(_zstream, sizeof(z_stream));
		_zstream = NULL;
	}

	if (_buffer)
	{
		NIT_DEALLOC(_buffer, _bufSize);
		_buffer = NULL;
	}
}

uint32 ZStreamReader::finish()
{
	close();

	return _adler32;
}

////////////////////////////////////////////////////////////////////////////////

ZStreamWriter::ZStreamWriter(StreamWriter* to, bool moreSpeed, bool flushOnlyOnClose, size_t bufSize)
: _to(to)
, _zstream(NULL)
, _buffer(NULL)
, _flushOnlyOnClose(flushOnlyOnClose)
, _adler32(0)
{
	if (bufSize == 0)
		bufSize = 4096;

	_bufSize = bufSize;
	_zstream = NIT_ALLOC(sizeof(z_stream));
	memset(_zstream, 0, sizeof(z_stream));

	_buffer = (uint8*)NIT_ALLOC(bufSize);

	z_stream* zs = (z_stream*)_zstream;
	zs->opaque = this;
	zs->zalloc = nit_zalloc;
	zs->zfree = nit_zfree;
	zs->next_out = _buffer;
	zs->avail_out = _bufSize;

	if (deflateInit(zs, moreSpeed ? Z_BEST_SPEED : Z_BEST_COMPRESSION) != Z_OK)
	{
		NIT_DEALLOC(_zstream, sizeof(z_stream));
		NIT_DEALLOC(_buffer, bufSize);
		NIT_THROW(EX_IO);
	}
}

void ZStreamWriter::close()
{
	if (_zstream)
	{
		doFlush(true);
		z_stream* zs = (z_stream*)_zstream;
		_adler32 = zs->adler;
		deflateEnd(zs);
		NIT_DEALLOC(_zstream, sizeof(z_stream));
		_zstream = NULL;
	}

	if (_buffer)
	{
		NIT_DEALLOC(_buffer, _bufSize);
		_buffer = NULL;
	}
}

size_t ZStreamWriter::writeRaw(const void* buf, size_t size)
{
	if (_zstream == NULL)
		NIT_THROW_FMT(EX_WRITE, "Stream already closed");

	if (size == 0)
		return 0;

	z_stream* zs = (z_stream*)_zstream;
	zs->next_in = (uint8*)buf;
	zs->avail_in = size;

	int err = Z_OK;

	while (err == Z_OK && zs->avail_in > 0)
	{
		if (zs->avail_out == 0)
		{
			size_t bytesWritten = _to->writeRaw(_buffer, _bufSize);
			if (bytesWritten != _bufSize)
			{
				close();
				NIT_THROW_FMT(EX_WRITE, "can't write to target stream");
			}

			zs->next_out = _buffer;
			zs->avail_out = _bufSize;
		}

		err = deflate(zs, Z_NO_FLUSH);
	}

	if (err != Z_OK)
	{
		String msg = zs->msg ? zs->msg : "";
		close();
		NIT_THROW_FMT(EX_WRITE, "can't write zstream: %s (%d)", msg.c_str(), err);
	}

	size -= zs->avail_in;
	return size;
}

bool ZStreamWriter::doFlush(bool finish)
{
	if (_zstream == NULL)
		return true;

	z_stream* zs = (z_stream*)_zstream;

	int err = Z_OK;
	bool done = false;

	while (err == Z_OK || err == Z_STREAM_END)
	{
		size_t len = _bufSize - zs->avail_out;
		if (len)
		{
			size_t bytesWritten = _to->writeRaw(_buffer, len);
			if (bytesWritten != len)
				return false;

			zs->next_out = _buffer;
			zs->avail_out = _bufSize;
		}

		if (done)
			break;

		err = deflate(zs, finish ? Z_FINISH : Z_PARTIAL_FLUSH);
		done = zs->avail_out != 0 || err == Z_STREAM_END;
	}

	return true;
}

void ZStreamWriter::onDelete()
{
	close();

	StreamWriter::onDelete();
}

bool ZStreamWriter::flush()
{
	if (!_flushOnlyOnClose)
		return doFlush(false);

	return false;
}

uint32 ZStreamWriter::finish()
{
	close();
	return _adler32;
}

////////////////////////////////////////////////////////////////////////////////

// TODO: Refactor to somewhere

void MemoryBuffer::compress(bool moreSpeed, uint32* outAdler32)
{
	size_t size = getSize();

	if (size == 0)
		return;

	z_stream zs		= { 0 };
	zs.opaque		= this;
	zs.zalloc		= nit_zalloc;
	zs.zfree		= nit_zfree;

	if (deflateInit(&zs, moreSpeed ? Z_BEST_SPEED : Z_BEST_COMPRESSION) != Z_OK)
		NIT_THROW(EX_IO);

	size_t pos = _start;
	size_t blockIdx = pos / _blockSize;
	size_t blockPos = pos % _blockSize;

	vector<uint8*>::type newBlocks;

	// compress routine
	bool done = false;

	while (true)
	{
		if (size)
		{
			if (zs.avail_in == 0)
			{
				zs.next_in = _blocks[blockIdx] + blockPos;
				zs.avail_in = _blockSize - blockPos;
				if (zs.avail_in > size)
					zs.avail_in = size;
				size -= zs.avail_in;
				++blockIdx;
				blockPos = 0;
			}
		}

		if (zs.avail_out == 0)
		{
			zs.next_out = allocateBlock();
			zs.avail_out = _blockSize;
			newBlocks.push_back(zs.next_out);
		}

		int err = deflate(&zs, done ? Z_FINISH : Z_NO_FLUSH); 
		// TODO: It seems that zlib allocates internal memory within itself when use Z_NO_FLUSH
		// We already have a MemoryBuffer here, so check if such a redundancy avoidable.

		if (err == Z_STREAM_END)
			break;

		if (err != Z_OK)
		{
			String msg = zs.msg ? zs.msg : "";
			deflateEnd(&zs);
			for (uint i=0; i<newBlocks.size(); ++i)
			{
				 NIT_DEALLOC(newBlocks[i], _blockSize);
			}
			NIT_THROW_FMT(EX_WRITE, "can't compress: %s (%d)", msg.c_str(), err);
		}

		done = zs.avail_in == 0 && size == 0;
	}

	clear();
	_blocks.swap(newBlocks);
	_end = zs.total_out;

	if (outAdler32)
		*outAdler32= zs.adler;

	deflateEnd(&zs);
}

void MemoryBuffer::uncompress(uint32* outAdler32)
{
	size_t size = getSize();

	if (size == 0)
		return;

	z_stream zs		= { 0 };
	zs.opaque		= this;
	zs.zalloc		= nit_zalloc;
	zs.zfree		= nit_zfree;

	if (inflateInit2(&zs, AUTODETECT_ZLIB_GZIP))
		NIT_THROW(EX_IO);

	size_t pos = _start;
	size_t blockIdx = pos / _blockSize;
	size_t blockPos = pos % _blockSize;

	vector<uint8*>::type newBlocks;

	// uncompress routine
	while (true)
	{
		if (size)
		{
			if (zs.avail_in == 0)
			{
				zs.next_in = _blocks[blockIdx] + blockPos;
				zs.avail_in = _blockSize - blockPos;
				if (zs.avail_in > size)
					zs.avail_in = size;
				size -= zs.avail_in;
				++blockIdx;
				blockPos = 0;
			}
		}

		if (zs.avail_out == 0)
		{
			zs.next_out = allocateBlock();
			zs.avail_out = _blockSize;
			newBlocks.push_back(zs.next_out);
		}

		int err = inflate(&zs, Z_SYNC_FLUSH);

		if (err == Z_STREAM_END)
			break;

		if (err != Z_OK)
		{
			String msg = zs.msg ? zs.msg : "";
			inflateEnd(&zs);
			for (uint i=0; i<newBlocks.size(); ++i)
			{
				NIT_DEALLOC(newBlocks[i], _blockSize);
			}
			NIT_THROW_FMT(EX_WRITE, "can't uncompress: %s (%d)", msg.c_str(), err);
		}
	}

	clear();
	_blocks.swap(newBlocks);
	_end = zs.total_out;

	if (outAdler32)
		*outAdler32 = zs.adler;

	inflateEnd(&zs);
}

////////////////////////////////////////////////////////////////////////////////

uint32 StreamUtil::calcCrc32(StreamReader* r)
{
	Ref<StreamReader> safe = r;

	ulong crc = crc32(0L, Z_NULL, 0);

	uint8 buf[4096];

	while (true)
	{
		size_t bytesRead = r->readRaw(buf, sizeof(buf));
		if (bytesRead == 0) break;
		crc = crc32(crc, buf, bytesRead);
	}

	return crc;
}

uint32 StreamUtil::calcCrc32(const void* buf, size_t size)
{
	ulong crc = crc32(0L, Z_NULL, 0);

	crc = crc32(crc, (const uint8*)buf, size);

	return crc;
}

////////////////////////////////////////////////////////////////////////////////

CalcCRC32Writer::CalcCRC32Writer()
{
	_value = crc32(0L, Z_NULL, 0);
}

size_t CalcCRC32Writer::writeRaw(const void* buf, size_t size)
{
	_value = crc32(_value, (uint8*)buf, size);
	return size;
}

////////////////////////////////////////////////////////////////////////////////

uint32 StreamSource::calcCrc32()
{
	ulong crc = crc32(0L, Z_NULL, 0);

	Ref<StreamReader> r = open();

	uint8 buf[4096];

	while (true)
	{
		size_t bytesRead = r->readRaw(buf, sizeof(buf));
		if (bytesRead == 0) break;
		crc = crc32(crc, buf, bytesRead);
	}

	return crc;
}

uint32 StreamSource::calcAdler32()
{
	uLong adler = adler32(0L, Z_NULL, 0);

	Ref<StreamReader> r = open();

	uint8 buf[4096];

	while (true)
	{
		size_t bytesRead = r->readRaw(buf, sizeof(buf));
		if (bytesRead == 0) break;
		adler = adler32(adler, buf, bytesRead);
	}

	return adler;
}

////////////////////////////////////////////////////////////////////////////////

uint32 MemoryBuffer::calcCrc32()
{
	uLong crc = crc32(0L, Z_NULL, 0);

	for (uint i = 0; i < _blocks.size(); ++i)
	{
		uint8* buf; size_t size;
		getBlock(i, buf, size);
		crc = crc32(crc, buf, size);
	}

	return crc;
}

uint32 MemoryBuffer::calcAdler32()
{
	uLong adler = adler32(0L, Z_NULL, 0);

	for (uint i = 0; i < _blocks.size(); ++i)
	{
		uint8* buf; size_t size;
		getBlock(i, buf, size);
		adler = adler32(adler, buf, size);
	}

	return adler;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
