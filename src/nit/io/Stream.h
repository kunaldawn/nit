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
#include "nit/data/DateTime.h"
#include "nit/io/ContentTypes.h"
#include "nit/ref/RefCache.h"

#undef require

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class StreamSource;
class StreamLocator;
class StreamReader;
class StreamWriter;
class MemoryBuffer;

typedef multimap<String, Ref<StreamSource> >::type StreamSourceMap;
typedef vector<Ref<StreamLocator> >::type StreamLocatorList;

////////////////////////////////////////////////////////////////////////////////

class NIT_API StreamUtil
{
public:
	static void							flipEndian(void * pData, size_t size);

	template <typename TValue>
	static void							flipEndian(TValue& value)				{ return flipEndian(&value, sizeof(value)); }

	static uint32						calcCrc32(StreamReader* r);
	static uint32						calcCrc32(const void* buf, size_t size);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API ContentBase : public MTRefCounted
{
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API StreamLocator : public MTRefCounted
{
public:
	StreamLocator(const String& name);
	virtual ~StreamLocator();

public:
	const String&						getName()								{ return this ? _name : StringUtil::BLANK(); }

public:
	// URL : Uniform Resource Locator : String which represents a Locator + Source pair.
	// This can be used as a key for a resource.
	// Similar to a web url but this is solely for local runtime resource management.

	virtual String						makeUrl(const String& sourceName);

	virtual bool						isCaseSensitive() = 0;

public:
	bool								hasProxy()								{ return _proxy != NULL; }
	StreamLocator*						getProxy()								{ return this && _proxy ? _proxy->getProxy() : this; }
	void								setProxy(StreamLocator* proxy)			{ _proxy = proxy; }

public:
	virtual StreamSource*				locate(const String& streamName, bool throwEx = true);
	virtual StreamSource*				locate(const String& streamName, const String& locatorPattern, bool throwEx = true);

	virtual void						find(const String& pattern, StreamSourceMap& varResults);
	virtual void						find(const String& streamPattern, const String& locatorPattern, StreamSourceMap& varResults);

	StreamLocator*						getLocator(const String& locatorName, bool throwEx = true);
	void								findLocator(const String& locatorPattern, StreamLocatorList& varResults);

public:
	virtual StreamSource*				locateLocal(const String& streamName) = 0;
	virtual void						findLocal(const String& pattern, StreamSourceMap& varResults) = 0;

public:
	const StreamLocatorList&			getRequiredList()						{ return _required; }
	bool								isRequired(StreamLocator* req);
	virtual void						require(StreamLocator* req, bool first = false);
	virtual void						unrequire(StreamLocator* req);
	virtual void						unrequireAll();
	virtual void						printDependancy(int depth);

public:									// UseCount management
	int									getUseCount()							{ return _useCount._unsafeGet(); }
	void								incUseCount();
	void								decUseCount()							{ _useCount.dec(); }

protected:
	friend class StreamSource;
	virtual Ref<ContentBase>			linkContent(StreamSource* source)		{ return NULL; }

protected:
	AtomicInt							_useCount;

	Weak<StreamLocator>					_proxy;
	StreamLocatorList					_required;
	String								_name;

	virtual void 						onDelete();
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API StreamSource : public TCachableRefCounted<StreamSource, StreamLocator, MTRefCounted>
{
protected:
	StreamSource(StreamLocator* locator, const String& name);
	StreamSource(StreamLocator* locator, const String& name, const ContentType& ct);

public:
	StreamLocator*						getLocator()							{ return this ? getRealLocator()->getProxy() : NULL; }
	const String&						getLocatorName()						{ return this ? getRealLocator()->getName() : StringUtil::BLANK(); }

	const String&						getName()								{ return this ? _name : StringUtil::BLANK(); }
	String								getUrl()								{ return this ? _makeUrl() : StringUtil::BLANK(); }

	const ContentType&					getContentType()						{ return _contentType; }
	virtual ContentBase*				getContent()							{ return NULL; }

	virtual String						_makeUrl()								{ StreamLocator* l = getRealLocator(); return l ? l->makeUrl(_name) : _name; }

public:
	virtual size_t						getStreamSize() = 0;
	virtual size_t						getMemorySize() = 0;
	virtual Timestamp					getTimestamp() = 0;

	virtual StreamReader*				open() = 0;

	template <typename TContent> 
	Ref<TContent>						link(bool throwEx = true);

public:
	virtual uint32						calcCrc32();
	virtual uint32						calcAdler32();

public:
	static String						getExtension(const String& streamName, bool lowercase = true);
	static String						stripExtension(const String& streamName);

protected:
	String								_name;
	ContentType							_contentType;

	StreamLocator*						getRealLocator()						{ return GetCacheManager(); }
	void								setRealLocator(StreamLocator* locator)	{ SetCacheManager(locator); }
};

////////////////////////////////////////////////////////////////////////////////

template <typename TReader>
class TBinaryReader
{
public:
	TBinaryReader() : _reader(NULL)											{ }
	TBinaryReader(TReader* reader) : _reader(reader)							{ }

	TReader*							getReader()								{ return _reader; }
	void								setReader(TReader* reader)				{ _reader = reader; }

private:
	typedef TBinaryReader<TReader> ThisClass;
	Ref<TReader>						_reader;

public:
	inline bool							isEof()									{ return _reader->isEof(); }
	inline void							skip(int count)							{ return _reader->skip(count); }

	inline size_t						read(void* buf, size_t size)			{ return _reader->read(buf, size); }
	inline size_t						readRaw(void* buf, size_t size)			{ return _reader->readRaw(buf, size); }

	inline int8							readInt8()								{ int8 ret; _reader->read(&ret, sizeof(ret)); return ret; }
	inline int16						readInt16()								{ int16 ret; _reader->read(&ret, sizeof(ret)); return ret; }
	inline int32						readInt32()								{ int32 ret; _reader->read(&ret, sizeof(ret)); return ret; }
	inline int64						readInt64()								{ int64 ret; _reader->read(&ret, sizeof(ret)); return ret; }
	inline uint8						readUint8()								{ uint8 ret; _reader->read(&ret, sizeof(ret)); return ret; }
	inline uint16						readUint16()							{ uint16 ret; _reader->read(&ret, sizeof(ret)); return ret; }
	inline uint32						readUint32()							{ uint32 ret; _reader->read(&ret, sizeof(ret)); return ret; }
	inline uint64						readUint64()							{ uint64 ret; _reader->read(&ret, sizeof(ret)); return ret; }
	inline float						readFloat()								{ float ret; _reader->read(&ret, sizeof(ret)); return ret; }
	inline double						readDouble()							{ double ret; _reader->read(&ret, sizeof(ret)); return ret; }
	inline String						readString();
	inline String						readLine(bool trimAfter = true, char linefeed = '\n', char ignore = '\r');
	inline String						readDocument();
	// TODO: Vector2, Vector3, etc

public:
	inline ThisClass&					operator >> (int8& value)				{ value = readInt8(); return *this; }
	inline ThisClass&					operator >> (int16& value)				{ value = readInt16(); return *this; }
	inline ThisClass&					operator >> (int32& value)				{ value = readInt32(); return *this; }
	inline ThisClass&					operator >> (int64& value)				{ value = readInt64(); return *this; }
	inline ThisClass&					operator >> (uint8& value)				{ value = readUint8(); return *this; }
	inline ThisClass&					operator >> (uint16& value)				{ value = readUint16(); return *this; }
	inline ThisClass&					operator >> (uint32& value)				{ value = readUint32(); return *this; }
	inline ThisClass&					operator >> (uint64& value)				{ value = readUint64(); return *this; }
	inline ThisClass&					operator >> (float& value)				{ value = readFloat(); return *this; }
	inline ThisClass&					operator >> (double& value)				{ value = readDouble(); return *this; }
	inline ThisClass&					operator >> (String& value)				{ value = readString(); return *this; }
};

template <typename TReader>
String TBinaryReader<TReader>::readString()
{
	String ret;

	char ch;
	while ((ch = readInt8()) != 0)
		ret.push_back(ch);

	return ret;
}

template <typename TReader>
String TBinaryReader<TReader>::readLine(bool trimAfter, char linefeed, char ignore)
{
	// TODO: Avoid Skip() when possible()
	ASSERT_THROW(_reader->isSeekable(), EX_INVALID_STATE);

	// from OgreDataStream.cpp
	char tmpBuf[MAX_PATH];
	String retString;
	size_t readCount;

	// Keep looping while not hitting delimiter
	while ((readCount = _reader->readRaw(tmpBuf, MAX_PATH-1)) != 0)
	{
		// Terminate string
		tmpBuf[readCount] = '\0';

		char* p = strchr(tmpBuf, linefeed);
		if (p != 0)
		{
			// Reposition backwards
			skip((long)(p + 1 - tmpBuf - readCount));
			*p = '\0';
		}

		retString += tmpBuf;

		if (p != 0)
		{
			// Trim off trailing CR if this was a CR/LF entry
			if (retString.length() && retString[retString.length()-1] == ignore)
			{
				retString.erase(retString.length()-1, 1);
			}

			// Found terminator, break out
			break;
		}
	}

	if (trimAfter)
	{
		StringUtil::trim(retString);
	}

	return retString;
}

template <typename TReader>
String TBinaryReader<TReader>::readDocument()
{
	size_t len = _reader->GetSize() - _reader->GetPos();
	char* buf = new char[len + 1];
	size_t bytesRead = _reader->ReadRaw(buf, len);
	buf[bytesRead] = 0;
	String ret = buf;
	delete[] buf;

	return ret;
}

////////////////////////////////////////////////////////////////////////////////

template <typename TWriter>
class TBinaryWriter
{
public:
	TBinaryWriter() : _writer(NULL)											{ }
	TBinaryWriter(TWriter* writer) : _writer(writer)							{ }
	~TBinaryWriter()															{ if (_writer) flush(); }

	TWriter*							getWriter()								{ return _writer; }
	void								setWriter(TWriter* writer)				{ _writer = writer; }

private:
	typedef TBinaryWriter<TWriter> ThisClass;
	Ref<TWriter>						_writer;

public:
	inline size_t						write(const void* buf, size_t size)		{ return _writer->write(buf, size); }
	inline size_t						writeRaw(const void* buf, size_t size)	{ return _writer->writeRaw(buf, size); }

	inline void							writeInt8(int8 value)					{ _writer->write(&value, sizeof(value)); }
	inline void							writeInt16(int16 value)					{ _writer->write(&value, sizeof(value)); }
	inline void							writeInt32(int32 value)					{ _writer->write(&value, sizeof(value)); }
	inline void							writeInt64(int64 value)					{ _writer->write(&value, sizeof(value)); }
	inline void							writeUint8(uint8 value)					{ _writer->write(&value, sizeof(value)); }
	inline void							writeUint16(uint16 value)				{ _writer->write(&value, sizeof(value)); }
	inline void							writeUint32(uint32 value)				{ _writer->write(&value, sizeof(value)); }
	inline void							writeUint64(uint64 value)				{ _writer->write(&value, sizeof(value)); }
	inline void							writeFloat(float value)					{ _writer->write(&value, sizeof(value)); }
	inline void							writeDouble(double value)				{ _writer->write(&value, sizeof(value)); }
	inline void							writeString(const String& value)		{ writeString(value.c_str(), value.length()); }
	inline void							writeString(const char* str, int len = -1);

	inline bool							flush()									{ return _writer->flush(); }

public:
	inline ThisClass&					operator >> (int8 value)				{ writeInt8(value); return *this; }
	inline ThisClass&					operator >> (int16 value)				{ writeInt16(value); return *this; }
	inline ThisClass&					operator >> (int32 value)				{ writeInt32(value); return *this; }
	inline ThisClass&					operator >> (int64 value)				{ writeInt64(value); return *this; }
	inline ThisClass&					operator >> (uint8 value)				{ writeUint8(value); return *this; }
	inline ThisClass&					operator >> (uint16 value)				{ writeUint16(value); return *this; }
	inline ThisClass&					operator >> (uint32 value)				{ writeUint32(value); return *this; }
	inline ThisClass&					operator >> (uint64 value)				{ writeUint64(value); return *this; }
	inline ThisClass&					operator >> (float value)				{ writeFloat(value); return *this; }
	inline ThisClass&					operator >> (double value)				{ writeDouble(value); return *this; }
	inline ThisClass&					operator >> (const String& value)		{ writeString(value); return *this; }
	inline ThisClass&					operator >> (const char* str)			{ writeString(str, -1); return *this; }
};

template <typename TWriter>
void TBinaryWriter<TWriter>::writeString(const char* str, int len)
{
	if (len < 0)
		len = strlen(str);

	_writer->writeRaw(str, len);
	writeInt8(0);
}

////////////////////////////////////////////////////////////////////////////////

class NIT_API StreamReader : public RefCounted, public PooledAlloc
{
public:
	virtual StreamSource*				getSource() = 0;
	const String&						getName()								{ return getSource()->getName(); }
	const String&						getLocatorName()						{ return getSource()->getLocator()->getName(); }
	String								getUrl()								{ return getSource()->getUrl(); }

public:
	virtual bool						isBuffered() = 0;
	virtual bool						isSized() = 0;
	virtual bool						isSeekable() = 0;

	virtual MemoryBuffer*				buffer();

public:
	virtual bool						isEof() = 0;
	virtual size_t						getSize() = 0;
	virtual void						skip(int count) = 0;
	virtual void						seek(size_t pos) = 0;
	virtual size_t						tell() = 0;

	virtual size_t						readRaw(void* buf, size_t size) = 0;

public:
	virtual size_t						read(void* buf, size_t size)			{ if (readRaw(buf, size) != size) NIT_THROW(EX_READ); return size; }
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API StreamWriter : public RefCounted, public PooledAlloc
{
public:
	virtual StreamSource*				getSource() = 0;
	const String&						getName()								{ return getSource()->getName(); }
	const String&						getLocatorName()						{ return getSource()->getLocator()->getName(); }
	String								getUrl()								{ return getSource()->getUrl(); }

public:
	virtual bool						isBuffered() = 0;
	virtual bool						isSized() = 0;
	virtual bool						isSeekable() = 0;

public:
	virtual size_t						getSize() = 0;
	virtual void						skip(int count) = 0;
	virtual void						seek(size_t pos) = 0;
	virtual size_t						tell() = 0;

	virtual size_t						writeRaw(const void* buf, size_t size) = 0;

	virtual size_t						copy(StreamReader* reader, size_t offset = 0, size_t len = 0, size_t bufSize = 0);

public:
	virtual size_t						write(const void* buf, size_t size)		{ if (writeRaw(buf, size) != size) NIT_THROW(EX_WRITE); return size; }
	virtual size_t						print(const char* str, size_t size = 0);

	virtual bool						flush() = 0;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API StdIOWriter : public StreamWriter
{
public:
	static StdIOWriter*					createStdOut()							{ return new StdIOWriter(stdout); }
	static StdIOWriter*					createStdErr()							{ return new StdIOWriter(stderr); }

public:
	virtual StreamSource*				getSource()								{ return NULL; }
	virtual bool						isBuffered()							{ return false; }
	virtual bool						isSized()								{ return false; }
	virtual bool						isSeekable()							{ return false; }
	virtual size_t						getSize()								{ NIT_THROW(EX_NOT_SUPPORTED); }
	virtual void						skip(int count)							{ NIT_THROW(EX_NOT_SUPPORTED); }
	virtual void						seek(size_t pos)						{ NIT_THROW(EX_NOT_SUPPORTED); }
	virtual size_t						tell()									{ NIT_THROW(EX_NOT_SUPPORTED); }

	virtual size_t						writeRaw(const void* buf, size_t size);
	virtual size_t						print(const char* str, size_t size = 0);
	virtual bool						flush();

protected:
	FILE*								_handle;

	StdIOWriter(FILE* file);
	virtual void						onDelete();
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API TempMemoryReader : public StreamReader
{
public:
	TempMemoryReader(const void* memory, size_t size) : _memory((const uint8*)memory), _size(size), _pos(0) { }
	virtual ~TempMemoryReader()													{ _memory = NULL; }

public:									// StreamReader impl
	virtual StreamSource*				getSource()								{ return NULL; }
	virtual bool						isBuffered()							{ return true; }
	virtual bool						isSized()								{ return true; }
	virtual bool						isSeekable()							{ return true; }

	virtual bool						isEof()									{ return _pos >= _size; }
	virtual size_t						getSize()								{ return _size; }
	virtual void						skip(int count)							{ _pos += count; }
	virtual void						seek(size_t pos)						{ _pos = pos; }
	virtual size_t						tell()									{ return _pos; }

	virtual size_t						readRaw(void* buf, size_t size);

public:
	void								close()									{ _memory = NULL; }

protected:
	virtual void						onDelete()								{ close(); }

private:
	const uint8*						_memory;
	size_t								_size;
	size_t								_pos;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API TempMemoryWriter : public StreamWriter
{
public:
	TempMemoryWriter(void* memory, size_t size) : _memory((uint8*)memory), _size(size), _pos(0) { }
	virtual ~TempMemoryWriter()													{ _memory = NULL; }

public:
	virtual StreamSource*				getSource()								{ return NULL; }
	virtual bool						isBuffered()							{ return true; }
	virtual bool						isSized()								{ return true; }
	virtual bool						isSeekable()							{ return true; }

	virtual size_t						getSize()								{ return _size; }
	virtual void						skip(int count)							{ _pos += count; }
	virtual void						seek(size_t pos)						{ _pos = pos; }
	virtual size_t						tell()									{ return _pos; }

	virtual size_t						writeRaw(const void* buf, size_t size);
	virtual bool						flush()									{ return true; }

public:
	void								close()									{ _memory = NULL; }

protected:
	virtual void						onDelete()								{ close(); }

private:
	uint8*								_memory;
	size_t								_size;
	size_t								_pos;
};

////////////////////////////////////////////////////////////////////////////////

// TODO: Refactor to the new EncodeUtil.h

class NIT_API Base64Encoder : public StreamWriter
{
public:
	Base64Encoder(StreamWriter* to, size_t bufSize = 0);

public:
	StreamWriter*						getTo()									{ return _to; }

public:									// StreamWriter Impl
	virtual StreamSource*				getSource()								{ return _to->getSource(); }
	virtual	bool						isBuffered()							{ return true; }
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
	uint8*								_buffer;
	size_t								_bufPos;
	size_t								_bufSize;

	int									_currByteShift;
	uint8								_prevByte;

	void								flushBuffer();
	void								finish();
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API Base64Decoder : public StreamReader
{
public:
	Base64Decoder(StreamReader* from, size_t bufSize = 0);

public:
	StreamReader*						getFrom()								{ return _from; }

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
	uint8*								_buffer;
	size_t								_bufPos;
	size_t								_bufAvail;
	size_t								_bufSize;

	int									_prevDecodedShift;
	uint8								_prevDecoded;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API CalcCRC32Writer : public StreamWriter
{
public:
	CalcCRC32Writer();

public:
	uint32								getValue()								{ return _value; }

public:									// StreamWriter Impl
	virtual StreamSource*				getSource()								{ return NULL; }
	virtual bool						isBuffered()							{ return false; }
	virtual bool						isSized()								{ return false; }
	virtual bool						isSeekable()							{ return false; }
	virtual size_t						getSize()								{ NIT_THROW(EX_NOT_SUPPORTED); }
	virtual void						skip(int count)							{ NIT_THROW(EX_NOT_SUPPORTED); }
	virtual void						seek(size_t pos)						{ NIT_THROW(EX_NOT_SUPPORTED); }
	virtual size_t						tell()									{ NIT_THROW(EX_NOT_SUPPORTED); }
	virtual bool						flush()									{ return true; }

	virtual size_t						writeRaw(const void* buf, size_t size);

private:
	uint32								_value;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API CopyReader : public StreamReader
{
public:
	CopyReader(StreamReader* from, StreamWriter* to) : _from(from), _to(to)	{ }

public:
	virtual StreamSource*				getSource()								{ return _from->getSource(); }

	StreamReader*						getFrom()								{ return _from; }
	StreamWriter*						getTo()									{ return _to; }

public:
	virtual bool						isBuffered()							{ return _from->isBuffered(); }
	virtual bool						isSized()								{ return _from->isSized(); }
	virtual bool						isSeekable()							{ return _from->isSeekable() && _to->isSeekable(); }

public:
	virtual bool						isEof()									{ return _from->isEof(); }
	virtual size_t						getSize()								{ return _from->getSize(); }
	virtual void						skip(int count)							{ _from->skip(count); _to->skip(count); }
	virtual void						seek(size_t pos)						{ _from->seek(pos); _to->seek(pos); }
	virtual size_t						tell()									{ return _from->tell(); }

	virtual size_t						readRaw(void* buf, size_t size);

private:
	Ref<StreamReader>					_from;
	Ref<StreamWriter>					_to;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API ShadowWriter : public StreamWriter
{
public:
	ShadowWriter(StreamWriter* target, StreamWriter* shadow) : _target(target), _shadow(shadow) { }

public:
	virtual StreamSource*				getSource()								{ return _target->getSource(); }

	StreamWriter*						getTarget()								{ return _target; }
	StreamWriter*						getShadow()								{ return _shadow; }

public:
	virtual bool						isBuffered()							{ return _target->isBuffered(); }
	virtual bool						isSized()								{ return _target->isSized(); }
	virtual bool						isSeekable()							{ return _target->isSeekable() && _shadow->isSeekable(); }

public:
	virtual size_t						getSize()								{ return _target->getSize(); }
	virtual void						skip(int count)							{ _target->skip(count); _shadow->skip(count); }
	virtual void						seek(size_t pos)						{ _target->seek(pos); _shadow->seek(pos); }
	virtual size_t						tell()									{ return _target->tell(); }

	virtual size_t						writeRaw(const void* buf, size_t size)	{ _shadow->writeRaw(buf, size); return _target->writeRaw(buf, size); }

public:
	virtual bool						flush()									{ _shadow->flush(); return _target->flush(); }

private:
	Ref<StreamWriter>					_target;
	Ref<StreamWriter>					_shadow;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API LogWriter : public StreamWriter
{
public:
	LogWriter(const char* tag = NULL, LogChannel* channel = NULL);

public:
	virtual StreamSource*				getSource()								{ return NULL; }

public:
	virtual bool						isBuffered()							{ return true; }
	virtual bool						isSized()								{ return false; }
	virtual bool						isSeekable()							{ return false; }

public:
	virtual size_t						getSize()								{ return 0; }
	virtual void						skip(int count)							{ }
	virtual void						seek(size_t pos)						{ }
	virtual size_t						tell()									{ return 0; }

	virtual size_t						writeRaw(const void* buf, size_t size);

public:
	virtual bool						flush();

	virtual void						onDelete();

private:
	String								_lineBuf;
	String								_tag;
	Ref<LogChannel>						_channel;

	int									_column;
	int									_maxColumn;

	void								lineFeed();
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API HexDumpWriter : public StreamWriter
{
public:
	HexDumpWriter(size_t column = 16, StreamWriter* target = NULL);
	virtual ~HexDumpWriter();

public:
	virtual StreamSource*				getSource()								{ return NULL; }

public:
	virtual bool						isBuffered()							{ return false; }
	virtual bool						isSized()								{ return false; }
	virtual bool						isSeekable()							{ return true; }

public:
	virtual size_t						getSize()								{ return 0; }
	virtual void						skip(int count);
	virtual void						seek(size_t pos);
	virtual size_t						tell()									{ return _pos; }

	virtual size_t						writeRaw(const void* buf, size_t size);

public:
	virtual bool						flush();

	virtual void						onDelete();

private:
	Ref<StreamWriter>					_target;
	byte*								_buffer;
	size_t								_column;
	size_t								_count;
	size_t								_pos;
	size_t								_begin;

	virtual void						line(const String& msg=StringUtil::BLANK());
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API StreamPrinter : public RefCounted
{
public:
	StreamPrinter(StreamWriter* w = NULL); // default to new MemoryBuffer::Writer()
	virtual ~StreamPrinter();

public:
	StreamWriter*						getWriter()								{ return _writer; }

	bool								isCompact()								{ return _compact; }
	const String&						getIndentStr()							{ return _indentStr; }
	const String&						getNewlineStr()							{ return _newlineStr; }

	virtual void						setCompact(bool flag);
	void								setIndentStr(const String& str)			{ _indentStr = str; }
	void								setNewlineStr(const String& str)		{ _newlineStr = str; }

public:

	int									getIndent()								{ return _indent; }
	void								incIndent()								{ ++_indent; }
	void								decIndent()								{ --_indent; }

	bool								newline();
	bool								doIndent();

public:
	void								putch(const int ch);
	void								putUnichar(const int ch);
	void 								print(const String& str)				{ print(str.c_str(), str.length()); }
	void 								print(const char* str, size_t len=0);

public:
	void								printf(const char* fmt, ...);
	void								vprintf(const char* fmt, va_list args);

	class IArgIterator
	{
	public:
		virtual bool					hasNext() = 0;
		virtual int						nextInt() = 0;
		virtual int64					nextInt64() = 0;
		virtual float					nextFloat() = 0;
		virtual double					nextDouble() = 0;
		virtual const char*				nextCStr() = 0;
	};

	void								aprintf(const char* fmt, IArgIterator* aitr);

public:
	bool								flush();

protected:
	Ref<StreamWriter>					_writer;

	vector<int>::type					_stateStack;
	int									topState()								{ return _stateStack.empty() ? 0 : _stateStack.back(); }
	void								pushState(int state)					{ _stateStack.push_back(state); }
	void								popState()								{ ASSERT_THROW(!_stateStack.empty(), EX_INVALID_STATE); _stateStack.pop_back(); }

	virtual void						printEscaped(const char* str, size_t len = 0);
	virtual void						printQuoted(const char* str, size_t len = 0);
	inline void							flushLineBuf()							{ if (!_lineBuf.empty()) doFlushLineBuf(); }

protected:
	bool								_compact;

	int									_indent;
	String								_indentStr;
	String								_newlineStr;
	String								_lineBuf;

	void								doFlushLineBuf();
	virtual void						onDelete();
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API JsonPrinter : public StreamPrinter
{
public:
	JsonPrinter(StreamWriter* w = NULL); // default to new MemoryBuffer::Writer()
	virtual ~JsonPrinter();

public:									// overrides
	virtual void						setCompact(bool flag);

public:
	void 								beginObject();
	void 								endObject();

	void 								beginArray();
	void 								endArray();

	void								key(const String& str)					{ key(str.c_str(), str.length()); }
	void								key(const char* str, size_t len = 0);

	void								valueNull()								{ comma(); print("null"); needComma(); }
	void								value(bool value)						{ comma(); print(value ? "true" : "false"); needComma(); }
	void								value(int value)						{ comma(); printf("%d", value); needComma(); }
	void								value(uint value)						{ comma(); printf("%u", value); needComma(); }
	void								value(int64 value)						{ comma(); printf("%lld", value); needComma(); }
	void								value(float value)						{ comma(); printf("%f", value); needComma(); }
	void								value(double value)						{ comma(); printf("%lf", value); needComma(); }
	void								value(const String& str)				{ comma(); value(str.c_str(), str.length()); needComma(); }
	void								value(const char* str, size_t len = 0)	{ comma(); printQuoted(str, len); needComma(); }

protected:
	bool								_needComma;

	String								_commaStr;
	String								_colonFmt;

	enum State
	{
		STATE_NONE		= 0,
		STATE_OBJECT,
		STATE_ARRAY,
	};

protected:
	void								open(const char* brace);
	void								close(const char* brace);
	void								end();
	void								comma()									{ if (_needComma) { print(_commaStr); newline(); } }
	inline void							needComma()								{ _needComma = true; }

protected:
	virtual void						printEscaped(const char* str, size_t len);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API XmlPrinter : public StreamPrinter
{
public:
	XmlPrinter(StreamWriter* w = NULL); // default to new MemoryBuffer::Writer()

public:
	void								beginElem(const char* elemName);
	void								endElem();

	void								attr(const char* name, const char* value);
	
	void								text(const String& str)					{ text(str.c_str(), str.length()); }
	void								text(const char* text, size_t len = 0);

	void								comment(const char* fmt, ...);
	void								commentA(const char* fmt, IArgIterator* aitr);

protected:
	virtual void						onDelete();

private:
	enum State
	{
		STATE_NONE			= 0,
		STATE_DOCUMENT,
		STATE_ELEM_TAG_OPEN,
		STATE_ELEM,
	};

	struct ElemState
	{
		String		name;
		bool		hasText;
		bool		hasAttrs;
		bool		hasChildElem;
		bool		indented;
	};

	vector<ElemState>::type				_elemStack;
	ElemState*							_currElem;

	ElemState*							pushElemState(const String& name);
	void								popElemState();

	void								closeElemTag(bool noChild = false);

	StringVector						_attrKeys;
	StringVector						_attrValues;
};

////////////////////////////////////////////////////////////////////////////////

template <typename TContent>
Ref<TContent> StreamSource::link(bool throwEx)
{
	Ref<ContentBase> contentBase = getContent();
	
	if (contentBase == NULL)
	{
		StreamLocator* locator = getLocator();
		if (locator) contentBase = locator->linkContent(this);
	}

	if (contentBase == NULL && throwEx)
		NIT_THROW_FMT(EX_NOT_FOUND, "can't link '%s' to any content", getUrl().c_str());

	TContent* content = dynamic_cast<TContent*>(contentBase.get());

	if (content == NULL && throwEx)
		NIT_THROW_FMT(EX_CASTING, "can't cast '%s' to %s", getUrl().c_str(), typeid(TContent).name());

	return content;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
