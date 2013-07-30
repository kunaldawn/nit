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
#include "nit/event/Event.h"
#include "nit/net/Socket.h"
#include "nit/data/DataValue.h"
#include "nit/io/ZStream.h"

////////////////////////////////////////////////////////////////////////////////

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class Remote;
class RemotePeer;
class RemoteRequestEvent;
class RemoteUploadStartEvent;

class DataContext;

////////////////////////////////////////////////////////////////////////////////

class NIT_API RemotePeer : public RefCounted, public TcpSocket::IListener
{
public:
	RemotePeer(Remote* remote, TcpSocket* socket, bool isHost);
	virtual ~RemotePeer();

public:
	Remote*								getRemote()								{ return _remote; }
	TcpSocket*							getSocket()								{ return _socket; }

	DataRecord*							getProperties()							{ return _properties; }

	void								disconnect();

	bool								isHost()								{ return _isHost; }
	virtual bool						isGuest()								{ return !_isHost; }
	virtual bool						isBroadcast()							{ return false; }

	virtual bool						isConnected()							{ return _socket->isConnected() || _socket->isConnecting(); }

	bool								isSendCompressed()						{ return _sendCompressed; }
	virtual void						setSendCompressed(bool flag);

	DataContext*						getDataContext();

protected:
	friend class Remote;

	virtual bool						sendPacket(uint16 msg, const void* hdr, size_t hdrSize, const void* data, size_t dataSize);
	virtual bool						sendPacket(uint16 msg, const void* hdr, size_t hdrSize, DataToSend* data);
	virtual bool						sendPacket(uint16 msg, const void* hdr, size_t hdrSize, const DataValue& value);

public:
	size_t								_readValue(MemoryBuffer* buf, size_t pos, DataValue& outValue); // TODO: hide

public:									// TcpSocket::IListener impl
	virtual bool						onSend(TcpSocket* socket);
	virtual bool						onRecv(TcpSocket* socket);
	virtual void						onError(TcpSocket* socket, char* msg, int err);
	virtual void						onDisconnect(TcpSocket* socket);

protected:
	Weak<Remote>						_remote;
	Ref<TcpSocket>						_socket;
	Ref<DataRecord>						_properties;

	Ref<ZStreamReader>					_zIn;
	Ref<MemoryBuffer>					_zRecvBuf;

	size_t								_zSendSize;
	Ref<ZStreamWriter>					_zOut;

	class DataImpl;
	DataImpl*							_dataImpl;
	Ref<MemoryBuffer::Reader>			_dataValueReader;
	Ref<MemoryBuffer::Writer>			_dataValueWriter;
	Ref<MemoryBuffer::Reader>			_zDataValueReader;
	Ref<MemoryBuffer::Writer>			_zDataValueWriter;

	bool								_isHost;
	bool								_sendCompressed;

