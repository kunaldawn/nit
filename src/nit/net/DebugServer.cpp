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

#include "nit/net/DebugServer.h"

////////////////////////////////////////////////////////////////////////////////

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class RemoteDebugLogger : public Logger
{
public:
	RemoteDebugLogger(DebugServer* server)
	{
		_debugServer = server;
		_buffer = new MemoryBuffer();
	}

	void writeCStr(const char* str, uint32 len = 0)
	{
		if (str)
		{
			if (len == 0)
				len = strlen(str);
			_buffer->pushBack(str, len);
		}

		uint8 zero = 0;
		_buffer->pushBack(&zero, 1);
	}

	void doLog(const LogEntry* entry)
	{
		RemotePeer* attached = _debugServer->getAttached();

		if (attached == NULL) return;
		if (attached->getRemote()->isRemoteLogSuppressed()) return;

		uint8 logLevel = (uint8)entry->logLevel;
		_buffer->pushBack(&logLevel, sizeof(logLevel));
		
		uint8 flags = 0;
		if (_lineStart)	flags |= RemoteLogEntry::FLAG_LINESTART;
		if (entry->lineEnd) flags |= RemoteLogEntry::FLAG_LINEEND;
		_buffer->pushBack(&flags, sizeof(flags));

		uint32 messageLen = entry->messageLen;
		_buffer->pushBack(&messageLen, sizeof(messageLen));
		_buffer->pushBack(entry->message, entry->messageLen);

		writeCStr(entry->act);
		writeCStr(entry->tagStr);
		_buffer->pushBack(&entry->time, sizeof(entry->time));
		writeCStr(entry->channel->getName().c_str());

		if (entry->logLevel >= LOG_LEVEL_WARNING)
		{
			uint32 line = entry->line;
			_buffer->pushBack(&line, sizeof(line));
			writeCStr(entry->srcName);
			writeCStr(entry->fnName);
		}

		attached->getRemote()->notify(attached, _debugServer->getChannelID(), DebugServer::NT_SVR_LOG_ENTRY, 
			DataValue(_buffer).convertTo(DataValue::TYPE_BLOB));
		_buffer->clear();
	}

	void doLog_old(const LogEntry* entry)
	{
		// TODO: If possible entry as-is to optimize

		RemotePeer* attached = _debugServer->getAttached();

		if (attached == NULL)
			return;

		char buf[LogManager::MAX_BUF_SIZE];
		size_t bufSize = sizeof(buf) - 1;

		if (_lineStart)
		{
			int len = formatLog(entry, buf, sizeof(buf));

			_buffer->pushBack(buf, len);
		}

		int len = int(entry->messageLen) > bufSize ? bufSize : entry->messageLen;
		_buffer->pushBack(entry->message, len);

		if (entry->lineEnd)
		{
			_buffer->pushBack("\n", 1);
		}

		attached->getRemote()->notify(attached, _debugServer->getChannelID(), DebugServer::NT_SVR_LOG_ENTRY, 
			DataValue(_buffer).convertTo(DataValue::TYPE_BLOB));

		_buffer->clear();
	}

	Weak<DebugServer>					_debugServer;
	Ref<MemoryBuffer>					_buffer;
};

////////////////////////////////////////////////////////////////////////////////

RemoteLogEntry::RemoteLogEntry(const void* packet, size_t packetSize)
{
	const char* ptr = (const char*)packet;

	logLevel	= *(uint8*)ptr++;
	flags		= *(uint8*)ptr++;

	messageLen	= *(uint32*)ptr;	ptr += sizeof(messageLen);
	message		= ptr;				ptr += messageLen;

	act			= ptr;				ptr += strlen(act) + 1;
	tagStr		= ptr;				ptr += strlen(tagStr) + 1;
	time		= *(float*)ptr;		ptr += sizeof(time);
	channel		= ptr;				ptr += strlen(channel) + 1;

	if (logLevel >= LOG_LEVEL_WARNING)
	{
		line	= *(uint32*)ptr;	ptr += sizeof(line);
		srcName	= ptr;				ptr += strlen(srcName) + 1;
		fnName	= ptr;				ptr += strlen(fnName) + 1;
	}
	else
	{
		line	= 0;
		srcName	= "";
		fnName	= "";
	}

	ASSERT_THROW(ptr == ((const char*)packet) + packetSize, EX_CORRUPTED);
}

