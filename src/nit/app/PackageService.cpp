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

#include "nit/app/PackageService.h"
#include "nit/app/PackBundle.h"

#include "nit/app/Package.h"
#include "nit/app/PackArchive.h"
#include "nit/app/Plugin.h"
#include "nit/app/AppBase.h"
#include "nit/app/Session.h"

#include "nit/io/FileLocator.h"
#include "nit/data/Settings.h"
#include "nit/event/Timer.h"

#ifndef NIT_THREAD_NONE
#	include "nit/async/EventSemaphore.h"
#endif

#include "nit/data/Database.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class PackageService::AsyncLoader : public Runnable
{
public:
	AsyncLoader(PackageService* svc, int processorNo = DEFAULT_PROCESSOR_NO)
	{
		_service = svc;
		_processorNo = processorNo;
		_terminated = false;

		_myThread = new Thread("AsyncLoader");
		_myThread->start(*this);
		_myThread->setPriority(Thread::PRIO_LOW);
	}

	virtual ~AsyncLoader()
	{
		ASSERT(_terminated);
		safeDelete(_myThread);
	}

	virtual void run()
	{
#ifdef _XBOX
		XSetThreadProcessor(GetCurrentThread(), _processorNo);
#endif
		Thread::yield();

		LOG(0, "&& AsyncLoader @%d: started\n", CurrentProcessorNo());

		while (!_terminated)
		{
			_mutex.lock();
			_current = NULL;
			PopFront(_current);
			_mutex.unlock();

			if (_current == NULL)
			{
				LOG(0, "&& AsyncLoader @%d: idle\n", CurrentProcessorNo());
				_queueReady.wait();
				continue;
			}

			LOG(0, "&& AsyncLoader @%d: pick-up '%s'\n", CurrentProcessorNo(), _current->getName().c_str());

			try
			{
				_current->prepare(true);
			}
			catch (...)
			{
				LOG(0, "&& AsyncLoader @%d: Terminated due to an exception.\n", CurrentProcessorNo());
#ifdef _XBOX
				XLaunchNewImage( XLAUNCH_KEYWORD_DASH_ARCADE, 0 );
				return;
#endif				
			}

			LOG(0, "&& AsyncLoader @%d: output '%s'\n", CurrentProcessorNo(), _current->getName().c_str());
			AddOutput(_current);
		}

		LOG(0, "&& AsyncLoader @%d: terminated\n", CurrentProcessorNo());
	}

	Thread* GetThread() { return _myThread; }

	void Terminate() // outer thread
	{
		_queueMutex.lock();
		_terminated = true;
		_queue.clear();
		_queueMutex.unlock();

		_queueReady.set();

		_myThread->join();
	}

	void Join(Package* p) // outer thread
	{
		Ref<Package> pack = p;

		LOG(0, ".. AsyncLoader: Removing %s\n", pack->getName().c_str());

		_queueMutex.lock();
		_queue.remove(pack);
		_queueMutex.unlock();

		LOG(0, ".. AsyncLoader: Joining %s\n", pack->getName().c_str());

		while (true)
		{
			_mutex.lock();
			bool loading = _current == pack;
			_mutex.unlock();

			if (!loading) break;
			Thread::sleep(10);
		}
	}

	bool IsBusy() // outer thread
	{
		_mutex.lock();
		bool busy = _current != NULL;
		_mutex.unlock();

		return busy;
	}

	bool IsQueued() // outer thread
	{
		_queueMutex.lock();
		bool queued = !_queue.empty();
		_queueMutex.unlock();

		return queued;
	}

	bool HasOutput() // outer thread
	{
		_outputMutex.lock();
		bool hasOutput = !_output.empty();
		_outputMutex.unlock();

		return hasOutput;
	}

	void Enqueue(Package* pack) // outer thread
	{
		_queueMutex.lock();
		_queue.push_back(pack);
		_queueMutex.unlock();

		_queueReady.set();
	}

	void NextOutput(Ref<Package>& ret) // outer thread
	{
		_outputMutex.lock();
		ret = _output.empty() ? NULL : _output.front();
		if (ret) _output.pop_front();
		_outputMutex.unlock();
	}

private:
	void PopFront(Ref<Package>& ret)
	{
		_queueMutex.lock();
		ret = _queue.empty() ? NULL : _queue.front();
		if (ret) _queue.pop_front();
		_queueMutex.unlock();
	}

	void AddOutput(Package* pack)
	{
		_outputMutex.lock();
		_output.push_back(pack);
		_outputMutex.unlock();
	}

	Thread* _myThread;

	typedef list<Ref<Package> >::type Queue;

	PackageService*	_service;

	Ref<Package>	_current;
	Mutex			_mutex;

	Queue			_queue;
	Mutex			_queueMutex;
	EventSemaphore	_queueReady;

	Queue			_output;
	Mutex			_outputMutex;

	int				_processorNo;
	bool			_terminated;

#ifdef _XBOX
	const static int DEFAULT_PROCESSOR_NO = 4; // TODO: refactor to AppConfig
	int CurrentProcessorNo()			{ return GetCurrentProcessorNumber(); }
#else
	const static int DEFAULT_PROCESSOR_NO = 0;
	int CurrentProcessorNo()			{ return _processorNo; }
#endif
};

