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

#include "nit/app/Package.h"

#include "nit/app/PackageService.h"
#include "nit/app/AppBase.h"
#include "nit/app/Session.h"
#include "nit/app/SessionService.h"
#include "nit/app/Plugin.h"

#include "nit/content/Content.h"
#include "nit/content/ContentManager.h"
#include "nit/content/ContentsService.h"

#include "nit/data/Settings.h"
#include "nit/data/Database.h"
#include "nit/io/FileLocator.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NIT_EVENT_DEFINE(OnPackageLinked, PackageEvent);
NIT_EVENT_DEFINE(OnPackageLoaded, PackageEvent);
NIT_EVENT_DEFINE(OnPackageUnloaded, PackageEvent);
NIT_EVENT_DEFINE(OnPackageUnlinked, PackageEvent);

////////////////////////////////////////////////////////////////////////////////

Package::Package(const String& name, Archive* archive, Package* base)
: Archive(name)
{
	_service			= NULL;
	_archive			= archive;
	_base				= base;

	_linking			= false;
	_linked			= false;
	_loading			= false;
	_prepared			= false;
	_hurry				= false;
	_scriptOpening 	= false;
	_scriptReady		= false;
	_loaded			= false;
	_stayResident		= false;
	_stayForCurrent	= false;
	_stayForNext		= false;
	_allowSubDir		= false;
}

Package::~Package()
{
	LOG(0, ".. package '%s': destroyed\n", _name.c_str());
}

bool Package::link(Settings* ss)
{
	_linking = true;
	
	PluginDLL* dll = NULL;

	NitRuntime* rt = NitRuntime::getSingleton();

	if (ss)
	{
		StringVector requires; 
		ss->find("package/require", requires);
		ss->find(rt->getConfig()->expand("package/require@$(platform)"), requires);
		ss->find(rt->getConfig()->expand("package/require@$(platform)_$(build)"), requires);
		ss->find(rt->getConfig()->expand("package/require@$(build)"), requires);

		StringVector optionals; 
		ss->find("package/optional", optionals);
		ss->find(rt->getConfig()->expand("package/optional@$(platform)"), requires);
		ss->find(rt->getConfig()->expand("package/optional@$(platform)_$(build)"), optionals);
		ss->find(rt->getConfig()->expand("package/optional@$(build)"), optionals);

		StringVector excludes; 
		ss->find("package/exclude", excludes);
		ss->find(rt->getConfig()->expand("package/exclude@$(platform)"), requires);
		ss->find(rt->getConfig()->expand("package/exclude@$(platform)_$(build)"), excludes);
		ss->find(rt->getConfig()->expand("package/exclude@$(build)"), excludes);

		for (uint i=0; i<requires.size(); ++i)
		{
			String& pack = requires[i];

			if (std::find(excludes.begin(), excludes.end(), pack) != excludes.end())
				continue;

			Package* required = _service->link(pack.c_str());
			if (required == NULL)
			{
				LOG(0, "*** package '%s' can't link required package '%s'\n", _name.c_str(), pack.c_str());
				goto fail;
			}

			require(required);
		}

		for (uint i=0; i<optionals.size(); ++i)
		{
			String& pack = optionals[i];
			if (std::find(excludes.begin(), excludes.end(), pack) != excludes.end())
				continue;

			Package* optional = _service->link(pack.c_str(), true);
			if (optional)
				require(optional);
		}

		ss->find("package/postload", _postLoads);

		ss->find("script/require", _requiredScripts);
		ss->find("script/OnLoad", _onLoadScripts);
		ss->find("script/OnUnload", _onUnloadScripts);

		Ref<Settings> lookup = ss->getSection("lookup");
		if (lookup)
		{
			for (Settings::Iterator itr = lookup->begin(), end = lookup->end(); itr != end; ++itr)
			{
				String type = itr->first;
				String key = itr->second;
				String sub, entry;
				PackageService::splitLookupEntry(type, key, sub, entry);

				if (!entry.empty() && (sub.empty() || sub == "script"))
					_scriptSchemas.push_back(key);
				else
					_dataSchemas.push_back(key);
			}
		}
	}

	// Register plugin (native Module, SpawnClass now registered by the plugin)
	dll = dynamic_cast<PluginDLL*>(_archive.get());
	if (dll && !dll->link(this))
		goto fail;

	// Link completed
	_linking = false;
	_linked = true;

	if (ss)
	{
		LOG(0, ".. package '%s' linked from '%s'\n", _name.c_str(), ss->getUrl().c_str());
	}
	else
	{
		if (dll)
			LOG(0, ".. package '%s' linked from '%s'\n", _name.c_str(), dll->getPath().c_str());
		else if (_archive)
			LOG(0, ".. package '%s' linked from '%s'\n", _name.c_str(), _archive->getName().c_str());
		else
			LOG(0, ".. package '%s' linked\n", _name.c_str());
	}

	// Register as proxy of the archive
	if (_archive && !_archive->hasProxy())
		_archive->setProxy(this);

	return true;

fail:
	_linking = false;

	return false;
}

