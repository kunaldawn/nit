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

#include "nitnet/NetService.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NIT_EVENT_DEFINE(NET_REQUEST_DONE, NetRequestEvent);
NIT_EVENT_DEFINE(NET_REQUEST_ERROR, NetRequestEvent);

////////////////////////////////////////////////////////////////////////////////

URLRequest::URLRequest()
{
	_handle = NULL;
	_error = CURLE_OK;

	_canceled = false;

	_startTime = 0.0f;
}

String URLRequest::urlEncode(const char* c, int len, bool fieldMode)
{
	String escaped;
	int max = len == -1 ? strlen(c) : len;
	for(int i=0; i<max; i++)
	{
		if ( (48 <= c[i] && c[i] <= 57) ||//0-9
			(65 <= c[i] && c[i] <= 90) ||//ABC...XYZ
			(97 <= c[i] && c[i] <= 122) || //abc...xyz
			(c[i]=='~' || c[i]=='-' || c[i]=='_' || c[i]=='.')
			)
		{
			escaped.push_back(c[i]);
		}
		else
		{
			if (fieldMode && (c[i] == '=' || c[i] == '&'))
				escaped.push_back(c[i]);
			else
			{
				escaped.push_back('%');

				char dec = c[i];
				char dig1 = (dec&0xF0)>>4;
				char dig2 = (dec&0x0F);
				if ( 0<= dig1 && dig1<= 9) dig1+=48;    //0,48 in ascii
				if (10<= dig1 && dig1<=15) dig1+=65-10; //A,65 in ascii
				if ( 0<= dig2 && dig2<= 9) dig2+=48;
				if (10<= dig2 && dig2<=15) dig2+=65-10;
				
				escaped.push_back(dig1);
				escaped.push_back(dig2);
			}
		}
	}
	return escaped;
}

bool URLRequest::doRequest()
{
	if (_handle) 
	{
		NIT_THROW_FMT(EX_INVALID_STATE, "%s '%s' bound yet", getTypeName(), _url.c_str());
		return false;
	}

	_canceled = false;
	_startTime = g_Net->getTime();

	return g_Net->startRequest(this);
}

bool URLRequest::start(CURL* handle)
{
	_handle = handle;

	curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt(handle, CURLOPT_PROGRESSFUNCTION, progressCallback);
	curl_easy_setopt(handle, CURLOPT_PROGRESSDATA, this);

	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, receiveDataCallback);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, this);

	curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, receiveHeaderCallback);
	curl_easy_setopt(handle, CURLOPT_HEADERDATA, this);

	return onStart(handle);
}

void URLRequest::cancel(bool cleanup)
{
	if (_handle == NULL || !cleanup)
		return;

	g_Net->cancelRequest(this, cleanup);

	if (cleanup)
		_handle = NULL;

	LOG(0, "*** %s '%s' canceled\n", getTypeName(), _url.c_str());

	_canceled = true;
	_error = CURLE_ABORTED_BY_CALLBACK;
	onError(_error);

	if (_scriptWaitBlock)
		_scriptWaitBlock->signal(0x00);
}

void URLRequest::done()
{
	if (_canceled)
	{
		_handle = NULL;
		return;
	}

	_error = CURLE_OK;
	onDone();

	_handle = NULL;

	if (_channel) 
		_channel->send(EVT::NET_REQUEST_DONE, new NetRequestEvent(this));

	if (_scriptWaitBlock)
		_scriptWaitBlock->signal(0x01);
}

void URLRequest::error(CURLcode err)
{
	if (_canceled)
	{
		_handle = NULL;
		return;
	}

	_error = err;
	LOG(0, "*** %s '%s' failed: %s\n", getTypeName(), _url.c_str(), getErrorString());

	onError(err);

	_handle = NULL;

	if (_channel) 
		_channel->send(EVT::NET_REQUEST_ERROR, new NetRequestEvent(this));

	if (_scriptWaitBlock)
		_scriptWaitBlock->signal(0x00);
}

int URLRequest::progressCallback(URLRequest* req, double dltotal, double dlnow, double ultotal, double ulnow)
{
	if (req->_canceled) return 0; // signal curl to abort the transfer
	return req->onProgress(dltotal, dlnow, ultotal, ulnow);
}

size_t URLRequest::receiveHeaderCallback(char* ptr, size_t size, size_t nmemb, URLRequest* req)
{
	if (req->_canceled) return 0; // signal curl to abort the transfer
	return req->onReceiveHeader(ptr, size * nmemb);
}

size_t URLRequest::receiveDataCallback(char* ptr, size_t size, size_t nmemb, URLRequest* req)
{
	if (req->_canceled) return 0; // signal curl to abort the transfer
	return req->onReceiveData(ptr, size * nmemb);
}

SQRESULT URLRequest::wait(HSQUIRRELVM v)
{
	if (_handle == NULL)
	{
		bool started = doRequest();
		if (!started)
			return NitBind::push(v, 0);
	}

	if (_scriptWaitBlock == NULL)
		_scriptWaitBlock = new ScriptWaitBlock(ScriptRuntime::getRuntime(v));

	return _scriptWaitBlock->wait(v, 0x01);
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
