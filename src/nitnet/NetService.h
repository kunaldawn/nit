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

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

#define g_Net							static_cast<nit::NetService*>(g_Service->getService(::nit::Service::SVC_NET))

////////////////////////////////////////////////////////////////////////////////

class URLRequest;
class HttpRequest;
class TimeEvent;

class NITNET_API NetService : public Service
{
public:
	NetService(Package* package);
	virtual ~NetService();

public:
	size_t								getNumActiveRequests()					{ return _active.size(); }
	bool								isBusy()								{ return !_active.empty(); }
	void								cancelAllRequests(bool cleanup);

public:
	float								getTime()								{ return _time; }
	EventChannel*						channel()								{ return _channel ? _channel : _channel = new EventChannel(); }

public:									// Module implementation
	virtual void						onRegister();
	virtual void						onUnregister();
	virtual void						onInit();
	virtual void						onFinish();
	virtual void						onContextEnter();

protected:
	Ref<EventChannel>					_channel;
	float								_time;

private:
	friend class URLRequest;
	bool								startRequest(URLRequest* req);
	void								cancelRequest(URLRequest* req, bool cleanup);

private:
	void								onTick(const TimeEvent* evt);

	friend class NB_NetService;
	typedef map<CURL*, Ref<URLRequest> >::type Requests;
	Requests							_active;

	CURLM*								_multiHandle;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
