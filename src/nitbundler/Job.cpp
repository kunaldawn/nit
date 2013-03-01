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

#include "nitbundler/Job.h"

#include "nit/runtime/NitRuntime.h"
#include "nit/io/ZStream.h"

NS_BUNDLER_BEGIN;

////////////////////////////////////////////////////////////////////////////////

Builder::Job::Job(Builder* builder, const String& packFilter, const String& fileFilter, bool makeBundle)
{
	_builder = builder;
	_packFilter = packFilter;
	_fileFilter = fileFilter;
	_makeBundle = makeBundle;
}

bool Builder::Job::onPrepare()
{
	LOG_TIMESCOPE(0, "++ Building '%s' revision %s", _builder->_appCfg->getName().c_str(), _builder->_revision.c_str());

	StreamSourceMap packCfgs;

	for (PackSources::iterator itr = _builder->_packs.begin(), end = _builder->_packs.end(); itr != end; ++itr)
	{
		PackSource* pack = itr->second;

		if (pack->isIgnored())
		{
			LOG(0, ".. Ignored: '%s'\n", pack->getName().c_str());
			continue;
		}

		if (!pack->isLinked())
		{
			LOG(0, ".. Not Linked: '%s'\n", pack->getName().c_str());
			continue;
		}

		if (!Wildcard::match(_packFilter, pack->getName()))
		{
			LOG(0, ".. Filtered out: '%s'\n", pack->getName().c_str());
			continue;
		}

		LOG_TIMESCOPE(0, "++ Building '%s'", pack->getName().c_str());

		pack->prepareCommands();

		Ref<Packer> packer = new Packer(pack, StringUtil::format("%s_packs/%s.pack", _builder->_buildTarget.c_str(), pack->getName().c_str()));

		pack->executeCommands(packer);

		packCfgs.insert(std::make_pair(pack->getName(), pack->getPackCfg()->getSource()));

		enqueueSubJob(packer->newJob(_fileFilter));
	}

	FileUtil::createDir(_builder->_outPath->makeUrl(StringUtil::format("%s_packs", _builder->_buildTarget.c_str())));
	String lookupDBPath = _builder->_outPath->makeUrl(StringUtil::format("%s_packs/lookup.sqlite", _builder->_buildTarget.c_str()));
	FileUtil::normalizeSeparator(lookupDBPath);
	FileUtil::remove(lookupDBPath);
	g_Package->buildLookupDB(lookupDBPath, packCfgs);
	
	return true;
}

bool Builder::Job::onExecute(bool async)
{
	// NO-OP until sub jobs completed
	return true;
}

void Builder::Job::onSubJobFinished(AsyncJob* subJob, Status status)
{
	if (getSubJobCount() == 0)
	{
		if (_makeBundle && _bundleJob == NULL)
		{
			_bundleJob = new BundleJob(_builder, _packFilter);
			enqueueSubJob(_bundleJob);
		}
	}
}

void Builder::Job::onFinish()
{
	// Wait if there're remaining pack jobs (by calling retry())
	if (getSubJobCount() > 0)
		return retry(true);
}

StreamSource* Builder::Job::getOutput()
{
	return _bundleJob ? _bundleJob->getOutput() : NULL;
}

////////////////////////////////////////////////////////////////////////////////

Builder::BundleJob::BundleJob(Builder* builder, const String& packFilter)
{
	_builder = builder;
	_packFilter = packFilter;
}

