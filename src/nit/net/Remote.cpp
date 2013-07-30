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

#include "nit/net/Remote.h"

#include "nit/io/MemoryBuffer.h"
#include "nit/data/DataChannel.h"
#include "nit/data/DataLoader.h"
#include "nit/data/DataSaver.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NIT_EVENT_DEFINE(REMOTE_CONNECT,		RemoteEvent);
NIT_EVENT_DEFINE(REMOTE_DISCONNECT,		RemoteEvent);
NIT_EVENT_DEFINE(REMOTE_ERROR,			RemoteErrorEvent);

NIT_EVENT_DEFINE(REMOTE_HELLO,			RemoteHelloEvent);

NIT_EVENT_DEFINE(REMOTE_USER_PACKET,	RemoteUserPacketEvent);

NIT_EVENT_DEFINE(REMOTE_CHANNEL_CLOSE,	RemoteChannelEvent);

NIT_EVENT_DEFINE(REMOTE_NOTIFY,			RemoteNotifyEvent);

NIT_EVENT_DEFINE(REMOTE_REQUEST,		RemoteRequestEvent);
NIT_EVENT_DEFINE(REMOTE_REQUEST_CANCEL,	RemoteRequestCancelEvent);
NIT_EVENT_DEFINE(REMOTE_RESPONSE,		RemoteResponseEvent);

NIT_EVENT_DEFINE(REMOTE_UPLOAD_START,	RemoteUploadStartEvent);
NIT_EVENT_DEFINE(REMOTE_UPLOAD_CANCEL,	RemoteUploadEvent);
NIT_EVENT_DEFINE(REMOTE_UPLOAD_END,		RemoteUploadEvent);

NIT_EVENT_DEFINE(REMOTE_DOWNLOAD_START,	RemoteUploadEvent);
NIT_EVENT_DEFINE(REMOTE_DOWNLOAD_CANCEL,RemoteUploadEvent);
NIT_EVENT_DEFINE(REMOTE_DOWNLOAD_END,	RemoteUploadEvent);

////////////////////////////////////////////////////////////////////////////////

class RemotePeer::DataImpl
{
public:
	DataImpl(RemotePeer* peer)
		: _peer(peer)
	{
		_context = new BinDataContext(true);
		_loader = new BinDataLoader(_context);
		_saver = new BinDataSaver(_context);
	}

	RemotePeer*							_peer;

	Ref<BinDataContext>					_context;
	Ref<BinDataLoader>					_loader;
	Ref<BinDataSaver>					_saver;
};

////////////////////////////////////////////////////////////////////////////////

RemotePeer::RemotePeer(Remote* remote, TcpSocket* socket, bool isHost)
{
	_remote = remote;
	_socket = socket;

	if (_socket)
	{
		_socket->replaceListener(this);
	}

	_sendCompressed = false;
	_zSendSize = 0;

	_properties = new DataRecord();

	_dataImpl = new DataImpl(this);
}

RemotePeer::~RemotePeer()
{
	safeDelete(_dataImpl);
}

bool RemotePeer::sendPacket(uint16 msg, const void* hdrData, size_t hdrDataSize, const void* data, size_t dataSize)
{
	Remote::Header hdr(msg);
	hdr.packetLen = sizeof(hdr) + hdrDataSize + dataSize;

	if (_zSendSize == 0 && !_sendCompressed)
	{
		bool ok = _socket->send(&hdr, sizeof(hdr));
		if (hdrData)
			ok = ok && _socket->send(hdrData, hdrDataSize);
		if (data)
			ok = ok && _socket->send(data, dataSize);
		return ok;
	}

	zBegin();
	_zOut->writeRaw(&hdr, sizeof(hdr));
	if (hdrData)
		_zOut->writeRaw(hdrData, hdrDataSize);
	if (data)
		_zOut->writeRaw(data, dataSize);
	_zSendSize += hdr.packetLen;

	return true;
}

bool RemotePeer::sendPacket(uint16 msg, const void* hdrData, size_t hdrDataSize, DataToSend* data)
{
	size_t dataSize = (data ? data->getSize() : 0);

	Remote::Header hdr(msg);
	hdr.packetLen = sizeof(hdr) + hdrDataSize + dataSize;

	if (_zSendSize == 0 && !_sendCompressed)
	{
		bool ok = _socket->send(&hdr, sizeof(hdr));
		if (hdrData)
			ok = ok && _socket->send(hdrData, hdrDataSize);
		if (data)
			ok = ok && _socket->send(data);
		return ok;
	}

	zBegin();
	_zOut->writeRaw(&hdr, sizeof(hdr));
	if (hdrData)
		_zOut->writeRaw(hdrData, hdrDataSize);

	if (dataSize)
	{
		switch (data->getType())
		{
		case DataToSend::DS_NONE:
			break;

		case DataToSend::DS_BUF:
			data->getBuffer()->save(_zOut, data->getOffset(), dataSize);
			break;

		case DataToSend::DS_READER:
			_zOut->copy(data->getReader(), data->getOffset(), dataSize);
			break;

		default:
			NIT_THROW(EX_NOT_SUPPORTED);
		}
	}

	_zSendSize += hdr.packetLen;

	return true;
}


bool RemotePeer::sendPacket(uint16 msg, const void* hdrData, size_t hdrDataSize, const DataValue& value)
{
	if (value.isVoid())
		return sendPacket(msg, hdrData, hdrDataSize, NULL, 0);

 	Remote::Header hdr(msg);

	// We can't determine DataValue before serialized so do skip(header) -> write -> skip(-size) -> rewrite
	
	if (_zSendSize == 0 && !_sendCompressed)
	{
		MemoryBuffer* sendBuf = _socket->getSendBuf();

		if (_dataValueWriter == NULL)
			_dataValueWriter = new MemoryBuffer::Writer(sendBuf, NULL);

		size_t begin = sendBuf->getSize();
		sendBuf->resize(begin + sizeof(hdr));

		_dataValueWriter->seek(begin + sizeof(hdr));
		if (hdrData)
			_dataValueWriter->write(hdrData, hdrDataSize);

		_dataImpl->_saver->write(value, _dataValueWriter);
		size_t end = sendBuf->getSize();
		
		hdr.packetLen = end - begin;
		sendBuf->copyFrom(&hdr, begin, sizeof(hdr));

		if (_remote->_packetDump)
		{
			_remote->_suppressRemoteLog = true;
			MemoryBuffer::Access temp(sendBuf, begin, hdr.packetLen);
			MemoryAccess::hexDump("Remote Send (DataValue)", (const uint8*)temp.getMemory(), temp.getSize());
			_remote->_suppressRemoteLog = false;
		}

		return _socket->flushSendBuffer();
	}

	// zstream is not seekable, so make a packet into a temp buffer and then write the whole buffer into the zstream

	if (_zDataValueWriter == NULL)
		_zDataValueWriter = new MemoryBuffer::Writer();

	MemoryBuffer* tempBuf = _zDataValueWriter->getBuffer();

	tempBuf->resize(sizeof(hdr) + hdrDataSize);
	_zDataValueWriter->seek(sizeof(hdr));
	
	if (hdrData)
		_zDataValueWriter->writeRaw(hdrData, hdrDataSize);

	_dataImpl->_saver->write(value, _zDataValueWriter);
	
	hdr.packetLen = _zDataValueWriter->tell();
	tempBuf->copyFrom(&hdr, 0, sizeof(hdr));

	zBegin();
	tempBuf->save(_zOut);

	_zSendSize += hdr.packetLen;

	return true;
}

size_t RemotePeer::_readValue(MemoryBuffer* buf, size_t pos, DataValue& outValue)
{
	MemoryBuffer::Reader* reader = NULL;
	
	if (buf == _zRecvBuf)
	{
		reader = _zDataValueReader;
	}
	else if (buf == _socket->getRecvBuf())
	{
		if (_dataValueReader == NULL)
			_dataValueReader = new MemoryBuffer::Reader(buf, NULL);
		reader = _dataValueReader;
	}

	ASSERT_THROW(reader, EX_INVALID_PARAMS);

	reader->seek(pos);
	_dataImpl->_loader->read(outValue, reader);

	return reader->tell() - pos;
}

