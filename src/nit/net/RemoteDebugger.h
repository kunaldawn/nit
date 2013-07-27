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

#include "nit/net/Remote.h"

////////////////////////////////////////////////////////////////////////////////

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NIT_API IDebugger
{
public:
	virtual bool						Break() = 0;
	virtual bool						go() = 0;
	virtual bool						stepInto() = 0;
	virtual bool						stepOver() = 0;
	virtual bool						stepOut() = 0;

	virtual bool						inspect(int inspectId, DataValue& outValue) = 0;

	virtual void						updateBreakpoints() = 0;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API IDebuggerFileSystem
{
public:
	virtual Ref<StreamSource>			getFile(const String& pack, const String& file, uint32& outCRC32) = 0;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API DebugServer : public WeakSupported
{
public:
	enum Cmd
	{
		RQ_ATTACH						= 0x0001,
		NT_DETACH						= 0x0002,

		NT_COMMAND						= 0x0010,

		RQ_PACKS						= 0x0020,	// Request all pack list the debuggee currently can figure out
		RQ_FILE							= 0x0021,

		NT_CLEAR_BP						= 0x0030,
		NT_ADD_BP						= 0x0031,
		NT_DEL_BP						= 0x0032,

		NT_CLEAR_WATCH					= 0x0040,
		NT_ADD_WATCH					= 0x0041,
		NT_DEL_WATCH					= 0x0042,

		RQ_BREAK						= 0x0050,

		RQ_GO							= 0x0051,
		RQ_STEP_INTO					= 0x0052,
		RQ_STEP_OVER					= 0x0053,
		RQ_STEP_OUT						= 0x0054,

		RQ_LOCALS						= 0x0060,	// Inspect local variables on a stack frame specified by stack_id
		RQ_INSPECT						= 0x0061,	// Inspect properties of an object specified by inspect_id
		RQ_EVALUATE						= 0x0062,	// Evaluate given string and return its value

		NT_SVR_ACTIVE					= 0x1001,
		NT_SVR_LOG_ENTRY				= 0x1002,
		NT_SVR_BREAK					= 0x1003,
		NT_SVR_RESUME					= 0x1004,
		NT_SVR_INACTIVE					= 0x1005,
		NT_SVR_SHUTDOWN					= 0x1006,

		UP_SVR_FILE						= 0x1021,
	};

	enum ResponseCode
	{
		RESPONSE_OK						= Remote::RESPONSE_OK,
		RESPONSE_ERROR					= Remote::RESPONSE_ERROR,

		RESPONSE_FILE_SAME				= 1,
		RESPONSE_FILE_NOT_FOUND			= -101,
		RESPONSE_FILE_ERROR				= -102,
	};

	enum
	{
		PORT_DEFAULT					= 51220,
	};

public:
	DebugServer(Remote::ChannelId channelID = 0xdeb6);
	~DebugServer();

public:
	void								listen(const String& addr = StringUtil::BLANK(), uint16 port = PORT_DEFAULT);

public:
	void								attach(RemotePeer* peer, DataValue params);
	void								detach();

	Remote::ChannelId					getChannelID()							{ return _channelID; }
	RemotePeer*							getAttached()							{ return _peer; }

	void								update();

	bool								isActive()								{ return _active; }
	bool								isDebugging()							{ return _debugging; }

	Remote*								getRemote()								{ return _remote; }

	IDebugger*							getDebugger()							{ return _debugger; }
	void								setDebugger(IDebugger* debugger);

	IDebuggerFileSystem*				getFileSystem()							{ return _fileSystem; }
	void								setFileSystem(IDebuggerFileSystem* fs)	{ _fileSystem = fs; }

	void								breakTrap(const DataValue& params);		// TODO: temporary impl
	void								Continue();

public:
	class NIT_API Breakpoint
	{
	public:
		String							file;
		String							pack;
		String							url;
		int								id;
		int								line;
		Ref<DataKey>					type;
	};

	typedef unordered_map<int, Breakpoint>::type Breakpoints;

	const Breakpoints&					getBreakpoints()						{ return _breakpoints; }

protected:
	Remote*								_remote;
	Remote::ChannelId					_channelID;

	Ref<RemotePeer>						_peer;
	IDebugger*							_debugger;
	IDebuggerFileSystem*				_fileSystem;
	
	bool								_active;
	bool								_debugging;

	Logger*								_logger;

	list<String>::type					_pendingCommands;

	typedef unordered_map<Remote::UploadId, Remote::ResponseId>::type FileRequests;
	FileRequests						_fileRequests;

	Breakpoints							_breakpoints;
	bool								_breakpointsUpdated;

private:
	void								onRemoteRequest(const RemoteRequestEvent* evt);
	void								onRemoteNotify(const RemoteNotifyEvent* evt);
	void								onRemoteDisconnect(const RemoteEvent* evt);
	void								onRemoteDownloadCancel(const RemoteUploadEvent* evt);
	void								onRemoteDownloadEnd(const RemoteUploadEvent* evt);

private:
	void								onNotifyCommand(const RemoteNotifyEvent* evt);

	void								onNotifyClearBP(const RemoteNotifyEvent* evt);
	void								onNotifyAddBP(const RemoteNotifyEvent* evt);
	void								onNotifyRemoveBP(const RemoteNotifyEvent* evt);

	void								onRequestPacks(const RemoteRequestEvent* evt);
	void								onRequestFile(const RemoteRequestEvent* evt);

	void								onRequestBreak(const RemoteRequestEvent* evt);

	void								onRequestGo(const RemoteRequestEvent* evt);
	void								onRequestStepInto(const RemoteRequestEvent* evt);
	void								onRequestStepOver(const RemoteRequestEvent* evt);
	void								onRequestStepOut(const RemoteRequestEvent* evt);

	void								onRequestLocals(const RemoteRequestEvent* evt);
	void								onRequestInspect(const RemoteRequestEvent* evt);
	void								onRequestEvaluate(const RemoteRequestEvent* evt);

};

////////////////////////////////////////////////////////////////////////////////

struct NIT_API RemoteLogEntry
{
	enum _Flags 
	{
		FLAG_LINESTART					= 0x01,
		FLAG_LINEEND					= 0x02,
	};

	uint8								logLevel;
	uint8								flags;

	uint32								messageLen;
	const char*							message;


	float								time;
	const char*							channel;
	const char*							act;
	const char*							tagStr;

	const char*							srcName;
	const char*							fnName;
	uint32								line;

	RemoteLogEntry(const void* packet, size_t packetSize);

	int									formatHeader(char* buf, int bufSize);

};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