Ref<Content> Package::link(const String& streamName, bool throwEx)
{
	Content* content = locateLocalContent(streamName);

	if (content) return content;

	// Search from the required list
	for (StreamLocatorList::reverse_iterator itr = _required.rbegin(); itr != _required.rend(); ++itr)
	{
		// TODO: avoid dynamic_cast
		Package* req = dynamic_cast<Package*>(itr->get());
		content = req->locateLocalContent(streamName);
		if (content) return content;
	}

	StreamSource* stream = locate(streamName);

	Package* pack = dynamic_cast<Package*>(stream->getLocator());
	ASSERT_THROW(pack, EX_INVALID_STATE);

	return pack->createLocalContent(stream, throwEx);
}

Ref<Content> Package::link(const String& streamName, const String& locatorPattern, bool throwEx)
{
	Content* content = locateContent(streamName, locatorPattern);
	if (content) return content;

	StreamSource* stream = locate(streamName, locatorPattern);

	Package* pack = dynamic_cast<Package*>(stream->getLocator());
	ASSERT_THROW(pack, EX_INVALID_STATE);

	return pack->createLocalContent(stream, throwEx);
}

Ref<Content> Package::linkLocal(const String& streamName)
{
	Content* content = locateLocalContent(streamName);

	if (content) return content;

	StreamSource* stream = locateLocal(streamName);
	if (stream == NULL) return NULL;

	return createLocalContent(stream, false);
}

Ref<ContentBase> Package::linkContent(StreamSource* stream)
{
	// StreamLocator version

	ASSERT_THROW(stream->getLocator() == this, EX_INVALID_STATE);

	Ref<ContentBase> content = NULL;

	ContentSource* source = dynamic_cast<ContentSource*>(stream);
	if (source)
		content = source->getContent();

	if (content) return content;

	ContentSources::iterator itr = _localContents.find(stream->getName());

	source = itr != _localContents.end() ? itr->second : NULL;

	if (source && source->getContent())
		content = source->getContent();

	if (content == NULL)
		content = createLocalContent(stream, false);

	return content;
}

Content* Package::locateLocalContent(const String& streamName)
{
	// Search from local contents
	Ref<ContentSource> source;
	ContentSources::iterator itr = _localContents.find(streamName);

	source = itr != _localContents.end() ? itr->second : NULL;

	Content* content = NULL;

	if (source && source->getContent())
		content = static_cast<Content*>(source->getContent());

	return content;
}

Content* Package::locateContent(const String& streamName, const String& locatorPattern)
{
	if (Wildcard::match(locatorPattern, _name))
	{
		Content* content = locateLocalContent(streamName);

		if (content) return content;
	}

	for (StreamLocatorList::reverse_iterator itr = _required.rbegin(); itr != _required.rend(); ++itr)
	{
		Package* req = dynamic_cast<Package*>(itr->get());
		Content* content = req->locateContent(streamName, locatorPattern);
		if (content) return content;
	}

	return NULL;
}

