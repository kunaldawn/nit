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

#include "nit/io/MemoryBuffer.h"

////////////////////////////////////////////////////////////////////////////////

// TODO: Ad-hoc platform-wise setting

#if defined(NIT_WIN32)
#	define socklen_t					int
#endif

#if !defined(NIT_WIN32)
	typedef int							SOCKET;
#	define SOCKET_ERROR					-1
#	define INVALID_SOCKET				-1
#	define WSAGetLastError()			errno
#	define WSAEWOULDBLOCK				EWOULDBLOCK
#	define TIMEVAL						struct timeval
#	define closesocket					close
#	define ioctlsocket					ioctl
#endif

#if defined(NIT_FAMILY_UNIX)
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <arpa/inet.h>
#	include <sys/errno.h>
#	include <sys/ioctl.h>
#endif

#if defined(NIT_ANDROID)
#	define NIT_SOCKET_SENDRECV_FLAGS MSG_DONTWAIT
#else
#	define NIT_SOCKET_SENDRECV_FLAGS 0
#endif


////////////////////////////////////////////////////////////////////////////////

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NIT_API SocketBase : public RefCounted
{
protected:
	SocketBase();
	virtual ~SocketBase();

public:
	typedef SOCKET						Handle;

	static bool							initialize();

	bool								isValid()								{ return _handle != INVALID_SOCKET; }

	virtual void						disconnect();

protected:
	bool								setNonBlocking(bool flag);

public:
#ifdef NIT_FAMILY_WIN32
	enum ErrorCode
	{
		ERR_NOT_SOCK					= WSAENOTSOCK,			// Socket operation on nonsocket
		ERR_INVAL						= WSAEINVAL,			// Invalid argument
		ERR_WOULD_BLOCK					= WSAEWOULDBLOCK,		// Resource temporarily unavailable
		ERR_IN_PROGRESS					= WSAEINPROGRESS,		// Operation now in progress
		ERR_ALREADY						= WSAEALREADY,			// Operation already in progress
		ERR_MSGSIZE						= WSAEMSGSIZE,			// Message too long
		ERR_PROTOTYPE					= WSAEPROTOTYPE,		// Protocol wrong type for socket
		ERR_NOPROTO_OPT					= WSAENOPROTOOPT,		// Bad protocol option
		ERR_PF_NO_SUPPORT				= WSAEPFNOSUPPORT,		// Protocol family not supported
		ERR_AF_NO_SUPPORT				= WSAEAFNOSUPPORT,		// Address family not supported by protocol family
		ERR_CONN_REFUSED				= WSAECONNREFUSED,		// Connection refuesed
		ERR_HOST_UNREACH				= WSAEHOSTUNREACH,		// No route to host
		ERR_ACCESS						= WSAEACCES,			// Permission denied
		ERR_FAULT						= WSAEFAULT,			// Bad address
		ERR_INTR						= WSAEINTR,				// Interrupted function call
		ERR_MFILE						= WSAEMFILE,			// Too many open files
		ERR_PROTO_NO_SUPPORT			= WSAEPROTONOSUPPORT,	// Protocol not supported
		ERR_SOCKT_NO_SUPPORT			= WSAESOCKTNOSUPPORT,	// Socket type not supported
		ERR_OP_NOT_SUPP					= WSAEOPNOTSUPP,		// Operation not supported
		ERR_NOT_INITIALIZED				= WSANOTINITIALISED,	// Successful WSAStartup not yet performed
		ERR_ADDR_IN_USE					= WSAEADDRINUSE,		// Address already in use
		ERR_ADDR_NOT_AVAIL				= WSAEADDRNOTAVAIL,		// Cannot assign requested address
		ERR_NET_DOWN					= WSAENETDOWN,			// Network is down
		ERR_NET_UNREACH					= WSAENETUNREACH,		// Network is unreachable
		ERR_NET_RESET					= WSAENETRESET,			// Network dropped connection on reset
		ERR_CONN_ABORTED				= WSAECONNABORTED,		// Software caused connection abort
		ERR_CONN_RESET					= WSAECONNRESET,		// Connection reset by peer
		ERR_NO_BUFS						= WSAENOBUFS,			// No buffer space available
		ERR_IS_CONN						= WSAEISCONN,			// Socket is already connected
		ERR_NOT_CONN					= WSAENOTCONN,			// Socket is not connected
		ERR_SHUTDOWN					= WSAESHUTDOWN,			// Cannot send after socket shutdown
		ERR_TIMED_OUT					= WSAETIMEDOUT,			// Connection timed out
	};
#endif

#ifdef NIT_FAMILY_UNIX
	enum ErrorCode
	{
		ERR_NOT_SOCK					= ENOTSOCK,			// Socket operation on nonsocket
		ERR_INVAL						= EINVAL,			// Invalid argument
		ERR_WOULD_BLOCK					= EWOULDBLOCK,		// Resource temporarily unavailable
		ERR_IN_PROGRESS					= EINPROGRESS,		// Operation now in progress
		ERR_ALREADY						= EALREADY,			// Operation already in progress
		ERR_MSGSIZE						= EMSGSIZE,			// Message too long
		ERR_PROTOTYPE					= EPROTOTYPE,		// Protocol wrong type for socket
		ERR_NOPROTO_OPT					= ENOPROTOOPT,		// Bad protocol option
		ERR_PF_NO_SUPPORT				= EPFNOSUPPORT,		// Protocol family not supported
		ERR_AF_NO_SUPPORT				= EAFNOSUPPORT,		// Address family not supported by protocol family
		ERR_CONN_REFUSED				= ECONNREFUSED,		// Connection refuesed
		ERR_HOST_UNREACH				= EHOSTUNREACH,		// No route to host
		ERR_ACCESS						= EACCES,			// Permission denied
		ERR_FAULT						= EFAULT,			// Bad address
		ERR_INTR						= EINTR,				// Interrupted function call
		ERR_MFILE						= EMFILE,			// Too many open files
		ERR_PROTO_NO_SUPPORT			= EPROTONOSUPPORT,	// Protocol not supported
		ERR_SOCKT_NO_SUPPORT			= ESOCKTNOSUPPORT,	// Socket type not supported
		ERR_OP_NOT_SUPP					= EOPNOTSUPP,		// Operation not supported
		ERR_NOT_INITIALIZED				= -999,				// Successful WSAStartup not yet performed
		ERR_ADDR_IN_USE					= EADDRINUSE,		// Address already in use
		ERR_ADDR_NOT_AVAIL				= EADDRNOTAVAIL,		// Cannot assign requested address
		ERR_NET_DOWN					= ENETDOWN,			// Network is down
		ERR_NET_UNREACH					= ENETUNREACH,		// Network is unreachable
		ERR_NET_RESET					= ENETRESET,			// Network dropped connection on reset
		ERR_CONN_ABORTED				= ECONNABORTED,		// Software caused connection abort
		ERR_CONN_RESET					= ECONNRESET,		// Connection reset by peer
		ERR_NO_BUFS						= ENOBUFS,			// No buffer space available
		ERR_IS_CONN						= EISCONN,			// Socket is already connected
		ERR_NOT_CONN					= ENOTCONN,			// Socket is not connected
		ERR_SHUTDOWN					= ESHUTDOWN,			// Cannot send after socket shutdown
		ERR_TIMED_OUT					= ETIMEDOUT,			// Connection timed out
	};
#endif
	
	static int							getLastError()							{ return WSAGetLastError(); }
	static const char*					errorToStr(int err);
	virtual bool						error(const char* msg, int err);

protected:
	Handle								_handle;

	virtual void						onDisconnect() = 0;
	virtual void						onDelete()								{ disconnect(); }
};