bool Builder::BundleJob::onPrepare()
{
	DateTime dt = DateTime::now();

	String bundleUID = "full_";
	bundleUID.append(_builder->_appCfg->get("app_name", "unnamed_bundle"));
	bundleUID.push_back('_');
	bundleUID.append(_builder->getPlatformID());
	bundleUID.push_back('_');
	bundleUID.append(_builder->getBuildTarget());
	bundleUID.push_back('_');
	bundleUID.append(_builder->_revision);
	bundleUID.push_back('_');
	bundleUID.append(_builder->_buildHost);
	bundleUID.push_back('_');
	bundleUID.append(dt.format("%Y%m%d_%H%M%S"));

	StringUtil::toLowerCase(bundleUID);

	String bundleName = bundleUID;

	StringUtil::toLowerCase(bundleName);

	String baseUID = ""; // base_uid will be generated at BuildDelta
	Ref<MemoryBuffer> bundleCfgBuf = new MemoryBuffer();
	Ref<MemoryBuffer> extDataBuf = new MemoryBuffer();

	// Make a bundle.cfg
	_bundleCfg = new Settings();
	Ref<Settings> sect = _builder->createBundleSection(bundleUID, baseUID);
	sect->add("build_host", _builder->_buildHost);
	sect->add("build_date", dt.format("%Y-%m-%d"));
	sect->add("build_time", dt.format("%H:%M:%S"));

	_bundleCfg->addSection("bundle", sect);
	_bundleCfg->saveCfg(new MemoryBuffer::Writer(bundleCfgBuf, NULL));

	_packer = new Packer(bundleName, _builder);

	{
		LOG_TIMESCOPE(0, "++ Making bundle '%s'", bundleName.c_str());

		Ref<FileLocator> packs = new FileLocator("$packs", _builder->_outPath->getBaseUrl() + "/" + _builder->_buildTarget + "_packs");

		// Include the app.cfg
		Ref<Handler> hdlr = _packer->assign("app.cfg", _builder->_appCfg->getSource());
		hdlr->setPayload("zlib");

		// Include the bundle.cfg
		hdlr = _packer->assign("bundle.cfg", Ref<StreamSource>(new MemorySource("bundle.cfg", bundleCfgBuf)));
		hdlr->setPayload("zlib");

		// Include the lookup.sqlite
		hdlr = _packer->assign("lookup.sqlite", packs->locate("lookup.sqlite"));

		// Add each generated pack into the bundle
		for (PackSources::iterator itr = _builder->_packs.begin(), end = _builder->_packs.end(); itr != end; ++itr)
		{
			PackSource* pack = itr->second;

			if (pack->isIgnored() || !pack->isLinked())
				continue;

			if (!Wildcard::match(_packFilter, pack->getName()))
				continue;

			Ref<StreamSource> src = packs->locate(pack->getName() + ".pack", false);
			if (src == NULL)
			{
				LOG(0, "*** '%s.pack' not found\n", pack->getName().c_str());
				continue;
			}

			_packer->assign(src->getName(), src);
		}
	}

	_packJob = _packer->newJob();
	enqueueSubJob(_packJob);

	return true;
}

bool Builder::BundleJob::onExecute(bool async)
{
	// NO-OP until sub jobs completed
	return true;
}

void Builder::BundleJob::onSubJobFinished(AsyncJob* subJob, Status status)
{
	if (subJob == _packJob && subJob->getStatus() == JOB_SUCCESS)
	{
		_zBundleBuildJob = new ZBundleBuildJob(
			_builder->_outPath->locate(_packer->getFilename()),
			_bundleCfg,
			_builder->_platform->isBigEndian(),
			_builder->_outPath.get(),
			NULL);

		enqueueSubJob(_zBundleBuildJob);
	}
}

void Builder::BundleJob::onFinish()
{
	// Wait if there're remaining pack jobs (by calling retry())
	if (getSubJobCount() > 0)
		return retry(true);
}

StreamSource* Builder::BundleJob::getOutput()
{
	return _zBundleBuildJob ? _zBundleBuildJob->getOutput() : NULL;
}

////////////////////////////////////////////////////////////////////////////////

Packer::Job::Job(Packer* packer, const String& fileFilter, bool rebuild, Ref<StreamWriter> packWriter)
{
	_packer = packer;
	_fileFilter = fileFilter;
	_rebuild = rebuild;
	_packWriter = packWriter;

	_entryCount = 0;
	_writeCount = 0;
}