int RemoteLogEntry::formatHeader(char* buf, int bufSize)
{
	--bufSize;

	int milli = int(time * 1000) % 1000;
	int sec = int(time);
	int min = sec / 60;
	int hour = min / (60);

	bool shortForm = true;
	int sz = 0;

	if (shortForm)
	{
		sz = _snprintf(buf, bufSize, "%02d:%02d.%03d%10s> %2s%3s ",
			min,
			sec % 60,
			milli / 10,
			channel,
			act,
			tagStr);
	}
	else
	{
		sz = _snprintf(buf, bufSize, "%02d:%02d:%02d.%04d%10s> %2s%3s ",
			hour,
			min % 60,
			sec % 60,
			milli,
			channel,
			act,
			tagStr);
	}

	return sz;
}

////////////////////////////////////////////////////////////////////////////////

DebugServer::DebugServer(Remote::ChannelId channelID /*= 0xdeb6*/)
{
	_remote		= NULL;
	_debugger	= NULL;

	_channelID = channelID;

	_active	= true; // controlled by session manager, etc.

	_debugging = false;

	_breakpointsUpdated = false;

	listen();
}

DebugServer::~DebugServer()
{
	if (_logger)
		LogManager::getSingleton().detach(_logger);

	safeDelete(_remote);
}

void DebugServer::listen(const String& addr, uint16 port)
{
	if (_remote)
	{
		_remote->shutdown();
		LogManager::getSingleton().detach(_logger);
	}

	_remote = new Remote();

	typedef DebugServer ThisClass;

	EventChannel* ch = _remote->getChannel(0);

	ch->bind(EVT::REMOTE_DISCONNECT, this, &ThisClass::onRemoteDisconnect);

	_logger = new RemoteDebugLogger(this);
	_logger->setLogLevel(LOG_LEVEL_VERBOSE);
	LogManager::getSingleton().attach(_logger);

	ch = _remote->openChannel(_channelID, "nit.RemoteDebug");

	ch->bind(EVT::REMOTE_REQUEST, this, &ThisClass::onRemoteRequest);
	ch->bind(EVT::REMOTE_NOTIFY, this, &ThisClass::onRemoteNotify);
	ch->bind(EVT::REMOTE_DOWNLOAD_CANCEL, this, &ThisClass::onRemoteDownloadCancel);
	ch->bind(EVT::REMOTE_DOWNLOAD_END, this, &ThisClass::onRemoteDownloadEnd);

	NitRuntime* rt = NitRuntime::getSingleton();

	String hostaddr = addr.empty() ? rt->getHostName() : addr;

	if (!_remote->listen(hostaddr, port))
	{
		rt->alert(rt->getTitle(), "Can't listen Remote");
	}
	else
	{
		rt->info(rt->getTitle(), (String("started & listening: ") + rt->getMainIp()).c_str());
	}
}

void DebugServer::update()
{
	_remote->update();

	if (_breakpointsUpdated && _debugger)
	{
		_debugger->updateBreakpoints();
		_breakpointsUpdated = false;
	}

	while (!_pendingCommands.empty())
	{
		String cmd = _pendingCommands.front();
		_pendingCommands.pop_front();

		LOG(0, "++ Remote> %s\n", cmd.c_str());
		NitRuntime::getSingleton()->debugCommand(cmd);
	}
}