	void								zBegin();
	void								zEnd();
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API Remote : public WeakSupported, 
	public TcpSocketServer::IListener, public UdpSocket::IListener
{
public:
	Remote();
	virtual ~Remote();

	enum
	{
		PORT_DEFAULT_TCP				= 51219,
		PORT_DEFAULT_UDP				= 51218,
	};

public:
	bool								serverWhere(const String& hostQuery, uint16 myPort = PORT_DEFAULT_UDP, uint16 targetPort = PORT_DEFAULT_UDP);

	bool								serverHere(const String& hostinfo, const String& targetAddr = "255.255.255.255", uint16 targetPort = PORT_DEFAULT_UDP);

public:
	bool								listen(const String& myhostname, uint16 port = PORT_DEFAULT_TCP);
	bool								connect(const String& addr);

	void								update();
	void								shutdown();

	bool								isListening()							{ return _server && _server->isListening(); }

	uint								getNumPeers()							{ return _guestPeers.size() + (_hostPeer ? 1 : 0); }

public:
	bool								getPacketDump()							{ return _packetDump; }
	void								setPacketDump(bool flag)				{ _packetDump = flag; }
	bool								isRemoteLogSuppressed()					{ return _suppressRemoteLog; }

public:
	RemotePeer*							getBroadcastPeer()						{ return _broadcastPeer; }
	TcpSocketServer*					getMyHostServer()						{ return _server; }

	typedef set<Ref<RemotePeer> >::type	RemotePeers;
	RemotePeer*							getHostPeer()							{ return _hostPeer; }
	const RemotePeers&					getGuestPeers()							{ return _guestPeers; }

public:									// UserPacket - Receiver should register its packet handler (if not Invalid Msg error occurs)
	void								sendUserPacket(RemotePeer* to, uint16 hdrMsg, const DataValue& data = DataValue::Void());
	void								registerUserMsg(uint16 hdrMsg);

public:
	typedef uint16						ChannelId;								// Channel 0 is always opened

	EventChannel*						openChannel(ChannelId id, const String& serviceName, const DataValue& info = DataValue::Void());
	inline EventChannel*				getChannel(ChannelId id);
	void								closeChannel(ChannelId id);

public:									// notify protocol
	typedef uint16						CommandId;

	void								notify(RemotePeer* to, ChannelId channel, CommandId cmd, const DataValue& param = DataValue::Void());

public:									// request-response protocol
	typedef uint32						RequestId;
	RequestId							request(RemotePeer* to, ChannelId channel, CommandId cmd, const DataValue& param = DataValue::Void());
	void								cancelRequest(RequestId id);
	bool								getRequestStatus(RequestId id, Timestamp* outStartTime = NULL);
	void								setRequestTimeout(RequestId id, uint millis);

	typedef uint32						ResponseId;
	void								delayedResponse(ResponseId id, int32 code, const DataValue& param = DataValue::Void());

public:									// upload-download protocol
	typedef uint32						UploadId;
	UploadId							upload(RemotePeer* to, ChannelId channel, CommandId cmd, RequestId requestId, StreamReader* reader, uint32 streamSize, const DataValue& param);
	void								setUploadTimeout(UploadId id, uint millis);
	void								cancelUpload(UploadId id);
	bool								getUploadStatus(UploadId id, size_t* outSent = NULL, size_t* outSize = NULL, Timestamp* outStartTime = NULL);

	typedef uint32						DownloadId;
	void								cancelDownload(DownloadId id);
	void								setDownloadTimeout(DownloadId id, uint millis);
	bool								getDownloadStatus(DownloadId id, size_t* outReceived = NULL, size_t* outSize = NULL, Timestamp* outStartTime = NULL);

public:
	class PacketReader : public WeakSupported
	{
	public:
		PacketReader(RemotePeer* peer, MemoryBuffer* recvBuf, size_t pos, size_t packetLen, size_t dataLen);

		size_t							getDataLeft()							{ return _dataLeft; }

		template <typename TValue>
		TValue							read()									{ TValue value; read(&value, sizeof(value)); return value; }

		void							read(void* buf, size_t size);
		void							copyWriteTo(StreamWriter* writer, size_t size);
		void							readValue(DataValue& outValue);

		void							consume();

	private:
		RemotePeer*						_peer;
		MemoryBuffer*					_recvBuf;
		size_t							_pos;
		size_t							_packetLen;
		size_t							_dataLeft;
	};

protected:
	friend class RemotePeer;
	bool								onRecv(RemotePeer* peer, MemoryBuffer* recvBuf);
	void								onError(RemotePeer* peer, char* msg, int err);
	void								onDisconnect(RemotePeer* peer);

protected:								// TcpSocketServer::IListener impl
	virtual TcpSocket*					onAccept(TcpSocketServer* server, int socketHandle, const String& peerAddr, uint16 peerPort);
	virtual void						onError(TcpSocketServer* server, char* msg, int err);

protected:								// UdpSocket::IListener impl
	virtual void						onUdpRecv(UdpSocket* socket, const String& peerAddr, uint16 peerPort, uint8* msg, size_t len);

protected:
	void								response(RemotePeer* to, ChannelId channel, CommandId cmd, RequestId requestId, uint32 code, DataToSend* msg);
	void								response(RemotePeer* to, ChannelId channel, CommandId cmd, RequestId requestId, uint32 code, const DataValue& msg);

protected:
	void								recvWelcome(RemotePeer* from, PacketReader* packet);

	void								recvChannelClose(RemotePeer* from, PacketReader* packet);

	void								recvNotify(RemotePeer* from, PacketReader* packet);

	void								recvRequest(RemotePeer* from, PacketReader* packet);
	void								recvRequestCancel(RemotePeer* from, PacketReader* packet);
	void								recvResponse(RemotePeer* from, PacketReader* packet);

	void								recvUploadStart(RemotePeer* from, PacketReader* packet);
	void								recvUploadPacket(RemotePeer* from, PacketReader* packet);
	void								recvUploadCancel(RemotePeer* from, PacketReader* packet);

	void								recvDownloadStart(RemotePeer* from, PacketReader* packet);
	void								recvDownloadCancel(RemotePeer* from, PacketReader* packet);
	void								recvDownloadEnd(RemotePeer* from, PacketReader* packet);

	void								processUploads();

protected:
	struct ChannelEntry
	{
		Ref<EventChannel>				channel;
		uint16							channelId;
		String							serviceName;
		DataValue						info;
	};

	typedef unordered_map<uint16, ChannelEntry>::type Channels;

	struct RequestEntry
	{
		Weak<EventChannel>				channel;
		Weak<RemotePeer>				to;
		uint16							channelId;
		uint16							command;
		uint32							requestId;
		Timestamp						startTime;
	};

	typedef unordered_map<uint32, RequestEntry>::type Requests;

	struct ResponseEntry
	{
		Weak<EventChannel>				channel;
		Weak<RemotePeer>				from;
		uint16							channelId;
		uint16							command;
		uint32							requestId;
		uint32							responseId;
	};

	typedef unordered_map<uint32, ResponseEntry>::type Responses;

	struct UploadEntry
	{
		Weak<EventChannel>				channel;
		Weak<RemotePeer>				to;
		Ref<StreamReader>				reader;
		uint16							channelId;
		uint16							command;
		uint32							requestId;
		uint32							uploadId;
		uint32							downloadId;
		uint32							streamSize;
		uint32							offset;
		uint32							bytesLeft;
		uint16							packetSize;
		Timestamp						startTime;
	};

	typedef unordered_map<uint32, UploadEntry>::type Uploads;

	struct DownloadEntry
	{
		Weak<EventChannel>				channel;
		Weak<RemotePeer>				from;
		Ref<StreamWriter>				writer;
		uint16							channelId;
		uint16							command;
		uint32							uploadId;
		uint32							downloadId;
		uint32							streamSize;
		uint32							bytesLeft;
		Timestamp						startTime;
	};

	typedef unordered_map<uint32, DownloadEntry>::type Downloads;

	struct TimeoutEntry
	{
		Timestamp						targetTime;
	};

private:
	friend class RemoteRequestEvent;
	friend class RemoteUploadStartEvent;

	Ref<TcpSocketServer>				_server;

	class BroadcastPeer;
	Ref<RemotePeer>						_broadcastPeer;

	Ref<RemotePeer>						_hostPeer;
	RemotePeers							_guestPeers;

	Ref<UdpSocket>						_udpReceiver;

	Channels							_channels;
	Ref<EventChannel>					_channel0;

	String								_serverHostInfo;

	uint16								_nextChannelID;
	uint32								_nextRequestId;
	uint32								_nextResponseId;
	uint32								_nextUploadId;
	uint32								_nextDownloadId;

	Requests							_requests;
	Responses							_responses;
	Uploads								_uploads;
	Downloads							_downloads;

	typedef unordered_map<uint32, TimeoutEntry>::type	Timeouts;
	Timeouts							_requestTimeouts;
	Timeouts							_uploadTimeouts;
	Timeouts							_downloadTimeouts;

	set<uint16>::type					_userMsg;

	bool								_updating;
	bool								_shutdown;
	bool								_packetDump;
	bool								_suppressRemoteLog;

private:
	template <typename TPredicate>
	void								cancelEntries(TPredicate pred);
	struct Predicates;

	void								updateTimeout();
	void								doShutdown();

public:
	struct Header
	{
	public:
		uint16							signature;
		uint16							msg;
		uint32							packetLen;

	public:
		enum { PACKET_LEN_OFFSET = 4 };

		Header()																{ }
		Header(ushort msg) : signature(HDR_SIGNATURE), msg(msg), packetLen(sizeof(Header))	{ }
	};

	struct HelloPacket
	{
		uint16							signature;
		uint16							msg;
		uint16							hostPort;
		uint16							hostInfoLen;
	};

	enum Msg
	{
		HDR_SIGNATURE					= 0x8164,

		HDR_DEPRECATED_WELCOME			= 0x0001,	// [hdr] 

		HDR_ZPACKET						= 0x0008,	// [hdr] [zdata]

		HDR_CHANNEL_CLOSE				= 0x0012,	// [hdr] [channel: u16]

		HDR_NOTIFY						= 0x0018,	// [hdr] [channel: u16] [cmd: u16] [data]

		HDR_REQUEST						= 0x0020,	// [hdr] [channel: u16] [cmd: u16] [requestId: u32] [msg]
		HDR_REQUEST_CANCEL				= 0x0021,	// [hdr] [channel: u16] [cmd: u16] [requestId: u32]
		HDR_RESPONSE					= 0x0022,	// [hdr] [channel: u16] [cmd: u16] [requestId: u32] [code: i32] [msg]

		HDR_UPLOAD_START				= 0x0030,	// [hdr] [channel: u16] [cmd: u16] [requestId: u32] [uploadId: u32] [streamSize: u32] [msg]
		HDR_UPLOAD_PACKET				= 0x0031,	// [hdr] [downloadId: u32] [packet]
		HDR_UPLOAD_CANCEL				= 0x0032,	// [hdr] [uploadId: u32] [downloadId: u32]

		HDR_DOWNLOAD_START				= 0x0038,	// [hdr] [uploadId: u32] [downloadId: u32] [offset: u32] [size: u32] [packetLen: u16]
		HDR_DOWNLOAD_CANCEL				= 0x0039,	// [hdr] [uploadId: u32]
		HDR_DOWNLOAD_END				= 0x003A,	// [hdr] [uploadId: u32]
	};

	enum ResponseCode
	{
		RESPONSE_OK						=  0,
		RESPONSE_ERROR					= -1,	// Generic error. If want more specific, use below negative value or define one.
		RESPONSE_NO_CHANNEL				= -2,
		RESPONSE_NO_HANDLER				= -3,
		RESPONSE_TIMEOUT				= -4,
		RESPONSE_CANCELED				= -5,
	};

	enum HelloMsg
	{
		HELLO_SERVER_HERE				= 0x0001,
		HELLO_SERVER_WHERE				= 0x0002,
	};
};

inline EventChannel* Remote::getChannel(ChannelId id)
{
	Channels::iterator itr = _channels.find(id); 
	return itr != _channels.end() ? itr->second.channel : NULL;
}

////////////////////////////////////////////////////////////////////////////////

class NIT_API RemoteEvent : public Event
{
public:
	RemoteEvent() { }
	RemoteEvent(nit::Remote* conn, RemotePeer* peer) : remote(conn), peer(peer) { }