DataContext* RemotePeer::getDataContext()
{
	return _dataImpl->_context;
}

void RemotePeer::zBegin()
{
	if (_zSendSize == 0)
	{
		Remote::Header hdr(Remote::HDR_ZPACKET);
		hdr.packetLen = 0; // Will be scribed at OnSend

		if (_zOut == NULL)
			_zOut = new ZStreamWriter(new MemoryBuffer::Writer(_socket->getSendBuf(), NULL), true, false);

		_zOut->getTo()->writeRaw(&hdr, sizeof(hdr));
	}
}

void RemotePeer::zEnd()
{
	if (_zSendSize == 0) return;

	// Flush first -> close the buffer -> send

	_zOut->flush();

	MemoryBuffer* buffer = _socket->getSendBuf();

	// Scribe packet length
	uint32 packetLen = _zSendSize + sizeof(Remote::Header);
	buffer->copyFrom(&packetLen, Remote::Header::PACKET_LEN_OFFSET, sizeof(packetLen));

	// Initialize writer's position for buffer
	_zOut->getTo()->seek(0);

	_zSendSize = 0;

	if (!_sendCompressed)
	{
		_zOut = NULL;
		_zDataValueWriter = NULL;
	}
}

void RemotePeer::setSendCompressed(bool flag)
{
	if (_sendCompressed == flag) return;

	_sendCompressed = flag;

	zEnd();

	if (!_socket->flushSendBuffer())
		_socket->disconnect();
}

bool RemotePeer::onSend(TcpSocket* socket)
{
	// Working only in compressed mode
	if (_zOut == NULL) return true;

	zEnd();

	return true;
}

bool RemotePeer::onRecv(TcpSocket* socket)
{
	if (_remote == NULL) return false;

	Ref<RemotePeer> safe = this;

	MemoryBuffer* recvBuf = socket->getRecvBuf();

	while (true)
	{
		if (recvBuf->getSize() < sizeof(Remote::Header))
			return true;

		Remote::Header hdr;
		recvBuf->copyTo(&hdr, 0, sizeof(hdr));

		if (hdr.signature == Remote::HDR_SIGNATURE && hdr.msg == Remote::HDR_ZPACKET)
		{
			if (_zIn == NULL)
				_zIn = new ZStreamReader(new MemoryBuffer::Reader(_socket->getRecvBuf(), NULL));

			if (_zRecvBuf == NULL)
			{
				_zRecvBuf = new MemoryBuffer();
				_zDataValueReader = new MemoryBuffer::Reader(_zRecvBuf, NULL);
			}

			_zIn->getFrom()->seek(sizeof(hdr));
			size_t zsize = hdr.packetLen - sizeof(hdr);
			_zRecvBuf->resize(zsize);
			_zRecvBuf->load(_zIn, 0, zsize);
			recvBuf->popFront(_zIn->getFrom()->tell());

			bool ok = _remote->onRecv(this, _zRecvBuf);
			if (!ok) return false;
		}
		else
		{
			return _remote->onRecv(this, socket->getRecvBuf());
		}
	}
}

void RemotePeer::onError(TcpSocket* socket, char* msg, int err)
{
	if (_remote == NULL) return;

	Ref<RemotePeer> safe = this;

	_remote->onError(this, msg, err);
}

void RemotePeer::onDisconnect(TcpSocket* socket)
{
	if (_remote == NULL) return;

	Ref<RemotePeer> safe = this;

	_socket->replaceListener(NULL);
}

void RemotePeer::disconnect()
{
	Ref<RemotePeer> safe = this;

	_socket->disconnect();
}

////////////////////////////////////////////////////////////////////////////////

class Remote::BroadcastPeer : public RemotePeer
{
protected:
	friend class Remote;

	BroadcastPeer(Remote* remote)
		: RemotePeer(remote, NULL, false)
	{
	}

	virtual bool isGuest()				{ return false; }
	virtual bool isBroadcast()			{ return true; }

	virtual bool isConnected()			{ return _remote->_hostPeer || _remote->_guestPeers.size() > 0; }
	virtual void setSendCompressed(bool flag) { return; } // don't support by itself - underlying can support it

	virtual bool sendPacket(uint16 msg, const void* hdr, size_t hdrSize, const void* data, size_t dataSize)
	{
		RemotePeers& peers = _remote->_guestPeers;

		for (RemotePeers::iterator itr = peers.begin(), end = peers.end(); itr != end; ++itr)
			(*itr)->sendPacket(msg, hdr, hdrSize, data, dataSize);

		if (_remote->_hostPeer)
			_remote->_hostPeer->sendPacket(msg, hdr, hdrSize, data, dataSize);

		return true;
	}

	virtual bool sendPacket(uint16 msg, const void* hdr, size_t hdrSize, DataToSend* data)
	{
		RemotePeers& peers = _remote->_guestPeers;

		for (RemotePeers::iterator itr = peers.begin(), end = peers.end(); itr != end; ++itr)
			(*itr)->sendPacket(msg, hdr, hdrSize, data);

		if (_remote->_hostPeer)
			_remote->_hostPeer->sendPacket(msg, hdr, hdrSize, data);

		return true;
	}

	virtual bool sendPacket(uint16 msg, const void* hdr, size_t hdrSize, const DataValue& value)
	{
		RemotePeers& peers = _remote->_guestPeers;

		for (RemotePeers::iterator itr = peers.begin(), end = peers.end(); itr != end; ++itr)
			(*itr)->sendPacket(msg, hdr, hdrSize, value);

		if (_remote->_hostPeer)
			_remote->_hostPeer->sendPacket(msg, hdr, hdrSize, value);

		return true;
	}
};

////////////////////////////////////////////////////////////////////////////////

Remote::Remote()
{
	_nextRequestId		= 1;
	_nextResponseId		= 1;
	_nextUploadId		= 1;
	_nextDownloadId		= 1;

	_server				= NULL;
	_broadcastPeer		= NULL;

	_updating			= false;
	_shutdown			= false;

	_packetDump			= false;
	_suppressRemoteLog	= false;

	_channel0			= new EventChannel();

	ChannelEntry e;
	e.channelId			= 0;
	e.serviceName		= "nit.Remote";
	e.channel			= _channel0;

	_channels.insert(std::make_pair(0, e));
}

Remote::~Remote()
{
	ASSERT_THROW(!_updating, EX_INVALID_STATE);
	shutdown();
}

void Remote::update()
{
	ASSERT_THROW(!_updating, EX_INVALID_STATE);

	_updating = true;

	updateTimeout();

	processUploads();

	Ref<TcpSocketServer> server = _server;

	if (server)
	{
		bool ok = server->update();
		if (!ok && _server == server) _server = NULL;
	}

	if (_hostPeer)
	{
		bool ok = _hostPeer->getSocket()->update();
	}

	Ref<UdpSocket> udpReceiver = _udpReceiver;

	if (udpReceiver)
	{
		bool ok = udpReceiver->update();
		if (!ok && _udpReceiver == udpReceiver) _udpReceiver = NULL;
	}

	if (_hostPeer && !_hostPeer->isConnected())
	{
		onDisconnect(_hostPeer);
		_hostPeer = NULL;
	}

	for (RemotePeers::iterator itr = _guestPeers.begin(); itr != _guestPeers.end(); )
	{
		RemotePeer* guest = *itr;
		if (guest->isConnected()) { ++itr; continue; }

		onDisconnect(guest);
		_guestPeers.erase(itr++);
	}

	_updating = false;

	if (_shutdown)
		doShutdown();
}