void DebugServer::onRemoteRequest(const RemoteRequestEvent* evt)
{
	if (evt->command == RQ_ATTACH)
	{
		if (_peer != NULL)
			return evt->response(RESPONSE_ERROR, "another debug client already attached");

		// TODO: check client version from the param
		attach(evt->peer, evt->param);
		return evt->response(RESPONSE_OK);
	}

	if (_peer != evt->peer)
		return evt->response(RESPONSE_ERROR, "invalid peer");

	////////////////////////////////////

	switch (evt->command)
	{
	case RQ_PACKS:						return onRequestPacks(evt);
	case RQ_FILE:						return onRequestFile(evt);
	}

	////////////////////////////////////

	// Following requests are handled only when a debugger presents

	if (_debugger == NULL)
		return evt->response(RESPONSE_ERROR, "no debugger");

	////////////////////////////////////

	// Following requests are handled only when active state
	if (!isActive())
		return evt->response(RESPONSE_ERROR, "debugger inactive");

	switch (evt->command)
	{
	case RQ_BREAK:						return onRequestBreak(evt);
	}

	////////////////////////////////////

	// Following requests are handled only when debugging state
 	if (!isDebugging())
 		return evt->response(RESPONSE_ERROR, "not debugging");

	switch (evt->command)
	{
	case RQ_GO:							return onRequestGo(evt);
	case RQ_STEP_INTO:					return onRequestStepInto(evt);
	case RQ_STEP_OVER:					return onRequestStepOver(evt);
	case RQ_STEP_OUT:					return onRequestStepOut(evt);

	case RQ_LOCALS:						return onRequestLocals(evt);
	case RQ_INSPECT:					return onRequestInspect(evt);
	case RQ_EVALUATE:					return onRequestEvaluate(evt);
	}
}

void DebugServer::onRemoteNotify(const RemoteNotifyEvent* evt)
{
	if (_peer != evt->peer) return;

	switch (evt->command)
	{
	case NT_DETACH:						detach(); break;

	case NT_COMMAND:					onNotifyCommand(evt); break;

	case NT_ADD_BP:						onNotifyAddBP(evt); break;
	case NT_DEL_BP:						onNotifyRemoveBP(evt); break;
	case NT_CLEAR_BP:					onNotifyClearBP(evt); break;
	}
}

void DebugServer::onRemoteDisconnect(const RemoteEvent* evt)
{
	if (_peer != evt->peer) return;

	_peer = NULL;

	// clear debugging states
	_debugging = false;

	_breakpoints.clear();

	if (_debugger)
	{
		_debugger->setActive(false);
		_debugger->updateBreakpoints();
	}

	LOG(0, "++ [DBGSVR] DebugClient detached\n");
}

void DebugServer::onNotifyCommand(const RemoteNotifyEvent* evt)
{
	NitRuntime* rt = NitRuntime::getSingleton();
	if (rt == NULL) return;

	String cmd = evt->param.toString();
	_pendingCommands.push_back(cmd);
}

void DebugServer::attach(RemotePeer* peer, DataValue params)
{
	ASSERT_THROW(_peer == NULL, EX_DUPLICATED);

	const String& client = params.get("client").toString();
	bool enableLog = params.get("log").Default(false);

	_peer = peer;

	LOG(0, "++ [DBGSVR] DebugClient attached\n");

	if (_debugger)
		_debugger->setActive(_active);

	if (_peer)
	{
		if (_active)
			_remote->notify(_peer, _channelID, NT_SVR_ACTIVE);
		else
			_remote->notify(_peer, _channelID, NT_SVR_INACTIVE);
	}
}

void DebugServer::setActive(bool flag)
{
	if (_active == flag) return;

	_active = flag;

	if (_debugger)
		_debugger->setActive(_active && _peer);

	if (_peer)
	{
		if (_active)
			_remote->notify(_peer, _channelID, NT_SVR_ACTIVE);
		else
			_remote->notify(_peer, _channelID, NT_SVR_INACTIVE);
	}
}

void DebugServer::detach()
{
	if (_peer == NULL)
		return;

	_peer->disconnect();

	// actual detaching will occur at onRemoteDisconnect()
}

void DebugServer::setDebugger(IDebugger* debugger)
{
	ASSERT_THROW(_debugger == NULL || debugger == NULL, EX_INVALID_STATE);

	_debugger = debugger;

	if (_debugger)
	{
		_debugger->setActive(_peer && _active);

		if (_active)
			_debugger->updateBreakpoints();
	}
}