bool Packer::Job::onPrepare()
{
	// Prepare all entries that are allocated
	_packer->prepare();

	assert(_packer->_pendingEntries.empty());

	// Generate final building list
	for (FileEntries::iterator itr = _packer->_fileEntries.begin(), end = _packer->_fileEntries.end(); itr != end; ++itr)
	{
		FileEntry& entry = itr->second;

		if (!Wildcard::match(_fileFilter, entry.getFilename()))
			continue;

		if (entry.isPrepared() && !entry.isDeletedMark())
			_entries.push_back(&entry);
	}

	// TODO: Compare to existing old build and skip if not needed

	if (_packWriter == NULL)
	{
		if (_packer->_builder == NULL) NIT_THROW(EX_NULL);
		_packWriter = _packer->_builder->getOutPath()->create(_packer->_filename);
	}

	// Prepare file generation
	writeHeader();
	writeDummyEntries();

	// Enqueue async job for each entry
	for (EntryList::iterator itr = _entries.begin(), end = _entries.end(); itr != end; ++itr)
	{
		FileEntry* entry = *itr;

		enqueueSubJob(new Packer::EntryJob(entry));
		++_entryCount;
	}

	return true;
}

void Packer::Job::writeHeader()
{
	PackArchive::Header header = { 0 };

	header.signature	= NIT_PACK_SIGNATURE;
	header.version		= NIT_PACK_VERSION;
	header.target		= _packer->_platformCode;
	header.numFiles		= _entries.size();
	header.timestamp	= Timestamp::now().getUnixTime64();

	if (_packer->_bigEndian)
		header.flipEndian();

	_packWriter->writeRaw(&header, sizeof(header));
}

void Packer::Job::writeDummyEntries()
{
	for (EntryList::iterator itr = _entries.begin(), end = _entries.end(); itr != end; ++itr)
	{
		FileEntry* entry = *itr;

		entry->_entryOffset = _packWriter->tell();

		_packWriter->skip(sizeof(uint));
		_packWriter->skip(entry->_filename.size());
		_packWriter->skip(sizeof(PackArchive::FileEntry));
	}
}

void Packer::Job::writeFileEntries()
{
	for (EntryList::iterator itr = _entries.begin(), end = _entries.end(); itr != end; ++itr)
	{
		FileEntry* entry = *itr;

		_packWriter->seek(entry->_entryOffset);

		PackArchive::FileEntry fe = entry->_data;
		if (_packer->_bigEndian)
			fe.flipEndian();

		uint filenameLen = entry->_filename.size();
		_packWriter->writeRaw(&filenameLen, sizeof(filenameLen));
		_packWriter->writeRaw(entry->_filename.c_str(), filenameLen);
		_packWriter->writeRaw(&fe, sizeof(fe));
	}
}

bool Packer::Job::onExecute(bool async)
{
	getMutex().lock();
	list<Ref<EntryJob> >::type queue;
	_writeQueue.swap(queue);
	getMutex().unlock();

	while (!queue.empty())
	{
		Ref<EntryJob> job = queue.front();
		queue.pop_front();

		// The position of current pack's writer points the entry's offset.
		job->getEntry()->_data.offset = _packWriter->tell();

		// Write entry's content to the pack writer
		job->getBuffer()->save(_packWriter);

		++_writeCount;
	}

	// NO-OP until sub jobs completed
	return true;
}

void Packer::Job::onSubJobFinished(AsyncJob* subJob, Status status)
{
	EntryJob* job = dynamic_cast<EntryJob*>(subJob);

	if (job && status == JOB_SUCCESS)
	{
		getMutex().lock();
		_writeQueue.push_back(job);
		getMutex().unlock();
	}
}

void Packer::Job::onFinish()
{
	// If there are more file to write, wait (by calling retry())
	if (_writeCount < _entryCount)
		return retry(true);

	writeFileEntries();

	_output = _packWriter->getSource();

	_packWriter->flush();
	_packWriter = NULL;
	_packer->_building = false;
	_packer->_built = true;

	_entries.clear();

	_packer = NULL;
}

////////////////////////////////////////////////////////////////////////////////

Packer::EntryJob::EntryJob(FileEntry* entry)
{
	_entry = entry;
}

bool Packer::EntryJob::onPrepare()
{
	if (_entry->_source)
	{
		if (_entry->_data.sourceSize == 0)
			_entry->_data.sourceSize = _entry->_source->getStreamSize();
		if (_entry->_data.memorySize == 0)
			_entry->_data.memorySize = _entry->_source->getMemorySize();
	}

	_writer = new MemoryBuffer::Writer();
	_buffer = _writer->getBuffer();

	_entry->_writer = _writer;

	return true;
}

