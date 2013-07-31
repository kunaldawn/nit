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

#include "nit/net/Socket.h"

#include "nit/runtime/MemManager.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

#ifdef NIT_WIN32
class _WSAAutoInit
{
public:
	_WSAAutoInit()
	{
		WSADATA wsadata;

		if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
		{
			LOG(0, "*** [Winsock] WSAStartup fail\n");
			_success = false;
		}
		else
		{
			_success = true;
		}
	}

	~_WSAAutoInit()
	{
		WSACleanup();
	}

	bool _success;
};


#endif

////////////////////////////////////////////////////////////////////////////////

bool SocketBase::initialize()
{
#ifdef NIT_WIN32
	static _WSAAutoInit init;
	return init._success;
#else
	return true;
#endif
}

SocketBase::SocketBase()
{
	initialize();

	_handle = INVALID_SOCKET;
}

SocketBase::~SocketBase()
{
	assert(_handle == INVALID_SOCKET);
}

bool SocketBase::error(const char* msg, int err)
{
	if (!isValid())
		return false;

	disconnect();
	LOG(0, "*** [Socket] %s: %s (%d)\n", msg, errorToStr(err), err);
	return false;
}

const char* SocketBase::errorToStr(int err)
{
	switch (err)
	{
	case ERR_NOT_SOCK					: return "Socket operation on nonsocket";
	case ERR_INVAL						: return "Invalid argument";
	case ERR_WOULD_BLOCK				: return "Resource temporarily unavailable";
	case ERR_IN_PROGRESS				: return "Operation now in progress";
	case ERR_ALREADY					: return "Operation already in progress";
	case ERR_MSGSIZE					: return "Message too long";
	case ERR_PROTOTYPE					: return "Protocol wrong type for socket";
	case ERR_NOPROTO_OPT				: return "Bad protocol option";
	case ERR_PF_NO_SUPPORT				: return "Protocol family not supported";
	case ERR_AF_NO_SUPPORT				: return "Address family not supported by protocol family";
	case ERR_CONN_REFUSED				: return "Connection refuesed";
	case ERR_HOST_UNREACH				: return "No route to host";
	case ERR_ACCESS						: return "Permission denied";
	case ERR_FAULT						: return "Bad address";
	case ERR_INTR						: return "Interrupted function call";
	case ERR_MFILE						: return "Too many open files";
	case ERR_PROTO_NO_SUPPORT			: return "Protocol not supported";
	case ERR_SOCKT_NO_SUPPORT			: return "Socket type not supported";
	case ERR_OP_NOT_SUPP				: return "Operation not supported";
	case ERR_NOT_INITIALIZED			: return "Successful WSAStartup not yet performed";
	case ERR_ADDR_IN_USE				: return "Address already in use";
	case ERR_ADDR_NOT_AVAIL				: return "Cannot assign requested address";
	case ERR_NET_DOWN					: return "Network is down";
	case ERR_NET_UNREACH				: return "Network is unreachable";
	case ERR_NET_RESET					: return "Network dropped connection on reset";
	case ERR_CONN_ABORTED				: return "Software caused connection abort";
	case ERR_CONN_RESET					: return "Connection reset by peer";
	case ERR_NO_BUFS					: return "No buffer space available";
	case ERR_IS_CONN					: return "Socket is already connected";
	case ERR_NOT_CONN					: return "Socket is not connected";
	case ERR_SHUTDOWN					: return "Cannot send after socket shutdown";
	case ERR_TIMED_OUT					: return "Connection timed out";
	default								: return "<unknown>";
	}
}

bool SocketBase::setNonBlocking(bool flag)
{
	ASSERT_THROW(isValid(), EX_INVALID_STATE);
	//	ASSERT_THROW(!IsConnected(), EX_INVALID_STATE);

	// convert to non-blocking socket
	u_long mode = flag ? 1 : 0;
	if (::ioctlsocket(_handle, FIONBIO, &mode))
	{
		return error("Connect.ioctrlsocket", getLastError());
	}

	return true;
}

void SocketBase::disconnect()
{
	if (_handle != INVALID_SOCKET)
	{
		closesocket(_handle);
		_handle = INVALID_SOCKET;

		onDisconnect();
	}
}

////////////////////////////////////////////////////////////////////////////////

TcpSocket::RecvBuffer::RecvBuffer(size_t blockSize)
: MemoryBuffer(blockSize)
{
}