static void populatePackRecord(DataRecord* packRec, Package* pack, map<String, Package*>::type& packOfName)
{
	DataNamespace* ns = packRec->getNamespace();
	packRec->set("name", ns->add(pack->getName()));

	switch (pack->getType())
	{
	case Package::PK_PACKFILE:		packRec->set("type", ns->add("packfile")); break;
	case Package::PK_PLUGIN:		packRec->set("type", ns->add("plugin")); break;
	case Package::PK_FOLDER:		packRec->set("type", ns->add("folder")); break;
	case Package::PK_ALIAS:			packRec->set("type", ns->add("alias")); break;
	case Package::PK_CUSTOM:		packRec->set("type", ns->add("custom")); break;
	case Package::PK_DELETED:		packRec->set("type", ns->add("deleted")); break;
	default:						packRec->set("type", ns->add("unknown")); break;
	}

	Ref<StreamSource> cfgSrc = pack->getSettingsSource();
	if (cfgSrc)
	{
		packRec->set("cfg", ns->add(cfgSrc->getName()));

		Ref<DataArray> requireArray = new DataArray();
		packRec->set("requires", requireArray);

		Ref<Settings> cfg = Settings::load(cfgSrc);
		if (cfg)
		{
			StringVector requires;
			StringVector optionals;
			StringVector excludes;

			pack->parseRequireSection(cfg, requires, optionals, excludes);

			for (uint i=0; i<requires.size(); ++i)
			{
				String& pack = requires[i];

				if (std::find(excludes.begin(), excludes.end(), pack) != excludes.end())
					continue;

				requireArray->append(ns->add(pack)); // client should check if the pack exists
			}

			for (uint i=0; i<optionals.size(); ++i)
			{
				String& pack = optionals[i];
				if (std::find(excludes.begin(), excludes.end(), pack) != excludes.end())
					continue;

				if (packOfName.find(pack) != packOfName.end()) // filter-out non existing packs
					requireArray->append(ns->add(pack));
			}
		}
	}

	PackBundle* bundle = pack->getBundle();
	if (bundle)
		packRec->set("bundle_name", ns->add(bundle->getName()));

	Ref<DataArray> filesArray = new DataArray();
	packRec->set("files", filesArray);

	StreamSourceMap files;
	pack->findLocal("*", files);

	for (StreamSourceMap::iterator fitr = files.begin(), fend = files.end(); fitr != fend; ++fitr)
	{
		StreamSource* file = fitr->second;
		Ref<DataRecord> fileRec = new DataRecord();

		fileRec->set("name", file->getName());
		fileRec->set("url", file->getUrl());
		fileRec->set("mime", ns->add(file->getContentType().getMimeType()));
		filesArray->append(fileRec);
	}
}

static void collectBundles(DataRecord* bundles, PackBundle* bundle)
{
	DataNamespace* ns = bundles->getNamespace();
	if (bundle && bundles->get(bundle->getName()).isVoid())
	{
		Ref<DataRecord> bundleRec = new DataRecord();
		bundles->set(bundle->getName(), bundleRec);

		bundleRec->set("name", ns->add(bundle->getName()));
		bundleRec->set("uid", bundle->getUid());
		bundleRec->set("base_uid", bundle->getBaseUid());
		bundleRec->set("revision", bundle->getRevision());

		PackBundle* base = bundle->getBase();
		if (base)
		{
			bundleRec->set("base", ns->add(base->getName()));
			collectBundles(bundles, base);
		}

		File* realFile = bundle->getRealFile();
		if (realFile)
			bundleRec->set("file_url", realFile->getUrl());
	}
}

void DebugServer::onRequestPacks(const RemoteRequestEvent* evt)
{
	if (_fileSystem == NULL)
		return evt->response(RESPONSE_ERROR, "no filesystem");

	StreamLocatorList packlist;
	_fileSystem->getPacks(packlist);

	// This packs represent merely raw file structrure - no requires, no dynamic contents.

	Ref<DataRecord> result = new DataRecord();

	Ref<DataRecord> packs = new DataRecord();
	result->set("packs", packs);

	Ref<DataRecord> bundles = new DataRecord();
	result->set("bundles", bundles);

	map<String, Package*>::type packOfName;
	for (StreamLocatorList::iterator itr = packlist.begin(), end = packlist.end(); itr != end; ++itr)
	{
		Package* pack = dynamic_cast<Package*>(itr->get());
		if (pack == NULL) continue;

		packOfName.insert(std::make_pair(pack->getName(), pack));
	}	

	for (StreamLocatorList::iterator itr = packlist.begin(), end = packlist.end(); itr != end; ++itr)
	{
		Package* pack = dynamic_cast<Package*>(itr->get());
		if (pack == NULL) continue;

		Ref<DataRecord> packRec = new DataRecord();
		packs->set(pack->getName(), packRec);

		populatePackRecord(packRec, pack, packOfName);

		collectBundles(bundles, pack->getBundle());
	}

	evt->response(RESPONSE_OK, result);
}