////////////////////////////////////////////////////////////////////////////////

PackageService::PackageService(Ref<PackBundle> topBundle)
: Service("PackageService", NULL, SVC_PACKAGE)
{
	_asyncLoader = NULL;
	_bundle = topBundle;
}

PackageService::~PackageService()
{
}

void PackageService::onRegister()
{
}

void PackageService::onUnregister()
{
}

static void InitSqliteVFS();
static void FinishSqliteVFS();

void PackageService::onInit()
{
	bool useAsyncLoading = DataValue(g_App->getConfig("async_loading", "false")).toBool();

	if (useAsyncLoading)
		_asyncLoader = new AsyncLoader(this);

	g_App->getClock()->channel()->bind(EVT::CLOCK, this, &PackageService::onClock);
	g_App->channel()->bind(EVT::SESSION_START, this, &PackageService::onSessionStart);
	g_App->channel()->bind(EVT::SESSION_STOP, this, &PackageService::onSessionStop);
	g_App->channel()->bind(EVT::SESSION_CHANGE, this, &PackageService::onSessionChange);

	InitSqliteVFS();
}

void PackageService::onFinish()
{
	FinishSqliteVFS();

	// Turn off all sticky flags - NO MERCY!
	for (PackageNameMap::iterator itr = _linkedPackages.begin(), end = _linkedPackages.end(); itr != end; ++itr)
	{
		itr->second->_stayResident = false;
		itr->second->_stayForCurrent = false;
		itr->second->_stayForNext = false;
	}

	// And compact so that all packages get disposed
	compact();

	// If Something's remaining, we're in danger!
	for (PackageNameMap::iterator itr = _linkedPackages.begin(), end = _linkedPackages.end(); itr != end; ++itr)
	{
		Package* pack = itr->second;
		LOG(0, "*** Package '%s': still alive: %d usage, requires:\n", pack->getName().c_str(), pack->getUseCount());
		if (pack->getRequiredList().empty())
			LOG(0, "  <none>\n");
		else
			pack->printDependancy(1);
	}

	// Terminate AsyncLoader
	if (_asyncLoader)
	{
		_asyncLoader->Terminate();
	}

	safeDelete(_asyncLoader);
}

void PackageService::compact()
{
	LOG_TIMESCOPE(0, "-- PackageService::Compact");

	PackageList packages;

	while (true)
	{
		packages.clear();

		// prepare current set of linked packages to unload for safe iteration
		for (PackageNameMap::iterator itr = _linkedPackages.begin(), end = _linkedPackages.end(); itr != end; ++itr)
		{
			Ref<Package> package = itr->second;

			if (package->getUseCount() > 0) continue;
			if (package->isStayResident()) continue;
			if (package->isStayForCurrent()) continue;
			if (package->isStayForNext()) continue;

			packages.push_back(package);
		}

		if (packages.empty()) 
			break;

		// unlink all in current set
		for (uint i=0; i<packages.size(); ++i)
		{
			Ref<Package> pack = packages[i];

			pack->unlink();
		}
	}
}