int TcpSocket::RecvBuffer::receive(Handle handle)
{
	while (true)
	{
		size_t blockIdx = _end / _blockSize;
		size_t blockPos = _end % _blockSize;

		if (blockIdx == _blocks.size())
			_blocks.push_back(allocateBlock());

		uint8* buf = _blocks[blockIdx] + blockPos;
		size_t bufSize = _blockSize - blockPos;

		int read = ::recv(handle, (char*)buf, bufSize, NIT_SOCKET_SENDRECV_FLAGS);

		if (read == SOCKET_ERROR || read == 0)
			return read;

		_end += read;

		assert(blockPos <= _blockSize);
	}
}

////////////////////////////////////////////////////////////////////////////////

static size_t s_DefaultTcpRecvBlockSize = 4096;
static size_t s_DefaultTcpSendBlockSize = 4096;

TcpSocket::TcpSocket(IListener* listener, const String& address, ushort port)
{
	_listener = listener;

	SOCKET socket = ::socket(AF_INET, SOCK_STREAM, 0);
	_handle = socket;

	sockaddr_in serverAddr = { 0 };
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(address.c_str());
	serverAddr.sin_port = htons(port);
	
	_connecting = false;
	_connected = false;

	if (!setNonBlocking(true))
		return;
	
	// start connection
	if (::connect(socket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		if (getLastError() != ERR_WOULD_BLOCK && getLastError() != ERR_IN_PROGRESS)
		{
			error("Connect", getLastError());
			return;
		}
	}

	_addr = address;
	_port = port;
	_connecting = true;
	_connected = false;

	_recvBuf = new RecvBuffer(s_DefaultTcpRecvBlockSize);
	_sendBuf = new MemoryBuffer(s_DefaultTcpSendBlockSize);
}

TcpSocket::TcpSocket(IListener* listener, int socketHandle, const String& addr, uint16 port)
{
	_listener = listener;

	_handle = socketHandle;
	_addr = addr;
	_port = 0;
	_connecting = true;
	_connected = false;

	_recvBuf = new RecvBuffer(s_DefaultTcpRecvBlockSize);
	_sendBuf = new MemoryBuffer(s_DefaultTcpSendBlockSize);
}

TcpSocket::~TcpSocket()
{
	assert(!_connected);
}

void TcpSocket::onDisconnect()
{
	_connecting = false;

	// HACK: Here we decided not to touch or clear the buffer 
	// because that provokes head-aches on disconnect.
	bool clearBufs = false;

	if (clearBufs)
	{
		_sendBuf->clear();
		_recvBuf->clear();
	}

	if (_connected && _listener) 
	{
		_connected = false;
		_listener->onDisconnect(this);
	}
}

bool TcpSocket::update()
{
	if (!isValid()) return false;

	Ref<TcpSocket> safe = this;

	// try non-blocking select
	TIMEVAL timeout = { 0, 0 };
	fd_set read_flags, write_flags, err_flags;
	FD_ZERO(&read_flags);	FD_SET(_handle, &read_flags);
	FD_ZERO(&write_flags);	FD_SET(_handle, &write_flags);
	FD_ZERO(&err_flags);	FD_SET(_handle, &err_flags);

	int r = ::select(NULL, &read_flags, &write_flags, &err_flags, &timeout);

	if (r == SOCKET_ERROR)
	{
		return error("Select", getLastError());
	}
	
	bool canRecv = FD_ISSET(_handle, &read_flags) != 0;
	bool canSend = FD_ISSET(_handle, &write_flags) != 0;
	bool hasError = FD_ISSET(_handle, &err_flags) != 0;

	FD_CLR(_handle, &read_flags);
	FD_CLR(_handle, &write_flags);
	FD_CLR(_handle, &err_flags);

	if (hasError)
	{
		int err = ERR_OP_NOT_SUPP;
		socklen_t len = sizeof(err);

		getsockopt(_handle, SOL_SOCKET, SO_ERROR, (char*)&err, &len);

		if (err)
			return error("Except", err);
	}

	if (!canRecv && !canSend) 
		return true;

	if (canSend && _connecting)
	{
		_connecting = false;
		_connected = true;

		if (_listener) _listener->onConnect(this);
	}

	if (canSend)
	{
		if (!updateSend())
		{
			disconnect();
			return false;
		}
	}

	if (canRecv)
	{
		if (!updateRecv())
		{
			disconnect();
			return false;
		}
	}

	if (_connected && _listener) _listener->onUpdate();

	return isValid();
}

bool TcpSocket::updateRecv()
{
	// Buffering into recv buffer
	int result = _recvBuf->receive(_handle);

	if (result == SOCKET_ERROR)
	{
		int err = getLastError();
		if (err != ERR_WOULD_BLOCK)
			return error("UpdateRecv", err);
	}
	else if (result == 0)
	{
		return error("Disconnected", ERR_CONN_RESET);
	}

	// At this point, all the bytes underlying API could read has been read and got WOULD_BLOCK state
	bool ok = _listener && _listener->onRecv(this); // It's listeners job to examine & consume (pop-front) buffer

	if (!ok || !_connected)
		return false;

	return true;
}

bool TcpSocket::updateSend()
{
	// Notify listener that it's time to send.
	if (_listener && !_listener->onSend(this))
		return false;

	return flushSendBuffer();
}

bool TcpSocket::flushSendBuffer()
{
	// Do nothing if buffer is empty
	if (_sendBuf->isEmpty()) return true;

	// Do nothing if connection in progress. (until connected)
	if (isConnecting()) return true;

	int totalSent = 0;
	bool blocked = false;

	// Send blocks of the send buf sequentially
	for (uint i=0; !blocked && i < _sendBuf->getNumBlocks(); ++i)
	{
		uint8* buf = NULL;
		size_t size = 0;

		_sendBuf->getBlock(i, buf, size);

		while (size > 0)
		{
			// Chop and feed to underlying API 
			int sent = ::send(_handle, (const char*)buf, size, NIT_SOCKET_SENDRECV_FLAGS);

			if (sent == SOCKET_ERROR)
			{
				int err = getLastError();
				if (err == ERR_WOULD_BLOCK)
				{
					// All the bytes the underlying API could send has been sent.
					blocked = true; 
					break;
				}
				else
					return error("UpdateSend", err);
			}

			assert(size_t(sent) <= size);

			buf += sent;
			totalSent += sent;
			size -= sent;
		}
	}

	assert(size_t(totalSent) <= _sendBuf->getSize());

	// Remove sent bytes from send buf
	_sendBuf->popFront(totalSent);

	return true;
}

bool TcpSocket::send(const void* data, size_t size)
{
	const char* buf = (const char*)data;

	if (isConnecting() || !_sendBuf->isEmpty())
	{
		// If buffering, just add tail to send buf then flush
		_sendBuf->pushBack(buf, size);
		return flushSendBuffer();
	}

	if (isConnected())
	{
		// If not buffering, try to send at once
		int sent = ::send(_handle, buf, size, 0);

		if (sent == size)
			return true;

		if (sent == SOCKET_ERROR)
		{
			int err = getLastError();
			if (err == ERR_WOULD_BLOCK)
				sent = 0;
			else
				return error("Send", err);
		}
		else if (sent == 0)
		{
			return error("Disconnected", ERR_CONN_RESET);
		}

		assert(size_t(sent) < size);

		// We have some remainder - save into sendbuf the remaing bytes
		buf += sent;
		size -= sent;
		_sendBuf->pushBack(buf, size);
		return true;
	}

	// Connecting in progress or not connected
	return false;
}

bool TcpSocket::send(DataToSend* data)
{
	if (!isValid()) return false;

	if (data == NULL) return true;

	switch (data->getType())
	{
	case DataToSend::DS_NONE:
		return true;

	case DataToSend::DS_BUF:
		// Copy into send buf and then flush
		_sendBuf->copyFrom(data->getBuffer(), data->getOffset(), _sendBuf->getSize(), data->getSize());
		return flushSendBuffer();

	case DataToSend::DS_READER:
		// Copy into send buf and then flush
		if (data->getOffset())
			data->getReader()->skip(data->getOffset());
		_sendBuf->load(data->getReader(), _sendBuf->getSize(), data->getSize());
		return flushSendBuffer();

	case DataToSend::DS_BYTES:
		return send(((const char*)data->getBytes()) + data->getOffset(), data->getSize());

	default:
		NIT_THROW(EX_NOT_SUPPORTED);
	}
}

bool TcpSocket::error(const char* msg, int err)
{
	SocketBase::error(msg, err);
	if (_listener) _listener->onError(this, msg, err); 
	return false;
}

////////////////////////////////////////////////////////////////////////////////

TcpSocketServer::TcpSocketServer(IListener* listener)
{
	_listener = listener;
	_listening = false;
	_bindPort = 0;
}

TcpSocketServer::~TcpSocketServer()
{
	assert(!_listening);
}

bool TcpSocketServer::listen(uint16 port)
{
	if (isListening())
		shutdown();

	_handle = socket(AF_INET, SOCK_STREAM, 0);

	// Unix sockets does not released when closed and there are about 4 minutes over TIME_WAIT,
	// tools like nit-rsh should use SO_REUSEADDR.
	// TODO: But there was also the case of 'bind fail: addr in use' so investigate more.
	
	long reuseaddr = 1;
	if (::setsockopt(_handle, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseaddr, sizeof(reuseaddr)) == SOCKET_ERROR)
	{
		return error("setsockopt(SO_REUSE_ADDR)", getLastError());
	}

	sockaddr_in bindAddr = { 0 };
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_port = htons(port);
	bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (::bind(_handle, (sockaddr*)&bindAddr, sizeof(bindAddr)) == SOCKET_ERROR)
	{
		return error("Bind", getLastError());
	}

	if (!setNonBlocking(true))
		return false;

	if (::listen(_handle, 0) == SOCKET_ERROR)
	{
		return error("Listen", getLastError());
	}

	_bindAddr = inet_ntoa(bindAddr.sin_addr);
	_bindPort = port;

	LOG(0, "++ ServerSocket: Listen to %s:%d\n", _bindAddr.c_str(), (int)_bindPort);
	_listening = true;

	if (_listener) _listener->onListen(this);

	return true;
}

void TcpSocketServer::shutdown()
{
	if (_listening)
		LOG(0, "++ ServerSocket: Shutdown %s:%d\n", _bindAddr.c_str(), (int)_bindPort);

	disconnect();
}

bool TcpSocketServer::update()
{
	if (!isValid())
		return false;

	sockaddr_in clientAddr = { 0 };
	socklen_t addrLen = sizeof(clientAddr);
	SOCKET accepted = ::accept(_handle, (sockaddr*)&clientAddr, &addrLen);

	if (accepted != INVALID_SOCKET)
	{
		String peerAddr = inet_ntoa(clientAddr.sin_addr);
		uint16 peerPort = clientAddr.sin_port;

		TcpSocket* client = _listener ? _listener->onAccept(this, accepted, peerAddr, peerPort) : NULL;

		if (client == NULL)
		{
			::closesocket(accepted);
			LOG(0, "++ ServerSocket: '%s: %d' refused\n", peerAddr.c_str(), peerPort);
		}
		else
		{
			LOG(0, "++ ServerSocket: '%s: %d' accepted\n", peerAddr.c_str(), peerPort);
			_clients.insert(client);
		}
	}
	else
	{
		int err = getLastError();
		if (err != ERR_WOULD_BLOCK)
			return error("Listen", err);
	}

	for (Clients::iterator itr = _clients.begin(), end = _clients.end(); itr != end; )
	{
		TcpSocket* client = *itr;
		if (!client->update())
		{
			if (_listener) _listener->onDisconnected(this, client);
			_clients.erase(itr++);
		}
		else
		{
			++itr;
		}
	}

	if (_listener) _listener->onUpdate();

	return true;
}

void TcpSocketServer::onDisconnect()
{
	for (Clients::iterator itr = _clients.begin(), end = _clients.end(); itr != end; ++itr)
	{
		TcpSocket* client = *itr;
		client->disconnect();
		if (_listener) _listener->onDisconnected(this, client);
	}
	_clients.clear();

	if (_listening)
	{
		if (_listener) _listener->onShutdown(this);
		_listening = false;
	}
}

void TcpSocketServer::broadcast(const void* data, size_t size)
{
	for (Clients::iterator itr = _clients.begin(), end = _clients.end(); itr != end; ++itr)
	{
		TcpSocket* client = *itr;
		client->send(data, size);
	}
}

void TcpSocketServer::broadcast(DataToSend* data)
{
	if (data == NULL) return;

	if (data->getType() != DataToSend::DS_READER)
	{
		for (Clients::iterator itr = _clients.begin(), end = _clients.end(); itr != end; ++itr)
		{
			TcpSocket* client = *itr;
			client->send(data);
		}
	}
	else
	{
		Ref<StreamReader> reader = data->getReader();
		ASSERT_THROW(reader->isSeekable(), EX_NOT_SUPPORTED);

		size_t pos = reader->tell();

		for (Clients::iterator itr = _clients.begin(), end = _clients.end(); itr != end; ++itr)
		{
			reader->seek(pos);
			TcpSocket* client = *itr;
			client->send(data);
		}
	}
}

bool TcpSocketServer::error(const char* msg, int err)
{
	SocketBase::error(msg, err);
	if (_listener) _listener->onError(this, msg, err);
	return false;
}

bool TcpSocketServer::isClient(TcpSocket* socket)
{
	return _clients.find(socket) != _clients.end();
}

////////////////////////////////////////////////////////////////////////////////

UdpSocket::UdpSocket(IListener* listener)
{
	_listener = listener;

	SOCKET socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	_handle = socket;

	_maxMsgSize = 0;
#if defined(NIT_WIN32)
	socklen_t optLen = sizeof(_maxMsgSize);
	getsockopt(_handle, SOL_SOCKET, SO_MAX_MSG_SIZE, (char*) &_maxMsgSize, &optLen);
#else
	// TODO: Reimplement on IOS
	_maxMsgSize = 65507;
#endif

//	LOG(0, "udp max msg size: %d\n", _maxMsgSize);

	// Turn on broadcasting (Generally UDP is meaningless without this)
	long broadcast = 1;

	if (setsockopt(_handle, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast, sizeof(broadcast)) == SOCKET_ERROR)
		error("can't set SO_BROADCAST", getLastError());

	// Turn on reuse-addr (which allows duplicated ip & port on bind)
	long reuseAddr = 1;
	if (setsockopt(_handle, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseAddr, sizeof(reuseAddr)) == SOCKET_ERROR)
		error("can't set SO_REUSEADDR", getLastError());

	// Turn on non-blocking
	setNonBlocking(true);

	_bindPort = 0;

	_recvBuf = NULL;
	_recvBufSize = 0;
}

UdpSocket::~UdpSocket()
{
}

bool UdpSocket::bind(uint16 port, uint bufSize)
{
	if (!isValid()) return false;

	if (bufSize > _maxMsgSize)
		return error("Too large buffer", ERR_MSGSIZE);

	if (port == 0)
		return error("Invalid port", ERR_INVAL);

	if (_bindPort != 0)
		return error("Bind(): already bound", ERR_ALREADY);

	sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if (::bind(_handle, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
		return error("bind() failed", getLastError());

	_bindAddr = inet_ntoa(addr.sin_addr);
	_bindPort = port;

	if (_recvBuf)
		NIT_DEALLOC(_recvBuf, _recvBufSize);

	_recvBufSize = bufSize;
	_recvBuf = (uint8*)NIT_ALLOC(bufSize);

	return true;
}

bool UdpSocket::sendTo(const String& addr, uint16 port, const char* buf, size_t size)
{
	if (!isValid()) return false;

	sockaddr_in dest = { 0 };
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr(addr.c_str());
	dest.sin_port = htons(port);

	int sent = ::sendto(_handle, buf, size, 0, (sockaddr*)&dest, sizeof(dest));

	if (sent == SOCKET_ERROR)
	{
		int err = getLastError();
		if (err != ERR_WOULD_BLOCK)
			return error("SendTo", err);
	}

	if (sent != size)
	{
		return error("SendTo: truncated", ERR_MSGSIZE);
	}

	return true;
}

bool UdpSocket::update()
{
	if (!isValid()) return false;
	if (!isBound()) return false;

	TIMEVAL timeout = { 0, 0 };
	fd_set read_flags;
	FD_ZERO(&read_flags); FD_SET(_handle, &read_flags);

	int r = ::select(NULL, &read_flags, NULL, NULL, &timeout);

	if (r == SOCKET_ERROR)
		return error("Select", getLastError());

	bool canRecv = FD_ISSET(_handle, &read_flags) != 0;
	FD_CLR(_handle, &read_flags);

	if (canRecv)
	{
		if (!updateRecv())
		{
			disconnect();
			return false;
		}
	}

	return isValid();
}

bool UdpSocket::updateRecv()
{
	while (true)
	{
		sockaddr_in from = { 0, 0 };
		socklen_t fromLen = sizeof(from);

		int read = ::recvfrom(_handle, (char*)_recvBuf, _recvBufSize, 0, (sockaddr*)&from, &fromLen);

		if (read == SOCKET_ERROR)
		{
			int err = getLastError();
			if (err == ERR_WOULD_BLOCK)
				break;

			return error("RecvFrom", getLastError());
		}

		String addr = inet_ntoa(from.sin_addr);
		uint16 port = from.sin_port;

		if (_listener) _listener->onUdpRecv(this, addr, port, _recvBuf, read);
	}

	// All packets received at this frame processed.
	return true;
}

void UdpSocket::onDisconnect()
{
	if (_recvBuf)
	{
		NIT_DEALLOC(_recvBuf, _recvBufSize);
		_recvBuf = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
