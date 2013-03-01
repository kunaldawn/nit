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

class NIT_API MemoryAccess : public PooledAlloc // TODO: Eliminate this class cuz it's nit so nit!
{
public:
	MemoryAccess(void* memory, size_t size) : _memory((uint8*)memory), _size(size), _pos(0) { }
	virtual ~MemoryAccess()														{ }

public:
	void								reassign(void* memory, size_t size, size_t pos)		{ _memory = (uint8*)memory; _size = size; _pos = pos; }

public:									// Access to memory pointer: for ext api usage
	inline const void*					getMemory()	const						{ return _memory; }
	inline void*						getMemory()								{ return _memory; }
	inline size_t						getSize() const							{ return _size; }

public:									// TBinaryReader/Writer compatible
	inline bool							isEof() const							{ return _pos >= _size; }
	inline void							skip(int count) const					{ _pos += count; }
	inline void							seek(size_t pos) const					{ _pos = pos; }
	inline size_t						tell() const							{ return _pos; }

	inline size_t						readRaw(void* buf, size_t size) const	{ memcpy(buf, _memory + _pos, size); _pos += size; return size; }
	inline size_t						read(void* buf, size_t size) const		{ return readRaw(buf, size); }

	inline size_t						writeRaw(const void* buf, size_t size)	{ memcpy(_memory + _pos, buf, size); _pos += size; return size; }
	inline size_t						write(void* buf, size_t size)			{ return writeRaw(buf, size); }

	inline bool							flush()									{ return true; }

public:
	// WARNING: compatible to TBinaryReader/Writer, but this is not a Ref<>, so do not new!
	MemoryAccess*						_ref() const							{ return const_cast<MemoryAccess*>(this); }
	void								incRefCount()							{ }
	void								decRefCount()							{ }

public:
	static void							hexDump(const String& title, const uint8* memory, uint size, uint column = 16, uint begin = 0, uint end = 0);

protected:
	MemoryAccess()																{ }