bool Packer::EntryJob::onExecute(bool async)
{
	// Tell the handler generate using the buffer
	LOG(0, "-- Packing '%s: %s'\n", _entry->_packer->getName().c_str(), _entry->getFilename().c_str());
	_entry->_handler->generate();

	// If handler specifies payload size, resize the buffer according to the size (if tried compression in vain)
	if (_entry->_data.payloadSize != _buffer->getSize())
		_buffer->resize(_entry->_data.payloadSize); // TODO: Do we have to fill zeroes?
	else
		_entry->_data.payloadSize = _buffer->getSize();

	// Calc CRC32 - for bundle diff
	if (_entry->_data.payloadCRC32 == 0)
		_entry->_data.payloadCRC32 = _buffer->calcCrc32();

	return true;
}

void Packer::EntryJob::onFinish()
{
	_entry->_writer = NULL;

	_writer = NULL;
}

////////////////////////////////////////////////////////////////////////////////

ZBundleBuildJob::ZBundleBuildJob(Ref<StreamSource> bundle, Ref<Settings> bundleCfg, bool bigEndian, Ref<Archive> outPath, Ref<MemoryBuffer> extData)
{
	_bundle	= bundle;
	_bundleCfg = bundleCfg;
	_bigEndian = bigEndian;
	_outPath	= outPath;
	_extData	= extData;
}

bool ZBundleBuildJob::onPrepare()
{
	_bundleUID = _bundleCfg->get("bundle/uid");
	_baseUID = _bundleCfg->get("bundle/base_uid");

	String ext;
	StringUtil::splitBaseFilename(_bundle->getName(), _bundleName, ext);

	_bundleCfgBuf = new MemoryBuffer();
	_bundleCfg->saveCfg(new MemoryBuffer::Writer(_bundleCfgBuf, NULL));

	return true;
}

bool ZBundleBuildJob::onExecute(bool async)
{
	LOG_TIMESCOPE(0, "++ Compressing bundle '%s'", _bundleName.c_str());

	// TODO: Do not allow app.cfg with different app_name or platform or app_version

	// Create a new zbundle file
	Ref<StreamWriter> w = _outPath->create(_bundleName + ".zbundle");

	bool flipEndian = _bigEndian;

	// Prepare header meta data
	PackBundle::ZBundleSigHeader sig = { 0 };
	PackBundle::ZBundleHeader hdr = { 0  };

	sig.signature	= NIT_ZBUNDLE_SIGNATURE;
	sig.version		= NIT_ZBUNDLE_VERSION;

	hdr.flags			= 0;
	hdr.uidLen			= _bundleUID.length();
	hdr.baseUidLen		= _baseUID.length();
	hdr.bundleCfgSize	= _bundleCfgBuf->getSize();
	hdr.extDataSize		= _extData ? _extData->getSize() : 0;

	// Placeholder for header meta data
	w->skip(sizeof(sig));
	w->skip(sizeof(hdr));
	w->skip(hdr.uidLen);
	w->skip(hdr.baseUidLen);
	w->skip(hdr.bundleCfgSize);
	w->skip(hdr.extDataSize);

	uint64 payloadBegin = w->tell();

	// Compress the bundle with ZStream

	Ref<CalcCRC32Writer> cw = new CalcCRC32Writer();
	Ref<ShadowWriter> sw = new ShadowWriter(w, cw);

	Ref<ZStreamWriter> zw = new ZStreamWriter(sw);
	zw->copy(_bundle->open());

	zw->finish();

	uint32 payloadCRC32 = cw->getValue();
	_sourceHash = _bundle->calcCrc32();

	uint64 payloadEnd = w->tell();

	// Rewind and write missing header meta data
	hdr.sourceSize		= _bundle->getStreamSize();
	hdr.payloadType		= PackArchive::PAYLOAD_ZLIB;
	hdr.payloadSize		= payloadEnd - payloadBegin;
	hdr.payloadCRC32	= payloadCRC32;
	hdr.payloadParam0	= 0;
	hdr.payloadParam1	= 0;

	w->seek(0);

	if (flipEndian) sig.flipEndian();
	if (flipEndian) hdr.flipEndian();

	w->writeRaw(&sig, sizeof(sig));
	w->writeRaw(&hdr, sizeof(hdr));
	w->writeRaw(_bundleUID.c_str(), hdr.uidLen);
	w->writeRaw(_baseUID.c_str(), hdr.baseUidLen);
	_bundleCfgBuf->save(w);
	if (_extData)
		_extData->save(w);

	return true;
}

