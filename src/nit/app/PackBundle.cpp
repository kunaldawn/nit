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

#include "nit/app/PackBundle.h"
#include "nit/app/PackArchive.h"
#include "nit/app/AppConfig.h"

#include "nit/data/Settings.h"
#include "nit/io/MemoryBuffer.h"
#include "nit/io/ZStream.h"

NS_NIT_BEGIN;

// Only one pack path for valid PackBundle over an app!

////////////////////////////////////////////////////////////////////////////////

PackBundle::PackBundle(Archive* bundleRoot, const String& uid)
: StreamLocator(uid)
{
	_active = false;
	_valid = true;
	_uid = uid;
	// TODO: add appVersion to compats?

	_locator = bundleRoot;
}

PackBundle::PackBundle(StreamSource* bundleFile, PackBundle* base)
: StreamLocator(bundleFile->getName())
{
	_base = base;

	_active = false;
	_realFile = dynamic_cast<File*>(bundleFile);

	if (bundleFile)
	{
		_pack = new PackArchive(_name, bundleFile);
		_locator = _pack;
	}

	_valid = loadCfg();
}

PackBundle::~PackBundle()
{
	if (_pack)
		_pack->unload();
}

StreamSource* PackBundle::locateLocal(const String& streamName)
{
	StreamSource* source = NULL;
	
	if (_locator)
		source = _locator->locate(streamName, false);

	if (source == NULL && _base)
		source = _base->locateLocal(streamName);

	return source;
}

void PackBundle::findLocal(const String& pattern, StreamSourceMap& varResults)
{
	if (_locator)
		_locator->find(pattern, varResults);

	if (_base)
		_base->findLocal(pattern, varResults);
}

static void _CollectRecursive(StreamLocator* locator, StreamSourceMap& varResults)
{
	FileLocator* fl = dynamic_cast<FileLocator*>(locator);

	if (fl)
	{
		fl->findFiles("pack.cfg", varResults, true);
		fl->findFiles("*.pack.cfg", varResults, true);
	}

	const StreamLocatorList& ll = locator->getRequiredList();

	for (uint i=0; i < ll.size(); ++i)
		_CollectRecursive(ll[i], varResults);
}

void PackBundle::collectPackCfgs(StreamSourceMap& varResults)
{
	if (_locator)
	{
		_CollectRecursive(_locator, varResults);
	}

	if (_base)
		_base->collectPackCfgs(varResults);
}

Package* PackBundle::link(const String& packName)
{
	Package* pack = linkLocal(packName);

	// Treat package which are marked DELEteD as NULL 
	if (pack && pack->getType() == Package::PK_DELETED)
	{
		Ref<Package> safe = pack;
		return NULL;
	}

	Package* basePack = NULL;

	if (_base)
		basePack = _base->link(packName);

	if (pack)
	{
		// link base pack
		pack->_base = basePack;

		if (basePack && !basePack->hasProxy())
			basePack->setProxy(pack);

		if (pack->_archive && !pack->_archive->hasProxy())
			pack->_archive->setProxy(pack);
	}
	else
	{
		// the pack is not on the current bundle. so return the base pack
		pack = basePack;
	}

	return pack;
}

