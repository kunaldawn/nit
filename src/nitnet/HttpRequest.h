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

#include "nitnet/URLRequest.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class HttpResponse;

class NITNET_API HttpRequest : public URLRequest
{
public:
	typedef map<String, String>::type StringTable;

	HttpRequest(const String& url, bool encoded = false, bool setFields = true);

public:
	enum RequestType					{ HTTP_GET, HTTP_POST, HTTP_MULTIPART };

	void								setBaseUrl(const String& baseURL, bool encoded = false);
	const String&						getBaseUrl()							{ return _baseURL; }

	const String&						getFields()								{ return _fields; }
	bool								isMultipart()							{ return _requestType == HTTP_MULTIPART; }

	bool								get(StreamWriter* downloadWriter = NULL);
	bool								post(StreamWriter* downloadWriter = NULL);

	const String&						getHeader()								{ return _header; }
	void								setHeader(const String& header);

	const String&						getUserId()								{ return _userID; }
	void								setUserId(const String& id);

	const String&						getUserPassword()						{ return _userPassword; }
	void								setUserPassword(const String& pw);

	void								addFields(const String& fieldsStr, bool encoded = false);
	void								addFields(const StringTable& fields, bool encoded = false);
	void								addFields(HSQUIRRELVM v, SQInteger tableIdx, bool encoded = false);

	void								clearFields()							{ _fields.clear(); }

	void								addMultipart(const String& name, const String& value, const String& filename = StringUtil::BLANK(), const String& mimeType = StringUtil::BLANK());
	void								addMultipart(const String& name, StreamReader* reader, const String& filename = StringUtil::BLANK(), const String& mimeType = StringUtil::BLANK());

public:
	float								getDownloadProgress()					{ return _downloadProgress; }
	float								GetUploadProgress()						{ return _uploadProgress; }

public:
	virtual const char*					getTypeName();

	HttpResponse*						getResponse()							{ return isBusy() ? NULL : _response; }

protected:
	virtual bool						onStart(CURL* handle);
	virtual int							onProgress(double dltotal, double dlnow, double ultotal, double ulnow);
	virtual size_t						onReceiveHeader(char* ptr, size_t size);
	virtual size_t						onReceiveData(char* ptr, size_t size);
	virtual void						onDone();
	virtual void						onError(CURLcode err);

	virtual void						onDelete();

	void								oonvertToMultipart(CURL* handle, const String& fields);

	String								_baseURL;
	String								_fields;
	RequestType							_requestType;
	String								_header;

	String								_userID;
	String								_userPassword;

	Ref<HttpResponse>					_response;

	float								_downloadProgress;
	float								_uploadProgress;

	class MultipartForm;

	MultipartForm*						_multipartForm;

	struct curl_slist*					_headerSList;

private:
	static size_t						multipartReadCallback(char* buffer, size_t size, size_t nitems, void* entry);
};

////////////////////////////////////////////////////////////////////////////////

class NITNET_API HttpResponse : public StreamSource
{
public:
	int									getCode()								{ return _code; }
	MemoryBuffer*						getHeader()								{ return _header; }
	MemoryBuffer*						getData()								{ return _data; }

public:									// StreamSource implementation
	virtual size_t						getStreamSize()							{ return _data ? _data->getSize() : 0; }
	virtual size_t						getMemorySize()							{ return _data ? _data->getSize() : 0; }
	virtual Timestamp					getTimestamp()							{ return _timestamp; }
	virtual StreamReader*				open();
	virtual String						_makeUrl()								{ return _url; }

public:
	const String&						getMimeType()							{ return _mimeType; }

	void								setName(const String& name)				{ _name = name; }
	void								setContentType(const ContentType& ct);
	void								setMimeType(const String& mimeType);

protected:
	int									_code;
	Ref<MemoryBuffer>					_header;
	Ref<MemoryBuffer>					_data;
	Ref<StreamWriter>					_downloadWriter;
	String								_baseUrl;
	String								_url;
	Timestamp							_timestamp;
	String								_mimeType;

private:
	friend class HttpRequest;
	HttpResponse();
	size_t								receiveHeader(char* ptr, size_t size);
	size_t								receiveData(char* ptr, size_t size);
	void								requestComplete(CURL* handle);
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
