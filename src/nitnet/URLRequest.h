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

#include "nitnet/nitnet.h"
#include "nit/io/MemoryBuffer.h"
#include "nit/io/Stream.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

#define g_Net							static_cast<nit::NetService*>(g_Service->getService(::nit::Service::SVC_NET))

////////////////////////////////////////////////////////////////////////////////

class NetService;

class NITNET_API URLRequest : public RefCounted
{
public:
	URLRequest();

public:
	CURL*								getHandle()								{ return _handle; }
	virtual const char*					getTypeName()							{ return "URLRequest"; }
	const String&						getUrl()								{ return _url; }
	float								getStartTime()							{ return _startTime; }

	bool								isBusy()								{ return _handle != NULL; }

	int									getError()								{ return _error; }
	const char*							getErrorString()						{ return curl_easy_strerror(_error); }

	void								cancel(bool cleanup = true);

	EventChannel*						channel()								{ return _channel ? _channel : _channel = new EventChannel(); }
	SQRESULT							wait(HSQUIRRELVM v);

	static String						urlEncode(const char* str, int len = -1, bool fieldMode = false);

protected:
	virtual bool						onStart(CURL* handle) = 0;
	virtual void						onDone() = 0;
	virtual void						onError(CURLcode err) = 0;
	virtual int							onProgress(double dltotal, double dlnow, double ultotal, double ulnow) { return 0; }
	virtual size_t						onReceiveHeader(char* ptr, size_t size) = 0;
	virtual size_t						onReceiveData(char* ptr, size_t size) = 0;

	bool								doRequest();

protected:
	Ref<EventChannel>					_channel;
	Ref<ScriptWaitBlock>				_scriptWaitBlock;

	String								_url;
	float								_startTime;

	CURL*								_handle;
	CURLcode							_error;

	bool								_canceled : 1;

private:
	friend class NetService;
	bool								start(CURL* handle);
	void								done();
	void								error(CURLcode err);
	static int							progressCallback(URLRequest* req, double dltotal, double dlnow, double ultotal, double ulnow);
	static size_t						receiveHeaderCallback(char* ptr, size_t size, size_t nmemb, URLRequest* req);
	static size_t						receiveDataCallback(char* ptr, size_t size, size_t nmemb, URLRequest* req);
};

////////////////////////////////////////////////////////////////////////////////

class NITNET_API NetRequestEvent : public Event
{
public:
	NetRequestEvent() { }
	NetRequestEvent(URLRequest* req) : request(req) { }

	Ref<URLRequest>						request;
};

////////////////////////////////////////////////////////////////////////////////

NIT_EVENT_DECLARE(NITNET_API, OnNetRequestDone, NetRequestEvent);
NIT_EVENT_DECLARE(NITNET_API, OnNetRequestError, NetRequestEvent);

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