Package* PackageService::link(const char* givenName, bool optional)
{
	String name = givenName;

	PackageNameMap::iterator itr = _linkedPackages.find(name);
	if (itr != _linkedPackages.end())
	{
		Package* pack = itr->second;

		if (pack->_linking)
		{
			LOG(0, "*** Circular link detected with package '%s'\n", name.c_str());
			return NULL;
		}

		if (pack->isLinked())
			return pack;

		// Delayed link of packages which are registered using LinkCustom() (static plugins, etc.)
		LOG(0, "++ delayed link : package '%s'\n", name.c_str());
		if (pack->link(NULL))
		{
			if (_channel) _channel->send(EVT::PACKAGE_LINKED, new PackageEvent(pack));
			return pack;
		}

		// Link failed - remove from table
		LOG(0, "*** link failed : package '%s'\n", name.c_str());
		_linkedPackages.erase(name);

		return NULL;
	}

	// We will save the pack using Ref<>.
	// Then when link fails the temporary pack will be released safely by Ref<> destructor
	// On the success the pack will get inserted into the _linkedPackages
	Ref<Package> pack = _bundle->link(givenName);
	if (pack == NULL)
	{
		// not found, return NULL
		if (!optional)
			LOG(0, "*** cannot locate package '%s'\n", givenName);
		return NULL;
	}

	Ref<StreamSource> cfgSrc = pack->getSettingsSource();
	Ref<Settings> cfg;

	if (cfgSrc) // for DLL packs, cfgSrc == NULL
	{
		cfg = Settings::load(cfgSrc);
		if (cfg == NULL)
		{
			LOG(0, "*** cannot load '%s'\n", pack->getSettingsSource()->getUrl().c_str());
			return NULL;
		}
	}

	_linkedPackages.insert(std::make_pair(name, pack));
	pack->_service = this;

	if (pack->link(cfg))
	{
		if (_channel) _channel->send(EVT::PACKAGE_LINKED, new PackageEvent(pack));
		return pack;
	}

	// Link failed - remove from table
	LOG(0, "*** link failed : package '%s'\n", name.c_str());
	_linkedPackages.erase(name);

	return NULL;
}

Package* PackageService::linkCustom(const String& givenName, Archive* archive, Settings* settings)
{
	String name = givenName;

	if (_linkedPackages.find(name) != _linkedPackages.end())
		NIT_THROW_FMT(EX_DUPLICATED, "package '%s' already exists", name.c_str());

	Package* pack = new Package(name, archive);
	pack->_service = this;
	_linkedPackages.insert(std::make_pair(name, pack));

	pack->_type = Package::PK_CUSTOM;
	pack->_name = name;

	return pack;
}

Package* PackageService::load(const char* name)
{
	Package* pack = link(name);

	if (pack) 
	{
		pack->load();
	}
	else
	{
		LOG(0, "*** cannot load package: '%s'\n", name);
	}

	return pack;
}

Package* PackageService::loadAsync(const char* name, bool hurry)
{
	Package* pack = link(name);

	if (pack) 
	{
		pack->loadAsync(hurry);
	}
	else
	{
		LOG(0, "*** cannot load async package: '%s'\n", name);
	}
	
	return pack;
}

void PackageService::find(const String& pattern, vector<Package*>::type& varResults)
{
	for (PackageNameMap::iterator itr = _linkedPackages.begin(); itr != _linkedPackages.end(); ++itr)
	{
		if (Wildcard::match(pattern, itr->first))
			varResults.push_back(itr->second.get());
	}
}

void PackageService::onPackageProcessBegin(Package* pack)
{
	_processing.push_back(pack);
}

void PackageService::onPackageProcessEnd(Package* pack)
{
	if (!_processing.empty() && _processing.back() == pack)
	{
		_processing.pop_back();
	}
	else
	{
		assert(false);
	}
}

