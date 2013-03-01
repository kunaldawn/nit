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

#include "Stream.h"

#include "nit/runtime/MemManager.h"
#include "nit/io/MemoryBuffer.h"
#include "nit/data/ParserUtil.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

void StreamUtil::flipEndian(void * pData, size_t size)
{
	char swapByte;
	for(unsigned int byteIndex = 0; byteIndex < size/2; byteIndex++)
	{
		swapByte = *(char *)((size_t)pData + byteIndex);
		*(char *)((size_t)pData + byteIndex) = *(char *)((size_t)pData + size - byteIndex - 1);
		*(char *)((size_t)pData + size - byteIndex - 1) = swapByte;
	}
}

////////////////////////////////////////////////////////////////////////////////

StreamSource::StreamSource(StreamLocator* locator, const String& name)
: TCachableRefCounted<StreamSource, StreamLocator, RefCounted>(locator), _name(name)
{
	_contentType = ContentType::fromStreamName(name);
}

StreamSource::StreamSource(StreamLocator* locator, const String& name, const ContentType& ct)
: TCachableRefCounted<StreamSource, StreamLocator, RefCounted>(locator), _name(name), _contentType(ct)
{
}

String StreamSource::getExtension(const String& streamName, bool lowercase /*= true*/)
{
	String ext;

	String::size_type pos = streamName.rfind('.');
	if (pos != String::npos && pos < (streamName.length() - 1))
	{
		ext = streamName.substr(pos + 1);
		if (lowercase)
			StringUtil::toLowerCase(ext);
	}

	return ext;
}

String StreamSource::stripExtension(const String& streamName)
{
	String::size_type pos = streamName.rfind('.');
	if (pos != String::npos && pos < (streamName.length() - 1))
	{
		return streamName.substr(0, pos);
	}

	return streamName;
}

////////////////////////////////////////////////////////////////////////////////

StreamLocator::StreamLocator(const String& name)
: _useCount(0), _name(name)
{
}

StreamLocator::~StreamLocator()
{
}

void StreamLocator::onDelete()
{
	if (_useCount._unsafeGet())
	{
		LOG(0, "*** StreamLocator '%s' : deleted without use count cleared: %d\n", _name.c_str(), _useCount._unsafeGet());
	}

	unrequireAll();
}


void StreamLocator::incUseCount()
{
	// Debugger breakpoint purpose - useCount is hard to debug so set a breakpoint here and pray!
	_useCount.inc();
}

StreamSource* StreamLocator::locate(const String& streamName, bool throwEx)
{
	StreamSource* source = locateLocal(streamName);

	if (source) return source;

	for (StreamLocatorList::reverse_iterator itr = _required.rbegin(); itr != _required.rend(); ++itr)
	{
		StreamLocator* req = itr->get();
		source = req->locate(streamName, false);
		if (source) return source;
	}

	if (throwEx)
	{
		NIT_THROW_FMT(EX_NOT_FOUND, "Can't find '%s' from '%s' and required locators", streamName.c_str(), _name.c_str());
	}

	return NULL;
}

StreamSource* StreamLocator::locate(const String& streamName, const String& locatorPattern, bool throwEx /*= true*/)
{
	if (Wildcard::match(locatorPattern, _name))
	{
		StreamSource* source = locateLocal(streamName);

		if (source) return source;
	}

	for (StreamLocatorList::reverse_iterator itr = _required.rbegin(); itr != _required.rend(); ++itr)
	{
		StreamLocator* req = itr->get();
		StreamSource* source = req->locate(streamName, locatorPattern, false);
		if (source) return source;
	}

	if (throwEx)
	{
		NIT_THROW_FMT(EX_NOT_FOUND, "Can't find '%s' from any '%s'", streamName.c_str(), locatorPattern.c_str());
	}

	return NULL;
}

void StreamLocator::find(const String& pattern, StreamSourceMap& varResults)
{
	findLocal(pattern, varResults);

	for (StreamLocatorList::reverse_iterator itr = _required.rbegin(); itr != _required.rend(); ++itr)
	{
		StreamLocator* req = itr->get();
		req->find(pattern, varResults);
	}
}

