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

#include "nitnet_pch.h"

#include "nitnet/URLRequest.h"
#include "nitnet/HttpRequest.h"
#include "nitnet/NetService.h"

#include "nit/runtime/MemManager.h"
#include "nit/event/Timer.h"
#include "nit/net/Socket.h"

#include "nit/app/AppBase.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NetService::NetService(Package* package) : Service("NetService", package, SVC_NET)
{
}

NetService::~NetService()
{
}

void NetService::onRegister()
{

}

void NetService::onUnregister()
{

}

static void* my_curl_malloc(size_t size)
{
	return g_MemManager->Allocate(size, MEM_DEFAULT_ALIGNMENT, MEM_HINT_NONE);
}

static void my_curl_free(void* ptr)
{
	g_MemManager->deallocate(ptr, 0);
}

static void* my_curl_realloc(void* ptr, size_t size)
{
	return g_MemManager->reallocate(ptr, size, 0, MEM_DEFAULT_ALIGNMENT, MEM_HINT_NONE);
}

static char* my_curl_strdup(const char* str)
{
	size_t size = strlen(str);
	char* newstr = (char*)g_MemManager->Allocate(size + 1, MEM_DEFAULT_ALIGNMENT, MEM_HINT_NONE);
	memcpy(newstr, str, size);

	return newstr;
}

static void* my_curl_calloc(size_t nmemb, size_t size)
{
	return g_MemManager->Allocate(nmemb * size, MEM_DEFAULT_ALIGNMENT, MEM_HINT_NONE);
}

void NetService::onInit()
{
	LOG(0, "++ NitNet %s\n", curl_version());

	SocketBase::initialize();

	// Initialize cURL
	bool wsaInitNeeded = false;
	bool useMemManager = false; // TODO: not working

	long flags = wsaInitNeeded ? CURL_GLOBAL_WIN32 : 0;
	CURLcode err;

	if (useMemManager)
		err = curl_global_init_mem(flags, my_curl_malloc, my_curl_free, my_curl_realloc, my_curl_strdup, my_curl_calloc);
	else
		err = curl_global_init(flags);

	if (err)
		NIT_THROW_FMT(EX_NET, "cURL global init failed: %s", curl_easy_strerror(err));

	// Initialize Multi Handle
	_multiHandle = curl_multi_init();

	if (_multiHandle == NULL)
		NIT_THROW_FMT(EX_NET, "can't init cURL multi handle");

	// register to app's timer channel
	_time = g_App->getTimer()->getTime();
	g_App->getTimer()->channel()->bind(EVT::TICK, this, &NetService::onTick);
}

void NetService::onFinish()
{
	// unbind timer
	g_App->getTimer()->channel()->unbind(0, this);

	// remove and cleanup all handles
	cancelAllRequests(true);

	// cleanup multi handle
	if (_multiHandle)
	{
		curl_multi_cleanup(_multiHandle);
		_multiHandle = NULL;
	}

	// Finish cURL
	curl_global_cleanup();
}

void NetService::onContextEnter()
{

}

void NetService::onTick(const TimeEvent* evt)
{
	_time = evt->getTime();

	if (_active.empty()) return;

	int numRunning = 0;
	CURLMcode err = curl_multi_perform(_multiHandle, &numRunning);

	if (err)
		LOG(0, "*** curl_multi_perform error: %s\n", curl_multi_strerror(err));

	if (_active.size() == numRunning)
		return;

	int msgsInQueue = 0;
	while (true)
	{
		CURLMsg* msg = curl_multi_info_read(_multiHandle, &msgsInQueue);
		if (msg == NULL)
			break;

		switch (msg->msg)
		{
		case CURLMSG_DONE:
			{
				CURL* handle = msg->easy_handle;
				CURLcode err = msg->data.result;

				Requests::iterator itr = _active.find(handle);
				if (itr != _active.end())
				{
					URLRequest* req = itr->second.get();

					if (err)
						req->error(err);
					else
						req->done();

					// TODO: handle pooling & reusing
					curl_multi_remove_handle(_multiHandle, handle);
					curl_easy_cleanup(handle);
					_active.erase(itr);
				}
				else
				{
					// The handle losts its binding somehow. Just remove it.
					curl_multi_remove_handle(_multiHandle, handle);
					curl_easy_cleanup(handle);
				}
			}
			break;

		default:
			LOG(0, "*** curl unknown msg: %d\n", msg->msg);
		}
	}
}

bool NetService::startRequest(URLRequest* req)
{
	CURL* handle = curl_easy_init();
	bool ok = req->start(handle);

	if (!ok)
	{
		req->onError(CURLE_ABORTED_BY_CALLBACK);
		return false;
	}

	CURLMcode err = curl_multi_add_handle(_multiHandle, handle);
	if (err)
	{
		req->onError(CURLE_FAILED_INIT);
		curl_easy_cleanup(handle);
		NIT_THROW_FMT(EX_NET, "cURL add handle failed: %s", curl_multi_strerror(err));
		return false;
	}

	_active.insert(std::make_pair(handle, req));

	return true;
}

void NetService::cancelRequest(URLRequest* req, bool cleanup)
{
	CURL* handle = req->getHandle();

	// TODO: Find a way to notify cancelation to curl.

	if (cleanup)
	{
		Requests::iterator itr = _active.find(handle);
		if (itr != _active.end())
		{
			// WARNING: There's a little blocking when curl cleans up.
	 		curl_multi_remove_handle(_multiHandle, handle);
	 		curl_easy_cleanup(handle);
			_active.erase(itr);
		}
	}
}

void NetService::cancelAllRequests(bool cleanup)
{
	if (_multiHandle == NULL)
		return;

	if (cleanup)
	{
		for (Requests::iterator itr = _active.begin(), end = _active.end(); itr != end; ++itr)
		{
			CURL* handle = itr->first;
			URLRequest* req = itr->second;

			req->_handle = NULL;
			req->cancel(false);
			curl_multi_remove_handle(_multiHandle, handle);
			curl_easy_cleanup(handle);
		}
	}
	else
	{
		for (Requests::iterator itr = _active.begin(), end = _active.end(); itr != end; ++itr)
		{
			URLRequest* req = itr->second;
			req->cancel(false);
		}
	}
	_active.clear();
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
