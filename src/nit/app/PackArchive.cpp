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

#include "nit/app/PackArchive.h"

#include "nit/io/FileLocator.h"
#include "nit/io/ZStream.h"
#include "nit/io/MemoryBuffer.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////

PackArchive::PackArchive(const String& name, StreamSource* packFile)
: Archive(name)
{
	_header.signature	= 0;
	_flipEndian		= false;

	_realFile			= dynamic_cast<nit::File*>(packFile);
	_realFileOffset	= 0;

	if (_realFile == NULL)
	{
		PackArchive::File* arcFile = dynamic_cast<PackArchive::File*>(packFile);
		if (arcFile)
		{
			_realFile = arcFile->getPack()->_realFile;
			_realFileOffset = arcFile->_fileEntry.offset;
		}
	}

	if (_realFile == NULL)
	{
		NIT_THROW_FMT(EX_INVALID_PARAMS, "PackArchive '%s': not based on a real file", name.c_str());
	}

	load();
}

void PackArchive::load()
{
	if (_header.signature) return;

	Ref<StreamReader> r = _realFile->open();
	r->seek((size_t)_realFileOffset); // TODO: apply uint64 on seek

	readHeader(r);

	for (uint i=0; i < _header.numFiles; ++i)
	{
		readFileEntry(r);
	}
}

void PackArchive::readHeader(StreamReader* reader)
{
	reader->read(&_header, sizeof(_header));

	if (_flipEndian)
		_header.flipEndian();

	if (_header.signature == NIT_PACK_SIGNATURE_FLIP)
	{
		_flipEndian = true;
		_header.flipEndian();
	}

	if (_header.signature != NIT_PACK_SIGNATURE)
	{
		_header.signature = 0;

		NIT_THROW_FMT(EX_CORRUPTED, "Invalid pack signature: %s", _name.c_str());
	}

	if (_header.version != NIT_PACK_VERSION)
	{
		_header.signature = 0;

		NIT_THROW_FMT(EX_CORRUPTED, "Invalid pack version: %s (%08x)", _name.c_str(), _header.version);
	}

	if (_header.extHeaderSize)
		reader->skip(_header.extHeaderSize);
}

void PackArchive::readFileEntry(StreamReader* reader)
{
	uint filenameLen = 0;
	reader->read(&filenameLen, sizeof(filenameLen));

	if (_flipEndian)
		StreamUtil::flipEndian(filenameLen);

	char name[MAX_PATH];
	reader->read(name, filenameLen);
	name[filenameLen] = 0;

	File* file = new File(this, name);
	FileEntry& entry = file->_fileEntry;

	reader->read(&entry, sizeof(entry));

	file->_contentType = entry.contentType;

	if (_flipEndian)
		entry.flipEndian();

	_files.insert(std::make_pair(name, file));
}

void PackArchive::unload()
{
 	if (_header.signature == 0)
 		return;

	_header.signature = 0;

	LOG(0, ".. unloading pack archive '%s' %08x (%d)\n", _name.c_str(), this, getRefCount());

	bool logUnloading = false;

	if (logUnloading)
	{
		for (Files::iterator itr = _files.begin(), end = _files.end(); itr != end; ++itr)
		{
			File* file = itr->second;
			LOG(0, ".. %s %s (%d)\n",
				itr->first.c_str(),
				file->IsCached() ? "cached" : "active", file->getRefCount());
		}
	}

	_files.clear();
}

StreamSource* PackArchive::locateLocal(const String& streamName)
{
	Files::iterator itr = _files.find(streamName);

	return itr != _files.end() ? itr->second : NULL;
}

void PackArchive::findLocal(const String& pattern, StreamSourceMap& varResults)
{
	for (Files::iterator itr = _files.begin(), end = _files.end(); itr != end; ++itr)
	{
		if (Wildcard::match(pattern, itr->first) && varResults.find(itr->first) == varResults.end())
			varResults.insert(std::make_pair(itr->first, itr->second));
	}
}

StreamReader* PackArchive::processPayload(FileEntry* entry, FileReader* reader)
{
	switch (entry->payloadType)
	{
	case PAYLOAD_RAW:					
	case PAYLOAD_VOID:					return reader;

	case PAYLOAD_ZLIB:					return new ZStreamReader(reader);
	case PAYLOAD_ZLIB_FAST:				return new MemoryBuffer::Reader(new ZStreamReader(reader), entry->memorySize);

	default:
		NIT_THROW_FMT(EX_NOT_SUPPORTED, "'%s': not supported payload (%d)", reader->getUrl().c_str(), entry->payloadType);
	}
}

StreamReader* PackArchive::openExtHeader()
{
	if (_header.signature == 0) return NULL;
	if (_header.extHeaderSize == 0) return NULL;

	// TODO: apply uint64 on offset
	FileReader* reader = dynamic_cast<FileReader*>(_realFile->openRange(size_t(_realFileOffset + sizeof(_header)), (size_t)_header.extHeaderSize));

	return reader;
}

////////////////////////////////////////////////////////////////////////////////////////////

PackArchive::File::File(PackArchive* pack, const String& name)
: StreamSource(pack, name, ContentType::UNKNOWN)
{
}

StreamReader* PackArchive::File::open()
{
	PackArchive* pack = getPack();
	return pack->processPayload(&_fileEntry, openPayload());
}

FileReader* PackArchive::File::openPayload()
{
	PackArchive* pack = getPack();

	// TODO: apply uint64 on offset
	FileReader* reader = dynamic_cast<FileReader*>(
		pack->_realFile->openRange(size_t(pack->_realFileOffset + _fileEntry.offset), _fileEntry.payloadSize, this));

	return reader;
}

////////////////////////////////////////////////////////////////////////////////////////////

void PackArchive::Header::flipEndian()
{
	StreamUtil::flipEndian(signature);
	StreamUtil::flipEndian(version);
	StreamUtil::flipEndian(extHeaderSize);
	StreamUtil::flipEndian(numFiles);
	StreamUtil::flipEndian(target);
	StreamUtil::flipEndian(timestamp);
}

////////////////////////////////////////////////////////////////////////////////////////////

void PackArchive::FileEntry::flipEndian()
{
	StreamUtil::flipEndian(offset);
	StreamUtil::flipEndian(contentType);
	StreamUtil::flipEndian(payloadType);
	StreamUtil::flipEndian(sourceSize);
	StreamUtil::flipEndian(memorySize);
	StreamUtil::flipEndian(sourceCRC32);
	StreamUtil::flipEndian(timestamp);
	StreamUtil::flipEndian(payloadSize);
	StreamUtil::flipEndian(payloadCRC32);
	StreamUtil::flipEndian(payloadParam0);
	StreamUtil::flipEndian(payloadParam1);
}

////////////////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
