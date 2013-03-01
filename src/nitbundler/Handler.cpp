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

#include "nitbundler_pch.h"

#include "nitbundler/Handler.h"

#include "nit/app/PackArchive.h"
#include "nit/io/ZStream.h"

NS_BUNDLER_BEGIN;

////////////////////////////////////////////////////////////////////////////////

Handler::Handler()
{
	_payloadType = PackArchive::PAYLOAD_RAW;
}

void Handler::addMergeList(const String& filename)
{
	NIT_THROW_FMT(EX_NOT_SUPPORTED, 
		"%s doesn't support merge: %s", typeid(*this).name(), filename.c_str());
}

void Handler::setCodec(const String& codec)
{
	NIT_THROW_FMT(EX_NOT_SUPPORTED, 
		"%s doesn't support codec: %s", typeid(*this).name(), codec.c_str());
}

void Handler::setPayload(const String& payload)
{
	if (payload == "raw")
	{
		_payloadType = PackArchive::PAYLOAD_RAW;
	}
	else if (payload == "zlib")
	{
		_payloadType = PackArchive::PAYLOAD_ZLIB;
	}
	else if (payload == "zlib_fast")
	{
		_payloadType = PackArchive::PAYLOAD_ZLIB_FAST;
	}
	else
	{
		NIT_THROW_FMT(EX_NOT_SUPPORTED, 
			"%s doesn't support payload: %s", typeid(*this).name(), payload.c_str());
	}

	_payloadStr = payload;
}

void Handler::setResize(const String& resize)
{
	NIT_THROW_FMT(EX_NOT_SUPPORTED, 
		"%s doesn't support resize: %s", typeid(*this).name(), resize.c_str());
}

void Handler::setCompile(const String& compile)
{
	NIT_THROW_FMT(EX_NOT_SUPPORTED, 
		"%s doesn't support compile: %s", typeid(*this).name(), compile.c_str());
}

void Handler::generate(StreamSource* source)
{
	Ref<StreamSource> safe = source;

	if (source == NULL)
	{
		LOG(0, "++ '%s: %s': no stream source\n", _entry->getPacker()->getName().c_str(), _entry->getFilename().c_str());
		_entry->getData()->payloadType = PackArchive::PAYLOAD_VOID;
		_entry->getData()->payloadSize = 0;
		return;
	}

	size_t srcSize = source->getStreamSize();

	if (srcSize == 0)
	{
		LOG(0, "++ '%s: %s': empty stream source\n", _entry->getPacker()->getName().c_str(), _entry->getFilename().c_str());
		_entry->getData()->payloadType = PackArchive::PAYLOAD_RAW;
		_entry->getData()->payloadSize = 0;
		return;
	}

	size_t begin = _entry->getWriter()->tell();

	Ref<StreamWriter> w = _entry->getWriter();

	ContentType ct = _entry->getData()->contentType;
	
	if (ct.isCompressed() || ct.isArchive())
	{
		_payloadType = PackArchive::PAYLOAD_RAW;
	}
	else
	{
		switch (_payloadType)
		{
		case PackArchive::PAYLOAD_ZLIB:
		case PackArchive::PAYLOAD_ZLIB_FAST:
			w = new ZStreamWriter(w, _payloadType == PackArchive::PAYLOAD_ZLIB_FAST); 
			break;
		}
	}

	w->copy(source->open());

	w = NULL; // finishing flush

	size_t end = _entry->getWriter()->tell();
	_entry->getData()->payloadType = _payloadType;
	_entry->getData()->payloadSize = end - begin;

	bool rawIfNotEfficient = true;

	if (rawIfNotEfficient && end - begin > srcSize)
	{
		LOG(0, "++ '%s: %s': payload '%s': inefficient %d bytes - replaced by 'raw'\n", 
			_entry->getPacker()->getName().c_str(),
			_entry->getFilename().c_str(), 
			_payloadStr.c_str(),
			(end - begin) - srcSize);

		_entry->getWriter()->seek(begin);
		_entry->getWriter()->copy(source->open());
		_entry->getData()->payloadType = PackArchive::PAYLOAD_RAW;
		_entry->getData()->payloadSize = srcSize;

		// In this case, writer may go beyond than needed when tried to compress (and discarded bloated compression).
		// So Packer have to check PayloadSize always and seek back appropriately.
	}
}

void Handler::generate()
{
	generate(_entry->getSource());
}

////////////////////////////////////////////////////////////////////////////////

void CopyHandler::generate()
{
	Handler::generate();
}

////////////////////////////////////////////////////////////////////////////////

void PackCfgHandler::generate()
{
	Handler::generate();
}

////////////////////////////////////////////////////////////////////////////////

NitScriptHandler::NitScriptHandler()
{
	_compile = false;
}

void NitScriptHandler::setCompile(const String& compile)
{
	if (compile == "nit")
		_compile = true;
	else
		Handler::setCompile(compile);
}

static void CompileErrorHandler(HSQUIRRELVM v,const SQChar *sErr,const SQChar *sSource,SQInteger line,SQInteger column)
{
	SQPRINTFUNCTION pf = sq_geterrorfunc(v);
	if(pf) {
		pf(v,_SC("*** %s at %s line %d, column %d\n"), sErr, sSource, line, column);
	}
}

static void printfunc(HSQUIRRELVM v,const SQChar *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	LogManager::getSingleton().vlog(0, 0, 0, 0, 0, fmt, args);
	va_end(args);
}

void NitScriptHandler::generate()
{
	if (!_compile)
		return Handler::generate();

	HSQUIRRELVM v = sq_open(1024);
	sq_setprintfunc(v, printfunc, printfunc);
	sq_setcompilererrorhandler(v, CompileErrorHandler);

	String unitName = StringUtil::format("%s: %s", _entry->getPacker()->getName().c_str(), _entry->getFilename().c_str());
	SQRESULT sr = ScriptIO::loadstream(v, _entry->getSource()->open(), unitName, true);

	if (SQ_FAILED(sr))
	{
		LOG(0, "*** can't compile '%s' - copying source code\n", _entry->getFilename().c_str());
		sq_close(v, NULL);
		return Handler::generate();
	}

	Ref<MemorySource> src = new MemorySource(_entry->getSource());
	Ref<StreamWriter> w = src->newWriter();
	sq_writeclosure(v, ScriptIO::bytecode_write, w, _entry->getPacker()->isBigEndian());

	_entry->getData()->contentType = ContentType::BINARY_NIT_SCRIPT;

	sq_close(v, NULL);
	Handler::generate(src);
}

////////////////////////////////////////////////////////////////////////////////

bool DiffEntryHandler::prepare()
{
	// Copy original entry as is.
	*_entry->getData() = _original->getEntry();

	// Reset the offset so that the packer can handle properly.
	_entry->getData()->offset = 0;

	return true;
}

void DiffEntryHandler::generate()
{
	Ref<StreamWriter> w = _entry->getWriter();

	// Copy original payload as is.
	w->copy(_original->openPayload());
}

////////////////////////////////////////////////////////////////////////////////

bool DeletedEntryHandler::prepare()
{
	_entry->getData()->contentType = ContentType::DELETED;

	return true;
}

void DeletedEntryHandler::generate()
{
	_entry->getData()->payloadType = PackArchive::PAYLOAD_VOID;

	// No copy action
}

////////////////////////////////////////////////////////////////////////////////

NS_BUNDLER_END;
