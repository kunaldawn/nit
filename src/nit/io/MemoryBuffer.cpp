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

#include "nit/io/MemoryBuffer.h"

#include "nit/runtime/MemManager.h"
#include "nit/io/FileLocator.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

MemoryBuffer* StreamReader::buffer()
{
	if (isSized())
		return new MemoryBuffer(this, getSize());
	else
		return new MemoryBuffer(this);
}

////////////////////////////////////////////////////////////////////////////////

// TODO: ifdef
const size_t MemoryBuffer::s_DefaultBlockSize = 4096;

MemoryBuffer::MemoryBuffer(size_t blockSize)
{
	if (blockSize == 0) blockSize = s_DefaultBlockSize;

	_blockSize = blockSize;
	_start = _end = 0;
}

MemoryBuffer::MemoryBuffer(StreamReader* reader, size_t blockSize)
{
	Ref<StreamReader> safe = reader;

	if (blockSize == 0) blockSize = s_DefaultBlockSize;

	_blockSize = blockSize;
	_start = _end = 0;
	load(reader);
}

MemoryBuffer::MemoryBuffer(const String& string, size_t blockSize)
{
	if (blockSize == 0) blockSize = string.length();
	if (blockSize == 0) blockSize = s_DefaultBlockSize;

	_blockSize = blockSize;
	_start = _end = 0;
	copyFrom(string.c_str(), 0, string.length());
}

MemoryBuffer::MemoryBuffer(const void* buf, size_t size, size_t blockSize)
{
	if (blockSize == 0) blockSize = size;
	if (blockSize == 0) blockSize = s_DefaultBlockSize;

	_blockSize = blockSize;
	_start = _end = 0;
	copyFrom(buf, 0, size);
}

void MemoryBuffer::reserve(size_t size)
{
	if (getSize() > size) return;

	_end = _start + size;

	if (_end == 0) return;

	// if block size set to 32,
	//  0 ~ 32 : 1 block
	// 33 ~ 64 : 2 blocks
	// 65 ~ 96 : 3 blocks
	// ...

	size_t blockCount = (_end - 1) / _blockSize + 1;

	while (_blocks.size() < blockCount)
	{
		uint8* block = allocateBlock();
		_blocks.push_back(block);
	}
}

void MemoryBuffer::resize(size_t size)
{
	if (size == 0)
	{
		_end = _start = 0;
		return;
	}

	if (size > getSize())
	{
		reserve(size);
	}

	_end = _start + size;
}

uint8* MemoryBuffer::allocateBlock()
{
	return (uint8*)NIT_ALLOC(_blockSize);
}

void MemoryBuffer::clear()
{
	for (uint i=0; i<_blocks.size(); ++i)
	{
		uint8* block = _blocks[i];
		NIT_DEALLOC(block, _blockSize);
	}
	_blocks.clear();
	_start = _end = 0;
}

void MemoryBuffer::onDelete()
{
	clear();
}

size_t MemoryBuffer::load(StreamReader* reader, size_t pos, size_t size)
{
	Ref<StreamReader> autorel = reader;

	size_t totalRead = 0;

	if (reader->isSized())
	{
		if (size == 0) size = reader->getSize();

		totalRead = size;
		reserve(pos + size);

		pos = _start + pos;

		size_t blockIdx = pos / _blockSize;
		size_t blockPos = pos % _blockSize;

		while (size > 0)
		{
			uint8* dst = _blocks[blockIdx] + blockPos;

			if (blockPos + size <= _blockSize)
			{
				reader->readRaw(dst, size);
				break;
			}

			size_t readSize = _blockSize - blockPos;
			reader->readRaw(dst, readSize);

			size -= readSize;
			++blockIdx;
			blockPos = 0;
		}

		return totalRead;
	}

	pos = _start + pos;

	while (true)
	{
		if (reader->isEof())
			break;

		if (pos >= _blocks.size() * _blockSize)
		{
			// It seems that GetAvailable() is more expensive.
			// So takes some room (+1) for efficiency.
			_blocks.push_back(allocateBlock());
		}

		size_t blockIdx = pos / _blockSize;
		size_t blockPos = pos % _blockSize;
		uint8* dst = _blocks[blockIdx] + blockPos;

		size_t bytesRead = _blockSize - blockPos;

		if (size && totalRead + bytesRead > size)
			bytesRead = size - totalRead;

		bytesRead = reader->readRaw(dst, bytesRead);

		pos += bytesRead;
		totalRead += bytesRead;

		if (size && totalRead >= size) break;
		if (size == 0 && bytesRead == 0) break;
	}

	if (_end < pos)
		_end = pos;

	return totalRead;
}