Package* PackBundle::linkLocal(const String& packName)
{
	String name = packName;

	String path;
	Package::Type type;
	Ref<StreamSource> source;

	// find plugin if '.plugin'
	static String pluginExt = ".plugin";

	// NOTE: static plugins will not reach here because they are already registered on PackageService

	if (StringUtil::endsWith(name, pluginExt))
	{
		type = Package::PK_PLUGIN;
		// replace *.plugin -> *.dll
		path = name.substr(0, name.length() - pluginExt.length());

#if defined(NIT_DEBUG)
		path += "_d." NIT_DLL_EXTENSION;
#elif defined(NIT_FASTDEBUG)
		path += "_fd." NIT_DLL_EXTENSION;
#else
		path += "." NIT_DLL_EXTENSION;
#endif

		source = locateLocal(path);
		if (source)
			goto found;
	}

	// find '<root>/name.pack'
	type = Package::PK_PACKFILE;
	path = name + ".pack";
	source = locateLocal(path);

	if (source)
		goto found;

	// find '<root>/name/pack.cfg'
	type = Package::PK_FOLDER;
	path = name + "/pack.cfg";
	source = locateLocal(path);
	if (source)
		goto found;

	// find '<root>/name.pack.cfg'
	type = Package::PK_ALIAS;
	path = name + ".pack.cfg";
	source = locateLocal(path);
	if (source)
		goto found;

	// not found, return NULL
	return NULL;

found:
	if (source->getContentType() == ContentType::DELETED)
	{
		Package* deleted = new Package(packName, NULL);
		deleted->_type = Package::PK_DELETED;
		deleted->_bundle = this;
		return deleted;
	}

	String fullpath = source->getUrl();

	String location;
	String basepath;
	String cfgFilename;

	NitRuntime* rt = NitRuntime::getSingleton();

	Ref<Archive> archive = NULL;

	switch (type)
	{
	case Package::PK_PACKFILE: // name.pack
		StringUtil::splitFilename(fullpath, location, basepath); // this location is dummy and ignored 
		archive = new PackArchive(packName, source);
		location = fullpath;
		cfgFilename = "pack.cfg";
		break;

	case Package::PK_PLUGIN: // name.dll
		StringUtil::splitFilename(fullpath, location, basepath);
		archive = new PluginDLL(name, fullpath);
		location = fullpath;
		cfgFilename = "";
		break;

	case Package::PK_FOLDER: // name/pack.cfg
		StringUtil::splitFilename(fullpath, cfgFilename, basepath);
		archive = rt->openArchive(name, basepath);
		location = basepath;
		break;

	case Package::PK_ALIAS: // name.pack.cfg
		StringUtil::splitFilename(fullpath, cfgFilename, basepath);
		archive = rt->openArchive(name, basepath);
		location = basepath;
		break;

	default:
		NIT_THROW(EX_NOT_SUPPORTED);
		break;
	}

	if (archive)
		archive->load();

	Ref<StreamSource> settingsSource;

	if (!cfgFilename.empty())
	{
		settingsSource = archive->locate(cfgFilename, false);

		if (settingsSource == NULL)
		{
			LOG(0, "*** cannot open '%s' on '%s'\n", cfgFilename.c_str(), location.c_str());
			return NULL;
		}
	}

	// A package of PK_ALIAS does not contains an archive (no file its own, only use required packages)
	Archive* packageArchive = archive; 
	if (type == Package::PK_ALIAS)
		packageArchive = NULL; // if we set 'archive' not 'packageArchive' here, release side-effect may occur.

	Package* pack = new Package(name, packageArchive);

	pack->_type = type;
	pack->_bundle = this;
	pack->_settingsSource = settingsSource;

	return pack;
}

bool PackBundle::isCompatible(const String& version)
{
	return std::find(_compats.begin(), _compats.end(), version) != _compats.end();
}

static int IntRevision(const String& revStr)
{
	if (revStr.empty())
		return 0;

	// Revisions should be composed by pure numerical digits. (ex 9894)
	// But develop builds can have them as '9894M' or '9894T10237' ('svnversion' behavior over local working-copy changes)
	// - 9894M : treat as 9894
	// - 9894T10237 : maybe 10237 on the repository, but 9894 on the local working-copy. So treat as 9894
	// Above the two case, only front valid digits are on the concern and we safely drop after the non-digit characters.
	// So atoi() can be used safely.

	return atoi(revStr.c_str());
}

bool PackBundle::isRecentThan(PackBundle* other)
{
	int thisRev = IntRevision(this->_revision);
	int otherRev = IntRevision(other->_revision);

	return otherRev < thisRev;
}

bool PackBundle::loadCfg()
{
	Ref<StreamSource> cfgFile = locate("bundle.cfg", false);
	if (cfgFile == NULL)
	{
		LOG(0, "*** can't find '%s'\n", makeUrl("bundle.cfg").c_str());
		return false;
	}

	Ref<Settings> bundleCfg = Settings::load(cfgFile);

	Ref<Settings> sect = bundleCfg->getSection("bundle");
	if (sect == NULL) 
	{
		LOG(0, "*** '%s' has no [bundle] section\n", cfgFile->getUrl().c_str());
		return false;
	}

	_uid = sect->get("uid");
	_baseUid = sect->get("base_uid");
	_revision = sect->get("revision");
	sect->find("app_compat_ver", _compats);

	if (_uid.empty()) return false;
	if (_revision.empty()) return false;

	if (_compats.empty())
	{
		LOG(0, "*** '%s' has no app_compat_ver\n", cfgFile->getUrl().c_str());
		return false;
	}

	return true;
}