////////////////////////////////////////////////////////////////////////////////

// TODO: Generalize more!

class NIT_API DataToSend
{
public:
	enum Type 
	{
		DS_NONE		= 0x0000,
		DS_BYTES	= 0x0001, 
		DS_BUF		= 0x1000,
		DS_READER	= 0x1001,

		FLAG_REFCOUNTED = 0x1000,
	};

	inline DataToSend() : _type(DS_NONE), _offset(0), _size(0), _ptr(0)													{ }

	inline DataToSend(MemoryBuffer* buf) : _type(DS_BUF), _offset(0), _size(buf->getSize()), _buffer(buf)				{ buf->incRefCount(); }
	inline DataToSend(const char* str) : _type(DS_BYTES), _offset(0), _size(strlen(str)), _bytes(str)					{ }

	inline DataToSend(const void* bytes, size_t size) : _type(DS_BYTES), _offset(0), _size(size), _bytes(bytes)			{ }
	inline DataToSend(MemoryBuffer* buf, size_t size) : _type(DS_BUF), _offset(0), _size(size), _buffer(buf)			{ buf->incRefCount(); }
	inline DataToSend(StreamReader* reader, size_t size) : _type(DS_READER), _offset(0), _size(size), _reader(reader)	{ reader->incRefCount(); }