Ref<Content> Package::createLocalContent(StreamSource* stream, bool throwEx)
{
	// find a stream and store its location
	Ref<Content> content = NULL;

	ASSERT_THROW(stream->getLocator() == this, EX_INVALID_STATE);

	// First, look-up packageService for the ContentManager of the ContentType and then link
	ContentManager* mgr = g_Contents->findManager(stream);

	Ref<ContentSource> source = dynamic_cast<ContentSource*>(stream);
	bool newSource;	

	if (mgr == NULL)
	{
		if (throwEx) NIT_THROW_FMT(EX_NOT_SUPPORTED, "Can't find manager for content '%s'", stream->getUrl().c_str());
		goto error;
	}

	if (!mgr->canLink(stream))
	{
		if (throwEx) NIT_THROW_FMT(EX_NOT_SUPPORTED, "'%s' can't link content '%s': ", mgr->getName().c_str(), stream->getUrl().c_str());
		goto error;
	}

	newSource = source == NULL;

	// If no contentSource allocated yet, allocate one
	if (newSource)
		source = new ContentSource(this, stream->getName(), stream, NULL);

	// Create content from ContentManager
	// TODO: Watch locking order (pack.Link() -> cmgr.CreateFrom() -> pack.LinkCustom call path)

	if (throwEx)
	{
		content = mgr->createFrom(source);
	}
	else
	{
		try { content = mgr->createFrom(source); } 
		catch (...) { goto error; }
	}

	if (content == NULL)
	{
		if (throwEx) NIT_THROW_FMT(EX_SYSTEM, "'%s' creates null content '%s'", mgr->getName().c_str(), stream->getUrl().c_str());
		goto error;
	}

	// Connect ContentSource with Content
	source->_content = content;

	// if new, register it on local contents
	if (newSource)
		_localContents.insert(std::make_pair(stream->getName(), source));

	return content;

error:
	LOG(0, "*** can't create content for '%s'\n", stream->getUrl().c_str());
	// If new ContentSource and error, it'll be released by Ref<> destructor
	return NULL;
}

ContentSource* Package::linkCustom(const String& streamName, Ref<Content> content, StreamSource* otherSource, bool throwEx)
{
	ContentSources::iterator itr = _localContents.find(streamName);

	ContentSource* source = NULL;

	if (itr != _localContents.end())
	{
		source = itr->second;
		if (source->getContent() != NULL)
		{
			if (throwEx)
				NIT_THROW_FMT(EX_DUPLICATED, "content '%s/%s' already exists", _name.c_str(), streamName.c_str());
			return NULL;
		}
	}

	if (otherSource == NULL)
		otherSource = content->getSource();

	if (source)
	{
		ASSERT_THROW(source != otherSource, EX_INVALID_PARAMS);
		source->_content = content;
		source->_streamSource = otherSource;
	}
	else
	{
		source = new ContentSource(this, streamName, otherSource, content);
		_localContents.insert(std::make_pair(streamName, source));
	}

	return source;
}

StreamSource* Package::locateLocal(const String& streamName)
{
	// Search from local contents
	ContentSources::iterator itr = _localContents.find(streamName);

	if (itr != _localContents.end())
		return itr->second;

	// Search from archive if not found
	if (_archive == NULL) return NULL;  // alias pack may not have archive

	StreamSource* ret = _archive->locate(streamName, false);

	if (ret)
	{
		// Treat a file which marked DELETED as NULL: That must be done here otherwise Require() can't work correctly.
		if (ret->getContentType() == ContentType::DELETED)
		{
			Ref<StreamSource> safe = ret;
			return NULL;
		}

		return ret;
	}

	if (_base)
		ret = _base->locateLocal(streamName);

	return ret;
}