void ZBundleBuildJob::onFinish()
{
	// Link result file
	_output = _outPath->locate(_bundleName + ".zbundle");

	// Generate informative file for service management team
	_bundleCfg->add("hash", StringUtil::format("%08X", _sourceHash)); // Hash isn't included in bundle, but here we report
	_bundleCfg->saveCfg(_outPath->create(_bundleName + ".bundle.cfg"));
	_bundleCfg->saveCfg(new LogWriter()); // Dump the cfg content to log also
}

////////////////////////////////////////////////////////////////////////////////

// Nit Bundle Diff Implementation

static bool isDifferent(PackArchive::File* a, PackArchive::File* b, bool checkStream)
{
	if (a->getContentType() != b->getContentType())		return true;

	if (!checkStream)
	{
		// Compare just meta data
		if (a->getStreamSize()		!= b->getStreamSize())		return true;
		if (a->getPayloadType()		!= b->getPayloadType())		return true;
		if (a->getPayloadCRC32()	!= b->getPayloadCRC32())	return true;
		if (a->getPayloadParam0()	!= b->getPayloadParam0())	return true;
		if (a->getPayloadParam1()	!= b->getPayloadParam1())	return true;
		if (a->getSourceSize()		!= b->getSourceSize())		return true;
		if (a->getSourceCRC32()		!= b->getSourceCRC32())		return true;

		return false;
	}

	// Start the file comparison.
	uint8 buf_a[4096];
	uint8 buf_b[4096];

	Ref<StreamReader> ra = a->open();
	Ref<StreamReader> rb = b->open();

	bool metaDifferent = isDifferent(a, b, false);
	bool different = false;

	while (true)
	{
		size_t read_a = ra->readRaw(buf_a, sizeof(buf_a));
		size_t read_b = rb->readRaw(buf_b, sizeof(buf_b));

		if (read_a != read_b)
		{
			different = true;
			break;
		}

		if (read_a == 0)
			break;

		if (memcmp(buf_a, buf_b, read_a) != 0)
		{
			different = true;
			break;
		}
	}

	// File comparison completed

	if (metaDifferent != different)
	{
		LOG(0, "*** meta diff (%d) != file diff (%d) : %s\n",
			metaDifferent, different, a->getUrl().c_str());
	}

	return different;
}

BundleDiffJob::BundleDiffJob(Ref<StreamSource> topZBundle, Ref<StreamSource> baseZBundle, Ref<Archive> outPath)
{
	_topZBundle = topZBundle;
	_baseZBundle = baseZBundle;
	_outPath = outPath;
}

void BundleDiffJob::testDiff(Package* basePack, Package* topPack, DiffResult& result, bool checkStream)
{
	StreamSourceMap topFiles, baseFiles;
	topPack->find("*", topFiles);
	basePack->find("*", baseFiles);

	for (StreamSourceMap::iterator itr = topFiles.begin(), end = topFiles.end(); itr != end; ++itr)
	{
		PackArchive::File* topFile = dynamic_cast<PackArchive::File*>(itr->second.get());
		assert(topFile);

		StreamSourceMap::iterator bf = baseFiles.find(itr->first);
		PackArchive::File* baseFile = bf != baseFiles.end() ? dynamic_cast<PackArchive::File*>(bf->second.get()) : NULL;

		if (topFile->getName() == "pack.cfg")
		{
			// pack.cfg should be always there
			result.insert(std::make_pair(topPack, DiffEntry(DiffEntry::MODIFIED, topFile)));
			continue;
		}

		if (baseFile == NULL || baseFile->getContentType() == ContentType::DELETED)
		{
			result.insert(std::make_pair(topPack, DiffEntry(DiffEntry::ADDED, topFile)));
			continue;
		}

		if (isDifferent(baseFile, topFile, checkStream))
		{
			result.insert(std::make_pair(topPack, DiffEntry(DiffEntry::MODIFIED, topFile)));
			continue;
		}
	}

	// Compare top to base, detect deletion
	for (StreamSourceMap::iterator itr = baseFiles.begin(), end = baseFiles.end(); itr != end; ++itr)
	{
		PackArchive::File* baseFile = dynamic_cast<PackArchive::File*>(itr->second.get());
		assert(baseFile);

		if (baseFile->getContentType() == ContentType::DELETED)
			continue;

		StreamSourceMap::iterator tf = topFiles.find(itr->first);

		if (tf == topFiles.end())
		{
			result.insert(std::make_pair(topPack, DiffEntry(DiffEntry::DELETED, baseFile)));
			continue;
		}
	}
}