	uint8*								_memory;
	size_t								_size;
	mutable size_t						_pos;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API MemoryBuffer : public RefCounted, public PooledAlloc
{
public:
	class Reader;
	class Writer;
	class Access;

	MemoryBuffer(size_t blockSize = 0);
	MemoryBuffer(StreamReader* reader, size_t blockSize = 0);
	MemoryBuffer(const String& string, size_t blockSize = 0);
	MemoryBuffer(const void* buf, size_t size, size_t blockSize = 0);

public:
	size_t								getSize() const							{ return _end - _start; }
	bool								isEmpty() const							{ return _start == _end; }

public:
	size_t								getNumBlocks() const					{ return _blocks.size(); }
	size_t								getBlockSize() const					{ return _blockSize; }
	bool								getBlock(size_t blockIdx, uint8*& buf, size_t& size) const;

public:
	size_t								load(StreamReader* reader, size_t pos = 0, size_t size = 0);
	size_t								save(StreamWriter* writer, size_t pos = 0, size_t size = 0) const;

	void								copyFrom(const void* buf, size_t pos, size_t size);
	void								copyTo(void* buf, size_t pos, size_t size) const;

	void								copyFrom(MemoryBuffer* src, size_t srcPos, size_t destPos, size_t size);

	MemoryBuffer*						clone(size_t blockSize = 0);

public:
	void								compress(bool moreSpeed = false, uint32* outAdler32 = NULL);
	void								uncompress(uint32* outAdler32 = NULL);
	uint32								calcCrc32();
	uint32								calcAdler32();

public:
	void								reserve(size_t size);
	void								resize(size_t size);
	void								clear();

	void								pushFront(const String& str)			{ pushFront(str.c_str(), str.length()); }
	void								pushFront(const void* buf, size_t size);
	void								pushFront(MemoryBuffer* src, size_t srcPos, size_t size);

	void								popFront(size_t size);

	void								pushBack(const String& str)				{ pushBack(str.c_str(), str.length()); }
	void								pushBack(const void* buf, size_t size);
	void								pushBack(MemoryBuffer* src, size_t srcPos, size_t size);

	void								popBack(size_t size);

	void								insert(size_t pos, size_t size)			{ NIT_THROW(EX_NOT_IMPLEMENTED); }
	void								insert(size_t pos, const String& str)	{ NIT_THROW(EX_NOT_IMPLEMENTED); }
	void								insert(size_t pos, const void* buf, size_t size) { NIT_THROW(EX_NOT_IMPLEMENTED); }
	void								insert(size_t pos, MemoryBuffer* src, size_t srcPos, size_t size) { NIT_THROW(EX_NOT_IMPLEMENTED); }

	void								erase(size_t pos, size_t size)			{ NIT_THROW(EX_NOT_IMPLEMENTED); }

public:
	String								toString(bool utf16 = false) const;
	String								toString(size_t pos, size_t size = 0, bool utf16 = false) const;

public:
	void								hexDump() const;

protected:
	uint8*								allocateBlock();

	vector<uint8*>::type				_blocks;
	size_t								_blockSize;
	size_t								_start;
	size_t								_end;

	friend class Reader;
	friend class Writer;
	friend class Access;

private:
	static const size_t					s_DefaultBlockSize;

	virtual void						onDelete();
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API MemoryBuffer::Access : public MemoryAccess
{
public:
	Access(MemoryBuffer* buffer, size_t pos = 0, size_t size = 0);
	virtual ~Access();

protected:
	Ref<MemoryBuffer>					_buffer;
	size_t								_bufferPos;
	bool								_flatten;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API MemoryBuffer::Reader : public StreamReader
{
public:
	enum { BUFFER_SIZE = -1 };
	Reader(MemoryBuffer* buffer, StreamSource* source, size_t pos=0, size_t size = BUFFER_SIZE);
	Reader(StreamReader* target, size_t blockSize = 0);

public:
	virtual StreamSource*				getSource()								{ return _source; }
	StreamReader*						getTarget()								{ return _target; }
	MemoryBuffer*						getBuffer()								{ return _buffer; }

public:
	virtual bool						isBuffered()							{ return true; }
	virtual bool						isSized()								{ return true; }
	virtual bool						isSeekable()							{ return true; }

public:
	virtual bool						isEof()									{ return _pos >= getSize(); }
	virtual size_t						getSize()								{ return _size == BUFFER_SIZE ? _buffer->getSize() : _size; }
	virtual void						skip(int count);
	virtual void						seek(size_t pos);
	virtual size_t						tell()									{ return _pos; }

	virtual size_t						readRaw(void* buf, size_t size);

	virtual MemoryBuffer*				buffer()								{ Ref<StreamReader> safe = this; return _buffer; }

protected:
	Ref<StreamSource>					_source;
	Ref<StreamReader>					_target;
	Ref<MemoryBuffer>					_buffer;
	size_t								_pos;
	size_t								_size;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API MemoryBuffer::Writer : public StreamWriter
{
public:
	Writer(size_t blockSize = 0);
	Writer(MemoryBuffer* buffer, StreamSource* source);
	Writer(StreamWriter* target, size_t blockSize = 0);

public:
	virtual StreamSource*				getSource()								{ return _source; }
	StreamWriter*						getTarget()								{ return _target; }
	MemoryBuffer*						getBuffer()								{ return _buffer; }
	
public:
	virtual bool						isBuffered()							{ return true; }
	virtual bool						isSized()								{ return _target ? _target->isSized() : true; }
	virtual bool						isSeekable()							{ return _target ? _target->isSeekable() : true; }

public:
	virtual size_t						getSize()								{ return _buffer->getSize(); }
	virtual void						skip(int count);
	virtual void						seek(size_t pos);
	virtual size_t						tell()									{ return _pos; }

	virtual size_t						writeRaw(const void* buf, size_t size);
	virtual size_t						copy(StreamReader* reader, size_t offset = 0, size_t len = 0, size_t bufSize = 0);
	virtual bool						flush();

protected:
	Ref<StreamSource>					_source;
	Ref<StreamWriter>					_target;
	Ref<MemoryBuffer>					_buffer;
	size_t								_pos;

	virtual void						onDelete();
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API MemorySource : public StreamSource
{
public:
	MemorySource(const String& name, MemoryBuffer* buffer=NULL);
	MemorySource(StreamSource* real, MemoryBuffer* buffer=NULL);

public:									// StreamSource implementation
	virtual size_t						getStreamSize()							{ return _buffer->getSize(); }
	virtual size_t						getMemorySize()							{ return _buffer->getSize(); }
	virtual Timestamp					getTimestamp()							{ return _timestamp; }
	virtual StreamReader*				open()									{ return newReader(); }
	virtual uint32						calcCrc32()								{ return _buffer->calcCrc32(); }
	virtual uint32						calcAdler32()							{ return _buffer->calcAdler32(); }

public:
	MemoryBuffer*						getBuffer()								{ return _buffer; }
	StreamReader*						newReader();
	StreamWriter*						newWriter();

	void								setLocator(StreamLocator* locator)		{ setRealLocator(locator); }
	void								setName(const String& name)				{ _name = name; }
	void								setContentType(const ContentType& ct)	{ _contentType = ct; }
	void								setTimestamp(const Timestamp& t)		{ _timestamp = t; }

public:
	Ref<MemoryBuffer>					_buffer;
	Timestamp							_timestamp;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