void Package::findLocal(const String& pattern, StreamSourceMap& varResults)
{
	// Search from local contents
	bool ignoreCase = !isCaseSensitive();
	for (ContentSources::iterator itr = _localContents.begin(), end = _localContents.end(); itr != end; ++itr)
	{
		if (Wildcard::match(pattern, itr->first, ignoreCase) && varResults.find(itr->first) == varResults.end())
			varResults.insert(std::make_pair(itr->first, itr->second));
	}

	// Search from archive additionally
	if (_archive)
		_archive->find(pattern, varResults);

	// Search from base additionally
	if (_base)
		_base->findLocal(pattern, varResults); // files with DELETED mark will be hidden

	// Remove files which are marked DELETED : That must be done here otherwise Require() can't work correctly.
	for (StreamSourceMap::iterator itr = varResults.begin(); itr != varResults.end(); )
	{
		if (itr->second->getContentType() == ContentType::DELETED)
			varResults.erase(itr++);
		else
			++itr;
	}
}

StreamReader* Package::open(const String& streamName)
{
	// TODO: impl cache
	return locate(streamName)->open();
}

class PackageService::ProcessScope
{
public:
	ProcessScope(Package* pack) : _package(pack) 
	{ 
		pack->_service->onPackageProcessBegin(_package);
	}

	~ProcessScope()
	{
		_package->_service->onPackageProcessEnd(_package);
	}

	Ref<Package> _package;
};

void Package::checkLinked()
{
	if (!_linked)
	{
		NIT_THROW_FMT(EX_INVALID_STATE, "package '%s' is not linked", _name.c_str());
	}
}

void Package::checkLoaded()
{
	if (!isLoaded())
	{
		NIT_THROW_FMT(EX_INVALID_STATE, "package '%s' is not loaded", _name.c_str());
	}
}

void Package::checkPrepared()
{
	if (!isPrepared())
	{
		NIT_THROW_FMT(EX_INVALID_STATE, "package '%s' is not prepared", _name.c_str());
	}
}

void Package::require(StreamLocator* req, bool first)
{
	StreamLocator::require(req, first);
}

void Package::load()
{
	if (_loaded) return;

	checkLinked();

	PackageService::ProcessScope scope(this);

	// load all required packages
	for (uint i=0; i<_required.size(); ++i)
	{
		Archive* arc = dynamic_cast<Archive*>(_required[i].get());
		if (arc)
			arc->load();
	}

	if (isLoading())
	{
		_service->joinAsyncLoader(this);
	}

	if (!_prepared)
	{
		_mutex.lock();
		_loading = true;	
		_mutex.unlock();
		LOG(0, "-- package '%s': loading\n", _name.c_str());

		prepare(false);
	}

	afterPrepared();
}

void Package::loadAsync(bool hurry)
{
	checkLinked();

	if (!_service->isAsyncLoadingEnabled())
	{
		LOG(0, "++ Package '%s': Async Loading disabled, loading synchronized\n", _name.c_str());
		return load();
	}

	{
		Mutex::ScopedLock lock(_mutex);

		if (_loaded) return;
		if (_loading) return;
		if (_prepared) return;

		_hurry = hurry;
		_loading = true;
		_prepared = false;
	}

	LOG(0, "%s package '%s': async loading\n", Thread::current() ? "&&" : "..", _name.c_str());

	// start job for all required packages
	for (uint i=0; i<_required.size(); ++i)
	{
		Package* pack = dynamic_cast<Package*>(_required[i].get());
		if (pack)
			pack->loadAsync(hurry);
	}

	_service->queuePreload(this);
}

// TODO: Tear off script reference from Session!

bool Package::initScripts()
{
	if (_scriptReady) return true;

	if (_stayForNext) 
	{
		LOG(0, "++ package '%s': for next use, skipping InitScripts()\n", _name.c_str());
		return false;
	}

	if (_scriptOpening) 
	{
		LOG(0, "*** package '%s': re-entry to InitScripts()\n", _name.c_str());
		return true;
	}

	if (!g_Session->getRuntime()->isStarted()) 
		return false;

	_scriptOpening = true;

	PackageService::ProcessScope scope(this);

	// init scripts of all required packages
	for (uint i=0; i<_required.size(); ++i)
	{
		Package* pack = dynamic_cast<Package*>(_required[i].get());
		if (pack)
			pack->initScripts();
	}
	
	LOG_TIMESCOPE(0, ".. package '%s': initializing scripts", _name.c_str());

	ScriptRuntime* script = g_Session->getRuntime();

	// add all required scripts
	for (uint i=0; i<_requiredScripts.size(); ++i)
	{
		Ref<ScriptUnit> unit;
		script->require(_requiredScripts[i].c_str(), unit, this);
	}

	// call OnLoad
	for (uint i=0; i<_onLoadScripts.size(); ++i)
	{
		LOG_TIMESCOPE(0, ".. package '%s'.OnLoad> %s", _name.c_str(), _onLoadScripts[i].c_str());
		sq_dostring(script->getRoot(), _onLoadScripts[i].c_str());
	}

	_scriptOpening = false;
	_scriptReady = true;

	return true;
}

