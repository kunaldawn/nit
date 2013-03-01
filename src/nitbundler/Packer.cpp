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

#include "nitbundler/Packer.h"
#include "nitbundler/Job.h"

#include "nit/app/PackArchive.h"
#include "nit/async/AsyncJob.h"

NS_BUNDLER_BEGIN;

////////////////////////////////////////////////////////////////////////////////

void Packer::FileEntry::init(const String& filename, Packer* packer, StreamSource* source, Handler* handler)
{
	_filename		= filename;
	_packer		= packer;
	_source		= source;
	_handler		= handler;
	_writer		= NULL;

	memset(&_data, 0, sizeof(_data));

	_entryOffset	= 0;
	
	_prepared		= false;
	_deletedMark	= false;
}

////////////////////////////////////////////////////////////////////////////////

Packer::Packer(PackSource* source, const String& filename)
{
	init(source->getBuilder());

	_name			= source->getName();
	_filename		= filename;

	if (filename.empty())
		_filename	= _name + ".pack";

	// Register the pack.cfg by default
	// TODO: Refactor to PackSource or Builder
	assign("pack.cfg", source->getPackCfg()->getSource(), new PackCfgHandler());
}

Packer::Packer(const String& bundleName, Builder* builder)
{
	init(builder);

	_name			= bundleName;
	_filename		= bundleName + ".bundle";
}

Packer::Packer(const String& packName, const String& filename, uint32 platformCode, bool bigEndian, FileLocator* dumpPath)
{
	init(NULL);

	_name			= packName;
	_filename		= filename;

	_platformCode	= platformCode;
	_bigEndian		= bigEndian;
	_dumpPath		= dumpPath;
}

void Packer::init(Builder* builder)
{
	_builder		= builder;

	if (builder)
	{
		_platformCode	= builder->getPlatform()->getPlatformCode();
		_bigEndian		= builder->getPlatform()->isBigEndian();
		_dumpPath		= builder->getDumpPath();
	}

	_preparing		= false;
	_building		= false;
	_built			= false;
	_deploying		= false;
	_deployed		= false;
}

Handler* Packer::assign(const String& filename, StreamSource* source)
{
	if (_building)
		NIT_THROW(EX_INVALID_STATE);

	if (filename.length() >= MAX_PATH)
		NIT_THROW_FMT(EX_OVERFLOW, "too long filename: '%s'", filename.c_str());

	FileEntries::iterator itr = _fileEntries.find(filename);
	if (itr != _fileEntries.end())
		NIT_THROW(EX_DUPLICATED);

	if (_builder == NULL) NIT_THROW(EX_NULL);
	Handler* handler = _builder->getPlatform()->NewHandler(StreamSource::getExtension(filename));

	if (handler == NULL)
		NIT_THROW(EX_NOT_SUPPORTED);

	return assign(filename, source, handler);
}

Handler* Packer::assign(const String& filename, StreamSource* source, Handler* handler)
{
	FileEntries::iterator itr = _fileEntries.insert(std::make_pair(filename, FileEntry())).first;
	FileEntry& entry = itr->second;

	entry.init(filename, this, source, handler);

	_pendingEntries.push_back(&entry);

	handler->_entry = &entry;

	return handler;
}

void Packer::prepare()
{
	_preparing = true;

	while (!_pendingEntries.empty()) // Till there're no more added entries
	{
		EntryList preparing;
		preparing.swap(_pendingEntries);

		// Prepare added entries so far.
		for (EntryList::iterator itr = preparing.begin(), end = preparing.end(); itr != end; ++itr)
		{
			FileEntry* entry = *itr;

			if (!entry->isPrepared())
				entry->_prepared = entry->getHandler()->prepare();

			// Setup fields not set during prepare.
			if (entry->_data.contentType == ContentType::UNKNOWN)
			{
				if (entry->_source)
					entry->_data.contentType = entry->_source->getContentType();
				else
					entry->_data.contentType = ContentType::fromExtension(entry->_filename);
			}

			if (entry->_source)
			{
				if (entry->_data.timestamp == 0)
					entry->_data.timestamp = entry->_source->getTimestamp().getUnixTime64();

				if (entry->_data.sourceCRC32 == 0 && entry->_source)
					entry->_data.sourceCRC32 = entry->_source->calcCrc32();
			}
		}
	}

	_preparing = false;
}

bool Packer::checkNeedBuild(const String& packfile)
{
	return true;
}

Packer::Job* Packer::newJob(const String& fileFilter, bool rebuild, Ref<StreamWriter> packWriter)
{
	return new Job(this, fileFilter, rebuild, packWriter);
}

void Packer::onDelete()
{
	MTRefCounted::onDelete();
}

////////////////////////////////////////////////////////////////////////////////

NS_BUNDLER_END;