bool Remote::serverWhere(const String& hostQuery, uint16 myPort, uint16 targetPort)
{
	if (_udpReceiver && _udpReceiver->isBound() && _udpReceiver->getBindPort() != targetPort)
	{
		_udpReceiver->disconnect();
		_udpReceiver = NULL;
	}

	_udpReceiver = new UdpSocket(this);
	_udpReceiver->bind(myPort);

	char buf[128];
	HelloPacket* pk = (HelloPacket*)buf;
	pk->signature = HDR_SIGNATURE;
	pk->msg = HELLO_SERVER_WHERE;
	pk->hostPort = _udpReceiver->getBindPort();

	pk->hostInfoLen = hostQuery.length();
	strncpy(buf + sizeof(HelloPacket), hostQuery.c_str(), hostQuery.length());

	size_t packetLen = sizeof(HelloPacket) + pk->hostInfoLen;

	return _udpReceiver->sendTo("255.255.255.255", targetPort, buf, packetLen);
}

bool Remote::serverHere(const String& hostinfo, const String& targetAddr, uint16 targetPort)
{
	if (_server == NULL || !_server->isListening())
		return false;

	Ref<UdpSocket> udpSender = new UdpSocket(this);

	char buf[128];
	HelloPacket* pk = (HelloPacket*)buf;
	pk->signature = HDR_SIGNATURE;
	pk->msg = HELLO_SERVER_HERE;
	pk->hostPort = _server->getBindPort();

	pk->hostInfoLen = hostinfo.length();
	strncpy(buf + sizeof(HelloPacket), hostinfo.c_str(), hostinfo.length());

	size_t packetLen = sizeof(HelloPacket) + pk->hostInfoLen;

	return udpSender->sendTo(targetAddr, targetPort, buf, packetLen);
}

bool Remote::listen(const String& hostinfo, uint16 port)
{
	if (_server)
		NIT_THROW_FMT(EX_INVALID_STATE, "already listening %s:%d", _server->getBindAddr().c_str(), _server->getBindPort());

	if (_hostPeer)
		NIT_THROW_FMT(EX_INVALID_STATE, "already connected to %s:%d", _hostPeer->getSocket()->getAddr().c_str(), _hostPeer->getSocket()->getPort());

	if (_server)
		_server->shutdown();

	_server = new TcpSocketServer(this);
	bool ok = _server->listen(port);

	if (!ok)
	{
		_server = NULL;
		return false;
	}

	_serverHostInfo = hostinfo;

	_broadcastPeer = new BroadcastPeer(this);

	serverHere(hostinfo);

	return true;
}

bool Remote::connect(const String& givenAddr)
{
	if (_server)
		NIT_THROW_FMT(EX_INVALID_STATE, "already listening %s:%d", _server->getBindAddr().c_str(), _server->getBindPort());

	if (_hostPeer)
		_hostPeer->disconnect();

	uint16 port = Remote::PORT_DEFAULT_TCP;
	String addr = givenAddr;

	size_t colonPos = addr.find(':');
	if (colonPos != addr.npos)
	{
		port = DataValue(addr.substr(colonPos+1)).toInt();
		addr = addr.substr(0, colonPos);
	}

	StringUtil::trim(addr);

	LOG(0, "connecting %s: %d\n", addr.c_str(), port);

	Ref<TcpSocket> socket = new TcpSocket(NULL, addr, port);
	
	if (!socket->isValid())
		return false;

	_hostPeer = new RemotePeer(this, socket, true);

	// Notify the new connection to channel 0 - management purpose
	_channel0->send(EVT::REMOTE_CONNECT, new RemoteEvent(this, _hostPeer));

	return true;
}

bool Remote::onRecv(RemotePeer* peer, MemoryBuffer* recvBuf)
{
	// Examine packet frame
	while (!_shutdown)
	{
		if (recvBuf->getSize() < sizeof(Header))
			return true;

		Header hdr;
		recvBuf->copyTo(&hdr, 0, sizeof(hdr));

		if (hdr.signature != HDR_SIGNATURE)
			return peer->getSocket()->error("invalid signature", SocketBase::ERR_INVAL);

		if (recvBuf->getSize() < hdr.packetLen)
			return true;

		PacketReader packet(peer, recvBuf, sizeof(hdr), hdr.packetLen, hdr.packetLen - sizeof(hdr));

		if (_packetDump)
		{
			_suppressRemoteLog = true;
			MemoryBuffer::Access temp(recvBuf, 0, hdr.packetLen);
			MemoryAccess::hexDump("Remote Recv", (const uint8*)temp.getMemory(), temp.getSize());
			_suppressRemoteLog = false;
		}

		switch (hdr.msg)
		{
		case HDR_DEPRECATED_WELCOME:	recvWelcome(peer, &packet); break;

		case HDR_CHANNEL_CLOSE:			recvChannelClose(peer, &packet); break;

		case HDR_NOTIFY:				recvNotify(peer, &packet); break;

		case HDR_REQUEST:				recvRequest(peer, &packet); break;
		case HDR_REQUEST_CANCEL:		recvRequestCancel(peer, &packet); break;
		case HDR_RESPONSE:				recvResponse(peer, &packet); break;

		case HDR_UPLOAD_START:			recvUploadStart(peer, &packet); break;
		case HDR_UPLOAD_PACKET:			recvUploadPacket(peer, &packet); break;
		case HDR_UPLOAD_CANCEL:			recvUploadCancel(peer, &packet); break;

		case HDR_DOWNLOAD_START:		recvDownloadStart(peer, &packet); break;
		case HDR_DOWNLOAD_CANCEL:		recvDownloadCancel(peer, &packet); break;
		case HDR_DOWNLOAD_END:			recvDownloadEnd(peer, &packet); break;

		default:
			if (hdr.msg < 0x0100 || _userMsg.find(hdr.msg) == _userMsg.end())
			{
				LOG(0, "*** unknown remote user msg: 0x%08X\n", hdr.msg);
			}
			else
			{
				Ref<RemoteUserPacketEvent> evt = new RemoteUserPacketEvent(this, peer, hdr.msg);
				packet.readValue(evt->data);
				packet.consume();
				_channel0->send(EVT::REMOTE_USER_PACKET, evt);
			}
		}

		packet.consume();
	}

	return true;
}

TcpSocket* Remote::onAccept(TcpSocketServer* server, int socketHandle, const String& peerAddr, uint16 peerPort)
{
	TcpSocket* socket = new TcpSocket(NULL, socketHandle, peerAddr, peerPort);

	RemotePeer* peer = new RemotePeer(this, socket, false);

	_guestPeers.insert(peer);

	peer->sendPacket(HDR_DEPRECATED_WELCOME, NULL, 0, NULL, 0);

	return socket;
}

void Remote::onUdpRecv(UdpSocket* socket, const String& peerAddr, uint16 peerPort, uint8* msg, size_t len)
{
	if (len < sizeof(HelloPacket))
		return;

	HelloPacket* pk = (HelloPacket*)msg;
	if (pk->signature != HDR_SIGNATURE)
		return;

	if (len < sizeof(HelloPacket) + pk->hostInfoLen)
		return;

	String hostinfo((const char*)(msg + sizeof(HelloPacket)), pk->hostInfoLen);

	bool testLog = true;
	if (testLog)
	{
		switch (pk->msg)
		{
		case HELLO_SERVER_HERE:
			LOG(0, "++ [HELLO] Here : %s:%d '%s'\n", peerAddr.c_str(), (int)pk->hostPort, hostinfo.c_str());
			break;

		case HELLO_SERVER_WHERE:
			LOG(0, "++ [HELLO] Where? %s:%d '%s'\n", peerAddr.c_str(), (int)pk->hostPort, hostinfo.c_str());
			break;

		default:
			LOG(0, "++ [HELLO] msg:%d %s:%d '%s'\n", pk->msg, peerAddr.c_str(), (int)pk->hostPort, hostinfo.c_str());
		}
	}

	Ref<Event> evt = new RemoteHelloEvent(this, hostinfo, peerAddr, pk->hostPort, (HelloMsg)pk->msg);
	_channel0->send(EVT::REMOTE_HELLO, evt);

	if (evt->isConsumed()) return;

	if (pk->msg == HELLO_SERVER_WHERE)
	{
		if (_server && _server->isListening())
			serverHere(_serverHostInfo, peerAddr, pk->hostPort);
	}
}