Ref<PackBundle> PackBundle::linkBundles(AppConfig* appConfig)
{
	LOG_TIMESCOPE(0, "++ Linking bundles from '%s'", appConfig->getSettings()->getUrl().c_str());

	// We decided bundler's builder do not consider app.cfg with different app_versions.
	const String& appVersion = appConfig->getVersion(); 

	NitRuntime* rt = NitRuntime::getSingleton();

	// Create root bundle - for plugin access
	Ref<Archive> pluginPath = rt->openArchive("$plugin_path", appConfig->getPluginPath());
	if (dynamic_cast<FileLocator*>(pluginPath.get()))
	{
		((FileLocator*)pluginPath.get())->addFiltered("*." NIT_DLL_EXTENSION);
	}

	Ref<PackBundle> root = new PackBundle(pluginPath, "$root_bundle");
	root->_compats.push_back(appVersion);

	BundleUIDLookup bundles;

	// find from patch_path first, and if not found then find app_bundle_path (reversed to registration order)
	Ref<Archive> bundleLocator = rt->openArchive("$app_bundle_path", appConfig->getBundlePath());

	// Make a new one when there's no patch path
	try
	{
		LOG(0, "++ $patch_path set to '%s'\n", appConfig->getPatchPath().c_str());
		FileUtil::createDir(appConfig->getPatchPath());
		bundleLocator->require(rt->openArchive("$patch_path", appConfig->getPatchPath()));
	}
	catch (Exception& ex)
	{
		rt->alert("Can't use patch path", ex.getFullDescription());
	}

	StreamSourceMap blist;
	bundleLocator->find("*.bundle", blist);

#if defined(NIT_ANDROID)
	// HACK: For Android SDK 4.0.3, there's no way to direct aapt not to compress a file,
	// We use 'rtttl' extension as workaround (rtttl files are not compressed by aapt default)
	bundleLocator->find("*.bundle.rtttl", blist);
#endif

	for (StreamSourceMap::iterator itr = blist.begin(), end = blist.end(); itr != end; ++itr)
	{
		Ref<PackBundle> bundle;
		try {
			bundle = new PackBundle(itr->second);
		}
		catch(Exception& ex)
		{
			LOG(0, "*** Can't open bundle '%s': %s\n", itr->second->getUrl().c_str(), ex.getFullDescription().c_str());
		}
		
		if (bundle == NULL || !bundle->isValid())
		{
			LOG(0, "*** invalid bundle: '%s'\n", itr->second->getUrl().c_str());
			// TODO: How do we handle this case?? erase them all?
			continue;
		}

		if (!bundle->isCompatible(appVersion))
		{
			// Ignore a non-compatible
			LOG(0, "*** '%s' not compatible with this app version %s\n",
				bundle->getUid().c_str(), appVersion.c_str());
			continue;
		}

		if (bundles.find(bundle->getUid()) != bundles.end())
		{
			// TODO: Same uid reoccurred - recent version first
		}

		bundles.insert(std::make_pair(bundle->getUid(), bundle));
	}

	// Link bundles
	for (BundleUIDLookup::iterator itr = bundles.begin(), end = bundles.end(); itr != end; ++itr)
	{
		PackBundle* bundle = itr->second;

		// TODO: handle duplicated entry on bundle list: bundle->_base already exists?
		if (bundle->_base)
			continue;

		const String& baseUID = bundle->getBaseUid();

		Ref<PackBundle> base = NULL;

		// A bundle with no base-uid will be treated as app-bundle
		if (baseUID.empty())
			base = root;
		else
		{
			BundleUIDLookup::iterator f = bundles.find(baseUID);
			if (f != bundles.end())
				base = f->second;
		}

		bundle->_base = base;

		if (base)
		{
			PackBundle* other = static_cast<PackBundle*>(base->_proxy.get());
			if (other == NULL || bundle->isRecentThan(other))
			{
				// Use the bundle of recent revision
				base->_proxy = bundle;
			}
		}
	}

	// active path decided, so determine the top-bundle
	Ref<PackBundle> topBundle = static_cast<PackBundle*>(root->getProxy());

	// Forge a dev-bundle if we could not find appropriate top-bundle
	if (topBundle == root)
	{
#if defined(NIT_SHIPPING)
		NitRuntime::getSingleton()->
			alert("Can't find top bundle", "Did you forget to add your .bundle file into bundles folder?", true);
#else
		LOG(0, "?? can't find top bundle, creating dev bundle\n");
#endif

		const String& pathlist = appConfig->get("dev_pack_path");

		Ref<Archive> devPackPath;

		if (!pathlist.empty())
		{
			StringVector paths = StringUtil::tokenise(pathlist, ";");
			for (uint i = 0; i < paths.size(); ++i)
			{
				String path = paths[i];
				StringUtil::trim(path);
				if (path.empty()) continue;

				Ref<Archive> fl = rt->openArchive("$dev_pack_path", path);

				if (devPackPath == NULL)
					devPackPath = fl;
				else
					devPackPath->require(fl);

				LOG(0, "-- '%s' mounted to $dev_bundle'\n", fl->makeUrl("").c_str());
			}
		}

		Ref<PackBundle> devBundle = new PackBundle(devPackPath, "$dev_bundle");
		devBundle->_compats.push_back(appVersion);
		devBundle->_revision = "<wc>";

		root->_proxy = devBundle;
		devBundle->_base = root;

		// Make dev-bundle as top bundle
		topBundle = devBundle;
	}

	// Set active flag on bundle of active path
	for (PackBundle* b = topBundle; b != NULL; b = b->_base)
	{
		b->_active = true;
		PackBundle* base = b->getBase();
		if (base)
			LOG(0, "++ bundle '%s' based on '%s' active\n", b->getUid().c_str(), base->getUid().c_str());
		else
			LOG(0, "++ bundle '%s' active\n", b->getUid().c_str());
	}

	// Log about ignored bundles
	for (BundleUIDLookup::iterator itr = bundles.begin(), end = bundles.end(); itr != end; ++itr)
	{
		PackBundle* bundle = itr->second;

		if (!bundle->isActive())
		{
			LOG(0, "*** bundle '%s' inactive\n", bundle->getUid().c_str());
		}
	}

	// Remove all bundles which has no ref and thus be ignored
	bundles.clear(); 

	return topBundle;
}