Package* PackageService::getProcessing()
{
	return _processing.empty() ? NULL : _processing.back();
}

void PackageService::onSessionStart(const SessionEvent* evt)
{
	LOG_TIMESCOPE(0, ".. PackageService::OnSessionStart");

	// init all scripts for all loaded packages
	for (PackageNameMap::iterator itr = _linkedPackages.begin(), end = _linkedPackages.end(); itr != end; ++itr)
	{
		Package* pack = itr->second;
		if (pack->isLoaded())
			pack->initScripts();
	}
}

void PackageService::onSessionStop(const SessionEvent* evt)
{
	// finish all scripts for every package
	for (PackageNameMap::iterator itr = _linkedPackages.begin(), end = _linkedPackages.end(); itr != end; ++itr)
	{
		Package* pack = itr->second;
		pack->finishScripts();
	}
}

void PackageService::onSessionChange(const SessionEvent* evt)
{
	// finish all scripts for every package
	for (PackageNameMap::iterator itr = _linkedPackages.begin(), end = _linkedPackages.end(); itr != end; ++itr)
	{
		Package* pack = itr->second;
		pack->_stayForCurrent = false;
	}

	// Compact and remove packages not needed more - except those StayForNext packages
	compact();

	// Transcend remaining StayForNext to next StayForCurrent
	for (PackageNameMap::iterator itr = _linkedPackages.begin(), end = _linkedPackages.end(); itr != end; ++itr)
	{
		Package* pack = itr->second;
		pack->_stayForCurrent = pack->_stayForNext;
		pack->_stayForNext = false;
	}
}

void PackageService::queuePreload(Package* pack)
{
	ASSERT(_asyncLoader);

	if (_asyncLoader == NULL) return;

	_asyncLoader->Enqueue(pack);
}

void PackageService::onClock(const Event* evt)
{
	updatePreloads();
}

void PackageService::updatePreloads(float timeLimit)
{
	if (_asyncLoader == NULL) return;

	double startTime = SystemTimer::now();

	while (true)
	{
		Ref<Package> pack = NULL;
		_asyncLoader->NextOutput(pack);

		if (pack == NULL)
			break;

		if (pack->isPrepared())
		{
			LOG(0, ".. package '%s': prepare completed\n", pack->getName().c_str());
			pack->afterPrepared();

			break; // TODO: reconsider this
		}

		double now = SystemTimer::now();
		if (now - startTime > timeLimit)
		{
			LOG(0, ".. package prepare time limit %.3f > %.3f\n", float(now - startTime) / float(CLOCKS_PER_SEC), timeLimit);
			break;
		}
	}
}

bool PackageService::isAsyncLoading()
{
	if (_asyncLoader == NULL) return false;

	return _asyncLoader->IsBusy() || _asyncLoader->IsQueued() || _asyncLoader->HasOutput();
}

void PackageService::joinAsyncLoader(Package* pack)
{
	if (_asyncLoader == NULL) return;

	_asyncLoader->Join(pack);
}

void PackageService::filterPackCfgs(Ref<Settings> appCfg, StreamSourceMap& varResults)
{
	StringVector includes;
	appCfg->find("bundler/bundler_include", includes);

	if (includes.size() > 1)
		NIT_THROW_FMT(EX_NOT_SUPPORTED, "only one 'bundler_include' supported yet");

	Ref<Settings> appCfgInclude;

	if (includes.size() == 1)
		appCfgInclude = Settings::load(appCfg->getSource()->getLocator()->locate(includes[0]));

	StringVector packIgnoreFilters;
	if (appCfgInclude)
	{
		appCfgInclude->find("bundler/pack_ignore", packIgnoreFilters);
		appCfgInclude->find(String("bundler/") + g_App->getConfigs()->getPlatform() + "/pack_ignore", packIgnoreFilters);
	}

	appCfg->find("bundler/pack_ignore", packIgnoreFilters);
	appCfg->find(String("bundler/") + g_App->getConfigs()->getPlatform() + "/pack_ignore", packIgnoreFilters);

	StreamSourceMap filtered;

	for (StreamSourceMap::iterator itr = varResults.begin(), end = varResults.end(); itr != end; ++itr)
	{
		bool isAlias = false;

		size_t tailPos = itr->first.find("/pack.cfg");
		if (tailPos == String::npos) 
		{
			tailPos = itr->first.find(".pack.cfg");
			isAlias = tailPos != String::npos;
		}

		if (tailPos == String::npos)
			continue;

		if (tailPos == 0) // bad name
			continue;

		String packname = itr->first.substr(0, tailPos);

		StringUtil::toLowerCase(packname);

		bool ignored = false;

		for (uint i=0; !ignored && i<packIgnoreFilters.size(); ++i)
		{
			if (Wildcard::match(packIgnoreFilters[i], packname))
				ignored = true;
		}

		if (ignored)
			continue;

		filtered.insert(std::make_pair(packname, itr->second));
	}

	varResults = filtered;
}