	Weak<Remote>						remote;
	Ref<RemotePeer>						peer;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API RemoteHelloEvent : public Event
{
public:
	RemoteHelloEvent() { }
	RemoteHelloEvent(nit::Remote* remote, const String& hostinfo, const String& addr, int port, Remote::HelloMsg msgId) 
		: remote(remote), hostInfo(hostinfo), addr(addr), port(port), msgId(msgId)
	{ }

	Weak<Remote>						remote;
	String								hostInfo;
	String								addr;
	int									port;
	Remote::HelloMsg					msgId;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API RemoteErrorEvent : public Event
{
public:
	RemoteErrorEvent() { }
	RemoteErrorEvent(nit::Remote* remote, RemotePeer* peer, int err, const char* msg) : remote(remote), peer(peer), error(err), message(msg) { }

	Weak<Remote>						remote;
	Ref<RemotePeer>						peer;
	int									error;
	const char*							message;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API RemoteUserPacketEvent : public RemoteEvent
{
public:
	RemoteUserPacketEvent() { }
	RemoteUserPacketEvent(nit::Remote* remote, RemotePeer* peer, uint16 hdrMsg) 
		: RemoteEvent(remote, peer), hdrMsg(hdrMsg) { }

	mutable DataValue					data;
	uint16								hdrMsg;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API RemoteChannelEvent : public RemoteEvent
{
public:
	RemoteChannelEvent() { }
	RemoteChannelEvent(nit::Remote* remote, RemotePeer* peer, uint16 channelId)
		: RemoteEvent(remote, peer), channelId(channelId) { }

	uint16								channelId;
	uint16								serviceID;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API RemoteNotifyEvent : public RemoteEvent
{
public:
	RemoteNotifyEvent() { }
	RemoteNotifyEvent(nit::Remote* remote, RemotePeer* peer, uint16 channelId, uint16 cmd) 
		: RemoteEvent(remote, peer), channelId(channelId), command(cmd) { }

	uint16								channelId;
	uint16								command;
	mutable DataValue					param;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API RemoteRequestEvent : public RemoteNotifyEvent
{
public:
	RemoteRequestEvent() { }
	RemoteRequestEvent(nit::Remote* remote, RemotePeer* peer, uint16 channelId, uint16 cmd, uint32 requestId)
		: RemoteNotifyEvent(remote, peer, channelId, cmd), requestId(requestId), _delayed(false) { }

	uint32								requestId;

	nit::Remote::ResponseId				delay() const;

	void								response(int32 code, const DataValue& param = DataValue::Void()) const;

private:
	friend class nit::Remote;
	mutable bool						_delayed;
	mutable uint32						_responseID;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API RemoteRequestCancelEvent : public RemoteEvent
{
public:
	RemoteRequestCancelEvent() { }
	RemoteRequestCancelEvent(nit::Remote* remote, RemotePeer* peer, uint16 channelId, uint16 cmd, uint32 responseId)
		: RemoteEvent(remote, peer), channelId(channelId), command(cmd), responseId(responseId) { }

	uint16								channelId;
	uint16								command;
	uint32								responseId;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API RemoteResponseEvent : public RemoteNotifyEvent
{
public:
	RemoteResponseEvent() {}
	RemoteResponseEvent(nit::Remote* remote, RemotePeer* peer, uint16 channelId, uint16 cmd, uint32 requestId, int32 code)
		: RemoteNotifyEvent(remote, peer, channelId, cmd), requestId(requestId), code(code) { }

	uint32								requestId;
	int32								code;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API RemoteUploadStartEvent : public RemoteNotifyEvent
{
public:
	RemoteUploadStartEvent() { }
	RemoteUploadStartEvent(nit::Remote* remote, RemotePeer* peer, uint16 channelId, uint16 cmd, uint32 requestId, uint32 uploadId, uint32 streamSize)
		: RemoteNotifyEvent(remote, peer, channelId, cmd), requestId(requestId), uploadId(uploadId), streamSize(streamSize), _offsetResponse(0), _sizeResponse(0), _packetSizeResponse(0) { }

	uint32								requestId;
	uint32								uploadId;
	uint32								streamSize;

	nit::Remote::DownloadId				download(StreamWriter* writer, uint32 offset = 0, uint32 size = 0, uint16 packetSize = 16384) const;

private:
	friend class nit::Remote;

	mutable uint32						_offsetResponse;
	mutable uint32						_sizeResponse;
	mutable Ref<StreamWriter>			_writerResponse;
	mutable uint16						_packetSizeResponse;
	mutable uint32						_downloadID;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API RemoteUploadEvent : public RemoteEvent
{
public:
	RemoteUploadEvent() { }
	RemoteUploadEvent(nit::Remote* remote, RemotePeer* peer, uint32 uploadId, uint32 downloadId)
		: RemoteEvent(remote, peer), uploadId(uploadId), downloadId(downloadId) { }

	uint32								uploadId;
	uint32								downloadId;
};

////////////////////////////////////////////////////////////////////////////////

/// Channel 0 - management events ////////////

NIT_EVENT_DECLARE(NIT_API, REMOTE_CONNECT, RemoteEvent);					// A remote peer connected
NIT_EVENT_DECLARE(NIT_API, REMOTE_DISCONNECT, RemoteEvent);					// A remote peer disconnected

NIT_EVENT_DECLARE(NIT_API, REMOTE_HELLO, RemoteHelloEvent);					// Received an UDP Hello, if not consumed, 'here' will be sent by 'where' query as default
NIT_EVENT_DECLARE(NIT_API, REMOTE_ERROR, RemoteErrorEvent);					// A peer or socket server has encountered an error

NIT_EVENT_DECLARE(NIT_API, REMOTE_USER_PACKET, RemoteUserPacketEvent);		// Received a UserPacket

/// Per channel events /////////////////

NIT_EVENT_DECLARE(NIT_API, REMOTE_CHANNEL_CLOSE, RemoteChannelEvent);		// The channel of remote peer closed

NIT_EVENT_DECLARE(NIT_API, REMOTE_NOTIFY, RemoteNotifyEvent);				// The remote peer sent a notification

NIT_EVENT_DECLARE(NIT_API, REMOTE_REQUEST, RemoteRequestEvent);				// The remote peer sent a request
NIT_EVENT_DECLARE(NIT_API, REMOTE_REQUEST_CANCEL, RemoteRequestCancelEvent);  // The remote peer cancelled its previous request
NIT_EVENT_DECLARE(NIT_API, REMOTE_RESPONSE, RemoteResponseEvent);			// The remote peer sent a response

NIT_EVENT_DECLARE(NIT_API, REMOTE_UPLOAD_START, RemoteUploadStartEvent);	// The remote peer starts uploading
NIT_EVENT_DECLARE(NIT_API, REMOTE_UPLOAD_CANCEL, RemoteUploadEvent);		// The remote peer cancels uploading
NIT_EVENT_DECLARE(NIT_API, REMOTE_UPLOAD_END, RemoteUploadEvent);			// The remote peer ends uploading

NIT_EVENT_DECLARE(NIT_API, REMOTE_DOWNLOAD_START, RemoteUploadEvent);		// The remote peer starts downloading
NIT_EVENT_DECLARE(NIT_API, REMOTE_DOWNLOAD_CANCEL, RemoteUploadEvent);		// The remote peer cancels downloading
NIT_EVENT_DECLARE(NIT_API, REMOTE_DOWNLOAD_END, RemoteUploadEvent);			// The remote peer ends downloading

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