void Remote::onError(RemotePeer* peer, char* msg, int err)
{
	_channel0->send(EVT::REMOTE_ERROR, new RemoteErrorEvent(this, peer, err, msg));
}

void Remote::onError(TcpSocketServer* server, char* msg, int err)
{
	_channel0->send(EVT::REMOTE_ERROR, new RemoteErrorEvent(this, NULL, err, msg));
}

void Remote::shutdown()
{
	if (_shutdown) return;

	_shutdown = true;

	if (!_updating)
		doShutdown();
}

void Remote::doShutdown()
{
	if (!_shutdown) return;

	// NOTE: A disconnection regarded as cancel to the remote peer, so we don't need to send cancels to it.
	// Just send events to channels of local peer. (Someone on local may be watching if canceled)

	// Cancel all remaining requests

	Requests canceledRequests;
	canceledRequests.swap(_requests);
	_requestTimeouts.clear();

	for (Requests::iterator itr = canceledRequests.begin(), end = canceledRequests.end(); itr != end; ++itr)
	{
		RequestEntry& e = itr->second;

		if (e.to && e.channel)
		{
			e.channel->send(EVT::REMOTE_RESPONSE, new RemoteResponseEvent(this, e.to, e.channelId, e.command, e.requestId, RESPONSE_CANCELED));
		}
	}

	// Cancel all remaining responses

	Responses canceledResponses;
	canceledResponses.swap(_responses);

	for (Responses::iterator itr = canceledResponses.begin(), end = canceledResponses.end(); itr != end; ++itr)
	{
		ResponseEntry& e = itr->second;

		if (e.from && e.channel)
			e.channel->send(EVT::REMOTE_REQUEST_CANCEL, new RemoteRequestCancelEvent(this, e.from, e.channelId, e.command, e.responseId));
	}

	// Cancel all remaining uploads

	Uploads canceledUploads;
	canceledUploads.swap(_uploads);

	for (Uploads::iterator itr = canceledUploads.begin(), end = canceledUploads.end(); itr != end; ++itr)
	{
		UploadEntry& e = itr->second;

		if (e.to && e.channel)
			e.channel->send(EVT::REMOTE_DOWNLOAD_CANCEL, new RemoteUploadEvent(this, e.to, e.uploadId, e.downloadId));
	}

	// Cancel all remaining downloads

	Downloads canceledDownloads;
	canceledDownloads.swap(_downloads);

	for (Downloads::iterator itr = canceledDownloads.begin(), end = canceledDownloads.end(); itr != end; ++itr)
	{
		DownloadEntry& e = itr->second;

		if (e.from && e.channel)
			e.channel->send(EVT::REMOTE_UPLOAD_CANCEL, new RemoteUploadEvent(this, e.from, e.uploadId, e.downloadId));
	}
	_downloads.clear();

	// Remove all timeouts
	_requestTimeouts.clear();
	_uploadTimeouts.clear();
	_downloadTimeouts.clear();

	// Remove all channels except channel 0
	ChannelEntry entry0 = _channels[0];

	// and renew event channel of channel 0
	entry0.channel = new EventChannel();

	_channels.clear();
	_channels[0] = entry0;

	// Disconnect and shutdown
	if (_server)
		_server->shutdown();

	if (_hostPeer)
		_hostPeer->disconnect();

	for (RemotePeers::iterator itr = _guestPeers.begin(); itr != _guestPeers.end(); ++itr)
	{
		RemotePeer* guest = *itr;
		guest->disconnect();
	}
	_guestPeers.clear();

	if (_udpReceiver)
		_udpReceiver->disconnect();

	// Remove referencess
	_server = NULL;
	_hostPeer = NULL;
	_udpReceiver = NULL;

	_shutdown = false;
}

// Workaround for C++98 not allowing local structs (android)
struct Remote::Predicates
{
	struct OnDisconnect;
	struct RecvChannelClose;
	struct CloseChannel;
};

template <typename TPredicate>
void Remote::cancelEntries(TPredicate pred)
{
	if (_shutdown) return;

	// NOTE: We don't need to remove time-outs (next UpdateTimeout() will remove them naturally)

	// Notify cancelation of remaining requests and delete them

	typedef list<RequestEntry>::type RequestList;
	RequestList canceledRequests; // Delete them first, notify them later

	for (Requests::iterator itr = _requests.begin(), end = _requests.end(); itr != end; )
	{
		RequestEntry& e = itr->second;
		if (!pred(e)) { ++itr; continue; }

		canceledRequests.push_back(e);

		_requests.erase(itr++);
	}

	for (RequestList::iterator itr = canceledRequests.begin(), end = canceledRequests.end(); itr != end; ++itr)
	{
		RequestEntry& e = *itr;
		if (e.to && e.channel)
		{
			e.channel->send(EVT::REMOTE_RESPONSE, new RemoteResponseEvent(this, e.to, e.channelId, e.command, e.requestId, RESPONSE_CANCELED));
		}
	}

	// Notify cancelation of remaining responses and delete them

	typedef list<ResponseEntry>::type ResponseList;
	ResponseList canceledResponses; // Delete them first, notify them later

	for (Responses::iterator itr = _responses.begin(), end = _responses.end(); itr != end; )
	{
		ResponseEntry& e = itr->second;
		if (!pred(e)) { ++itr; continue; }

		canceledResponses.push_back(e);

		_responses.erase(itr++);
	}

	for (ResponseList::iterator itr = canceledResponses.begin(), end = canceledResponses.end(); itr != end; ++itr)
	{
		ResponseEntry& e = *itr;

		if (e.from && e.channel)
			e.channel->send(EVT::REMOTE_REQUEST_CANCEL, new RemoteRequestCancelEvent(this, e.from, e.channelId, e.command, e.responseId));
	}

	// Notify cancelation of remaining uploads and delete them

	typedef list<UploadEntry>::type UploadList;
	UploadList canceledUploads; // Delete them first, notify them later

	for (Uploads::iterator itr = _uploads.begin(), end = _uploads.end(); itr != end; )
	{
		UploadEntry& e = itr->second;
		if (!pred(e)) { ++itr; continue; }

		canceledUploads.push_back(e);

		_uploads.erase(itr++);
	}

	for (UploadList::iterator itr = canceledUploads.begin(), end = canceledUploads.end(); itr != end; ++itr)
	{
		UploadEntry& e = *itr;

		if (e.to && e.channel)
			e.channel->send(EVT::REMOTE_DOWNLOAD_CANCEL, new RemoteUploadEvent(this, e.to, e.uploadId, e.downloadId));
	}

	// Notify cancelation of remaining Downloads and delete them

	typedef list<DownloadEntry>::type DownloadList;
	DownloadList canceledDownloads;	// Delete them first, notify them later

	for (Downloads::iterator itr = _downloads.begin(), end = _downloads.end(); itr != end; )
	{
		DownloadEntry& e = itr->second;
		if (!pred(e)) { ++itr; continue; }

		canceledDownloads.push_back(e);

		_downloads.erase(itr++);
	}

	for (DownloadList::iterator itr = canceledDownloads.begin(), end = canceledDownloads.end(); itr != end; ++itr)
	{
		DownloadEntry& e = *itr;

		if (e.from && e.channel)
			e.channel->send(EVT::REMOTE_UPLOAD_CANCEL, new RemoteUploadEvent(this, e.from, e.uploadId, e.downloadId));
	}
}

// TODO: On c++0x, we may refactor this struct local
struct Remote::Predicates::OnDisconnect
{
	OnDisconnect(RemotePeer* peer) : peer(peer) { }

	bool operator () (RequestEntry& e)		{ return e.to == peer; }
	bool operator () (ResponseEntry& e)		{ return e.from == peer; }
	bool operator () (UploadEntry& e)		{ return e.to == peer; }
	bool operator () (DownloadEntry& e)		{ return e.from == peer; }