void PackageService::splitLookupEntry(String& varType, String& varKey, String& varSubType, String& varEntry)
{
	size_t p = varType.find(':');
	if (p != varType.npos)
	{
		varSubType = varType.substr(p+1);
		varType = varType.substr(0, p);
		StringUtil::trim(varSubType);
		StringUtil::trim(varType);
	}

	p = varKey.find(':');
	if (p != varKey.npos)
	{
		varEntry = varKey.substr(p+1);
		varKey = varKey.substr(0, p);
		StringUtil::trim(varKey);
		StringUtil::trim(varEntry);
	}
}

Ref<Database> PackageService::buildLookupDB(const String& path, StreamSourceMap& packCfgs)
{
	Ref<Database> db = Database::open(path);

	db->exec("DROP TABLE IF EXISTS packs; CREATE TABLE packs (name PRIMARY KEY, timestamp INT)");
	db->exec("DROP TABLE IF EXISTS lookup; CREATE TABLE lookup (type, key, subtype, pack, entry, PRIMARY KEY (type, key))");
	db->exec("DROP INDEX IF EXISTS lookup_packidx; CREATE INDEX lookup_packidx ON lookup (pack ASC)");

	Ref<Database::Query> packInsert = 
		db->prepare("INSERT INTO packs (name, timestamp) VALUES (?, ?)");

	Ref<Database::Query> lookupInsert = 
		db->prepare("INSERT INTO lookup (type, key, subtype, pack, entry) VALUES ($type, $key, $subtype, $pack, $entry)");

	for (StreamSourceMap::iterator itr = packCfgs.begin(), end = packCfgs.end(); itr != end; ++itr)
	{
		const String& packName = itr->first;
		Ref<Settings> cfg = Settings::load(itr->second);

		if (cfg == NULL)
		{
			LOG(0, "*** can't load '%s'\n", cfg->getUrl().c_str());
			continue;
		}

		Timestamp time = itr->second->getTimestamp();

		packInsert->reset();
		packInsert->bind(1, packName);
		packInsert->bind(2, time.getUnixTime64());

		packInsert->step();

		Ref<Settings> section = cfg->getSection("lookup");
		if (section == NULL)
			continue;

		for (Settings::Iterator itr = section->begin(), end = section->end(); itr != end; ++itr)
		{
			String type = itr->first;
			String key = itr->second;

			String sub, entry;

			splitLookupEntry(type, key, sub, entry);

			lookupInsert->reset();
			lookupInsert->bind("$type", type);
			lookupInsert->bind("$key", key);
			lookupInsert->bind("$subtype", sub);
			lookupInsert->bind("$entry", entry);
			lookupInsert->bind("$pack", packName);

			lookupInsert->step();
		}
	}

	return db;
}