size_t MemoryBuffer::save(StreamWriter* writer, size_t pos, size_t size) const
{
	Ref<StreamWriter> autoRel = writer;

	if (size == 0) size = getSize() - pos;

	ASSERT_THROW(pos + size <= getSize(), EX_INVALID_RANGE);

	pos = _start + pos;

	size_t blockIdx = pos / _blockSize;
	size_t blockPos = pos % _blockSize;

	size_t totalWritten = 0;

	while (size > 0)
	{
		uint8* src = _blocks[blockIdx] + blockPos;

		if (blockPos + size <= _blockSize)
		{
			if (writer->writeRaw(src, size) != size)
				NIT_THROW(EX_WRITE);
			totalWritten += size;
			break;
		}

		size_t writeSize = _blockSize - blockPos;
		if (writer->writeRaw(src, writeSize) != writeSize)
			NIT_THROW(EX_WRITE);

		totalWritten += writeSize;
		size -= writeSize;

		++blockIdx;
		blockPos = 0;
	}

	return totalWritten;
}

void MemoryBuffer::copyFrom(const void* buf, size_t pos, size_t size)
{
	reserve(pos + size);

	pos = _start + pos;

	size_t blockIdx = pos / _blockSize;
	size_t blockPos = pos % _blockSize;

	const uint8* src = (const uint8*)buf;

	while (size > 0)
	{
		uint8* dst = _blocks[blockIdx] + blockPos;

		if (blockPos + size <= _blockSize)
		{
			memcpy(dst, src, size);
			break;
		}

		size_t bytesRead = _blockSize - blockPos;
		memcpy(dst, src, bytesRead);
		size -= bytesRead;
		src += bytesRead;

		++blockIdx;
		blockPos = 0;
	}
}

void MemoryBuffer::copyFrom(MemoryBuffer* src, size_t srcPos, size_t destPos, size_t size)
{
	// TODO: optimize this

	MemoryBuffer::Access access(src, srcPos, size);
	copyFrom(access.getMemory(), destPos, access.getSize());
}

MemoryBuffer* MemoryBuffer::clone(size_t blockSize)
{
	if (blockSize == 0)
		blockSize = _blockSize;

	MemoryBuffer* cloned = new MemoryBuffer(blockSize);
	cloned->copyFrom(this, 0, 0, getSize());
	return cloned;
}

void MemoryBuffer::copyTo(void* buf, size_t pos, size_t size) const
{
	ASSERT_THROW(pos + size <= getSize(), EX_INVALID_RANGE);

	pos = _start + pos;

	size_t blockIdx = pos / _blockSize;
	size_t blockPos = pos % _blockSize;

	uint8* dst = (uint8*)buf;

	while (size > 0)
	{
		uint8* src = _blocks[blockIdx] + blockPos;

		if (blockPos + size <= _blockSize)
		{
			memcpy(dst, src, size);
			break;
		}

		size_t bytesWritten = _blockSize - blockPos;
		memcpy(dst, src, bytesWritten);
		size -= bytesWritten;
		dst += bytesWritten;

		++blockIdx;
		blockPos = 0;
	}
}

bool MemoryBuffer::getBlock(size_t blockIdx, uint8*& buf, size_t& size) const
{
	if (blockIdx >= _blocks.size()) return false;

	if (blockIdx == 0)
	{
		buf = _blocks[0] + _start;
		size = _blockSize - _start;
	}
	else
	{
		buf = _blocks[blockIdx];
		size = _blockSize;
	}

	if (blockIdx == _end / _blockSize)
	{
		size_t blockEnd = _end % _blockSize;
		size -= (_blockSize - blockEnd);
	}

	return true;
}

void MemoryBuffer::pushBack(const void* buf, size_t size)
{
	copyFrom(buf, getSize(), size);
}

void MemoryBuffer::pushBack(MemoryBuffer* src, size_t srcPos, size_t size)
{
	// TODO: optimize this
	MemoryBuffer::Access access(src, srcPos, size);
	pushBack(access.getMemory(), access.getSize());
}

void MemoryBuffer::popBack(size_t size)
{
	if (_end >= size)
		_end = size;
	else
		_end = 0;

	if (_end < _start)
		_end = _start;
}

void MemoryBuffer::pushFront(const void* buf, size_t size)
{
	if (_start >= size)
	{
		_start -= size;

		copyFrom(buf, 0, size);
		return;
	}

	NIT_THROW(EX_NOT_IMPLEMENTED);

//	_locks.insert(_blocks.begin(), allocateBlock());
}

void MemoryBuffer::pushFront(MemoryBuffer* src, size_t srcPos, size_t size)
{
	// TODO: optimize this
	MemoryBuffer::Access access(src, srcPos, size);
	pushFront(access.getMemory(), access.getSize());
}