	RemotePeer* peer;
};

void Remote::onDisconnect(RemotePeer* peer)
{
	// NOTE: We don't need to implement TcpSocketServer.OnDisconnected
	// (handled by RemotePeer.OnDisconnect)

	cancelEntries(Predicates::OnDisconnect(peer));

	// TODO: notify disconnection to the associated channels

	// Notify disconnection to channel 0 - management purpose
	_channel0->send(EVT::REMOTE_DISCONNECT, new RemoteEvent(this, peer));
}

void Remote::recvWelcome(RemotePeer* from, PacketReader* packet)
{
	Ref<TcpSocket> socket = from->getSocket();
	LOG(0, "++ Connected %s: %d\n", socket->getAddr().c_str(), (int)socket->getPort());

	// TODO: mutual version comparison, exchange hostinfo, rename welcome -> greeting
}

// TODO: On c++0x, we may refactor this struct local
struct Remote::Predicates::RecvChannelClose
{
	RecvChannelClose(ChannelId channelId, RemotePeer* peer) : channelId(channelId), peer(peer) { }

	bool operator () (RequestEntry& e)		{ return e.channelId == channelId && e.to == peer; }
	bool operator () (ResponseEntry& e)		{ return e.channelId == channelId && e.from == peer; }
	bool operator () (UploadEntry& e)		{ return e.channelId == channelId && e.to == peer; }
	bool operator () (DownloadEntry& e)		{ return e.channelId == channelId && e.from == peer; }

	ChannelId channelId;
	RemotePeer* peer;
};

void Remote::recvChannelClose(RemotePeer* from, PacketReader* packet)
{
	uint16 channelId = packet->read<uint16>();
	packet->consume();

	Channels::iterator itr = _channels.find(channelId);
	if (itr == _channels.end()) return;

	EventChannel* channel = itr->second.channel;

	cancelEntries(Predicates::RecvChannelClose(channelId, from));

	Ref<Event> evt = new RemoteChannelEvent(this, from, channelId);
	channel->send(EVT::REMOTE_CHANNEL_CLOSE, evt);
}

void Remote::recvNotify(RemotePeer* from, PacketReader* packet)
{
	uint16 channelId = packet->read<uint16>();

	EventChannel* channel = getChannel(channelId);
	if (channel)
	{
		uint16 command = packet->read<uint16>();
		Ref<RemoteNotifyEvent> evt = new RemoteNotifyEvent(this, from, channelId, command);
		packet->readValue(evt->param);
		packet->consume();
		channel->send(EVT::REMOTE_NOTIFY, evt);
	}
}

void Remote::recvRequest(RemotePeer* from, PacketReader* packet)
{
	uint16 channelId	= packet->read<uint16>();
	uint16 command		= packet->read<uint16>();
	uint32 requestId	= packet->read<uint32>();

	EventChannel* channel = getChannel(channelId);
	if (channel == NULL)
		return response(from, channelId, command, requestId, RESPONSE_NO_CHANNEL, NULL);

	Ref<RemoteRequestEvent> evt = new RemoteRequestEvent(this, from, channelId, command, requestId);
	packet->readValue(evt->param);
	packet->consume();

	channel->send(EVT::REMOTE_REQUEST, evt);
	if (evt->_delayed)
	{
		// A delayed request needs an entry
		uint32 responseId	= evt->_responseID;

		ResponseEntry e;
		e.channel		= channel;
		e.channelId		= channelId;
		e.command		= command;
		e.from			= from;
		e.requestId		= requestId;
		e.responseId	= responseId;

		_responses.insert(std::make_pair(responseId, e));
	}
	else if (!evt->isConsumed())
	{
		// If no one consumes this event, regard it as no handler.
		// So response a 'no handler'
		response(from, channelId, command, requestId, RESPONSE_NO_HANDLER, NULL);
	}
}

void Remote::recvRequestCancel(RemotePeer* from, PacketReader* packet)
{
	uint16 channelId = packet->read<uint16>();

	EventChannel* channel = getChannel(channelId);
	if (channel == NULL) return;

	uint16 command		= packet->read<uint16>();
	uint32 requestId	= packet->read<uint32>();
	packet->consume();

	// TODO: Check performance
	for (Responses::iterator itr = _responses.begin(), end = _responses.end(); itr != end; ++itr)
	{
		ResponseEntry& e = itr->second;

		if (e.from == from && e.requestId == requestId)
		{
			Ref<RemoteRequestCancelEvent> evt = new RemoteRequestCancelEvent(this, from, channelId, command, e.responseId);
			_responses.erase(itr); // Delete first, then notify
			channel->send(EVT::REMOTE_REQUEST_CANCEL, evt);
			return;
		}
	}
}

void Remote::recvResponse(RemotePeer* from, PacketReader* packet)
{
	uint16 channelId	= packet->read<uint16>();

	EventChannel* channel = getChannel(channelId);

	if (channel == NULL) return;

	uint16 command		= packet->read<uint16>();
	uint32 requestId	= packet->read<uint32>();
	int32 code			= packet->read<int32>();

	Requests::iterator itr = _requests.find(requestId);
	if (itr == _requests.end())
	{
		// Maybe it's an already canceled request - ignore
		return;
	}

	_requests.erase(itr); // Delete first, then notify

	Ref<RemoteResponseEvent> evt = new RemoteResponseEvent(this, from, channelId, command, requestId, code);
	packet->readValue(evt->param);
	packet->consume();
	channel->send(EVT::REMOTE_RESPONSE, evt);
}

void Remote::recvUploadStart(RemotePeer* from, PacketReader* packet)
{
	uint16 channelId	= packet->read<uint16>();
	uint16 command		= packet->read<uint16>();
	uint32 requestId	= packet->read<uint32>();
	uint32 uploadId		= packet->read<uint32>();

	EventChannel* channel = getChannel(channelId);

	// If no such channel, notify that we can't download
	if (channel == NULL)
	{
		from->sendPacket(HDR_DOWNLOAD_CANCEL, &uploadId, sizeof(uploadId), NULL, 0);
		return;
	}

	uint32 streamSize	= packet->read<uint32>();

	Ref<RemoteUploadStartEvent> evt = new RemoteUploadStartEvent(this, from, channelId, command, requestId, uploadId, streamSize);
	packet->readValue(evt->param);
	packet->consume();

	channel->send(EVT::REMOTE_UPLOAD_START, evt);

	// If no writer assigned during event dispatch, notify that we can't download
	if (evt->_writerResponse == NULL)
	{
		LOG(0, "&& No download writer response\n");
		from->sendPacket(HDR_DOWNLOAD_CANCEL, &uploadId, sizeof(uploadId), NULL, 0);
	}
	else
	{
		uint32 downloadId = evt->_downloadID;

		// Setup an entry
		DownloadEntry e;
		e.channel		= channel;
		e.from			= from;
		e.writer		= evt->_writerResponse;
		e.channelId		= channelId;
		e.command		= command;
		e.uploadId		= uploadId;
		e.downloadId	= downloadId;
		e.streamSize	= evt->_sizeResponse;
		e.bytesLeft		= e.streamSize;
		e.startTime		= Timestamp::now();

		_downloads.insert(std::make_pair(downloadId, e));

		// Notify that we can download now
		struct  
		{
			uint32 uploadId;
			uint32 downloadId;
			uint32 offset;
			uint32 size;
			uint16 packetSize;
		} hdr = { uploadId, downloadId, evt->_offsetResponse, evt->_sizeResponse, evt->_packetSizeResponse };

		from->sendPacket(HDR_DOWNLOAD_START, &hdr, sizeof(hdr), NULL, 0);
	}
}