void DebugServer::onRequestFile(const RemoteRequestEvent* evt)
{
	if (_fileSystem == NULL)
		return evt->response(RESPONSE_ERROR, "no filesystem");

	Ref<DataRecord> rec = evt->param.toRecord();
	if (rec == NULL) 
		return evt->response(RESPONSE_ERROR);

	const String& pack	= rec->get("pack").toString();
	const String& file	= rec->get("file").toString();
	uint32 remote_crc	= rec->get("remote_crc").toInt();

	uint32 local_crc = 0;

	// TODO: associate with nit::PackageManager or inspect in debugger
	Ref<StreamSource> src = _fileSystem->getFile(pack, file, local_crc);
	if (src == NULL) 
		return evt->response(RESPONSE_FILE_NOT_FOUND);

	if (remote_crc && local_crc && remote_crc == local_crc)
		return evt->response(RESPONSE_FILE_SAME);

	Ref<StreamReader> reader;

	try
	{
		if (remote_crc && local_crc == 0)
		{
			// Peer expects crc but we don't have a local_crc yet, then calc here
			local_crc = src->calcCrc32();
			if (remote_crc == local_crc)
				return evt->response(RESPONSE_FILE_SAME);
		}

		reader = src->open();
	}
	catch (Exception& ex)
	{
		return evt->response(RESPONSE_FILE_ERROR, String("can't open file: ") + ex.getDescription());
	}
	catch (...)
	{
		return evt->response(RESPONSE_FILE_ERROR, "can't open file: unknown exception");
	}

	// Reuse rec - preserve pack, file
	rec->Delete("remote_crc"); // This field is unnecessary so remove
	rec->set("local_crc", (int)local_crc);
	rec->set("content_type", (int)src->getContentType());

	// Delay here, and later response upon upload success/failure
	Remote::ResponseId responseID = evt->delay();
	Remote::UploadId uploadID = _remote->upload(evt->peer, _channelID, UP_SVR_FILE, evt->requestId, reader, src->getStreamSize(), rec);

	_fileRequests.insert(std::make_pair(uploadID, responseID));
}

void DebugServer::onRemoteDownloadCancel(const RemoteUploadEvent* evt)
{
	FileRequests::iterator itr = _fileRequests.find(evt->uploadId);
	if (itr != _fileRequests.end())
	{
		_remote->delayedResponse(itr->second, RESPONSE_FILE_ERROR, "request cancelled");
		_fileRequests.erase(itr);
	}
}

void DebugServer::onRemoteDownloadEnd(const RemoteUploadEvent* evt)
{
	FileRequests::iterator itr = _fileRequests.find(evt->uploadId);
	if (itr != _fileRequests.end())
	{
		_remote->delayedResponse(itr->second, RESPONSE_OK);
		_fileRequests.erase(itr);
	}
}

void DebugServer::onRequestBreak(const RemoteRequestEvent* evt)
{
	if (_debugging)
		return evt->response(RESPONSE_ERROR, "ALREADY DEBUGGING");

	bool ok = _debugger->Break();

	if (ok)
	{
		evt->response(RESPONSE_OK);
		LOG(0, "++ [DBGSVR] Break accepted - breaking...\n");
	}
	else
	{
		evt->response(RESPONSE_ERROR);
		LOG(0, "++ [DBGSVR] Break denied\n");
	}
}