void BundleDiffJob::testDiff(PackBundle* baseBundle, PackBundle* topBundle, DiffResult& result, bool checkStream)
{
	StreamSourceMap topFiles, baseFiles;
	topBundle->find("*", topFiles);
	baseBundle->find("*", baseFiles);

	for (StreamSourceMap::iterator itr = topFiles.begin(), end = topFiles.end(); itr != end; ++itr)
	{
		PackArchive::File* topFile = dynamic_cast<PackArchive::File*>(itr->second.get());
		assert(topFile);

		StreamSourceMap::iterator bf = baseFiles.find(itr->first);
		PackArchive::File* baseFile = bf != baseFiles.end() ? dynamic_cast<PackArchive::File*>(bf->second.get()) : NULL;

		if (baseFile == NULL || baseFile->getContentType() == ContentType::DELETED)
		{
			result.insert(std::make_pair(topBundle, DiffEntry(DiffEntry::ADDED, topFile)));
			continue;
		}

		if (baseFile->getContentType() == ContentType::ARCHIVE_PACK && topFile->getContentType() == ContentType::ARCHIVE_PACK)
		{
			Ref<Package> basePack = baseBundle->link(StreamSource::stripExtension(baseFile->getName()));
			Ref<Package> topPack = topBundle->link(StreamSource::stripExtension(topFile->getName()));
			testDiff(basePack, topPack, result, checkStream);
			continue;
		}

		if (isDifferent(baseFile, topFile, checkStream))
		{
			result.insert(std::make_pair(topBundle, DiffEntry(DiffEntry::MODIFIED, topFile)));
			continue;
		}
	}

	// Compare top to base, detect deletion
	for (StreamSourceMap::iterator itr = baseFiles.begin(), end = baseFiles.end(); itr != end; ++itr)
	{
		PackArchive::File* baseFile = dynamic_cast<PackArchive::File*>(itr->second.get());
		assert(baseFile);

		if (baseFile->getContentType() == ContentType::DELETED)
			continue;

		StreamSourceMap::iterator tf = topFiles.find(itr->first);

		if (tf == topFiles.end())
		{
			result.insert(std::make_pair(topBundle, DiffEntry(DiffEntry::DELETED, baseFile)));
			continue;
		}
	}
}

static Ref<PackBundle::ZBundleInfo> UnpackAndLink(Ref<StreamSource> zbundle, Ref<FileLocator> tempPath, Ref<PackBundle>& outBundle)
{
	String bundleFilename = zbundle->getName() + ".bundle";
	Ref<StreamWriter> unpackWriter = tempPath->create(bundleFilename);
	Ref<PackBundle::ZBundleInfo> info = PackBundle::unpackZBundle(zbundle->open(), unpackWriter);
	unpackWriter = NULL;

	Ref<PackBundle> baseBundle;

	if (!info->getBaseUid().empty())
	{
		// If a base needs another base, link them also (Fails if any missing link)
		Ref<StreamSource> baseZBundle = zbundle->getLocator()->locate(info->getBaseUid() + ".zbundle");
		Ref<PackBundle::ZBundleInfo> baseInfo = UnpackAndLink(baseZBundle, tempPath, baseBundle);
	}

	outBundle = new PackBundle(tempPath->locate(bundleFilename), baseBundle);

	return info;
}

bool BundleDiffJob::onPrepare()
{
	return true;
};