void Remote::recvUploadPacket(RemotePeer* from, PacketReader* packet)
{
	uint32 downloadId = packet->read<uint32>();

	Downloads::iterator itr = _downloads.find(downloadId);

	if (itr == _downloads.end())
	{
		// TODO: Invalid uploadId - dump some log message
		return;
	}

	DownloadEntry& e = itr->second;

	size_t packetLen = packet->getDataLeft();

	if (e.bytesLeft < packetLen)
	{
		// Something got corrupted - cancel the transmit

		// TODO: Dump some log message

		packet->consume();
		cancelDownload(downloadId);
		return;
	}

	bool ok = false;

	try
	{
		packet->copyWriteTo(e.writer, packetLen);
		packet->consume();
		ok = true;
		e.bytesLeft -= packetLen;
	}
	catch (Exception&)
	{
		// TODO: log something or throw something
	}

	if (!ok)
	{
		cancelDownload(downloadId);
		return;
	}

	if (e.bytesLeft > 0) return;

	// Notify that we've received everything they sent
	from->sendPacket(HDR_DOWNLOAD_END, &e.uploadId, sizeof(e.uploadId), NULL, 0);

	EventChannel* channel = e.channel;
	UploadId uploadId = e.uploadId;

	_downloads.erase(itr); // Delete first, then notify

	if (channel) channel->send(EVT::REMOTE_DOWNLOAD_END, new RemoteUploadEvent(this, from, uploadId, downloadId));
}

void Remote::recvUploadCancel(RemotePeer* from, PacketReader* packet)
{
	uint32 uploadId = packet->read<uint32>();
	uint32 downloadId = packet->read<uint32>();
	packet->consume();

	if (downloadId == 0)
	{
		// Search for the sender for it may not received downloadId yet
		for (Downloads::iterator itr = _downloads.begin(), end = _downloads.end(); itr != end; ++itr)
		{
			DownloadEntry& e = itr->second;
			if (e.from == from && e.uploadId == uploadId)
			{
				downloadId = e.downloadId;
				break;
			}
		}
	}

	if (downloadId == 0) return;

	Downloads::iterator itr = _downloads.find(downloadId);
	if (itr == _downloads.end()) return;

	DownloadEntry& e = itr->second;

	EventChannel* channel = e.channel;

	_downloads.erase(itr); // Delete first, then notify

	if (channel) channel->send(EVT::REMOTE_UPLOAD_CANCEL, new RemoteUploadEvent(this, from, uploadId, downloadId));
}

void Remote::recvDownloadStart(RemotePeer* from, PacketReader* packet)
{
	uint32 uploadId		= packet->read<uint32>();
	uint32 downloadId	= packet->read<uint32>();
	uint32 offset		= packet->read<uint32>();
	uint32 size			= packet->read<uint32>();
	uint16 packetLen	= packet->read<uint16>();
	packet->consume();

	Uploads::iterator itr = _uploads.find(uploadId);

	if (itr == _uploads.end())
	{
		// Invalid UploadID - Send cancel
		from->sendPacket(HDR_UPLOAD_CANCEL, &uploadId, sizeof(uploadId), NULL, 0);
		return;
	}

	UploadEntry& e = itr->second;
	e.downloadId	= downloadId;
	e.offset		= offset;
	e.streamSize	= size;
	e.bytesLeft		= size;
	e.packetSize	= packetLen;

	if (offset != 0)
	{
		if (e.reader->isSeekable())
			e.reader->skip(offset);
		else
		{
			// Consume by read up
			while (offset > 0)
			{
				char buf[1024];
				size_t skip = offset < sizeof(buf) ? offset : sizeof(buf);
				size_t read = e.reader->readRaw(buf, skip);
				ASSERT_THROW(read == skip, EX_READ);

				offset -= skip;
			}
		}
	}
	
	EventChannel* channel = e.channel;
	if (channel)
		channel->send(EVT::REMOTE_DOWNLOAD_START, new RemoteUploadEvent(this, e.to, e.uploadId, e.downloadId));

	// Actual upload will start at next ProcessUploads() loop
}

void Remote::recvDownloadCancel(RemotePeer* from, PacketReader* packet)
{
	uint32 uploadId = packet->read<uint32>();
	packet->consume();

	cancelUpload(uploadId);
}

void Remote::recvDownloadEnd(RemotePeer* from, PacketReader* packet)
{
	uint32 uploadId = packet->read<uint32>();
	packet->consume();

	Uploads::iterator itr = _uploads.find(uploadId);
	if (itr == _uploads.end()) return;

	UploadEntry& e = itr->second;

	EventChannel* channel = e.channel;
	DownloadId downloadId = e.downloadId;

	_uploads.erase(itr); // delete first, then notify

	if (channel) channel->send(EVT::REMOTE_DOWNLOAD_END, new RemoteUploadEvent(this, from, uploadId, downloadId));
}

void Remote::sendUserPacket(RemotePeer* to, uint16 hdrMsg, const DataValue& data)
{
	ASSERT_THROW(to && to->getRemote() == this, EX_INVALID_PARAMS);

	to->sendPacket(hdrMsg, NULL, 0, data);
}

void Remote::notify(RemotePeer* to, ChannelId channel, CommandId cmd, const DataValue& param)
{
	ASSERT_THROW(to && to->getRemote() == this, EX_INVALID_PARAMS);

	struct  
	{
		uint16 channel;
		uint16 cmd;
	} hdr = { channel, cmd };

	to->sendPacket(HDR_NOTIFY, &hdr, sizeof(hdr), param);
}

Remote::RequestId Remote::request(RemotePeer* to, ChannelId channelId, CommandId cmd, const DataValue& param)
{
	ASSERT_THROW(to && to->getRemote() == this, EX_INVALID_PARAMS);
	ASSERT_THROW(!_shutdown, EX_INVALID_STATE);

	EventChannel* channel = getChannel(channelId);
	ASSERT_THROW(channel, EX_INVALID_PARAMS);

	uint32 requestId = _nextRequestId++;

	struct
	{
		uint16 channelId;
		uint16 cmd;
		uint32 requestId;
	} hdr = { channelId, cmd, requestId };

	bool ok = to->sendPacket(HDR_REQUEST, &hdr, sizeof(hdr), param);

	if (!ok) return 0;

	RequestEntry e;
	e.channel	= channel;
	e.channelId = channelId;
	e.command	= cmd;
	e.requestId = requestId;
	e.to		= to;
	e.startTime = Timestamp::now();

	_requests.insert(std::make_pair(requestId, e));

	return requestId;
}

void Remote::cancelRequest(RequestId id)
{
	if (_shutdown) return;

	Requests::iterator itr = _requests.find(id);
	if (itr == _requests.end()) return;

	RequestEntry& e = itr->second;

	Ref<RemotePeer> to = e.to.get();

	if (to)
	{
		struct 
		{
			uint16 channelId;
			uint16 command;
			uint32 requestId;
		} hdr = { e.channelId, e.command, id };

		to->sendPacket(HDR_REQUEST_CANCEL, &hdr, sizeof(hdr), NULL, 0);
	}

	_requests.erase(itr);
}

void Remote::delayedResponse(ResponseId id, int32 code, const DataValue& param)
{
	Responses::iterator itr = _responses.find(id);
	
	if (itr == _responses.end()) return;

	ResponseEntry& e = itr->second;

	Ref<RemotePeer> from = e.from.get();

	if (from)
	{
		struct
		{
			uint16 channelId;
			uint16 command;
			uint32 requestId;
			int32  code;
		} hdr = { e.channelId, e.command, e.requestId, code };

		from->sendPacket(HDR_RESPONSE, &hdr, sizeof(hdr), param);
	}

	_responses.erase(itr);
}

void Remote::response(RemotePeer* to, ChannelId channel, CommandId cmd, RequestId requestId, uint32 code, DataToSend* msg)
{
	ASSERT_THROW(to && to->getRemote() == this, EX_INVALID_PARAMS);
	ASSERT_THROW(!_shutdown, EX_INVALID_STATE);

	struct
	{
		uint16 channel;
		uint16 cmd;
		uint32 requestId;
		int32  code;
	} hdr = { channel, cmd, requestId, code };

	to->sendPacket(HDR_RESPONSE, &hdr, sizeof(hdr), msg);
}