Database* PackageService::getLookupDB()
{
	if (_lookupDB != NULL) return _lookupDB;

	if (_bundle->getRealFile())
	{
		// If we can access a *real* bundle : lookup from the bundle
		_lookupDB = Database::open("file:lookup.sqlite?vfs=nit&pack=$bundle&mode=ro");
		return _lookupDB;
	}

	// dev-bundle : Generate & update db file on save folder
	String dbFilename = g_App->getConfigs()->getAppSavePath() + "/lookup.sqlite";
	FileUtil::normalizeSeparator(dbFilename);

	bool rebuild = true;

	// TODO: android my need !FileUtil::Exists(dbFilename)..

	if (rebuild)
	{
		LOG_TIMESCOPE(0, "++ rebuilding '%s'", dbFilename.c_str());

		StreamSourceMap packCfgs;

		// Collect every single pack.cfg from the bundle and linked pathes
		_bundle->collectPackCfgs(packCfgs);

		// filter out
		filterPackCfgs(g_App->getConfigs()->getSettings(), packCfgs);

		// remove old db file
		FileUtil::createDir(g_App->getConfigs()->getAppSavePath());
		FileUtil::remove(dbFilename);

		buildLookupDB(dbFilename, packCfgs);
	}

	// TODO: use more appropriate path (in the bundle or cache folder)
	_lookupDB = Database::open(String("file:") + dbFilename + "?mode=ro");

	return _lookupDB;
}

bool PackageService::lookup(const String& type, const String& key, LookupResult& outResult)
{
	getLookupDB();

	if (_lookupDB == NULL)
		NIT_THROW_FMT(EX_INVALID_STATE, "can't get lookup db");

	if (_lookupSelect == NULL)
	{
		_lookupSelect = _lookupDB->prepare(
			"SELECT subtype, pack, entry FROM lookup WHERE type=$type AND key=$key");
	}

	_lookupSelect->reset();
	_lookupSelect->bind(1, type);
	_lookupSelect->bind(2, key);

	if (_lookupSelect->step())
	{
		_lookupSelect->getText(0, outResult.subType);
		_lookupSelect->getText(1, outResult.pack);
		_lookupSelect->getText(2, outResult.entry);

		_lookupSelect->reset();
		return true;
	}

	_lookupSelect->reset();
	return false;
}

////////////////////////////////////////////////////////////////////////////////

/// readonly 'nit' vfs for sqlite3
/// usage: ex) when you want to open test.db in /ui/db.pack:
/// file:test.db?vfs=nit&mode=ro&pack=ui/db
/// 'vfs=nit', 'mode=ro' : mandatory
/// 'pack=' is optional which defaults to current session's pack

struct NitSqliteVFS : public sqlite3_vfs
{
	sqlite3_vfs*						os;
	const struct sqlite3_io_methods*	methods;

	struct DBFile
	{
		const sqlite3_io_methods*		methods;
		size_t							size;
		Ref<StreamReader>				reader;
		Ref<StreamLocator>				locator;
	};

	static int FileClose(sqlite3_file* pFile)
	{
		DBFile* file = (DBFile*)pFile;

		if (file->locator)
		{
			file->locator->decUseCount();
			file->locator = NULL;
		}
		file->reader = NULL;

		return SQLITE_OK;
	}

	static int FileRead(sqlite3_file* pFile, void* pBuf, int amt, sqlite3_int64 offset64)
	{
		DBFile* file = (DBFile*)pFile;
		StreamReader* reader = file->reader;

		if (reader == NULL)
			return SQLITE_IOERR_READ;

		assert(offset64 >= 0);

		size_t offset = (size_t)offset64; // TODO: support 64bit
		reader->seek(offset);

		int read = reader->readRaw(pBuf, amt);
		if (read < amt)
		{
			uint8* fillStart = (uint8*)pBuf;
			fillStart += read;

			// sqlite needs us to fill zeroes when IOERR_SHORT_READ
			memset(fillStart, 0, amt - read);
			return SQLITE_IOERR_SHORT_READ;
		}

		return SQLITE_OK;
	}

	static int FileSize(sqlite3_file* pFile, sqlite3_int64* pSize)
	{
		DBFile* file = (DBFile*)pFile;
		*pSize = file->size;
		return SQLITE_OK;
	}

	static int FileSectorSize(sqlite3_file*)
	{
		return 4096; // TODO: remove all magic numbers
	}