void DebugServer::onRequestGo(const RemoteRequestEvent* evt)
{
	bool ok = _debugger->go();

	if (!ok) 
		evt->response(RESPONSE_ERROR);
	else
		evt->response(RESPONSE_OK);
}

void DebugServer::onRequestStepInto(const RemoteRequestEvent* evt)
{
	bool ok = _debugger->stepInto();

	if (!ok) 
		evt->response(RESPONSE_ERROR);
	else
		evt->response(RESPONSE_OK);
}

void DebugServer::onRequestStepOver(const RemoteRequestEvent* evt)
{
	bool ok = _debugger->stepOver();

	if (!ok) 
		evt->response(RESPONSE_ERROR);
	else
		evt->response(RESPONSE_OK);
}

void DebugServer::onRequestStepOut(const RemoteRequestEvent* evt)
{
	bool ok = _debugger->stepOut();

	if (!ok) 
		evt->response(RESPONSE_ERROR);
	else
		evt->response(RESPONSE_OK);
}

void DebugServer::onRequestLocals(const RemoteRequestEvent* evt)
{

}

void DebugServer::onRequestInspect(const RemoteRequestEvent* evt)
{
	int inspectId = evt->param.get("inspect_id").toInt();

	DataValue value;
	bool ok = _debugger->inspect(inspectId, value);

	if (!ok) 
		evt->response(RESPONSE_ERROR);
	else
		evt->response(RESPONSE_OK, value);
}

void DebugServer::onRequestEvaluate(const RemoteRequestEvent* evt)
{

}

void DebugServer::breakTrap(const DataValue& params)
{
	if (_peer == NULL) return;

	ASSERT_THROW(!_debugging, EX_INVALID_STATE);

//	LOG(0, ".. [DBGSVR] Entering break trap\n");

	_debugging = true;
	_remote->notify(_peer, _channelID, NT_SVR_BREAK, params);

	NitRuntime* rt = NitRuntime::getSingleton();

	while (_active && _peer && _debugging)
	{
		bool alive = rt->debuggerLoop();
		if (!alive) break;
	}

	if (_peer)
	{
		if (_active)
			_remote->notify(_peer, _channelID, NT_SVR_RESUME);
		else
			_remote->notify(_peer, _channelID, NT_SVR_INACTIVE);
	}

	_debugging = false;

//	LOG(0, ".. [DBGSVR] Leaving break trap\n");
}

void DebugServer::Continue()
{
	_debugging = false;
}

void DebugServer::onNotifyClearBP(const RemoteNotifyEvent* evt)
{
	DataValue& params = evt->param;

	Ref<DataKey> type = params.get("type").toKey();

	if (type->getName() == "all")
		_breakpoints.clear();
	else
	{
		for (Breakpoints::iterator itr = _breakpoints.begin(); itr != _breakpoints.end(); )
		{
			Breakpoint& bp = itr->second;
			if (bp.type == type) 
				_breakpoints.erase(itr++);
			else
				++itr;
		}
	}

	_breakpointsUpdated = true;
}

void DebugServer::onNotifyAddBP(const RemoteNotifyEvent* evt)
{
	DataValue& params = evt->param;

	Breakpoint bp;
	bp.id		= params.get("id");
	bp.line	= params.get("line");
	bp.file	= params.get("file").toString();
	bp.pack	= params.get("pack").toString();
	bp.url	= params.get("url").toString();
	bp.type	= params.get("type").toKey();

	_breakpoints.insert(std::make_pair(bp.id, bp));
	_breakpointsUpdated = true;
}

void DebugServer::onNotifyRemoveBP(const RemoteNotifyEvent* evt)
{
	DataValue& params = evt->param;

	int bp_id = params.get("id");

	_breakpoints.erase(bp_id);
	_breakpointsUpdated = true;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;

// Ash nazg durba-tul-uk,				One ring to rule them all,
// Ash nazg gimba-tul,					One ring to find them,
// Ash nazg thraka-tul-uk				One ring to bring them all
// agh burzum-ishi krimpa-tul.			and in the darkness bind them.

// ash:one nazg:ring tul:them uk:all durba:rule gimba:find thraka:bring krimpa:bind agh:and ishi:in-the burzum:darkness