bool Package::finishScripts()
{
	if (!_scriptReady) return true;

	ScriptRuntime* runtime = g_Session ? g_Session->getRuntime() : NULL;

	if (runtime == NULL)
	{
		_scriptReady = false;
		return true;
	}

	assert(runtime->isStarted());

	PackageService::ProcessScope scope(this);

	// Release all script-DataSchema
	DataSchemaLookup* schemaLookup = g_App->getSchemaLookup();

	for (uint i=0; i<_scriptSchemas.size(); ++i)
	{
		DataSchema* schema = schemaLookup->get(_scriptSchemas[i]);
		if (schema)
			schemaLookup->Unregister(schema);
	}

	// Unload script-unit
	runtime->unloadUnitsFrom(getName());

	// call OnUnload
	for (uint i=0; i<_onUnloadScripts.size(); ++i)
	{
		LOG(0, ".. package '%s'.OnUnload> %s\n", _name.c_str(), _onUnloadScripts[i].c_str());
		sq_dostring(runtime->getRoot(), _onUnloadScripts[i].c_str());
	}

	_scriptReady = false;

	return true;
}

void Package::unload()
{
	if (!_linked) return;

	if (_useCount._unsafeGet() > 0) return;
	if (_stayResident) return;
	if (_stayForCurrent) return;
	if (_stayForNext) return;

	if (isLoading())
	{
		LOG(0, "-- package '%s': Aborted loading\n", _name.c_str());

		// Turn off loading flag: to tell async Prepare() function.
		_mutex.lock();
		_loading = false;
		_mutex.unlock();

		// Wait till async loader finished
		_service->joinAsyncLoader(this);
	}
	else
	{
		if (!_loaded) return;
	}

	PackageService::ProcessScope scope(this);

	// Perform Dispose() on every single local contents.
	// TODO: Consider the scenario of a content which get disposed tries to dispose another content
	// and then touch _localContent by accident, and async scenarios
	for (ContentSources::iterator itr = _localContents.begin(), end = _localContents.end(); itr != end; ++itr)
	{
		ContentSource* source = itr->second;
		Content* content = source ? static_cast<Content*>(source->getContent()) : NULL;
		if (content)
			content->dispose();
	}
	_localContents.clear();

	finishScripts();

	// Release all non-script DataSchema
	DataSchemaLookup* schemaLookup = g_App->getSchemaLookup();

	for (uint i=0; i<_dataSchemas.size(); ++i)
	{
		DataSchema* schema = schemaLookup->get(_dataSchemas[i]);
		if (schema)
			schemaLookup->Unregister(schema);
	}

	// try to unload all required packages
	for (uint i=0; i<_required.size(); ++i)
	{
		Package* pack = dynamic_cast<Package*>(_required[i].get());
		if (pack)
			pack->unload();
	}

	_loaded = false;
	_prepared = false;

	LOG(0, "-- package '%s': Unloaded\n", _name.c_str());

	if (_channel) 
		_channel->send(Events::OnPackageUnloaded, new PackageEvent(this));

	if (_service->_channel)
		_service->_channel->send(Events::OnPackageUnloaded, new PackageEvent(this));
}

