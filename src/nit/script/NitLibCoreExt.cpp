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

#include "nit/script/NitLibCore.h"

#include "nit/script/NitBind.h"
#include "nit/script/NitBindMacro.h"

#include "nit/script/ScriptRuntime.h"

#include "nit/net/Remote.h"

#include "nit/net/RemoteDebugger.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::SocketBase, RefCounted, incRefCount, decRefCount);

class NB_SocketBase : TNitClass<SocketBase>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_VALUE(NIT_API, nit::DataToSend, NULL);

class NB_DataToSend : TNitClass<DataToSend>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] =
		{
			// NOTE: const char* or const void* is dangerous for script so do not provide constructor for them
			CONS_ENTRY_H(	"(buf: MemoryBuffer, size: int)"
			"\n"			"(rdr: StreamReader, size: int)"
			"\n"			"(buf: MemoryBuffer, offset: int, size: int)"
			"\n"			"(rdr: StreamReader, offset: int, size: int)"
			"\n"			"(str: string)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		switch (sq_gettop(v))
		{
		case 2:
			if (isString(v, 2))
				new (self(v)) DataToSend(getString(v, 2), sq_getsize(v, 2));
			else
				new (self(v)) DataToSend(get<MemoryBuffer>(v, 2));
			break;

		case 3:
			if (is<MemoryBuffer>(v, 2))
				new (self(v)) DataToSend(get<MemoryBuffer>(v, 2), getInt(v, 3));
			else
				new (self(v)) DataToSend(get<StreamReader>(v, 2), getInt(v, 3));
			break;

		case 4:
			if (is<MemoryBuffer>(v, 2))
				new (self(v)) DataToSend(get<MemoryBuffer>(v, 2), getInt(v, 3), getInt(v, 4));
			else
				new (self(v)) DataToSend(get<StreamReader>(v, 2), getInt(v, 3), getInt(v, 4));
			break;

		default:
			return sq_throwerror(v, "invalid arguments");
		}

		return SQ_OK;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::TcpSocket, SocketBase, incRefCount, decRefCount);

class NB_TcpSocket : TNitClass<TcpSocket>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(addr),
			PROP_ENTRY_R(port),
			PROP_ENTRY_R(connected),
			PROP_ENTRY_R(connecting),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(addr)					{ return push(v, self(v)->getAddr()); }
	NB_PROP_GET(port)					{ return push(v, self(v)->getPort()); }
	NB_PROP_GET(connected)				{ return push(v, self(v)->isConnected()); }
	NB_PROP_GET(connecting)				{ return push(v, self(v)->isConnecting()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::TcpSocketServer, SocketBase, incRefCount, decRefCount);

class NB_TcpSocketServer : TNitClass<TcpSocketServer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(bindAddr),
			PROP_ENTRY_R(bindPort),
			PROP_ENTRY_R(listening),
			PROP_ENTRY_R(numClients),
			PROP_ENTRY_R(clients),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(isClient,		"(socket: TcpSocket): bool"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(bindAddr)				{ return push(v, self(v)->getBindAddr()); }
	NB_PROP_GET(bindPort)				{ return push(v, self(v)->getBindPort()); }
	NB_PROP_GET(listening)				{ return push(v, self(v)->isListening()); }
	NB_PROP_GET(numClients)				{ return push(v, self(v)->getNumClients()); }

	NB_PROP_GET(clients)
	{
		sq_newarray(v, 0);
		const TcpSocketServer::Clients& clients = self(v)->getClients();
		for (TcpSocketServer::Clients::const_iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr)
		{
			arrayAppend(v, -1, itr->get());
		}
		return 1;
	}

	NB_FUNC(isClient)					{ return push(v, self(v)->isClient(get<TcpSocket>(v, 2))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WEAK(NIT_API, nit::Remote, NULL);

class NB_Remote : TNitClass<Remote>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(listening),
			PROP_ENTRY_R(numPeers),
			PROP_ENTRY_R(hostPeer),
			PROP_ENTRY_R(guestPeers),
			PROP_ENTRY_R(broadcastPeer),
			PROP_ENTRY	(packetDump),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(listen,			"(hostname: string, port=PORT_DEFAULT.TCP): bool"),
			FUNC_ENTRY_H(connect,			"(addr: string, port=PORT_DEFAULT.TCP): bool"),

			FUNC_ENTRY_H(shutdown,			"()"),
			FUNC_ENTRY_H(update,			"()"),

			FUNC_ENTRY_H(serverWhere,		"(hostQuery: string, myPort=PORT_DEFAULT.UDP, targetPort=PORT_DEFAULT.UDP): bool"),
			FUNC_ENTRY_H(serverHere,		"(hostinfo: string, broadcastAddr=\"255.255.255.255\", targetPort=PORT_DEFAULT.UDP): bool"),

			FUNC_ENTRY_H(sendUserPacket,	"(to: RemotePeer, hdrMsg: uint16, data: DataValue or DataToSend)"),
			FUNC_ENTRY_H(registerUserMsg,	"(hdrMsg: uint16)"),

			FUNC_ENTRY_H(notify,			"(to: RemotePeer, channel: uint16, cmd: uint16, params: DataValue or DataToSend=null)"),

			FUNC_ENTRY_H(request,			"(to: RemotePeer, channel: uint16, cmd: uint16, params: DataValue or DataToSend=null): RequestID"),
			FUNC_ENTRY_H(cancelRequest,		"(id: RequestID)"),
			FUNC_ENTRY_H(setRequestTimeout,	"(id: RequestID, millis: int) // clears timeout when millis == 0"),
			FUNC_ENTRY_H(getRequestStatus,	"(id: RequestID) : { StartTime: Timestamp } // null if not valid"),

			FUNC_ENTRY_H(delayedResponse,	"(id: ResponseID, code: int, params: DataValue or DataToSend=null)"),

			FUNC_ENTRY_H(upload,			"(to: RemotePeer, channel: uint16, cmd: uint16, requestID: uint32, reader: StreamReader, streamSize: uint32, params: DataValue or DataToSend=null): UploadID"),
			FUNC_ENTRY_H(setUploadTimeout,	"(id: UploadID, millis: int) // clears timeout when millis == 0"),
			FUNC_ENTRY_H(cancelUpload,		"(id: UploadID)"),
			FUNC_ENTRY_H(getUploadStatus,	"(id: UploadID) : { Sent: int, Size: int, StartTime: Timestamp } // null if not valid"),

			FUNC_ENTRY_H(cancelDownload,	"(id: DownloadID)"),
			FUNC_ENTRY_H(setDownloadTimeout,"(id: DownloadID, millis: int) // clears timeout when millis == 0"),
			FUNC_ENTRY_H(getDownloadStatus,	"(id: DownloadID) : { Received: int, Size: int, StartTime: Timestamp } // null if not valid"),

			FUNC_ENTRY_H(openChannel,		"(id: uint16, serviceName: string, info: DataValue=null): EventChannel"),
			FUNC_ENTRY_H(getChannel,		"(id: uint16): EventChannel"),
			FUNC_ENTRY_H(closeChannel,		"(id: uint16)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "CONNECT",			EVT::REMOTE_CONNECT);
		newSlot(v, -1, "DISCONNECT",		EVT::REMOTE_DISCONNECT);
		newSlot(v, -1, "HELLO",				EVT::REMOTE_HELLO);
		newSlot(v, -1, "ERROR",				EVT::REMOTE_ERROR);
		newSlot(v, -1, "USER_PACKET",		EVT::REMOTE_USER_PACKET);
		newSlot(v, -1, "CHANNEL_CLOSE",		EVT::REMOTE_CHANNEL_CLOSE);
		newSlot(v, -1, "NOTIFY",			EVT::REMOTE_NOTIFY);
		newSlot(v, -1, "REQUEST",			EVT::REMOTE_REQUEST);
		newSlot(v, -1, "REQUEST_CANCEL",	EVT::REMOTE_REQUEST_CANCEL);
		newSlot(v, -1, "RESPONSE",			EVT::REMOTE_RESPONSE);
		newSlot(v, -1, "UPLOAD_START",		EVT::REMOTE_UPLOAD_START);
		newSlot(v, -1, "UPLOAD_CANCEL",		EVT::REMOTE_UPLOAD_CANCEL);
		newSlot(v, -1, "UPLOAD_END",		EVT::REMOTE_UPLOAD_END);
		newSlot(v, -1, "DOWNLOAD_START",	EVT::REMOTE_DOWNLOAD_START);
		newSlot(v, -1, "DOWNLOAD_CANCEL",	EVT::REMOTE_DOWNLOAD_CANCEL);
		newSlot(v, -1, "DOWNLOAD_END",		EVT::REMOTE_DOWNLOAD_END);
		sq_poptop(v);

		addStaticTable(v, "RESPONSE");
		newSlot(v, -1, "OK",			(int)type::RESPONSE_OK);
		newSlot(v, -1, "ERROR",			(int)type::RESPONSE_ERROR);
		newSlot(v, -1, "NO_CHANNEL",	(int)type::RESPONSE_NO_CHANNEL);
		newSlot(v, -1, "NO_HANDLER",	(int)type::RESPONSE_NO_HANDLER);
		newSlot(v, -1, "TIMEOUT",		(int)type::RESPONSE_TIMEOUT);
		sq_poptop(v);

		addStaticTable(v, "PORT_DEFAULT");
		newSlot(v, -1, "TCP",			(int)type::PORT_DEFAULT_TCP);
		newSlot(v, -1, "UDP",			(int)type::PORT_DEFAULT_UDP);
		sq_poptop(v);

		addStaticTable(v, "HELLO");
		newSlot(v, -1, "SERVER_HERE",	(int)type::HELLO_SERVER_HERE);
		newSlot(v, -1, "SERVER_WHERE",	(int)type::HELLO_SERVER_WHERE);
		sq_poptop(v);
	}

	NB_PROP_GET(listening)				{ return push(v, self(v)->isListening()); }
	NB_PROP_GET(numPeers)				{ return push(v, self(v)->getNumPeers()); }
	NB_PROP_GET(hostPeer)				{ return push(v, self(v)->getHostPeer()); }
	NB_PROP_GET(broadcastPeer)			{ return push(v, self(v)->getBroadcastPeer()); }
	NB_PROP_GET(packetDump)				{ return push(v, self(v)->getPacketDump()); }

	NB_PROP_GET(guestPeers)
	{ 
		const type::RemotePeers& peers = self(v)->getGuestPeers();
		
		sq_newarray(v, 0);
		for (type::RemotePeers::const_iterator itr = peers.begin(), end = peers.end(); itr != end; ++itr)
			arrayAppend(v, -1, itr->get());
		return 1;
	}

	NB_PROP_SET(listening)				{ self(v)->setListening(getBool(v, 2)); return 0; }
	NB_PROP_SET(packetDump)				{ self(v)->setPacketDump(getBool(v, 2)); return 0; }

	NB_FUNC(listen)						{ return push(v, self(v)->listen(getString(v, 2), optInt(v, 3, type::PORT_DEFAULT_TCP))); }
	NB_FUNC(connect)					{ return push(v, self(v)->connect(getString(v, 2), optInt(v, 3, type::PORT_DEFAULT_TCP))); }

	NB_FUNC(shutdown)					{ self(v)->shutdown(); return 0; }
	NB_FUNC(update)						{ self(v)->update(); return 0; }

	NB_FUNC(serverWhere)				{ return push(v, self(v)->serverWhere(getString(v, 2), optInt(v, 3, type::PORT_DEFAULT_UDP), optInt(v, 4, type::PORT_DEFAULT_UDP))); }
	NB_FUNC(serverHere)					{ return push(v, self(v)->serverHere(getString(v, 2), optString(v, 3, "255.255.255.255"), optInt(v, 4, type::PORT_DEFAULT_UDP))); }

	NB_FUNC(sendUserPacket)
	{
		if (isNone(v, 4) || is<DataToSend>(v, 4))
			self(v)->sendUserPacket(get<RemotePeer>(v, 2), getInt(v, 3), opt<DataToSend>(v, 4, NULL));
		else
		{
			DataValue value;
			SQRESULT sr = ScriptDataValue::toValue(v, 4, value);
			if (SQ_FAILED(sr)) return sr;
			self(v)->sendUserPacket(get<RemotePeer>(v, 2), getInt(v, 3), value);
		}
		return 0;
	}

	NB_FUNC(registerUserMsg)			{ self(v)->registerUserMsg(getInt(v, 2)); return 0; }

	NB_FUNC(notify)					
	{ 
		if (isNone(v, 5) || is<DataToSend>(v, 5))
			self(v)->notify(get<RemotePeer>(v, 2), getInt(v, 3), getInt(v, 4), opt<DataToSend>(v, 5, NULL)); 
		else
		{
			DataValue value;
			SQRESULT sr = ScriptDataValue::toValue(v, 5, value);
			if (SQ_FAILED(sr)) return sr;
			self(v)->notify(get<RemotePeer>(v, 2), getInt(v, 3), getInt(v, 4), value);
		}
		return 0; 
	}

	NB_FUNC(request)					
	{ 
		if (isNone(v, 5) || is<DataToSend>(v, 5))
			return push(v, self(v)->request(get<RemotePeer>(v, 2), getInt(v, 3), getInt(v, 4), opt<DataToSend>(v, 5, NULL))); 

		DataValue value;
		SQRESULT sr = ScriptDataValue::toValue(v, 5, value);
		if (SQ_FAILED(sr)) return sr;
		return push(v, self(v)->request(get<RemotePeer>(v, 2), getInt(v, 3), getInt(v, 4), value));
	}

	NB_FUNC(setRequestTimeout)			{ self(v)->setRequestTimeout(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(cancelRequest)				{ self(v)->cancelRequest(getInt(v, 2)); return 0; }

	NB_FUNC(delayedResponse)			
	{ 
		if (isNone(v, 4) || is<DataToSend>(v, 4))
			self(v)->delayedResponse(getInt(v, 2), getInt(v, 3), opt<DataToSend>(v, 4, NULL)); 
		else
		{
			DataValue value;
			SQRESULT sr = ScriptDataValue::toValue(v, 4, value);
			if (SQ_FAILED(sr)) return sr;
			self(v)->delayedResponse(getInt(v, 2), getInt(v, 3), value);
		}
		return 0; 
	}

	NB_FUNC(upload)
	{ 
		if (isNone(v, 7) || is<DataToSend>(v, 8))
			return push(v, self(v)->upload(get<RemotePeer>(v, 2), getInt(v, 3), getInt(v, 4), getInt(v, 5), get<StreamReader>(v, 6), getInt(v, 7), opt<DataToSend>(v, 8, NULL))); 

		DataValue value;
		SQRESULT sr = ScriptDataValue::toValue(v, 8, value);
		if (SQ_FAILED(sr)) return sr;
		return push(v, self(v)->upload(get<RemotePeer>(v, 2), getInt(v, 3), getInt(v, 4), getInt(v, 5), get<StreamReader>(v, 6), getInt(v, 7), value));
	}

	NB_FUNC(setUploadTimeout)			{ self(v)->setUploadTimeout(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(cancelUpload)				{ self(v)->cancelUpload(getInt(v, 2)); return 0; }

	NB_FUNC(setDownloadTimeout)			{ self(v)->setDownloadTimeout(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(cancelDownload)				{ self(v)->cancelDownload(getInt(v, 2)); return 0; }

	NB_FUNC(getRequestStatus)
	{
		Timestamp startTime;
		bool valid = self(v)->getRequestStatus(getInt(v, 2), &startTime);
		if (!valid) return 0;

		sq_newtable(v);
		newSlot(v, -1, "StartTime", startTime);
		return 1;
	}

	NB_FUNC(getUploadStatus)
	{
		size_t sent, size;
		Timestamp startTime;
		bool valid = self(v)->getUploadStatus(getInt(v, 2), &sent, &size, &startTime);
		if (!valid) return 0; 

		sq_newtable(v);
		newSlot(v, -1, "Sent", sent);
		newSlot(v, -1, "Size", size);
		newSlot(v, -1, "StartTime", startTime);
		return 1;
	}

	NB_FUNC(getDownloadStatus)
	{
		size_t recv, size;
		Timestamp startTime;
		bool valid = self(v)->getUploadStatus(getInt(v, 2), &recv, &size, &startTime);
		if (!valid) return 0; 

		sq_newtable(v);
		newSlot(v, -1, "Received", recv);
		newSlot(v, -1, "Size", size);
		newSlot(v, -1, "StartTime", startTime);
		return 1;
	}

	NB_FUNC(openChannel)
	{ 
		DataValue value;
		if (!isNone(v, 4))
		{
			SQRESULT sr = ScriptDataValue::toValue(v, 4, value);
			if (SQ_FAILED(sr)) return sr;
		}

		return push(v, self(v)->openChannel(getInt(v, 2), getString(v, 3), value)); 
	}

	NB_FUNC(getChannel)					{ return push(v, self(v)->getChannel(getInt(v, 2))); }
	NB_FUNC(closeChannel)				{ self(v)->closeChannel(getInt(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WEAK(NIT_API, nit::RemotePeer, NULL);

class NB_RemotePeer : TNitClass<RemotePeer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(remote),
			PROP_ENTRY_R(socket),
			PROP_ENTRY_R(properties),
			PROP_ENTRY_R(dataContext),
			PROP_ENTRY_R(connected),
			PROP_ENTRY	(sendCompressed),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(isHost,	"(): bool"),
			FUNC_ENTRY_H(isGuest,	"(): bool"),
			FUNC_ENTRY_H(isBroadcast,"(): bool"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(remote)					{ return push(v, self(v)->getRemote()); }
	NB_PROP_GET(socket)					{ return push(v, self(v)->getSocket()); }
	NB_PROP_GET(properties)				{ return push(v, self(v)->getProperties()); }
	NB_PROP_GET(dataContext)			{ return push(v, self(v)->getDataContext()); }
	NB_PROP_GET(connected)				{ return push(v, self(v)->isConnected()); }
	NB_PROP_GET(sendCompressed)			{ return push(v, self(v)->isSendCompressed()); }

	NB_PROP_SET(sendCompressed)			{ self(v)->setSendCompressed(getBool(v, 2)); return 0; }

	NB_FUNC(isHost)						{ return push(v, self(v)->isHost()); }
	NB_FUNC(isGuest)					{ return push(v, self(v)->isGuest()); }
	NB_FUNC(isBroadcast)				{ return push(v, self(v)->isBroadcast()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WEAK(NIT_API, nit::Remote::PacketIO, NULL);

class NB_RemotePacketIO : TNitClass<Remote::PacketIO>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(pos),
			PROP_ENTRY_R(dataLeft),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(write,			"(writer: StreamWriter, size: int)"),
			FUNC_ENTRY_H(openReader,	"(size: int): StreamReader"),
			FUNC_ENTRY_H(skip,			"(size: int)"),
			FUNC_ENTRY_H(readString,	"(size: int): string"),
			FUNC_ENTRY_H(readValue,		"(): DataValue"),
			FUNC_ENTRY_H(readI32,		"(): int"),
			FUNC_ENTRY_H(readU32,		"(): int"),
			FUNC_ENTRY_H(readI16,		"(): int"),
			FUNC_ENTRY_H(readU16,		"(): int"),
			FUNC_ENTRY_H(readI8,		"(): int"),
			FUNC_ENTRY_H(readU8,		"(): int"),
			FUNC_ENTRY_H(readF32,		"(): float"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(pos)					{ return push(v, self(v)->getPos()); }
	NB_PROP_GET(dataLeft)				{ return push(v, self(v)->getDataLeft()); }

	NB_FUNC(write)						{ self(v)->write(get<StreamWriter>(v, 2), getInt(v, 2)); return 0; }
	NB_FUNC(openReader)					{ return push(v, self(v)->openReader(getInt(v, 2))); }
	NB_FUNC(skip)						{ self(v)->skip(getInt(v, 2)); return 0; }

	NB_FUNC(readString)					{ return push(v, self(v)->readString(getInt(v, 2))); }
	NB_FUNC(readValue)					{ return push(v, self(v)->readValue()); }
	NB_FUNC(readI32)					{ return push(v, (int)self(v)->read<int32>()); }
	NB_FUNC(readU32)					{ return push(v, (int)self(v)->read<uint32>()); }
	NB_FUNC(readI16)					{ return push(v, (int)self(v)->read<int16>()); }
	NB_FUNC(readU16)					{ return push(v, (int)self(v)->read<uint16>()); }
	NB_FUNC(readI8)						{ return push(v, (int)self(v)->read<int8>()); }
	NB_FUNC(readU8)						{ return push(v, (int)self(v)->read<uint8>()); }
	NB_FUNC(readF32)					{ return push(v, (float)self(v)->read<float>()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::RemoteEvent, Event, incRefCount, decRefCount);

class NB_RemoteEvent : TNitClass<RemoteEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(remote),
			PROP_ENTRY_R(peer),
			NULL
		};

		bind(v, props, NULL);
	}

	NB_PROP_GET(remote)					{ return push(v, self(v)->remote.get()); }
	NB_PROP_GET(peer)					{ return push(v, self(v)->peer.get()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::RemoteHelloEvent, Event, incRefCount, decRefCount);

class NB_RemoteHelloEvent : TNitClass<RemoteHelloEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(remote),
			PROP_ENTRY_R(hostInfo),
			PROP_ENTRY_R(addr),
			PROP_ENTRY_R(port),
			PROP_ENTRY_R(msgId),
			NULL
		};

		bind(v, props, NULL);

		addStaticTable(v, "HELLO");
		newSlot(v, -1, "SERVER_WHERE",	(int)Remote::HELLO_SERVER_WHERE);
		newSlot(v, -1, "SERVER_HERE",	(int)Remote::HELLO_SERVER_HERE);
		sq_poptop(v);
	}

	NB_PROP_GET(remote)					{ return push(v, self(v)->remote.get()); }
	NB_PROP_GET(hostInfo)				{ return push(v, self(v)->hostInfo); }
	NB_PROP_GET(addr)					{ return push(v, self(v)->addr); }
	NB_PROP_GET(port)					{ return push(v, self(v)->port); }
	NB_PROP_GET(msgId)					{ return push(v, (int)self(v)->msgId); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::RemoteErrorEvent, RemoteEvent, incRefCount, decRefCount);

class NB_RemoteErrorEvent : TNitClass<RemoteErrorEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(error),
			PROP_ENTRY_R(message),
			NULL
		};

		bind(v, props, NULL);
	}

	NB_PROP_GET(error)					{ return push(v, self(v)->error); }
	NB_PROP_GET(message)				{ return push(v, self(v)->message); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::RemoteUserPacketEvent, RemoteEvent, incRefCount, decRefCount);

class NB_RemoteUserPacketEvent : TNitClass<RemoteUserPacketEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(hdrMsg),
			PROP_ENTRY_R(packet),
			NULL
		};

		bind(v, props, NULL);
	}

	NB_PROP_GET(hdrMsg)					{ return push(v, self(v)->hdrMsg); }
	NB_PROP_GET(packet)					{ return push(v, self(v)->packet.get()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::RemoteChannelEvent, RemoteEvent, incRefCount, decRefCount);

class NB_RemoteChannelEvent : TNitClass<RemoteChannelEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(channelId),
			NULL
		};

		bind(v, props, NULL);
	}

	NB_PROP_GET(channelId)				{ return push(v, self(v)->channelId); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::RemoteNotifyEvent, RemoteEvent, incRefCount, decRefCount);

class NB_RemoteNotifyEvent : TNitClass<RemoteNotifyEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(channelId),
			PROP_ENTRY_R(command),
			PROP_ENTRY_R(packet),
			NULL
		};

		bind(v, props, NULL);
	}

	NB_PROP_GET(channelId)				{ return push(v, self(v)->channelId); }
	NB_PROP_GET(command)				{ return push(v, self(v)->command); }
	NB_PROP_GET(packet)					{ return push(v, self(v)->packet.get()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::RemoteRequestEvent, RemoteNotifyEvent, incRefCount, decRefCount);

class NB_RemoteRequestEvent : TNitClass<RemoteRequestEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(requestId),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(delay,			"(): ResponseID"),
			FUNC_ENTRY_H(response,		"(code: int, params: DataValue or DataToSend=null)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(requestId)				{ return push(v, self(v)->requestId); }

	NB_FUNC(delay)						{ return push(v, self(v)->delay()); }

	NB_FUNC(response)
	{ 
		if (isNone(v, 4) || is<DataToSend>(v, 4))
			self(v)->response(getInt(v, 2), opt<DataToSend>(v, 4, NULL)); 
		else
		{
			DataValue value;
			SQRESULT sr = ScriptDataValue::toValue(v, 4, value);
			if (SQ_FAILED(sr)) return sr;
			self(v)->response(getInt(v, 2), value);
		}
		return 0; 
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::RemoteRequestCancelEvent, RemoteEvent, incRefCount, decRefCount);

class NB_RemoteRequestCancelEvent : TNitClass<RemoteRequestCancelEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(channelId),
			PROP_ENTRY_R(command),
			PROP_ENTRY_R(responseId),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(channelId)				{ return push(v, self(v)->channelId); }
	NB_PROP_GET(command)				{ return push(v, self(v)->command); }
	NB_PROP_GET(responseId)				{ return push(v, self(v)->responseId); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::RemoteResponseEvent, RemoteNotifyEvent, incRefCount, decRefCount);

class NB_RemoteResponseEvent : TNitClass<RemoteResponseEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(requestId),
			PROP_ENTRY_R(code),
			NULL
		};

		bind(v, props, NULL);
	}

	NB_PROP_GET(requestId)				{ return push(v, self(v)->requestId); }
	NB_PROP_GET(code)					{ return push(v, self(v)->code); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::RemoteUploadStartEvent, RemoteNotifyEvent, incRefCount, decRefCount);

class NB_RemoteUploadStartEvent : TNitClass<RemoteUploadStartEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(requestId),
			PROP_ENTRY_R(uploadId),
			PROP_ENTRY_R(streamSize),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(download,		"(writer: StreamWriter, offset=0, size=0, packetSize=16384): Remote.DownloadID"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(requestId)				{ return push(v, self(v)->requestId); }
	NB_PROP_GET(uploadId)				{ return push(v, self(v)->uploadId); }
	NB_PROP_GET(streamSize)				{ return push(v, self(v)->streamSize); }

	NB_FUNC(download)					{ return push(v, self(v)->download(get<StreamWriter>(v, 2), optInt(v, 3, 0), optInt(v, 4, 0), optInt(v, 5, 16384))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::RemoteUploadEvent, RemoteEvent, incRefCount, decRefCount);

class NB_RemoteUploadEvent : TNitClass<RemoteUploadEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(uploadId),
			PROP_ENTRY_R(downloadId),
			NULL
		};

		bind(v, props, NULL);
	}

	NB_PROP_GET(uploadId)				{ return push(v, self(v)->uploadId); }
	NB_PROP_GET(downloadId)				{ return push(v, self(v)->downloadId); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WEAK(NIT_API, nit::DebugServer, NULL);

class NB_DebugServer : TNitClass<DebugServer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(remote),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(remote)					{ return push(v, self(v)->getRemote()); }
};

////////////////////////////////////////////////////////////////////////////////

SQRESULT NitLibCoreExt(HSQUIRRELVM v)
{
	NB_DataToSend::Register(v);
	NB_SocketBase::Register(v);
	NB_TcpSocket::Register(v);
	NB_TcpSocketServer::Register(v);

	NB_Remote::Register(v);
	NB_RemotePeer::Register(v);
	NB_RemotePacketIO::Register(v);

	NB_RemoteEvent::Register(v);
	NB_RemoteHelloEvent::Register(v);
	NB_RemoteErrorEvent::Register(v);
	NB_RemoteUserPacketEvent::Register(v);
	NB_RemoteChannelEvent::Register(v);
	NB_RemoteNotifyEvent::Register(v);
	NB_RemoteRequestEvent::Register(v);
	NB_RemoteRequestCancelEvent::Register(v);
	NB_RemoteResponseEvent::Register(v);
	NB_RemoteUploadStartEvent::Register(v);
	NB_RemoteUploadEvent::Register(v);

	NB_DebugServer::Register(v);

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;