void Remote::response(RemotePeer* to, ChannelId channel, CommandId cmd, RequestId requestId, uint32 code, const DataValue& msg)
{
	ASSERT_THROW(to && to->getRemote() == this, EX_INVALID_PARAMS);
	ASSERT_THROW(!_shutdown, EX_INVALID_STATE);

	struct
	{
		uint16 channel;
		uint16 cmd;
		uint32 requestId;
		int32  code;
	} hdr = { channel, cmd, requestId, code };

	to->sendPacket(HDR_RESPONSE, &hdr, sizeof(hdr), msg);
}

Remote::UploadId Remote::upload(RemotePeer* to, ChannelId channelId, CommandId cmd, RequestId requestId, StreamReader* reader, uint32 streamSize, const DataValue& param)
{
	ASSERT_THROW(to && to->getRemote() == this, EX_INVALID_PARAMS);
	ASSERT_THROW(!_shutdown, EX_INVALID_STATE);

	EventChannel* channel = getChannel(channelId);
	ASSERT_THROW(channel, EX_INVALID_PARAMS);

	uint32 uploadId = _nextUploadId++;

	struct
	{
		uint16 channelId;
		uint16 command;
		uint32 requestId;
		uint32 uploadId;
		uint32 streamSize;
	} hdr = { channelId, cmd, requestId, uploadId, streamSize };

	bool ok = to->sendPacket(HDR_UPLOAD_START, &hdr, sizeof(hdr), param);

	if (!ok) return 0;

	UploadEntry e;
	e.channel		= channel;
	e.to			= to;
	e.reader		= reader;
	e.channelId		= channelId;
	e.command		= cmd;
	e.requestId		= requestId;
	e.uploadId		= uploadId;
	e.downloadId	= 0;
	e.streamSize	= streamSize;
	e.offset		= 0;
	e.bytesLeft		= 0;
	e.packetSize	= 0;
	e.startTime		= Timestamp::now();

	_uploads.insert(std::make_pair(uploadId, e));

	return uploadId;
}

void Remote::processUploads()
{
	vector<uint32>::type canceled;

	for (Uploads::iterator itr = _uploads.begin(), end = _uploads.end(); itr != end; ++itr)
	{
		UploadEntry& e = itr->second;

		uint32 downloadId = e.downloadId;

		if (downloadId == 0)
		{
			// No DownloadStart received yet
			continue;
		}

		if (e.bytesLeft == 0)
		{
			// No DownloadEnd received yet
			continue;
		}

		Ref<RemotePeer> to = e.to.get();

		if (to == NULL)
		{
			// confirm disconnected and to be cancelled
			canceled.push_back(e.uploadId);
			continue;
		}

		size_t byteCount = e.bytesLeft;
		if (byteCount > e.packetSize)
			byteCount = e.packetSize;

		if (byteCount > 0)
		{
			// send packet
            DataToSend data(e.reader, byteCount);
			to->sendPacket(HDR_UPLOAD_PACKET, &downloadId, sizeof(downloadId), &data);

			e.offset += byteCount;
			e.bytesLeft -= byteCount;
		}
	}

	// Do cancelation to targets which are discovered during above loop
	for (uint i=0; i < canceled.size(); ++i)
	{
		cancelUpload(canceled[i]);
	}
}

void Remote::cancelUpload(UploadId id)
{
	if (_shutdown) return;

	Uploads::iterator itr = _uploads.find(id);
	if (itr == _uploads.end()) return;

	UploadEntry& e = itr->second;

	Ref<RemotePeer> to = e.to.get();

	if (to)
	{
		struct  
		{
			uint32 uploadId;
			uint32 downloadId;
		} hdr = { id, e.downloadId };

		to->sendPacket(HDR_UPLOAD_CANCEL, &hdr, sizeof(hdr), NULL, 0);
	}

	EventChannel* channel = e.channel;
	UploadId uploadId = e.uploadId;
	DownloadId downloadId = e.downloadId;

	_uploads.erase(itr); // delete first, then notify

	if (channel) channel->send(EVT::REMOTE_DOWNLOAD_CANCEL, new RemoteUploadEvent(this, to, uploadId, downloadId));
}

void Remote::cancelDownload(DownloadId id)
{
	if (_shutdown) return;

	Downloads::iterator itr = _downloads.find(id);
	if (itr == _downloads.end()) return;

	DownloadEntry& e = itr->second;

	Ref<RemotePeer> from = e.from.get();

	if (from)
	{
		from->sendPacket(HDR_DOWNLOAD_CANCEL, &e.uploadId, sizeof(e.uploadId), NULL, 0);
	}

	EventChannel* channel = e.channel;
	UploadId uploadId = e.uploadId;
	DownloadId downloadId = e.downloadId;

	_downloads.erase(itr); // delete first, then notify

	if (channel) channel->send(EVT::REMOTE_UPLOAD_CANCEL, new RemoteUploadEvent(this, from, uploadId, downloadId));
}

bool Remote::getRequestStatus(RequestId id, Timestamp* outStartTime)
{
	Requests::iterator itr = _requests.find(id);

	if (itr == _requests.end()) return false;

	RequestEntry& e = itr->second;

	if (outStartTime)
		*outStartTime = e.startTime;

	return true;
}

bool Remote::getUploadStatus(UploadId id, size_t* outSent, size_t* outSize, Timestamp* outStartTime)
{
	Uploads::iterator itr = _uploads.find(id);

	if (itr == _uploads.end()) return false;

	UploadEntry& e = itr->second;

	if (outSent)
		*outSent = e.streamSize - e.bytesLeft;

	if (outSize)
		*outSize = e.streamSize;

	if (outStartTime)
		*outStartTime = e.startTime;

	return true;
}

bool Remote::getDownloadStatus(DownloadId id, size_t* outReceived, size_t* outSize, Timestamp* outStartTime)
{
	Downloads::iterator itr = _downloads.find(id);

	if (itr == _downloads.end()) return false;

	DownloadEntry& e = itr->second;

	if (outReceived)
		*outReceived = e.streamSize - e.bytesLeft;

	if (outSize)
		*outSize = e.streamSize;

	if (outStartTime)
		*outStartTime = e.startTime;

	return true;
}

void Remote::setRequestTimeout(RequestId id, uint millis)
{
	if (millis == 0)
	{
		_requestTimeouts.erase(id);
		return;
	}

	Timestamp targetTime = Timestamp::now() + double(millis * 0.001);

	Timeouts::iterator itr = _requestTimeouts.find(id);
	if (itr != _requestTimeouts.end())
		itr->second.targetTime = targetTime;
	else
	{
		TimeoutEntry t;
		t.targetTime = targetTime;
		_requestTimeouts.insert(std::make_pair(id, t));
	}
}

void Remote::setUploadTimeout(UploadId id, uint millis)
{
	if (millis == 0)
	{
		_uploadTimeouts.erase(id);
		return;
	}

	Timestamp targetTime = Timestamp::now() + double(millis * 0.001);

	Timeouts::iterator itr = _uploadTimeouts.find(id);
	if (itr != _uploadTimeouts.end())
		itr->second.targetTime = targetTime;
	else
	{
		TimeoutEntry t;
		t.targetTime = targetTime;
		_uploadTimeouts.insert(std::make_pair(id, t));
	}
}

void Remote::setDownloadTimeout(DownloadId id, uint millis)
{
	if (millis == 0)
	{
		_downloadTimeouts.erase(id);
		return;
	}

	Timestamp targetTime = Timestamp::now() + double(millis * 0.001);

	Timeouts::iterator itr = _downloadTimeouts.find(id);
	if (itr != _downloadTimeouts.end())
		itr->second.targetTime = targetTime;
	else
	{
		TimeoutEntry t;
		t.targetTime = targetTime;
		_downloadTimeouts.insert(std::make_pair(id, t));
	}
}