void StreamLocator::find(const String& streamPattern, const String& locatorPattern, StreamSourceMap& varResults)
{
	if (Wildcard::match(locatorPattern, _name))
	{
		findLocal(streamPattern, varResults);
	}

	for (StreamLocatorList::reverse_iterator itr = _required.rbegin(); itr != _required.rend(); ++itr)
	{
		StreamLocator* req = itr->get();
		req->find(streamPattern, locatorPattern, varResults);
	}
}

StreamLocator* StreamLocator::getLocator(const String& locatorName, bool throwEx)
{
	if (_name == locatorName)
		return this;

	for (StreamLocatorList::reverse_iterator itr = _required.rbegin(); itr != _required.rend(); ++itr)
	{
		StreamLocator* loc = itr->get()->getLocator(locatorName, false);
		if (loc) return loc;
	}

	if (throwEx)
	{
		NIT_THROW_FMT(EX_NOT_FOUND, "Can't find locator '%s' from '%s'", locatorName.c_str(), _name.c_str());
	}

	return NULL;
}

void StreamLocator::findLocator(const String& locatorPattern, StreamLocatorList& varResults)
{
	if (Wildcard::match(locatorPattern, _name))
		varResults.push_back(this);

	for (StreamLocatorList::reverse_iterator itr = _required.rbegin(); itr != _required.rend(); ++itr)
	{
		StreamLocator* req = itr->get();
		req->findLocator(locatorPattern, varResults);
	}
}

bool StreamLocator::isRequired(StreamLocator* req)
{
	for (uint i=0; i<_required.size(); ++i)
	{
		if (_required[i] == req) return true;
		if (_required[i]->isRequired(req)) return true;
	}

	return false;
}

void StreamLocator::require(StreamLocator* req, bool first /*= false*/)
{
	if (isRequired(req)) return;

	req->incUseCount();

	if (first)
		_required.insert(_required.begin(), req);
	else
		_required.push_back(req);
}

void StreamLocator::unrequire(StreamLocator* req)
{
	StreamLocatorList::iterator begin = std::remove(_required.begin(), _required.end(), req);

	for (StreamLocatorList::iterator itr = begin; itr != _required.end(); ++itr)
	{
		(*itr)->decUseCount();
	}

	_required.erase(begin, _required.end());
}

void StreamLocator::unrequireAll()
{
	for (uint i=0; i<_required.size(); ++i)
	{
		_required[i]->decUseCount();
	}

	_required.clear();
}

void StreamLocator::printDependancy(int depth)
{
	for (uint i=0; i<_required.size(); ++i)
	{
		for (int d=0; d<depth; ++d)
		{
			LOG(0, "  ");
		}

		LOG(0, "- %s\n", _required[i]->getName().c_str());
		_required[i]->printDependancy(depth+1);
	}
}

String StreamLocator::makeUrl(const String& sourceName)
{
	String url = getName() + ": " + sourceName;
	if (!isCaseSensitive())
		StringUtil::toLowerCase(url);
	return url;
}

////////////////////////////////////////////////////////////////////////////////

size_t StreamWriter::copy(StreamReader* reader, size_t offset, size_t len, size_t bufSize)
{
	Ref<StreamReader> safe = reader;

	if (reader->isBuffered())
	{
		Ref<MemoryBuffer> buf = reader->buffer();
		size_t srcPos = reader->tell() + offset;
		len = len ? len : buf->getSize() - srcPos;
		size_t written = buf->save(this, srcPos, len);
		reader->skip(offset + written);
		return written;
	}

	if (bufSize == 0)
		bufSize = 4096; // TODO: remove magic numbers

	void* buf = NIT_ALLOC(bufSize);

	try
	{
		size_t totalRead = 0;

		if (reader->isSeekable() && offset)
		{
			reader->skip(offset);
		}
		else
		{
			size_t totalSkip = 0;
			while (totalSkip < offset)
			{
				size_t toSkip = std::min(offset - totalSkip, bufSize);
				size_t read = reader->readRaw(buf, toSkip);
				totalSkip += read;
				if (read == 0) break;
			}
		}

		while (len == 0 || totalRead < len)
		{
			size_t toRead = len ? std::min(len - totalRead, bufSize) : bufSize;
			size_t read = reader->readRaw(buf, toRead);
			writeRaw(buf, read);
			totalRead += read;
			if (read == 0) break;
		}

		NIT_DEALLOC(buf, bufSize);

		return totalRead;
	}
	catch (...)
	{
		NIT_DEALLOC(buf, bufSize);
		throw;
	}
}