	static int FileDeviceCharacteristics(sqlite3_file*)
	{
		// TODO: Investigate
		return 0;
	}

	// Unnecessary write related methods (pack filesystem is only for only read-only only!)
	static int FileWrite(sqlite3_file*, const void*, int, sqlite3_int64)		{ return SQLITE_IOERR_WRITE; }
	static int FileTruncate(sqlite3_file*, sqlite3_int64)						{ return SQLITE_IOERR_TRUNCATE; }
	static int FileSync(sqlite3_file*, int)										{ return SQLITE_IOERR_FSYNC; }
	static int FileLock(sqlite3_file*, int)										{ return SQLITE_OK; }
	static int FileUnlock(sqlite3_file*, int)									{ return SQLITE_OK; }
	static int FileCheckReservedLock(sqlite3_file*, int*)						{ return SQLITE_OK; }
	static int FileControl(sqlite3_file*, int, void*)							{ return SQLITE_OK; }

	static int VFSOpen(sqlite3_vfs* pVfs, const char* zPath, sqlite3_file* pFile, int flags, int* pOutFlags)
	{
		const NitSqliteVFS* vfs = (NitSqliteVFS*)pVfs;
		DBFile* file = (DBFile*)pFile;

		// methods must be set to NULL, even if xOpen call fails
		file->methods = NULL;

		// Only permit readonly permission
		if ( 
			zPath == NULL ||
			(flags & SQLITE_OPEN_DELETEONCLOSE) ||
			(flags & SQLITE_OPEN_READWRITE) ||
			(flags & SQLITE_OPEN_CREATE) ||
			!(flags & SQLITE_OPEN_READONLY) ||
			!(flags & SQLITE_OPEN_MAIN_DB) 
			)
			return SQLITE_PERM;

		Ref<StreamLocator> locator;
		const char* packname = sqlite3_uri_parameter(zPath, "pack");

		if (packname)
		{
			if (strcmp(packname, "$bundle") == 0)
				locator = g_Package->getBundle();
			else
				locator = g_Package->load(packname);
		}
		else if (g_SessionService && g_Session)
			locator = g_Session->getPackage();

		if (locator == NULL)
		{
			LOG(0, "*** can't find package for db path '%s'\n", zPath);
			return SQLITE_CANTOPEN;
		}

		Ref<StreamSource> source = locator->locate(zPath, false);
		if (source == NULL)
		{
			LOG(0, "*** can't find db file '%s' from '%s'\n", zPath, locator->getName().c_str());
			return SQLITE_CANTOPEN;
		}

		Ref<StreamReader> reader = source->open();

		if (dynamic_cast<FileReader*>(reader.get()) == NULL)
		{
			LOG(0, "?? db file '%s' is not based on a real file\n", source->getUrl().c_str());
		}

		if (!(reader->isSized() && reader->isSeekable()) )
		{
			LOG(0, "?? db file '%s': whole contents into memory\n", source->getUrl().c_str());
			reader = new MemoryBuffer::Reader(reader);
		}

		file->reader = reader;

		file->size = reader->getSize();
		file->locator = locator;
		locator->incUseCount();

		file->methods = vfs->methods;

		if (pOutFlags)
			*pOutFlags = flags;

		return SQLITE_OK;
	}

	static int VFSAccess(sqlite3_vfs* pVfs, const char* zPath, int flags, int* pResOut)
	{
		*pResOut = 0;

		if (flags != SQLITE_ACCESS_EXISTS && flags != SQLITE_ACCESS_READ)
			return SQLITE_OK;

		// Allow *.db or *.sqlite as file extension.
		// It seems to be safe to ignore other extensions.
		// (the db is readonly but these extensions are -journal -wal etc. for writing purpose)

		if (!Wildcard::match("*.db", zPath) && !Wildcard::match("*.sqlite", zPath))
			return SQLITE_OK;

		Ref<StreamLocator> locator;
		const char* packname = sqlite3_uri_parameter(zPath, "pack");

		if (packname)
		{
			if (strcmp(packname, "$bundle") == 0)
				locator = g_Package->getBundle();
			else
				locator = g_Package->load(packname);
		}
		else if (g_SessionService && g_Session)
			locator = g_Session->getPackage();

		if (locator == NULL)
			return SQLITE_OK;

		Ref<StreamSource> source = locator->locate(zPath, false);
		if (source == NULL)
			return SQLITE_OK;

		*pResOut = 1;
		return SQLITE_OK;
	}