void Remote::updateTimeout()
{
	Timestamp now = Timestamp::now();

	// NOTE: events should dispatched after internal cleanup
	// We repeat codes here so to avoid side-effect of invocation to Cancel()

	// Check request timeout
	for (Timeouts::iterator itr = _requestTimeouts.begin(); itr != _requestTimeouts.end(); )
	{
		TimeoutEntry& t = itr->second;
		if (t.targetTime > now) { itr++; continue; }

		Requests::iterator ritr = _requests.find(itr->first);
		if (ritr == _requests.end()) { _requestTimeouts.erase(itr++); continue; }

		RequestEntry& e = ritr->second;

		Ref<RemoteResponseEvent> evt;
		Ref<EventChannel> channel = e.channel.get();
		if (channel) evt = new RemoteResponseEvent(this, e.to, e.channelId, e.command, e.requestId, RESPONSE_TIMEOUT);

		Ref<RemotePeer> to = e.to.get();

		_requests.erase(ritr);
		_requestTimeouts.erase(itr++);

		if (to)
		{
			struct
			{
				uint16 channelId;
				uint16 command;
				uint32 requestId;
			} hdr = { e.channelId, e.command, e.requestId };

			to->sendPacket(HDR_REQUEST_CANCEL, &hdr, sizeof(hdr), NULL, 0);
		}

		if (evt) channel->send(EVT::REMOTE_RESPONSE, evt);
	}

	// Check upload timeout
	for (Timeouts::iterator itr = _uploadTimeouts.begin(); itr != _uploadTimeouts.end(); )
	{
		TimeoutEntry& t = itr->second;
		if (t.targetTime > now) { itr++; continue; }

		Uploads::iterator uitr = _uploads.find(itr->first);
		if (uitr == _uploads.end()) { _uploadTimeouts.erase(itr++); continue; }

		UploadEntry& e = uitr->second;

		Ref<RemoteUploadEvent> evt;
		Ref<EventChannel> channel = e.channel.get();
		if (channel) evt = new RemoteUploadEvent(this, e.to, e.uploadId, e.downloadId);

		Ref<RemotePeer> to = e.to.get();

		_uploads.erase(uitr);
		_uploadTimeouts.erase(itr++);

		if (to)
		{
			struct  
			{
				uint32 uploadId;
				uint32 downloadId;
			} hdr = { e.uploadId, e.downloadId };

			to->sendPacket(HDR_UPLOAD_CANCEL, &hdr, sizeof(hdr), NULL, 0);
		}

		if (evt) channel->send(EVT::REMOTE_DOWNLOAD_CANCEL, evt);
	}

	// Check download timeout
	for (Timeouts::iterator itr = _downloadTimeouts.begin(); itr != _downloadTimeouts.end(); )
	{
		TimeoutEntry& t = itr->second;
		if (t.targetTime > now) { itr++; continue; }

		Downloads::iterator ditr = _downloads.find(itr->first);
		if (ditr == _downloads.end()) { _downloadTimeouts.erase(itr++); continue; }

		DownloadEntry& e = ditr->second;

		Ref<RemoteUploadEvent> evt;
		Ref<EventChannel> channel = e.channel.get();
		if (channel) evt = new RemoteUploadEvent(this, e.from, e.uploadId, e.downloadId);

		Ref<RemotePeer> from = e.from.get();

		_downloads.erase(ditr);
		_downloadTimeouts.erase(itr++);

		if (from)
		{
			from->sendPacket(HDR_DOWNLOAD_CANCEL, &e.downloadId, sizeof(e.downloadId), NULL, 0);
		}

		if (evt) channel->send(EVT::REMOTE_UPLOAD_CANCEL, evt);
	}
}

void Remote::registerUserMsg(uint16 hdrMsg)
{
	if (hdrMsg < 0x100)
		NIT_THROW(EX_DUPLICATED);

	if (_userMsg.find(hdrMsg) != _userMsg.end())
		NIT_THROW(EX_DUPLICATED);

	_userMsg.insert(hdrMsg);
}

EventChannel* Remote::openChannel(ChannelId id, const String& serviceName, const DataValue& info)
{
	ASSERT_THROW(id != 0, EX_INVALID_PARAMS);

	Channels::iterator itr = _channels.find(id);
	if (itr != _channels.end())
		NIT_THROW(EX_DUPLICATED);

	ChannelEntry e;
	e.channel = new EventChannel();
	e.channelId = id;
	e.serviceName = serviceName;
	e.info = info;

	_channels.insert(std::make_pair(id, e));
	return e.channel;
}

// TODO: On c++0x, we may refactor this struct local
struct Remote::Predicates::CloseChannel
{
	CloseChannel(ChannelId channelId) : channelId(channelId) { }

	bool operator () (RequestEntry& e)		{ return e.channelId == channelId; }
	bool operator () (ResponseEntry& e)		{ return e.channelId == channelId; }
	bool operator () (UploadEntry& e)		{ return e.channelId == channelId; }
	bool operator () (DownloadEntry& e)		{ return e.channelId == channelId; }

	ChannelId channelId;
};

void Remote::closeChannel(ChannelId channelId)
{
	ASSERT_THROW(channelId != 0, EX_INVALID_PARAMS);

	Channels::iterator itr = _channels.find(channelId);
	if (itr == _channels.end()) return;

	cancelEntries(Predicates::CloseChannel(channelId));

	if (_broadcastPeer)
		_broadcastPeer->sendPacket(HDR_CHANNEL_CLOSE, &channelId, sizeof(channelId), NULL, 0);

	ChannelEntry e = itr->second;

	_channels.erase(itr); // delete first, then notify
}

////////////////////////////////////////////////////////////////////////////////

Remote::PacketReader::PacketReader(RemotePeer* peer, MemoryBuffer* recvBuf, size_t pos, size_t packetLen, size_t dataLen) 
: _peer(peer), _recvBuf(recvBuf), _pos(pos), _packetLen(packetLen), _dataLeft(dataLen)
{

}

void Remote::PacketReader::read(void* buf, size_t size)
{
	assert(_recvBuf && "packet consumed");

	_recvBuf->copyTo(buf, _pos, size);
	_pos += size;
	_dataLeft -= size;
}

void Remote::PacketReader::copyWriteTo(StreamWriter* writer, size_t size)
{
	assert(_recvBuf && "packet consumed");

	_recvBuf->save(writer, _pos, size);
	_pos += size;
	_dataLeft -= size;
}

void Remote::PacketReader::readValue(DataValue& outValue)
{
	assert(_recvBuf && "packet consumed");

	if (_dataLeft == 0)
	{
		outValue.toVoid();
		return;
	}

	size_t bytesRead = _peer->_readValue(_recvBuf, _pos, outValue);
	_pos += bytesRead;
	_dataLeft -= bytesRead;
}

void Remote::PacketReader::consume()
{
	if (_recvBuf)
	{
		_recvBuf->popFront(_packetLen);
		_recvBuf = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////

Remote::ResponseId RemoteRequestEvent::delay() const
{
	ASSERT_THROW(!isConsumed(), EX_INVALID_STATE);

	_delayed = true;
	consume();

	_responseID = remote->_nextResponseId++;

	return _responseID;
}

void RemoteRequestEvent::response(int32 code, const DataValue& param) const
{
	ASSERT_THROW(!isConsumed(), EX_INVALID_STATE);

	remote->response(peer, channelId, command, requestId, code, param);
	consume();
}

////////////////////////////////////////////////////////////////////////////////

Remote::DownloadId RemoteUploadStartEvent::download(StreamWriter* writer, uint32 offset, uint32 size, uint16 packetSize) const
{
	ASSERT_THROW(_writerResponse == NULL, EX_INVALID_STATE);
	ASSERT_THROW(!isConsumed(), EX_INVALID_STATE);

	_writerResponse = writer;
	_offsetResponse = offset;
	_sizeResponse = size ? size : streamSize;
	_packetSizeResponse = packetSize;
	consume();

	_downloadID = remote->_nextDownloadId++;

	return _downloadID;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