bool BundleDiffJob::onExecute(bool async)
{
	String tempPathStr = NitRuntime::getSingleton()->getSysTempPath() + "/bundler";

	FileUtil::createDir(tempPathStr);

	Ref<FileLocator> tempPath = 
		new FileLocator("$temp", tempPathStr, false);

	// unpack base
	_baseInfo = UnpackAndLink(_baseZBundle, tempPath, _baseBundle);

	// unpack top
	_topInfo = UnpackAndLink(_topZBundle, tempPath, _topBundle);

	_baseAppCfg = Settings::load(_baseBundle->locate("app.cfg"));
	_topAppCfg = Settings::load(_topBundle->locate("app.cfg"));

	_baseBundleCfg = Settings::load(_baseBundle->locate("bundle.cfg"));
	_topBundleCfg = Settings::load(_topBundle->locate("bundle.cfg"));

	// TODO: Check if same value of app_version, etc

	// Start the package comparison
	testDiff(_baseBundle, _topBundle, _result, true);

	return true;
}

void BundleDiffJob::onFinish()
{
	// Dump diff result
	for (DiffResult::iterator itr = _result.begin(), end = _result.end(); itr != end; ++itr)
	{
		StreamLocator* top = itr->first;
		DiffEntry& entry = itr->second;

		PackArchive::File* file = entry.file;

		LOG(0, "-- %s '%s' : '%s'\n", entry.StatusStr(), top->getName().c_str(), file->getName().c_str());
	}
}

////////////////////////////////////////////////////////////////////////////////

DeltaBuildJob::DeltaBuildJob(Ref<StreamSource> topZBundle, Ref<StreamSource> baseZBundle, Ref<Archive> outPath)
{
	_topZBundle = topZBundle;
	_baseZBundle = baseZBundle;
	_outPath = outPath;
}

bool DeltaBuildJob::onPrepare()
{
	_bundleDiffJob = new BundleDiffJob(_topZBundle, _baseZBundle, _outPath);
	enqueueSubJob(_bundleDiffJob);

	return true;
}

bool DeltaBuildJob::onExecute(bool async)
{
	// NO-OP until sub jobs completed
	return true;
}

