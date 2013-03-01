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

#include "nit/script/NitBind.h"
#include "nit/script/NitBindMacro.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NITNET_API, nit::NetService, Service, incRefCount, decRefCount);

class NB_NetService : TNitClass<NetService>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(numActiveRequests),
			PROP_ENTRY_R(busy),
			PROP_ENTRY_R(time),
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY_H(channel,		"(): EventChannel"),
			FUNC_ENTRY_H(cancelAllRequests, "(cleanup: bool)"),
			FUNC_ENTRY_H(debugAllRequests, "(): NetRequest[]"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(numActiveRequests)		{ return push(v, self(v)->getNumActiveRequests()); }
	NB_PROP_GET(busy)					{ return push(v, self(v)->isBusy()); }
	NB_PROP_GET(time)					{ return push(v, self(v)->getTime()); }

	NB_FUNC(channel)					{ return push(v, self(v)->channel()); }

	NB_FUNC(cancelAllRequests)			{ self(v)->cancelAllRequests(getBool(v, 2)); return 0; }

	NB_FUNC(debugAllRequests)
	{
		NetService::Requests& req = self(v)->_active;
		sq_newarray(v, 0);
		for (NetService::Requests::iterator itr = req.begin(), end = req.end(); itr != end; ++itr)
		{
			arrayAppend(v, -1, itr->second.get());
		}
		return 1;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NITNET_API, nit::URLRequest, RefCounted, incRefCount, decRefCount);

class NB_URLRequest : TNitClass<URLRequest>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(handle),
			PROP_ENTRY_R(busy),
			PROP_ENTRY_R(error),
			PROP_ENTRY_R(errorString),
			PROP_ENTRY_R(url),
			PROP_ENTRY_R(typeName),
			PROP_ENTRY_R(startTime),
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY_H(cancel,		"(cleanup=true)"),
			FUNC_ENTRY_H(channel,		"(): EventChannel"),
			FUNC_ENTRY_H(wait,			"(): int // 0 : fail, 1 : success"),
			FUNC_ENTRY_H(urlEncode,		"(str:string, len=-1:int, fieldMode=false:bool):string"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(handle)					{ sq_pushuserpointer(v, self(v)->getHandle()); return 1; }
	NB_PROP_GET(busy)					{ return push(v, self(v)->isBusy()); }
	NB_PROP_GET(error)					{ return push(v, self(v)->getError()); }
	NB_PROP_GET(errorString)			{ return push(v, self(v)->getErrorString()); }
	NB_PROP_GET(url)					{ return push(v, self(v)->getUrl()); }
	NB_PROP_GET(typeName)				{ return push(v, self(v)->getTypeName()); }
	NB_PROP_GET(startTime)				{ return push(v, self(v)->getStartTime()); }

	NB_FUNC(cancel)						{ self(v)->cancel(optBool(v, 2, true)); return 0; }
	NB_FUNC(channel)					{ return push(v, self(v)->channel()); }
	NB_FUNC(wait)						{ return self(v)->wait(v); } // HACK: We can't use push the return value (as it is special value for suspended state)

	NB_FUNC(urlEncode)					{ return push(v, URLRequest::urlEncode(getString(v, 2), optInt(v, 3, -1), optBool(v, 4, false))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NITNET_API, nit::HttpRequest, URLRequest, incRefCount, decRefCount);

class NB_HttpRequest : TNitClass<HttpRequest>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(baseUrl),
			PROP_ENTRY_R(fields),
			PROP_ENTRY_R(multipart),

			PROP_ENTRY_R(downloadProgress),
			PROP_ENTRY_R(uploadProgress),

			PROP_ENTRY_R(response),
			PROP_ENTRY	(header),

			PROP_ENTRY	(userId),
			PROP_ENTRY	(userPassword),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"(url: string, urlEncoded=false)"),
			FUNC_ENTRY_H(get,			"(downloadWriter: StreamWriter=null): HttpRequest"),
			FUNC_ENTRY_H(post,			"(downloadWriter: StreamWriter=null): HttpRequest"),
			FUNC_ENTRY_H(setBaseUrl,	"(baseUrl: string, urlEncoded=false)"),
			FUNC_ENTRY_H(addFields,		"(fieldsStr: string, urlEncoded=false)\n"
										"(fields: table, urlEncoded=false)"),
			FUNC_ENTRY_H(clearFields,	"()"),
			FUNC_ENTRY_H(addMultipart,	"(name: string, value: string, filename=\"\", mimeType=\"\")\n"
										"(name: string, reader: StreamReader, filename=\"\", mimeType=\"\")"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(baseUrl)				{ return push(v, self(v)->getBaseUrl()); }
	NB_PROP_GET(fields)					{ return push(v, self(v)->getFields()); }
	NB_PROP_GET(multipart)				{ return push(v, self(v)->isMultipart()); }

	NB_PROP_GET(downloadProgress)		{ return push(v, self(v)->getDownloadProgress()); }
	NB_PROP_GET(uploadProgress)			{ return push(v, self(v)->GetUploadProgress()); }

	NB_PROP_GET(response)				{ return push(v, self(v)->getResponse()); }
	NB_PROP_GET(header)					{ return push(v, self(v)->getHeader()); }
	NB_PROP_GET(userId)					{ return push(v, self(v)->getUserId()); }
	NB_PROP_GET(userPassword)				{ return push(v, self(v)->getUserPassword()); }

	NB_PROP_SET(header)					{ self(v)->setHeader(getString(v, 2)); return 0; }
	NB_PROP_SET(userId)					{ self(v)->setUserId(getString(v, 2)); return 0; }
	NB_PROP_SET(userPassword)			{ self(v)->setUserPassword(getString(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, new HttpRequest(getString(v, 2), optBool(v, 3, false), optBool(v, 4, true))); return 0; }

	NB_FUNC(get)						{ return push(v, self(v)->get(opt<StreamWriter>(v, 2, NULL)) ? self(v) : (type*)NULL); }
	NB_FUNC(post)						{ return push(v, self(v)->post(opt<StreamWriter>(v, 2, NULL)) ? self(v) : (type*)NULL); }
	NB_FUNC(setBaseUrl)					{ self(v)->setBaseUrl(getString(v, 2), optBool(v, 3, false)); return 0; }

	NB_FUNC(addFields)
	{
		if (!isNone(v, 2) && sq_gettype(v, 2) == OT_TABLE)
			self(v)->addFields(v, 2, optBool(v, 3, false));
		else
			self(v)->addFields(getString(v, 2), optBool(v, 3, false));
		return 0;
	}

	NB_FUNC(clearFields)				{ self(v)->clearFields(); return 0; }

	NB_FUNC(addMultipart)
	{
		if (isString(v, 3))
			self(v)->addMultipart(getString(v, 2), getString(v, 3), optString(v, 4, ""), optString(v, 5, ""));
		else if (is<StreamReader>(v, 3))
			self(v)->addMultipart(getString(v, 2), get<StreamReader>(v, 3), optString(v, 4, ""), optString(v, 5, ""));

		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NITNET_API, nit::HttpResponse, StreamSource, incRefCount, decRefCount);

class NB_HttpResponse : TNitClass<HttpResponse>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(code),
			PROP_ENTRY_R(header),
			PROP_ENTRY_R(data),
			PROP_ENTRY	(name),
			PROP_ENTRY	(contentType),
			PROP_ENTRY	(mimeType),
			NULL
		};

		FuncEntry funcs[] = 
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(code)					{ return push(v, self(v)->getCode()); }
	NB_PROP_GET(header)					{ return push(v, self(v)->getHeader()); }
	NB_PROP_GET(data)					{ return push(v, self(v)->getData()); }
	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }
	NB_PROP_GET(contentType)			{ return push(v, self(v)->getContentType()); }
	NB_PROP_GET(mimeType)				{ return push(v, self(v)->getMimeType()); }

	NB_PROP_SET(name)					{ self(v)->setName(getString(v, 2)); return 0; }
	NB_PROP_SET(contentType)			{ self(v)->setContentType(*get<ContentType>(v, 2)); return 0; }
	NB_PROP_SET(mimeType)				{ self(v)->setMimeType(getString(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NITNET_API SQRESULT NitLibNet(HSQUIRRELVM v)
{
	NB_NetService::Register(v);

	NB_URLRequest::Register(v);
	NB_HttpRequest::Register(v);
	NB_HttpResponse::Register(v);

	////////////////////////////////////

	sq_pushroottable(v);

	if (g_Service)
	{
		NitBind::newSlot(v, -1, "net", g_Net);
	}

	sq_poptop(v);

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
