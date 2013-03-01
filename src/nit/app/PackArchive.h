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

#include "nit/io/Archive.h"
#include "nit/io/FileLocator.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

#define NIT_PACK_SIGNATURE				NIT_MAKE_CC('N', 'E', 'A', 'T')
#define NIT_PACK_SIGNATURE_FLIP			NIT_MAKE_CC('T', 'A', 'E', 'N')
#define NIT_PACK_VERSION				NIT_MAKE_CC('1', '.', '0', '0')

#define NIT_PACK_TARGET_WIN32			NIT_MAKE_CC('w', '3', '2', '!')
#define NIT_PACK_TARGET_IOS				NIT_MAKE_CC('i', 'o', 's', '!')
#define NIT_PACK_TARGET_MAC32			NIT_MAKE_CC('m', '3', '2', '!')
#define NIT_PACK_TARGET_ANDROID			NIT_MAKE_CC('g', 'o', 'g', 'l')

////////////////////////////////////////////////////////////////////////////////

class NIT_API PackArchive : public Archive
{
public:
	struct NIT_API Header
	{
		// 32 byte header
		uint32							signature;		//  4
		uint32							version;		//  8
		uint16							extHeaderSize;	// 10
		uint16							numFiles;		// 12
		uint32							target;			// 16
		uint32							_reserved0;		// 20
		uint32							_reserved1;		// 24
		uint64							timestamp;		// 32

		void							flipEndian();
	};

	struct NIT_API FileEntry
	{
		// 64 byte entry
		uint64							offset;			//  8
		ContentType::ValueType			contentType;	// 10
		uint16							payloadType;	// 12
		uint32							_reserved0;		// 16

		uint32							sourceSize;		// 20
		uint32							_reserved1;		// 24
		uint32							memorySize;		// 28
		uint32							_reserved2;		// 32
		
		uint32							sourceCRC32;	// 36
		uint32							_reserved3;		// 40
		uint64							timestamp;		// 48

		uint32							payloadSize;	// 52
		uint32							payloadCRC32;	// 56
		uint32							payloadParam0;	// 60
		uint32							payloadParam1;	// 64

		void							flipEndian();
	};

	enum PayloadType
	{
		PAYLOAD_RAW						= 0,
		PAYLOAD_VOID					= 1,
		PAYLOAD_ZLIB					= 2,
		PAYLOAD_ZLIB_FAST				= 3,
	};

	class File;

public:
	PackArchive(const String& name, StreamSource* packFile);

public:									// StreamLocator implementation
	virtual bool						isCaseSensitive()						{ return false; }

	virtual StreamSource*				locateLocal(const String& streamName);
	virtual void						findLocal(const String& pattern, StreamSourceMap& varResults);

public:									// Archive implementation
	virtual void						load();
	virtual void						unload();

	virtual bool						isReadOnly()							{ return true; }

	virtual StreamWriter*				create(const String& streamName)		{ NIT_THROW(EX_NOT_SUPPORTED); return NULL; }
	virtual StreamWriter*				modify(const String& streamName)		{ NIT_THROW(EX_NOT_SUPPORTED); return NULL; }
	virtual void						remove(const String& pattern)			{ NIT_THROW(EX_NOT_SUPPORTED); }
	virtual void						rename(const String& streamName, const String& newName) { NIT_THROW(EX_NOT_SUPPORTED); }

public:
	const Header&						getHeader()								{ return _header; }
	StreamReader*						openExtHeader();

public:
	nit::File*							getRealFile()							{ return _realFile; }
	uint64								getRealFileOffset()						{ return _realFileOffset; }

	bool								isEndianFlip()							{ return _flipEndian; }

private:
	void								readHeader(StreamReader* reader);
	void								readFileEntry(StreamReader* reader);

private:
	Ref<nit::File>						_realFile;
	uint64								_realFileOffset;

	Header								_header;

	friend class						PackBuilder;

	typedef map<String, RefCache<File>, StringUtil::LessIgnoreCase>::type Files;
	Files								_files;
	bool								_flipEndian;

	StreamReader*						processPayload(FileEntry* entry, FileReader* reader);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API PackArchive::File : public StreamSource
{
protected:
	File(PackArchive* pack, const String& name);

public:									// StreamSource Implementation
	virtual size_t						getStreamSize()							{ return _fileEntry.payloadSize; }
	virtual size_t						getMemorySize()							{ return _fileEntry.memorySize; }
	virtual Timestamp					getTimestamp()							{ return _fileEntry.timestamp; }

	virtual StreamReader*				open();

public:
	FileReader*							openPayload();

	PackArchive*						getPack()								{ return static_cast<PackArchive*>(getRealLocator()); }
	const FileEntry&					getEntry()								{ return _fileEntry; }
	uint64								getOffset()								{ return _fileEntry.offset; }
	uint16								getPayloadType()						{ return _fileEntry.payloadType; }
	uint32								getPayloadCRC32()						{ return _fileEntry.payloadCRC32; }
	uint32								getPayloadParam0()						{ return _fileEntry.payloadParam0; }
	uint32								getPayloadParam1()						{ return _fileEntry.payloadParam1; }
	uint32								getSourceSize()							{ return _fileEntry.sourceSize; }
	uint32								getSourceCRC32()						{ return _fileEntry.sourceCRC32; }

protected:
	FileEntry							_fileEntry;

	friend class PackArchive;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