Ref<PackBundle::ZBundleInfo> PackBundle::unpackZBundle(Ref<StreamReader> r, Ref<StreamWriter> w)
{
	LOG(0, ".. sizeof(ZBundleHeader): %d\n", sizeof(ZBundleHeader));
	
	ZBundleSigHeader sig = { 0 };
	r->readRaw(&sig, sizeof(sig));

	bool flipEndian = false;

	if (sig.signature == NIT_ZBUNDLE_SIGNATURE_FLIP)
	{
		flipEndian = true;
		sig.flipEndian();
	}

	if (sig.signature != NIT_ZBUNDLE_SIGNATURE)
		NIT_THROW_FMT(EX_CORRUPTED, "Invalid zbundle signature: %s", r->getUrl().c_str());

	if (sig.version != NIT_ZBUNDLE_VERSION)
		NIT_THROW_FMT(EX_CORRUPTED, "Invalid zbundle version: %s", r->getUrl().c_str());

	ZBundleHeader hdr = { 0 };
	if (r->readRaw(&hdr, sizeof(hdr)) != sizeof(hdr))
		NIT_THROW_FMT(EX_CORRUPTED, "Truncated zbundle header: %s", r->getUrl().c_str());

	if (hdr.payloadType != PackArchive::PAYLOAD_ZLIB && hdr.payloadType != PackArchive::PAYLOAD_ZLIB)
		NIT_THROW_FMT(EX_NOT_SUPPORTED, "Not supported payload type: %s", r->getUrl().c_str());

	Ref<ZBundleInfo> info = new ZBundleInfo;
	info->_header = hdr;

	Ref<MemoryBuffer> temp = new MemoryBuffer();

	if (hdr.uidLen)
	{
		temp->clear();
		temp->load(r, 0, hdr.uidLen);
		info->_uid = temp->toString();
	}

	if (hdr.baseUidLen)
	{
		temp->clear();
		temp->load(r, 0, hdr.baseUidLen);
		info->_baseUid = temp->toString();
	}

	if (hdr.bundleCfgSize)
	{
		temp = new MemoryBuffer();
		temp->load(r, 0, hdr.bundleCfgSize);
		info->_bundleCfg = new MemorySource("bundle.cfg", temp);
	}

	if (hdr.extDataSize)
	{
		temp = new MemoryBuffer();
		temp->load(r, 0, hdr.extDataSize);
		info->_extData = new MemorySource("bundle.extdata", temp);
	}

	// Header completed to this point

	// Calculate Payload CRC32 (zstream will go crazy if file broken)
	uint64 payloadBegin = r->tell();
	uint32 crc = StreamUtil::calcCrc32(r);

	if (crc != hdr.payloadCRC32)
		NIT_THROW_FMT(EX_CORRUPTED, "payload crc32 mismatch: %s", r->getUrl().c_str());

	r->seek((size_t)payloadBegin); // TODO: support uint64

	// start decompression
	Ref<ZStreamReader> zr = new ZStreamReader(r);
	w->copy(zr);

	uint64 payloadEnd = r->tell();
	uint64 payloadSize = payloadEnd - payloadBegin;

	if (payloadSize != hdr.payloadSize)
		NIT_THROW_FMT(EX_CORRUPTED, "payload size mismatch: %s", r->getUrl().c_str());

	return info;
}

////////////////////////////////////////////////////////////////////////////////

void PackBundle::ZBundleSigHeader::flipEndian()
{
	StreamUtil::flipEndian(signature);
	StreamUtil::flipEndian(version);
}

void PackBundle::ZBundleHeader::flipEndian()
{
	StreamUtil::flipEndian(payloadType);
	StreamUtil::flipEndian(flags);
	StreamUtil::flipEndian(uidLen);
	StreamUtil::flipEndian(baseUidLen);
	StreamUtil::flipEndian(bundleCfgSize);
	StreamUtil::flipEndian(extDataSize);
	StreamUtil::flipEndian(payloadSize);
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