void Package::unlink()
{
	if (!_linked) return;

	if (_useCount._unsafeGet() > 0) return;
	if (_stayResident) return;
	if (_stayForCurrent) return;
	if (_stayForNext) return;

	unload();

	// Now this package is not linked and invalid even if it's still referenced to other objects.
	// (Simply they can't use this package. it's something like 'disposed' state - use IsLinked() to be sure)
	// If you want to use the contents again, 
	// Just re-link by the name and then PackageService will return another instance of same package.

	_linked = false;

	// Unload archive (for plugins: native module, DataSchema now released by the plugin)
	if (_archive)
		_archive->unload();

	LOG(0, ".. package '%s': Unlinked\n", _name.c_str());

	if (_channel) 
		_channel->send(Events::OnPackageUnlinked, new PackageEvent(this));

	if (_service->_channel)
		_service->_channel->send(Events::OnPackageUnlinked, new PackageEvent(this));

	// try to unlink all required packages
	for (uint i=0; i<_required.size(); ++i)
	{
		_required[i]->decUseCount();
		Package* pack = dynamic_cast<Package*>(_required[i].get());
		if (pack)
			pack->unlink();
	}
	_required.clear();

	_service->_linkedPackages.erase(_name);
}

Package::PrepareOrder Package::getPrepareOrder(StreamSource* source)
{
	static Mutex mutex;

	Mutex::ScopedLock lock(mutex);

	typedef map<ContentType::ValueType, PrepareOrder>::type PreloadOrderMap;
	static PreloadOrderMap preloadOrders;

	const ContentType& ct = source->getContentType();

	if (preloadOrders.empty())
	{
		preloadOrders[ContentType::SCRIPT_NIT]				= RO_SCRIPT;
		preloadOrders[ContentType::BINARY_NIT_SCRIPT]		= RO_SCRIPT;

		preloadOrders[ContentType::OGRE_PROGRAM]			= RO_PROGRAM;

		preloadOrders[ContentType::IMAGE_GIF]				= RO_TEXTURE;
		preloadOrders[ContentType::IMAGE_JPEG]				= RO_TEXTURE;
		preloadOrders[ContentType::IMAGE_PNG]				= RO_TEXTURE;
		preloadOrders[ContentType::IMAGE_TGA]				= RO_TEXTURE;
		preloadOrders[ContentType::IMAGE_DDS]				= RO_TEXTURE;
		preloadOrders[ContentType::IMAGE_BMP]				= RO_TEXTURE;
		preloadOrders[ContentType::IMAGE_PVR]				= RO_TEXTURE;
		preloadOrders[ContentType::IMAGE_PSD]				= RO_TEXTURE;
		preloadOrders[ContentType::IMAGE_CCZ]				= RO_TEXTURE;
		preloadOrders[ContentType::IMAGE_NTEX]				= RO_TEXTURE;

		preloadOrders[ContentType::OGRE_SKELETON]			= RO_TEXTURE;
		preloadOrders[ContentType::OGRE_XML_SKELETON]		= RO_TEXTURE;

		// material needs program or texture
		preloadOrders[ContentType::OGRE_MATERIAL]			= RO_MATERIAL;
		preloadOrders[ContentType::OGRE_AST_MATERIAL]		= RO_MATERIAL;

		// font needs texture or material
		preloadOrders[ContentType::OGRE_FONTDEF]			= RO_FONT;
		preloadOrders[ContentType::OGRE_AST_FONTDEF]		= RO_FONT;

		// compositor needs material
		preloadOrders[ContentType::OGRE_COMPOSITOR]			= RO_COMPOSITOR;
		preloadOrders[ContentType::OGRE_AST_COMPOSITOR]		= RO_COMPOSITOR;

		// mesh needs material or skeleton
		preloadOrders[ContentType::OGRE_MESH]				= RO_MESH;
		preloadOrders[ContentType::OGRE_XML_MESH]			= RO_MESH;

		// overlay needs font or material
		preloadOrders[ContentType::OGRE_OVERLAY]			= RO_OVERLAY;
		preloadOrders[ContentType::OGRE_AST_OVERLAY]		= RO_OVERLAY;

		// particle needs material or mesh
		preloadOrders[ContentType::OGRE_PU]					= RO_PARTICLE;
		preloadOrders[ContentType::OGRE_AST_PU]				= RO_PARTICLE;
	}

	PreloadOrderMap::iterator poItr = preloadOrders.find(ct);
	if (poItr == preloadOrders.end()) 
		return RO_POSTLOAD;

	return poItr->second;
}