void MemoryBuffer::popFront(size_t size)
{
	assert(size <= getSize());

	if (_start + size < _blockSize)
	{
		_start += size;

		if (isEmpty())
			clear();
		return;
	}

	size_t start = _start + size;
	size_t numBlocks = start / _blockSize;
	size_t blockPos = start % _blockSize;

	// erase popped blocks
	for (uint i=0; i<numBlocks; ++i)
	{
		uint8* block = _blocks[i];
		NIT_DEALLOC(block, _blockSize);
	}

	_blocks.erase(_blocks.begin(), _blocks.begin() + numBlocks);

	_start = blockPos;
	_end -= numBlocks * _blockSize;
}

// TODO: Refactor to create a EncodeUtil class and handle utf16 there

String MemoryBuffer::toString(bool utf16) const
{
	String ret;

	size_t size = getSize();

	if (size == 0) return ret;

	ret.reserve(getSize());

	for (uint i = 0; i < _blocks.size(); ++i)
	{
		uint8* buf; size_t size;
		getBlock(i, buf, size);
		ret.append(buf, buf + size);
	}

	if (utf16)
		ret = Unicode::toUtf8((UniChar*)ret.c_str(), ret.size() / 2);

	return ret;
}

String MemoryBuffer::toString(size_t pos, size_t size, bool utf16) const
{
	if (size == 0) size = getSize() - pos;

	ASSERT_THROW(pos + size <= getSize(), EX_INVALID_RANGE);

	String ret;

	if (size == 0) return ret;

	ret.reserve(size);

	pos = _start + pos;

	size_t blockIdx = pos / _blockSize;
	size_t blockPos = pos % _blockSize;

	while (size > 0)
	{
		uint8* src = _blocks[blockIdx] + blockPos;

		if (blockPos + size <= _blockSize)
		{
			ret.append(src, src + size);
			break;
		}

		size_t bytesWritten = _blockSize - blockPos;
		ret.append(src, src + bytesWritten);
		size -= bytesWritten;

		++blockIdx;
		blockPos = 0;
	}

	if (utf16)
		ret = Unicode::toUtf8((UniChar*)ret.c_str(), ret.size() / 2);

	return ret;
}

void MemoryBuffer::hexDump() const
{
	uint columns = _blockSize <= 32 ? _blockSize : 32;

	for (uint i = 0; i < _blocks.size(); ++i)
	{
		uint begin = i == 0 ? _start : 0;
		uint end = i == _blocks.size() - 1 ? _end % _blockSize : 0;

		MemoryAccess::hexDump(StringUtil::format("block %d", i), _blocks[i], _blockSize, columns, begin, end);
	}
}

////////////////////////////////////////////////////////////////////////////////

MemoryBuffer::Access::Access(MemoryBuffer* buffer, size_t pos, size_t size)
: _buffer(buffer)
{
	if (size == 0) size = buffer->getSize() - pos;

	_bufferPos = pos;
	pos += buffer->_start;

	size_t blockSize = buffer->getBlockSize();
	size_t blockIdx = pos / blockSize;
	size_t blockPos = pos % blockSize;

	if (blockPos + size > blockSize)
	{
		_flatten = true;
		_memory = (uint8*)NIT_ALLOC(size);
		buffer->copyTo(_memory, _bufferPos, size);
	}
	else
	{
		_flatten = false;
		_memory = buffer->_blocks[blockIdx] + blockPos;
	}

	_pos = 0;
	_size = size;
}

MemoryBuffer::Access::~Access()
{
	if (_flatten)
	{
		// TODO: If not a single writing occurs yet, do not copy
		_buffer->copyFrom(_memory, _bufferPos, _size);
		NIT_DEALLOC(_memory, _size);
	}
}

////////////////////////////////////////////////////////////////////////////////

MemorySource::MemorySource(const String& name, MemoryBuffer* buffer)
: StreamSource(NULL, name)
{
	_buffer = buffer ? buffer : new MemoryBuffer();
}

MemorySource::MemorySource(StreamSource* real, MemoryBuffer* buffer)
: StreamSource(real->getLocator(), real->getName(), real->getContentType())
{
	_buffer = buffer ? buffer : new MemoryBuffer();
	_timestamp = real->getTimestamp();
}

StreamReader* MemorySource::newReader()
{
	return new MemoryBuffer::Reader(_buffer, this);
}

StreamWriter* MemorySource::newWriter()
{
	return new MemoryBuffer::Writer(_buffer, this);
}

////////////////////////////////////////////////////////////////////////////////

MemoryBuffer::Reader::Reader(MemoryBuffer* buffer, StreamSource* source, size_t pos, size_t size)
{
	_source = source;
	_buffer = buffer;
	_pos = pos;
	_size = size;
}