	static int VFSFullPathname(sqlite3_vfs* pVfs, const char* zPath, int nOut, char* zOut)
	{
		// It seems that sqlite perform combination of fullpath in prior and then call this fn.
		// So hand over original path as return
		strncpy(zOut, zPath, nOut);
		return SQLITE_OK;
	}

	static int VFSDelete(sqlite3_vfs*, const char*, int)
	{ 
		// read-only
		return SQLITE_ERROR; 
	}

	static int VFSGetLastError(sqlite3_vfs* pVfs, int arg1, char* arg2)
	{
		// TODO: investigate
		return 0;
	}

	// default impls
	static int VFSRandomness(sqlite3_vfs* pVfs, int nBuf, char* zBuf)
	{
		const NitSqliteVFS* vfs = (NitSqliteVFS*)pVfs;
		return vfs->os->xRandomness(vfs->os, nBuf, zBuf);
	}

	static int VFSSleep(sqlite3_vfs* pVfs, int microseconds)
	{
		const NitSqliteVFS* vfs = (NitSqliteVFS*)pVfs;
		return vfs->os->xSleep(vfs->os, microseconds);
	}

	static int VFSCurrentTime(sqlite3_vfs* pVfs, double* prNow)
	{
		const NitSqliteVFS* vfs = (NitSqliteVFS*)pVfs;
		return vfs->os->xCurrentTime(vfs->os, prNow);
	}

	static int VFSCurrentTimeInt64(sqlite3_vfs* pVfs, sqlite3_int64* piNow)
	{
		const NitSqliteVFS* vfs = (NitSqliteVFS*)pVfs;
		return vfs->os->xCurrentTimeInt64(vfs->os, piNow);
	}

	NitSqliteVFS(const char* osVfsName = NULL)
	{
		os = sqlite3_vfs_find(osVfsName);

		static const sqlite3_io_methods fileMethods =
		{
			1,
			FileClose,
			FileRead,
			FileWrite,
			FileTruncate,
			FileSync,
			FileSize,
			FileLock,
			FileUnlock,
			FileCheckReservedLock,
			FileControl,
			FileSectorSize,
			FileDeviceCharacteristics,
		};

		methods = &fileMethods;

		iVersion			= 3;
		szOsFile			= sizeof(DBFile);
		mxPathname			= MAX_PATH;
		pNext				= 0;
		zName				= "nit";
		pAppData			= 0;
		xOpen				= VFSOpen;
		xDelete				= VFSDelete;
		xAccess				= VFSAccess;
		xFullPathname		= VFSFullPathname;
		xDlOpen				= 0;
		xDlError			= 0;
		xDlClose			= 0;
		xRandomness			= VFSRandomness;
		xSleep				= VFSSleep;
		xCurrentTime		= VFSCurrentTime;
		xCurrentTimeInt64	= VFSCurrentTimeInt64;
		xGetLastError		= VFSGetLastError;
		xSetSystemCall		= 0;
		xGetSystemCall		= 0;
		xNextSystemCall		= 0;
	}
};

static NitSqliteVFS* g_NitSqliteVFS = NULL;

static void InitSqliteVFS()
{
	assert(g_NitSqliteVFS == NULL);

	Database::initialize();

	g_NitSqliteVFS = new NitSqliteVFS();

	sqlite3_vfs_register(g_NitSqliteVFS, 0);
}

static void FinishSqliteVFS()
{
	assert(g_NitSqliteVFS);

	sqlite3_vfs_unregister(g_NitSqliteVFS);

	safeDelete(g_NitSqliteVFS);
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