size_t StreamWriter::print(const char* str, size_t size)
{
	if (size == 0) size = strlen(str);

	if (writeRaw(str, size) != size) NIT_THROW(EX_WRITE); return size;
}

////////////////////////////////////////////////////////////////////////////////

StdIOWriter::StdIOWriter(FILE* file)
: _handle(file)
{
}

size_t StdIOWriter::writeRaw(const void* buf, size_t size)
{
	return fwrite(buf, size, 1, _handle);
}

size_t StdIOWriter::print(const char* str, size_t size)
{
	if (size == 0) size = strlen(str);

#if defined(NIT_WIN32)
	return fputws(Unicode::toUtf16(str, size).c_str(), _handle);
#else
	return fwrite(str, size, 1, _handle);
#endif
}

bool StdIOWriter::flush()
{
	return fflush(_handle) != EOF;
}

void StdIOWriter::onDelete()
{
	if (_handle)
	{
		fclose(_handle);
		_handle = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////

size_t TempMemoryReader::readRaw(void* buf, size_t size)
{
	if (!_memory) NIT_THROW(EX_ACCESS); 

	if (_pos + size > _size)
		size = _size - _pos;

	memcpy(buf, _memory + _pos, size); 
	_pos += size; 
	
	return size;
}

////////////////////////////////////////////////////////////////////////////////

size_t TempMemoryWriter::writeRaw(const void* buf, size_t size)
{
	if (!_memory) NIT_THROW(EX_ACCESS); 

	if (_pos + size > _size)
		size = _size - _pos;

	memcpy(_memory + _pos, buf, size); 
	_pos += size; 

	return size;
}

////////////////////////////////////////////////////////////////////////////////

size_t CopyReader::readRaw(void* buf, size_t size)
{
	// NOTE: we will use the buf which caller give us
	size_t read = _from->readRaw(buf, size);

	if (read)
	{
		size_t written = _to->writeRaw(buf, read);

		ASSERT_THROW(read == written, EX_WRITE);
	}

	return read;
}

////////////////////////////////////////////////////////////////////////////////

LogWriter::LogWriter(const char* tag, LogChannel* channel)
{
	_tag = tag ? tag : "";
	_channel = channel;

	_column = 0;
	_maxColumn = 80;
}

size_t LogWriter::writeRaw(const void* buf, size_t size)
{
	const char* str = (const char*) buf;

	_lineBuf.reserve(_lineBuf.size() + size);
	for (size_t i=0; i<size; ++i)
	{
		if (str[i] == '\n')
		{
			lineFeed();
			continue;
		}

		if (_maxColumn && _column >= _maxColumn)
		{
			lineFeed();
			_lineBuf.append(">> ");
		}

		_lineBuf.push_back(str[i]);
		++_column;
	}

	return size;
}

void LogWriter::lineFeed()
{
	LogManager& log = LogManager::getSingleton();

	LogEntry e;
	e.time			= float(SystemTimer::now());
	e.channel		= _channel ? _channel.get() : log.needThreadRootChannel();
	e.srcName		= NULL;
	e.line			= 0;
	e.fnName		= NULL;
	e.act			= NULL;
	e.tagStr		= _tag.c_str();
	e.tagID			= log.tagId(e.tagStr);
	e.logLevel		= log.getLogLevel(e.tagID);
	e.message		= _lineBuf.c_str();
	e.messageLen	= _lineBuf.length();
	e.lineEnd		= true;

	log.doLog(&e);

	_lineBuf.clear();
	_column = 0;
}

bool LogWriter::flush()
{
	if (!_lineBuf.empty()) 
		lineFeed();

	return true;
}

void LogWriter::onDelete()
{
	flush();

	StreamWriter::onDelete();
}

////////////////////////////////////////////////////////////////////////////////

HexDumpWriter::HexDumpWriter(size_t column, StreamWriter* target)
: _buffer(NULL)
{
	if (target == NULL)
		target = new LogWriter();

	_target = target;
	_column = column;
	_count = 0;

	_buffer = (byte*)NIT_ALLOC(_column);
	_pos = 0;
	_begin = 0;
}

HexDumpWriter::~HexDumpWriter()
{
	if (_buffer)
		NIT_DEALLOC(_buffer, _column);
}

void HexDumpWriter::skip(int count)
{
	line(StringUtil::format("<skip(%d)>\n", count));
	_pos += count;
	_begin = _pos;
}

void HexDumpWriter::seek(size_t pos)
{
	line(StringUtil::format("<seek(%08X)>\n", pos));
	_pos = pos;
	_begin = pos;
}

size_t HexDumpWriter::writeRaw(const void* buf, size_t size)
{
	byte* bytes = (byte*)buf;

	for (size_t i=0; i<size; ++i)
	{
		_buffer[_count++] = *bytes++;
		_pos++;

		if (_count == _column)
			line();
	}

	return size;
}

bool HexDumpWriter::flush()
{
	line("<flush>\n");
	return true;
}

void HexDumpWriter::line(const String& msg)
{
	if (_count > 0)
	{
		String hex;
		String ascii;

		const char* digits = "0123456789ABCDEF";

		for (uint i=0; i<_column; ++i)
		{
			if (i < _count)
			{
				byte b = _buffer[i];
				if (b >= 32 && b <= 127)
					ascii.push_back(b);
				else
					ascii.push_back('.');

				hex.push_back(digits[b >> 4]);
				hex.push_back(digits[b & 0xF]);
				hex.push_back(' ');
			}
			else
			{
				hex.append("-- ");
			}
		}

		for (int i=7; i>=0; --i)
		{
			int ad = (_begin >> (i << 2)) & 0xF;
			_target->writeRaw(&digits[ad], 1);
		}

		_target->writeRaw(": ", 2);
		_target->writeRaw(hex.c_str(), hex.size());
		_target->writeRaw(" ", 1);
		_target->writeRaw(ascii.c_str(), ascii.size());
		_target->writeRaw("\n", 1);

		_begin = _pos;
		_count = 0;
	}

	if (!msg.empty())
	{
		_target->writeRaw(msg.c_str(), msg.length());
	}
}

void HexDumpWriter::onDelete()
{
	line();

	StreamWriter::onDelete();
}

////////////////////////////////////////////////////////////////////////////////

StreamPrinter::StreamPrinter(StreamWriter* w)
{
	if (w == NULL)
		w = new MemoryBuffer::Writer();

	_writer		= w;
	_compact		= true;
	_indent		= 0;
	_indentStr		= "  ";
	_newlineStr	= "\n";
}

StreamPrinter::~StreamPrinter()
{
}

void StreamPrinter::setCompact(bool flag)
{
	_compact = flag;
}

bool StreamPrinter::newline()
{
	if (_compact) return false;

	if (_newlineStr.empty())
		return false;

	print(_newlineStr);
	doIndent();
	return true;
}

bool StreamPrinter::doIndent()
{
	if (_compact) return false;

	for (int i=0; i<_indent; ++i)
	{
		print(_indentStr);
	}
	return true;
}

void StreamPrinter::printf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

void StreamPrinter::vprintf(const char* fmt, va_list args)
{
	// TODO: unify with APrintf
	char numbuf[128];

	for (const char* ch = fmt; *ch; ++ch)
	{
		if (*ch != '%')
		{
			putch(*ch);
			continue;
		}

		++ch;
		switch (*ch)
		{
		case '%':						putch(*ch); break;
		case 's':						print(va_arg(args, char*)); break;
		case 'q':						printQuoted(va_arg(args, char*)); break;
		case 'z':						printEscaped(va_arg(args, char*)); break;

		case 'u':						sprintf(numbuf, "%u", va_arg(args, uint)); print(numbuf); break;
		case 'd':						sprintf(numbuf, "%d", va_arg(args, int)); print(numbuf); break;
		case 'f':						sprintf(numbuf, "%f", va_arg(args, double)); print(numbuf); break;

		case 'l':
			++ch;
			if (*ch == 'f')				{ sprintf(numbuf, "%lf", va_arg(args, double)); print(numbuf); }
			else if (*ch == 'l')
			{
				++ch;
				if (*ch == 'd')			{ sprintf(numbuf, "%lld", va_arg(args, int64)); print(numbuf); }
				else					{ print("%ll"); putch(*ch); }
			}
			else						{ print("%l"); putch(*ch); }
			break;
		}
	}

	flushLineBuf();
}

void StreamPrinter::aprintf(const char* fmt, IArgIterator* aitr)
{
	// TODO: number format support (see sqstring_format)
	char numbuf[128];

	for (const char* ch = fmt; *ch; ++ch)
	{
		if (*ch != '%')
		{
			putch(*ch);
			continue;
		}

		++ch;

		if (*ch == '%')
		{
			putch(*ch);
			continue;
		}

		ASSERT_THROW(aitr->hasNext(), EX_INVALID_PARAMS);

		switch (*ch)
		{
		case 's':						print(aitr->nextCStr()); break;
		case 'q':						printQuoted(aitr->nextCStr()); break;
		case 'z':						printEscaped(aitr->nextCStr()); break;

		case 'u':						sprintf(numbuf, "%u", aitr->nextInt()); print(numbuf); break;
		case 'd':						sprintf(numbuf, "%d", aitr->nextInt()); print(numbuf); break;
		case 'f':						sprintf(numbuf, "%f", aitr->nextDouble()); print(numbuf); break;

		case 'l':
			++ch;
			if (*ch == 'f')				{ sprintf(numbuf, "%lf", aitr->nextDouble()); print(numbuf); }
			else if (*ch == 'l')
			{
				++ch;
				if (*ch == 'd')			{ sprintf(numbuf, "%lld", aitr->nextInt64()); print(numbuf); }
				else					{ print("%ll"); putch(*ch); }
			}
			else						{ print("%l"); putch(*ch); }
			break;
		}
	}

	flushLineBuf();
}

void StreamPrinter::print(const char* str, size_t len)
{
	if (len == 0) 
		len = strlen(str);

	flushLineBuf();
	_writer->print(str, len);
}

void StreamPrinter::printEscaped(const char* str, size_t len)
{
	print(str);
}

void StreamPrinter::printQuoted(const char* str, size_t len)
{
	putch('\"'); printEscaped(str, len); putch('\"');
}

void StreamPrinter::putch(const int ch)
{
	bool useUnichar = false;

	if (ch < 128) 
		_lineBuf.push_back(ch);
	else if (useUnichar)
		putUnichar(ch);
	else
		_lineBuf.push_back(ch);
}

void StreamPrinter::putUnichar(const int ch)
{
	if (ch < 128)
	{
		_lineBuf.push_back(ch);
		return;
	}

	flushLineBuf();

	// utf-8 conversion
	// TODO: if already in UTF-8??
	char utfchar[4];
	size_t len = Unicode::toUtf8Char(ch, utfchar);
	_writer->write(utfchar, len);
}

void StreamPrinter::doFlushLineBuf()
{
	_writer->print(_lineBuf.c_str(), _lineBuf.size());
	_lineBuf.resize(0);
}

void StreamPrinter::onDelete()
{
	flushLineBuf();
}

bool StreamPrinter::flush()
{
	flushLineBuf();
	return _writer->flush();
}

////////////////////////////////////////////////////////////////////////////////

JsonPrinter::JsonPrinter(StreamWriter* w)
: StreamPrinter(w)
{
	_colonFmt	= "%q:";
	_commaStr	= ",";
	_needComma	= false;
}

JsonPrinter::~JsonPrinter()
{
}

void JsonPrinter::end()
{
	switch (topState())
	{
	case STATE_OBJECT:					endObject(); break;
	case STATE_ARRAY:					endArray(); break;
	default:
		NIT_THROW(EX_INVALID_STATE);
	}
}

void JsonPrinter::setCompact(bool flag)
{
	StreamPrinter::setCompact(flag);

	if (_compact)
	{
		_colonFmt = "%q:";
		_commaStr = ",";
	}
	else
	{
		_colonFmt = "%q : ";
		_commaStr = ", ";
	}
}

void JsonPrinter::beginObject()
{
	comma();
	pushState(STATE_OBJECT);
	open("{");
	_needComma = false;
}

void JsonPrinter::endObject()
{
	while (topState() != STATE_OBJECT)
		end();

	popState();
	close("}");
	_needComma = true;

	needComma();
}

void JsonPrinter::beginArray()
{
	comma();
	pushState(STATE_ARRAY);
	open("[");
	_needComma = false;
}

void JsonPrinter::endArray()
{
	while (topState() != STATE_ARRAY)
		end();

	popState();
	close("]");
	_needComma = true;

	needComma();
}

void JsonPrinter::key(const char* str, size_t len)
{
	if (topState() != STATE_OBJECT)
		beginObject();

	if (_needComma)
	{
		print(_commaStr);
		newline();
	}

	printf(_colonFmt.c_str(), str);
	_needComma = false;
}

void JsonPrinter::open(const char* brace)
{
	if (_compact || newline())
		print(brace);
	else
		printf(" %s", brace);

	++_indent;
	newline();
}

void JsonPrinter::close(const char* brace)
{
	--_indent;
	if (_compact || newline())
		print(brace);
	else
		printf(" %s", brace);
}

void JsonPrinter::printEscaped(const char* str, size_t len)
{
	flushLineBuf();
	Json::writeEscaped(_writer, str);
}

////////////////////////////////////////////////////////////////////////////////

XmlPrinter::XmlPrinter(StreamWriter* w)
: StreamPrinter(w)
{
	_currElem = NULL;

	pushState(STATE_DOCUMENT);
}

XmlPrinter::ElemState* XmlPrinter::pushElemState(const String& name)
{
	ElemState* parent = _currElem;

	_elemStack.push_back(ElemState());
	ElemState* elem = &_elemStack.back();
	elem->name			= name;
	elem->hasAttrs		= false;
	elem->hasText		= false;
	elem->hasChildElem	= false;
	elem->indented		= false;

	if (parent)
	{
		elem->indented = !parent->hasText;
		parent->hasChildElem = true;
	}

	_currElem = elem;

	return elem;
}

void XmlPrinter::popElemState()
{
	_elemStack.pop_back();
	_currElem = _elemStack.empty() ? NULL : &_elemStack.back();
}

void XmlPrinter::beginElem(const char* name)
{
	if (_currElem)
		_currElem->hasChildElem = true;

	if (topState() == STATE_ELEM_TAG_OPEN)
		closeElemTag();

	pushElemState(name);
	pushState(STATE_ELEM);
	pushState(STATE_ELEM_TAG_OPEN);
}

void XmlPrinter::closeElemTag(bool noChild)
{
	if (topState() != STATE_ELEM_TAG_OPEN)
		return;

	popState(); // STATE_ELEM_TAG_OPEN

	if (_currElem->indented)
		newline();

	printf("<%s", _currElem->name.c_str());

	for (uint i=0; i<_attrKeys.size(); ++i)
	{
		const String& key = _attrKeys[i];
		const String& value = _attrValues[i];
		printf(" %s=%q", key.c_str(), value.c_str());
	}

	_attrKeys.resize(0);
	_attrValues.resize(0);

	if (noChild)
		putch('/');

	putch('>');

	incIndent();
}

void XmlPrinter::endElem()
{
	bool noChild = false;

	if (topState() == STATE_ELEM_TAG_OPEN)
	{
		closeElemTag(true);
		noChild = true;
	}

	if (topState() != STATE_ELEM)
		NIT_THROW(EX_INVALID_STATE);

	decIndent();

	if (!noChild)
	{
		if (!_currElem->hasText && _currElem->hasChildElem)
			newline();

		printf("</%s>", _currElem->name.c_str());
	}

	popElemState();
	popState();
}

void XmlPrinter::attr(const char* name, const char* value)
{
	if (topState() != STATE_ELEM_TAG_OPEN)
		NIT_THROW(EX_INVALID_STATE);

	_attrKeys.push_back(name);
	_attrValues.push_back(value);

	_currElem->hasAttrs = true;
}

void XmlPrinter::text(const char* text, size_t len)
{
	if (len == 0) 
		len = strlen(text);

	if (_currElem)
		_currElem->hasText = true;

	if (topState() == STATE_ELEM_TAG_OPEN)
		closeElemTag();

	if (topState() != STATE_ELEM)
		NIT_THROW(EX_INVALID_STATE);

	if (len == 0)
		return;

	printEscaped(text, len);
}

void XmlPrinter::comment(const char* fmt, ...)
{
	print("<-- ");

	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);

	print(" -->");
}

void XmlPrinter::commentA(const char* fmt, IArgIterator* aitr)
{
	print("<-- ");

	aprintf(fmt, aitr);

	print(" -->");
}

void XmlPrinter::onDelete()
{
	while (topState() != STATE_DOCUMENT)
		endElem();

	StreamPrinter::onDelete();
}

////////////////////////////////////////////////////////////////////////////////

// from : http://mews.codeplex.com/SourceControl/changeset/view/52969#392973

class Base64
{
public:
	enum
	{
		BITS_PER_BYTE_PRIME_FACTOR		= 2,
		BITS_PER_BYTE					= BITS_PER_BYTE_PRIME_FACTOR * BITS_PER_BYTE_PRIME_FACTOR * BITS_PER_BYTE_PRIME_FACTOR,
		BITS_PER_ENCODED_BYTE			= 6,

		BIT_OFFSET						= BITS_PER_BYTE - BITS_PER_ENCODED_BYTE,
		ENCODED_BYTE_MASK				= (1 << BITS_PER_ENCODED_BYTE) - 1,

		CANCEL_FATOR1					= BITS_PER_ENCODED_BYTE % BITS_PER_BYTE_PRIME_FACTOR == 0 ? BITS_PER_BYTE_PRIME_FACTOR : 1,
		CANCEL_FACTOR2					= (BITS_PER_ENCODED_BYTE / CANCEL_FATOR1) % BITS_PER_BYTE_PRIME_FACTOR == 0 ? BITS_PER_BYTE_PRIME_FACTOR : 1,

		LEAST_COMMON_MULTIPLE			= BITS_PER_BYTE * BITS_PER_ENCODED_BYTE / CANCEL_FATOR1 / CANCEL_FACTOR2,
		ORIGINAL_BYTES_PER_GROUP		= LEAST_COMMON_MULTIPLE / BITS_PER_BYTE,
		ENCODED_BYTES_PER_GROUP			= LEAST_COMMON_MULTIPLE / BITS_PER_ENCODED_BYTE,
	};

	static const uint8 g_DecodeTable[];
	static const uint8 g_EncodeTable[];
};

const uint8 Base64::g_DecodeTable[] =
{
	// 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // 0x00
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // 0x10
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3e, 0xff, 0xff, 0xff, 0x3f, // 0x20
	0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // 0x30
	0xff, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, // 0x40
	0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xff, 0xff, 0xff, 0xff, 0xff, // 0x50
	0xff, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, // 0x60
	0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff, // 0x70
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // 0x80
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // 0x90
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // 0xa0
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // 0xb0
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // 0xc0
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // 0xd0
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // 0xe0
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff  // 0xf0
};

const uint8 Base64::g_EncodeTable[] =
{
	// 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
	0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, // 0x00
	0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, // 0x10
	0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, // 0x20
	0x77, 0x78, 0x79, 0x7a, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x2b, 0x2f  // 0x30
};

////////////////////////////////////////////////////////////////////////////////

Base64Encoder::Base64Encoder(StreamWriter* to, size_t bufSize /*= 0*/)
{
	if (bufSize == 0)
		bufSize = 256;

	_bufSize = bufSize;
	_buffer = (uint8*) NIT_ALLOC(_bufSize);
	_bufPos = 0;

	_to = to;

	_prevByte = 0xFF;
	_currByteShift = 0;
}

void Base64Encoder::onDelete()
{
	if (_buffer)
	{
		finish();
		NIT_DEALLOC(_buffer, _bufSize);
	}

	StreamWriter::onDelete();
}

size_t Base64Encoder::writeRaw(const void* buf, size_t size)
{
	uint8 currByte = 0xff;
	uint8 encoded;

	uint8* pos = (uint8*)buf;
	uint8* end = pos + size;

	while (pos < end)
	{
		if (_bufPos == _bufSize)
			flushBuffer();

		if (_currByteShift != Base64::BITS_PER_ENCODED_BYTE)
			currByte = *pos++;

		encoded = Base64::g_EncodeTable[(
			(_prevByte << (Base64::BITS_PER_ENCODED_BYTE - _currByteShift)) +
			(currByte >> (_currByteShift + Base64::BIT_OFFSET))) &
			Base64::ENCODED_BYTE_MASK];
		_currByteShift = (_currByteShift + Base64::BIT_OFFSET) % Base64::BITS_PER_BYTE;
		_prevByte = currByte;

		_buffer[_bufPos++] = encoded;
	}

	if (_bufPos == _bufSize)
		flushBuffer();

	return size;
}

void Base64Encoder::flushBuffer()
{
	if (_bufPos == 0) return;

	_to->writeRaw(_buffer, _bufPos);
	_bufPos = 0;
}

void Base64Encoder::finish()
{
	bool isPadding = false;

	while (_currByteShift != 0)
	{
		if (_bufPos == _bufSize)
			flushBuffer();

		_buffer[_bufPos++] = isPadding ? '=' :
			Base64::g_EncodeTable[(_prevByte << (Base64::BITS_PER_ENCODED_BYTE - _currByteShift)) & Base64::ENCODED_BYTE_MASK];
		_currByteShift = (_currByteShift + Base64::BIT_OFFSET) % Base64::BITS_PER_BYTE;
		isPadding = true;
	}

	flushBuffer();
}

bool Base64Encoder::flush()
{
	finish();

	return _to->flush();
}

////////////////////////////////////////////////////////////////////////////////

Base64Decoder::Base64Decoder(StreamReader* from, size_t bufSize)
{
	if (bufSize == 0)
		bufSize = 256;

	_from = from;

	_bufSize = bufSize;
	_buffer = (uint8*) NIT_ALLOC(_bufSize);
	_bufPos = 0;
	_bufAvail = 0;

	_prevDecoded = 0xFF;
	_prevDecodedShift = 0;
}

void Base64Decoder::onDelete()
{
	if (_buffer)
	{
		// rewind source stream if _bufAvail remains
		if (_bufAvail > 0)
		{
			if (_from->isSeekable())
			{
				_from->seek(-(int)_bufAvail);
				_bufAvail = 0;
			}

			ASSERT_THROW(_bufAvail == 0, EX_CORRUPTED);
		}

		NIT_DEALLOC(_buffer, _bufSize);
	}
}

bool Base64Decoder::isEof()
{
	if (_bufAvail > 0) return false;

	return _from && _from->isEof();
}

size_t Base64Decoder::readRaw(void* buf, size_t size)
{
	uint8* bytes = (uint8*)buf;
	size_t index = 0;

	while (index < size)
	{
		if (_bufAvail == 0)
		{
			_bufPos = 0;
			_bufAvail = _from->readRaw(_buffer, _bufSize);
			if (_bufAvail == 0)
				return index;
		}

		// TODO: handle '='
		uint8 encoded = _buffer[_bufPos++];
		--_bufAvail;
		uint8 decoded = Base64::g_DecodeTable[encoded];

		if (decoded != 0xFF)
		{
			if (_prevDecodedShift != 0)
			{
				bytes[index++] = ((_prevDecoded << _prevDecodedShift) +
					(decoded >> (Base64::BITS_PER_ENCODED_BYTE - _prevDecodedShift))) & 0xFF;
			}

			_prevDecoded = decoded;
			_prevDecodedShift = (_prevDecodedShift + Base64::BIT_OFFSET) % Base64::BITS_PER_BYTE;
		}
	}

	return index;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