	inline DataToSend(const void* bytes, size_t offset, size_t size) : _type(DS_BYTES), _offset(offset), _size(size), _bytes(bytes)			{ }
	inline DataToSend(MemoryBuffer* buf, size_t offset, size_t size) : _type(DS_BUF), _offset(offset), _size(size), _buffer(buf)			{ buf->incRefCount(); }
	inline DataToSend(StreamReader* reader, size_t offset, size_t size) : _type(DS_READER), _offset(offset), _size(size), _reader(reader)	{ reader->incRefCount(); }

	inline DataToSend(const DataToSend& other)
		: _type(other._type), _offset(other._offset), _size(other._size), _ptr(other._ptr)
	{
		if (_type & FLAG_REFCOUNTED) _refCounted->incRefCount();
	}

	inline DataToSend& operator = (const DataToSend& other)
	{
		if (other._type & FLAG_REFCOUNTED) other._refCounted->incRefCount();
		RefCounted* temp = (_type & FLAG_REFCOUNTED) ? _refCounted : NULL;
		_type = other._type; _offset = other._offset; _size = other._size; _ptr = other._ptr;
		if (temp) temp->decRefCount();
		return *this;
	}

	inline ~DataToSend()
	{
		if (_type & FLAG_REFCOUNTED) _refCounted->decRefCount();
	}

public:
	Type								getType() const							{ return _type; }
	size_t								getOffset() const						{ return _offset; }
	size_t								getSize() const							{ return _size; }

	const void*							getBytes() const						{ return _bytes; }
	MemoryBuffer*						getBuffer() const						{ return _buffer; }
	StreamReader*						getReader() const						{ return _reader; }

private:
	Type								_type;
	size_t								_offset;
	size_t								_size;