void Package::collectPrepareFiles(PreloadFiles& varFiles, bool async)
{
	LOG(0, "%s package '%s': Collecting preload files\n", async ? "&&" : "..", _name.c_str());

	Mutex::ScopedLock lock(_mutex);

	if (!_loading) return;

	bool cacheScripts = DataValue(g_App->getConfig("cache_script_files", "false")).toBool();

	StreamSourceMap all;
	_archive->findLocal("*", all);

	for (StreamSourceMap::iterator itr = all.begin(); itr != all.end(); ++itr)
	{
		const String& filename = itr->first;
		Ref<StreamSource> source = itr->second;

		PrepareOrder order = getPrepareOrder(source);

		for (uint i=0; i<_postLoads.size(); ++i)
		{
			if (Wildcard::match(_postLoads[i], filename))
				order = RO_POSTLOAD;
		}

		if (order == RO_POSTLOAD) continue;

		if (order == RO_SCRIPT && !cacheScripts)
			continue;

		varFiles.insert(std::make_pair(order, source));
	}
}

// NOTE: Package vars which are accessed by async Preload:
// - [r] _loading
// - [w] _prepared
// - [r] _hurry
// These are to be synced (using locked IsXXX(), SetXXX()
// Make sure that other vars not touched by async thread!

void Package::prepare(bool async)
{
	// TODO: impl this
}

void Package::afterPrepared()
{
	if (!_loading) 
		return;

	_loading = false;
	initScripts();
	_loaded = true;

	LOG(0, "-- package '%s' loaded\n", _name.c_str());

	if (_channel) 
		_channel->send(Events::OnPackageLoaded, new PackageEvent(this));

	if (_service->_channel)
		_service->_channel->send(Events::OnPackageLoaded, new PackageEvent(this));
}

StreamSource* Package::locate(const String& streamName, bool throwEx)
{
	if (!_allowSubDir && streamName.find('/') != streamName.npos)
	{
		if (throwEx) NIT_THROW_FMT(EX_INVALID_PARAMS, "'%s': subdir '%s' not allowed", _name.c_str(), streamName.c_str());
		return NULL;
	}

	return Archive::locate(streamName, throwEx);
}

StreamSource* Package::locate(const String& streamName, const String& locatorPattern, bool throwEx)
{
	if (!_allowSubDir && streamName.find('/') != streamName.npos)
	{
		if (throwEx) NIT_THROW_FMT(EX_INVALID_PARAMS, "'%s': subdir '%s' not allowed", _name.c_str(), streamName.c_str());
		return NULL;
	}

	return Archive::locate(streamName, locatorPattern, throwEx);
}

void Package::find(const String& pattern, StreamSourceMap& varResults)
{
	if (!_allowSubDir && pattern.find('/') != pattern.npos)
	{
		LOG(0, "*** '%s': subdir '%s' not allowed\n", _name.c_str(), pattern.c_str());
		return;
	}

	return Archive::find(pattern, varResults);
}

void Package::find(const String& streamPattern, const String& locatorPattern, StreamSourceMap& varResults)
{
	if (!_allowSubDir && streamPattern.find('/') != streamPattern.npos)
	{
		LOG(0, "*** '%s': subdir '%s' not allowed\n", _name.c_str(), streamPattern.c_str());
		return;
	}

	return Archive::find(streamPattern, locatorPattern, varResults);
}

Database* Package::openDb(const String& streamName)
{
	String dbpath = "file:";
	dbpath += streamName;
	if (StreamSource::getExtension(streamName).empty())
		dbpath += ".sqlite";
	dbpath += "?vfs=nit&mode=ro&pack=";
	dbpath += getName();

	return Database::open(dbpath);
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