MemoryBuffer::Reader::Reader(StreamReader* target, size_t blockSize)
{
	_source = target->getSource();
	_target = target;

	if (blockSize == 0)
		blockSize = target->isSized() ? target->getSize() : s_DefaultBlockSize;

	_buffer = new MemoryBuffer(target, blockSize);

	_pos = 0;
	_size = BUFFER_SIZE;
}

void MemoryBuffer::Reader::skip(int count)
{
	ASSERT_THROW(!(count < 0 && _pos < (uint)-count), EX_INVALID_RANGE);
	ASSERT_THROW(!(count > 0 && _pos + count > getSize()), EX_INVALID_RANGE);
	_pos += count;
}

void MemoryBuffer::Reader::seek(size_t pos)
{
	ASSERT_THROW(0 <= pos && pos <= getSize(), EX_INVALID_RANGE);
	_pos = pos;
}

size_t MemoryBuffer::Reader::readRaw(void* buf, size_t size)
{
	size_t sz = getSize();
	if (_pos + size >= sz)
		size = sz - _pos;

	if (size > 0)
	{
		_buffer->copyTo(buf, _pos, size);
		_pos += size;
	}

	return size;
}

////////////////////////////////////////////////////////////////////////////////

MemoryBuffer::Writer::Writer(size_t blockSize)
{
	_source = NULL;
	_buffer = new MemoryBuffer(blockSize);
	_pos = 0;
}

MemoryBuffer::Writer::Writer(MemoryBuffer* buffer, StreamSource* source)
{
	_source = source;
	_buffer = buffer;
	_pos = 0;
}

MemoryBuffer::Writer::Writer(StreamWriter* target, size_t blockSize)
{
	_source = target->getSource();
	_target = target;
	_buffer = new MemoryBuffer(blockSize);
	_pos = 0;
}

void MemoryBuffer::Writer::skip(int count)
{
	if (_target)
	{
		flush();
		_target->skip(count);
	}
	else
	{
		ASSERT_THROW(!(count < 0 && _pos < (uint)-count), EX_IO);
		_pos += count;
		if (_pos > getSize())
			_buffer->resize(_pos);
	}
}

void MemoryBuffer::Writer::seek(size_t pos)
{
	if (_target)
	{
		flush();
		_target->seek(pos);
	}
	else
	{
		ASSERT_THROW(0 <= pos && pos <= getSize(), EX_IO);
		_pos = pos;
		if (_pos > getSize())
			_buffer->resize(_pos);
	}
}

size_t MemoryBuffer::Writer::writeRaw(const void* buf, size_t size)
{
	if (size > 0)
	{
		_buffer->copyFrom(buf, _pos, size);
		_pos += size;
	}

	return size;
}

bool MemoryBuffer::Writer::flush()
{
	if (_target)
	{
		_buffer->save(_target, 0, _pos);
		_pos = 0;
		return _target->flush();
	}

	return true;
}

void MemoryBuffer::Writer::onDelete()
{
	flush();
	StreamWriter::onDelete();
}

size_t MemoryBuffer::Writer::copy(StreamReader* reader, size_t offset, size_t len, size_t bufSize)
{
	Ref<StreamReader> safe = reader;
	
	if (reader->isBuffered())
	{
		Ref<MemoryBuffer> buf = reader->buffer();
		size_t srcPos = reader->tell() + offset;
		len = len ? len : buf->getSize() - srcPos;
		_buffer->copyFrom(buf, srcPos, _pos, len);
		reader->skip(offset + len);
		_pos += len;
		return len;
	}

	if (offset)
	{
		if (reader->isSeekable())
			reader->skip(offset);
		else
		{
			// TODO: utilize _buffer
			return StreamWriter::copy(reader, offset, len, bufSize);
		}
	}

	size_t totalRead = _buffer->load(reader, _pos, len);

	_pos += totalRead;

	return totalRead;
}

////////////////////////////////////////////////////////////////////////////////

void MemoryAccess::hexDump(const String& title, const uint8* memory, uint size, uint column, uint begin, uint end)
{
	LOG(0, "%s: %08x - %08x\n", title.c_str(), memory, memory + size);

	if (end == 0) 
		end = size;

	uint pos = 0;

	while (pos < size)
	{
		const uint8* addr = memory + pos;

		if (pos + column < begin)
		{
			pos += column;
			continue;
		}

		if (pos >= end)
			break;

		String hex;
		String ascii;

		for (uint i = 0; i < column; ++i)
		{
			if (pos < begin || pos >= end)
			{
				ascii.push_back('-');
				hex += "-- ";
			}
			else
			{
				uint8 byte = *(memory + pos);
				if (byte >= 32 && byte <= 127)
					ascii.push_back(byte);
				else
					ascii.push_back('.');

				hex += StringUtil::format("%02x ", byte);
			}
			++pos;
		}
		LOG(0, "  %08x: %s %s\n", addr, hex.c_str(), ascii.c_str());
	}
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