void DeltaBuildJob::onSubJobFinished(AsyncJob* subJob, Status status)
{
	if (subJob == _bundleDiffJob)
	{
		PackBundle* topBundle = _bundleDiffJob->getTopBundle();
		Settings* topBundleCfg = _bundleDiffJob->getTopBundleCfg();
		Settings* topAppCfg = _bundleDiffJob->getTopAppCfg();
		PackBundle::ZBundleInfo* topInfo = _bundleDiffJob->getTopInfo();
		Settings* baseBundleCfg = _bundleDiffJob->getBaseBundleCfg();
		PackBundle::ZBundleInfo* baseInfo = _bundleDiffJob->getBaseInfo();

		// Unify platform-code and endianness by top bundle
		uint32 platformCode = topBundle->getPack()->getHeader().target;
		_bigEndian = topBundle->getPack()->isEndianFlip();

		// Prepare new bundle
		DateTime dt = DateTime::now();

		String buildHost = g_Bundler->getBuildHost();

		String newUID = "delta_";
		newUID.append(topAppCfg->get("app_name"));
		newUID.push_back('_');
		newUID.append(topBundleCfg->get("bundle/platform"));
		newUID.push_back('_');
		newUID.append(topBundleCfg->get("bundle/build_target"));
		newUID.push_back('_');
		newUID.append(baseBundleCfg->get("bundle/revision"));
		newUID.push_back('_');
		newUID.append(topBundleCfg->get("bundle/revision"));
		newUID.push_back('_');
		newUID.append(buildHost);
		newUID.push_back('_');
		newUID.append(dt.format("%Y%m%d_%H%M%S"));

		StringUtil::toLowerCase(newUID);

		_newBundleName = newUID;

		StringUtil::toLowerCase(_newBundleName);

		_bundlePacker = new bundler::Packer(_newBundleName, _newBundleName + ".bundle", platformCode, _bigEndian);

		// Assign new packs
		typedef map<Ref<StreamLocator>, Ref<bundler::Packer> >::type Packers;
		Packers packers;

		BundleDiffJob::DiffResult& result = _bundleDiffJob->getDiffResult();

		for (BundleDiffJob::DiffResult::iterator itr = result.begin(), end = result.end(); itr != end; ++itr)
		{
			Ref<bundler::Packer> packer;

			StreamLocator* pack = itr->first;
			BundleDiffJob::DiffEntry& entry = itr->second;

			if (pack == topBundle) // A bundle file needs special consideration
			{
				if (entry.file->getName() == "bundle.cfg")
				{
					// We'll make a new bundle.cfg so ignore it
					continue;
				}
				packer = _bundlePacker;
			}
			else
			{
				Packers::iterator pf = packers.find(pack);
				if (pf == packers.end())
				{
					packer = new bundler::Packer(pack->getName(), pack->getName() + ".pack", platformCode, _bigEndian);
					packers.insert(std::make_pair(pack, packer));
				}
				else
					packer = pf->second;
			}

			if (entry.status == BundleDiffJob::DiffEntry::DELETED)
				packer->assign(entry.file->getName(), NULL, new bundler::DeletedEntryHandler());
			else
				packer->assign(entry.file->getName(), NULL, new bundler::DiffEntryHandler(entry.file));
		}

		// Build prepared packs
		for (Packers::iterator itr = packers.begin(), end = packers.end(); itr != end; ++itr)
		{
			bundler::Packer* packer = itr->second;

			// HACK: Use memory source for now
			Ref<MemorySource> memPack = new MemorySource(packer->getFilename());
			Ref<StreamWriter> packWriter = memPack->newWriter();

			_bundlePacker->assign(memPack->getName(), memPack, new bundler::CopyHandler());

			enqueueSubJob(packer->newJob("*", true, packWriter));
		}

		// Modify the bundle.cfg and embed it
		topBundleCfg->set("bundle/uid", newUID);
		topBundleCfg->set("bundle/base_uid", baseInfo->getUid());

		topBundleCfg->set("bundle_top/uid", topInfo->getUid());
		topBundleCfg->set("bundle_top/build_date", topBundleCfg->get("bundle/build_date"));
		topBundleCfg->set("bundle_top/build_time", topBundleCfg->get("bundle/build_time"));
		topBundleCfg->set("bundle_top/build_host", topBundleCfg->get("bundle/build_host"));

		topBundleCfg->set("bundle/build_host", buildHost);
		topBundleCfg->set("bundle/build_date", dt.format("%Y-%m-%d"));
		topBundleCfg->set("bundle/build_time", dt.format("%H:%M:%S"));

		Ref<MemorySource> bundleCfgSrc = new MemorySource("bundle.cfg");
		topBundleCfg->saveCfg(bundleCfgSrc->newWriter());

		Ref<bundler::Handler> hdlr = _bundlePacker->assign("bundle.cfg", bundleCfgSrc, new bundler::CopyHandler());
		hdlr->setPayload("zlib");
	}
	else if (getSubJobCount() == 0 && _bundlePackJob == NULL)
	{
		// Enqueue a new bundle build job
		Ref<StreamWriter> bundleWriter = _outPath->create(_newBundleName + ".bundle");

		_bundlePackJob = _bundlePacker->newJob("*", true, bundleWriter);
		enqueueSubJob(_bundlePackJob);
	}

	if (subJob == _bundlePackJob)
	{
		Settings* topBundleCfg = _bundleDiffJob->getTopBundleCfg();
		PackBundle::ZBundleInfo* topInfo = _bundleDiffJob->getTopInfo();

		// Build a ZBundle
		Ref<MemorySource> extDataSrc;

		if (topInfo->getExtData())
		{
			extDataSrc = new MemorySource("extdata");
			extDataSrc->getBuffer()->load(topInfo->getExtData()->open());
		}

		_zBundleBuildJob = new ZBundleBuildJob(
 			_outPath->locate(_newBundleName + ".bundle"),
 			topBundleCfg,
 			_bigEndian,
 			_outPath,
 			extDataSrc ? extDataSrc->getBuffer() : NULL);

		enqueueSubJob(_zBundleBuildJob);
	}
}

void DeltaBuildJob::onFinish()
{
	if (getSubJobCount() > 0)
		return retry(true);
}

StreamSource* DeltaBuildJob::getOutput()
{
	return _zBundleBuildJob ? _zBundleBuildJob->getOutput() : NULL;
}

////////////////////////////////////////////////////////////////////////////////

NS_BUNDLER_END;