	union
	{
		const void*						_ptr;
		const void*						_bytes;
		MemoryBuffer*					_buffer;
		StreamReader*					_reader;
		RefCounted*						_refCounted;
	};
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API TcpSocket : public SocketBase
{
public:
	class NIT_API IListener
	{
	public:
		// (Optional) Called after a connection established and ready to send/recv.
		virtual void					onConnect(TcpSocket* socket)			{ }

		// (Optional) Called after internal send / recv update.
		virtual void					onUpdate()								{ }

		// (Required) Called after received some data. You may return false when something is wrong.
		virtual bool					onRecv(TcpSocket* socket) = 0;

		// (Optional) Called when it's time to send some data.
		virtual bool					onSend(TcpSocket* socket)				{ return true; }

		// (Optional) Called after buffered data sent to underlying API.
		virtual void					onSent(TcpSocket* socket, const void* buf, int size) { }

		// (Optional) Called after the connection closed.
		virtual void					onDisconnect(TcpSocket* socket)			{ }

		// (Optional) Called after an error occurs
		virtual void					onError(TcpSocket* socket, const char* msg, int err) { }
	};

public:
	TcpSocket(IListener* listener, const String& address, ushort port);
	TcpSocket(IListener* listener, int socketHandle, const String& addr, uint16 port);
	virtual ~TcpSocket();

public:
	const String&						getAddr()								{ return _addr; }
	uint16								getPort()								{ return _port; }

	bool								isConnected()							{ return _connected; }
	bool								isConnecting()							{ return _connecting; }

public:
	MemoryBuffer*						getRecvBuf()							{ return _recvBuf; }
	MemoryBuffer*						getSendBuf()							{ return _sendBuf; }

	bool								send(const void* data, size_t size);
	bool								send(DataToSend* data);

	template <typename TValue>
	bool								send(const TValue& value)				{ return send(&value, sizeof(value)); }

	bool								flushSendBuffer();

	virtual bool						error(const char* msg, int err);

public:
	bool								update();

	IListener*							replaceListener(IListener* listener)	{ IListener* old = _listener; _listener = listener; return old; }

protected:
	bool								updateRecv();
	bool								updateSend();

	virtual void						onDisconnect();

protected:
	String								_addr;
	uint16								_port;

	bool								_connecting : 1;
	bool								_connected : 1;

	IListener*							_listener;

public:
	class NIT_API RecvBuffer : public MemoryBuffer
	{
	public:
		RecvBuffer(size_t blockSize);

		int receive(Handle handle);
	};

	Ref<RecvBuffer>						_recvBuf;
	Ref<MemoryBuffer>					_sendBuf;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API TcpSocketServer : public SocketBase
{
public:
	class NIT_API IListener
	{
	public:
		// (Optional) Called after listening starts.
		virtual void					onListen(TcpSocketServer* server)		{ }

		// (Optional) Called after internal send / recv update.
		virtual void					onUpdate()								{ }

		// (Required) Called when establishing a new connection - You may return NULL if undesired.
		virtual TcpSocket*				onAccept(TcpSocketServer* server, int socketHandle, const String& peerAddr, uint16 peerPort) = 0;

		// (Optional) Called after a connection closed.
		virtual void 					onDisconnected(TcpSocketServer* server, TcpSocket* client) { }

		// (Optional) Called after stop listening or shutdown all.
		virtual void					onShutdown(TcpSocketServer* server)		{ }

		// (Optional) Called after an error occurs.
		virtual void					onError(TcpSocketServer* socket, const char* msg, int err) { }
	};

public:
	TcpSocketServer(IListener* listener);
	virtual ~TcpSocketServer();

public:
	const String&						getBindAddr()							{ return _bindAddr; }
	uint16								getBindPort()							{ return _bindPort; }

	bool								isListening()							{ return _listening; }

	typedef set<Ref<TcpSocket> >::type Clients;

	uint								getNumClients()							{ return _clients.size(); }
	const Clients&						getClients()							{ return _clients; }
	bool								isClient(TcpSocket* socket);

public:
	bool								listen(uint16 port);
	void								shutdown()								{ disconnect(); }

	virtual bool						error(const char* msg, int err);

public:
	void								broadcast(const void* data, size_t size);
	void								broadcast(DataToSend* data);

	template <typename TValue>
	void								broadcast(const TValue& value)			{ broadcast(&value, sizeof(value)); }

public:
	bool								update();

protected:
	bool								_listening : 1;

	String								_bindAddr;
	uint16								_bindPort;

	IListener*							_listener;

	Clients								_clients;

	virtual void						onDisconnect();
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API UdpSocket : public SocketBase
{
public:
	class NIT_API IListener
	{
	public:
		virtual void					onUdpRecv(UdpSocket* socket, const String& peerAddr, uint16 peerPort, uint8* msg, size_t len) { }
	};

public:
	UdpSocket(IListener* listener);
	virtual ~UdpSocket();

public:
	bool								isBound()								{ return _bindPort != 0; }
	const String&						getBindAddr()							{ return _bindAddr; }
	uint16								getBindPort()							{ return _bindPort; }
	uint								getRecvBufSize()						{ return _recvBufSize; }
	uint								getMaxMsgSize()							{ return _maxMsgSize; }

public:
	bool								bind(uint16 port, uint bufSize = 4096);

	bool								sendTo(const String& addr, uint16 port, const char* buf, size_t size);

public:
	bool								update();

protected:
	IListener*							_listener;

	String								_bindAddr;
	uint16								_bindPort;

	uint								_maxMsgSize;
	uint								_recvBufSize;
	uint8*								_recvBuf;

	bool								updateRecv();

	virtual void						onDisconnect();
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
